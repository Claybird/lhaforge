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
#include "Dlg_Base.h"
#include "../../resource.h"
#include "../ConfigShellExt.h"

class CConfigDialog;


//====================================
// 一般設定項目
//====================================
class CConfigDlgShellExt : public LFConfigDialogBase<CConfigDlgShellExt>
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

	void LoadConfig(CConfigFile& Config){
		m_Config.load(Config);
	}
	void StoreConfig(CConfigFile& Config){
		m_Config.store(Config);
	}
};

