﻿/*
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

#include "ArcFileContent.h"
#include "EventDispatcher.h"
#include "FileListMessages.h"
#include "Utilities/FileOperation.h"
#include "compress.h"

enum class FILEINFO_TYPE : int {
	INVALID = -1,
	FILENAME,
	FULLPATH,
	ORIGINALSIZE,
	TYPENAME,
	FILETIME,
	COMPRESSEDSIZE,
	METHOD,
	RATIO,		//compression ratio

	ENUM_COUNT_AND_LASTITEM,
};

class CFileListModel:public CEventDispatcher
{
protected:
	CArchiveFileContent	m_Content;
	const ARCHIVE_ENTRY_INFO*	m_lpCurrentDir;
	ARCHIVE_ENTRY_INFO	m_FoundItems;

	std::vector<std::shared_ptr<ARCHIVE_ENTRY_INFO> >	m_SortedChildren;
	CTemporaryDirectoryManager	m_TempDirManager;
	const LF_COMPRESS_ARGS &mr_compressArgs;

	//sort
	bool	m_bSortAtoZ;
	int		m_nSortKeyType;
protected:
	//---internal functions
	void SortCurrentEntries();
public:
	CFileListModel(const LF_COMPRESS_ARGS& compressArgs, std::shared_ptr<ILFPassphrase> passphrase) :
		m_lpCurrentDir(nullptr),
		m_bSortAtoZ(true),
		m_nSortKeyType((int)FILEINFO_TYPE::INVALID),
		m_Content(passphrase),
		mr_compressArgs(compressArgs)
	{}
	virtual ~CFileListModel() {}

	void Open(const std::filesystem::path& path, ILFScanProgressHandler&);
	void Clear() {
		m_Content.clear();
		m_lpCurrentDir = nullptr;
		m_SortedChildren.clear();

		m_FoundItems.clear();
	}
	std::wstring getCurrentDirPath()const;
	void setCurrentDirPath(const std::wstring& path);

	const ARCHIVE_ENTRY_INFO* getCurrentDir()const{return m_lpCurrentDir;}
	void setCurrentDir(const ARCHIVE_ENTRY_INFO* lpN);

	void SetSortKeyType(int nSortKeyType) {
		if (m_nSortKeyType != nSortKeyType) {
			m_nSortKeyType = nSortKeyType;
			SortCurrentEntries();
			dispatchEvent(WM_FILELIST_UPDATED);
		}
	}
	void SetSortAtoZ(bool bAtoZ) {
		m_bSortAtoZ = bAtoZ;
		SortCurrentEntries();
		dispatchEvent(WM_FILELIST_UPDATED);
	}
	int GetSortKeyType()const{return m_nSortKeyType;}
	bool IsSortAtoZ()const{return m_bSortAtoZ;}

	bool MoveUpDir();
	bool MoveDownDir(const ARCHIVE_ENTRY_INFO*);

	bool IsRoot()const { return (getCurrentDir() == m_Content.getRootNode()); }
	bool IsOK()const{return m_Content.isOK();}

	const ARCHIVE_ENTRY_INFO* GetFileListItemByIndex(int iIndex)const;

	bool IsFindMode()const { return m_lpCurrentDir == &m_FoundItems; }
	const ARCHIVE_ENTRY_INFO* FindItem(const ARCHIVE_FIND_CONDITION& condition, const ARCHIVE_ENTRY_INFO *parent=nullptr) {
		m_FoundItems._children = m_Content.findItem(condition, parent);
		m_FoundItems._parent = nullptr;
		return &m_FoundItems;
	}
	void EndFindItem() {
		if (IsFindMode()) {
			setCurrentDir(m_Content.getRootNode());
		}
	}

	std::filesystem::path GetArchiveFileName()const{return m_Content.getArchivePath();}
	ARCHIVE_ENTRY_INFO* GetRootNode(){return m_Content.getRootNode();}
	const ARCHIVE_ENTRY_INFO* GetRootNode()const{return m_Content.getRootNode();}

	bool IsArchiveEncrypted()const{return m_Content.isArchiveEncrypted();}
	bool IsModifySupported()const { return m_Content.isModifySupported(); }
	BOOL CheckArchiveExists()const{return m_Content.checkArchiveExists();}

	void AddItem(
		const std::vector<std::filesystem::path> &files,
		const ARCHIVE_ENTRY_INFO* parent,
		ILFProgressHandler& progressHandler,
		ILFOverwriteInArchiveConfirm& overwriteConfirmHandler,
		ARCLOG &arcLog) {
		m_Content.addEntries(
			mr_compressArgs,
			files,
			parent,
			progressHandler,
			overwriteConfirmHandler,
			arcLog);
	}
	void DeleteItems(
		const std::vector<const ARCHIVE_ENTRY_INFO*>& items,
		ILFProgressHandler& progressHandler,
		ARCLOG &arcLog) {
		m_Content.deleteEntries(mr_compressArgs, items, progressHandler, arcLog);
	}
	void ExtractItems(
		const std::vector<const ARCHIVE_ENTRY_INFO*> &items,
		const std::filesystem::path &outputDir,
		const ARCHIVE_ENTRY_INFO* lpBase,
		ILFProgressHandler& progressHandler,
		ARCLOG &arcLog) {
		m_Content.extractEntries(items, outputDir, lpBase, progressHandler, arcLog);
	}

	std::vector<std::filesystem::path> MakeSureItemsExtracted(
		const std::vector<const ARCHIVE_ENTRY_INFO*> &items,
		std::filesystem::path outputDir,
		const ARCHIVE_ENTRY_INFO* lpBase,
		ILFProgressHandler& progressHandler,
		overwrite_options options,
		ARCLOG &arcLog) {
		return m_Content.makeSureItemsExtracted(
			items,
			outputDir,
			lpBase,
			progressHandler,
			options,
			arcLog);
	}

	bool ExtractArchive(ILFProgressHandler &progressHandler);
	bool TestArchive(ILFProgressHandler &progressHandler);

	bool ClearTempDir() {
		return UtilDeleteDir(m_TempDirManager.path(), false);
	}
	std::filesystem::path getTempDir()const { return m_TempDirManager.path(); }
};


