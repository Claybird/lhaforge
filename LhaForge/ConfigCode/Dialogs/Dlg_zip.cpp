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
#include "Dlg_zip.h"
#include "../../Dialogs/SevenZipVolumeSizeDlg.h"

//=================
// ZIP一般設定画面
//=================
LRESULT CConfigDlgZIP::OnInitDialog(HWND hWnd, LPARAM lParam)
{
	// メッセージループにメッセージフィルタとアイドルハンドラを追加
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	pLoop->AddMessageFilter(this);
	//----------------
	// 圧縮形式の選択
	//----------------
	Radio_CompressType[ZIP_COMPRESS_DEFLATE]=GetDlgItem(IDC_RADIO_ZIP_DEFLATE);
	Radio_CompressType[ZIP_COMPRESS_DEFLATE64]=GetDlgItem(IDC_RADIO_ZIP_DEFLATE64);
	Radio_CompressType[ZIP_COMPRESS_BZIP2]=GetDlgItem(IDC_RADIO_ZIP_BZIP2);
	Radio_CompressType[ZIP_COMPRESS_COPY]=GetDlgItem(IDC_RADIO_ZIP_COPY);
	Radio_CompressType[ZIP_COMPRESS_LZMA]=GetDlgItem(IDC_RADIO_ZIP_LZMA);
	Radio_CompressType[ZIP_COMPRESS_PPMD]=GetDlgItem(IDC_RADIO_ZIP_PPMD);

	Radio_CompressType[m_Config.CompressType].SetCheck(1);

	//------------------
	// 圧縮レベルの選択
	//------------------
	Radio_CompressLevel[ZIP_COMPRESS_LEVEL0]=GetDlgItem(IDC_RADIO_ZIP_COMPRESS_LEVEL_0);
	Radio_CompressLevel[ZIP_COMPRESS_LEVEL5]=GetDlgItem(IDC_RADIO_ZIP_COMPRESS_LEVEL_5);
	Radio_CompressLevel[ZIP_COMPRESS_LEVEL9]=GetDlgItem(IDC_RADIO_ZIP_COMPRESS_LEVEL_9);

	Radio_CompressLevel[m_Config.CompressLevel].SetCheck(1);


	//---------------
	// Deflateの設定
	//---------------
	//メモリサイズ
	::SendMessage(GetDlgItem(IDC_EDIT_ZIP_DEFLATE_MEMORY_SIZE),EM_LIMITTEXT,3,NULL);
	Check_SpecifyDeflateMemorySize=GetDlgItem(IDC_CHECK_ZIP_SPECIFY_MEMORY_SIZE);
	Check_SpecifyDeflateMemorySize.SetCheck(m_Config.SpecifyDeflateMemorySize);
	::EnableWindow(GetDlgItem(IDC_EDIT_ZIP_DEFLATE_MEMORY_SIZE),m_Config.SpecifyDeflateMemorySize);
	//パス数
	::SendMessage(GetDlgItem(IDC_EDIT_ZIP_DEFLATE_PASS_NUMBER),EM_LIMITTEXT,1,NULL);
	Check_SpecifyDeflatePassNumber=GetDlgItem(IDC_CHECK_ZIP_SPECIFY_PASS_NUMBER);
	Check_SpecifyDeflatePassNumber.SetCheck(m_Config.SpecifyDeflatePassNumber);
	::EnableWindow(GetDlgItem(IDC_EDIT_ZIP_DEFLATE_PASS_NUMBER),m_Config.SpecifyDeflatePassNumber);

	//------------------
	// 分割サイズの設定
	//------------------
	::EnableWindow(GetDlgItem(IDC_EDIT_ZIP_SPLIT_SIZE), m_Config.SpecifySplitSize);
	::EnableWindow(GetDlgItem(IDC_COMBO_ZIP_SPLIT_SIZE_UNIT), m_Config.SpecifySplitSize);

	Combo_SizeUnit=GetDlgItem(IDC_COMBO_ZIP_SPLIT_SIZE_UNIT);
	for(int i=0;i<ZIP_VOLUME_UNIT_MAX_NUM;i++){
		Combo_SizeUnit.InsertString(-1,ZIP_VOLUME_UNIT[i].DispName);
	}
	Combo_SizeUnit.SetCurSel(m_Config.SplitSizeUnit);

	//DDX情報アップデート
	DoDataExchange(FALSE);

	return TRUE;
}

LRESULT CConfigDlgZIP::OnApply()
{
//===============================
// 設定をConfigManagerに書き戻す
//===============================
	//----------------
	// 圧縮形式の選択
	//----------------
	for(int Type=0;Type<COUNTOF(Radio_CompressType);Type++){
		if(Radio_CompressType[Type].GetCheck()){
			m_Config.CompressType=(ZIP_COMPRESS_TYPE)Type;
			break;
		}
	}

	//------------------
	// 圧縮レベルの選択
	//------------------
	for(int Type=0;Type<COUNTOF(Radio_CompressLevel);Type++){
		if(Radio_CompressLevel[Type].GetCheck()){
			m_Config.CompressLevel=(ZIP_COMPRESS_LEVEL)Type;
			break;
		}
	}

	//------------------
	// 分割サイズの設定
	//------------------
	m_Config.SplitSizeUnit = Combo_SizeUnit.GetCurSel();

	//---------------
	// DDXデータ更新
	//---------------
	if(!DoDataExchange(TRUE)){
		return FALSE;
	}
	//---------------
	// Deflateの設定
	//---------------
	//メモリサイズ
	m_Config.SpecifyDeflateMemorySize=Check_SpecifyDeflateMemorySize.GetCheck();
	//パス数
	m_Config.SpecifyDeflatePassNumber=Check_SpecifyDeflatePassNumber.GetCheck();
	return TRUE;
}

LRESULT CConfigDlgZIP::OnSpecifyDeflateMemorySize(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if(BN_CLICKED==wNotifyCode){
		::EnableWindow(GetDlgItem(IDC_EDIT_ZIP_DEFLATE_MEMORY_SIZE),Check_SpecifyDeflateMemorySize.GetCheck());
	}
	return 0;
}

LRESULT CConfigDlgZIP::OnSpecifyDeflatePassNumber(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if(BN_CLICKED==wNotifyCode){
		::EnableWindow(GetDlgItem(IDC_EDIT_ZIP_DEFLATE_PASS_NUMBER),Check_SpecifyDeflatePassNumber.GetCheck());
	}
	return 0;
}

LRESULT CConfigDlgZIP::OnSpecifySplitSize(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if(BN_CLICKED==wNotifyCode){
		::EnableWindow(GetDlgItem(IDC_EDIT_ZIP_SPLIT_SIZE), Button_GetState(GetDlgItem(IDC_CHECK_ZIP_SPECIFY_SPLIT_SIZE)));
		::EnableWindow(GetDlgItem(IDC_COMBO_ZIP_SPLIT_SIZE_UNIT), Button_GetState(GetDlgItem(IDC_CHECK_ZIP_SPECIFY_SPLIT_SIZE)));
	}
	return 0;
}

