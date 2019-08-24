/*
 * Copyright (c) 2005-, Claybird
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
#include "FileListFrame.h"
#include "../ConfigCode/configwnd.h"
#include "../ConfigCode/ConfigManager.h"
#include "../ConfigCode/ConfigFileListWindow.h"
#include "../resource.h"
#include "../Dialogs/LFFolderDialog.h"
#include "../Dialogs/LogDialog.h"
#include "../Utilities/OSUtil.h"
#include "../Utilities/StringUtil.h"
#include "../CommonUtil.h"

HWND g_hFirstWindow=NULL;
CString g_FileToOpen;


CString CFileListFrame::ms_strPropString(CString(MAKEINTRESOURCE(IDS_MESSAGE_CAPTION))+CString(MAKEINTRESOURCE(IDS_LHAFORGE_VERSION_STRING)));

CFileListFrame::CFileListFrame(CConfigManager &conf):
	mr_Config(conf),
	m_TabClientWnd(conf,*this),
	m_DropTarget(this)
{
}


BOOL CFileListFrame::PreTranslateMessage(MSG* pMsg)
{
	if(CFrameWindowImpl<CFileListFrame>::PreTranslateMessage(pMsg)){
		return TRUE;
	}
	//�ǉ��̃A�N�Z�����[�^
	if(!m_AccelEx.IsNull()&&m_AccelEx.TranslateAccelerator(m_hWnd, pMsg)){
		return TRUE;
	}
	if(m_TabClientWnd.PreTranslateMessage(pMsg))return TRUE;
	return FALSE;
}

LRESULT CFileListFrame::OnCreate(LPCREATESTRUCT lpcs)
{
	CConfigFileListWindow ConfFLW;
	ConfFLW.load(mr_Config);
//========================================
//      �t���[���E�B���h�E�̏�����
//========================================
	//�E�B���h�E�v���p�e�B�̐ݒ�:LhaForge�E�B���h�E�ł��鎖������
	::SetProp(m_hWnd,ms_strPropString,m_hWnd);

	//�E�B���h�E�̃T�C�Y�̐ݒ�
	if(ConfFLW.StoreSetting){
		if(ConfFLW.StoreWindowPosition){	//�E�B���h�E�ʒu�𕜌�����ꍇ
			MoveWindow(ConfFLW.WindowPos_x,ConfFLW.WindowPos_y,ConfFLW.Width,ConfFLW.Height);
		}else{
			CRect Rect;
			GetWindowRect(Rect);
			MoveWindow(Rect.left,Rect.top,ConfFLW.Width,ConfFLW.Height);
		}
	}else if(ConfFLW.StoreWindowPosition){	//�E�B���h�E�ʒu������������ꍇ
		CRect Rect;
		GetWindowRect(Rect);
		MoveWindow(ConfFLW.WindowPos_x,ConfFLW.WindowPos_y,Rect.Width(),Rect.Height());
	}
	//�E�B���h�E�T�C�Y�擾
	GetWindowRect(m_WindowRect);

	// �傫���A�C�R���ݒ�
	HICON hIcon = AtlLoadIconImage(IDI_APP, LR_DEFAULTCOLOR,::GetSystemMetrics(SM_CXICON),::GetSystemMetrics(SM_CYICON));
	SetIcon(hIcon, TRUE);
	// �������A�C�R���ݒ�
	HICON hIconSmall = AtlLoadIconImage(IDI_APP, LR_DEFAULTCOLOR,::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON));
	SetIcon(hIconSmall, FALSE);

	if(ConfFLW.ShowToolbar){
		// ���o�[���쐬
		CreateSimpleReBar(ATL_SIMPLE_REBAR_NOBORDER_STYLE);
		// �c�[���o�[���쐬���ăo���h�ɒǉ�
		HIMAGELIST hImageList=NULL;
		if(!ConfFLW.strCustomToolbarImage.IsEmpty()){
			//�J�X�^���c�[���o�[
			hImageList = ImageList_LoadImage(NULL, ConfFLW.strCustomToolbarImage, 0, 1, CLR_DEFAULT, IMAGE_BITMAP, LR_CREATEDIBSECTION | LR_DEFAULTSIZE|LR_LOADFROMFILE);
		}
		HWND hWndToolBar=CreateToolBarCtrl(m_hWnd,IDR_MAINFRAME,hImageList);//CreateSimpleToolBarCtrl(m_hWnd,IDR_MAINFRAME, FALSE, ATL_SIMPLE_TOOLBAR_PANE_STYLE);
		AddSimpleReBarBand(hWndToolBar);
		UIAddToolBar(hWndToolBar);
		SizeSimpleReBarBands();
	}

	// �X�e�[�^�X�o�[���쐬
	m_hWndStatusBar=m_StatusBar.Create(m_hWnd);
	UIAddStatusBar(m_hWndStatusBar);
	int nPanes[] = {ID_DEFAULT_PANE, IDS_PANE_ITEMCOUNT_INITIAL,IDS_PANE_DLL_NAME_INITIAL};
	m_StatusBar.SetPanes(nPanes, COUNTOF(nPanes));
	{
		CString Text;
		Text.Format(IDS_PANE_ITEMCOUNT,0,0);
		m_StatusBar.SetPaneText(IDS_PANE_ITEMCOUNT_INITIAL,Text);
	}

	//===�֘A�Â��ŊJ������/���ۂ̐ݒ�
	SetOpenAssocLimitation(ConfFLW);

//========================================
//      �^�u�R���g���[���̏�����
//========================================
	m_TabClientWnd.Create(m_hWnd,rcDefault,NULL,WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN );
	m_TabClientWnd.addEventListener(m_hWnd);

	//�^�u���g��Ȃ��Ȃ��\����
	if(ConfFLW.DisableTab)m_TabClientWnd.ShowTabCtrl(false);

	//---------
	//���X�g�r���[�X�^�C���I��p���j���[�o�[�̃��W�I�`�F�b�N��L���ɂ���
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

	//���X�g�r���[�X�^�C���̐ݒ�
	if(ConfFLW.StoreSetting){
		//���݂̕\���ݒ�̃��j���[�Ƀ`�F�b�N��t����
		switch(ConfFLW.ListStyle){
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
		//���݂̕\�����[�h�Ƀ`�F�b�N������
		switch(ConfFLW.FileListMode){
		case FILELIST_TREE:
			UISetCheck(ID_MENUITEM_LISTMODE_TREE, TRUE);
			break;
		case FILELIST_FLAT:
			UISetCheck(ID_MENUITEM_LISTMODE_FLAT, TRUE);
			break;
		case FILELIST_FLAT_FILESONLY:
			UISetCheck(ID_MENUITEM_LISTMODE_FLAT_FILESONLY, TRUE);
			break;
		default:
			ASSERT(!"Error");
			break;
		}
	}else{
		UISetCheck(ID_MENUITEM_LISTVIEW_LARGEICON, TRUE);
		UISetCheck(ID_MENUITEM_LISTMODE_TREE, TRUE);
	}

	m_hWndClient = m_TabClientWnd;
	UpdateLayout();

//========================================
//      ���b�Z�[�W�n���h���̐ݒ�
//========================================
	// ���b�Z�[�W���[�v�Ƀ��b�Z�[�W�t�B���^�ƃA�C�h���n���h����ǉ�
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	pLoop->AddMessageFilter(this);
	pLoop->AddIdleHandler(this);

//========================================
//      �ǉ��̃L�[�{�[�h�A�N�Z�����[�^
//========================================
	if(ConfFLW.ExitWithEscape){
		m_AccelEx.LoadAccelerators(IDR_ACCEL_EX);
	}

	//���j���[�X�V
	EnableEntryExtractOperationMenu(false);
	EnableEntryDeleteOperationMenu(false);
	EnableAddItemsMenu(false);

//========================================
//    �t�@�C���ꗗ�E�B���h�E�̃R�}���h
//========================================
	MenuCommand_MakeSendToCommands();
	MenuCommand_UpdateUserAppCommands(ConfFLW);

	MenuCommand_MakeUserAppMenu(GetUserAppMenuHandle());
	MenuCommand_MakeSendToMenu(GetSendToMenuHandle());
	DrawMenuBar();

//==============================
// �E�B���h�E���A�N�e�B�u�ɂ���
//==============================
	UtilSetAbsoluteForegroundWindow(m_hWnd);
	UpdateLayout();

	//DnD�ɂ��t�@�C���{�����\��
	EnableDropTarget(true);
	return 0;
}

HMENU CFileListFrame::GetUserAppMenuHandle()
{
	CMenuHandle cMenu=GetMenu();
	CMenuHandle cSubMenu=cMenu.GetSubMenu(1);	//TODO:�}�W�b�N�i���o�[
	int MenuCount=cSubMenu.GetMenuItemCount();
	int iIndex=-1;
	for(int i=0;i<=MenuCount;i++){
		if(-1==cSubMenu.GetMenuItemID(i)){	//�|�b�v�A�b�v�̐e
			iIndex=i;
			break;
		}
	}
	ASSERT(-1!=iIndex);
	if(-1!=iIndex){
		return cSubMenu.GetSubMenu(iIndex);
	}else return NULL;
}

HMENU CFileListFrame::GetSendToMenuHandle()
{
	CMenuHandle cMenu=GetMenu();
	CMenuHandle cSubMenu=cMenu.GetSubMenu(1);	//TODO:�}�W�b�N�i���o�[
	int MenuCount=cSubMenu.GetMenuItemCount();
	int iIndex=-1;
	for(int i=0;i<=MenuCount;i++){
		if(-1==cSubMenu.GetMenuItemID(i)){	//�|�b�v�A�b�v�̐e
			iIndex=i;
			break;
		}
	}
	ASSERT(-1!=iIndex);
	if(-1!=iIndex){
		return cSubMenu.GetSubMenu(iIndex+1);
	}else return NULL;
}


LRESULT CFileListFrame::OnDestroy(UINT, WPARAM, LPARAM, BOOL& bHandled)
{
	//�E�B���h�E�v���p�e�B�̉���
	RemoveProp(m_hWnd,ms_strPropString);

	CConfigFileListWindow ConfFLW;
	ConfFLW.load(mr_Config);

	bool bSave=false;
	//�E�B���h�E�ݒ�̕ۑ�
	if(ConfFLW.StoreSetting){
		//�E�B���h�E�T�C�Y
		ConfFLW.Width=m_WindowRect.Width();
		ConfFLW.Height=m_WindowRect.Height();

		m_TabClientWnd.StoreSettings(ConfFLW);

		if(ConfFLW.StoreWindowPosition){	//�E�B���h�E�ʒu��ۑ�
			ConfFLW.WindowPos_x=m_WindowRect.left;
			ConfFLW.WindowPos_y=m_WindowRect.top;
		}

		ConfFLW.store(mr_Config);
		bSave=true;
	}
	if(ConfFLW.StoreWindowPosition){	//�E�B���h�E�ʒu�����ۑ�
		ConfFLW.WindowPos_x=m_WindowRect.left;
		ConfFLW.WindowPos_y=m_WindowRect.top;
		ConfFLW.store(mr_Config);
		bSave=true;
	}
	if(bSave){
		CString strErr;
		if(!mr_Config.SaveConfig(strErr)){
			ErrorMessage(strErr);
		}
	}


	if(m_TabClientWnd.IsWindow())m_TabClientWnd.DestroyWindow();

	CMessageLoop* pLoop = _Module.GetMessageLoop();
	pLoop->RemoveMessageFilter(this);
	pLoop->RemoveIdleHandler(this);

	PostQuitMessage(0);
	//bHandled=false;
	return 0;
}


HRESULT CFileListFrame::OpenArchiveFile(LPCTSTR fname,DLL_ID idForceDLL,bool bAllowRelayOpen)
{
	if(m_TabClientWnd.GetPageCount()>0 && !m_TabClientWnd.IsTabEnabled()){
		//�^�u�@�\�������Ȃ̂ŁA�������g���d���N�����\��������
		CString strParam(_T("/l "));
			 if(idForceDLL==DLL_ID_XACRETT)	strParam+=_T("/! ");
		else if(idForceDLL==DLL_ID_B2E)		strParam+=_T("/b2e ");

		CPath filePath=fname;
		filePath.QuoteSpaces();
		strParam+=(LPCTSTR)filePath;
		int ret=(int)ShellExecute(NULL,NULL,UtilGetModulePath(),strParam,NULL,SW_RESTORE);
		if(ret<=32){
			return E_FAIL;
		}else return S_OK;
	}else{
		CConfigFileListWindow ConfFLW;
		ConfFLW.load(mr_Config);

		//�E�B���h�E����ɕۂ�?
		if(bAllowRelayOpen && ConfFLW.KeepSingleInstance){
			g_hFirstWindow=NULL;
			EnumWindows(EnumFirstFileListWindowProc,(LPARAM)m_hWnd);
			if(g_hFirstWindow){
				/*
				 * 1.����̃E�B���h�E��{�t�@�C����,�����̃v���Z�XID}�Ńv���p�e�B��ݒ�
				 * 2.����̃E�B���h�E�Ɏ����̃v���Z�XID�����v���p�e�B������������
				 * 3.�������v���p�e�B���t�@�C�����Ƃ��ĊJ��
				 * 4.�v���p�e�B�폜
				 */
				DWORD dwID=GetCurrentProcessId();
				::SetProp(g_hFirstWindow,fname,(HANDLE)dwID);
				HRESULT hr=::SendMessage(g_hFirstWindow,WM_FILELIST_OPEN_BY_PROPNAME,dwID,idForceDLL);
				::RemoveProp(g_hFirstWindow,fname);
				if(SUCCEEDED(hr))return S_FALSE;
				//else return hr;	���ۂ��ꂽ�̂Ŏ����ŊJ��
			}
		}//else

		//�d���I�[�v���h�~
		CString strMutex(fname);
		strMutex.MakeLower();	//�������ɓ���
		strMutex.Replace(_T('\\'),_T('/'));
		strMutex=_T("LF")+strMutex;

		HANDLE hMutex=GetMultiOpenLockMutex(strMutex);
		if(!hMutex){
			//���łɓ����t�@�C�����{������Ă����ꍇ
			//���̃t�@�C����\�����Ă���E�B���h�E����������
			EnumWindows(EnumFileListWindowProc,(LPARAM)(LPCTSTR)strMutex);
			return S_FALSE;
		}

		CString Title(MAKEINTRESOURCE(IDR_MAINFRAME));
		SetWindowText(Title);
		EnableWindow(FALSE);

		//�t�@�C���ꗗ�쐬
		CString strErr;
		HRESULT hr=m_TabClientWnd.OpenArchiveInTab(fname,idForceDLL,ConfFLW,strMutex,hMutex,strErr);

		EnableWindow(TRUE);
		//SetForegroundWindow(m_hWnd);

		if(FAILED(hr)){
			ErrorMessage(strErr);
			//EnableAll(false);
		}
		return hr;
	}
}

