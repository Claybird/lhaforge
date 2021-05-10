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
#include "resource.h"
#include "FileListWindow/FileListModel.h"
#include "ConfigCode/ConfigFileListWindow.h"
#include "ConfigCode/ConfigFile.h"

class CConfigDlgFileListWindow : public LFConfigDialogBase<CConfigDlgFileListWindow>
{
protected:
	CConfigFileListWindow	m_Config;
	CListViewCtrl List_Command;

	std::vector<CLFMenuCommandItem> m_MenuCommandArray;
	CLFMenuCommandItem *m_lpMenuCommandItem;	//command item in edit

	void OnClearTemporary(UINT, int, HWND) {
		//clear remaining temoorary directory
		if (!UtilDeleteDir(UtilGetTempPath().c_str(), false))MessageBeep(MB_ICONHAND);
	}
	void OnResetExt(UINT, int nID, HWND) {
		//reset disallowed/allowed extension list
		switch (nID) {
		case IDC_BUTTON_RESET_OPENASSOC_ACCEPT:
			SetDlgItemText(IDC_EDIT_OPENASSOC_ACCEPT, UtilLoadString(IDS_FILELIST_OPENASSOC_DEFAULT_ACCEPT).c_str());
			break;
		case IDC_BUTTON_RESET_OPENASSOC_DENY:
			SetDlgItemText(IDC_EDIT_OPENASSOC_DENY, UtilLoadString(IDS_FILELIST_OPENASSOC_DEFAULT_DENY).c_str());
			break;
		}
	}

	void OnBrowsePath(UINT, int, HWND);
	void OnBrowseDir(UINT, int, HWND);
	void OnBrowseCustomToolbarImage(UINT, int, HWND);

	LRESULT OnGetDispInfo(LPNMHDR pnmh);
	LRESULT OnSelect(LPNMHDR pnmh);
	LRESULT OnUserAppMoveUpDown(WORD,WORD,HWND,BOOL&);
	LRESULT OnUserAppNew(WORD,WORD,HWND,BOOL&);
	LRESULT OnUserAppDelete(WORD,WORD,HWND,BOOL&);
	LRESULT OnCheckChanged(WORD, WORD, HWND, BOOL&) {
		auto checked= CButton(GetDlgItem(IDC_CHECK_DISABLE_TAB)).GetCheck();
		::EnableWindow(GetDlgItem(IDC_CHECK_KEEP_SINGLE_INSTANCE), !checked);
		return 0;
	}

	void getUserAppEdit() {
		CString buf;
		GetDlgItemText(IDC_EDIT_FILELIST_USERAPP_PATH, buf);
		m_lpMenuCommandItem->Path = buf;
		GetDlgItemText(IDC_EDIT_FILELIST_USERAPP_PARAM, buf);
		m_lpMenuCommandItem->Param = buf;
		GetDlgItemText(IDC_EDIT_FILELIST_USERAPP_DIR, buf);
		m_lpMenuCommandItem->Dir = buf;
		GetDlgItemText(IDC_EDIT_FILELIST_USERAPP_CAPTION, buf);
		m_lpMenuCommandItem->Caption = buf;
	}
	void setUserAppEdit() {
		SetDlgItemText(IDC_EDIT_FILELIST_USERAPP_PATH, m_lpMenuCommandItem->Path.c_str());
		SetDlgItemText(IDC_EDIT_FILELIST_USERAPP_PARAM, m_lpMenuCommandItem->Param.c_str());
		SetDlgItemText(IDC_EDIT_FILELIST_USERAPP_DIR, m_lpMenuCommandItem->Dir.c_str());
		SetDlgItemText(IDC_EDIT_FILELIST_USERAPP_CAPTION, m_lpMenuCommandItem->Caption.c_str());
	}
public:
	enum { IDD = IDD_PROPPAGE_CONFIG_FILELISTWINDOW };

