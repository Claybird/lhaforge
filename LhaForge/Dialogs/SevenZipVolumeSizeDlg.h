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

