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
#include "Dlg_Base.h"
#include "../ConfigManager.h"
#include "../../resource.h"
#include "../../FileListWindow/FileListModel.h"
#include "../ConfigFileListWindow.h"

//==================================
// �t�@�C���ꗗ�E�B���h�E�̐ݒ荀��
//==================================
class CConfigDlgFileListWindow : public CDialogImpl<CConfigDlgFileListWindow>,public CWinDataExchange<CConfigDlgFileListWindow>,public CMessageFilter,public IConfigDlgBase
{
protected:
	CConfigFileListWindow	m_Config;

	virtual BOOL PreTranslateMessage(MSG* pMsg){
		return IsDialogMessage(pMsg);
	}

	void OnClearTemporary(UINT,int,HWND);	//�c���Ă��܂����e���|�����f�B���N�g�����폜
	void OnResetExt(UINT,int,HWND);			//�֎~�E���g���q�̃��Z�b�g

	CButton Radio_FileListMode[FILELISTMODE_ITEM_COUNT];

	CEdit Edit_Path,Edit_Param,Edit_Dir,Edit_Caption;
	CListViewCtrl List_Command;
	void OnBrowsePath(UINT, int, HWND);
	void OnBrowseDir(UINT, int, HWND);
	void OnBrowseCustomToolbarImage(UINT, int, HWND);

	std::vector<CMenuCommandItem> m_MenuCommandArray;
	CMenuCommandItem *m_lpMenuCommandItem;	//�ҏW���̃R�}���h�A�C�e��

	LRESULT OnGetDispInfo(LPNMHDR pnmh);
	LRESULT OnSelect(LPNMHDR pnmh);
	LRESULT OnUserAppMoveUp(WORD,WORD,HWND,BOOL&);
	LRESULT OnUserAppMoveDown(WORD,WORD,HWND,BOOL&);
	LRESULT OnUserAppNew(WORD,WORD,HWND,BOOL&);
	LRESULT OnUserAppDelete(WORD,WORD,HWND,BOOL&);
	LRESULT OnCheckChanged(WORD,WORD,HWND,BOOL&);
public:
	enum { IDD = IDD_PROPPAGE_CONFIG_FILELISTWINDOW };

	// DDX�}�b�v
	BEGIN_DDX_MAP(CConfigDlgFileListWindow)
		DDX_CHECK(IDC_CHECK_STORE_FILELISTWINDOW_SETTING, m_Config.StoreSetting)
		DDX_CHECK(IDC_CHECK_STORE_FILELISTWINDOW_POSITION, m_Config.StoreWindowPosition)
		DDX_CHECK(IDC_CHECK_IGNORE_MEANINGLESS_PATH, m_Config.IgnoreMeaninglessPath)
		DDX_CHECK(IDC_CHECK_EXPAND_TREE, m_Config.ExpandTree)
		DDX_CHECK(IDC_CHECK_DISPLAY_FILESIZE_IN_BYTE,m_Config.DisplayFileSizeInByte)
		DDX_CHECK(IDC_CHECK_EXIT_WITH_ESCAPE,m_Config.ExitWithEscape)
		DDX_CHECK(IDC_CHECK_DISABLE_TAB,m_Config.DisableTab)
		DDX_CHECK(IDC_CHECK_KEEP_SINGLE_INSTANCE,m_Config.KeepSingleInstance)
		DDX_TEXT(IDC_EDIT_OPENASSOC_ACCEPT, m_Config.OpenAssoc.Accept)
		DDX_TEXT(IDC_EDIT_OPENASSOC_DENY, m_Config.OpenAssoc.Deny)
		DDX_TEXT(IDC_EDIT_CUSTOMTOOLBAR_IMAGE, m_Config.strCustomToolbarImage)
		DDX_CHECK(IDC_CHECK_SHOW_TOOLBAR,m_Config.ShowToolbar)
		DDX_CHECK(IDC_CHECK_DENY_PATHEXT,m_Config.DenyPathExt)
	END_DDX_MAP()

	// ���b�Z�[�W�}�b�v
	BEGIN_MSG_MAP_EX(CConfigDlgFileListWindow)
		NOTIFY_CODE_HANDLER_EX(LVN_GETDISPINFO, OnGetDispInfo)	//���z���X�g�r���[
		NOTIFY_CODE_HANDLER_EX(LVN_ITEMCHANGED, OnSelect)
		COMMAND_ID_HANDLER_EX(IDC_BUTTON_CLEAR_TEMPORARY,OnClearTemporary)
		COMMAND_ID_HANDLER_EX(IDC_BUTTON_RESET_OPENASSOC_ACCEPT,OnResetExt)
		COMMAND_ID_HANDLER_EX(IDC_BUTTON_RESET_OPENASSOC_DENY,OnResetExt)

		COMMAND_ID_HANDLER(IDC_BUTTON_FILELIST_USERAPP_NEW,OnUserAppNew)
		COMMAND_ID_HANDLER(IDC_BUTTON_FILELIST_USERAPP_DELETE,OnUserAppDelete)
		COMMAND_ID_HANDLER(IDC_BUTTON_FILELIST_USERAPP_MOVEUP,OnUserAppMoveUp)
		COMMAND_ID_HANDLER(IDC_BUTTON_FILELIST_USERAPP_MOVEDOWN,OnUserAppMoveDown)

		COMMAND_ID_HANDLER_EX(IDC_BUTTON_FILELIST_USERAPP_BROWSE, OnBrowsePath)
		COMMAND_ID_HANDLER_EX(IDC_BUTTON_FILELIST_USERAPP_BROWSEDIR, OnBrowseDir)
		COMMAND_ID_HANDLER_EX(IDC_BUTTON_BROWSE_CUSTOMTOOLBAR_IMAGE, OnBrowseCustomToolbarImage)

		COMMAND_ID_HANDLER(IDC_CHECK_DISABLE_TAB, OnCheckChanged)
		MSG_WM_INITDIALOG(OnInitDialog)
		MSG_WM_DESTROY(OnDestroy)
	END_MSG_MAP()

	LRESULT OnInitDialog(HWND hWnd, LPARAM lParam);
	LRESULT OnApply();

	LRESULT OnDestroy(){
		CMessageLoop* pLoop = _Module.GetMessageLoop();
		pLoop->RemoveMessageFilter(this);

		return TRUE;
	}

	void LoadConfig(CConfigManager& Config){
		m_Config.load(Config);
	}
	void StoreConfig(CConfigManager& Config){
		m_Config.store(Config);
	}
};