	BEGIN_DDX_MAP(CConfigDlgFileListWindow)
		DDX_CHECK(IDC_CHECK_STORE_FILELISTWINDOW_SETTING, m_Config.general.StoreSetting)
		DDX_CHECK(IDC_CHECK_EXIT_WITH_ESCAPE, m_Config.general.ExitWithEscape)
		DDX_CHECK(IDC_CHECK_KEEP_SINGLE_INSTANCE, m_Config.general.KeepSingleInstance)
		DDX_CHECK(IDC_CHECK_DISABLE_TAB, m_Config.general.DisableTab)

		DDX_CHECK(IDC_CHECK_STORE_FILELISTWINDOW_POSITION, m_Config.dimensions.StoreWindowPosition)
		DDX_CHECK(IDC_CHECK_EXPAND_TREE, m_Config.view.ExpandTree)
		DDX_CHECK(IDC_CHECK_DISPLAY_FILESIZE_IN_BYTE,m_Config.view.DisplayFileSizeInByte)
		DDX_CHECK(IDC_CHECK_DISPLAY_PATH_ONLY,m_Config.view.DisplayPathOnly)
		DDX_TEXT(IDC_EDIT_OPENASSOC_ACCEPT, m_Config.view.OpenAssoc.Accept)
		DDX_TEXT(IDC_EDIT_OPENASSOC_DENY, m_Config.view.OpenAssoc.Deny)
		DDX_CHECK(IDC_CHECK_DENY_PATHEXT, m_Config.view.OpenAssoc.DenyExecutables)
		DDX_TEXT(IDC_EDIT_CUSTOMTOOLBAR_IMAGE, m_Config.view.strCustomToolbarImage)
		DDX_CHECK(IDC_CHECK_SHOW_TOOLBAR,m_Config.view.ShowToolbar)
	END_DDX_MAP()

	BEGIN_MSG_MAP_EX(CConfigDlgFileListWindow)
		NOTIFY_CODE_HANDLER_EX(LVN_GETDISPINFO, OnGetDispInfo)	//virtual list view
		NOTIFY_CODE_HANDLER_EX(LVN_ITEMCHANGED, OnSelect)
		COMMAND_ID_HANDLER_EX(IDC_BUTTON_CLEAR_TEMPORARY,OnClearTemporary)
		COMMAND_ID_HANDLER_EX(IDC_BUTTON_RESET_OPENASSOC_ACCEPT,OnResetExt)
		COMMAND_ID_HANDLER_EX(IDC_BUTTON_RESET_OPENASSOC_DENY,OnResetExt)

		COMMAND_ID_HANDLER(IDC_BUTTON_FILELIST_USERAPP_NEW,OnUserAppNew)
		COMMAND_ID_HANDLER(IDC_BUTTON_FILELIST_USERAPP_DELETE,OnUserAppDelete)
		COMMAND_ID_HANDLER(IDC_BUTTON_FILELIST_USERAPP_MOVEUP, OnUserAppMoveUpDown)
		COMMAND_ID_HANDLER(IDC_BUTTON_FILELIST_USERAPP_MOVEDOWN, OnUserAppMoveUpDown)

		COMMAND_ID_HANDLER_EX(IDC_BUTTON_FILELIST_USERAPP_BROWSE, OnBrowsePath)
		COMMAND_ID_HANDLER_EX(IDC_BUTTON_FILELIST_USERAPP_BROWSEDIR, OnBrowseDir)
		COMMAND_ID_HANDLER_EX(IDC_BUTTON_BROWSE_CUSTOMTOOLBAR_IMAGE, OnBrowseCustomToolbarImage)

		COMMAND_ID_HANDLER(IDC_CHECK_DISABLE_TAB, OnCheckChanged)
		MSG_WM_INITDIALOG(OnInitDialog)
	END_MSG_MAP()

	LRESULT OnInitDialog(HWND hWnd, LPARAM lParam);
	LRESULT OnApply();

	void LoadConfig(CConfigFile& Config){
		m_Config.load(Config);
	}
	void StoreConfig(CConfigFile& Config, CConfigFile& assistant){
		m_Config.store(Config);
	}
};

