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

