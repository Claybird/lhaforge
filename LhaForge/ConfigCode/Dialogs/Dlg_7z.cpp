﻿/*
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
#include "Dlg_7z.h"
#include "../../Dialogs/SevenZipVolumeSizeDlg.h"

//=================
// 7Z一般設定画面
//=================
LRESULT CConfigDlg7Z::OnInitDialog(HWND hWnd, LPARAM lParam)
{
	// メッセージループにメッセージフィルタとアイドルハンドラを追加
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	pLoop->AddMessageFilter(this);

	//----------------
	// 圧縮形式の選択
	//----------------
	//プリセットを使うか
	Check_UsePreset=GetDlgItem(IDC_CHECK_7Z_USE_PRESET);
	Check_UsePreset.SetCheck(m_Config.UsePreset);
	//手動設定するか
	Radio_CompressType[SEVEN_ZIP_COMPRESS_LZMA]=GetDlgItem(IDC_RADIO_7Z_METHOD_LZMA);
	Radio_CompressType[SEVEN_ZIP_COMPRESS_PPMD]=GetDlgItem(IDC_RADIO_7Z_METHOD_PPMD);
	Radio_CompressType[SEVEN_ZIP_COMPRESS_BZIP2]=GetDlgItem(IDC_RADIO_7Z_METHOD_BZIP2);
	Radio_CompressType[SEVEN_ZIP_COMPRESS_DEFLATE]=GetDlgItem(IDC_RADIO_7Z_METHOD_DEFLATE);
	Radio_CompressType[SEVEN_ZIP_COMPRESS_COPY]=GetDlgItem(IDC_RADIO_7Z_METHOD_COPY);
	Radio_CompressType[SEVEN_ZIP_COMPRESS_LZMA2]=GetDlgItem(IDC_RADIO_7Z_METHOD_LZMA2);

	Radio_CompressType[m_Config.CompressType].SetCheck(1);

	//プリセットを使う場合には無効に
	for(int Type=0;Type<COUNTOF(Radio_CompressType);Type++){
		Radio_CompressType[Type].EnableWindow(!m_Config.UsePreset);
	}

	//------------------------------
	// 圧縮レベルの選択(プリセット)
	//------------------------------
	Radio_CompressLevel[SEVEN_ZIP_COMPRESS_LEVEL0]=GetDlgItem(IDC_RADIO_7Z_COMPRESS_LEVEL_0);
	Radio_CompressLevel[SEVEN_ZIP_COMPRESS_LEVEL1]=GetDlgItem(IDC_RADIO_7Z_COMPRESS_LEVEL_1);
	Radio_CompressLevel[SEVEN_ZIP_COMPRESS_LEVEL5]=GetDlgItem(IDC_RADIO_7Z_COMPRESS_LEVEL_5);
	Radio_CompressLevel[SEVEN_ZIP_COMPRESS_LEVEL7]=GetDlgItem(IDC_RADIO_7Z_COMPRESS_LEVEL_7);
	Radio_CompressLevel[SEVEN_ZIP_COMPRESS_LEVEL9]=GetDlgItem(IDC_RADIO_7Z_COMPRESS_LEVEL_9);

	Radio_CompressLevel[m_Config.CompressLevel].SetCheck(1);
	//プリセットを使わない場合には無効に
	for(int Type=0;Type<COUNTOF(Radio_CompressLevel);Type++){
		Radio_CompressLevel[Type].EnableWindow(m_Config.UsePreset);
	}

	//----------------------
	// LZMA圧縮モードの選択
	//----------------------
	Radio_LZMA_Mode[SEVEN_ZIP_LZMA_MODE0]=GetDlgItem(IDC_RADIO_7Z_LZMA_MODE0);
	Radio_LZMA_Mode[SEVEN_ZIP_LZMA_MODE1]=GetDlgItem(IDC_RADIO_7Z_LZMA_MODE1);

	Radio_LZMA_Mode[m_Config.LZMA_Mode].SetCheck(1);
	//プリセットを使う場合には無効に
	for(int Type=0;Type<COUNTOF(Radio_LZMA_Mode);Type++){
		Radio_LZMA_Mode[Type].EnableWindow((SEVEN_ZIP_COMPRESS_LZMA==m_Config.CompressType || SEVEN_ZIP_COMPRESS_LZMA2==m_Config.CompressType)&&(!m_Config.UsePreset));
	}

	//------------
	// ヘッダ圧縮
	//------------
	Check_HeaderCompression=GetDlgItem(IDC_CHECK_7Z_HEADER_COMPRESSION);

	//------------
	// PPMdの設定
	//------------
	//モデルサイズ
	::SendMessage(GetDlgItem(IDC_EDIT_7Z_PPMD_MODEL_SIZE),EM_LIMITTEXT,2,NULL);
	Check_SpecifyPPMdModelSize=GetDlgItem(IDC_CHECK_7Z_SPECIFY_PPMD_MODEL_SIZE);
	Check_SpecifyPPMdModelSize.SetCheck(m_Config.SpecifyPPMdModelSize);
	//プリセットを使う場合には無効に
	Check_SpecifyPPMdModelSize.EnableWindow((SEVEN_ZIP_COMPRESS_PPMD==m_Config.CompressType)&&!m_Config.UsePreset);
	::EnableWindow(GetDlgItem(IDC_EDIT_7Z_PPMD_MODEL_SIZE),(SEVEN_ZIP_COMPRESS_PPMD==m_Config.CompressType)&&m_Config.SpecifyPPMdModelSize&&!m_Config.UsePreset);

	//------------------
	// 分割サイズの設定
	//------------------
	::EnableWindow(GetDlgItem(IDC_EDIT_7Z_SPLIT_SIZE), m_Config.SpecifySplitSize);
	::EnableWindow(GetDlgItem(IDC_COMBO_7Z_SPLIT_SIZE_UNIT), m_Config.SpecifySplitSize);

	Combo_SizeUnit=GetDlgItem(IDC_COMBO_7Z_SPLIT_SIZE_UNIT);
	for(int i=0;i<ZIP_VOLUME_UNIT_MAX_NUM;i++){
		Combo_SizeUnit.InsertString(-1,ZIP_VOLUME_UNIT[i].DispName);
	}
	Combo_SizeUnit.SetCurSel(m_Config.SplitSizeUnit);

	//DDX情報アップデート
	DoDataExchange(FALSE);

	//ヘッダ完全圧縮の設定
	::EnableWindow(GetDlgItem(IDC_CHECK_7Z_FULL_HEADER_COMPRESSION),Check_HeaderCompression.GetCheck());

	return TRUE;
}

LRESULT CConfigDlg7Z::OnApply()
{
//===============================
// 設定をConfigManagerに書き戻す
//===============================
	//----------------
	// 圧縮形式の選択
	//----------------
	//プリセットを使うかどうか
	m_Config.UsePreset=Check_UsePreset.GetCheck();

	//手動設定の場合
	for(int Type=0;Type<COUNTOF(Radio_CompressType);Type++){
		if(Radio_CompressType[Type].GetCheck()){
			m_Config.CompressType=(SEVEN_ZIP_COMPRESS_TYPE)Type;
			break;
		}
	}

	//------------------
	// 圧縮レベルの選択
	//------------------
	for(int Type=0;Type<COUNTOF(Radio_CompressLevel);Type++){
		if(Radio_CompressLevel[Type].GetCheck()){
			m_Config.CompressLevel=(SEVEN_ZIP_COMPRESS_LEVEL)Type;
			break;
		}
	}

	//----------------------
	// LZMA圧縮モードの選択
	//----------------------
	for(int Type=0;Type<COUNTOF(Radio_LZMA_Mode);Type++){
		if(Radio_LZMA_Mode[Type].GetCheck()){
			m_Config.LZMA_Mode=(SEVEN_ZIP_LZMA_MODE)Type;
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
	// PPMdの設定
	//---------------
	//モデルサイズ
	m_Config.SpecifyPPMdModelSize=Check_SpecifyPPMdModelSize.GetCheck();
//	m_Config.PPMdModelSize=PPMdModelSize;
	return TRUE;
}

LRESULT CConfigDlg7Z::OnUsePreset(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if(BN_CLICKED==wNotifyCode){
		for(int Type=0;Type<COUNTOF(Radio_CompressType);Type++){
			Radio_CompressType[Type].EnableWindow(!Check_UsePreset.GetCheck());
		}
		for(int Type=0;Type<COUNTOF(Radio_CompressLevel);Type++){
			Radio_CompressLevel[Type].EnableWindow(Check_UsePreset.GetCheck());
		}
		for(int Type=0;Type<COUNTOF(Radio_LZMA_Mode);Type++){
			BOOL bCheck=Radio_CompressType[SEVEN_ZIP_COMPRESS_LZMA].GetCheck() || Radio_CompressType[SEVEN_ZIP_COMPRESS_LZMA2].GetCheck();
			Radio_LZMA_Mode[Type].EnableWindow(bCheck&&!Check_UsePreset.GetCheck());
		}
		Check_SpecifyPPMdModelSize.EnableWindow(Radio_CompressType[SEVEN_ZIP_COMPRESS_PPMD].GetCheck()&&!Check_UsePreset.GetCheck());
		::EnableWindow(GetDlgItem(IDC_EDIT_7Z_PPMD_MODEL_SIZE),Radio_CompressType[SEVEN_ZIP_COMPRESS_PPMD].GetCheck()&&Check_SpecifyPPMdModelSize.GetCheck()&&!Check_UsePreset.GetCheck());
	}
	return 0;
}

LRESULT CConfigDlg7Z::OnSpecifyPPMdModelSize(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if(BN_CLICKED==wNotifyCode){
		::EnableWindow(GetDlgItem(IDC_EDIT_7Z_PPMD_MODEL_SIZE),(Check_SpecifyPPMdModelSize.GetCheck())&&(!Check_UsePreset.GetCheck()));
	}
	return 0;
}

LRESULT CConfigDlg7Z::OnSelectCompressType(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if(BN_CLICKED==wNotifyCode){
		//LZMA
		for(int Type=0;Type<COUNTOF(Radio_LZMA_Mode);Type++){
			BOOL bCheck=Radio_CompressType[SEVEN_ZIP_COMPRESS_LZMA].GetCheck() || Radio_CompressType[SEVEN_ZIP_COMPRESS_LZMA2].GetCheck();
			Radio_LZMA_Mode[Type].EnableWindow(bCheck&&!Check_UsePreset.GetCheck());
		}
		//PPMd
		Check_SpecifyPPMdModelSize.EnableWindow(Radio_CompressType[SEVEN_ZIP_COMPRESS_PPMD].GetCheck()&&!Check_UsePreset.GetCheck());
		::EnableWindow(GetDlgItem(IDC_EDIT_7Z_PPMD_MODEL_SIZE),Radio_CompressType[SEVEN_ZIP_COMPRESS_PPMD].GetCheck()&&Check_SpecifyPPMdModelSize.GetCheck()&&!Check_UsePreset.GetCheck());
	}
	return 0;
}

LRESULT CConfigDlg7Z::OnHeaderCompression(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if(BN_CLICKED==wNotifyCode){
		::EnableWindow(GetDlgItem(IDC_CHECK_7Z_FULL_HEADER_COMPRESSION),Check_HeaderCompression.GetCheck());
	}
	return 0;
}

LRESULT CConfigDlg7Z::OnSpecifySplitSize(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if(BN_CLICKED==wNotifyCode){
		::EnableWindow(GetDlgItem(IDC_EDIT_7Z_SPLIT_SIZE), Button_GetState(GetDlgItem(IDC_CHECK_7Z_SPECIFY_SPLIT_SIZE)));
		::EnableWindow(GetDlgItem(IDC_COMBO_7Z_SPLIT_SIZE_UNIT), Button_GetState(GetDlgItem(IDC_CHECK_7Z_SPECIFY_SPLIT_SIZE)));
	}
	return 0;
}

