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
#include "Dlg_shortcut.h"
#include "../../Compress.h"
#include "../../Dialogs/SelectDlg.h"
#include "../../Utilities/OSUtil.h"

//========================
// ショートカット作成画面
//========================
LRESULT CConfigDlgShortcut::OnInitDialog(HWND hWnd, LPARAM lParam)
{
	// メッセージループにメッセージフィルタとアイドルハンドラを追加
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	pLoop->AddMessageFilter(this);

	return TRUE;
}

LRESULT CConfigDlgShortcut::OnCreateShortcut(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if(BN_CLICKED==wNotifyCode){
		//------------------------
		// LhaForge本体のパス取得
		//------------------------
		const auto ExePath = UtilGetModulePath();

		//ショートカット ファイル名
		std::filesystem::path ShortcutFileName;

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
			ShortcutFileName = UtilGetDesktopPath();
			break;
		case IDC_BUTTON_CREATE_COMPRESS_SHORTCUT_SENDTO://FALLTHROUGH
		case IDC_BUTTON_CREATE_EXTRACT_SHORTCUT_SENDTO://FALLTHROUGH
		case IDC_BUTTON_CREATE_AUTOMATIC_SHORTCUT_SENDTO://FALLTHROUGH
		case IDC_BUTTON_CREATE_LIST_SHORTCUT_SENDTO://FALLTHROUGH
		case IDC_BUTTON_CREATE_TESTARCHIVE_SHORTCUT_SENDTO:
			// 「送る」フォルダに作成
			ShortcutFileName = UtilGetSendToPath();
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
			ShortcutFileName /= UtilLoadString(IDS_SHORTCUT_NAME_EXTRACT);
			Param=_T("/e");
			DescriptionID=IDS_SHORTCUT_DESCRIPTION_EXTRACT;
			IconIndex=2;
			break;

		case IDC_BUTTON_CREATE_AUTOMATIC_SHORTCUT_DESKTOP://FALLTHROUGH
		case IDC_BUTTON_CREATE_AUTOMATIC_SHORTCUT_SENDTO:
			//LhaForgeで処理
			ShortcutFileName /= UtilLoadString(IDS_SHORTCUT_NAME_AUTOMATIC);
			Param.Empty();
			DescriptionID=IDS_SHORTCUT_DESCRIPTION_AUTOMATIC;
			IconIndex=0;
			break;

		case IDC_BUTTON_CREATE_LIST_SHORTCUT_DESKTOP://FALLTHROUGH
		case IDC_BUTTON_CREATE_LIST_SHORTCUT_SENDTO:
			//LhaForgeで閲覧
			ShortcutFileName /= UtilLoadString(IDS_SHORTCUT_NAME_LIST);
			Param=_T("/l");
			DescriptionID=IDS_SHORTCUT_DESCRIPTION_LIST;
			IconIndex=3;
			break;

		case IDC_BUTTON_CREATE_TESTARCHIVE_SHORTCUT_DESKTOP://FALLTHROUGH
		case IDC_BUTTON_CREATE_TESTARCHIVE_SHORTCUT_SENDTO:
			//LhaForgeで検査
			ShortcutFileName /= UtilLoadString(IDS_SHORTCUT_NAME_TESTARCHIVE);
			Param=_T("/t");
			DescriptionID=IDS_SHORTCUT_DESCRIPTION_TESTARCHIVE;
			IconIndex=4;
			break;

		default:ASSERT(!"OnCreateShortcut:this code must not be run.");return 0;
		}
		//拡張子
		ShortcutFileName += L".lnk";

		if(FAILED(UtilCreateShortcut(ShortcutFileName, ExePath, (const wchar_t*)Param, ExePath,IconIndex, UtilLoadString(DescriptionID).c_str()))){
			ErrorMessage(UtilLoadString(IDS_ERROR_CREATE_SHORTCUT));
		}else{
			//作成成功で音を鳴らす
			MessageBeep(MB_ICONASTERISK);
		}
	}
	return 0;
}

//作成するショートカットの情報を取得
//Path:ショートカットファイル名,Param:コマンドライン引数
bool CConfigDlgShortcut::GetCompressShortcutInfo(std::filesystem::path &Path,CString &Param)
{
	//圧縮形式を今決めておくか、後で決めるかを選ばせる
	if(IDYES== UtilMessageBox(m_hWnd, (const wchar_t*)CString(MAKEINTRESOURCE(IDS_ASK_SHORTCUT_COMPRESS_TYPE_ALWAYS_ASK)),MB_YESNO|MB_ICONQUESTION)){
		int Options=-1;
		bool bSingleCompression=false;

		//形式選択ダイアログ
		auto format = SelectCompressType(Options,bSingleCompression);
		if(format==LF_FMT_INVALID)return false;	//キャンセル

		//選択ダイアログの条件に一致するパラメータを検索
		try {
			const auto &args = get_archive_format_args(format, Options);
			//ショートカット名取得
			CString Buf;
			if (bSingleCompression) {
				//一つずつ圧縮
				Buf.Format(IDS_SHORTCUT_NAME_COMPRESS_EX_SINGLE, UtilLoadString(args.FormatName).c_str());
			} else {
				//通常
				Buf.Format(IDS_SHORTCUT_NAME_COMPRESS_EX, UtilLoadString(args.FormatName).c_str());
			}
			Path /= Buf.operator LPCWSTR();
			//パラメータ
			Param = (L"/c:" + args.name).c_str();
			if (bSingleCompression) {
				//一つずつ圧縮
				Param += _T(" /s");
			}
		} catch (const ARCHIVE_EXCEPTION&) {
			//一覧に指定された圧縮方式がない
			//つまり、サポートしていない圧縮方式だったとき
			ErrorMessage((const wchar_t*)CString(MAKEINTRESOURCE(IDS_ERROR_ILLEGAL_FORMAT_TYPE)));
			return false;
		}
	}
	else{
		//圧縮形式をその都度決める
		Path /= UtilLoadString(IDS_SHORTCUT_NAME_COMPRESS);
		Param=_T("/c");
	}
	return true;
}

