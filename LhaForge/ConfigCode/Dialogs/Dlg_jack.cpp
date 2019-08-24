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

