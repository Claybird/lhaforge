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
#include "SelectDlg.h"
#include "../Compress.h"
#include "../ArchiverManager.h"

LRESULT CSelectDialog::OnInitDialog(HWND hWnd, LPARAM lParam)
{
	//DDX情報アップデート
	DoDataExchange(FALSE);
	::EnableWindow(GetDlgItem(IDC_CHECK_COMPRESS_PUBLIC_PASSWORD),bPassword);	//公開パスワード欄を使用不可にする
	CenterWindow();
	return TRUE;
}


#define BUTTON_PARAM(x) case IDC_BUTTON_FORMAT_##x: Param=PARAMETER_##x;break
void CSelectDialog::OnCommand(UINT nCode, int nID, HWND hWnd)
{
	//DDX情報アップデート
	DoDataExchange(TRUE);
	PARAMETER_TYPE Param;
	switch(nID){
	case IDCANCEL:
		Param=PARAMETER_UNDEFINED;
		break;
	case IDC_BUTTON_USEB2E:
		Param=PARAMETER_B2E;
		break;
	BUTTON_PARAM(LZH);
	BUTTON_PARAM(ZIP);
	BUTTON_PARAM(CAB);
	BUTTON_PARAM(7Z);
	BUTTON_PARAM(TAR);
	BUTTON_PARAM(GZ);
	BUTTON_PARAM(BZ2);
	BUTTON_PARAM(XZ);
	BUTTON_PARAM(LZMA);
	BUTTON_PARAM(TAR_GZ);
	BUTTON_PARAM(TAR_BZ2);
	BUTTON_PARAM(TAR_XZ);
	BUTTON_PARAM(TAR_LZMA);
	BUTTON_PARAM(JACK);
	BUTTON_PARAM(YZ1);
	BUTTON_PARAM(HKI);
	BUTTON_PARAM(BZA);
	BUTTON_PARAM(GZA);
	BUTTON_PARAM(ISH);
	BUTTON_PARAM(UUE);
	default:/*ASSERT(!"Not implemented");*/return;
	}
	EndDialog(Param);
}

LRESULT CSelectDialog::OnPassword(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	//DDX情報アップデート
	DoDataExchange(TRUE);
	if(BN_CLICKED==wNotifyCode){
		::EnableWindow(GetDlgItem(IDC_CHECK_COMPRESS_PUBLIC_PASSWORD),bPassword);
	}
	return 0;
}

LRESULT CSelectDialog::OnSFX(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	//DDX情報アップデート
	DoDataExchange(TRUE);
	if(BN_CLICKED==wNotifyCode){
		::EnableWindow(GetDlgItem(IDC_CHECK_COMPRESS_SPLIT),!bSFX);
		bSplit=false;
	}
	DoDataExchange(FALSE);
	return 0;
}

int CSelectDialog::GetOptions()
{
	int Options=0;
	if(bSFX)Options|=COMPRESS_SFX;
	if(bSplit)Options|=COMPRESS_SPLIT;
	if(bPassword){
		if(bPublicPassword)Options|=COMPRESS_PUBLIC_PASSWORD;
		else Options|=COMPRESS_PASSWORD;
	}
	return Options;
}

//----------------------------------------------------

LRESULT CB2ESelectDialog::OnInitDialog(HWND hWnd, LPARAM lParam)
{
	//DDX情報アップデート
	DoDataExchange(FALSE);

	//---------------
	// B2Eの情報取得
	//---------------
	//B2Eハンドラ
	CArchiverB2E &ArcB2E=CArchiverDLLManager::GetInstance().GetB2EHandler();
	ASSERT(ArcB2E.IsOK());	//B2Eが使えない時は呼ばないように！
	if(!ArcB2E.IsOK())return FALSE;

	//B2E情報取得
	if(!ArcB2E.EnumCompressB2EScript(m_ScriptInfoArray))m_ScriptInfoArray.clear();

	//使えるB2Eがあれば
	if(!m_ScriptInfoArray.empty()){
		//---形式
		UINT uIdx=0;
		for(;uIdx<m_ScriptInfoArray.size();uIdx++){
			Combo_Format.AddString(CA2T(m_ScriptInfoArray[uIdx].szFormat));
		}
		::EnableWindow(GetDlgItem(IDC_CHECK_COMPRESS_SFX),m_ScriptInfoArray[0].wAbility&B2EABILITY_SFX);
		Combo_Format.SetCurSel(0);

		//---メソッド
		if(m_ScriptInfoArray[0].MethodArray.empty()){
			Combo_Method.AddString(_T("Default"));
		}
		else{
			for(uIdx=0;uIdx<m_ScriptInfoArray[0].MethodArray.size();uIdx++){
				Combo_Method.AddString(CA2T(m_ScriptInfoArray[0].MethodArray[uIdx]));
			}
		}
		Combo_Method.SetCurSel(m_ScriptInfoArray[0].nDefaultMethod);
	}
	else{
		::EnableWindow(GetDlgItem(IDC_CHECK_COMPRESS_SFX),false);
		::EnableWindow(GetDlgItem(IDC_CHECK_SINGLE_COMPRESSION),false);
		Combo_Format.EnableWindow(false);
		Combo_Method.EnableWindow(false);
		::EnableWindow(GetDlgItem(IDOK),false);
	}

	CenterWindow();
	return TRUE;
}

