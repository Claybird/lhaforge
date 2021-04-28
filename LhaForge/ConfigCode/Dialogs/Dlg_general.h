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
#include "ArchiverCode/archive.h"
#include "ConfigCode/ConfigFile.h"
#include "ConfigCode/ConfigGeneral.h"
#include "Utilities/OSUtil.h"

class CConfigDlgGeneral : public LFConfigDialogBase<CConfigDlgGeneral>
{
protected:
	CConfigGeneral m_Config;

	LRESULT OnInitDialog(HWND hWnd, LPARAM lParam) {
		::EnableWindow(GetDlgItem(IDC_EDIT_FILER_PATH), m_Config.Filer.UseFiler);
		::EnableWindow(GetDlgItem(IDC_EDIT_FILER_PARAM), m_Config.Filer.UseFiler);
		::EnableWindow(GetDlgItem(IDC_BUTTON_BROWSE_FILER), m_Config.Filer.UseFiler);
		//DDX
		DoDataExchange(FALSE);
		return TRUE;
	}
	LRESULT OnCheckFiler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
		if (BN_CLICKED == wNotifyCode) {
			BOOL bActive = IsDlgButtonChecked(IDC_CHECK_USE_FILER);
			::EnableWindow(GetDlgItem(IDC_EDIT_FILER_PATH), bActive);
			::EnableWindow(GetDlgItem(IDC_EDIT_FILER_PARAM), bActive);
			::EnableWindow(GetDlgItem(IDC_BUTTON_BROWSE_FILER), bActive);
		}
		return 0;
	}
	LRESULT OnBrowseFiler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
		if (BN_CLICKED == wNotifyCode) {
			CString FilerPath;
			GetDlgItemTextW(IDC_EDIT_FILER_PATH, FilerPath);

			const COMDLG_FILTERSPEC filter[] = {
				{L"Executable File(*.exe)",L"*.exe"},
				{L"All Files", L"*.*"},
			};
			LFShellFileOpenDialog dlg(FilerPath, FOS_FORCEFILESYSTEM | FOS_FILEMUSTEXIST | FOS_PATHMUSTEXIST, nullptr, filter, COUNTOF(filter));
			if (IDOK == dlg.DoModal()) {
				dlg.GetFilePath(FilerPath);
				SetDlgItemTextW(IDC_EDIT_FILER_PATH, FilerPath);
			}
		}
		return 0;
	}
	LRESULT OnBrowseTempPath(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
		if (BN_CLICKED == wNotifyCode) {
			CString path;
			GetDlgItemTextW(IDC_EDIT_TEMP_PATH, path);

			LFShellFileOpenDialog dlg(path, FOS_FORCEFILESYSTEM | FOS_FILEMUSTEXIST | FOS_PATHMUSTEXIST | FOS_PICKFOLDERS);
			if (IDOK == dlg.DoModal()) {
				dlg.GetFilePath(path);
				SetDlgItemTextW(IDC_EDIT_TEMP_PATH, path);
			} else {
				return E_ABORT;
			}
		}
		return 0;
	}
public:
	enum { IDD = IDD_PROPPAGE_CONFIG_GENERAL };
	BEGIN_DDX_MAP(CConfigDlgGeneral)
		DDX_CHECK(IDC_CHECK_WARN_NETWORK, m_Config.WarnNetwork)
		DDX_CHECK(IDC_CHECK_WARN_REMOVABLE, m_Config.WarnRemovable)
		DDX_CHECK(IDC_CHECK_NOTIFY_SHELL, m_Config.NotifyShellAfterProcess)
		DDX_TEXT(IDC_EDIT_TEMP_PATH, m_Config.TempPath)
		DDX_RADIO(IDC_RADIO_LOSTDIR_ASK_TO_CREATE, m_Config.OnDirNotFound)
		DDX_RADIO(IDC_RADIO_VIEW_LOG_ON_ERROR, m_Config.LogViewEvent)
		DDX_CHECK(IDC_CHECK_USE_FILER, m_Config.Filer.UseFiler)
		DDX_TEXT(IDC_EDIT_FILER_PATH, m_Config.Filer.FilerPath)
		DDX_TEXT(IDC_EDIT_FILER_PARAM, m_Config.Filer.Param)
	END_DDX_MAP()

	BEGIN_MSG_MAP_EX(CConfigDlgGeneral)
		MSG_WM_INITDIALOG(OnInitDialog)
		COMMAND_ID_HANDLER(IDC_CHECK_USE_FILER,OnCheckFiler)
		COMMAND_ID_HANDLER(IDC_BUTTON_BROWSE_FILER,OnBrowseFiler)
		COMMAND_ID_HANDLER(IDC_BUTTON_BROWSE_TEMP,OnBrowseTempPath)
	END_MSG_MAP()

	LRESULT OnApply() {
		// DDX
		if (!DoDataExchange(TRUE)) {
			return FALSE;
		}
		return TRUE;
	}

	void LoadConfig(CConfigFile& Config){
		m_Config.load(Config);
	}
	void StoreConfig(CConfigFile& Config, CConfigFile& assistant){
		m_Config.store(Config);
	}
};