//�E�B���h�E�v���p�e�B�̗�
BOOL CALLBACK CFileListFrame::EnumPropProc(HWND hWnd,LPTSTR lpszString,HANDLE hData,ULONG_PTR dwData)
{
	if(dwData!=(ULONG_PTR)hData)return TRUE;
	else{
		g_FileToOpen=lpszString;
		return FALSE;
	}
}

LRESULT CFileListFrame::OnOpenByPropName(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	DWORD dwID=wParam;
	DLL_ID idForceDLL=(DLL_ID)lParam;
	g_FileToOpen=_T("");
	EnumPropsEx(m_hWnd,EnumPropProc,dwID);
	if(!g_FileToOpen.IsEmpty() && m_TabClientWnd.IsTabEnabled()){
		return OpenArchiveFile(g_FileToOpen,idForceDLL,false);
	}else{
		return E_FAIL;
	}
}


HANDLE CFileListFrame::GetMultiOpenLockMutex(LPCTSTR lpszMutex)
{
	//�~���[�e�b�N�X�쐬
	HANDLE hMutex=::CreateMutex(NULL, TRUE, lpszMutex);
	if(ERROR_ALREADY_EXISTS==GetLastError()){
		//���łɉ{������Ă���ꍇ
		CloseHandle(hMutex);
		return NULL;
	}else{
		return hMutex;
	}
}


