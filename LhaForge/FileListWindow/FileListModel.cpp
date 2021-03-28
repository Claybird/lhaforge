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

#include "stdafx.h"
#include "FileListModel.h"
#include "ConfigCode/ConfigFile.h"
#include "ConfigCode/ConfigFileListWindow.h"
#include "ConfigCode/ConfigExtract.h"
#include "Utilities/StringUtil.h"
#include "Utilities/FileOperation.h"
#include "Utilities/Utility.h"
#include "CommonUtil.h"
#include "extract.h"
#include "FileListMessages.h"
#include "resource.h"

void CFileListModel::Open(
	const std::filesystem::path& path,
	ILFScanProgressHandler& progressHandler)
{
	Clear();
	m_Content.scanArchiveStruct(path, progressHandler);

	m_lpCurrentDir = m_Content.getRootNode();
	dispatchEvent(WM_FILELIST_ARCHIVE_LOADED);

	SortCurrentEntries();
}

std::wstring CFileListModel::getCurrentDirPath()const
{
	std::wstring path;
	if(m_lpCurrentDir){
		path = m_lpCurrentDir->calcFullpath();
	}
	return path;
}

void CFileListModel::setCurrentDirPath(const std::wstring& path)
{
	//try to restore path
	auto pathElements = UtilSplitString(path, L"/");
	m_lpCurrentDir = m_Content.getRootNode();
	for (const auto& p : pathElements) {
		auto c = m_lpCurrentDir->getChild(p);
		if (c) {
			m_lpCurrentDir = c;
		} else {
			break;
		}
	}
	dispatchEvent(WM_FILELIST_NEWCONTENT);
	SortCurrentEntries();
}

void CFileListModel::setCurrentDir(const ARCHIVE_ENTRY_INFO* lpN)
{
	ASSERT(lpN);
	m_lpCurrentDir = lpN;
	dispatchEvent(WM_FILELIST_NEWCONTENT);
	SortCurrentEntries();
}

bool CFileListModel::MoveUpDir()
{
	if (!m_lpCurrentDir)return false;

	ARCHIVE_ENTRY_INFO* lpNode = m_lpCurrentDir->_parent;
	if (!lpNode) {
		return false;
	}
	m_lpCurrentDir = lpNode;

	SortCurrentEntries();
	dispatchEvent(WM_FILELIST_NEWCONTENT);
	return true;
}

bool CFileListModel::MoveDownDir(const ARCHIVE_ENTRY_INFO* toDir)
{
	if(!toDir || !toDir->is_directory()){
		return false;
	}
	m_lpCurrentDir = toDir;
	dispatchEvent(WM_FILELIST_NEWCONTENT);

	SortCurrentEntries();
	return true;
}

const ARCHIVE_ENTRY_INFO* CFileListModel::GetFileListItemByIndex(int iIndex)const
{
	ASSERT(m_lpCurrentDir);
	if(!m_lpCurrentDir)return nullptr;

	size_t numChildren=m_lpCurrentDir->getNumChildren();

	if(iIndex<0 || numChildren<=(unsigned)iIndex)return nullptr;

	if(FILEINFO_INVALID==m_nSortKeyType || m_SortedChildren.empty()){	//not sorted
		return m_lpCurrentDir->getChild(iIndex);
	}else{
		if(m_bSortDescending){
			return m_SortedChildren[iIndex].get();
		}else{
			return m_SortedChildren[numChildren-1-iIndex].get();
		}
	}
}


struct FILELIST_SORT_COMPARATOR{
	FILEINFO_TYPE Type;
	bool bReversed;
	bool operator()(const std::shared_ptr<ARCHIVE_ENTRY_INFO>& x, const std::shared_ptr<ARCHIVE_ENTRY_INFO>& y)const {
		return compare_no_reversed(x, y) ^ bReversed;
	}
	bool compare_no_reversed(const std::shared_ptr<ARCHIVE_ENTRY_INFO>& x, const std::shared_ptr<ARCHIVE_ENTRY_INFO>& y)const {
		switch(Type){
		case FILEINFO_FILENAME:
			{
				//directory priority
				if(x->is_directory()){
					if(!y->is_directory()){
						return true;
					}
				}else if(y->is_directory()){
					return false;
				}
				int result = _tcsicmp(x->_entryName.c_str(), y->_entryName.c_str());
				if(result == 0){
					//sort by pathname
					return (_tcsicmp(x->_entry.path.c_str(), y->_entry.path.c_str())<0);
				}else{
					return (result<0);
				}
			}
		case FILEINFO_FULLPATH:
			return (_tcsicmp(x->_entry.path.c_str(), y->_entry.path.c_str())<0);
		case FILEINFO_ORIGINALSIZE:
			if(x->_originalSize == y->_originalSize){
				//sort by pathname
				return (_tcsicmp(x->_entry.path.c_str(), y->_entry.path.c_str())<0);
			}else{
				return (x->_originalSize < y->_originalSize);
			}
		case FILEINFO_TYPENAME:
			{
				int result = _tcsicmp(x->getExt().c_str(), y->getExt().c_str());
				if(result == 0){
					//sort by pathname
					return (_tcsicmp(x->_entry.path.c_str(), y->_entry.path.c_str())<0);
				}else{
					return (result < 0);
				}
			}
		case FILEINFO_FILETIME:
			{
				if(x->_entry.stat.st_mtime == y->_entry.stat.st_mtime){
					//sort by pathname
					return (_tcsicmp(x->_entry.path.c_str(), y->_entry.path.c_str())<0);
				}else{
					return (x->_entry.stat.st_mtime < y->_entry.stat.st_mtime);
				}
			}
		case FILEINFO_ATTRIBUTE:
			if(x->_entry.stat.st_mode == y->_entry.stat.st_mode){
				//sort by pathname
				return (_tcsicmp(x->_entry.path.c_str(), y->_entry.path.c_str())<0);
			}else{
				return (x->_entry.stat.st_mode < y->_entry.stat.st_mode);
			}
		case FILEINFO_COMPRESSEDSIZE:
		case FILEINFO_METHOD:
		case FILEINFO_RATIO:
		case FILEINFO_CRC:
#pragma message("FIXME!")
			return false;
		}
		return false;
	}
};


void CFileListModel::SortCurrentEntries()
{
	if(m_lpCurrentDir){
		FILEINFO_TYPE Type=(FILEINFO_TYPE)m_nSortKeyType;
		if (FILEINFO_INVALID != Type) {
			m_SortedChildren = m_lpCurrentDir->_children;

			if (Type<FILEINFO_INVALID || Type>FILEINFO_LAST_ITEM)return;
			FILELIST_SORT_COMPARATOR comp;
			comp.Type = Type;
			comp.bReversed = !m_bSortDescending;
			std::sort(m_SortedChildren.begin(), m_SortedChildren.end(), comp);
			dispatchEvent(WM_FILELIST_UPDATED);
		}
	}
}

bool CFileListModel::ExtractArchive(ILFProgressHandler &progressHandler)
{
	std::vector<std::filesystem::path> archiveList;
	archiveList.push_back(GetArchiveFileName());
	return GUI_extract_multiple_files(archiveList, progressHandler, nullptr);
}

bool CFileListModel::TestArchive(ILFProgressHandler &progressHandler)
{
	std::vector<std::filesystem::path> archiveList;
	archiveList.push_back(GetArchiveFileName());
	return GUI_test_multiple_files(archiveList, progressHandler, nullptr);
}
