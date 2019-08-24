/*
 * Copyright (c) 2005-, Claybird
 * All rights reserved.

 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:

 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Claybird nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.

 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
 * THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
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