//�t�@�C���ꗗ�E�B���h�E�̗�
BOOL CALLBACK CFileListFrame::EnumFileListWindowProc(HWND hWnd,LPARAM lParam)
{
	TCHAR Buffer[_MAX_PATH+1];
	FILL_ZERO(Buffer);
	GetClassName(hWnd,Buffer,_MAX_PATH);
	if(0!=_tcsncmp(LHAFORGE_FILE_LIST_CLASS,Buffer,_MAX_PATH)){
		return TRUE;
	}

	if(GetProp(hWnd,ms_strPropString)){
		HANDLE hProp=GetProp(hWnd,(LPCTSTR)lParam);
		if(hProp){
			::SendMessage(hWnd,WM_LHAFORGE_FILELIST_ACTIVATE_FILE,(WPARAM)hProp,NULL);
			return FALSE;
		}
	}
	return TRUE;
}

//�ŏ��̃t�@�C���ꗗ�E�B���h�E�̗�
BOOL CALLBACK CFileListFrame::EnumFirstFileListWindowProc(HWND hWnd,LPARAM lParam)
{
	if(hWnd!=(HWND)lParam){
		TCHAR Buffer[_MAX_PATH+1];
		FILL_ZERO(Buffer);
		GetClassName(hWnd,Buffer,_MAX_PATH);
		if(0!=_tcsncmp(LHAFORGE_FILE_LIST_CLASS,Buffer,_MAX_PATH)){
			return TRUE;
		}

		//�ŏ��Ɍ�����LhaForge�̃t�@�C���ꗗ�E�B���h�E���L�^����
		if(!g_hFirstWindow){
			g_hFirstWindow=hWnd;
			return FALSE;
		}
	}
	return TRUE;
}

