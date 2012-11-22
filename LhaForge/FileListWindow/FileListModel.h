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

#pragma once
//ファイル一覧構造を保持する
#include "ArcFileContent.h"
#include "../Utilities/EventDispatcher.h"
#include "../Utilities/TemporaryDirMgr.h"

class CArchiverDLL;
enum DLL_ID;

enum FILELISTMODE{	//ファイル一覧ウィンドウの表示方法
	FILELIST_TREE,				//エクスプローラライクのディレクトリ階層表示
	FILELIST_FLAT,				//フォルダ階層を無視して表示
	FILELIST_FLAT_FILESONLY,	//フォルダ階層を無視し、ファイルのみ表示

	ENUM_COUNT_AND_LASTITEM(FILELISTMODE),
};

//ファイル情報
enum FILEINFO_TYPE{
	FILEINFO_INVALID=-1,
	FILEINFO_FILENAME,		//ファイル名
	FILEINFO_FULLPATH,		//フルパス情報
	FILEINFO_ORIGINALSIZE,	//圧縮前ファイルサイズ
	FILEINFO_TYPENAME,		//ファイル種類名
	FILEINFO_FILETIME,		//ファイル最終更新日時
	FILEINFO_ATTRIBUTE,		//ファイル属性
	FILEINFO_COMPRESSEDSIZE,//圧縮後ファイルサイズ
	FILEINFO_METHOD,		//圧縮メソッド
	FILEINFO_RATIO,			//圧縮率
	FILEINFO_CRC,			//CRC

	ENUM_COUNT_AND_LASTITEM(FILEINFO),
};


class CFileListModel:public CEventDispatcher
{
protected:
	CArchiveFileContent			m_Content;
	ARCHIVE_ENTRY_INFO_TREE*	m_lpCurrentNode;
	ARCHIVE_ENTRY_INFO_TREE		m_FoundItems;
	//ソート済みのカレントノード状態
	std::vector<ARCHIVE_ENTRY_INFO_TREE*>	m_SortedChildren;
	CTemporaryDirectoryManager	m_TempDirManager;	//一時フォルダ管理

	CConfigManager&				mr_Config;
	//ソート関係
	bool	m_bSortDescending;
	int		m_nSortKeyType;

	FILELISTMODE m_Mode;
	DLL_ID	m_idForceDLL;
	//openassoc
	static CString ms_strExtAccept,ms_strExtDeny;
protected:
	//---internal functions
	void SortCurrentEntries();
public:
	CFileListModel(CConfigManager&);
	virtual ~CFileListModel();

	HRESULT OpenArchiveFile(LPCTSTR,DLL_ID idForceDLL,FILELISTMODE flMode,CString &strErr,IArchiveContentUpdateHandler* =NULL);
	HRESULT ReopenArchiveFile(FILELISTMODE flMode,CString &strErr,IArchiveContentUpdateHandler* =NULL);
	void Clear();

	void GetDirStack(std::stack<CString>&);
	bool SetDirStack(const std::stack<CString>&);

	ARCHIVE_ENTRY_INFO_TREE* GetCurrentNode(){return m_lpCurrentNode;}
	const ARCHIVE_ENTRY_INFO_TREE* GetCurrentNode()const{return m_lpCurrentNode;}
	void SetCurrentNode(ARCHIVE_ENTRY_INFO_TREE* lpN);

	void SetSortKeyType(int nSortKeyType);
	void SetSortMode(bool bSortDescending);
	int GetSortKeyType()const{return m_nSortKeyType;}
	bool GetSortMode()const{return m_bSortDescending;}

	bool MoveUpDir();
	bool MoveDownDir(ARCHIVE_ENTRY_INFO_TREE*);

	bool IsRoot()const{return (GetCurrentNode()==m_Content.GetRootNode());}
	bool IsOK()const{return m_Content.GetArchiver()!=NULL;}	//ファイルリストが正常なときは、lpArchiverはnon-NULL
	bool IsFindMode()const{return m_lpCurrentNode==&m_FoundItems;}

	ARCHIVE_ENTRY_INFO_TREE* GetFileListItemByIndex(long iIndex);

	//lpTop以下のファイルを検索;検索結果を格納したARCHIVE_ENTRY_INFO_TREEのポインタを返す
	ARCHIVE_ENTRY_INFO_TREE* FindItem(LPCTSTR lpszMask,ARCHIVE_ENTRY_INFO_TREE *lpTop);
	void EndFindItem();

	bool ReloadArchiverIfLost(CString &strErr);

	//処理対象アーカイブ名を取得
	LPCTSTR GetArchiveFileName()const{return m_Content.GetArchiveFileName();}
	const CArchiverDLL* GetArchiver()const{return m_Content.GetArchiver();}
	ARCHIVE_ENTRY_INFO_TREE* GetRootNode(){return m_Content.GetRootNode();}
	const ARCHIVE_ENTRY_INFO_TREE* GetRootNode()const{return m_Content.GetRootNode();}

	bool IsExtractEachSupported()const{return m_Content.IsExtractEachSupported();}
	bool IsDeleteItemsSupported()const{return m_Content.IsDeleteItemsSupported();}
	bool IsAddItemsSupported()const{return m_Content.IsAddItemsSupported();}
	bool IsUnicodeCapable()const{return m_Content.IsUnicodeCapable();}
	BOOL CheckArchiveExists()const{return m_Content.CheckArchiveExists();}

	HRESULT AddItem(const std::list<CString>&,LPCTSTR lpDestDir,CString&);	//ファイルを追加圧縮
	bool ExtractItems(const std::list<ARCHIVE_ENTRY_INFO_TREE*> &items,LPCTSTR lpszDir,const ARCHIVE_ENTRY_INFO_TREE* lpBase,bool bCollapseDir,CString &strLog);
	HRESULT ExtractItems(HWND hWnd,bool bSameDir,const std::list<ARCHIVE_ENTRY_INFO_TREE*> &items,const ARCHIVE_ENTRY_INFO_TREE* lpBase,CString &strLog);
	//bOverwrite:trueなら存在するテンポラリファイルを削除してから解凍する
	bool MakeSureItemsExtracted(LPCTSTR lpOutputDir,const ARCHIVE_ENTRY_INFO_TREE* lpBase,const std::list<ARCHIVE_ENTRY_INFO_TREE*> &items,std::list<CString> &r_filesList,bool bOverwrite,CString &strLog);
	bool DeleteItems(const std::list<ARCHIVE_ENTRY_INFO_TREE*>&,CString&);

	static void SetOpenAssocExtDeny(LPCTSTR lpExtDeny){ms_strExtDeny=lpExtDeny;}
	static LPCTSTR GetOpenAssocExtDeny(){return ms_strExtDeny;}
	static void SetOpenAssocExtAccept(LPCTSTR lpExtAccept){ms_strExtAccept=lpExtAccept;}
	static LPCTSTR GetOpenAssocExtAccept(){return ms_strExtAccept;}

	FILELISTMODE GetListMode()const{return m_Mode;}

	bool ExtractArchive();	//::Extract()を呼ぶ
	void TestArchive();

	void ClearTempDir();
};
