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
#include "Dlg_compress_general.h"
#include "../../ArchiverCode/arc_interface.h"
#include "../../Dialogs/selectdlg.h"
#include "../../compress.h"

//==================
// 圧縮一般設定画面
//==================
LRESULT CConfigDlgCompressGeneral::OnInitDialog(HWND hWnd, LPARAM lParam)
{
	// メッセージループにメッセージフィルタとアイドルハンドラを追加
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	pLoop->AddMessageFilter(this);

	//------------------
	// 圧縮出力先タイプ
	//------------------
	Radio_CompressTo[OUTPUT_TO_DESKTOP]=GetDlgItem(IDC_RADIO_COMPRESS_TO_DESKTOP);
	Radio_CompressTo[OUTPUT_TO_SAME_DIR]=GetDlgItem(IDC_RADIO_COMPRESS_TO_SAME_DIR);
	Radio_CompressTo[OUTPUT_TO_SPECIFIC_DIR]=GetDlgItem(IDC_RADIO_COMPRESS_TO_SPECIFIC_DIR);
	Radio_CompressTo[OUTPUT_TO_ALWAYS_ASK_WHERE]=GetDlgItem(IDC_RADIO_COMPRESS_TO_ALWAYS_ASK_WHERE);

	Radio_CompressTo[m_Config.OutputDirType].SetCheck(1);

	//----------------------------------------------------
	// 出力先フォルダのパスをエディットコントロールに設定
	//----------------------------------------------------
	Edit_CompressOutputDirPath=GetDlgItem(IDC_EDIT_COMPRESS_TO_SPECIFIC_DIR);
	Edit_CompressOutputDirPath.SetLimitText(_MAX_PATH);
	Edit_CompressOutputDirPath.SetWindowText(m_Config.OutputDir);

	Button_CompressToFolder=GetDlgItem(IDC_BUTTON_COMPRESS_BROWSE_FOLDER);

	//出力先を指定するためのボタンとエディットコントロールの有効無効を切り替え
	bool bActive=(OUTPUT_TO_SPECIFIC_DIR==m_Config.OutputDirType);
	Edit_CompressOutputDirPath.EnableWindow(bActive);
	Button_CompressToFolder.EnableWindow(bActive);

	//--------------------------------
	// 同時に圧縮するファイル数の上限
	//--------------------------------
	UpDown_MaxCompressFileCount=GetDlgItem(IDC_SPIN_MAX_COMPRESS_FILECOUNT);

	UpDown_MaxCompressFileCount.SetPos(m_Config.MaxCompressFileCount);
	UpDown_MaxCompressFileCount.SetRange(1,32767);

	UpDown_MaxCompressFileCount.EnableWindow(m_Config.LimitCompressFileCount);
	::EnableWindow(GetDlgItem(IDC_EDIT_MAX_COMPRESS_FILECOUNT),m_Config.LimitCompressFileCount);

	//「同時に圧縮するファイル数を制限する」チェックボックス
	Check_LimitCompressFileCount=GetDlgItem(IDC_CHECK_LIMIT_COMPRESS_FILECOUNT);

	//--------------------------
	// デフォルト圧縮パラメータ
	//--------------------------
	Check_UseDefaultParameter=GetDlgItem(IDC_CHECK_USE_DEFAULTPARAMETER);
	Edit_DefaultParameterInfo=GetDlgItem(IDC_EDIT_DEFAULTPARAMETER);

	::EnableWindow(GetDlgItem(IDC_BUTTON_SELECT_DEFAULTPARAMETER),m_Config.UseDefaultParameter);

	SetParameterInfo();

	//----------------------------------
	// 圧縮後元ファイルを削除する機能
	//----------------------------------
	//「解凍後圧縮ファイルを削除する」チェックボックス
	Check_DeleteAfterCompress=GetDlgItem(IDC_CHECK_DELETE_AFTER_COMPRESS);
	//「ごみ箱へ移動する」チェックボックスの有効無効
	::EnableWindow(GetDlgItem(IDC_CHECK_MOVETO_RECYCLE_BIN),m_Config.DeleteAfterCompress);
	//「確認しない」の有効無効
	::EnableWindow(GetDlgItem(IDC_CHECK_DELETE_NOCONFIRM),m_Config.DeleteAfterCompress);
	//強制削除の有効無効
	::EnableWindow(GetDlgItem(IDC_CHECK_FORCE_DELETE),m_Config.DeleteAfterCompress);

	//DDX情報設定
	DoDataExchange(FALSE);

	return TRUE;
}

LRESULT CConfigDlgCompressGeneral::OnApply()
{
//===============================
// 設定をConfigManagerに書き戻す
//===============================
	//------------------
	// 圧縮出力先タイプ
	//------------------
	for(int Type=0;Type<COUNTOF(Radio_CompressTo);Type++){
		if(Radio_CompressTo[Type].GetCheck()){
			m_Config.OutputDirType=(OUTPUT_TO)Type;
			break;
		}
	}
	//----------------------
	// 出力先フォルダのパス
	//----------------------
	Edit_CompressOutputDirPath.GetWindowText(m_Config.OutputDir);

	//--------------------------------
	// 同時に圧縮するファイル数の上限
	//--------------------------------
	m_Config.MaxCompressFileCount=UpDown_MaxCompressFileCount.GetPos();

	//---------------
	// DDXデータ取得
	//---------------
	if(!DoDataExchange(TRUE)){
		return FALSE;
	}
	return TRUE;
}

