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

#include "stdafx.h"
#include "Dlg_jack.h"

//=================
// JACK一般設定画面
//=================
LRESULT CConfigDlgJACK::OnInitDialog(HWND hWnd, LPARAM lParam)
{
	// メッセージループにメッセージフィルタとアイドルハンドラを追加
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	pLoop->AddMessageFilter(this);

	//--------------------------------------------
	// ボリュームサイズを圧縮時に指定するかどうか
	//--------------------------------------------
	Radio_SpecifyVolumeSize[0]=GetDlgItem(IDC_RADIO_JACK_VOLUME_SIZE_USE_STORED_SETTING);
	Radio_SpecifyVolumeSize[1]=GetDlgItem(IDC_RADIO_JACK_VOLUME_SIZE_SPECIFY_AT_COMPRESS);
	if(m_Config.SpecifyVolumeSizeAtCompress){
		Radio_SpecifyVolumeSize[1].SetCheck(1);
	}
	else{
		Radio_SpecifyVolumeSize[0].SetCheck(1);
	}
	::EnableWindow(GetDlgItem(IDC_EDIT_JACK_VOLUME_SIZE),!m_Config.SpecifyVolumeSizeAtCompress);
	//ボリュームサイズ
	VolumeSize=m_Config.VolumeSize;
	//DDX情報アップデート
	DoDataExchange(FALSE);

	return TRUE;
}

LRESULT CConfigDlgJACK::OnApply()
{
//===============================
// 設定をConfigManagerに書き戻す
//===============================
	//--------------------------------------------
	// ボリュームサイズを圧縮時に指定するかどうか
	//--------------------------------------------
	m_Config.SpecifyVolumeSizeAtCompress=Radio_SpecifyVolumeSize[1].GetCheck();

	//---------------
	// DDXデータ更新
	//---------------
	if(!DoDataExchange(TRUE)){
		return FALSE;
	}
	if(VolumeSize<=0){
		::SetFocus(GetDlgItem(IDC_EDIT_JACK_VOLUME_SIZE));
		::SendMessage(GetDlgItem(IDC_EDIT_JACK_VOLUME_SIZE),EM_SETSEL,0,-1);
		ErrorMessage(CString(MAKEINTRESOURCE(IDS_ERROR_VALUE_MUST_BE_PLUS)));
		return FALSE;
	}
	m_Config.VolumeSize=VolumeSize;
	return TRUE;
}

LRESULT CConfigDlgJACK::OnRadioVolumeSize(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if(BN_CLICKED==wNotifyCode){
		::EnableWindow(GetDlgItem(IDC_EDIT_JACK_VOLUME_SIZE),Radio_SpecifyVolumeSize[0].GetCheck());
	}
	return 0;
}

