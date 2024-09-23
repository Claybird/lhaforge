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

class CTextInputDialog : public CDialogImpl<CTextInputDialog>,public LFWinDataExchange<CTextInputDialog>
{
protected:
	std::wstring strInput;
	const std::wstring strMessage;
	const bool _isPassphraseBox;
	const wchar_t hideChar;
public:
	enum {IDD = IDD_DIALOG_INPUT};
	CTextInputDialog(const std::wstring & message, bool isPassphraseBox)
		:strMessage(message), _isPassphraseBox(isPassphraseBox), hideChar(L'\u26AB'){}

	BEGIN_DDX_MAP(CTextInputDialog)
		DDX_TEXT(IDC_EDIT_INPUT,strInput)
	END_DDX_MAP()

	BEGIN_MSG_MAP_EX(CTextInputDialog)
		MSG_WM_INITDIALOG(OnInitDialog)
		COMMAND_ID_HANDLER_EX(IDOK, OnButton)
		COMMAND_ID_HANDLER_EX(IDCANCEL, OnButton)
		COMMAND_ID_HANDLER_EX(IDC_CHECK_SHOW, OnShowPassphrase)
	END_MSG_MAP()

	LRESULT OnInitDialog(HWND hWnd, LPARAM lParam){
		SetDlgItemText(IDC_STATIC_MESSAGE, strMessage.c_str());

		if (_isPassphraseBox) {
			::ShowWindow(GetDlgItem(IDC_CHECK_SHOW), SW_SHOW);
			CEdit hEdit = GetDlgItem(IDC_EDIT_INPUT);
			hEdit.SetPasswordChar(hideChar);
			hEdit.Invalidate();
		} else {
			::ShowWindow(GetDlgItem(IDC_CHECK_SHOW), SW_HIDE);
		}

		DoDataExchange(FALSE);
		return TRUE;
	}
	void OnButton(UINT uNotifyCode, int nID, HWND hWndCtl){
		DoDataExchange(TRUE);
		EndDialog(nID);
	}
	void OnShowPassphrase(UINT uNotifyCode, int nID, HWND hWndCtl) {
		if (_isPassphraseBox) {
			CEdit hEdit = GetDlgItem(IDC_EDIT_INPUT);
			if (BST_CHECKED==Button_GetCheck(GetDlgItem(IDC_CHECK_SHOW))) {
				hEdit.SetPasswordChar(L'\0');
			} else {
				hEdit.SetPasswordChar(hideChar);
			}
			hEdit.Invalidate();
		}
	}

	std::wstring GetInputText()const { return strInput; }
	void SetInputText(const std::wstring &input) { strInput = input; }
};

