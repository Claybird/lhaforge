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

class CTextInputDialog : public CDialogImpl<CTextInputDialog>,public CWinDataExchange<CTextInputDialog>
{
protected:
	CString strInput;
	CString strMessage;
public:
	enum {IDD = IDD_DIALOG_INPUT};
	CTextInputDialog(LPCTSTR lpszMessage):strMessage(lpszMessage){}

	BEGIN_DDX_MAP(CTextInputDialog)
		DDX_TEXT(IDC_EDIT_INPUT,strInput)
	END_DDX_MAP()

	BEGIN_MSG_MAP_EX(CTextInputDialog)
		MSG_WM_INITDIALOG(OnInitDialog)
		COMMAND_ID_HANDLER_EX(IDOK, OnButton)
		COMMAND_ID_HANDLER_EX(IDCANCEL, OnButton)
	END_MSG_MAP()

	LRESULT OnInitDialog(HWND hWnd, LPARAM lParam){
		CStatic cStatic=GetDlgItem(IDC_STATIC_MESSAGE);
		cStatic.SetWindowText(strMessage);

		DoDataExchange(FALSE);
		return TRUE;
	}
	void OnButton(UINT uNotifyCode, int nID, HWND hWndCtl){
		DoDataExchange(TRUE);
		EndDialog(nID);
	}
	std::wstring GetInputText()const { return (LPCWSTR)strInput; }
	void SetInputText(const std::wstring &input) { strInput = input.c_str(); }
};

