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
#include "ConfigCode/Dialogs/configwnd.h"
#include "ConfigCode/ConfigFile.h"
#include "ConfigCode/ConfigFileListWindow.h"
#include "Dialogs/LogListDialog.h"
#include "Utilities/OSUtil.h"
#include "Utilities/StringUtil.h"
#include "CommonUtil.h"
#include "resource.h"
#include "FileListFrame.h"

// SetProp identifier
std::wstring g_strPropIdentifier = UtilLoadString(IDS_MESSAGE_CAPTION) + UtilLoadString(IDS_LHAFORGE_VERSION_STRING);

CFileListFrame::CFileListFrame(CConfigFile &conf):
	mr_Config(conf),
	m_DropTarget(this)
{
	m_ConfFLW.load(mr_Config);
	m_compressArgs.load(mr_Config);
	m_TabClientWnd = std::make_unique<CFileListTabClient>(m_ConfFLW, m_compressArgs, *this);
}

BOOL CFileListFrame::PreTranslateMessage(MSG* pMsg)
{
	if (CFrameWindowImpl<CFileListFrame>::PreTranslateMessage(pMsg)) {
		return TRUE;
	}
	if (!m_AccelEx.IsNull() && m_AccelEx.TranslateAccelerator(m_hWnd, pMsg)) {
		return TRUE;
	}
	if (m_TabClientWnd->PreTranslateMessage(pMsg))return TRUE;
	return FALSE;
}

