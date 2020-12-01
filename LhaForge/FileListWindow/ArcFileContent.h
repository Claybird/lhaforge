/*
* MIT License

* Copyright (c) 2005- Claybird

* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:

* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.

* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/

#pragma once
#include "ArchiverCode/arc_interface.h"

struct ARCHIVE_ENTRY_INFO;
struct IArchiveContentUpdateHandler {
	virtual ~IArchiveContentUpdateHandler() {}
	virtual void onUpdated(ARCHIVE_ENTRY_INFO&) = 0;
	virtual bool isAborted() = 0;
};

//TODO
struct CConfigManager;

//reconstructed archive content structure
struct ARCHIVE_ENTRY_INFO {
	std::vector<std::shared_ptr<ARCHIVE_ENTRY_INFO> > _children;
	ARCHIVE_ENTRY_INFO*	_parent;

	std::wstring _entryName;
	std::wstring _fullpath;

	int			_nAttribute;

	//for file, original file size. / for directory, sum of children file size
	UINT64		_originalSize;
	__time64_t	_st_mtime;

	//---
	ARCHIVE_ENTRY_INFO() { clear(); }
	virtual ~ARCHIVE_ENTRY_INFO() {}

	static const wchar_t* dirDummyExt() {
		const wchar_t* FOLDER_EXTENSION_STRING = L"***directory/dummy/extension***";
		return FOLDER_EXTENSION_STRING;
	}

	bool isDirectory()const { return (!_children.empty()) || ((_nAttribute & S_IFDIR) != 0); }
	std::wstring getExt()const {
		if (isDirectory()) {
			return dirDummyExt();
		} else {
			auto fname = std::filesystem::path(_fullpath).filename();
			if (!fname.empty() && fname.wstring()[0]==L'.') {
				//ex. ".gitignore"
				return fname;
			} else {
				auto ext = fname.extension();
				if (ext.empty()) {
					return L"file_without_extension";
				} else {
					return ext;
				}
			}
		}
	}

	size_t getNumChildren()const { return _children.size(); }
	ARCHIVE_ENTRY_INFO* getChild(size_t idx)const {
		if (0 <= idx && idx < getNumChildren()) {
			return _children[idx].get();
		} else {
			return nullptr;
		}
	}
	ARCHIVE_ENTRY_INFO* getChild(const std::wstring &name)const {
		for(const auto& entry: _children){
			if (0 == _wcsicmp(entry->_entryName.c_str(), name.c_str())) {
				return entry.get();
			}
		}
		return nullptr;
	}
	ARCHIVE_ENTRY_INFO* makeSureChildExists(const std::wstring& entryName) {
		auto p = getChild(entryName);
		if (p)return p;
		auto s = std::make_shared<ARCHIVE_ENTRY_INFO>();
		s->_entryName = entryName;
		s->_parent = this;
		_children.push_back(s);
		return s.get();
	}
	ARCHIVE_ENTRY_INFO& addEntry(const std::vector<std::wstring> &elements) {
		if (elements.empty()) {
			return *this;
		} else {
			const auto entryName = elements.front();
			auto p = makeSureChildExists(entryName);
			auto subVector = std::vector<std::wstring>(std::next(elements.begin()), elements.end());
			return p->addEntry(subVector);
		}
	}
	void clear() {
		_children.clear();
		_parent = nullptr;
		_entryName.clear();
		_fullpath.clear();

		_nAttribute = 0;
		_originalSize = -1;
		_st_mtime = 0;
	}

	std::wstring getFullpath()const {
		if (_fullpath.empty()) {
			if (_parent) {
				auto parent_name = _parent->getFullpath();
				if (parent_name.empty()) {
					return _entryName;
				} else {
					return parent_name + L"/" + _entryName;
				}
			} else {
				return L"";
			}
		} else {
			return _fullpath;
		}
	}
	//returns pathname from base to this
	std::wstring getRelativePath(const ARCHIVE_ENTRY_INFO* base)const {
		if (_parent && base != this) {
			auto parent_name = _parent->getRelativePath(base);
			if (parent_name.empty()) {
				return _entryName;
			} else {
				return parent_name + L"/" + _entryName;
			}
		} else {
			return L"";
		}
	}
	//enumerate myself and children
	std::vector<std::wstring> enumFiles()const {
		std::vector<std::wstring> files;
		if (isDirectory()) {
			for (const auto &entry: _children) {
				auto subFiles = entry->enumFiles();
				files.insert(files.end(), subFiles.begin(), subFiles.end());
			}
		}
		if (_parent) {
			//when _parent is nullptr, it is the virtual root
			files.push_back(getFullpath());
		}
		return files;
	}
};

class CArchiveFileContent{
protected:
	std::shared_ptr<ARCHIVE_ENTRY_INFO> m_pRoot;

	std::wstring	m_pathArchive;
	bool			m_bExtractEachSupported;
	bool			m_bReadOnly;

	bool			m_bEncrypted;	//true if at least one entry is encrypted

protected:
	//---internal functions
	std::vector<std::shared_ptr<ARCHIVE_ENTRY_INFO> > findSubItem(
		const std::wstring& pattern,
		const ARCHIVE_ENTRY_INFO* parent
	)const;

	void postInspectArchive(ARCHIVE_ENTRY_INFO*);
	void collectUnextractedFiles(const std::wstring& outputDir,const ARCHIVE_ENTRY_INFO* lpBase,const ARCHIVE_ENTRY_INFO* lpParent,std::map<const ARCHIVE_ENTRY_INFO*,std::vector<ARCHIVE_ENTRY_INFO*> > &toExtractList);

public:
	CArchiveFileContent(): m_bReadOnly(false),m_bEncrypted(false), m_bExtractEachSupported(false){
		clear();
	}
	virtual ~CArchiveFileContent() {}
	void clear() {
		m_pRoot.reset();
		m_bReadOnly = false;
		m_pathArchive.clear();
		m_bExtractEachSupported = false;
		m_bEncrypted = false;
	}
	void inspectArchiveStruct(const std::wstring& archiveName, IArchiveContentUpdateHandler* lpHandler);
	const wchar_t* getArchivePath()const { return m_pathArchive.c_str(); }
	const ARCHIVE_ENTRY_INFO* getRootNode()const { return m_pRoot.get(); }
	ARCHIVE_ENTRY_INFO* getRootNode(){ return m_pRoot.get(); }

	std::vector<std::shared_ptr<ARCHIVE_ENTRY_INFO> > findItem(
		const wchar_t* name_or_pattern,
		const ARCHIVE_ENTRY_INFO* parent = nullptr
	)const {
		if (!parent)parent = m_pRoot.get();
		auto pattern = replace(name_or_pattern, L"\\", L"/");
		return findSubItem(pattern, parent);
	}

	bool isArchiveEncrypted()const { return m_bEncrypted; }

	bool checkArchiveExists()const { return std::filesystem::exists(m_pathArchive); }

	//TODO
	bool IsOK()const { return m_pRoot.get() != nullptr; }

	void extractItems(
		CConfigManager&,
		const std::vector<ARCHIVE_ENTRY_INFO*> &items,
		const std::wstring& outputDir,
		const ARCHIVE_ENTRY_INFO* lpBase,
		bool bCollapseDir,
		ARCLOG &strLog);
	HRESULT AddItem(const std::vector<std::wstring>&,LPCTSTR lpDestDir,CConfigManager& rConfig,CString&);	//ファイルを追加圧縮
	//bOverwrite:trueなら存在するテンポラリファイルを削除してから解凍する
	bool MakeSureItemsExtracted(
		CConfigManager&,
		const std::wstring &outputDir,
		bool bOverwrite,
		const ARCHIVE_ENTRY_INFO* lpBase,
		const std::vector<ARCHIVE_ENTRY_INFO*> &items,
		std::vector<std::wstring> &r_extractedFiles,
		std::wstring &strLog);
	bool DeleteItems(CConfigManager&,const std::list<ARCHIVE_ENTRY_INFO*>&,CString&);
};

