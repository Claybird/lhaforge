/*
 * Copyright (c) 2005-2012, Claybird
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
#include "Dlg_HKI.h"

//=================
// HKI一般設定画面
//=================
LRESULT CConfigDlgHKI::OnInitDialog(HWND hWnd, LPARAM lParam)
{
	// メッセージループにメッセージフィルタとアイドルハンドラを追加
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	pLoop->AddMessageFilter(this);

	//------------------
	// 圧縮レベルの選択
	//------------------
	Radio_CompressLevel[HKI_COMPRESS_LEVEL_NONE]=GetDlgItem(IDC_RADIO_HKI_LV0);
	Radio_CompressLevel[HKI_COMPRESS_LEVEL_FAST]=GetDlgItem(IDC_RADIO_HKI_LV1);
	Radio_CompressLevel[HKI_COMPRESS_LEVEL_NORMAL]=GetDlgItem(IDC_RADIO_HKI_LV2);
	Radio_CompressLevel[HKI_COMPRESS_LEVEL_HIGH]=GetDlgItem(IDC_RADIO_HKI_LV3);

	Radio_CompressLevel[m_Config.CompressLevel].SetCheck(1);

	//--------------------------
	// 暗号化アルゴリズムの選択
	//--------------------------
	Radio_EncryptAlgorithm[HKI_ENCRYPT_NONE]=GetDlgItem(IDC_RADIO_HKI_ENCRYPT0);
	Radio_EncryptAlgorithm[HKI_ENCRYPT_RIJNDAEL128]=GetDlgItem(IDC_RADIO_HKI_ENCRYPT1);
	Radio_EncryptAlgorithm[HKI_ENCRYPT_RIJNDAEL256]=GetDlgItem(IDC_RADIO_HKI_ENCRYPT2);
	Radio_EncryptAlgorithm[HKI_ENCRYPT_SINGLE_DES]=GetDlgItem(IDC_RADIO_HKI_ENCRYPT3);
	Radio_EncryptAlgorithm[HKI_ENCRYPT_TRIPLE_DES]=GetDlgItem(IDC_RADIO_HKI_ENCRYPT4);
	Radio_EncryptAlgorithm[HKI_ENCRYPT_BLOWFISH448]=GetDlgItem(IDC_RADIO_HKI_ENCRYPT5);
	Radio_EncryptAlgorithm[HKI_ENCRYPT_TWOFISH128]=GetDlgItem(IDC_RADIO_HKI_ENCRYPT6);
	Radio_EncryptAlgorithm[HKI_ENCRYPT_TWOFISH256]=GetDlgItem(IDC_RADIO_HKI_ENCRYPT7);
	Radio_EncryptAlgorithm[HKI_ENCRYPT_SQUARE]=GetDlgItem(IDC_RADIO_HKI_ENCRYPT8);

	Radio_EncryptAlgorithm[m_Config.EncryptAlgorithm].SetCheck(1);

	return TRUE;
}

LRESULT CConfigDlgHKI::OnApply()
{
//===============================
// 設定をConfigManagerに書き戻す
//===============================
	//------------------
	// 圧縮レベルの選択
	//------------------
	for(int i=0;i<COUNTOF(Radio_CompressLevel);i++){
		if(Radio_CompressLevel[i].GetCheck()){
			m_Config.CompressLevel=(HKI_COMPRESS_LEVEL)i;
			break;
		}
	}
	//--------------------------
	// 暗号化アルゴリズムの選択
	//--------------------------
	for(int i=0;i<COUNTOF(Radio_EncryptAlgorithm);i++){
		if(Radio_EncryptAlgorithm[i].GetCheck()){
			m_Config.EncryptAlgorithm=(HKI_ENCRYPT_ALGORITHM)i;
			break;
		}
	}

	return TRUE;
}

