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
#include "resource.h"

class CWaitDialog : public CDialogImpl<CWaitDialog>,public CMessageFilter, public CIdleHandler
{
protected:
	CStatic	Static_MessageString;
	bool m_bAborted;
protected:
	BEGIN_MSG_MAP_EX(CMainDlg)
		MSG_WM_INITDIALOG(OnInitDialog)
		MSG_WM_DESTROY(OnDestroy)
		COMMAND_ID_HANDLER_EX(IDOK, __noop)
		COMMAND_ID_HANDLER_EX(IDCANCEL, OnCancel)
	END_MSG_MAP()

    LRESULT OnInitDialog(HWND hWnd, LPARAM lParam) {
		m_bAborted = false;
		Static_MessageString = GetDlgItem(IDC_STATIC_WAIT_MESSAGE);

		CenterWindow();

		CMessageLoop* pLoop = _Module.GetMessageLoop();
		pLoop->AddMessageFilter(this);
		pLoop->AddIdleHandler(this);
		return TRUE;
	}

	void OnDestroy() {
		CMessageLoop* pLoop = _Module.GetMessageLoop();
		pLoop->RemoveMessageFilter(this);
		pLoop->RemoveIdleHandler(this);
	}

	BOOL PreTranslateMessage(MSG* pMsg){
		return IsDialogMessage(pMsg);
	}

	void OnCancel(UINT uNotifyCode, int nID, HWND hWndCtl){
		m_bAborted=true;
	}

	BOOL OnIdle(){
		return FALSE;
	}
public:
	enum {IDD = IDD_DIALOG_WAIT};
	void SetMessageString(LPCTSTR t){Static_MessageString.SetWindowText(t);}

	void Prepare(HWND hWndParent, LPCTSTR lpMsg) {
		Create(hWndParent);
		SetMessageString(lpMsg);
		ShowWindow(SW_SHOW);
		UpdateWindow();
	}
	bool IsAborted(){return m_bAborted;}
};

