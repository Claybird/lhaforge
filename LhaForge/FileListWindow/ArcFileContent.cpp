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
#include "ArcFileContent.h"
#include "../Utilities/StringUtil.h"
#include "../Utilities/FileOperation.h"
#include "../ArchiverCode/arc_interface.h"
#include "../ArchiverManager.h"

CArchiveFileContent::CArchiveFileContent():
m_bReadOnly(false),
m_bEncrypted(false)
{
	Clear();
}

CArchiveFileContent::~CArchiveFileContent()
{
	Clear();
}

void CArchiveFileContent::Clear()
{
	m_Root.Clear();
	m_Root.lpParent=NULL;

	m_bReadOnly=false;
	m_GC.DeleteAll();

	m_lpArchiver=NULL;
	m_pathArcFileName.Empty();
	m_bExtractEachSupported=false;
}

ARCHIVE_ENTRY_INFO_TREE* CArchiveFileContent::ForceFindEntry(ARCHIVE_ENTRY_INFO_TREE* lpParent,LPCTSTR lpName)
{
	ASSERT(lpParent);
	ARCHIVE_ENTRY_INFO_TREE::DICT::iterator ite=lpParent->childrenDict.find(lpName);
	if(lpParent->childrenDict.end()==ite){
		ARCHIVE_ENTRY_INFO_TREE* lpTree=new ARCHIVE_ENTRY_INFO_TREE;
		m_GC.Add(lpTree);
		lpTree->Clear();

		lpTree->lpParent=lpParent;
		lpParent->childrenDict.insert(ARCHIVE_ENTRY_INFO_TREE::DICT::value_type(lpName,lpTree));
		lpParent->childrenArray.push_back(lpTree);
		return lpTree;
	}else{
		return (*ite).second;
	}
}

HRESULT CArchiveFileContent::InspectArchiveStruct(LPCTSTR lpFile,CConfigManager &ConfMan,CArchiverDLL *lpArchiver,std::vector<ARCHIVE_ENTRY_INFO> &entries,IArchiveContentUpdateHandler* lpHandler)
{
	//解析開始
	TRACE(_T("ディレクトリ構造解析開始\n"));
	ARCHIVE_FILE arc;
	if(!lpArchiver->InspectArchiveBegin(arc,lpFile,ConfMan)){
		//スキャンできない
		return E_LF_FILELIST_NOT_SUPPORTED;
	}

	//少なくとも一つのファイルが暗号化されているならtrue
	bool bEncrypted = false;

	HRESULT hr=S_OK;
	//一覧取得
	while(lpArchiver->InspectArchiveNext(arc)){
		ARCHIVE_ENTRY_INFO item;

		//格納ファイル名取得
		lpArchiver->InspectArchiveGetFileName(arc,item.strFullPath);
		//属性取得
		item.nAttribute=lpArchiver->InspectArchiveGetAttribute(arc);
		//ファイルサイズ(圧縮前)
		if(!lpArchiver->InspectArchiveGetOriginalFileSize(arc,item.llOriginalSize)){
			item.llOriginalSize.LowPart=-1;
			item.llOriginalSize.HighPart=-1;
		}
		//ファイルサイズ(圧縮後)
		item.llCompressedSize.LowPart=-1;
		item.llCompressedSize.HighPart=-1;
		//日時取得
		if(!lpArchiver->InspectArchiveGetWriteTime(arc,item.cFileTime)){
			item.cFileTime.dwLowDateTime=-1;
			item.cFileTime.dwHighDateTime=-1;
		}
		//圧縮率
		item.wRatio=-1;
		//CRC
		item.dwCRC=-1;
		//メソッド
		item.strMethod=L"---";

		//暗号
		bEncrypted = bEncrypted || ((item.nAttribute & FA_ENCRYPTED)!=0);

		//登録		
		entries.push_back(item);

		//更新
		if(lpHandler){
			while(UtilDoMessageLoop())continue;
			lpHandler->onUpdated(item);
			if(lpHandler->isAborted()){
				hr=E_ABORT;
				break;
			}
		}
	}
	//解析終了
	lpArchiver->InspectArchiveEnd(arc);

	m_bEncrypted = bEncrypted;
	return hr;
}


