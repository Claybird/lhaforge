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
