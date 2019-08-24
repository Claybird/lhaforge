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

class CJackVolumeSizeDialog : public CDialogImpl<CJackVolumeSizeDialog>,public CWinDataExchange<CJackVolumeSizeDialog>
{
protected:
public:
	int VolumeSize;
	enum {IDD = IDD_DIALOG_JACK_SPECIFY_VOLUME_SIZE};

	// DDXマップ
	BEGIN_DDX_MAP(CJackVolumeSizeDialog)
		DDX_INT(IDC_EDIT_JACK_VOLUME_SIZE_AT_COMPRESS, VolumeSize)
	END_DDX_MAP()

	// メッセージマップ
	BEGIN_MSG_MAP_EX(CJackVolumeSizeDialog)
		COMMAND_ID_HANDLER_EX(IDOK, OnOK)
		COMMAND_ID_HANDLER_EX(IDCANCEL, OnCancel)
	END_MSG_MAP()

	void OnOK(UINT uNotifyCode, int nID, HWND hWndCtl){
		if(!DoDataExchange(TRUE)){
			return;
		}
		if(VolumeSize<=0){
			::SetFocus(GetDlgItem(IDC_EDIT_JACK_VOLUME_SIZE_AT_COMPRESS));
			::SendMessage(GetDlgItem(IDC_EDIT_JACK_VOLUME_SIZE_AT_COMPRESS),EM_SETSEL,0,-1);
			ErrorMessage(CString(MAKEINTRESOURCE(IDS_ERROR_VALUE_MUST_BE_PLUS)));
			return;
		}
		EndDialog(nID);
	}

	void OnCancel(UINT uNotifyCode, int nID, HWND hWndCtl){
		EndDialog(nID);
	}
};

