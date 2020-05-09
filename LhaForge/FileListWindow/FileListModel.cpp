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
#include "../TestArchive.h"
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
	HRESULT hr;
	//解析
	if(flMode==FILELIST_TREE){
		CConfigFileListWindow ConfFLW;
		ConfFLW.load(mr_Config);
		hr=m_Content.ConstructTree(strArchive,mr_Config,ConfExtract.DenyExt,BOOL2bool(ConfFLW.IgnoreMeaninglessPath),strErr,lpHandler);
	}else{
		hr=m_Content.ConstructFlat(strArchive,mr_Config,ConfExtract.DenyExt,(flMode==FILELIST_FLAT_FILESONLY),strErr,lpHandler);
	}

	if(FAILED(hr)){
		m_Content.SetArchiveFileName(strArchive);
		return hr;
	}

	m_lpCurrentNode=m_Content.GetRootNode();
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
	m_Content.Clear();
	m_lpCurrentNode=NULL;
	m_SortedChildren.clear();

	m_FoundItems.Clear();
}

void CFileListModel::GetDirStack(std::stack<CString> &dirStack)
{
	//カレントディレクトリの保存
	if(m_lpCurrentNode){
		ARCHIVE_ENTRY_INFO_TREE* lpNode=m_lpCurrentNode;
		ARCHIVE_ENTRY_INFO_TREE* lpRoot=m_Content.GetRootNode();
		for(;lpNode!=lpRoot;lpNode=lpNode->lpParent){
			dirStack.push(lpNode->strTitle);
		}
	}
}

bool CFileListModel::SetDirStack(const std::stack<CString> &_dirStack)
{
	std::stack<CString> dirStack(_dirStack);
	//カレントディレクトリの復元
	while(!dirStack.empty()){
		TRACE(_T("SetDirStack:%s\n"),(LPCTSTR)dirStack.top());
		m_lpCurrentNode=m_lpCurrentNode->GetChild(dirStack.top());
		dirStack.pop();
		if(!m_lpCurrentNode){
			m_lpCurrentNode=m_Content.GetRootNode();
			dispatchEvent(WM_FILELIST_NEWCONTENT);
			SortCurrentEntries();
			return false;
		}
	}
	dispatchEvent(WM_FILELIST_NEWCONTENT);
	SortCurrentEntries();
	return true;
}

void CFileListModel::SetCurrentNode(ARCHIVE_ENTRY_INFO_TREE* lpN)
{
	ASSERT(lpN);
	m_lpCurrentNode=lpN;
	dispatchEvent(WM_FILELIST_NEWCONTENT);
	SortCurrentEntries();
}

//ディレクトリを掘り下げる
bool CFileListModel::MoveDownDir(ARCHIVE_ENTRY_INFO_TREE* lpNode)
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
	TRACE(_T("UpDir\n"));
	ASSERT(m_lpCurrentNode);
	if(!m_lpCurrentNode)return false;

	ARCHIVE_ENTRY_INFO_TREE* lpNode=m_lpCurrentNode->lpParent;
	if(!lpNode){
		return false;
	}
	TRACE(_T("%s==>%s\n"),(LPCTSTR)m_lpCurrentNode->strTitle,(LPCTSTR)lpNode->strTitle);
	m_lpCurrentNode=lpNode;

	SortCurrentEntries();
	dispatchEvent(WM_FILELIST_NEWCONTENT);
	return true;
}




