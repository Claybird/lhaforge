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

const struct tagZIPVOLUMEUNIT{
	LPCTSTR DispName;
	LPCTSTR ParamName;
}ZIP_VOLUME_UNIT[]={
	{_T("MB"),_T("m")},
	{_T("KB"),_T("k")},
	{_T("GB"),_T("g")},
	{_T("Byte"),_T("b")}
};
const int ZIP_VOLUME_UNIT_MAX_NUM=COUNTOF(ZIP_VOLUME_UNIT);

class C7Zip32VolumeSizeDialog : public CDialogImpl<C7Zip32VolumeSizeDialog>,public CWinDataExchange<C7Zip32VolumeSizeDialog>
{
protected:
public:
	int VolumeSize;
	int SelectIndex;
	CComboBox Combo_VolumeUnit;
	enum {IDD = IDD_DIALOG_7Z_VOLUME_SIZE};

	// DDXマップ
	BEGIN_DDX_MAP(C7Zip32VolumeSizeDialog)
		DDX_INT(IDC_EDIT_7Z_VOLUME_SIZE, VolumeSize)
	END_DDX_MAP()

	// メッセージマップ
	BEGIN_MSG_MAP_EX(C7Zip32VolumeSizeDialog)
		MSG_WM_INITDIALOG(OnInitDialog)
		COMMAND_ID_HANDLER_EX(IDOK, OnOK)
		COMMAND_ID_HANDLER_EX(IDCANCEL, OnCancel)
	END_MSG_MAP()

	LRESULT OnInitDialog(HWND hWnd, LPARAM lParam){
		Combo_VolumeUnit=GetDlgItem(IDC_COMBO_7Z_VOLUME_UNIT);
		SelectIndex=0;
		for(int i=0;i<ZIP_VOLUME_UNIT_MAX_NUM;i++){
			Combo_VolumeUnit.InsertString(-1,ZIP_VOLUME_UNIT[i].DispName);
		}
		Combo_VolumeUnit.SetCurSel(0);
		return TRUE;
	}

	void OnOK(UINT uNotifyCode, int nID, HWND hWndCtl){
		if(!DoDataExchange(TRUE)){
			return;
		}
		if(VolumeSize<=0){
			::SetFocus(GetDlgItem(IDC_EDIT_7Z_VOLUME_SIZE));
			::SendMessage(GetDlgItem(IDC_EDIT_7Z_VOLUME_SIZE),EM_SETSEL,0,-1);
			ErrorMessage(CString(MAKEINTRESOURCE(IDS_ERROR_VALUE_MUST_BE_PLUS)));
			return;
		}
		SelectIndex=Combo_VolumeUnit.GetCurSel();
		if(SelectIndex<0||SelectIndex>=ZIP_VOLUME_UNIT_MAX_NUM)return;
		EndDialog(nID);
	}

	void OnCancel(UINT uNotifyCode, int nID, HWND hWndCtl){
		EndDialog(nID);
	}
};

