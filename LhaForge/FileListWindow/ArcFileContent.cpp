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
#include "Utilities/StringUtil.h"
#include "Utilities/FileOperation.h"
#include "ArchiverCode/arc_interface.h"
#include "CommonUtil.h"


void UtilModifyPath(CString &strPath)
{
	// パスの修正
	strPath.Replace(_T("/"), _T("\\"));	//パス区切り文字

	int Ret = 0;
	do {
		Ret = strPath.Replace(_T("\\\\"), _T("\\"));	//\\を\に置き換えていく
	} while (Ret != 0);
	strPath.Replace(_T("..\\"), _T("__\\"));	//相対パス指定..は__に置き換える
	strPath.Replace(_T(":"), _T("__"));	//ドライブ名も置き換える(C:からC__へ)
}

void ArcEntryInfoTree_GetNodePathRelative(const ARCHIVE_ENTRY_INFO* lpNode, const ARCHIVE_ENTRY_INFO* lpBase, CString &strPath)
{
	/*
	//ここで面倒なことをしているのは、
	//lpBaseがフルパス名を持っていない(=LhaForgeが仮想的に作り出したフォルダ)ことがあるから。
	strPath = _T("");

	for (; lpNode->lpParent && lpBase != lpNode; lpNode = lpNode->lpParent) {
		CString strTmp = lpNode->strEntryName.c_str();
		strTmp.Replace(_T('/'), _T('\\'));

		CPath strBuffer = strTmp;
		if (-1 != strTmp.Find(_T('\\'))) {
			//ディレクトリノードの名前にパス区切り文字が入るのは、階層構造を無視した一覧の時のみ。
			//このときは、パス区切りを全部飛ばす必要がある。
			strBuffer.StripPath();	//一番最後の部分を残して、パスをカット
			strBuffer.RemoveBackslash();	//パス区切り文字を一旦削除
		}
		if (lpNode->isDirectory()) {
			strBuffer.AddBackslash();
		}
		strPath.Insert(0, strBuffer);
	}
	// 出力パスの修正
	UtilModifyPath(strPath);*/
}



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
	m_Root.clear();
	m_bReadOnly=false;

	m_pathArcFileName.Empty();
	m_bExtractEachSupported=false;
}


void CArchiveFileContent::InspectArchiveStruct(
	LPCTSTR lpFile,
	IArchiveContentUpdateHandler* lpHandler)
{
	ARCHIVE_FILE_TO_READ arc;
	arc.read_open(lpFile);
		
	bool bEncrypted = false;

	for (LF_ARCHIVE_ENTRY* entry = arc.begin(); entry; entry = arc.next()) {
		auto pathname = UtilPathRemoveLastSeparator(LF_sanitize_pathname(entry->get_pathname()));
		auto elements = UtilSplitString(pathname, L"/");

		if (elements.empty())continue;
		auto &item = m_Root.addEntry(elements);
		item._entryName = elements.back();
		item._fullpath = pathname;
		item._nAttribute = entry->get_file_mode();
		item._originalSize = entry->get_original_filesize();
		item._st_mtime = entry->get_mtime();

		bEncrypted = bEncrypted || entry->is_encrypted();

		//notifier
		if (lpHandler) {
			while (UtilDoMessageLoop())continue;
			lpHandler->onUpdated(item);
			if (lpHandler->isAborted()) {
				CANCEL_EXCEPTION();
			}
		}
	}

	m_bEncrypted = bEncrypted;
}


HRESULT CArchiveFileContent::ConstructFlat(LPCTSTR lpFile,CConfigManager &ConfMan,LPCTSTR lpDenyExt,bool bFilesOnly,CString &strErr,IArchiveContentUpdateHandler* lpHandler)
{
	return E_NOTIMPL;
}


HRESULT CArchiveFileContent::ConstructTree(LPCTSTR lpFile,CConfigManager &ConfMan,LPCTSTR lpDenyExt,bool bSkipMeaningless,CString &strErr,IArchiveContentUpdateHandler* lpHandler)
{
	Clear();

	m_bReadOnly = GetFileAttributesW(lpFile) & FILE_ATTRIBUTE_READONLY;
	m_pathArcFileName = lpFile;

	try {
		InspectArchiveStruct(lpFile, lpHandler);
		PostProcess(nullptr);
	} catch (LF_EXCEPTION&) {
		strErr.Format(IDS_ERROR_USERCANCEL);	//TODO
		Clear();
		return E_ABORT;
	}

	return S_OK;
}

void CArchiveFileContent::PostProcess(ARCHIVE_ENTRY_INFO* pNode)
{
	if (!pNode)pNode = &m_Root;

	if (0 == pNode->_nAttribute) {
		if (!pNode->_children.empty()) {
			pNode->_nAttribute = S_IFDIR;
		} else {
			pNode->_nAttribute = S_IFREG;
		}
	} else {
		if (!pNode->_children.empty()) {
			pNode->_nAttribute |= S_IFDIR;
		}
	}
	if (pNode->isDirectory()) {
		pNode->_originalSize = 0;
	}

	//children
	for (auto& child : pNode->_children) {
		PostProcess(child.get());

		if (pNode->isDirectory()) {
			if(pNode->_originalSize>=0){	//file size is known
				if (child->_originalSize >= 0) {
					pNode->_originalSize += child->_originalSize;
				} else {
					//file size unknown
					pNode->_originalSize = -1;
				}
			}
		}
	}
}


