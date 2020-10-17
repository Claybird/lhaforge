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
#include "Utilities/Utility.h"

//====================================
// ショートカット作成
//====================================
class CConfigDlgShortcut : public LFConfigDialogBase<CConfigDlgShortcut>
{
protected:
	virtual BOOL PreTranslateMessage(MSG* pMsg){
		return IsDialogMessage(pMsg);
	}

public:
	enum { IDD = IDD_PROPPAGE_SHORTCUT };

	// メッセージマップ
	BEGIN_MSG_MAP_EX(CConfigDlgShortcut)
		MSG_WM_INITDIALOG(OnInitDialog)
		COMMAND_RANGE_HANDLER(IDC_BUTTON_CREATE_EXTRACT_SHORTCUT_DESKTOP,IDC_BUTTON_CREATE_EXTRACT_SHORTCUT_SENDTO, OnCreateShortcut)
		COMMAND_RANGE_HANDLER(IDC_BUTTON_CREATE_COMPRESS_SHORTCUT_DESKTOP,IDC_BUTTON_CREATE_COMPRESS_SHORTCUT_SENDTO, OnCreateShortcut)
		COMMAND_RANGE_HANDLER(IDC_BUTTON_CREATE_AUTOMATIC_SHORTCUT_DESKTOP,IDC_BUTTON_CREATE_AUTOMATIC_SHORTCUT_SENDTO, OnCreateShortcut)
		COMMAND_RANGE_HANDLER(IDC_BUTTON_CREATE_LIST_SHORTCUT_DESKTOP,IDC_BUTTON_CREATE_LIST_SHORTCUT_SENDTO, OnCreateShortcut)
		COMMAND_RANGE_HANDLER(IDC_BUTTON_CREATE_TESTARCHIVE_SHORTCUT_DESKTOP,IDC_BUTTON_CREATE_TESTARCHIVE_SHORTCUT_SENDTO, OnCreateShortcut)
		MSG_WM_DESTROY(OnDestroy)
	END_MSG_MAP()

	LRESULT OnInitDialog(HWND hWnd, LPARAM lParam);
	LRESULT OnCreateShortcut(WORD,WORD,HWND,BOOL&);
	bool GetCompressShortcutInfo(LPTSTR,CString&);
	LRESULT OnApply(){return TRUE;}

	CConfigDlgShortcut(){
		TRACE(_T("CConfigDlgShortcut()\n"));
	}

	LRESULT OnDestroy(){
		CMessageLoop* pLoop = _Module.GetMessageLoop();
		pLoop->RemoveMessageFilter(this);

		return TRUE;
	}
	void LoadConfig(CConfigManager& Config){}
	void StoreConfig(CConfigManager& Config){}
};

