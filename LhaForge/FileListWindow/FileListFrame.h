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

#pragma once
#include "../resource.h"
#include "../ArchiverManager.h"
#include "../ConfigCode/ConfigManager.h"
#include "MenuCommand.h"
#include "FileListTabClient.h"
#include "OLE/DropTarget.h"	//�h���b�v�󂯓���,IDropCommunicator

const LPCTSTR LHAFORGE_FILE_LIST_CLASS=_T("LhaForgeFileList");

#define FILELISTWINDOW_DEFAULT_WIDTH	760
#define FILELISTWINDOW_DEFAULT_HEIGHT	500

class CFileListFrame:
	public CFrameWindowImpl<CFileListFrame>,
	public CUpdateUI<CFileListFrame>,
	public CMessageFilter,
	public CIdleHandler,
	public IDropCommunicator//���O��DnD�C���^�[�t�F�C�X
{
protected:
	//bool m_bFirst;		//����\��
	//DLL_ID m_idForceDLL;	//�����g�p����DLL

	CConfigManager		&mr_Config;

	//----------------------
	// �E�B���h�E�֌W�����o
	//----------------------
	CAccelerator			m_AccelEx;		//�ǉ��̃E�B���h�E�A�N�Z�����[�^:[ESC]�ŃE�B���h�E�����A����S��
	CFileListTabClient		m_TabClientWnd;
	CMultiPaneStatusBarCtrl m_StatusBar;		//�X�e�[�^�X�o�[
	CRect					m_WindowRect;		// �E�B���h�E�T�C�Y

	static CString			ms_strPropString;	// SetProp�̎��ʖ�:LhaForge�̃E�B���h�E�ł��鎖������
protected:
	//---�h���b�v�󂯓���
	CDropTarget m_DropTarget;	//�h���b�v�󂯓���Ɏg��
	void EnableDropTarget(bool bEnable);
	//IDropCommunicator�̎���
	HRESULT DragEnter(IDataObject*,POINTL&,DWORD&);
	HRESULT DragLeave();
	HRESULT DragOver(IDataObject*,POINTL&,DWORD&);
	HRESULT Drop(IDataObject*,POINTL&,DWORD&);
protected:
	void ReopenArchiveFile(FILELISTMODE);

	void UpdateUpDirButtonState();
	void EnableEntryExtractOperationMenu(bool);
	void EnableEntryDeleteOperationMenu(bool);
	void EnableAddItemsMenu(bool);
	void UpdateWindowTitle();
	void UpdateStatusBar();
	void UpdateMenuState();
	HANDLE GetMultiOpenLockMutex(LPCTSTR lpszMutex);
	void SetOpenAssocLimitation(const CConfigFileListWindow& ConfFLW);

	HMENU GetUserAppMenuHandle();
	HMENU GetSendToMenuHandle();
	//�t�@�C���ꗗ�E�B���h�E�̗�
	static BOOL CALLBACK EnumFileListWindowProc(HWND hWnd,LPARAM lParam);
	//�ŏ��̃t�@�C���ꗗ�E�B���h�E�̗�
	static BOOL CALLBACK EnumFirstFileListWindowProc(HWND hWnd,LPARAM lParam);
	//�E�B���h�E�v���p�e�B�̗�
	static BOOL CALLBACK EnumPropProc(HWND hWnd,LPTSTR lpszString,HANDLE hData,ULONG_PTR dwData);

	//�c�[���o�[�쐬
	HWND CreateToolBarCtrl(HWND hWndParent, UINT nResourceID,HIMAGELIST hImageList);
protected:
	//---internal window functions
	LRESULT OnCreate(LPCREATESTRUCT lpcs);
	LRESULT OnDestroy(UINT, WPARAM, LPARAM, BOOL& bHandled);
	void OnSize(UINT uType, CSize);
	void OnMove(const CPoint&);
	void OnCommandCloseWindow(UINT uNotifyCode, int nID, HWND hWndCtl);
	void OnUpDir(UINT,int,HWND);
	void OnConfigure(UINT,int,HWND);

	void OnListViewStyle(UINT,int,HWND);
	void OnRefresh(UINT,int,HWND);
	LRESULT OnRefresh(UINT, WPARAM, LPARAM, BOOL& bHandled);
	void OnListMode(UINT,int,HWND);	//�\�����[�h�ύX
	void OnOpenArchive(UINT,int,HWND);
	void OnCloseTab(UINT,int,HWND);
	void OnNextTab(UINT,int,HWND);
	void OnToggleFocus(UINT,int,HWND);
	LRESULT OnMouseWheel(UINT,short,CPoint&);

	LRESULT OnFileListModelChanged(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnFileListArchiveLoaded(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnFileListNewContent(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnFileListUpdated(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnFileListWndStateChanged(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnOpenByPropName(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnActivateFile(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
protected:
	// ���b�Z�[�W�}�b�v
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
		COMMAND_ID_HANDLER_EX(ID_MENUITEM_LISTMODE_TREE,OnListMode)
		COMMAND_ID_HANDLER_EX(ID_MENUITEM_LISTMODE_FLAT,OnListMode)
		COMMAND_ID_HANDLER_EX(ID_MENUITEM_LISTMODE_FLAT_FILESONLY,OnListMode)
		COMMAND_ID_HANDLER_EX(ID_MENUITEM_OPENARCHIVE,OnOpenArchive);
		COMMAND_ID_HANDLER_EX(ID_MENUITEM_CLOSETAB,OnCloseTab);
		COMMAND_ID_HANDLER_EX(ID_MENUITEM_NEXTTAB,OnNextTab);
		COMMAND_ID_HANDLER_EX(ID_MENUITEM_PREVTAB,OnNextTab);
		MESSAGE_HANDLER(WM_FILELIST_MODELCHANGED,OnFileListModelChanged);
		MESSAGE_HANDLER(WM_FILELIST_ARCHIVE_LOADED, OnFileListArchiveLoaded)
		MESSAGE_HANDLER(WM_FILELIST_NEWCONTENT, OnFileListNewContent)
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
		UPDATE_ELEMENT(ID_MENUITEM_SORT_CRC,					UPDUI_MENUPOPUP|UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_MENUITEM_EXTRACT_ARCHIVE_ALL,			UPDUI_MENUPOPUP|UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_MENUITEM_EXTRACT_ARCHIVE_AND_CLOSE_ALL,UPDUI_MENUPOPUP|UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_MENUITEM_EXTRACT_ARCHIVE_AND_CLOSE,	UPDUI_MENUPOPUP|UPDUI_TOOLBAR)
	END_UPDATE_UI_MAP()

	//---
	CFileListFrame(CConfigManager &conf);
	virtual ~CFileListFrame(){}

	BOOL OnIdle(){
		UIUpdateStatusBar();
        UIUpdateToolBar();
		return FALSE;
	}
	BOOL PreTranslateMessage(MSG* pMsg);

	HRESULT OpenArchiveFile(LPCTSTR fname,DLL_ID idForceDLL,bool bAllowRelayOpen=true);

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