std::vector<std::shared_ptr<ARCHIVE_ENTRY_INFO> > CArchiveFileContent::findSubItem(
	const std::wstring& pattern,
	const ARCHIVE_ENTRY_INFO* parent)const
{
	//---breadth first search

	//pattern contains '/', then need to match fullpath
	bool matchPath = (std::wstring::npos != pattern.find(L'/'));

	std::vector<std::shared_ptr<ARCHIVE_ENTRY_INFO> > found;
	for (auto& child : parent->_children) {
		if (matchPath) {
			if (UtilPathMatchSpec(child->_fullpath, pattern)) {
				found.push_back(child);
			}
		} else {
			if (UtilPathMatchSpec(child->_entryName, pattern)) {
				found.push_back(child);
			}
		}
	}
	for (auto& child : parent->_children) {
		if (child->isDirectory()) {
			auto subFound = findSubItem(pattern, child.get());
			found.insert(found.end(), subFound.begin(), subFound.end());
		}
	}
	return found;
}


bool CArchiveFileContent::ExtractItems(CConfigManager &ConfMan,const std::list<ARCHIVE_ENTRY_INFO*> &items,LPCTSTR lpszDir,const ARCHIVE_ENTRY_INFO* lpBase,bool bCollapseDir,CString &strLog)
{
	//TODO
	RAISE_EXCEPTION(L"NOT INMPELEMTED");
	return false;// return m_lpArchiver->ExtractItems(m_pathArcFileName, ConfMan, lpBase, items, lpszDir, bCollapseDir, strLog);
}

void CArchiveFileContent::CollectUnextractedFiles(LPCTSTR lpOutputDir,const ARCHIVE_ENTRY_INFO* lpBase,const ARCHIVE_ENTRY_INFO* lpParent,std::map<const ARCHIVE_ENTRY_INFO*,std::list<ARCHIVE_ENTRY_INFO*> > &toExtractList)
{
	size_t numChildren=lpParent->getNumChildren();
	for(size_t i=0;i<numChildren;i++){
		ARCHIVE_ENTRY_INFO* lpNode=lpParent->getChild(i);
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
bool CArchiveFileContent::MakeSureItemsExtracted(CConfigManager &ConfMan,LPCTSTR lpOutputDir,const ARCHIVE_ENTRY_INFO* lpBase,const std::list<ARCHIVE_ENTRY_INFO*> &items,std::list<CString> &r_filesList,bool bOverwrite,CString &strLog)
{
	//選択されたアイテムを列挙
	std::map<const ARCHIVE_ENTRY_INFO*,std::list<ARCHIVE_ENTRY_INFO*> > toExtractList;

	std::list<CString> newFilesList;	//これから解凍するファイルのディスク上のパス名

	for(std::list<ARCHIVE_ENTRY_INFO*>::const_iterator ite=items.begin();ite!=items.end();++ite){
		// 存在をチェックし、もし解凍済みであればそれを開く
		ARCHIVE_ENTRY_INFO* lpNode=*ite;
		CPath path=lpOutputDir;

		CString strItem;
		ArcEntryInfoTree_GetNodePathRelative(lpNode,lpBase,strItem);
		path.Append(strItem);

		if(bOverwrite){
			// 上書き解凍するので、存在するファイルは削除
			if(lpNode->isDirectory()){
				if(::PathIsDirectory(path))UtilDeleteDir((const wchar_t*)path,true);
			}else{
				if(::PathFileExists(path))UtilDeletePath((const wchar_t*)path);
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
	for(std::map<const ARCHIVE_ENTRY_INFO*,std::list<ARCHIVE_ENTRY_INFO*> >::iterator ite=toExtractList.begin();ite!=toExtractList.end();++ite){
		const std::list<ARCHIVE_ENTRY_INFO*> &filesList = (*ite).second;
		if(!ExtractItems(ConfMan,filesList,lpOutputDir,lpBase,false,strLog)){
			for(std::list<ARCHIVE_ENTRY_INFO*>::const_iterator iteRemove=filesList.begin(); iteRemove!=filesList.end(); ++iteRemove){
				//失敗したので削除
				ARCHIVE_ENTRY_INFO* lpNode = *iteRemove;
				CPath path=lpOutputDir;
				CString strItem;
				ArcEntryInfoTree_GetNodePathRelative(lpNode,lpBase,strItem);
				path.Append(strItem);
				UtilDeletePath((const wchar_t*)path);
			}
			return false;
		}
	}
	return true;
}


HRESULT CArchiveFileContent::AddItem(const std::list<CString> &fileList,LPCTSTR lpDestDir,CConfigManager& rConfig,CString &strLog)
{
	//---ファイル名チェック
	for(std::list<CString>::const_iterator ite=fileList.begin();ite!=fileList.end();++ite){
		if(0==m_pathArcFileName.CompareNoCase(*ite)){
			//アーカイブ自身を追加しようとした
			return E_LF_SAME_INPUT_AND_OUTPUT;
		}
	}

	//TODO
	RAISE_EXCEPTION(L"NOT INMPELEMTED");
	return E_NOTIMPL;
/*	//---追加
	//基底ディレクトリ取得などはCArchiverDLL側に任せる
	if(m_lpArchiver->AddItemToArchive(m_pathArcFileName,m_bEncrypted,fileList,rConfig,lpDestDir,strLog)){
		return S_OK;
	}else{
		return S_FALSE;
	}*/
}

bool CArchiveFileContent::DeleteItems(CConfigManager &ConfMan,const std::list<ARCHIVE_ENTRY_INFO*> &fileList,CString &strLog)
{
	//TODO
	RAISE_EXCEPTION(L"NOT INMPELEMTED");
	return false;
/*	//削除対象を列挙
	std::list<CString> filesToDel;
	for(std::list<ARCHIVE_ENTRY_INFO*>::const_iterator ite=fileList.begin();ite!=fileList.end();++ite){
		(*ite)->EnumFiles(filesToDel);
	}
	return m_lpArchiver->DeleteItemFromArchive(m_pathArcFileName,ConfMan,filesToDel,strLog);*/
}