LRESULT CFileListFrame::OnActivateFile(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	m_TabClientWnd.SetCurrentTab((HANDLE)wParam);
	ShowWindow(SW_RESTORE);
	UtilSetAbsoluteForegroundWindow(m_hWnd);

	//�_��
	FLASHWINFO fi;
	FILL_ZERO(fi);
	fi.dwFlags=FLASHW_ALL;
	fi.hwnd=m_hWnd;
	fi.cbSize=sizeof(fi);
	fi.uCount=3;
	FlashWindowEx(&fi);
	return 0;
}


void CFileListFrame::EnableEntryExtractOperationMenu(bool bActive)
{
	// �t�@�C�����I������Ă��Ȃ��Ɩ����ȃ��j���[
	UINT menuList[]={
		ID_MENUITEM_EXTRACT_SELECTED,
		ID_MENUITEM_EXTRACT_SELECTED_SAMEDIR,
		ID_MENUITEM_OPEN_ASSOCIATION,
		ID_MENUITEM_OPEN_ASSOCIATION_OVERWRITE,
		ID_MENUITEM_EXTRACT_TEMPORARY,
	};
	for(size_t i=0;i<COUNTOF(menuList);i++){
		UIEnable(menuList[i],bActive);
	}

	//�v���O��������J��/����̃��j���[
	CMenuHandle cMenu[]={GetUserAppMenuHandle(),GetSendToMenuHandle()};
	for(int iMenu=0;iMenu<COUNTOF(cMenu);iMenu++){
		int size=cMenu[iMenu].GetMenuItemCount();
		for(int i=0;i<size;i++){
			cMenu[iMenu].EnableMenuItem(i,MF_BYPOSITION | (bActive ? MF_ENABLED : MF_GRAYED));
		}
	};
}

void CFileListFrame::EnableEntryDeleteOperationMenu(bool bActive)
{
	UIEnable(ID_MENUITEM_DELETE_SELECTED,bActive);
}

void CFileListFrame::EnableAddItemsMenu(bool bActive)
{
	UIEnable(ID_MENUITEM_ADD_FILE,bActive);
	UIEnable(ID_MENUITEM_ADD_DIRECTORY,bActive);
}

void CFileListFrame::OnCommandCloseWindow(UINT uNotifyCode, int nID, HWND hWndCtl)
{
	DestroyWindow();
}

