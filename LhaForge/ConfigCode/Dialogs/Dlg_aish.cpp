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

