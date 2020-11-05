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
#include "../ConfigCode/ConfigManager.h"
#include "../ConfigCode/ConfigFileListWindow.h"
#include "../ConfigCode/ConfigExtract.h"
#include "../Utilities/StringUtil.h"
#include "../Utilities/FileOperation.h"
#include "../CommonUtil.h"
#include "../Extract.h"
#include "FileListMessages.h"

CString CFileListModel::ms_strExtAccept;
CString CFileListModel::ms_strExtDeny;

CFileListModel::CFileListModel(CConfigManager &conf):
	mr_Config(conf),
	m_lpCurrentNode(NULL),
	m_bSortDescending(true),
	m_nSortKeyType(FILEINFO_INVALID),
	m_Mode(FILELIST_TREE)
{
}

CFileListModel::~CFileListModel()
{
}

HRESULT CFileListModel::OpenArchiveFile(LPCTSTR lpszArchive,FILELISTMODE flMode,CString &strErr,IArchiveContentUpdateHandler* lpHandler)
{
	CString strArchive=lpszArchive;	//次のClearでlpszArchveが破壊されるため、ここで保持
	Clear();
	m_Mode=flMode;

	if(!PathFileExists(strArchive)){	//存在しないならエラー
		strErr.Format(IDS_ERROR_FILE_NOT_FOUND,(LPCTSTR)strArchive);
		return E_LF_FILE_NOT_FOUND;
	}

	CConfigExtract ConfExtract;
	ConfExtract.load(mr_Config);
	//if (flMode == FILELIST_TREE) {
	try {
		//解析
		m_Content.inspectArchiveStruct((LPCTSTR)strArchive, lpHandler);
	}catch(const LF_EXCEPTION& e){
		strErr = e.what();
		return E_FAIL;	//TODO
	}

	m_lpCurrentNode = m_Content.getRootNode();
	dispatchEvent(WM_FILELIST_ARCHIVE_LOADED);

	//ソートしておく
	SortCurrentEntries();
	return S_OK;
}

HRESULT CFileListModel::ReopenArchiveFile(FILELISTMODE flMode,CString &strErr,IArchiveContentUpdateHandler* lpHandler)
{
	return OpenArchiveFile(GetArchiveFileName(),flMode,strErr,lpHandler);
}

void CFileListModel::Clear()
{
	m_Content.clear();
	m_lpCurrentNode=NULL;
	m_SortedChildren.clear();

	m_FoundItems.clear();
}

void CFileListModel::GetDirStack(std::stack<CString> &dirStack)
{
	//カレントディレクトリの保存
	if(m_lpCurrentNode){
		ARCHIVE_ENTRY_INFO* lpNode=m_lpCurrentNode;
		ARCHIVE_ENTRY_INFO* lpRoot=m_Content.getRootNode();
		for(;lpNode!=lpRoot;lpNode=lpNode->_parent){
			dirStack.push(lpNode->_entryName.c_str());
		}
	}
}

bool CFileListModel::SetDirStack(const std::stack<CString> &_dirStack)
{
	std::stack<CString> dirStack(_dirStack);
	//カレントディレクトリの復元
	while(!dirStack.empty()){
		TRACE(_T("SetDirStack:%s\n"),(LPCTSTR)dirStack.top());
		m_lpCurrentNode=m_lpCurrentNode->getChild((LPCTSTR)dirStack.top());
		dirStack.pop();
		if(!m_lpCurrentNode){
			m_lpCurrentNode=m_Content.getRootNode();
			dispatchEvent(WM_FILELIST_NEWCONTENT);
			SortCurrentEntries();
			return false;
		}
	}
	dispatchEvent(WM_FILELIST_NEWCONTENT);
	SortCurrentEntries();
	return true;
}

void CFileListModel::SetCurrentNode(ARCHIVE_ENTRY_INFO* lpN)
{
	ASSERT(lpN);
	m_lpCurrentNode=lpN;
	dispatchEvent(WM_FILELIST_NEWCONTENT);
	SortCurrentEntries();
}

//ディレクトリを掘り下げる
bool CFileListModel::MoveDownDir(ARCHIVE_ENTRY_INFO* lpNode)
{
	//階層構造を無視する場合には、この動作は無視される
	//if(Config.FileListWindow.FileListMode!=FILELIST_TREE)return false;

	if(!lpNode){
		return false;
	}
	if(!lpNode->isDirectory()){
		//ディレクトリでなければ帰ってもらう
		return false;
	}
	m_lpCurrentNode=lpNode;
	dispatchEvent(WM_FILELIST_NEWCONTENT);

	SortCurrentEntries();
	return true;
}