LRESULT CFileListFrame::OnCreate(LPCREATESTRUCT lpcs)
{
	//Set window prop: set identity of LhaForge window
	::SetPropW(m_hWnd, g_strPropIdentifier.c_str(), m_hWnd);

	CRect Rect;
	GetWindowRect(Rect);
	//Window size
	if(m_ConfFLW.general.StoreSetting){
		if(m_ConfFLW.dimensions.StoreWindowPosition){	//restore window position and size
			MoveWindow(
				m_ConfFLW.dimensions.WindowPos_x,
				m_ConfFLW.dimensions.WindowPos_y,
				m_ConfFLW.dimensions.Width,
				m_ConfFLW.dimensions.Height);
		}else{
			MoveWindow(Rect.left,Rect.top,
				m_ConfFLW.dimensions.Width,
				m_ConfFLW.dimensions.Height);
		}
	}else if(m_ConfFLW.dimensions.StoreWindowPosition){	//restore window position only
		MoveWindow(
			m_ConfFLW.dimensions.WindowPos_x,
			m_ConfFLW.dimensions.WindowPos_y,
			Rect.Width(),
			Rect.Height());
	}
	GetWindowRect(m_WindowRect);

	// icons
	HICON hIcon = AtlLoadIconImage(IDI_APP, LR_DEFAULTCOLOR,::GetSystemMetrics(SM_CXICON),::GetSystemMetrics(SM_CYICON));
	SetIcon(hIcon, TRUE);
	HICON hIconSmall = AtlLoadIconImage(IDI_APP, LR_DEFAULTCOLOR,::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON));
	SetIcon(hIconSmall, FALSE);

	if(m_ConfFLW.view.ShowToolbar){
		//toolbar
		CreateSimpleReBar(ATL_SIMPLE_REBAR_NOBORDER_STYLE);
		HIMAGELIST hImageList=NULL;
		if(!m_ConfFLW.view.strCustomToolbarImage.empty()){
			//custom toolbar image
			hImageList = ImageList_LoadImage(NULL,
				m_ConfFLW.view.strCustomToolbarImage.c_str(),
				0, 1, CLR_DEFAULT, IMAGE_BITMAP,
				LR_CREATEDIBSECTION | LR_DEFAULTSIZE|LR_LOADFROMFILE);
		}
		HWND hWndToolBar=CreateToolBarCtrl(m_hWnd,IDR_MAINFRAME,hImageList);//CreateSimpleToolBarCtrl(m_hWnd,IDR_MAINFRAME, FALSE, ATL_SIMPLE_TOOLBAR_PANE_STYLE);
		AddSimpleReBarBand(hWndToolBar);
		UIAddToolBar(hWndToolBar);
		SizeSimpleReBarBands();
	}

	// status bar
	{
		m_hWndStatusBar = m_StatusBar.Create(m_hWnd);
		UIAddStatusBar(m_hWndStatusBar);
		int nPanes[] = { ID_DEFAULT_PANE, IDS_PANE_ITEMCOUNT_INITIAL };
		m_StatusBar.SetPanes(nPanes, COUNTOF(nPanes));
		auto text = Format(UtilLoadString(IDS_PANE_ITEMCOUNT), 0, 0);
		m_StatusBar.SetPaneText(IDS_PANE_ITEMCOUNT_INITIAL, text.c_str());
	}

	// tab control
	{
		m_TabClientWnd->Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
		m_TabClientWnd->addEventListener(m_hWnd);

		//hide tab if disabled
		if (m_ConfFLW.general.DisableTab)m_TabClientWnd->ShowTabCtrl(false);
		m_hWndClient = *m_TabClientWnd;
	}

	//---------
	//Enable/disable list view style selector
	{
		CMenuHandle menuView = GetMenu();
		CMenuItemInfo mii;
		mii.fMask = MIIM_FTYPE;
		mii.fType = MFT_RADIOCHECK;
		menuView.SetMenuItemInfo(ID_MENUITEM_LISTVIEW_SMALLICON, FALSE, &mii);
		menuView.SetMenuItemInfo(ID_MENUITEM_LISTVIEW_LARGEICON, FALSE, &mii);
		menuView.SetMenuItemInfo(ID_MENUITEM_LISTVIEW_REPORT, FALSE, &mii);
		menuView.SetMenuItemInfo(ID_MENUITEM_LISTVIEW_LIST, FALSE, &mii);

		menuView.SetMenuItemInfo(ID_MENUITEM_LISTMODE_TREE, FALSE, &mii);
		menuView.SetMenuItemInfo(ID_MENUITEM_LISTMODE_FLAT, FALSE, &mii);
		menuView.SetMenuItemInfo(ID_MENUITEM_LISTMODE_FLAT_FILESONLY, FALSE, &mii);
	}

	//list view style
	if(m_ConfFLW.general.StoreSetting){
		//current status
		switch(m_ConfFLW.view.ListStyle){
		case LVS_SMALLICON:
			UISetCheck(ID_MENUITEM_LISTVIEW_SMALLICON, TRUE);
			break;
		case LVS_ICON:
			UISetCheck(ID_MENUITEM_LISTVIEW_LARGEICON, TRUE);
			break;
		case LVS_LIST:
			UISetCheck(ID_MENUITEM_LISTVIEW_LIST, TRUE);
			break;
		case LVS_REPORT:
			UISetCheck(ID_MENUITEM_LISTVIEW_REPORT, TRUE);
			break;
		default:
			ASSERT(!"Error");
			break;
		}
	}else{
		//default status
		UISetCheck(ID_MENUITEM_LISTVIEW_LARGEICON, TRUE);
		UISetCheck(ID_MENUITEM_LISTMODE_TREE, TRUE);
	}

	UpdateLayout();

	//message handler
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	pLoop->AddMessageFilter(this);
	pLoop->AddIdleHandler(this);

	//keyboard accelerator
	if(m_ConfFLW.general.ExitWithEscape){
		m_AccelEx.LoadAccelerators(IDR_ACCEL_EX);
	}

	//update menu
	EnableEntryExtractOperationMenu(false);
	EnableEntryDeleteOperationMenu(false);
	EnableAddItemsMenu(false);

	// build commands on file list window
	MenuCommand_MakeSendToCommands();
	MenuCommand_UpdateUserAppCommands(m_ConfFLW);

	MenuCommand_MakeUserAppMenu(GetAdditionalMenuHandle(MENUTYPE::UserApp));
	MenuCommand_MakeSendToMenu(GetAdditionalMenuHandle(MENUTYPE::SendTo));
	DrawMenuBar();

	// activate window
	SetForegroundWindow(m_hWnd);
	UpdateLayout();

	// enable Drag & Drop
	EnableDropTarget(true);
	return 0;
}

