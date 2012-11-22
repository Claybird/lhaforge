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
#include "../../resource.h"
#include "../ConfigShellExt.h"

class CConfigDialog;


//====================================
// 一般設定項目
//====================================
class CConfigDlgShellExt : public CDialogImpl<CConfigDlgShellExt>,public CMessageFilter,public CWinDataExchange<CConfigDlgShellExt>,public IConfigDlgBase
{
protected:
	CButton Check_ShellExt;
	CButton Check_ShellExtForceExtra;
	CConfigDialog	&mr_ConfigDlg;
	CConfigShellExt m_Config;

	virtual BOOL PreTranslateMessage(MSG* pMsg){
		return IsDialogMessage(pMsg);
	}

	LRESULT OnInitDialog(HWND hWnd, LPARAM lParam);
	LRESULT OnShellExt(WORD,WORD,HWND,BOOL&);
	LRESULT OnShellExtForceExtra(WORD,WORD,HWND,BOOL&);
	LRESULT OnEditMenu(WORD,WORD,HWND,BOOL&);
public:
	enum { IDD = IDD_PROPPAGE_CONFIG_SHELLEXT };
	// DDXマップ
	BEGIN_DDX_MAP(CConfigDlgShellExt)
		DDX_CHECK(IDC_CHECK_SHELLMENU_COMPRESS,m_Config.ShellMenuCompress)
		DDX_CHECK(IDC_CHECK_SHELLMENU_EXTRACT,m_Config.ShellMenuExtract)
		DDX_CHECK(IDC_CHECK_SHELLMENU_LIST,m_Config.ShellMenuList)
		DDX_CHECK(IDC_CHECK_SHELLMENU_TEST,m_Config.ShellMenuTest)
		DDX_CHECK(IDC_CHECK_SHELLMENU_UNDER_SUBMENU,m_Config.ShellMenuUnderSubMenu)

		DDX_CHECK(IDC_CHECK_DRAGMENU_COMPRESS,m_Config.DragMenuCompress)
		DDX_CHECK(IDC_CHECK_DRAGMENU_EXTRACT,m_Config.DragMenuExtract)
		DDX_CHECK(IDC_CHECK_DRAGMENU_UNDER_SUBMENU,m_Config.DragMenuUnderSubMenu)

		DDX_CHECK(IDC_CHECK_SHELL_EXT_FORCE_EXTRA,m_Config.ForceExtraMenu)
		DDX_CHECK(IDC_CHECK_SHELL_EXT_USECUSTOM,m_Config.UseCustomMenu)
	END_DDX_MAP()

	// メッセージマップ
	BEGIN_MSG_MAP_EX(CConfigDlgShellExt)
		MSG_WM_INITDIALOG(OnInitDialog)
		COMMAND_ID_HANDLER(IDC_BUTTON_EDIT_SHELLMENU,OnEditMenu)
		COMMAND_ID_HANDLER(IDC_CHECK_SHELL_EXT,OnShellExt)
		COMMAND_ID_HANDLER(IDC_CHECK_SHELL_EXT_FORCE_EXTRA,OnShellExtForceExtra)
		MSG_WM_DESTROY(OnDestroy)
	END_MSG_MAP()

	LRESULT OnApply();

	CConfigDlgShellExt(CConfigDialog &dlg):mr_ConfigDlg(dlg){
		TRACE(_T("CConfigDlgShellExt()\n"));
	}

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