HRESULT CArchiveFileContent::ConstructFlat(LPCTSTR lpFile,CConfigManager &ConfMan,DLL_ID idForce,LPCTSTR lpDenyExt,bool bFilesOnly,CString &strErr,IArchiveContentUpdateHandler* lpHandler)
{
	Clear();

	m_bReadOnly = GetFileAttributes(lpFile) & FILE_ATTRIBUTE_READONLY;

	CArchiverDLL* lpArchiver=CArchiverDLLManager::GetInstance().GetArchiver(lpFile,lpDenyExt,idForce);
	if(!lpArchiver){
		//不明な形式 or 非対応DLLでUNICODEファイル名を扱おうとした
		strErr.Format(IDS_FILELIST_FORMAT_UNKNOWN,lpFile);
		return E_LF_UNKNOWN_FORMAT;
	}
	if(!lpArchiver->QueryInspectSupported()){
		//閲覧できない形式
		strErr.Format(IDS_FILELIST_FORMAT_NOTSUPPORTED,lpFile);
		return E_LF_FILELIST_NOT_SUPPORTED;
	}

	//--構造取得
	std::vector<ARCHIVE_ENTRY_INFO> entries;
	HRESULT hr=InspectArchiveStruct(lpFile,ConfMan,lpArchiver,entries,lpHandler);
	if(FAILED(hr)){
		ASSERT(hr==E_ABORT);
		if(hr==E_ABORT){
			strErr.Format(IDS_ERROR_USERCANCEL);
		}
		return hr;
	}
	//記録
	m_lpArchiver=lpArchiver;
	m_pathArcFileName=lpFile;
	m_bExtractEachSupported=m_lpArchiver->QueryExtractSpecifiedOnlySupported(m_pathArcFileName);

	//解析
	size_t numEntries=entries.size();
	for(size_t i=0;i<numEntries;i++){
		if(bFilesOnly && entries[i].nAttribute&FA_DIREC)continue;	//ファイルのみの場合はディレクトリは無視

		ARCHIVE_ENTRY_INFO_TREE* lpEntry=new ARCHIVE_ENTRY_INFO_TREE;
		m_GC.Add(lpEntry);
		lpEntry->Clear();

		//エントリ設定
		*((ARCHIVE_ENTRY_INFO*)lpEntry)=entries[i];
		lpEntry->lpParent=&m_Root;
		UtilPathGetLastSection(lpEntry->strTitle,entries[i].strFullPath);

		m_Root.childrenDict.insert(ARCHIVE_ENTRY_INFO_TREE::DICT::value_type((LPCTSTR)lpEntry->strTitle,lpEntry));
		m_Root.childrenArray.push_back(lpEntry);

		//後で設定する
		lpEntry->bDir=false;
		lpEntry->bSafe=true;
	}

	PostProcess(m_lpArchiver->IsUnicodeCapable(),NULL);
	return S_OK;
}