HMENU CFileListFrame::GetAdditionalMenuHandle(MENUTYPE type)
{
	const int nPos = 1;
	CMenuHandle cMenu=GetMenu();
	CMenuHandle cSubMenu=cMenu.GetSubMenu(nPos);
	int MenuCount=cSubMenu.GetMenuItemCount();
	for (int i = 0; i <= MenuCount; i++) {
		if (-1 == cSubMenu.GetMenuItemID(i)) {	//parent item for popup
			switch (type) {
			case MENUTYPE::SendTo:
				return cSubMenu.GetSubMenu(i + 1);
			case MENUTYPE::UserApp:
			default:
				return cSubMenu.GetSubMenu(i);
			}
			break;
		}
	}
	return nullptr;
}


LRESULT CFileListFrame::OnDestroy(UINT, WPARAM, LPARAM, BOOL& bHandled)
{
	::RemovePropW(m_hWnd, g_strPropIdentifier.c_str());

	m_ConfFLW.load(mr_Config);

	bool bSave=false;
	//store window settings
	if (m_ConfFLW.general.StoreSetting) {
		//window size
		m_ConfFLW.dimensions.Width = m_WindowRect.Width();
		m_ConfFLW.dimensions.Height = m_WindowRect.Height();

		m_TabClientWnd->StoreSettings(m_ConfFLW);

		if (m_ConfFLW.dimensions.StoreWindowPosition) {
			m_ConfFLW.dimensions.WindowPos_x = m_WindowRect.left;
			m_ConfFLW.dimensions.WindowPos_y = m_WindowRect.top;
		}

		m_ConfFLW.store(mr_Config);
		bSave = true;
	}
	if (m_ConfFLW.dimensions.StoreWindowPosition) {
		m_ConfFLW.dimensions.WindowPos_x = m_WindowRect.left;
		m_ConfFLW.dimensions.WindowPos_y = m_WindowRect.top;
		m_ConfFLW.store(mr_Config);
		bSave = true;
	}
	if(bSave){
		try {
			mr_Config.save();
		}catch(const LF_EXCEPTION& e){
			ErrorMessage(e.what());
		}
	}

	if(m_TabClientWnd->IsWindow())m_TabClientWnd->DestroyWindow();

	CMessageLoop* pLoop = _Module.GetMessageLoop();
	pLoop->RemoveMessageFilter(this);
	pLoop->RemoveIdleHandler(this);

	PostQuitMessage(0);
	//bHandled=false;
	return 0;
}


HRESULT CFileListFrame::OpenArchiveFile(const std::filesystem::path& fname,bool bAllowRelayOpen)
{
	if(m_TabClientWnd->GetPageCount()>0 && !m_TabClientWnd->IsTabEnabled()){
		//tab is disabled; clone instance
		std::wstring strParam(L"/l ");

		auto filePath=fname;
		strParam += L"\"" + filePath.wstring() + L"\"";
		int ret = (int)ShellExecuteW(nullptr, nullptr, UtilGetModulePath().c_str(), strParam.c_str(), nullptr, SW_RESTORE);
		if(ret<=32){
			//If the function succeeds, it returns a value greater than 32
			return E_FAIL;
		} else {
			return S_OK;
		}
	}else{
		//keep single instance
		if(bAllowRelayOpen && m_ConfFLW.general.KeepSingleInstance){
			static HWND s_hFirstWindow;
			
			s_hFirstWindow = nullptr;
			EnumWindows([](HWND hWnd, LPARAM lParam)->BOOL {
				if (hWnd != (HWND)lParam) {
					auto className = UtilGetWindowClassName(hWnd);
					if (LHAFORGE_FILE_LIST_CLASS == className) {
						if (!s_hFirstWindow) {
							s_hFirstWindow = hWnd;
							return FALSE;
						}
					}
				}
				return TRUE;//continue
			}, (LPARAM)m_hWnd);
			if(s_hFirstWindow){
				/*
				 * 1. set property {filename, my process id} to subject window
				 * 2. request subject window to find property containing my process id
				 * 3. open file found in property
				 * 4. remove my propety
				 */
				DWORD dwID = GetCurrentProcessId();
				::SetPropW(s_hFirstWindow, fname.c_str(), (HANDLE)dwID);
				HRESULT hr = ::SendMessageW(s_hFirstWindow, WM_FILELIST_OPEN_BY_PROPNAME, dwID, 0);
				::RemovePropW(s_hFirstWindow, fname.c_str());
				if (SUCCEEDED(hr))return S_FALSE;
			}
		}

		//prevent duplicated open
		std::wstring strMutex = L"LF" + replace(toLower(fname), L'\\', L'/');

		auto hMutex = GetMultiOpenLockMutex(strMutex);
		if (hMutex) {
			//set title
			SetWindowTextW(UtilLoadString(IDR_MAINFRAME).c_str());
			EnableWindow(FALSE);

			//list content
			ARCLOG arcLog;
			try {
				m_TabClientWnd->OpenArchiveInTab(fname, strMutex, hMutex, arcLog);
			} catch (...) {
				//TODO
				ErrorMessage(arcLog.toString());
			}

			EnableWindow(TRUE);
			return S_OK;
		}else{
			//same file is already opened in another window; highlight existing window
			EnumWindows([](HWND hWnd, LPARAM lParam)->BOOL {
				auto className = UtilGetWindowClassName(hWnd);
				if (LHAFORGE_FILE_LIST_CLASS == className) {
					if (::GetPropW(hWnd, g_strPropIdentifier.c_str())) {
						HANDLE hProp = ::GetPropW(hWnd, (const wchar_t*)lParam);
						if (hProp) {
							::SendMessageW(hWnd, WM_LHAFORGE_FILELIST_ACTIVATE_FILE, (WPARAM)hProp, NULL);
							return FALSE;	//found it
						}
					}
				}
				return TRUE;	//continue
			}, (LPARAM)strMutex.c_str());
			return S_FALSE;
		}
	}
}

