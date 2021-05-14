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
#include "ConfigCode/ConfigFile.h"
#include "Dlg_shellext.h"
#include "Utilities/shellmanager.h"
#include "Utilities/FileOperation.h"
#include "ConfigCode/ConfigWnd.h"

LRESULT CConfigDlgShellExt::OnInitDialog(HWND hWnd, LPARAM lParam)
{
	BOOL bActive = ShellRegistCheck();
	Check_ShellExt=GetDlgItem(IDC_CHECK_SHELL_EXT);
	Check_ShellExt.SetCheck(bActive);

	Check_ShellExtForceExtra=GetDlgItem(IDC_CHECK_SHELL_EXT_FORCE_EXTRA);

	//update via DDX
	DoDataExchange(FALSE);

	//shell extension is active?
	::EnableWindow(GetDlgItem(IDC_CHECK_SHELLMENU_COMPRESS),bActive);
	::EnableWindow(GetDlgItem(IDC_CHECK_SHELLMENU_EXTRACT),bActive);
	::EnableWindow(GetDlgItem(IDC_CHECK_SHELLMENU_LIST),bActive);
	::EnableWindow(GetDlgItem(IDC_CHECK_SHELLMENU_TEST),bActive);
	::EnableWindow(GetDlgItem(IDC_CHECK_SHELLMENU_UNDER_SUBMENU),bActive);

	::EnableWindow(GetDlgItem(IDC_CHECK_DRAGMENU_COMPRESS),bActive);
	::EnableWindow(GetDlgItem(IDC_CHECK_DRAGMENU_EXTRACT),bActive);
	::EnableWindow(GetDlgItem(IDC_CHECK_DRAGMENU_UNDER_SUBMENU),bActive);

	// Use extended menu always?
	::EnableWindow(GetDlgItem(IDC_CHECK_SHELL_EXT_FORCE_EXTRA),bActive);

	//Customize menu
	bool bCustomActive = bActive && (!Check_ShellExtForceExtra.GetCheck());
	::EnableWindow(GetDlgItem(IDC_BUTTON_EDIT_SHELLMENU), bCustomActive);

	return TRUE;
}

LRESULT CConfigDlgShellExt::OnApply()
{
	_requests.clear();
	bool bCurrentStatus=(0!=Check_ShellExt.GetCheck());

	//request to enable/disable shell
	if (bCurrentStatus) {
		_requests[L"Shell"] = { { L"set", L"1" } };
	} else {
		_requests[L"Shell"] = { { L"set", L"0" } };
	}

	// DDX: write back to config
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


		bool bCustomActive = bActive && (!Check_ShellExtForceExtra.GetCheck());
		::EnableWindow(GetDlgItem(IDC_BUTTON_EDIT_SHELLMENU),		bCustomActive);

		bool bOldStatus = ShellRegistCheck();
		if(bActive ^ bOldStatus){
			//request LFAssist.exe
			mr_ConfigDlg.RequireAssistant();
		}else{
			//cancel LFAssist.exe
			mr_ConfigDlg.UnrequireAssistant();
		}
	}
	return 0;
}

LRESULT CConfigDlgShellExt::OnShellExtForceExtra(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if(BN_CLICKED==wNotifyCode){
		BOOL bActive = (!Check_ShellExtForceExtra.GetCheck()) && Check_ShellExt.GetCheck();
		::EnableWindow(GetDlgItem(IDC_BUTTON_EDIT_SHELLMENU),bActive);
	}
	return 0;
}

LRESULT CConfigDlgShellExt::OnEditMenu(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if(BN_CLICKED!=wNotifyCode){
		return 0;
	}

	//Run menu editor
	std::filesystem::path path = UtilGetModuleDirectoryPath();
	path /= L"MenuEditor.exe";

	ShellExecuteW(m_hWnd, L"open",
		(L'"'+path.make_preferred().wstring()+ L'"').c_str(), nullptr, nullptr, SW_SHOW);
	return 0;
}
