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
#include "Dlg_shortcut.h"
#include "../../Compress.h"
#include "../../Dialogs/SelectDlg.h"
#include "../../ArchiverManager.h"
#include "../../Utilities/OSUtil.h"

//========================
// ショートカット作成画面
//========================
LRESULT CConfigDlgShortcut::OnInitDialog(HWND hWnd, LPARAM lParam)
{
	// メッセージループにメッセージフィルタとアイドルハンドラを追加
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	pLoop->AddMessageFilter(this);

	//Windows2000以前ならショートカットに引数がつけられない
	//自動のショートカット以外は無効に
	if(AtlIsOldWindows()){
		//解凍のショートカット
		::EnableWindow(GetDlgItem(IDC_BUTTON_CREATE_EXTRACT_SHORTCUT_DESKTOP),false);
		::EnableWindow(GetDlgItem(IDC_BUTTON_CREATE_EXTRACT_SHORTCUT_SENDTO),false);

		//圧縮のショートカット
		::EnableWindow(GetDlgItem(IDC_BUTTON_CREATE_COMPRESS_SHORTCUT_DESKTOP),false);
		::EnableWindow(GetDlgItem(IDC_BUTTON_CREATE_COMPRESS_SHORTCUT_SENDTO),false);

		//閲覧のショートカット
		::EnableWindow(GetDlgItem(IDC_BUTTON_CREATE_LIST_SHORTCUT_DESKTOP),false);
		::EnableWindow(GetDlgItem(IDC_BUTTON_CREATE_LIST_SHORTCUT_SENDTO),false);

		//検査のショートカット
		::EnableWindow(GetDlgItem(IDC_BUTTON_CREATE_TESTARCHIVE_SHORTCUT_DESKTOP),false);
		::EnableWindow(GetDlgItem(IDC_BUTTON_CREATE_TESTARCHIVE_SHORTCUT_SENDTO),false);
	}
	return TRUE;
}