LRESULT CFileListFrame::OnOpenByPropName(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	DWORD dwID=wParam;
	static std::wstring fileToOpen;
	fileToOpen.clear();

	::EnumPropsExW(m_hWnd, [](HWND hWnd, LPTSTR lpszString, HANDLE hData, ULONG_PTR dwData)->BOOL {
		if (dwData != (ULONG_PTR)hData) {
			return TRUE;	//continue
		} else {
			fileToOpen = lpszString;
			return FALSE;
		}
	}, dwID);
	if(!fileToOpen.empty() && m_TabClientWnd->IsTabEnabled()){
		return OpenArchiveFile(fileToOpen,false);
	}else{
		return E_FAIL;
	}
}


HANDLE CFileListFrame::GetMultiOpenLockMutex(const std::wstring& strMutex)
{
	HANDLE hMutex=::CreateMutexW(nullptr, TRUE, strMutex.c_str());
	if(ERROR_ALREADY_EXISTS==GetLastError()){
		//already exists
		CloseHandle(hMutex);
		return nullptr;
	}else{
		return hMutex;
	}
}


LRESULT CFileListFrame::OnActivateFile(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	m_TabClientWnd->SetActivePage((HANDLE)wParam);
	ShowWindow(SW_RESTORE);
	SetForegroundWindow(m_hWnd);

	//flash
	FLASHWINFO fi = {};
	fi.dwFlags=FLASHW_ALL;
	fi.hwnd=m_hWnd;
	fi.cbSize=sizeof(fi);
	fi.uCount=3;
	FlashWindowEx(&fi);
	return 0;
}


void CFileListFrame::EnableEntryExtractOperationMenu(bool bActive)
{
	UINT menuList[] = {
		ID_MENUITEM_EXTRACT_SELECTED,
		ID_MENUITEM_EXTRACT_SELECTED_SAMEDIR,
		ID_MENUITEM_OPEN_ASSOCIATION,
		ID_MENUITEM_OPEN_ASSOCIATION_OVERWRITE,
		ID_MENUITEM_EXTRACT_TEMPORARY,
	};
	for (auto item : menuList) {
		UIEnable(item, bActive);
	}

	// open with program / sendto
	EnableSendTo_OpenAppMenu(bActive);
}

