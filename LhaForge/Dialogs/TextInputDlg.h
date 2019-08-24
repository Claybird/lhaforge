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

class CInputDialog : public CDialogImpl<CInputDialog>,public CWinDataExchange<CInputDialog>
{
protected:
	CString &strInput;
	CString strMessage;
public:
	enum {IDD = IDD_DIALOG_INPUT};
	CInputDialog(LPCTSTR lpszMessage,CString &str):strInput(str),strMessage(lpszMessage){}

	// DDXマップ
	BEGIN_DDX_MAP(CInputDialog)
		DDX_TEXT(IDC_EDIT_INPUT,strInput)
	END_DDX_MAP()

	// メッセージマップ
	BEGIN_MSG_MAP_EX(CInputDialog)
		MSG_WM_INITDIALOG(OnInitDialog)
		COMMAND_ID_HANDLER_EX(IDOK, OnButton)
		COMMAND_ID_HANDLER_EX(IDCANCEL, OnButton)
	END_MSG_MAP()

	LRESULT OnInitDialog(HWND hWnd, LPARAM lParam){
		//メッセージ設定
		CStatic cStatic=GetDlgItem(IDC_STATIC_MESSAGE);
		cStatic.SetWindowText(strMessage);

		//DDX情報設定
		DoDataExchange(FALSE);
		return TRUE;
	}
	void OnButton(UINT uNotifyCode, int nID, HWND hWndCtl){
		//DDX情報設定
		DoDataExchange(TRUE);
		EndDialog(nID);
	}
};