bool CFileListModel::MoveUpDir()
{
	TRACE(L"UpDir\n");
	ASSERT(m_lpCurrentNode);
	if(!m_lpCurrentNode)return false;

	ARCHIVE_ENTRY_INFO* lpNode=m_lpCurrentNode->_parent;
	if(!lpNode){
		return false;
	}
	TRACE(L"%s==>%s\n", m_lpCurrentNode->_entryName.c_str(), lpNode->_entryName.c_str());
	m_lpCurrentNode=lpNode;

	SortCurrentEntries();
	dispatchEvent(WM_FILELIST_NEWCONTENT);
	return true;
}




//リストビューでのアイテム番号に対応するファイルアイテムを取得
ARCHIVE_ENTRY_INFO* CFileListModel::GetFileListItemByIndex(long iIndex)
{
	ASSERT(m_lpCurrentNode);
	if(!m_lpCurrentNode)return NULL;

	size_t numChildren=m_lpCurrentNode->getNumChildren();

	//ASSERT(iIndex>=0 && numChildren>(unsigned)iIndex);
	if(iIndex<0 || numChildren<=(unsigned)iIndex)return NULL;

	if(FILEINFO_INVALID==m_nSortKeyType || m_SortedChildren.empty()){	//非ソート状態
		return m_lpCurrentNode->getChild(iIndex);
	}else{
		if(m_bSortDescending){
			return m_SortedChildren[iIndex].get();
		}else{
			return m_SortedChildren[numChildren-1-iIndex].get();
		}
	}
}

//------ソート状態
void CFileListModel::SetSortKeyType(int nSortKeyType)
{
	if(m_nSortKeyType!=nSortKeyType){
		m_nSortKeyType=nSortKeyType;
		SortCurrentEntries();
		dispatchEvent(WM_FILELIST_UPDATED);
	}
}

void CFileListModel::SetSortMode(bool bSortDescending)
{
	m_bSortDescending=bSortDescending;
	SortCurrentEntries();
	dispatchEvent(WM_FILELIST_UPDATED);
}

//比較関数オブジェクト
struct COMP{
	FILEINFO_TYPE Type;
	bool bReversed;
	bool operator()(const std::shared_ptr<ARCHIVE_ENTRY_INFO>& x, const std::shared_ptr<ARCHIVE_ENTRY_INFO>& y)const {
		return compare_no_reversed(x, y) ^ bReversed;
	}
	bool compare_no_reversed(const std::shared_ptr<ARCHIVE_ENTRY_INFO>& x, const std::shared_ptr<ARCHIVE_ENTRY_INFO>& y)const {
		switch(Type){
		case FILEINFO_FILENAME:
			{
				//ディレクトリが上に来るようにする
				if(x->isDirectory()){
					if(!y->isDirectory()){
						return true;
					}
				}else if(y->isDirectory()){
					return false;
				}
				int result = _tcsicmp(x->_entryName.c_str(), y->_entryName.c_str());
				if(result == 0){
					//同じならパス名でソート
					return (_tcsicmp(x->_fullpath.c_str(), y->_fullpath.c_str())<0);
				}else{
					return (result<0);
				}
			}
		case FILEINFO_FULLPATH:
			return (_tcsicmp(x->_fullpath.c_str(), y->_fullpath.c_str())<0);
		case FILEINFO_ORIGINALSIZE:
			if(x->_originalSize == y->_originalSize){
				//同じならパス名でソート
				return (_tcsicmp(x->_fullpath.c_str(), y->_fullpath.c_str())<0);
			}else{
				return (x->_originalSize < y->_originalSize);
			}
		case FILEINFO_TYPENAME:
			{
				int result = _tcsicmp(x->getExt().c_str(), y->getExt().c_str());
				if(result == 0){
					//同じならパス名でソート
					return (_tcsicmp(x->_fullpath.c_str(), y->_fullpath.c_str())<0);
				}else{
					return (result < 0);
				}
			}
		case FILEINFO_FILETIME:
			{
				if(x->_st_mtime == y->_st_mtime){
					//同じならパス名でソート
					return (_tcsicmp(x->_fullpath.c_str(), y->_fullpath.c_str())<0);
				}else{
					return (x->_st_mtime < y->_st_mtime);
				}
			}
		case FILEINFO_ATTRIBUTE:
			if(x->_nAttribute == y->_nAttribute){
				//同じならパス名でソート
				return (_tcsicmp(x->_fullpath.c_str(), y->_fullpath.c_str())<0);
			}else{
				return (x->_nAttribute < y->_nAttribute);
			}
		case FILEINFO_COMPRESSEDSIZE://圧縮後ファイルサイズ
		case FILEINFO_METHOD:			//圧縮メソッド
		case FILEINFO_RATIO:			//圧縮率
		case FILEINFO_CRC:			//CRC
			/*if(x->llCompressedSize == y->llCompressedSize){
				//同じならパス名でソート
				return (_tcsicmp(x->strFullPath , y->strFullPath)<0);
			}else{
				return (x->llCompressedSize < y->llCompressedSize);
			}*/
#pragma message("FIXME!")
			return false;
		}
		return false;
	}
};