void CFileListFrame::OnConfigure(UINT uNotifyCode, int nID, HWND hWndCtl)
{
	CConfigDialog confdlg(mr_Config);
	if (IDOK == confdlg.DoModal()) {
		try {
			mr_Config.save();
		} catch (const LF_EXCEPTION& e) {
			ErrorMessage(e.what());
		}

		MenuCommand_UpdateUserAppCommands(m_ConfFLW);
		MenuCommand_MakeUserAppMenu(GetAdditionalMenuHandle(MENUTYPE::UserApp));
		m_TabClientWnd->UpdateFileListConfig(m_ConfFLW);
	} else {
		//reload
		try {
			mr_Config.load();
		} catch (const LF_EXCEPTION& e) {
			ErrorMessage(e.what());
		}
	}

	m_ConfFLW.load(mr_Config);
	m_compressArgs.load(mr_Config);
	//reload accelerator
	if (m_ConfFLW.general.ExitWithEscape) {
		if (m_AccelEx.IsNull())m_AccelEx.LoadAccelerators(IDR_ACCEL_EX);
	} else {
		m_AccelEx.DestroyObject();
	}

	MenuCommand_MakeSendToCommands();
	MenuCommand_MakeSendToMenu(GetAdditionalMenuHandle(MENUTYPE::SendTo));
	DrawMenuBar();
}

void CFileListFrame::OnSize(UINT uType, CSize)
{
	// to invoke WM_SIZE/WM_MOVE message handler of super class
	SetMsgHandled(false);

	//if is window is maximized/minimized, do not get size
	if (IsZoomed() || IsIconic())return;
	GetWindowRect(m_WindowRect);
}

void CFileListFrame::OnMove(const CPoint&)
{
	// to invoke WM_SIZE/WM_MOVE message handler of super class
	SetMsgHandled(false);

	//if is window is maximized/minimized, do not get size
	if (IsZoomed() || IsIconic())return;
	GetWindowRect(m_WindowRect);
}

void CFileListFrame::OnListViewStyle(UINT uNotifyCode,int nID,HWND hWndCtrl)
{
	DWORD dwStyle=0;

	switch(nID){
	case ID_MENUITEM_LISTVIEW_SMALLICON:
		dwStyle=LVS_SMALLICON;
		break;
	case ID_MENUITEM_LISTVIEW_LARGEICON:
		dwStyle=LVS_ICON;
		break;
	case ID_MENUITEM_LISTVIEW_LIST:
		dwStyle=LVS_LIST;
		break;
	case ID_MENUITEM_LISTVIEW_REPORT:
		dwStyle=LVS_REPORT;
		break;
	}

	m_TabClientWnd->SetListViewStyle(dwStyle);

	UISetCheck(ID_MENUITEM_LISTVIEW_SMALLICON, false);
	UISetCheck(ID_MENUITEM_LISTVIEW_LARGEICON, false);
	UISetCheck(ID_MENUITEM_LISTVIEW_REPORT, false);
	UISetCheck(ID_MENUITEM_LISTVIEW_LIST, false);
	UISetCheck(nID, true);
}


void CFileListFrame::UpdateUpDirButtonState()
{
	CFileListTabItem* pTab = m_TabClientWnd->GetCurrentTab();
	if (pTab) {
		if (pTab->Model.IsRoot()) {
			UIEnable(ID_MENUITEM_UPDIR, false);
		} else {
			UIEnable(ID_MENUITEM_UPDIR, true);
		}
	} else {
		UIEnable(ID_MENUITEM_UPDIR, false);
	}
}