LRESULT CConfigDlgShortcut::OnCreateShortcut(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if(BN_CLICKED==wNotifyCode){
		//------------------------
		// LhaForge本体のパス取得
		//------------------------
		TCHAR ExePath[_MAX_PATH+1];
		FILL_ZERO(ExePath);
		GetModuleFileName(GetModuleHandle(NULL), ExePath, _MAX_PATH);

		//ショートカット ファイル名
		TCHAR ShortcutFileName[_MAX_PATH+1];
		FILL_ZERO(ShortcutFileName);

		//----------------------
		// 作成先フォルダの取得
		//----------------------
		switch(wID){
		case IDC_BUTTON_CREATE_COMPRESS_SHORTCUT_DESKTOP://FALLTHROUGH
		case IDC_BUTTON_CREATE_EXTRACT_SHORTCUT_DESKTOP://FALLTHROUGH
		case IDC_BUTTON_CREATE_AUTOMATIC_SHORTCUT_DESKTOP://FALLTHROUGH
		case IDC_BUTTON_CREATE_LIST_SHORTCUT_DESKTOP://FALLTHROUGH
		case IDC_BUTTON_CREATE_TESTARCHIVE_SHORTCUT_DESKTOP:
			// デスクトップに作成
			if(!SHGetSpecialFolderPath(NULL,ShortcutFileName,CSIDL_DESKTOPDIRECTORY,FALSE)){
				ErrorMessage(CString(MAKEINTRESOURCE(IDS_ERROR_GET_DESKTOP)));
				return 0;
			}
			break;
		case IDC_BUTTON_CREATE_COMPRESS_SHORTCUT_SENDTO://FALLTHROUGH
		case IDC_BUTTON_CREATE_EXTRACT_SHORTCUT_SENDTO://FALLTHROUGH
		case IDC_BUTTON_CREATE_AUTOMATIC_SHORTCUT_SENDTO://FALLTHROUGH
		case IDC_BUTTON_CREATE_LIST_SHORTCUT_SENDTO://FALLTHROUGH
		case IDC_BUTTON_CREATE_TESTARCHIVE_SHORTCUT_SENDTO:
			// 「送る」フォルダに作成
			if(!SHGetSpecialFolderPath(NULL,ShortcutFileName,CSIDL_SENDTO,FALSE)){
				ErrorMessage(CString(MAKEINTRESOURCE(IDS_ERROR_GET_SENDTO)));
				return 0;
			}
			break;
		default:ASSERT(!"OnCreateShortcut:this code must not be run.");return 0;
		}

		//----------------------
		// ショートカットの設定
		//----------------------
		CString Param;	//コマンドライン引数
		int IconIndex=-1;	//ショートカットアイコン
		WORD DescriptionID;	//ショートカットの説明のリソースID
		switch(wID){
		case IDC_BUTTON_CREATE_COMPRESS_SHORTCUT_DESKTOP://FALLTHROUGH
		case IDC_BUTTON_CREATE_COMPRESS_SHORTCUT_SENDTO:
			//LhaForgeで圧縮
			if(!GetCompressShortcutInfo(ShortcutFileName,Param))return 0;
			DescriptionID=IDS_SHORTCUT_DESCRIPTION_COMPRESS;
			IconIndex=1;
			break;

		case IDC_BUTTON_CREATE_EXTRACT_SHORTCUT_DESKTOP://FALLTHROUGH
		case IDC_BUTTON_CREATE_EXTRACT_SHORTCUT_SENDTO:
			//LhaForgeで解凍
			PathAppend(ShortcutFileName,CString(MAKEINTRESOURCE(IDS_SHORTCUT_NAME_EXTRACT)));
			Param=_T("/e");
			DescriptionID=IDS_SHORTCUT_DESCRIPTION_EXTRACT;
			IconIndex=2;
			break;

		case IDC_BUTTON_CREATE_AUTOMATIC_SHORTCUT_DESKTOP://FALLTHROUGH
		case IDC_BUTTON_CREATE_AUTOMATIC_SHORTCUT_SENDTO:
			//LhaForgeで処理
			PathAppend(ShortcutFileName,CString(MAKEINTRESOURCE(IDS_SHORTCUT_NAME_AUTOMATIC)));
			Param.Empty();
			DescriptionID=IDS_SHORTCUT_DESCRIPTION_AUTOMATIC;
			IconIndex=0;
			break;

		case IDC_BUTTON_CREATE_LIST_SHORTCUT_DESKTOP://FALLTHROUGH
		case IDC_BUTTON_CREATE_LIST_SHORTCUT_SENDTO:
			//LhaForgeで閲覧
			PathAppend(ShortcutFileName,CString(MAKEINTRESOURCE(IDS_SHORTCUT_NAME_LIST)));
			Param=_T("/l");
			DescriptionID=IDS_SHORTCUT_DESCRIPTION_LIST;
			IconIndex=3;
			break;

		case IDC_BUTTON_CREATE_TESTARCHIVE_SHORTCUT_DESKTOP://FALLTHROUGH
		case IDC_BUTTON_CREATE_TESTARCHIVE_SHORTCUT_SENDTO:
			//LhaForgeで検査
			PathAppend(ShortcutFileName,CString(MAKEINTRESOURCE(IDS_SHORTCUT_NAME_TESTARCHIVE)));
			Param=_T("/t");
			DescriptionID=IDS_SHORTCUT_DESCRIPTION_TESTARCHIVE;
			IconIndex=4;
			break;

		default:ASSERT(!"OnCreateShortcut:this code must not be run.");return 0;
		}
		//拡張子
		_tcsncat_s(ShortcutFileName,_T(".lnk"),_MAX_PATH);

		if(FAILED(UtilCreateShortcut(ShortcutFileName,ExePath,Param,ExePath,IconIndex,CString(MAKEINTRESOURCE(DescriptionID))))){
			ErrorMessage(CString(MAKEINTRESOURCE(IDS_ERROR_CREATE_SHORTCUT)));
		}else{
			//作成成功で音を鳴らす
			MessageBeep(MB_ICONASTERISK);
		}
	}
	return 0;
}