void CFileListFrame::OnConfigure(UINT uNotifyCode, int nID, HWND hWndCtl)
{
	//mr_Config.SaveConfig();
	CString strErr;
	CConfigDialog confdlg(mr_Config);
	if(IDOK==confdlg.DoModal()){
		if(!mr_Config.SaveConfig(strErr)){
			ErrorMessage(strErr);
		}
		CConfigFileListWindow ConfFLW;
		ConfFLW.load(mr_Config);

		MenuCommand_UpdateUserAppCommands(ConfFLW);
		MenuCommand_MakeUserAppMenu(GetUserAppMenuHandle());
		//===�֘A�Â��ŊJ������/���ۂ̐ݒ�
		SetOpenAssocLimitation(ConfFLW);
		m_TabClientWnd.UpdateFileListConfig(ConfFLW);

		//�A�N�Z�����[�^�̓ǂݒ���
		if(ConfFLW.ExitWithEscape){
			if(m_AccelEx.IsNull())m_AccelEx.LoadAccelerators(IDR_ACCEL_EX);
		}else{
			m_AccelEx.DestroyObject();
		}
	}else{
		//�O�̂��ߍēǂݍ���
		if(!mr_Config.LoadConfig(strErr)){
			ErrorMessage(strErr);
		}
	}

	CArchiverDLLManager::GetInstance().UpdateDLLConfig();

	m_TabClientWnd.ReloadArchiverIfLost();
/*	else{	�ʂ�IDCANCEL�ł����[�h�������K�v�͂Ȃ��B�Ȃ��Ȃ�f�[�^�̓_�C�A���O���ŗ��܂�AConfig�\���̂ɓ��炸�̂Ă��Ă��邩��
		Config.LoadConfig(CONFIG_LOAD_ALL);
	}*/
	//�t�@�C���ꗗ�E�B���h�E�̃R�}���h
	MenuCommand_MakeSendToCommands();

	MenuCommand_MakeSendToMenu(GetSendToMenuHandle());
	DrawMenuBar();
}

void CFileListFrame::OnSize(UINT uType, CSize)
{
	// ���N���X��WM_SIZE���b�Z�[�W�n���h�����Ăяo������
	SetMsgHandled(false);

	if(0==uType){//0 (SIZE_RESTORED)�E�B���h�E���T�C�Y�ύX����܂����B�������ŏ����܂��͍ő剻�ł͂���܂���B
		//�ő剻/�ŏ�������Ă���Ƃ��ɂ̓E�B���h�E�T�C�Y�͎擾���Ȃ�
		if(IsZoomed()||IsIconic())return;

		GetWindowRect(m_WindowRect);
	}
}

void CFileListFrame::OnMove(const CPoint&)
{
	// ���N���X��WM_MOVE���b�Z�[�W�n���h�����Ăяo������
	SetMsgHandled(false);

	//�ő剻/�ŏ�������Ă���Ƃ��ɂ̓E�B���h�E�T�C�Y�͎擾���Ȃ�
	if(IsZoomed()||IsIconic())return;
	GetWindowRect(m_WindowRect);
}

void CFileListFrame::OnUpDir(UINT,int,HWND)
{
	CFileListTabItem* pTab=m_TabClientWnd.GetCurrentTab();
	if(pTab)pTab->Model.MoveUpDir();
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

	m_TabClientWnd.SetListViewStyle(dwStyle);

	UISetCheck(ID_MENUITEM_LISTVIEW_SMALLICON, false);
	UISetCheck(ID_MENUITEM_LISTVIEW_LARGEICON, false);
	UISetCheck(ID_MENUITEM_LISTVIEW_REPORT, false);
	UISetCheck(ID_MENUITEM_LISTVIEW_LIST, false);
	UISetCheck(nID, true);
}


void CFileListFrame::UpdateUpDirButtonState()
{
	//�u��ɏ��v�{�^���̗L��/����
	CFileListTabItem* pTab=m_TabClientWnd.GetCurrentTab();
	if(pTab){
		if(pTab->Model.IsRoot()){
			UIEnable(ID_MENUITEM_UPDIR,false);
		}else{
			UIEnable(ID_MENUITEM_UPDIR,true);
		}
	}else{
		UIEnable(ID_MENUITEM_UPDIR,false);
	}
}