void CFileListFrame::UpdateMenuState()
{
	bool bActive=m_TabClientWnd->GetActivePage()!=-1;
	bool bTabActive=m_TabClientWnd->IsTabEnabled();

	const int subjects[] = {
		ID_MENUITEM_CLOSETAB,
		ID_MENUITEM_EXTRACT_ARCHIVE,
		ID_MENUITEM_TEST_ARCHIVE,
		ID_MENUITEM_UPDIR,
		ID_MENUITEM_REFRESH,
		ID_MENUITEM_SELECT_ALL,
		ID_MENUITEM_CLEAR_TEMPORARY,
		ID_MENUITEM_EXTRACT_SELECTED,
		ID_MENUITEM_EXTRACT_SELECTED_SAMEDIR,
		ID_MENUITEM_DELETE_SELECTED,
		ID_MENUITEM_OPEN_ASSOCIATION,
		ID_MENUITEM_OPEN_ASSOCIATION_OVERWRITE,
		ID_MENUITEM_EXTRACT_TEMPORARY,
		ID_MENUITEM_FINDITEM,
		ID_MENUITEM_FINDITEM_END,
		ID_MENUITEM_LISTVIEW_SMALLICON,
		ID_MENUITEM_LISTVIEW_LARGEICON,
		ID_MENUITEM_LISTVIEW_REPORT,
		ID_MENUITEM_LISTVIEW_LIST,
		ID_MENUITEM_SHOW_COLUMNHEADER_MENU,
		ID_MENUITEM_LISTMODE_TREE,
		ID_MENUITEM_LISTMODE_FLAT,
		ID_MENUITEM_LISTMODE_FLAT_FILESONLY,
		ID_MENUITEM_SORT_FILENAME,
		ID_MENUITEM_SORT_FULLPATH,
		ID_MENUITEM_SORT_ORIGINALSIZE,
		ID_MENUITEM_SORT_TYPENAME,
		ID_MENUITEM_SORT_FILETIME,
		ID_MENUITEM_SORT_ATTRIBUTE,
		ID_MENUITEM_SORT_COMPRESSEDSIZE,
		ID_MENUITEM_SORT_METHOD,
		ID_MENUITEM_SORT_RATIO,
	};

	for (auto id : subjects) {
		UIEnable(id, bActive);
	}

	//tab menu
	UIEnable(ID_MENUITEM_NEXTTAB,bActive && bTabActive);
	UIEnable(ID_MENUITEM_PREVTAB,bActive && bTabActive);
	UIEnable(ID_MENUITEM_ADD_FILE,bActive && bTabActive);
	UIEnable(ID_MENUITEM_ADD_DIRECTORY,bActive && bTabActive);

	// open with program / sendto
	EnableSendTo_OpenAppMenu(bActive);
}

void CFileListFrame::UpdateWindowTitle()
{
	CFileListTabItem* pTab=m_TabClientWnd->GetCurrentTab();
	if(pTab){
		//set filename to title
		std::wstring title;
		if(pTab->Model.IsArchiveEncrypted()){
			//encrypted archive
			title = Format(
				L"[%s] %s - %s",
				UtilLoadString(IDS_ENCRYPTED_ARCHIVE).c_str(),
				pTab->Model.GetArchiveFileName().c_str(),
				UtilLoadString(IDR_MAINFRAME).c_str());
		}else{
			//standard archive
			title = Format(
				L"%s - %s",
				pTab->Model.GetArchiveFileName().c_str(),
				UtilLoadString(IDR_MAINFRAME).c_str());
		}
		SetWindowTextW(title.c_str());
	}else{
		SetWindowTextW(UtilLoadString(IDR_MAINFRAME).c_str());
	}
}

void CFileListFrame::UpdateStatusBar()
{
	CFileListTabItem* pTab=m_TabClientWnd->GetCurrentTab();
	if(pTab){
		// file selection
		auto text = Format(UtilLoadString(IDS_PANE_ITEMCOUNT),
			pTab->ListView.GetItemCount(),
			pTab->ListView.GetSelectedCount());
		m_StatusBar.SetPaneText(IDS_PANE_ITEMCOUNT_INITIAL, text.c_str());
	}
}

LRESULT CFileListFrame::OnFileListWndStateChanged(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	OnFileListUpdated(uMsg,wParam,lParam,bHandled);

	CFileListTabItem* pTab=m_TabClientWnd->GetCurrentTab();
	if(pTab && pTab->Model.IsOK()){
		bool bFileListActive=(::GetFocus()==pTab->ListView);

		int SelCount=pTab->ListView.GetSelectedCount();
		bool bSelected=SelCount>0;

		//update menu
		EnableEntryExtractOperationMenu(bFileListActive && bSelected);
		EnableEntryDeleteOperationMenu(bFileListActive && pTab->Model.IsModifySupported() && bSelected);
		EnableAddItemsMenu(pTab->Model.IsModifySupported());

		//update status
		auto text = Format(UtilLoadString(IDS_PANE_ITEMCOUNT),
			pTab->ListView.GetItemCount(),
			SelCount);
		m_StatusBar.SetPaneText(IDS_PANE_ITEMCOUNT_INITIAL, text.c_str());
	}else{
		EnableEntryExtractOperationMenu(false);
		EnableEntryDeleteOperationMenu(false);
		EnableAddItemsMenu(false);

		m_StatusBar.SetPaneText(IDS_PANE_ITEMCOUNT_INITIAL, L"");
	}
	return 0;
}

//set focus between tree/file list view
void CFileListFrame::OnToggleFocus(UINT,int,HWND)
{
	CFileListTabItem* pTab=m_TabClientWnd->GetCurrentTab();
	if(pTab){
		pTab->Splitter.ActivateNextPane();
	}
}

