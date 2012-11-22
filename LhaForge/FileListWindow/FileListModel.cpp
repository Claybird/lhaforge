/*
 * Copyright (c) 2005-2012, Claybird
 * All rights reserved.

 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:

 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Claybird nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.

 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
 * THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 */

#include "stdafx.h"
#include "../ArchiverManager.h"
#include "FileListModel.h"
#include "../ConfigCode/ConfigManager.h"
#include "../ConfigCode/ConfigFileListWindow.h"
#include "../ConfigCode/ConfigExtract.h"
#include "../Utilities/StringUtil.h"
#include "../Utilities/FileOperation.h"
#include "../CommonUtil.h"
#include "../Extract.h"
#include "../TestArchive.h"
#include "../Dialogs/LFFolderDialog.h"
#include "FileListMessages.h"

CString CFileListModel::ms_strExtAccept;
CString CFileListModel::ms_strExtDeny;

CFileListModel::CFileListModel(CConfigManager &conf):
	mr_Config(conf),
	m_lpCurrentNode(NULL),
	m_bSortDescending(true),
	m_nSortKeyType(FILEINFO_INVALID),
	m_TempDirManager(_T("lhaf")),
	m_Mode(FILELIST_TREE),
	m_idForceDLL(DLL_ID_UNKNOWN)
{
}

CFileListModel::~CFileListModel()
{
}

HRESULT CFileListModel::OpenArchiveFile(LPCTSTR lpszArchive,DLL_ID idForceDLL,FILELISTMODE flMode,CString &strErr,IArchiveContentUpdateHandler* lpHandler)
{
	CString strArchive=lpszArchive;	//次のClearでlpszArchveが破壊されるため、ここで保持
	Clear();
	m_Mode=flMode;
	m_idForceDLL=idForceDLL;

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
		hr=m_Content.ConstructTree(strArchive,mr_Config,idForceDLL,ConfExtract.DenyExt,BOOL2bool(ConfFLW.IgnoreMeaninglessPath),strErr,lpHandler);
	}else{
		hr=m_Content.ConstructFlat(strArchive,mr_Config,idForceDLL,ConfExtract.DenyExt,(flMode==FILELIST_FLAT_FILESONLY),strErr,lpHandler);
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
	return OpenArchiveFile(GetArchiveFileName(),m_idForceDLL,flMode,strErr,lpHandler);
}

void CFileListModel::Clear()
{
	m_Content.Clear();
	m_lpCurrentNode=NULL;
	m_SortedChildren.clear();
	//m_nSortKeyType=FILEINFO_INVALID;

	m_FoundItems.Clear();
	m_FoundItems.bDir=true;
	m_FoundItems.bSafe=true;

	//dispatchEvent(WM_FILELIST_NEWCONTENT);
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
	if(!lpNode->bDir){
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
	dispatchEvent(WM_FILELIST_UPDATED);
}

//比較関数オブジェクト
struct COMP{
	FILEINFO_TYPE Type;
	bool operator()(ARCHIVE_ENTRY_INFO_TREE* x, ARCHIVE_ENTRY_INFO_TREE* y)const{
		switch(Type){
		case FILEINFO_FILENAME:
			//ディレクトリが上に来るようにする
			if(x->bDir){
				if(!y->bDir){
					return true;
				}
			}else if(y->bDir){
				return false;
			}
			return (_tcsicmp(x->strTitle , y->strTitle)<0);
		case FILEINFO_FULLPATH:
			return (_tcsicmp(x->strFullPath , y->strFullPath)<0);
		case FILEINFO_ORIGINALSIZE:
			return (x->llOriginalSize.QuadPart < y->llOriginalSize.QuadPart);
		case FILEINFO_TYPENAME:
			return (_tcsicmp(x->strExt , y->strExt)<0);
		case FILEINFO_FILETIME:
			return (CompareFileTime(&x->cFileTime , &y->cFileTime)<0);
		case FILEINFO_ATTRIBUTE:
			return (x->nAttribute < y->nAttribute);
		case FILEINFO_COMPRESSEDSIZE://圧縮後ファイルサイズ
			return (x->llCompressedSize.QuadPart < y->llCompressedSize.QuadPart);
		case FILEINFO_METHOD:			//圧縮メソッド
			return (_tcsicmp(x->strMethod , y->strMethod)<0);
		case FILEINFO_RATIO:			//圧縮率
			return (x->wRatio < y->wRatio);
		case FILEINFO_CRC:			//CRC
			return (x->dwCRC < y->dwCRC);
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

bool CFileListModel::ReloadArchiverIfLost(CString &strErr)
{
	return m_Content.ReloadArchiverIfLost(mr_Config,strErr);
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
	HRESULT hr=GetOutputDirPathFromConfig(destType,GetArchiveFileName(),ConfExtract.OutputDir,pathOutputBaseDir,bTmp,strLog);
	if(FAILED(hr)){
		return hr;
	}

	pathOutputBaseDir.AddBackslash();
	TRACE(_T("Default path from config:%s\n"),(LPCTSTR)pathOutputBaseDir);
	if(!bSameDir){	//出力先をダイアログで選ばせる
		CString title(MAKEINTRESOURCE(IDS_INPUT_TARGET_FOLDER));
		CLFFolderDialog dlg(hWnd,title,BIF_RETURNONLYFSDIRS|BIF_NEWDIALOGSTYLE);
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
		return m_Content.MakeSureItemsExtracted(mr_Config,m_TempDirManager.GetDirPath(),lpBase,items,r_filesList,bOverwrite,strLog);
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
	//解凍
	std::list<CString> archiveList;
	archiveList.push_back(GetArchiveFileName());
	return ::Extract(archiveList,mr_Config,m_idForceDLL,NULL);
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
	m_TempDirManager.ClearSubDir();
}