HRESULT CArchiveFileContent::ConstructTree(LPCTSTR lpFile,CConfigManager &ConfMan,DLL_ID idForce,LPCTSTR lpDenyExt,bool bSkipMeaningless,CString &strErr,IArchiveContentUpdateHandler* lpHandler)
{
	Clear();

	m_bReadOnly = GetFileAttributes(lpFile) & FILE_ATTRIBUTE_READONLY;

	CArchiverDLL* lpArchiver=CArchiverDLLManager::GetInstance().GetArchiver(lpFile,lpDenyExt,idForce);
	if(!lpArchiver){
		//不明な形式 or 非対応DLLでUNICODEファイル名を扱おうとした
		strErr.Format(IDS_FILELIST_FORMAT_UNKNOWN,lpFile);
		//UNICODE関係のチェック
		if(!UtilCheckT2A(lpFile)){
			//UNICODEに対応していないのにUNICODEファイル名のファイルを扱おうとした可能性がある
			strErr+=_T("\r\n\r\n");
			strErr.AppendFormat(IDS_ERROR_UNICODEPATH);
		}
		return E_LF_UNKNOWN_FORMAT;
	}
	if(!lpArchiver->QueryInspectSupported()){
		//閲覧できない形式
		strErr.Format(IDS_FILELIST_FORMAT_NOTSUPPORTED,lpFile);
		return E_LF_FILELIST_NOT_SUPPORTED;
	}

	//--構造取得
	std::vector<ARCHIVE_ENTRY_INFO> entries;
	HRESULT hr=InspectArchiveStruct(lpFile,ConfMan,lpArchiver,entries,lpHandler);
	if(FAILED(hr)){
		ASSERT(hr==E_ABORT && "Not Implemented");
		if(hr==E_ABORT){
			strErr.Format(IDS_ERROR_USERCANCEL);
		}
		return hr;
	}

	//記録
	m_lpArchiver=lpArchiver;
	m_pathArcFileName=lpFile;
	m_bExtractEachSupported=m_lpArchiver->QueryExtractSpecifiedOnlySupported(m_pathArcFileName);

	//解析
	//TODO:既に出現したディレクトリのみを対象に比較を行う
	size_t numEntries=entries.size();
	for(size_t i=0;i<numEntries;i++){
		ARCHIVE_ENTRY_INFO_TREE* lpEntry=&m_Root;
		LPCTSTR lpPath=entries[i].strFullPath;

		CString strEntry;
		CString strTitle;
		bool bBreak=false;
		for(;*lpPath!=L'\0';){
			LPCTSTR lpStart,lpEnd;
			if(UtilPathNextSection(lpPath,lpStart,lpEnd,bSkipMeaningless)){
				UtilAssignSubString(strEntry,lpStart,lpEnd);
				lpPath=lpEnd;
				if(*lpPath!=L'\0')lpPath++;
			}else{
				//分割不能
				strEntry=lpPath;
				bBreak=true;
			}
			strTitle=strEntry;
			//小文字化
			strEntry.MakeLower();

			//子エントリ検索
			lpEntry=ForceFindEntry(lpEntry,strEntry);
			if(bBreak){
				break;
			}else{
				if(lpEntry->strTitle.IsEmpty()){
					//仮想ディレクトリの設定
					lpEntry->strTitle=strTitle;
				}
			}
		}
		ASSERT(lpEntry);
		//エントリ設定
		*((ARCHIVE_ENTRY_INFO*)lpEntry)=entries[i];

		lpEntry->strTitle=strTitle;

		//後で設定する
		lpEntry->bDir=false;
		lpEntry->bSafe=true;
	}

	PostProcess(m_lpArchiver->IsUnicodeCapable(),NULL);
	return S_OK;
}

/*
 * ファイル一覧追加後の処理
 * ディレクトリ内ファイルのサイズを取得するなど
 * bUnicodeは使用したDLLがUNICODEに対応している場合にはtrue
 */