void CFileListFrame::UpdateMenuState()
{
	bool bActive=m_TabClientWnd.GetActivePage()!=-1;
	bool bTabActive=m_TabClientWnd.IsTabEnabled();

	UIEnable(ID_MENUITEM_CLOSETAB,bActive);
	UIEnable(ID_MENUITEM_EXTRACT_ARCHIVE,bActive);
	UIEnable(ID_MENUITEM_TEST_ARCHIVE,bActive);
	UIEnable(ID_MENUITEM_UPDIR,bActive);
	UIEnable(ID_MENUITEM_REFRESH,bActive);
	UIEnable(ID_MENUITEM_SELECT_ALL,bActive);
	UIEnable(ID_MENUITEM_CLEAR_TEMPORARY,bActive);
	UIEnable(ID_MENUITEM_EXTRACT_SELECTED,bActive);
	UIEnable(ID_MENUITEM_EXTRACT_SELECTED_SAMEDIR,bActive);
	UIEnable(ID_MENUITEM_DELETE_SELECTED,bActive);
	UIEnable(ID_MENUITEM_OPEN_ASSOCIATION,bActive);
	UIEnable(ID_MENUITEM_OPEN_ASSOCIATION_OVERWRITE,bActive);
	UIEnable(ID_MENUITEM_EXTRACT_TEMPORARY,bActive);
	UIEnable(ID_MENUITEM_FINDITEM,bActive);
	UIEnable(ID_MENUITEM_FINDITEM_END,bActive);

	UIEnable(ID_MENUITEM_LISTVIEW_SMALLICON,bActive);
	UIEnable(ID_MENUITEM_LISTVIEW_LARGEICON,bActive);
	UIEnable(ID_MENUITEM_LISTVIEW_REPORT,bActive);
	UIEnable(ID_MENUITEM_LISTVIEW_LIST,bActive);

	UIEnable(ID_MENUITEM_SHOW_COLUMNHEADER_MENU,bActive);
	UIEnable(ID_MENUITEM_LISTMODE_TREE,bActive);
	UIEnable(ID_MENUITEM_LISTMODE_FLAT,bActive);
	UIEnable(ID_MENUITEM_LISTMODE_FLAT_FILESONLY,bActive);


	UIEnable(ID_MENUITEM_SORT_FILENAME,bActive);
	UIEnable(ID_MENUITEM_SORT_FULLPATH,bActive);
	UIEnable(ID_MENUITEM_SORT_ORIGINALSIZE,bActive);
	UIEnable(ID_MENUITEM_SORT_TYPENAME,bActive);
	UIEnable(ID_MENUITEM_SORT_FILETIME,bActive);
	UIEnable(ID_MENUITEM_SORT_ATTRIBUTE,bActive);
	UIEnable(ID_MENUITEM_SORT_COMPRESSEDSIZE,bActive);
	UIEnable(ID_MENUITEM_SORT_METHOD,bActive);
	UIEnable(ID_MENUITEM_SORT_RATIO,bActive);
	UIEnable(ID_MENUITEM_SORT_CRC,bActive);

	//�^�u�֘A���j���[
	UIEnable(ID_MENUITEM_NEXTTAB,bActive && bTabActive);
	UIEnable(ID_MENUITEM_PREVTAB,bActive && bTabActive);

	UIEnable(ID_MENUITEM_ADD_FILE,bActive && bTabActive);
	UIEnable(ID_MENUITEM_ADD_DIRECTORY,bActive && bTabActive);

	//�v���O��������J��/����̃��j���[
	CMenuHandle cMenu[]={GetUserAppMenuHandle(),GetSendToMenuHandle()};
	for(int iMenu=0;iMenu<COUNTOF(cMenu);iMenu++){
		int size=cMenu[iMenu].GetMenuItemCount();
		for(int i=0;i<size;i++){
			cMenu[iMenu].EnableMenuItem(i,MF_BYPOSITION | (bActive ? MF_ENABLED : MF_GRAYED));
		}
	};
}

void CFileListFrame::UpdateWindowTitle()
{
	CFileListTabItem* pTab=m_TabClientWnd.GetCurrentTab();
	if(pTab){
		//�E�B���h�E�^�C�g���Ƀt�@�C�����ݒ�
		CString Title;
		if(pTab->Model.IsArchiveEncrypted()){
			//�p�X���[�h�t���̏ꍇ
			Title.Format(_T("[%s] %s - %s"),CString(MAKEINTRESOURCE(IDS_ENCRYPTED_ARCHIVE)),pTab->Model.GetArchiveFileName(),CString(MAKEINTRESOURCE(IDR_MAINFRAME)));
		}else{
			//�ʏ�A�[�J�C�u
			Title.Format(_T("%s - %s"),pTab->Model.GetArchiveFileName(),CString(MAKEINTRESOURCE(IDR_MAINFRAME)));
		}
		SetWindowText(Title);
	}else{
		SetWindowText(CString(MAKEINTRESOURCE(IDR_MAINFRAME)));
	}
}

void CFileListFrame::UpdateStatusBar()
{
	CFileListTabItem* pTab=m_TabClientWnd.GetCurrentTab();
	if(pTab){
		CString Text;
		//---DLL���
		const CArchiverDLL *pDLL=pTab->Model.GetArchiver();
		if(pDLL){
			Text.Format(IDS_PANE_DLL_NAME,pDLL->GetName());
			m_StatusBar.SetPaneText(IDS_PANE_DLL_NAME_INITIAL,Text);
		}

		//---�t�@�C���I�����
		Text.Format(IDS_PANE_ITEMCOUNT,pTab->ListView.GetItemCount(),pTab->ListView.GetSelectedCount());
		m_StatusBar.SetPaneText(IDS_PANE_ITEMCOUNT_INITIAL,Text);
	}
}

LRESULT CFileListFrame::OnFileListModelChanged(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	UpdateWindowTitle();
	UpdateMenuState();
	UpdateStatusBar();
	UpdateUpDirButtonState();

	return 0;
}

LRESULT CFileListFrame::OnFileListArchiveLoaded(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	UpdateUpDirButtonState();
	return 0;
}

