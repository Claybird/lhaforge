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


//---------------------------------------------------------------

//圧縮形式選択:キャンセルでPARAMETER_UNDEFINEDが返る
PARAMETER_TYPE SelectCompressType(int &Options,bool &bSingleCompression)
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
			}else{
				Options=SelDlg.GetOptions();
				bSingleCompression=SelDlg.IsSingleCompression();
				return CompressType;
			}
		}
	}
}
