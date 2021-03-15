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
#include "../ConfigManager.h"
#include "Dlg_shellext.h"
#include "../../Utilities/shellmanager.h"
#include "../configwnd.h"

//==============
// 一般設定画面
//==============
LRESULT CConfigDlgShellExt::OnInitDialog(HWND hWnd, LPARAM lParam)
{
	// メッセージループにメッセージフィルタとアイドルハンドラを追加
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	pLoop->AddMessageFilter(this);

	//------------------
	// シェル拡張の有無
	//------------------
	Check_ShellExt=GetDlgItem(IDC_CHECK_SHELL_EXT);
	Check_ShellExt.SetCheck(ShellRegistCheck());

	Check_ShellExtForceExtra=GetDlgItem(IDC_CHECK_SHELL_EXT_FORCE_EXTRA);

	//DDX情報アップデート
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
	// 常時拡張メニューを使用するかどうか
	//------------------------------------
	::EnableWindow(GetDlgItem(IDC_CHECK_SHELL_EXT_FORCE_EXTRA),bActive);

	//メニューカスタマイズ
	bActive=bActive&&(!Check_ShellExtForceExtra.GetCheck());
	::EnableWindow(GetDlgItem(IDC_CHECK_SHELL_EXT_USECUSTOM),bActive);
	::EnableWindow(GetDlgItem(IDC_BUTTON_EDIT_SHELLMENU),bActive);

	return TRUE;
}

LRESULT CConfigDlgShellExt::OnApply()
{
//==========================
// シェル拡張のON/OFFを反映
//==========================
	bool bCurrentStatus=(0!=Check_ShellExt.GetCheck());

	//依頼内容を記述
	std::wstring strIniName = mr_ConfigDlg.GetAssistantFile();
	//---登録or解除
	CConfigManager tmp;
	tmp.setPath(strIniName);
	try {
		tmp.load();
		if (bCurrentStatus) {
			//登録
			tmp.setValue(L"Shell", L"set", 1);
		} else {
			//解除
			tmp.setValue(L"Shell", L"set", 0);
		}
		tmp.save();
	} catch (const LF_EXCEPTION &e) {
		ErrorMessage(e.what());
	}

//===============================
// 設定をConfigManagerに書き戻す
//===============================
	//---------------
	// DDXデータ更新
	//---------------
	if(!DoDataExchange(TRUE)){
		return FALSE;
	}
	return TRUE;
}


LRESULT CConfigDlgShellExt::OnShellExt(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if(BN_CLICKED==wNotifyCode){
		bool bActive = (FALSE != Check_ShellExt.GetCheck());
		::EnableWindow(GetDlgItem(IDC_CHECK_SHELLMENU_COMPRESS),	  bActive);
		::EnableWindow(GetDlgItem(IDC_CHECK_SHELLMENU_EXTRACT),		  bActive);
		::EnableWindow(GetDlgItem(IDC_CHECK_SHELLMENU_LIST),		  bActive);
		::EnableWindow(GetDlgItem(IDC_CHECK_SHELLMENU_TEST),		  bActive);
		::EnableWindow(GetDlgItem(IDC_CHECK_SHELLMENU_UNDER_SUBMENU), bActive);

		::EnableWindow(GetDlgItem(IDC_CHECK_DRAGMENU_COMPRESS),		 bActive);
		::EnableWindow(GetDlgItem(IDC_CHECK_DRAGMENU_EXTRACT),		 bActive);
		::EnableWindow(GetDlgItem(IDC_CHECK_DRAGMENU_UNDER_SUBMENU), bActive);

		::EnableWindow(GetDlgItem(IDC_CHECK_SHELL_EXT_FORCE_EXTRA),	 bActive);


		//メニューカスタマイズ
		bool bCustomActive = bActive && (!Check_ShellExtForceExtra.GetCheck());
		::EnableWindow(GetDlgItem(IDC_CHECK_SHELL_EXT_USECUSTOM),	bCustomActive);
		::EnableWindow(GetDlgItem(IDC_BUTTON_EDIT_SHELLMENU),		bCustomActive);

		bool bOldStatus=ShellRegistCheck();
		if(bActive ^ bOldStatus){	//if( (!bActive && bOldStatus) || (bActive && !bOldStatus)){
			//LFAssist.exeの実行を要請
			mr_ConfigDlg.RequireAssistant();
		}else{
			//LFAssist.exeの実行要請を取り消し
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

	//編集プログラム起動
	std::filesystem::path path = UtilGetModuleDirectoryPath();
	path /= L"MenuEditor.exe";

	ShellExecuteW(m_hWnd, L"open", path.make_preferred().c_str(), nullptr, nullptr, SW_SHOW);
	return 0;
}