void CFileListFrame::OnOpenArchive(UINT uNotifyCode, int nID, HWND hWndCtrl)
{
	const COMDLG_FILTERSPEC filter[] = {
		{ L"All Files", L"*.*" },
	};

	CLFShellFileOpenDialog dlg(nullptr, FOS_FORCEFILESYSTEM | FOS_FILEMUSTEXIST | FOS_PATHMUSTEXIST | FOS_ALLOWMULTISELECT, nullptr, filter, COUNTOF(filter));
	if (IDCANCEL == dlg.DoModal()) {	//cancel
		return;
	}
	auto files = dlg.GetMultipleFiles();

	for (const auto &file : files) {
		HRESULT hr = OpenArchiveFile(file, false);
		if (E_ABORT == hr)break;
	}
}

void CFileListFrame::OnCloseTab(UINT uNotifyCode,int nID,HWND hWndCtrl)
{
	m_TabClientWnd->CloseCurrentTab();
	if(!m_TabClientWnd->IsTabEnabled()){
		DestroyWindow();
	}
}

void CFileListFrame::OnNextTab(UINT uNotifyCode,int nID,HWND hWndCtrl)
{
	int size=m_TabClientWnd->GetPageCount();
	if(m_TabClientWnd && size>0){
		int nActive=m_TabClientWnd->GetActivePage();
		if(ID_MENUITEM_NEXTTAB==nID){
			m_TabClientWnd->SetActivePage((nActive+1)%size);
		}else{
			m_TabClientWnd->SetActivePage((nActive+size-1)%size);
		}
	}
}

LRESULT CFileListFrame::OnMouseWheel(UINT uCode,short delta,CPoint&)
{
	int size=m_TabClientWnd->GetPageCount();
	if(m_TabClientWnd && size>0 && uCode & MK_CONTROL){
		int step=-delta/WHEEL_DELTA;
		while(step<-size)step+=size;

		int nActive=m_TabClientWnd->GetActivePage();
		m_TabClientWnd->SetActivePage((nActive+size+step)%size);
	}else{
		SetMsgHandled(FALSE);
	}
	return 0;
}


void CFileListFrame::EnableDropTarget(bool bEnable)
{
	if(bEnable){
		//enable drop
		::RegisterDragDrop(m_hWnd, &m_DropTarget);
	}else{
		//disable drop
		::RevokeDragDrop(m_hWnd);
	}
}

// IDropCommunicator
HRESULT CFileListFrame::DragOver(IDataObject *lpDataObject,POINTL &pt,DWORD &dwEffect)
{
	//drop on the frame window: open as an archive
	if(!m_DropTarget.QueryFormat(CF_HDROP)){
		//not a file; reject
		dwEffect = DROPEFFECT_NONE;
	}else{
		dwEffect = DROPEFFECT_COPY;// : DROPEFFECT_NONE;
		auto[hr, files] = m_DropTarget.GetDroppedFiles(lpDataObject);
		//---if it contains directory, then reject
		if(S_OK==hr){
			for (const auto &file : files) {
				if(std::filesystem::is_directory(file)){
					dwEffect = DROPEFFECT_NONE;
					break;
				}
			}
		}
	}
	return S_OK;
}

//dropped
HRESULT CFileListFrame::Drop(IDataObject *lpDataObject,POINTL &pt,DWORD &dwEffect)
{
	auto[hr, files] = m_DropTarget.GetDroppedFiles(lpDataObject);
	if(S_OK==hr){
		dwEffect = DROPEFFECT_COPY;

		//drop on the frame window: open as an archive
		for (const auto &file : files) {
			if (E_ABORT == OpenArchiveFile(file, false)) {
				break;
			}
		}

		return S_OK;
	}else{
		//reject
		dwEffect = DROPEFFECT_NONE;
		return S_FALSE;
	}
}