//リストビューでのアイテム番号に対応するファイルアイテムを取得
ARCHIVE_ENTRY_INFO_TREE* CFileListModel::GetFileListItemByIndex(long iIndex)
{
	ASSERT(m_lpCurrentNode);
	if(!m_lpCurrentNode)return NULL;

	size_t numChildren=m_lpCurrentNode->GetNumChildren();

	//ASSERT(iIndex>=0 && numChildren>(unsigned)iIndex);
	if(iIndex<0 || numChildren<=(unsigned)iIndex)return NULL;

	if(FILEINFO_INVALID==m_nSortKeyType || m_SortedChildren.empty()){	//非ソート状態
		return m_lpCurrentNode->GetChild(iIndex);
	}else{
		if(m_bSortDescending){
			return m_SortedChildren[iIndex];
		}else{
			return m_SortedChildren[numChildren-1-iIndex];
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
	bool operator()(const ARCHIVE_ENTRY_INFO_TREE* x, const ARCHIVE_ENTRY_INFO_TREE* y)const {
		return compare_no_reversed(x, y) ^ bReversed;
	}
	bool compare_no_reversed(const ARCHIVE_ENTRY_INFO_TREE* x, const ARCHIVE_ENTRY_INFO_TREE* y)const {
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
				int result = _tcsicmp(x->strTitle , y->strTitle);
				if(result == 0){
					//同じならパス名でソート
					return (_tcsicmp(x->strFullPath , y->strFullPath)<0);
				}else{
					return (result<0);
				}
			}
		case FILEINFO_FULLPATH:
			return (_tcsicmp(x->strFullPath , y->strFullPath)<0);
		case FILEINFO_ORIGINALSIZE:
			if(x->llOriginalSize == y->llOriginalSize){
				//同じならパス名でソート
				return (_tcsicmp(x->strFullPath , y->strFullPath)<0);
			}else{
				return (x->llOriginalSize < y->llOriginalSize);
			}
		case FILEINFO_TYPENAME:
			{
				int result = _tcsicmp(x->getExt() , y->getExt());
				if(result == 0){
					//同じならパス名でソート
					return (_tcsicmp(x->strFullPath , y->strFullPath)<0);
				}else{
					return (result < 0);
				}
			}
		case FILEINFO_FILETIME:
			{
				if(x->st_mtime == y->st_mtime){
					//同じならパス名でソート
					return (_tcsicmp(x->strFullPath , y->strFullPath)<0);
				}else{
					return (x->st_mtime < y->st_mtime);
				}
			}
		case FILEINFO_ATTRIBUTE:
			if(x->nAttribute == y->nAttribute){
				//同じならパス名でソート
				return (_tcsicmp(x->strFullPath , y->strFullPath)<0);
			}else{
				return (x->nAttribute < y->nAttribute);
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
		m_SortedChildren=m_lpCurrentNode->childrenArray;

		if(Type<FILEINFO_INVALID||Type>FILEINFO_LAST_ITEM)return;
		COMP Comp;
		Comp.Type=Type;
		Comp.bReversed = !m_bSortDescending;
		std::sort(m_SortedChildren.begin(),m_SortedChildren.end(),Comp);
		dispatchEvent(WM_FILELIST_UPDATED);
	}
}


//lpTop以下のファイルを検索;検索結果を格納したARCHIVE_ENTRY_INFO_TREEのポインタを返す
ARCHIVE_ENTRY_INFO_TREE* CFileListModel::FindItem(LPCTSTR lpszMask,ARCHIVE_ENTRY_INFO_TREE *lpTop)
{
	m_FoundItems.childrenArray.clear();

	m_Content.FindItem(lpszMask,lpTop,m_FoundItems.childrenArray);

	m_FoundItems.lpParent=lpTop;
	m_FoundItems.childrenDict.clear();

	for(size_t i=0;i<m_FoundItems.childrenArray.size();i++){
		ARCHIVE_ENTRY_INFO_TREE* lpNode=m_FoundItems.childrenArray[i];
		m_FoundItems.childrenDict[(LPCTSTR)lpNode->strTitle]=lpNode;
	}

	return &m_FoundItems;
}

void CFileListModel::EndFindItem()
{
	if(IsFindMode()){
		SetCurrentNode(m_FoundItems.lpParent);
	}
}

bool CFileListModel::ExtractItems(const std::list<ARCHIVE_ENTRY_INFO_TREE*> &items,LPCTSTR lpszDir,const ARCHIVE_ENTRY_INFO_TREE* lpBase,bool bCollapseDir,CString &strLog)
{
	return m_Content.ExtractItems(mr_Config,items,lpszDir,lpBase,bCollapseDir,strLog);
}

HRESULT CFileListModel::ExtractItems(HWND hWnd,bool bSameDir,const std::list<ARCHIVE_ENTRY_INFO_TREE*> &items,const ARCHIVE_ENTRY_INFO_TREE* lpBase,CString &strLog)
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
	HRESULT hr=GetOutputDirPathFromConfig(destType,GetArchiveFileName(),ConfExtract.OutputDirUserSpecified,pathOutputBaseDir,bTmp,strLog);
	if(FAILED(hr)){
		return hr;
	}

	pathOutputBaseDir.AddBackslash();
	TRACE(_T("Default path from config:%s\n"),(LPCTSTR)pathOutputBaseDir);
	if(!bSameDir){	//出力先をダイアログで選ばせる
		CString title(MAKEINTRESOURCE(IDS_INPUT_TARGET_FOLDER));
		CFolderDialog dlg(hWnd,title,BIF_RETURNONLYFSDIRS|BIF_NEWDIALOGSTYLE);
		dlg.SetInitialFolder(pathOutputBaseDir);
		if(IDOK!=dlg.DoModal()){
			return S_FALSE;//E_ABORT;
		}
		pathOutputBaseDir=dlg.GetFolderPath();
		pathOutputBaseDir.AddBackslash();
	}

	//ウィンドウを使用不可に
	::EnableWindow(hWnd,FALSE);
	//解凍
	if(ExtractItems(items,pathOutputBaseDir,lpBase,IsFindMode(),strLog)){
		//ウィンドウを使用可能に
		::EnableWindow(hWnd,TRUE);
		return S_OK;
	}else{
		//ウィンドウを使用可能に
		::EnableWindow(hWnd,TRUE);
		return E_FAIL;
	}
}

bool CFileListModel::MakeSureItemsExtracted(LPCTSTR lpOutputDir,const ARCHIVE_ENTRY_INFO_TREE* lpBase,const std::list<ARCHIVE_ENTRY_INFO_TREE*> &items,std::list<CString> &r_filesList,bool bOverwrite,CString &strLog)
{
	if(lpOutputDir){
		return m_Content.MakeSureItemsExtracted(mr_Config,lpOutputDir,lpBase,items,r_filesList,bOverwrite,strLog);
	}else{
		return m_Content.MakeSureItemsExtracted(mr_Config,m_TempDirManager.path(),lpBase,items,r_filesList,bOverwrite,strLog);
	}
}

HRESULT CFileListModel::AddItem(const std::list<CString> &fileList,LPCTSTR lpDestDir,CString &strLog)
{
	return m_Content.AddItem(fileList,lpDestDir,mr_Config,strLog);
}

bool CFileListModel::DeleteItems(const std::list<ARCHIVE_ENTRY_INFO_TREE*> &fileList,CString &strLog)
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

void CFileListModel::TestArchive()
{
	//検査
	std::list<CString> archiveList;
	archiveList.push_back(GetArchiveFileName());
	::TestArchive(archiveList,mr_Config);
}

void CFileListModel::ClearTempDir()
{
	UtilDeleteDir(m_TempDirManager.path(), false);
}