void CArchiveFileContent::PostProcess(bool bUnicode,ARCHIVE_ENTRY_INFO_TREE* pNode)
{
	if(!pNode)pNode=&m_Root;
	//属性
	if(-1==pNode->nAttribute){	//まったく分かっていない場合
		if(!pNode->childrenDict.empty()||UtilPathEndWithSeparator(pNode->strFullPath)){
			//以下の条件のいずれかに合致すればディレクトリ
			//・ノード名末尾にパス区切り文字が付いている(bDir)
			//・子ノードが空でない
			pNode->nAttribute=FA_DIREC;
			pNode->bDir=true;
		}else{
			pNode->nAttribute=FA_UNKNOWN;
		}
	}else{
		if(!pNode->childrenDict.empty()||UtilPathEndWithSeparator(pNode->strFullPath)){
			pNode->nAttribute|=FA_DIREC;
			pNode->bDir=true;
		}
	}
	if(pNode->nAttribute&FA_DIREC){
		//フォルダには拡張子はない
		pNode->strExt=FOLDER_EXTENSION_STRING;

		pNode->llOriginalSize.QuadPart=0;
		pNode->llCompressedSize.QuadPart=0;
	}else{
		pNode->strExt=PathFindExtension(pNode->strFullPath);
	}

	//危険判定
	if(bUnicode){
		pNode->bSafe=UtilIsSafeUnicode(pNode->strFullPath);
	}

	//子ノードにも適用
	size_t numChildren=pNode->childrenArray.size();
	for(size_t i=0;i<numChildren;i++){
		ARCHIVE_ENTRY_INFO_TREE* pChild=pNode->childrenArray[i];
		PostProcess(bUnicode,pChild);

		//---ディレクトリなら、中のデータのサイズを集計する
		if(pNode->bDir){
			//---圧縮前サイズ
			if(pNode->llOriginalSize.QuadPart>=0){	//ファイルサイズ取得に失敗していない
				if(pChild->llOriginalSize.QuadPart>=0){
					pNode->llOriginalSize.QuadPart+=pChild->llOriginalSize.QuadPart;
				}else{
					pNode->llOriginalSize.LowPart=-1;
					pNode->llOriginalSize.HighPart=-1;
				}
			}
			//---圧縮後サイズ
			if(pNode->llCompressedSize.QuadPart>=0){	//ファイルサイズ取得に失敗していない
				if(pChild->llCompressedSize.QuadPart>=0){
					pNode->llCompressedSize.QuadPart+=pChild->llCompressedSize.QuadPart;
				}else{
					pNode->llCompressedSize.LowPart=-1;
					pNode->llCompressedSize.HighPart=-1;
				}
			}
		}
	}
}



void CArchiveFileContent::FindSubItem(LPCTSTR lpszMask,bool bMatchPath,const ARCHIVE_ENTRY_INFO_TREE *lpTop,std::vector<ARCHIVE_ENTRY_INFO_TREE*> &founds)const
{
	//幅優先探索
	std::vector<ARCHIVE_ENTRY_INFO_TREE*> dirs;
	for(size_t i=0;i<lpTop->childrenArray.size();i++){
		ARCHIVE_ENTRY_INFO_TREE* lpNode=lpTop->childrenArray[i];
		ASSERT(lpNode);
		CString strKey;
		if(bMatchPath){	//パス名も一致
			strKey=lpNode->strFullPath;
			strKey.Replace(_T("/"),_T("\\"));
		}else{
			strKey=lpNode->strTitle;
		}
		if(::PathMatchSpec(strKey,lpszMask)){
			founds.push_back(lpNode);
		}
		if(lpNode->bDir){	//ディレクトリは再帰検索
			dirs.push_back(lpNode);
		}
	}
	//ディレクトリ
	for(size_t i=0;i<dirs.size();i++){
		FindSubItem(lpszMask,bMatchPath,dirs[i],founds);
	}
}


void CArchiveFileContent::FindItem(LPCTSTR lpszMask,const ARCHIVE_ENTRY_INFO_TREE *lpTop,std::vector<ARCHIVE_ENTRY_INFO_TREE*> &founds)const
{
	founds.clear();
	ASSERT(lpTop);
	if(!lpTop)return;

	CString strMask(lpszMask);
	strMask.Replace(_T("/"),_T("\\"));
	bool bMatchPath=(-1!=strMask.Find(_T('\\')));

	//*も?も付いていない場合は*を検索条件に追加
	if(-1==strMask.FindOneOf(_T("*?"))){
		strMask.Insert(0,_T("*"));
		strMask+=_T("*");
	}

	FindSubItem(strMask,bMatchPath,lpTop,founds);
}


