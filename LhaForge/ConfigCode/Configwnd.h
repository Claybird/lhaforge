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
#include "Utilities/Utility.h"
#include "resource.h"
#include "ConfigFile.h"
#include "Dialogs/Dlg_Base.h"
#include "Dialogs/Dlg_version.h"
#include "Dialogs/Dlg_general.h"
#include "Dialogs/Dlg_compress_general.h"
#include "Dialogs/Dlg_extract_general.h"
#include "Dialogs/Dlg_assoc.h"
#include "Dialogs/Dlg_shortcut.h"
#include "Dialogs/Dlg_filelistwindow.h"
#include "Dialogs/Dlg_openaction.h"
#include "Dialogs/Dlg_shellext.h"

#define WM_USER_WM_SIZE		(WM_APP+1)

class CConfigDialog : public CDialogImpl<CConfigDialog>, public CDialogResize<CConfigDialog>
{
protected:
	CConfigDlgGeneral				PageGeneral;
	CConfigDlgShellExt				PageShellExt;
	CConfigDlgVersion				PageVersion;
	CConfigDlgCompressGeneral		PageCompressGeneral;
	CConfigDlgExtractGeneral		PageExtractGeneral;
	CConfigDlgAssociation			PageAssociation;
	CConfigDlgOpenAction			PageOpenAction;
	CConfigDlgShortcut				PageShortcut;
	CConfigDlgFileListWindow		PageFileListWindow;

	HWND hActiveDialogWnd;
	CTreeViewCtrl SelectTreeView;

	//scroll container to paste dialogs on
	CScrollContainer ScrollWindow;

	CConfigFile &mr_Config;

	UINT m_nAssistRequireCount;//0: no need to call LFAssistant; 0<: need to call

	std::set<IConfigDlgBase*> m_ConfigDlgList;
public:
	enum { IDD = IDD_DIALOG_CONFIG };

	BEGIN_MSG_MAP_EX(CConfigDialog)
		MSG_WM_INITDIALOG(OnInitDialog)
		NOTIFY_CODE_HANDLER_EX(TVN_SELCHANGED, OnTreeSelect)
		COMMAND_ID_HANDLER_EX(IDOK, OnOK)
		COMMAND_ID_HANDLER_EX(IDCANCEL, OnCancel)
		MSG_WM_SIZE(OnSize)
		CHAIN_MSG_MAP(CDialogResize<CConfigDialog>)
		MESSAGE_HANDLER(WM_USER_WM_SIZE, OnUserSize)
		END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(CConfigDialog)
		DLGRESIZE_CONTROL(IDC_TREE_SELECT_PROPPAGE, DLSZ_SIZE_Y)
		DLGRESIZE_CONTROL(IDC_STATIC_FRAME, DLSZ_SIZE_X | DLSZ_SIZE_Y)
		DLGRESIZE_CONTROL(IDOK, DLSZ_MOVE_X | DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDCANCEL, DLSZ_MOVE_X | DLSZ_MOVE_Y)
	END_DLGRESIZE_MAP()

	CConfigDialog(CConfigFile &cfg);
	virtual ~CConfigDialog() {}

	LRESULT OnTreeSelect(LPNMHDR pnmh);
	LRESULT OnInitDialog(HWND hWnd, LPARAM lParam);
	void OnSize(UINT, CSize&) {
		SetMsgHandled(false);
		PostMessage(WM_USER_WM_SIZE);
	}
	LRESULT OnUserSize(UINT, WPARAM wParam, LPARAM, BOOL& bHandled);

	void OnOK(UINT uNotifyCode, int nID, HWND hWndCtl);
	void OnCancel(UINT uNotifyCode, int nID, HWND hWndCtl) { EndDialog(nID); }

	void RequireAssistant() {
		m_nAssistRequireCount++;
		Button_SetElevationRequiredState(GetDlgItem(IDOK), m_nAssistRequireCount);
	}
	void UnrequireAssistant() {
		ASSERT(m_nAssistRequireCount > 0);
		if (m_nAssistRequireCount > 0) {
			m_nAssistRequireCount--;
			Button_SetElevationRequiredState(GetDlgItem(IDOK), m_nAssistRequireCount);
		}
	}
};

