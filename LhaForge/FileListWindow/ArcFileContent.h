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
#include "ConfigCode/ConfigManager.h"

struct ARCHIVE_ENTRY_INFO;
struct IArchiveContentUpdateHandler {
	virtual ~IArchiveContentUpdateHandler() {}
	virtual void onUpdated(ARCHIVE_ENTRY_INFO&) = 0;
	virtual bool isAborted() = 0;
};

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

	bool isDirectory()const { return (!_children.empty()) || ((_nAttribute & S_IFDIR) != 0); }
	std::wstring getExt()const {
		if (isDirectory())return L"";
		else return std::filesystem::path(_fullpath).extension();
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
#ifdef UNIT_TEST
public:
#else
protected:
#endif
	ARCHIVE_ENTRY_INFO m_Root;

	std::wstring	m_pathArchive;
	bool			m_bExtractEachSupported;
	bool			m_bReadOnly;

	bool			m_bEncrypted;	//true if at least one entry is encrypted

protected:
	//---internal functions
	std::vector<std::shared_ptr<ARCHIVE_ENTRY_INFO> > findSubItem(const std::wstring& pattern, const ARCHIVE_ENTRY_INFO* parent)const;

	void PostProcess(ARCHIVE_ENTRY_INFO*);
	void CollectUnextractedFiles(LPCTSTR lpOutputDir,const ARCHIVE_ENTRY_INFO* lpBase,const ARCHIVE_ENTRY_INFO* lpParent,std::map<const ARCHIVE_ENTRY_INFO*,std::list<ARCHIVE_ENTRY_INFO*> > &toExtractList);

public:
	CArchiveFileContent(): m_bReadOnly(false),m_bEncrypted(false), m_bExtractEachSupported(false){
		clear();
	}
	virtual ~CArchiveFileContent() {}
	void clear() {
		m_Root.clear();
		m_bReadOnly = false;
		m_pathArchive.clear();
		m_bExtractEachSupported = false;
		m_bEncrypted = false;
	}
	void inspectArchiveStruct(const std::wstring& archiveName, IArchiveContentUpdateHandler* lpHandler);
	//処理対象アーカイブ名を取得
	LPCTSTR GetArchiveFileName()const{return m_pathArchive.c_str();}

	std::vector<std::shared_ptr<ARCHIVE_ENTRY_INFO> > FindItem(const wchar_t* name_or_pattern, const ARCHIVE_ENTRY_INFO* parent)const {
		if (!parent)parent = &m_Root;
		auto pattern = replace(name_or_pattern, L"\\", L"/");
		return findSubItem(pattern, parent);
	}

	ARCHIVE_ENTRY_INFO* GetRootNode(){return &m_Root;}
	const ARCHIVE_ENTRY_INFO* GetRootNode()const{return &m_Root;}

	bool IsArchiveEncrypted()const{return m_bEncrypted;}
	BOOL CheckArchiveExists()const{return PathFileExists(m_pathArchive.c_str());}
	bool IsOK()const { return false; /*TODO*/
#pragma message("FIXME!")
	}

	HRESULT AddItem(const std::list<CString>&,LPCTSTR lpDestDir,CConfigManager& rConfig,CString&);	//ファイルを追加圧縮
	bool ExtractItems(CConfigManager&,const std::list<ARCHIVE_ENTRY_INFO*> &items,LPCTSTR lpszDir,const ARCHIVE_ENTRY_INFO* lpBase,bool bCollapseDir,CString &strLog);
	//bOverwrite:trueなら存在するテンポラリファイルを削除してから解凍する
	bool MakeSureItemsExtracted(CConfigManager&,LPCTSTR lpOutputDir,const ARCHIVE_ENTRY_INFO* lpBase,const std::list<ARCHIVE_ENTRY_INFO*> &items,std::list<CString> &r_filesList,bool bOverwrite,CString &strLog);
	bool DeleteItems(CConfigManager&,const std::list<ARCHIVE_ENTRY_INFO*>&,CString&);
};

