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
#include "Dlg_lzh.h"

//=================
// LZH一般設定画面
//=================
LRESULT CConfigDlgLZH::OnInitDialog(HWND hWnd, LPARAM lParam)
{
	// メッセージループにメッセージフィルタとアイドルハンドラを追加
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	pLoop->AddMessageFilter(this);

	//----------------
	// 圧縮形式の選択
	//----------------
	Radio_CompressType[LZH_COMPRESS_LH0]=GetDlgItem(IDC_RADIO_LZH_LH0);
	Radio_CompressType[LZH_COMPRESS_LH1]=GetDlgItem(IDC_RADIO_LZH_LH1);
	Radio_CompressType[LZH_COMPRESS_LH5]=GetDlgItem(IDC_RADIO_LZH_LH5);
	Radio_CompressType[LZH_COMPRESS_LH6]=GetDlgItem(IDC_RADIO_LZH_LH6);
	Radio_CompressType[LZH_COMPRESS_LH7]=GetDlgItem(IDC_RADIO_LZH_LH7);

	Radio_CompressType[m_Config.CompressType].SetCheck(1);

	//------------------------------------------------
	// 自己解凍ファイルを作成する際設定を行うかどうか
	//------------------------------------------------
	Check_ConfigSFX=GetDlgItem(IDC_CHECK_LZH_SFX_CONFIG);
	Check_ConfigSFX.SetCheck(0!=m_Config.ConfigSFX);

	return TRUE;
}

LRESULT CConfigDlgLZH::OnApply()
{
//===============================
// 設定をConfigManagerに書き戻す
//===============================
	//----------------
	// 圧縮形式の選択
	//----------------
	for(int Type=0;Type<COUNTOF(Radio_CompressType);Type++){
		if(Radio_CompressType[Type].GetCheck()){
			m_Config.CompressType=(LZH_COMPRESS_TYPE)Type;
			break;
		}
	}
	//------------------------------------------------
	// 自己解凍ファイルを作成する際設定を行うかどうか
	//------------------------------------------------
	m_Config.ConfigSFX=Check_ConfigSFX.GetCheck();

	return TRUE;
}