LRESULT CConfigDlgCompressGeneral::OnRadioCompressTo(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if(BN_CLICKED==wNotifyCode){
		bool bActive=(0!=Radio_CompressTo[OUTPUT_TO_SPECIFIC_DIR].GetCheck());
		Edit_CompressOutputDirPath.EnableWindow(bActive);
		Button_CompressToFolder.EnableWindow(bActive);
	}
	return 0;
}

LRESULT CConfigDlgCompressGeneral::OnBrowseFolder(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if(BN_CLICKED==wNotifyCode){
		TCHAR FolderPath[_MAX_PATH+1];
		FILL_ZERO(FolderPath);

		Edit_CompressOutputDirPath.GetWindowText(FolderPath,_MAX_PATH);

		CString title(MAKEINTRESOURCE(IDS_INPUT_TARGET_FOLDER));
		CFolderDialog dlg(m_hWnd,title,BIF_RETURNONLYFSDIRS|BIF_NEWDIALOGSTYLE);
		dlg.SetInitialFolder(FolderPath);
		if(IDOK==dlg.DoModal()){
			_tcsncpy_s(FolderPath,dlg.GetFolderPath(),_MAX_PATH);
			Edit_CompressOutputDirPath.SetWindowText(FolderPath);
		}
	}
	return 0;
}

LRESULT CConfigDlgCompressGeneral::OnCheckLimitCompressFileCount(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if(BN_CLICKED==wNotifyCode){
		BOOL State=Check_LimitCompressFileCount.GetCheck();
		UpDown_MaxCompressFileCount.EnableWindow(State);
		::EnableWindow(GetDlgItem(IDC_EDIT_MAX_COMPRESS_FILECOUNT),State);
	}
	return 0;
}

//デフォルト圧縮パラメータの有効無効
LRESULT CConfigDlgCompressGeneral::OnCheckUseDefaultParameter(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if(BN_CLICKED==wNotifyCode){
		BOOL State=Check_UseDefaultParameter.GetCheck();
		::EnableWindow(GetDlgItem(IDC_BUTTON_SELECT_DEFAULTPARAMETER),State);
	}
	return 0;
}

//デフォルト圧縮パラメータの選択
LRESULT CConfigDlgCompressGeneral::OnSelectDefaultParameter(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if(BN_CLICKED==wNotifyCode){
		int Options=-1;
		bool bSingleCompression=false;	//無視される

		//形式選択ダイアログ
		auto format =SelectCompressType(Options,bSingleCompression);
		if(format == LF_FMT_INVALID)return 1;	//キャンセル

		//選択ダイアログの条件に一致するパラメータを検索
		try {
			const auto &caps = get_archive_capability(format);
			if (!isIn(caps.allowed_combinations, Options)) {
				throw ARCHIVE_EXCEPTION(EINVAL);
			}
		} catch (const ARCHIVE_EXCEPTION& ) {
			//一覧に指定された圧縮方式がない
			//つまり、サポートしていない圧縮方式だったとき
			ErrorMessage((const wchar_t*)CString(MAKEINTRESOURCE(IDS_ERROR_ILLEGAL_FORMAT_TYPE)));
			return 1;
		}
		//設定を保存
		m_Config.DefaultType=format;
		m_Config.DefaultOptions=Options;

		SetParameterInfo();//Editに現在のパラメータの情報を表示する
		return 0;
	}
	return 0;
}


void CConfigDlgCompressGeneral::SetParameterInfo()//Editに現在のパラメータの情報を表示する
{
	if(LF_FMT_INVALID==m_Config.DefaultType){
		//未定義
		Edit_DefaultParameterInfo.SetWindowText(_T(""));
	}else{
		//選択ダイアログの条件に一致するパラメータを検索
		try {
			const auto &args = get_archive_format_args(m_Config.DefaultType, m_Config.DefaultOptions);
			//正常な設定
			Edit_DefaultParameterInfo.SetWindowText(CString(MAKEINTRESOURCE(args.FormatName)));
		} catch (const ARCHIVE_EXCEPTION&) {
			//一覧に指定された圧縮方式がない
			//つまり、サポートしていない圧縮方式だったとき
			Edit_DefaultParameterInfo.SetWindowText(CString(MAKEINTRESOURCE(IDS_ERROR_ILLEGAL_FORMAT_TYPE)));
		}
	}
}


//解凍後削除の設定に合わせてチェックボックスの有効無効を決める
LRESULT CConfigDlgCompressGeneral::OnCheckDelete(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if(BN_CLICKED==wNotifyCode){
		BOOL State=Check_DeleteAfterCompress.GetCheck();
		::EnableWindow(GetDlgItem(IDC_CHECK_MOVETO_RECYCLE_BIN),State);
		::EnableWindow(GetDlgItem(IDC_CHECK_DELETE_NOCONFIRM),State);
		::EnableWindow(GetDlgItem(IDC_CHECK_FORCE_DELETE),State);
	}
	return 0;
}