//----------------
HWND CFileListFrame::CreateToolBarCtrl(HWND hWndParent, UINT nResourceID,HIMAGELIST hImageList)
{
	DWORD dwStyle = ATL_SIMPLE_TOOLBAR_PANE_STYLE;
	UINT nID = ATL_IDW_TOOLBAR;
	HINSTANCE hInst = ModuleHelper::GetResourceInstance();
	HRSRC hRsrc = ::FindResource(hInst, MAKEINTRESOURCE(nResourceID), RT_TOOLBAR);
	if (hRsrc == NULL)return NULL;

	HGLOBAL hGlobal = ::LoadResource(hInst, hRsrc);
	if (hGlobal == NULL)return NULL;

	_AtlToolBarData* pData = (_AtlToolBarData*)::LockResource(hGlobal);
	if (pData == NULL)return NULL;
	ATLASSERT(pData->wVersion == 1);

	WORD* pItems = pData->items();
	int nItems = pData->wItemCount;
	CTempBuffer<TBBUTTON, _WTL_STACK_ALLOC_THRESHOLD> buff;
	TBBUTTON* pTBBtn = buff.Allocate(nItems);
	ATLASSERT(pTBBtn != NULL);
	if(pTBBtn == NULL)return NULL;

	const int cxSeparator = 8;

	int nBmp = 0;
	for(int i = 0; i < pData->wItemCount; i++){
		if(pItems[i] != 0){
			pTBBtn[i].iBitmap = nBmp++;
			pTBBtn[i].idCommand = pItems[i];
			pTBBtn[i].fsState = TBSTATE_ENABLED;
			pTBBtn[i].fsStyle = TBSTYLE_BUTTON;
			pTBBtn[i].dwData = 0;
			pTBBtn[i].iString = 0;
		}else{
			pTBBtn[i].iBitmap = cxSeparator;
			pTBBtn[i].idCommand = 0;
			pTBBtn[i].fsState = 0;
			pTBBtn[i].fsStyle = TBSTYLE_SEP;
			pTBBtn[i].dwData = 0;
			pTBBtn[i].iString = 0;
		}
	}

	HWND hWnd = ::CreateWindowEx(0, TOOLBARCLASSNAME, NULL, dwStyle, 0, 0, 100, 100, hWndParent, (HMENU)LongToHandle(nID), ModuleHelper::GetModuleInstance(), NULL);
	if(hWnd == NULL){
		ATLASSERT(FALSE);
		return NULL;
	}

	::SendMessage(hWnd, TB_BUTTONSTRUCTSIZE, sizeof(TBBUTTON), 0L);

	// check if font is taller than our bitmaps
	CFontHandle font = (HFONT)::SendMessage(hWnd, WM_GETFONT, 0, 0L);
	if(font.IsNull())
		font = AtlGetDefaultGuiFont();
	LOGFONT lf = { 0 };
	font.GetLogFont(lf);
	WORD cyFontHeight = (WORD)abs(lf.lfHeight);

	WORD bitsPerPixel = AtlGetBitmapResourceBitsPerPixel(nResourceID);
	if(hImageList || bitsPerPixel > 4){
		COLORREF crMask = CLR_DEFAULT;
		if(bitsPerPixel == 32){
			// 32-bit color bitmap with alpha channel (valid for Windows XP and later)
			crMask = CLR_NONE;
		}
		if(!hImageList){
			hImageList = ImageList_LoadImage(ModuleHelper::GetResourceInstance(), MAKEINTRESOURCE(nResourceID), pData->wWidth, 1, crMask, IMAGE_BITMAP, LR_CREATEDIBSECTION | LR_DEFAULTSIZE);
		}
		ATLASSERT(hImageList != NULL);
		::SendMessage(hWnd, TB_SETIMAGELIST, 0, (LPARAM)hImageList);
	}else{
		TBADDBITMAP tbab = { 0 };
		tbab.hInst = hInst;
		tbab.nID = nResourceID;
		::SendMessage(hWnd, TB_ADDBITMAP, nBmp, (LPARAM)&tbab);
	}

	::SendMessage(hWnd, TB_ADDBUTTONS, nItems, (LPARAM)pTBBtn);
	::SendMessage(hWnd, TB_SETBITMAPSIZE, 0, MAKELONG(pData->wWidth, std::max(pData->wHeight, cyFontHeight)));
	const int cxyButtonMargin = 7;
	::SendMessage(hWnd, TB_SETBUTTONSIZE, 0, MAKELONG(pData->wWidth + cxyButtonMargin, std::max(pData->wHeight, cyFontHeight) + cxyButtonMargin));

	return hWnd;
}


