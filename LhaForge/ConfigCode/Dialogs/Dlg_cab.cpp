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
#include "Dlg_cab.h"

//=================
// CAB一般設定画面
//=================
LRESULT CConfigDlgCAB::OnInitDialog(HWND hWnd, LPARAM lParam)
{
	// メッセージループにメッセージフィルタとアイドルハンドラを追加
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	pLoop->AddMessageFilter(this);

	//----------------
	// 圧縮形式の選択
	//----------------
	Radio_CompressType[CAB_COMPRESS_MSZIP]=GetDlgItem(IDC_RADIO_CAB_MSZIP);
	Radio_CompressType[CAB_COMPRESS_LZX]=GetDlgItem(IDC_RADIO_CAB_LZX);
	Radio_CompressType[CAB_COMPRESS_PLAIN]=GetDlgItem(IDC_RADIO_CAB_PLAIN);

	Radio_CompressType[m_Config.CompressType].SetCheck(1);

	//-------------------------------
	// LZX圧縮レベルの設定用スライダ
	//-------------------------------
	Track_LZX_Level=GetDlgItem(IDC_SLIDER_CAB_LZX_LEVEL);
	Track_LZX_Level.SetRange(CAB_LZX_LOWEST,CAB_LZX_HIGHEST);
	Track_LZX_Level.SetTicFreq(1);
	Track_LZX_Level.SetPageSize(1);
	Track_LZX_Level.SetLineSize(1);
	Track_LZX_Level.SetPos(m_Config.LZX_Level);

	//---------------------------------
	// LZX圧縮レベルの確認用エディット
	//---------------------------------
	Edit_LZX_Level=GetDlgItem(IDC_EDIT_CAB_LZX_LEVEL);
	CString Buffer;
	Buffer.Format(_T("%d"),m_Config.LZX_Level);
	Edit_LZX_Level.SetWindowText(Buffer);

	//LZXが選択されていなければ圧縮レベルの設定は無効にしておく
	bool bActive=(0!=Radio_CompressType[CAB_COMPRESS_LZX].GetCheck());
	Track_LZX_Level.EnableWindow(bActive);
	Edit_LZX_Level.EnableWindow(bActive);

	return TRUE;
}

LRESULT CConfigDlgCAB::OnApply()
{
//===============================
// 設定をConfigManagerに書き戻す
//===============================
	//----------------
	// 圧縮形式の選択
	//----------------
	for(int Type=0;Type<COUNTOF(Radio_CompressType);Type++){
		if(Radio_CompressType[Type].GetCheck()){
			m_Config.CompressType=(CAB_COMPRESS_TYPE)Type;
			break;
		}
	}

	//---------------
	// LZX圧縮レベル
	//---------------
	m_Config.LZX_Level=Track_LZX_Level.GetPos();

	return TRUE;
}


void CConfigDlgCAB::OnHScroll(int, short, HWND)
{
	CString Buffer;
	Buffer.Format(_T("%d"),Track_LZX_Level.GetPos());
	Edit_LZX_Level.SetWindowText(Buffer);
}

LRESULT CConfigDlgCAB::OnRadioCompressType(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if(BN_CLICKED==wNotifyCode){
		bool bActive=(0!=Radio_CompressType[CAB_COMPRESS_LZX].GetCheck());
		Track_LZX_Level.EnableWindow(bActive);
		Edit_LZX_Level.EnableWindow(bActive);
	}
	return 0;
}
