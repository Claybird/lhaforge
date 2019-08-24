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

