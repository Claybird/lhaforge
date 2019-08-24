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
#include "Dlg_AISH.h"

//=================
// AISH一般設定画面
//=================
LRESULT CConfigDlgAISH::OnInitDialog(HWND hWnd, LPARAM lParam)
{
	// メッセージループにメッセージフィルタとアイドルハンドラを追加
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	pLoop->AddMessageFilter(this);

	//------------------
	// エンコードの選択
	//------------------
	Radio_EncodeType[ISH_SHIFT_JIS]=GetDlgItem(IDC_RADIO_ISH_SHIFT_JIS);
	Radio_EncodeType[ISH_JIS7]=GetDlgItem(IDC_RADIO_ISH_JIS7);
	Radio_EncodeType[ISH_JIS8]=GetDlgItem(IDC_RADIO_ISH_JIS8);
	Radio_EncodeType[ISH_NON_KANA_SHIFT_JIS]=GetDlgItem(IDC_RADIO_ISH_NON_KANA_SHIFT_JIS);
	Radio_EncodeType[ISH_JIS7]=GetDlgItem(IDC_RADIO_ISH_JIS7);

	Radio_EncodeType[m_Config.EncodeType].SetCheck(1);

	//------------------------
	// uuencodeのチェックサム
	//------------------------
	Check_UUEncodeChecksum=GetDlgItem(IDC_CHECK_ISH_UUENCODE_CHECKSUM);
	Check_UUEncodeChecksum.SetCheck(m_Config.UUEncodeChecksum);

	return TRUE;
}

LRESULT CConfigDlgAISH::OnApply()
{
//===============================
// 設定をConfigManagerに書き戻す
//===============================
	//------------------
	// エンコードの選択
	//------------------
	for(int i=0;i<COUNTOF(Radio_EncodeType);i++){
		if(Radio_EncodeType[i].GetCheck()){
			m_Config.EncodeType=(ISH_ENCODE_TYPE)i;
			break;
		}
	}
	//------------------------
	// uuencodeのチェックサム
	//------------------------
	m_Config.UUEncodeChecksum=Check_UUEncodeChecksum.GetCheck();

	return TRUE;
}

