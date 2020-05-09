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
//ファイル一覧構造を保持する
#include "ArcFileContent.h"
#include "Utilities/EventDispatcher.h"
#include "Utilities/FileOperation.h"

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
	//openassoc
	static CString ms_strExtAccept,ms_strExtDeny;
protected:
	//---internal functions
	void SortCurrentEntries();
public:
	CFileListModel(CConfigManager&);
	virtual ~CFileListModel();

	HRESULT OpenArchiveFile(LPCTSTR,FILELISTMODE flMode,CString &strErr,IArchiveContentUpdateHandler* =NULL);
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
	bool IsOK()const{return m_Content.IsOK();}	//ファイルリストが正常なときは、lpArchiverはnon-NULL
	bool IsFindMode()const{return m_lpCurrentNode==&m_FoundItems;}

	ARCHIVE_ENTRY_INFO_TREE* GetFileListItemByIndex(long iIndex);

	//lpTop以下のファイルを検索;検索結果を格納したARCHIVE_ENTRY_INFO_TREEのポインタを返す
	ARCHIVE_ENTRY_INFO_TREE* FindItem(LPCTSTR lpszMask,ARCHIVE_ENTRY_INFO_TREE *lpTop);
	void EndFindItem();

	//処理対象アーカイブ名を取得
	LPCTSTR GetArchiveFileName()const{return m_Content.GetArchiveFileName();}
	ARCHIVE_ENTRY_INFO_TREE* GetRootNode(){return m_Content.GetRootNode();}
	const ARCHIVE_ENTRY_INFO_TREE* GetRootNode()const{return m_Content.GetRootNode();}

	bool IsArchiveEncrypted()const{return m_Content.IsArchiveEncrypted();}
	[[deprecated("just a placeholder")]] bool IsExtractEachSupported()const { return false; }
	[[deprecated("just a placeholder")]] bool IsDeleteItemsSupported()const { return false; }
	[[deprecated("just a placeholder")]] bool IsAddItemsSupported()const { return false; }
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

	//ファイル名が指定した2つの条件で[許可]されるかどうか;拒否が優先
	bool IsPathAcceptableToOpenAssoc(LPCTSTR lpszPath, bool bDenyOnly)const {
		const auto lpDeny = GetOpenAssocExtDeny();
		const auto lpAccept = GetOpenAssocExtAccept();
		if (UtilExtMatchSpec(lpszPath, lpDeny)) {
			return false;
		}
		if (bDenyOnly) {
			return true;
		} else {
			if (UtilExtMatchSpec(lpszPath, lpAccept)) {
				return true;
			}
		}
		return false;
	}
};