LRESULT CFileListFrame::OnFileListNewContent(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	UpdateUpDirButtonState();
	return 0;
}

LRESULT CFileListFrame::OnFileListUpdated(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	UpdateUpDirButtonState();
	//nothing to do
	return 0;
}

LRESULT CFileListFrame::OnFileListWndStateChanged(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	UpdateWindowTitle();
	UpdateStatusBar();
	UpdateMenuState();
	UpdateUpDirButtonState();

	CFileListTabItem* pTab=m_TabClientWnd.GetCurrentTab();
	if(pTab && pTab->Model.IsOK()){
		bool bFileListActive=(::GetFocus()==pTab->ListView);

		int SelCount=pTab->ListView.GetSelectedCount();
		bool bSelected=SelCount>0;

		//UI�X�V
		EnableEntryExtractOperationMenu(bFileListActive && pTab->Model.IsExtractEachSupported() && bSelected);
		EnableEntryDeleteOperationMenu(bFileListActive && pTab->Model.IsDeleteItemsSupported() && bSelected);
		EnableAddItemsMenu(pTab->Model.IsAddItemsSupported());

		//�X�e�[�^�X�o�[�X�V
		CString Text;
		Text.Format(IDS_PANE_ITEMCOUNT,pTab->ListView.GetItemCount(),SelCount);
		m_StatusBar.SetPaneText(IDS_PANE_ITEMCOUNT_INITIAL,Text);
	}else{
		//UI�X�V
		EnableEntryExtractOperationMenu(false);
		EnableEntryDeleteOperationMenu(false);
		EnableAddItemsMenu(false);

		//�X�e�[�^�X�o�[�X�V
		m_StatusBar.SetPaneText(IDS_PANE_ITEMCOUNT_INITIAL,_T(""));
	}
	return 0;
}

//���X�g�r���[�ƃc���[�r���[�Ńt�H�[�J�X�؂�ւ�
void CFileListFrame::OnToggleFocus(UINT,int,HWND)
{
	CFileListTabItem* pTab=m_TabClientWnd.GetCurrentTab();
	if(pTab){
		pTab->Splitter.ActivateNextPane();
	}
}

//�t�@�C�����X�g�X�V
void CFileListFrame::OnRefresh(UINT,int,HWND)
{
	ReopenArchiveFile(m_TabClientWnd.GetFileListMode());
}

//�t�@�C�����X�g�X�V
LRESULT CFileListFrame::OnRefresh(UINT, WPARAM, LPARAM, BOOL& bHandled)
{
	ReopenArchiveFile(m_TabClientWnd.GetFileListMode());
	return 0;
}


void CFileListFrame::ReopenArchiveFile(FILELISTMODE flMode)
{
	m_TabClientWnd.ReopenArchiveFile(flMode);
}

void CFileListFrame::OnListMode(UINT uNotifyCode,int nID,HWND hWndCtrl)
{
	FILELISTMODE newMode;
	switch(nID){
	case ID_MENUITEM_LISTMODE_TREE:
		newMode=FILELIST_TREE;
		break;
	case ID_MENUITEM_LISTMODE_FLAT:
		newMode=FILELIST_FLAT;
		break;
	case ID_MENUITEM_LISTMODE_FLAT_FILESONLY:
		newMode=FILELIST_FLAT_FILESONLY;
		break;
	}

	if(m_TabClientWnd.GetFileListMode()!=newMode){
		ReopenArchiveFile(newMode);
		UISetCheck(ID_MENUITEM_LISTMODE_TREE, false);
		UISetCheck(ID_MENUITEM_LISTMODE_FLAT, false);
		UISetCheck(ID_MENUITEM_LISTMODE_FLAT_FILESONLY, false);
		UISetCheck(nID, true);
	}
}

void CFileListFrame::OnOpenArchive(UINT uNotifyCode,int nID,HWND hWndCtrl)
{
	//�u�S�Ẵt�@�C���v�̃t�B���^���������
	CString strAnyFile(MAKEINTRESOURCE(IDS_FILTER_ANYFILE));
	std::vector<TCHAR> filter(strAnyFile.GetLength()+1+1);
	UtilMakeFilterString(strAnyFile,&filter[0],filter.size());
	//CFileDialog dlg(TRUE, NULL, NULL, OFN_NOCHANGEDIR|OFN_FILEMUSTEXIST|OFN_HIDEREADONLY,&filter[0]);
	CMultiFileDialog dlg(NULL, NULL, OFN_NOCHANGEDIR|OFN_DONTADDTORECENT|OFN_HIDEREADONLY|OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST|OFN_ALLOWMULTISELECT,&filter[0]);
	if(IDCANCEL==dlg.DoModal()){	//�L�����Z��
		return;
	}
	//�t�@�C�������o��
	CString tmp;
	if(dlg.GetFirstPathName(tmp)){
		do{
			HRESULT hr=OpenArchiveFile(tmp,DLL_ID_UNKNOWN,false);
			if(E_ABORT==hr)break;
		}while(dlg.GetNextPathName(tmp));
	}
}

