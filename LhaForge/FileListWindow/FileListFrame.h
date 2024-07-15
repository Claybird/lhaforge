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
#include "resource.h"
#include "ConfigCode/ConfigFile.h"
#include "MenuCommand.h"
#include "FileListTabClient.h"
#include "OLE/DropTarget.h"

const auto LHAFORGE_FILE_LIST_CLASS = L"LhaForgeFileList";

class CFileListFrame:
	public CFrameWindowImpl<CFileListFrame>,
	public CUpdateUI<CFileListFrame>,
	public CMessageFilter,
	public CIdleHandler,
	public ILFDropCommunicator
{
protected:
	CConfigFile &mr_Config;
	CConfigFileListWindow m_ConfFLW;
	LF_COMPRESS_ARGS m_compressArgs;

	CAccelerator m_AccelEx;		//keyboard accelerator
	std::unique_ptr<CFileListTabClient> m_TabClientWnd;
	CMultiPaneStatusBarCtrl m_StatusBar;
	CRect m_WindowRect;

protected:
	CLFDropTarget m_DropTarget;
	void EnableDropTarget(bool bEnable);
	//---IDropCommunicator
	HRESULT DragEnter(IDataObject *lpDataObject, POINTL &pt, DWORD &dwEffect)override {return DragOver(lpDataObject, pt, dwEffect);}
	HRESULT DragLeave()override { return S_OK; }
	HRESULT DragOver(IDataObject*,POINTL&,DWORD&)override;
	HRESULT Drop(IDataObject*,POINTL&,DWORD&)override;
protected:
	void UpdateUpDirButtonState();
	void EnableEntryExtractOperationMenu(bool);
	void EnableEntryDeleteOperationMenu(bool bActive) { UIEnable(ID_MENUITEM_DELETE_SELECTED, bActive); }
	void EnableAddItemsMenu(bool bActive) {
		UIEnable(ID_MENUITEM_ADD_FILE, bActive);
		UIEnable(ID_MENUITEM_ADD_DIRECTORY, bActive);
	}
	void EnableSendTo_OpenAppMenu(bool bEnable) {
		//open with app / sendto
		CMenuHandle cMenu[] = {
			GetAdditionalMenuHandle(MENUTYPE::UserApp),
			GetAdditionalMenuHandle(MENUTYPE::SendTo)
		};
		for (auto &menu : cMenu) {
			int size = menu.GetMenuItemCount();
			for (int i = 0; i < size; i++) {
				menu.EnableMenuItem(i, MF_BYPOSITION | (bEnable ? MF_ENABLED : MF_GRAYED));
			}
		}
	}
	void UpdateWindowTitle();
	void UpdateStatusBar();
	void UpdateMenuState();
	HANDLE GetMultiOpenLockMutex(const std::wstring& strMutex);

	enum class MENUTYPE { UserApp, SendTo, };
	HMENU GetAdditionalMenuHandle(MENUTYPE type);

	HWND CreateToolBarCtrl(HWND hWndParent, UINT nResourceID,HIMAGELIST hImageList);
protected:
	//---internal window functions
	LRESULT OnCreate(LPCREATESTRUCT lpcs);
	LRESULT OnDestroy(UINT, WPARAM, LPARAM, BOOL& bHandled);
	void OnSize(UINT uType, CSize);
	void OnMove(const CPoint&);
	void OnCommandCloseWindow(UINT uNotifyCode, int nID, HWND hWndCtl) { DestroyWindow(); }
	void OnUpDir(UINT, int, HWND) {
		CFileListTabItem* pTab = m_TabClientWnd->GetCurrentTab();
		if (pTab)pTab->Model.MoveUpDir();
	}
	void OnConfigure(UINT,int,HWND);

	void OnListViewStyle(UINT,int,HWND);
	void OnRefresh(UINT, int, HWND) { m_TabClientWnd->ReopenArchiveFile(); }
	LRESULT OnRefresh(UINT, WPARAM, LPARAM, BOOL& bHandled) { m_TabClientWnd->ReopenArchiveFile(); return 0; }
	void OnOpenArchive(UINT,int,HWND);
	void OnCloseTab(UINT,int,HWND);
	void OnNextTab(UINT,int,HWND);
	void OnToggleFocus(UINT,int,HWND);
	LRESULT OnMouseWheel(UINT,short,CPoint&);

	LRESULT OnFileListUpdated(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
		UpdateWindowTitle();
		UpdateMenuState();
		UpdateStatusBar();
		UpdateUpDirButtonState();
		return 0;
	}
	LRESULT OnFileListWndStateChanged(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnOpenByPropName(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnActivateFile(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
protected:
	BEGIN_MSG_MAP_EX(CFileListFrame)
		MSG_WM_CREATE(OnCreate)
		MESSAGE_HANDLER(WM_DESTROY,OnDestroy)
		MSG_WM_SIZE(OnSize)
		MSG_WM_MOVE(OnMove)
		MSG_WM_MOUSEWHEEL(OnMouseWheel)
		MESSAGE_HANDLER(WM_FILELIST_REFRESH,OnRefresh);
		MESSAGE_HANDLER(WM_FILELIST_OPEN_BY_PROPNAME,OnOpenByPropName);
		MESSAGE_HANDLER(WM_LHAFORGE_FILELIST_ACTIVATE_FILE,OnActivateFile);
		COMMAND_ID_HANDLER_EX(ID_MENUITEM_CLOSE_WINDOW,OnCommandCloseWindow)
		COMMAND_ID_HANDLER_EX(ID_MENUITEM_UPDIR,OnUpDir)
		COMMAND_ID_HANDLER_EX(ID_MENUITEM_LISTVIEW_SMALLICON,OnListViewStyle)
		COMMAND_ID_HANDLER_EX(ID_MENUITEM_LISTVIEW_LARGEICON,OnListViewStyle)
		COMMAND_ID_HANDLER_EX(ID_MENUITEM_LISTVIEW_REPORT,OnListViewStyle)
		COMMAND_ID_HANDLER_EX(ID_MENUITEM_LISTVIEW_LIST,OnListViewStyle)
		COMMAND_ID_HANDLER_EX(ID_MENUITEM_REFRESH,OnRefresh)
		COMMAND_ID_HANDLER_EX(ID_MENUITEM_CONFIGURE,OnConfigure)
		COMMAND_ID_HANDLER_EX(ID_MENUITEM_OPENARCHIVE,OnOpenArchive);
		COMMAND_ID_HANDLER_EX(ID_MENUITEM_CLOSETAB,OnCloseTab);
		COMMAND_ID_HANDLER_EX(ID_MENUITEM_NEXTTAB,OnNextTab);
		COMMAND_ID_HANDLER_EX(ID_MENUITEM_PREVTAB,OnNextTab);
		MESSAGE_HANDLER(WM_FILELIST_MODELCHANGED, OnFileListUpdated);
		MESSAGE_HANDLER(WM_FILELIST_ARCHIVE_LOADED, OnFileListUpdated)
		MESSAGE_HANDLER(WM_FILELIST_NEWCONTENT, OnFileListUpdated)
		MESSAGE_HANDLER(WM_FILELIST_UPDATED, OnFileListUpdated)
		MESSAGE_HANDLER(WM_FILELIST_WND_STATE_CHANGED, OnFileListWndStateChanged)
		COMMAND_ID_HANDLER_EX(ID_MENUITEM_TOGGLE_FOCUS,OnToggleFocus)
		CHAIN_CLIENT_COMMANDS()
		CHAIN_MSG_MAP(CUpdateUI<CFileListFrame>)
		CHAIN_MSG_MAP(CFrameWindowImpl<CFileListFrame>)
		REFLECT_NOTIFICATIONS()
	END_MSG_MAP()

public:
	DECLARE_FRAME_WND_CLASS_EX(LHAFORGE_FILE_LIST_CLASS,IDR_MAINFRAME, CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS, COLOR_WINDOW)

	BEGIN_UPDATE_UI_MAP(CFileListFrame)
		UPDATE_ELEMENT(ID_MENUITEM_LISTVIEW_SMALLICON,			UPDUI_MENUPOPUP|UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_MENUITEM_LISTVIEW_LARGEICON,			UPDUI_MENUPOPUP|UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_MENUITEM_LISTVIEW_REPORT,				UPDUI_MENUPOPUP|UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_MENUITEM_LISTVIEW_LIST,				UPDUI_MENUPOPUP|UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_MENUITEM_REFRESH,						UPDUI_MENUPOPUP|UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_MENUITEM_EXTRACT_ARCHIVE,				UPDUI_MENUPOPUP|UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_MENUITEM_TEST_ARCHIVE,				UPDUI_MENUPOPUP|UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_MENUITEM_CLEAR_TEMPORARY,				UPDUI_MENUPOPUP|UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_MENUITEM_EXTRACT_SELECTED,			UPDUI_MENUPOPUP|UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_MENUITEM_EXTRACT_TEMPORARY,			UPDUI_MENUPOPUP|UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_MENUITEM_EXTRACT_SELECTED_SAMEDIR,	UPDUI_MENUPOPUP|UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_MENUITEM_DELETE_SELECTED,				UPDUI_MENUPOPUP|UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_MENUITEM_OPEN_ASSOCIATION,			UPDUI_MENUPOPUP|UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_MENUITEM_OPEN_ASSOCIATION_OVERWRITE,	UPDUI_MENUPOPUP|UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_MENUITEM_UPDIR,						UPDUI_MENUPOPUP|UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_MENUITEM_SELECT_ALL,					UPDUI_MENUPOPUP|UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_MENUITEM_SHOW_COLUMNHEADER_MENU,		UPDUI_MENUPOPUP|UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_MENUITEM_LISTMODE_TREE,				UPDUI_MENUPOPUP|UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_MENUITEM_LISTMODE_FLAT,				UPDUI_MENUPOPUP|UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_MENUITEM_LISTMODE_FLAT_FILESONLY,		UPDUI_MENUPOPUP|UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_MENUITEM_FINDITEM,					UPDUI_MENUPOPUP|UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_MENUITEM_FINDITEM_END,				UPDUI_MENUPOPUP|UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_MENUITEM_NEXTTAB,						UPDUI_MENUPOPUP|UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_MENUITEM_PREVTAB,						UPDUI_MENUPOPUP|UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_MENUITEM_CLOSETAB,					UPDUI_MENUPOPUP|UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_MENUITEM_ADD_FILE,					UPDUI_MENUPOPUP|UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_MENUITEM_ADD_DIRECTORY,				UPDUI_MENUPOPUP|UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_MENUITEM_SORT_FILENAME,				UPDUI_MENUPOPUP|UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_MENUITEM_SORT_FULLPATH,				UPDUI_MENUPOPUP|UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_MENUITEM_SORT_ORIGINALSIZE,			UPDUI_MENUPOPUP|UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_MENUITEM_SORT_TYPENAME,				UPDUI_MENUPOPUP|UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_MENUITEM_SORT_FILETIME,				UPDUI_MENUPOPUP|UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_MENUITEM_SORT_ATTRIBUTE,				UPDUI_MENUPOPUP|UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_MENUITEM_SORT_COMPRESSEDSIZE,			UPDUI_MENUPOPUP|UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_MENUITEM_SORT_METHOD,					UPDUI_MENUPOPUP|UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_MENUITEM_SORT_RATIO,					UPDUI_MENUPOPUP|UPDUI_TOOLBAR)
	END_UPDATE_UI_MAP()

	//---
	CFileListFrame(CConfigFile &conf);
	virtual ~CFileListFrame(){}

	BOOL OnIdle(){
		UIUpdateStatusBar();
        UIUpdateToolBar();
		return FALSE;
	}
	BOOL PreTranslateMessage(MSG* pMsg);

	HRESULT OpenArchiveFile(const std::filesystem::path& fname,bool bAllowRelayOpen=true);

	void GetFreeClientRect(CRect &rc){
		GetClientRect(rc);
		CRect rcToolbar;
		::GetWindowRect(m_hWndToolBar,rcToolbar);
		CRect rcStatusBar;
		::GetWindowRect(m_hWndStatusBar,rcStatusBar);
		rc.top+=rcToolbar.Height();
		rc.bottom-=rcStatusBar.Height();
	}
};