bool CArchiveFileContent::ExtractItems(CConfigManager &ConfMan,const std::list<ARCHIVE_ENTRY_INFO_TREE*> &items,LPCTSTR lpszDir,const ARCHIVE_ENTRY_INFO_TREE* lpBase,bool bCollapseDir,CString &strLog)
{
	if(!IsExtractEachSupported()){
		//選択ファイルの解凍はサポートされていない
		return false;
	}

	return m_lpArchiver->ExtractItems(m_pathArcFileName,ConfMan,lpBase,items,lpszDir,bCollapseDir,strLog);
}

void CArchiveFileContent::CollectUnextractedFiles(LPCTSTR lpOutputDir,const ARCHIVE_ENTRY_INFO_TREE* lpBase,const ARCHIVE_ENTRY_INFO_TREE* lpParent,std::map<const ARCHIVE_ENTRY_INFO_TREE*,std::list<ARCHIVE_ENTRY_INFO_TREE*> > &toExtractList)
{
	size_t numChildren=lpParent->GetNumChildren();
	for(size_t i=0;i<numChildren;i++){
		ARCHIVE_ENTRY_INFO_TREE* lpNode=lpParent->GetChild(i);
		CPath path=lpOutputDir;

		CString strItem;
		ArcEntryInfoTree_GetNodePathRelative(lpNode,lpBase,strItem);
		path.Append(strItem);

		if(::PathIsDirectory(path)){
			// フォルダが存在するが中身はそろっているか?
			CollectUnextractedFiles(lpOutputDir,lpBase,lpNode,toExtractList);
		}else if(!::PathFileExists(path)){
			// キャッシュが存在しないので、解凍要請リストに加える
			toExtractList[lpParent].push_back(lpNode);
		}
	}
}


//bOverwrite:trueなら存在するテンポラリファイルを削除してから解凍する
bool CArchiveFileContent::MakeSureItemsExtracted(CConfigManager &ConfMan,LPCTSTR lpOutputDir,const ARCHIVE_ENTRY_INFO_TREE* lpBase,const std::list<ARCHIVE_ENTRY_INFO_TREE*> &items,std::list<CString> &r_filesList,bool bOverwrite,CString &strLog)
{
	//選択されたアイテムを列挙
	std::map<const ARCHIVE_ENTRY_INFO_TREE*,std::list<ARCHIVE_ENTRY_INFO_TREE*> > toExtractList;

	std::list<CString> newFilesList;	//これから解凍するファイルのディスク上のパス名

	for(std::list<ARCHIVE_ENTRY_INFO_TREE*>::const_iterator ite=items.begin();ite!=items.end();++ite){
		// 存在をチェックし、もし解凍済みであればそれを開く
		ARCHIVE_ENTRY_INFO_TREE* lpNode=*ite;
		CPath path=lpOutputDir;

		CString strItem;
		ArcEntryInfoTree_GetNodePathRelative(lpNode,lpBase,strItem);
		path.Append(strItem);

		if(bOverwrite){
			// 上書き解凍するので、存在するファイルは削除
			if(lpNode->bDir){
				if(::PathIsDirectory(path))UtilDeleteDir(path,true);
			}else{
				if(::PathFileExists(path))UtilDeletePath(path);
			}
			//解凍要請リストに加える
			toExtractList[lpBase].push_back(lpNode);
			newFilesList.push_back(path);
		}else{	//上書きはしない
			if(::PathIsDirectory(path)){
				// フォルダが存在するが中身はそろっているか?
				CollectUnextractedFiles(lpOutputDir,lpBase,lpNode,toExtractList);
			}else if(!::PathFileExists(path)){
				// キャッシュが存在しないので、解凍要請リストに加える
				toExtractList[lpBase].push_back(lpNode);
				newFilesList.push_back(path);
			}
		}
		path.RemoveBackslash();
		//開く予定リストに追加
		r_filesList.push_back(path);
	}
	if(toExtractList.empty()){
		return true;
	}

	//未解凍の物のみ一時フォルダに解凍
	for(std::map<const ARCHIVE_ENTRY_INFO_TREE*,std::list<ARCHIVE_ENTRY_INFO_TREE*> >::iterator ite=toExtractList.begin();ite!=toExtractList.end();++ite){
		const std::list<ARCHIVE_ENTRY_INFO_TREE*> &filesList = (*ite).second;
		if(!ExtractItems(ConfMan,filesList,lpOutputDir,lpBase,false,strLog)){
			for(std::list<ARCHIVE_ENTRY_INFO_TREE*>::const_iterator iteRemove=filesList.begin(); iteRemove!=filesList.end(); ++iteRemove){
				//失敗したので削除
				ARCHIVE_ENTRY_INFO_TREE* lpNode = *iteRemove;
				CPath path=lpOutputDir;
				CString strItem;
				ArcEntryInfoTree_GetNodePathRelative(lpNode,lpBase,strItem);
				path.Append(strItem);
				UtilDeletePath(path);
			}
			return false;
		}
	}
	return true;
}


