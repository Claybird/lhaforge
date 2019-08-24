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
#include "../ConfigManager.h"
#include "Dlg_shellext.h"
#include "../../Utilities/shellmanager.h"
#include "../configwnd.h"

//==============
// ��ʐݒ���
//==============
LRESULT CConfigDlgShellExt::OnInitDialog(HWND hWnd, LPARAM lParam)
{
	// ���b�Z�[�W���[�v�Ƀ��b�Z�[�W�t�B���^�ƃA�C�h���n���h����ǉ�
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	pLoop->AddMessageFilter(this);

	//------------------
	// �V�F���g���̗L��
	//------------------
	Check_ShellExt=GetDlgItem(IDC_CHECK_SHELL_EXT);
	Check_ShellExt.SetCheck(ShellRegistCheck());

	Check_ShellExtForceExtra=GetDlgItem(IDC_CHECK_SHELL_EXT_FORCE_EXTRA);

	//DDX���A�b�v�f�[�g
	DoDataExchange(FALSE);

	BOOL bActive=Check_ShellExt.GetCheck();
	::EnableWindow(GetDlgItem(IDC_CHECK_SHELLMENU_COMPRESS),bActive);
	::EnableWindow(GetDlgItem(IDC_CHECK_SHELLMENU_EXTRACT),bActive);
	::EnableWindow(GetDlgItem(IDC_CHECK_SHELLMENU_LIST),bActive);
	::EnableWindow(GetDlgItem(IDC_CHECK_SHELLMENU_TEST),bActive);
	::EnableWindow(GetDlgItem(IDC_CHECK_SHELLMENU_UNDER_SUBMENU),bActive);

	::EnableWindow(GetDlgItem(IDC_CHECK_DRAGMENU_COMPRESS),bActive);
	::EnableWindow(GetDlgItem(IDC_CHECK_DRAGMENU_EXTRACT),bActive);
	::EnableWindow(GetDlgItem(IDC_CHECK_DRAGMENU_UNDER_SUBMENU),bActive);

	//------------------------------------
	// �펞�g�����j���[���g�p���邩�ǂ���
	//------------------------------------
	::EnableWindow(GetDlgItem(IDC_CHECK_SHELL_EXT_FORCE_EXTRA),bActive);

	//���j���[�J�X�^�}�C�Y
	bActive=bActive&&(!Check_ShellExtForceExtra.GetCheck());
	::EnableWindow(GetDlgItem(IDC_CHECK_SHELL_EXT_USECUSTOM),bActive);
	::EnableWindow(GetDlgItem(IDC_BUTTON_EDIT_SHELLMENU),bActive);

	return TRUE;
}

LRESULT CConfigDlgShellExt::OnApply()
{
//==========================
// �V�F���g����ON/OFF�𔽉f
//==========================
	bool bCurrentStatus=(0!=Check_ShellExt.GetCheck());

	//�˗����e���L�q
	CString strIniName(mr_ConfigDlg.GetAssistantFile());
	//---�o�^or����
	if(bCurrentStatus){
		//�o�^
		UtilWritePrivateProfileInt(_T("Shell"),_T("set"),1,strIniName);
	}else{
		//����
		UtilWritePrivateProfileInt(_T("Shell"),_T("set"),0,strIniName);
	}

//===============================
// �ݒ��ConfigManager�ɏ����߂�
//===============================
	//---------------
	// DDX�f�[�^�X�V
	//---------------
	if(!DoDataExchange(TRUE)){
		return FALSE;
	}
	return TRUE;
}


LRESULT CConfigDlgShellExt::OnShellExt(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if(BN_CLICKED==wNotifyCode){
		bool bActive=BOOL2bool(Check_ShellExt.GetCheck());
		::EnableWindow(GetDlgItem(IDC_CHECK_SHELLMENU_COMPRESS),	  bActive);
		::EnableWindow(GetDlgItem(IDC_CHECK_SHELLMENU_EXTRACT),		  bActive);
		::EnableWindow(GetDlgItem(IDC_CHECK_SHELLMENU_LIST),		  bActive);
		::EnableWindow(GetDlgItem(IDC_CHECK_SHELLMENU_TEST),		  bActive);
		::EnableWindow(GetDlgItem(IDC_CHECK_SHELLMENU_UNDER_SUBMENU), bActive);

		::EnableWindow(GetDlgItem(IDC_CHECK_DRAGMENU_COMPRESS),		 bActive);
		::EnableWindow(GetDlgItem(IDC_CHECK_DRAGMENU_EXTRACT),		 bActive);
		::EnableWindow(GetDlgItem(IDC_CHECK_DRAGMENU_UNDER_SUBMENU), bActive);

		::EnableWindow(GetDlgItem(IDC_CHECK_SHELL_EXT_FORCE_EXTRA),	 bActive);


		//���j���[�J�X�^�}�C�Y
		bool bCustomActive = bActive && (!Check_ShellExtForceExtra.GetCheck());
		::EnableWindow(GetDlgItem(IDC_CHECK_SHELL_EXT_USECUSTOM),	bCustomActive);
		::EnableWindow(GetDlgItem(IDC_BUTTON_EDIT_SHELLMENU),		bCustomActive);

		bool bOldStatus=ShellRegistCheck();
		if(bActive ^ bOldStatus){	//if( (!bActive && bOldStatus) || (bActive && !bOldStatus)){
			//LFAssist.exe�̎��s��v��
			mr_ConfigDlg.RequireAssistant();
		}else{
			//LFAssist.exe�̎��s�v����������
			mr_ConfigDlg.UnrequireAssistant();
		}
	}
	return 0;
}

LRESULT CConfigDlgShellExt::OnShellExtForceExtra(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if(BN_CLICKED==wNotifyCode){
		BOOL bActive=(!Check_ShellExtForceExtra.GetCheck())&&Check_ShellExt.GetCheck();
		::EnableWindow(GetDlgItem(IDC_CHECK_SHELL_EXT_USECUSTOM),bActive);
		::EnableWindow(GetDlgItem(IDC_BUTTON_EDIT_SHELLMENU),bActive);
	}
	return 0;
}

LRESULT CConfigDlgShellExt::OnEditMenu(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if(BN_CLICKED!=wNotifyCode){
		return 0;
	}

	//�ҏW�v���O�����N��
	TCHAR szPath[_MAX_PATH]={0};
	GetModuleFileName(NULL,szPath,_MAX_PATH);

	PathRemoveFileSpec(szPath);
	PathAppend(szPath,_T("MenuEditor.exe"));

	ShellExecute(m_hWnd,_T("open"),szPath,NULL,NULL,SW_SHOW);
	return 0;
}
