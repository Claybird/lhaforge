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
#include "FileListTabItem.h"
#include "../resource.h"

#define FILELISTWINDOW_DEFAULT_TREE_WIDTH	175

class CFileListFrame;
struct CConfigFileListWindow;
class CFileListTabClient:public CTabView,public CEventDispatcher
{
public:
	BOOL PreTranslateMessage(MSG* pMsg);
protected:
	std::vector<std::shared_ptr<CFileListTabItem> > m_GC;
	//HWND				m_hFrameWnd;
	CFileListFrame&		m_rFrameWnd;
	int					m_ColumnIndexArray[FILEINFO_ITEM_COUNT];	//リストビューカラムの並び順
	int					m_FileInfoWidth[FILEINFO_ITEM_COUNT];
	bool				m_bShowTab;			//タブ表示?
	bool				m_bShowTreeView;	//ツリービュー表示ならTrue

	CFileListTabItem*	m_lpPrevTab;
	const CConfigFileListWindow& m_confFLW;
	const CConfigFile& mr_Config;

	//各リストビューで共通化する設定
	int		m_nTreeWidth;
	bool	m_bSortDescending;
	int		m_nSortKeyType;
	DWORD	m_dwListStyle;
protected:
	BEGIN_MSG_MAP_EX(CFileListTabClient)
		//MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MSG_WM_DESTROY(OnDestroy)
		MSG_WM_SIZE(OnSize)
		NOTIFY_CODE_HANDLER_EX(NM_SETFOCUS, OnWndStateChanged)
		NOTIFY_CODE_HANDLER_EX(LVN_ITEMCHANGED, OnWndStateChanged)
		REFLECTED_NOTIFY_CODE_HANDLER_EX(TCN_SELCHANGING, OnTabSelChanging)
		REFLECTED_NOTIFY_CODE_HANDLER_EX(TCN_SELCHANGE, OnTabSelChanged)
		REFLECTED_NOTIFY_CODE_HANDLER_EX(TBVN_PAGEACTIVATED, OnTabSelChanged)
		REFLECTED_NOTIFY_CODE_HANDLER_EX(TBVN_CONTEXTMENU,OnContextMenu)	//タブメニュー
		COMMAND_ID_HANDLER_EX(ID_MENUITEM_TOGGLE_TREEVIEW,OnToggleTreeView)
		COMMAND_ID_HANDLER_EX(ID_MENUITEM_EXTRACT_ARCHIVE,OnExtractArchive)
		COMMAND_ID_HANDLER_EX(ID_MENUITEM_EXTRACT_ARCHIVE_AND_CLOSE,OnExtractArchive)
		COMMAND_ID_HANDLER_EX(ID_MENUITEM_TEST_ARCHIVE,OnTestArchive)
		COMMAND_ID_HANDLER_EX(ID_MENUITEM_EXTRACT_ARCHIVE_ALL,OnExtractAll);
		COMMAND_ID_HANDLER_EX(ID_MENUITEM_EXTRACT_ARCHIVE_AND_CLOSE_ALL,OnExtractAll);
		COMMAND_RANGE_HANDLER_EX(ID_MENUITEM_SORT_FILENAME,ID_MENUITEM_SORT_CRC,OnSortItemMenu)
	if(GetActivePage()!=-1)CHAIN_COMMANDS_MEMBER(GetCurrentTab()->ListView)
		CHAIN_MSG_MAP(CTabView)
		REFLECT_NOTIFICATIONS()
		DEFAULT_REFLECTION_HANDLER()
	ALT_MSG_MAP(1)	//タブコントロール
		MSG_WM_MBUTTONUP(OnMButtonUp)	//マウスホイール(中ボタン)クリック
		REFLECTED_NOTIFY_CODE_HANDLER_EX(TCN_SELCHANGING, OnTabSelChanging)
		REFLECTED_NOTIFY_CODE_HANDLER_EX(TCN_SELCHANGE, OnTabSelChanged)
		CHAIN_MSG_MAP_ALT(CTabView,1)
	END_MSG_MAP()
protected:
	//---イベントハンドラ
	LRESULT OnDestroy();
	void OnSize(UINT uType, CSize &size);
	LRESULT OnTabSelChanging(LPNMHDR pnmh);
	LRESULT OnTabSelChanged(LPNMHDR pnmh);
	LRESULT OnWndStateChanged(LPNMHDR pnmh);
	LRESULT OnContextMenu(LPNMHDR pnmh);
	void OnMButtonUp(UINT, CPoint&);
	void OnExtractArchive(UINT,int,HWND);	//アーカイブをすべて解凍
	void OnTestArchive(UINT,int,HWND);	//アーカイブを検査
	void OnSortItemMenu(UINT,int,HWND);
	void OnToggleTreeView(UINT,int,HWND);
	void OnExtractAll(UINT,int,HWND);
protected:
	int CreateNewTab();
	void ClearAllTabs();
	void RemoveTab(int);
	void RemoveTabExcept(int);
	void FitClient();
	void OnActivateTab(int newIdx);
	void OnDeactivateTab(CFileListTabItem*);
	void GetTabSettingsToClient(CFileListTabItem*);	//指定したタブの設定をメンバ変数に読み込む
	void UpdateClientArea();
public:
	CFileListTabClient(const CConfigFile&, const CConfigFileListWindow&, CFileListFrame&);
	virtual ~CFileListTabClient(){ClearAllTabs();}
	//void SetFrameWnd(HWND hWnd){m_hFrameWnd=hWnd;}
	CFileListTabItem* GetCurrentTab();

	HRESULT OpenArchiveInTab(LPCTSTR lpszArc,LPCTSTR lpMutexName,HANDLE hMutex,CString &strErr);
	HRESULT ReopenArchiveFile(int nPage=-1);
	void UpdateFileListConfig(const CConfigFileListWindow& ConfFLW);
	bool ReopenArchiveFileAll();

	void StoreSettings(CConfigFileListWindow&);

	void SetCurrentTab(int idx);
	void SetCurrentTab(HANDLE);
	void CloseCurrentTab(){RemoveTab(GetActivePage());}

	DWORD GetListViewStyle();
	void SetListViewStyle(DWORD);

	void ShowTabCtrl(bool bShow);
	bool IsTabEnabled()const{return m_bShowTab;}
};