//ソート
void CFileListModel::SortCurrentEntries()
{
	if(m_lpCurrentNode){
		FILEINFO_TYPE Type=(FILEINFO_TYPE)m_nSortKeyType;
		if(FILEINFO_INVALID==Type){		//ソート解除
			return;
		}
		m_SortedChildren = m_lpCurrentNode->_children;

		if(Type<FILEINFO_INVALID||Type>FILEINFO_LAST_ITEM)return;
		COMP Comp;
		Comp.Type=Type;
		Comp.bReversed = !m_bSortDescending;
		std::sort(m_SortedChildren.begin(),m_SortedChildren.end(),Comp);
		dispatchEvent(WM_FILELIST_UPDATED);
	}
}


//lpTop以下のファイルを検索;検索結果を格納したARCHIVE_ENTRY_INFOのポインタを返す
ARCHIVE_ENTRY_INFO* CFileListModel::FindItem(LPCTSTR lpszMask,ARCHIVE_ENTRY_INFO *lpTop)
{
	m_FoundItems._children = m_Content.findItem(lpszMask, lpTop);

	m_FoundItems._parent = nullptr;

	return &m_FoundItems;
}

void CFileListModel::EndFindItem()
{
	if(IsFindMode()){
		SetCurrentNode(m_FoundItems._parent);
	}
}

HRESULT CFileListModel::ExtractItems(HWND hWnd,bool bSameDir,const std::list<ARCHIVE_ENTRY_INFO*> &items,const ARCHIVE_ENTRY_INFO* lpBase,CString &strLog)
{
	CConfigExtract ConfExtract;
	ConfExtract.load(mr_Config);

	// 出力先の取得
	CString strErr;
	bool bTmp;
	CPath pathOutputBaseDir;
	OUTPUT_TO destType=ConfExtract.OutputDirType;
	if(bSameDir){
		destType=OUTPUT_TO_SAME_DIR;
	}
	HRESULT hr=GetOutputDirPathFromConfig(destType,GetArchiveFileName(),ConfExtract.OutputDirUserSpecified.c_str(),pathOutputBaseDir,bTmp,strLog);
	if(FAILED(hr)){
		return hr;
	}

	pathOutputBaseDir.AddBackslash();
	TRACE(_T("Default path from config:%s\n"),(LPCTSTR)pathOutputBaseDir);
	if(!bSameDir){	//出力先をダイアログで選ばせる
		LFShellFileOpenDialog dlg(pathOutputBaseDir, FOS_FORCEFILESYSTEM | FOS_FILEMUSTEXIST | FOS_PATHMUSTEXIST | FOS_PICKFOLDERS);
		if(IDOK!=dlg.DoModal()){
			return S_FALSE;//E_ABORT;
		}
		dlg.GetFilePath(pathOutputBaseDir);
		pathOutputBaseDir.AddBackslash();
	}

	//ウィンドウを使用不可に
	::EnableWindow(hWnd,FALSE);
	//解凍
	std::wstring log;
	std::vector<ARCHIVE_ENTRY_INFO*> itemsTmp(items.begin(), items.end());
	if(ExtractItems(itemsTmp,pathOutputBaseDir.operator LPCWSTR(),lpBase,IsFindMode(),log)){
		//ウィンドウを使用可能に
		strLog = log.c_str();
		::EnableWindow(hWnd,TRUE);
		return S_OK;
	}else{
		//ウィンドウを使用可能に
		strLog = log.c_str();
		::EnableWindow(hWnd,TRUE);
		return E_FAIL;
	}
}

HRESULT CFileListModel::AddItem(const std::vector<std::wstring> &fileList,LPCTSTR lpDestDir,CString &strLog)
{
	return m_Content.AddItem(fileList,lpDestDir,mr_Config,strLog);
}

bool CFileListModel::DeleteItems(const std::list<ARCHIVE_ENTRY_INFO*> &fileList,CString &strLog)
{
	return m_Content.DeleteItems(mr_Config,fileList,strLog);
}

//::Extract()を呼ぶ
bool CFileListModel::ExtractArchive()
{
	std::vector<std::wstring> archiveList;
	archiveList.push_back(GetArchiveFileName());
	return GUI_extract_multiple_files(archiveList, NULL);
}

bool CFileListModel::TestArchive()
{
	//検査
	std::vector<std::wstring> archiveList;
	archiveList.push_back(GetArchiveFileName());
	return GUI_test_multiple_files(archiveList, NULL);
}

void CFileListModel::ClearTempDir()
{
	UtilDeleteDir(m_TempDirManager.path(), false);
}