//作成するショートカットの情報を取得
//Path:ショートカットファイル名,Param:コマンドライン引数
bool CConfigDlgShortcut::GetCompressShortcutInfo(LPTSTR Path,CString &Param)
{
/*	int Options=-1;
	bool bSingleCompression=false;
	bool bB2ESFX=false;
	CString strB2EFormat,strB2EMethod;*/

	//圧縮形式を今決めておくか、後で決めるかを選ばせる
	if(IDYES==MessageBox(CString(MAKEINTRESOURCE(IDS_ASK_SHORTCUT_COMPRESS_TYPE_ALWAYS_ASK)),UtilGetMessageCaption(),MB_YESNO|MB_ICONQUESTION)){
		int Options=-1;
		bool bSingleCompression=false;
		bool bB2ESFX=false;
		CString strB2EFormat,strB2EMethod;

		//形式選択ダイアログ
		PARAMETER_TYPE CompressType=SelectCompressType(Options,bSingleCompression,strB2EFormat,strB2EMethod,bB2ESFX);
		if(CompressType==PARAMETER_UNDEFINED)return false;	//キャンセル

		if(CompressType!=PARAMETER_B2E){	//通常DLLを使用
			//選択ダイアログの条件に一致するパラメータを検索
			int Index=0;
			for(;Index<COMPRESS_PARAM_COUNT;Index++){
				if(CompressType==CompressParameterArray[Index].Type){
					if(Options==CompressParameterArray[Index].Options){
						break;
					}
				}
			}
			if(Index>=COMPRESS_PARAM_COUNT){
				//一覧に指定された圧縮方式がない
				//つまり、サポートしていない圧縮方式だったとき
				ErrorMessage(CString(MAKEINTRESOURCE(IDS_ERROR_ILLEGAL_FORMAT_TYPE)));
				return false;
			}
			//ショートカット名取得
			CString Buf;
			if(bSingleCompression){
				//一つずつ圧縮
				Buf.Format(IDS_SHORTCUT_NAME_COMPRESS_EX_SINGLE,CString(MAKEINTRESOURCE(CompressParameterArray[Index].FormatName)));
			}
			else{
				//通常
				Buf.Format(IDS_SHORTCUT_NAME_COMPRESS_EX,CString(MAKEINTRESOURCE(CompressParameterArray[Index].FormatName)));
			}
			PathAppend(Path,Buf);
			//パラメータ
			Param=CompressParameterArray[Index].Param;
			if(bSingleCompression){
				//一つずつ圧縮
				Param+=_T(" /s");
			}
		}else{	//B2E32.dllを使用
			//パラメータ
			Param.Format(_T("/b2e \"/c:%s\" \"/method:%s\""),strB2EFormat,strB2EMethod);
			if(bB2ESFX){
				Param+=_T(" /b2esfx");
			}

			//ショートカット名取得
			CString strInfo;
			if(bB2ESFX){
				strInfo.Format(IDS_FORMAT_NAME_B2E_SFX,strB2EFormat,strB2EMethod);
			}
			else{
				strInfo.Format(IDS_FORMAT_NAME_B2E,strB2EFormat,strB2EMethod);
			}
			CString Buf;
			if(bSingleCompression){
				//一つずつ圧縮
				Buf.Format(IDS_SHORTCUT_NAME_COMPRESS_EX_SINGLE,strInfo);
				Param+=_T(" /s");
			}
			else{
				//通常
				Buf.Format(IDS_SHORTCUT_NAME_COMPRESS_EX,strInfo);
			}
			PathAppend(Path,Buf);
		}
	}
	else{
		//圧縮形式をその都度決める
		PathAppend(Path,CString(MAKEINTRESOURCE(IDS_SHORTCUT_NAME_COMPRESS)));
		Param=_T("/c");
	}
	return true;
}

