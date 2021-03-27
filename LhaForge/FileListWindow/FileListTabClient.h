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
#include "resource.h"


class CFileListFrame;
struct CConfigFileListWindow;
class CFileListTabClient:public CTabView,public CEventDispatcher
{
public:
	BOOL PreTranslateMessage(MSG* pMsg);
protected:
	std::vector<std::shared_ptr<CFileListTabItem> > m_GC;
	CFileListFrame&		m_rFrameWnd;
	bool				m_bShowTab;

	const CConfigFileListWindow& m_confFLW;
	const LF_COMPRESS_ARGS& mr_compressArgs;
protected:
	BEGIN_MSG_MAP_EX(CFileListTabClient)
		MSG_WM_DESTROY(OnDestroy)
		MSG_WM_SIZE(OnSize)
		NOTIFY_CODE_HANDLER_EX(NM_SETFOCUS, OnWndStateChanged)
		NOTIFY_CODE_HANDLER_EX(LVN_ITEMCHANGED, OnWndStateChanged)
		NOTIFY_CODE_HANDLER_EX(TCN_SELCHANGING, OnDeactivatingTab)
		REFLECTED_NOTIFY_CODE_HANDLER_EX(TCN_SELCHANGE, OnActivateTab)
		REFLECTED_NOTIFY_CODE_HANDLER_EX(TBVN_PAGEACTIVATED, OnActivateTab)
		REFLECTED_NOTIFY_CODE_HANDLER_EX(TBVN_CONTEXTMENU,OnContextMenu)
		REFLECTED_NOTIFY_CODE_HANDLER_EX(TBVN_TABCLOSEBTN, OnTabCloseBtn)
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
	ALT_MSG_MAP(1)	//tab control
		MSG_WM_MBUTTONUP(OnMButtonUp)	//mouse middle button click
		CHAIN_MSG_MAP_ALT(CTabView,1)
	END_MSG_MAP()
protected:
	LRESULT OnDestroy() { ClearAllTabs(); return 0; }
	void OnSize(UINT uType, CSize &size);
	LRESULT OnDeactivatingTab(LPNMHDR pnmh) {
		OnDeactivatingTab(GetActivePage());
		SetMsgHandled(FALSE);
		return 0;
	}
	void OnDeactivatingTab(int page);
	LRESULT OnActivateTab(LPNMHDR pnmh) {
		OnActivateTab(GetActivePage());
		SetMsgHandled(FALSE);
		return 0;
	}
	void OnActivateTab(int page);
	LRESULT OnWndStateChanged(LPNMHDR) { dispatchEvent(WM_FILELIST_WND_STATE_CHANGED); return 0; }
	LRESULT OnContextMenu(LPNMHDR pnmh);
	LRESULT OnTabCloseBtn(LPNMHDR pnmh);
	void OnMButtonUp(UINT, CPoint&);
	void OnExtractArchive(UINT,int,HWND);
	void OnTestArchive(UINT,int,HWND);
	void OnSortItemMenu(UINT,int,HWND);
	void OnToggleTreeView(UINT,int,HWND);
	void OnExtractAll(UINT,int,HWND);
protected:
	int CreateNewTab();
	void ClearAllTabs();
	void RemoveTab(int);
	void RemoveTabExcept(int);
	void UpdateClientArea();
public:
	CFileListTabClient(const CConfigFileListWindow&, const LF_COMPRESS_ARGS&, CFileListFrame&);
	virtual ~CFileListTabClient(){ClearAllTabs();}
	CFileListTabItem* GetCurrentTab();

	HRESULT OpenArchiveInTab(const std::filesystem::path& arcpath, const std::wstring& mutexName, HANDLE hMutex, ARCLOG &arcLog);
	HRESULT ReopenArchiveFile(int nPage=-1);
	void UpdateFileListConfig(const CConfigFileListWindow& ConfFLW);
	bool ReopenArchiveFileAll();

	void StoreSettings(CConfigFileListWindow&);

	void SetActivePage(int i) { __super::SetActivePage(i); }
	void SetActivePage(HANDLE);
	void CloseCurrentTab(){RemoveTab(GetActivePage());}

	DWORD GetListViewStyle();
	void SetListViewStyle(DWORD);

	void ShowTabCtrl(bool bShow);
	bool IsTabEnabled()const{return m_bShowTab;}
};