void CB2ESelectDialog::OnCommand(UINT nCode, int nID, HWND hWnd)
{
	switch(nID){
	case IDC_COMPRESS_USENORMAL:
	case IDOK:
	case IDCANCEL:
		break;
	default:return;
	}
	//DDX情報アップデート
	DoDataExchange(TRUE);
	{
		int nIndex = Combo_Format.GetCurSel();
		if(nIndex == CB_ERR){
			m_strFormat.Empty();
		}else{
			Combo_Format.GetLBText(nIndex,m_strFormat);
		}
	}
	{
		int nIndex = Combo_Method.GetCurSel();
		if(nIndex == CB_ERR){
			m_strMethod.Empty();
		}else{
			Combo_Method.GetLBText(nIndex,m_strMethod);
		}
	}
	EndDialog(nID);
}

void CB2ESelectDialog::OnComboFormat(UINT uNotifyCode, int nID, HWND hWndCtl)
{
	//---形式選択変更
	int nIndex = Combo_Format.GetCurSel();
	if(nIndex == CB_ERR || nIndex<0 || (unsigned)nIndex>=m_ScriptInfoArray.size())return;

	m_bSFX=false;

	//DDX情報アップデート
	DoDataExchange(FALSE);

	//自己解凍対応？
	::EnableWindow(GetDlgItem(IDC_CHECK_COMPRESS_SFX),m_ScriptInfoArray[nIndex].wAbility&B2EABILITY_SFX);

	//メソッド列挙
	Combo_Method.ResetContent();	//消去
	if(m_ScriptInfoArray[nIndex].MethodArray.empty()){
		Combo_Method.AddString(_T("Default"));
	}
	else{
		for(UINT uIdx=0;uIdx<m_ScriptInfoArray[nIndex].MethodArray.size();uIdx++){
			Combo_Method.AddString(CA2T(m_ScriptInfoArray[nIndex].MethodArray[uIdx]));
		}
	}
	Combo_Method.SetCurSel(m_ScriptInfoArray[nIndex].nDefaultMethod);
}


//---------------------------------------------------------------

//圧縮形式選択:キャンセルでPARAMETER_UNDEFINEDが返る
PARAMETER_TYPE SelectCompressType(int &Options,bool &bSingleCompression,CString &strB2EFormat,CString &strB2EMethod,bool &bB2ESFX)
{
	//初期化
	PARAMETER_TYPE CompressType=PARAMETER_UNDEFINED;
	bSingleCompression=false;
	Options=0;

	//OkかCancelまで繰り返し
	while(true){	//---使用DLLを決定
		if(PARAMETER_UNDEFINED==CompressType){	//形式が指定されていない場合
			CSelectDialog SelDlg;
			CompressType=(PARAMETER_TYPE)SelDlg.DoModal();
			if(PARAMETER_UNDEFINED==CompressType){	//キャンセルの場合
				return PARAMETER_UNDEFINED;
			}else if(CompressType!=PARAMETER_B2E){
				Options=SelDlg.GetOptions();
				bSingleCompression=SelDlg.IsSingleCompression();
				return CompressType;
			}
		}
		if(CompressType==PARAMETER_B2E){	//B2Eを使用する場合
			//---B2E32.dllのチェック
			CArchiverB2E &B2EHandler=CArchiverDLLManager::GetInstance().GetB2EHandler();
			if(!B2EHandler.IsOK()){
				CompressType=PARAMETER_UNDEFINED;
				CString msg;
				msg.Format(IDS_ERROR_DLL_LOAD,B2EHandler.GetName());
				ErrorMessage(msg);
				continue;
			}

			//---形式選択
			CB2ESelectDialog SelDlg;
			INT_PTR Ret=SelDlg.DoModal();
			if(IDCANCEL==Ret){	//キャンセルの場合
				return PARAMETER_UNDEFINED;
			}else if(IDC_COMPRESS_USENORMAL==Ret){	//通常のDLLを使う
				CompressType=PARAMETER_UNDEFINED;
			}else{
				Options=-1;
				bB2ESFX=SelDlg.IsSFX();
				bSingleCompression=SelDlg.IsSingleCompression();
				strB2EFormat=SelDlg.GetFormat();
				strB2EMethod=SelDlg.GetMethod();
				return CompressType;
			}
		}
	}
}