void CFileListFrame::OnCloseTab(UINT uNotifyCode,int nID,HWND hWndCtrl)
{
	m_TabClientWnd.CloseCurrentTab();
	if(!m_TabClientWnd.IsTabEnabled()){
		DestroyWindow();
	}
}

void CFileListFrame::OnNextTab(UINT uNotifyCode,int nID,HWND hWndCtrl)
{
	int size=m_TabClientWnd.GetPageCount();
	if(m_TabClientWnd && size>0){
		int nActive=m_TabClientWnd.GetActivePage();
		if(ID_MENUITEM_NEXTTAB==nID){
			m_TabClientWnd.SetCurrentTab((nActive+1)%size);
		}else{
			m_TabClientWnd.SetCurrentTab((nActive+size-1)%size);
		}
	}
}

LRESULT CFileListFrame::OnMouseWheel(UINT uCode,short delta,CPoint&)
{
	int size=m_TabClientWnd.GetPageCount();
	if(m_TabClientWnd && size>0 && uCode & MK_CONTROL){
		int step=-delta/WHEEL_DELTA;
		while(step<-size)step+=size;

		int nActive=m_TabClientWnd.GetActivePage();
		m_TabClientWnd.SetCurrentTab((nActive+size+step)%size);
	}else{
		SetMsgHandled(FALSE);
	}
	return 0;
}


//�֘A�Â��ŊJ������/���ۂ̐ݒ�
void CFileListFrame::SetOpenAssocLimitation(const CConfigFileListWindow& ConfFLW)
{
	CFileListModel::SetOpenAssocExtAccept(ConfFLW.OpenAssoc.Accept);
	if(ConfFLW.DenyPathExt){
		//���ϐ��ō\�z
		std::map<stdString,stdString> envs;
		UtilGetEnvInfo(envs);
		for(std::map<stdString,stdString>::iterator ite=envs.begin();ite!=envs.end();++ite){
			CFileListModel::SetOpenAssocExtDeny((envs[_T("PATHEXT")]+_T(";")).c_str()+ConfFLW.OpenAssoc.Deny);
		}
	}else{
		CFileListModel::SetOpenAssocExtDeny(ConfFLW.OpenAssoc.Deny);
	}
}


void CFileListFrame::EnableDropTarget(bool bEnable)
{
	if(bEnable){
		//�h���b�v�󂯓���ݒ�
		::RegisterDragDrop(m_hWnd,&m_DropTarget);
	}else{
		//�h���b�v���󂯓���Ȃ�
		::RevokeDragDrop(m_hWnd);
	}
}

//---------------------------------------------------------
//    IDropCommunicator�̎���:�h���b�O&�h���b�v�ɂ��{��
//---------------------------------------------------------
HRESULT CFileListFrame::DragEnter(IDataObject *lpDataObject,POINTL &pt,DWORD &dwEffect)
{
	return DragOver(lpDataObject,pt,dwEffect);
}

HRESULT CFileListFrame::DragLeave()
{
	return S_OK;
}

HRESULT CFileListFrame::DragOver(IDataObject *lpDataObject,POINTL &pt,DWORD &dwEffect)
{
	//�t�H�[�}�b�g�ɑΉ���������������
	if(!m_DropTarget.QueryFormat(CF_HDROP)){	//�t�@�C����p
		//�t�@�C���ł͂Ȃ��̂ŋ���
		dwEffect = DROPEFFECT_NONE;
	}else{
		dwEffect = DROPEFFECT_COPY;// : DROPEFFECT_NONE;
		//�t�@�C���擾
		std::list<CString> fileList;
		//---�f�B���N�g�����܂܂�Ă����狑��
		if(S_OK==m_DropTarget.GetDroppedFiles(lpDataObject,fileList)){
			for(std::list<CString>::iterator ite=fileList.begin();ite!=fileList.end();++ite){
				if(PathIsDirectory(*ite)){
					dwEffect = DROPEFFECT_NONE;
					break;
				}
			}
		}
	}
	return S_OK;
}

//�t�@�C���̃h���b�v
HRESULT CFileListFrame::Drop(IDataObject *lpDataObject,POINTL &pt,DWORD &dwEffect)
{
	//�t�@�C���擾
	std::list<CString> fileList;
	if(S_OK==m_DropTarget.GetDroppedFiles(lpDataObject,fileList)){
		dwEffect = DROPEFFECT_COPY;

		//�J��
		for(std::list<CString>::iterator ite=fileList.begin();ite!=fileList.end();++ite){
			HRESULT hr=OpenArchiveFile(*ite,DLL_ID_UNKNOWN,false);
			if(E_ABORT==hr)break;
		}

		return S_OK;
	}else{
		//�󂯓���ł��Ȃ��`��
		dwEffect = DROPEFFECT_NONE;
		return S_FALSE;	//S_OK
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
	::SendMessage(hWnd, TB_SETBITMAPSIZE, 0, MAKELONG(pData->wWidth, max(pData->wHeight, cyFontHeight)));
	const int cxyButtonMargin = 7;
	::SendMessage(hWnd, TB_SETBUTTONSIZE, 0, MAKELONG(pData->wWidth + cxyButtonMargin, max(pData->wHeight, cyFontHeight) + cxyButtonMargin));

	return hWnd;
}


