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
//ログ表示ウィンドウ
#include "../resource.h"

class CLogDialog : public CDialogImpl<CLogDialog>,public CDialogResize<CLogDialog>
{
protected:
	CEdit LogView;
	CString LogStr;
public:
	enum {IDD = IDD_DIALOG_LOG};

	// ダイアログリサイズマップ
	BEGIN_DLGRESIZE_MAP(CLogDialog)
		DLGRESIZE_CONTROL(IDC_EDIT_LOG, DLSZ_SIZE_X | DLSZ_SIZE_Y)
	END_DLGRESIZE_MAP()

	// メッセージマップ
	BEGIN_MSG_MAP_EX(CLogDialog)
		MSG_WM_INITDIALOG(OnInitDialog)
		COMMAND_ID_HANDLER_EX(IDOK, OnOK)
		COMMAND_ID_HANDLER_EX(IDCANCEL, OnCancel)
		CHAIN_MSG_MAP(CDialogResize<CLogDialog>)    // CDialogResizeクラスへのチェーン
	END_MSG_MAP()

	void SetData(LPCTSTR log);
	LRESULT OnInitDialog(HWND hWnd, LPARAM lParam);
	void OnOK(UINT uNotifyCode, int nID, HWND hWndCtl){
		EndDialog(nID);
	}

	void OnCancel(UINT uNotifyCode, int nID, HWND hWndCtl){
		EndDialog(nID);
	}
};