HRESULT CArchiveFileContent::AddItem(const std::list<CString> &fileList,LPCTSTR lpDestDir,CConfigManager& rConfig,CString &strLog)
{
	ASSERT(IsAddItemsSupported());
	if(!IsAddItemsSupported())return false;

	//---ファイル名チェック
	bool bUnicode=IsUnicodeCapable();
	for(std::list<CString>::const_iterator ite=fileList.begin();ite!=fileList.end();++ite){
		if(0==m_pathArcFileName.CompareNoCase(*ite)){
			//アーカイブ自身を追加しようとした
			return E_LF_SAME_INPUT_AND_OUTPUT;
		}
		//---UNICODEチェック
		if(!bUnicode){
			if(!UtilCheckT2A(*ite)){
				//ファイル名にUNICODE文字を持つファイルを圧縮しようとした
				return E_LF_UNICODE_NOT_SUPPORTED;
			}
		}
	}

	//---追加
	//基底ディレクトリ取得などはCArchiverDLL側に任せる
	if(m_lpArchiver->AddItemToArchive(m_pathArcFileName,m_bEncrypted,fileList,rConfig,lpDestDir,strLog)){
		return S_OK;
	}else{
		return S_FALSE;
	}
}

bool CArchiveFileContent::DeleteItems(CConfigManager &ConfMan,const std::list<ARCHIVE_ENTRY_INFO_TREE*> &fileList,CString &strLog)
{
	ASSERT(IsDeleteItemsSupported());
	if(!IsDeleteItemsSupported())return false;
	//削除対象を列挙
	std::list<CString> filesToDel;
	for(std::list<ARCHIVE_ENTRY_INFO_TREE*>::const_iterator ite=fileList.begin();ite!=fileList.end();++ite){
		(*ite)->EnumFiles(filesToDel);
	}
	return m_lpArchiver->DeleteItemFromArchive(m_pathArcFileName,ConfMan,filesToDel,strLog);
}

bool CArchiveFileContent::IsDeleteItemsSupported()const
{
	return (!m_bReadOnly) && m_lpArchiver && m_lpArchiver->QueryDeleteItemFromArchiveSupported(m_pathArcFileName);
}

bool CArchiveFileContent::IsAddItemsSupported()const
{
	return (!m_bReadOnly) && m_lpArchiver && m_lpArchiver->QueryAddItemToArchiveSupported(m_pathArcFileName);
}

bool CArchiveFileContent::IsUnicodeCapable()const
{
	return m_lpArchiver && m_lpArchiver->IsUnicodeCapable();
}

bool CArchiveFileContent::ReloadArchiverIfLost(CConfigManager &ConfigManager,CString &strErr)
{
	if(m_lpArchiver && !m_lpArchiver->IsOK()){
		return (LOAD_RESULT_OK==m_lpArchiver->LoadDLL(ConfigManager,strErr));
	}
	return true;
}
