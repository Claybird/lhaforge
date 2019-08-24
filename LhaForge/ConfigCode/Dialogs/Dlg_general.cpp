/*
 * Copyright (c) 2005-, Claybird
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
#include "Dlg_general.h"
#include "../../Dialogs/LFFolderDialog.h"


//==============
// 一般設定画面
//==============
LRESULT CConfigDlgGeneral::OnInitDialog(HWND hWnd, LPARAM lParam)
{
	// メッセージループにメッセージフィルタとアイドルハンドラを追加
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	pLoop->AddMessageFilter(this);

	//----------------------------------------
	// 出力先フォルダが見つからない場合の対処
	//----------------------------------------
	Radio_LostDir[LOSTDIR_ASK_TO_CREATE]=GetDlgItem(IDC_RADIO_LOSTDIR_ASK_TO_CREATE);
	Radio_LostDir[LOSTDIR_FORCE_CREATE]=GetDlgItem(IDC_RADIO_LOSTDIR_FORCE_CREATE);
	Radio_LostDir[LOSTDIR_ERROR]=GetDlgItem(IDC_RADIO_LOSTDIR_ERROR);

	Radio_LostDir[m_Config.OnDirNotFound].SetCheck(1);

	//-------------------------------------
	// 圧縮/解凍処理のログ表示のタイミング
	//-------------------------------------
	Radio_LogView[LOGVIEW_ON_ERROR]=GetDlgItem(IDC_RADIO_VIEW_LOG_ON_ERROR);
	Radio_LogView[LOGVIEW_ALWAYS]=GetDlgItem(IDC_RADIO_VIEW_LOG_ALWAYS);
	Radio_LogView[LOGVIEW_NEVER]=GetDlgItem(IDC_RADIO_VIEW_LOG_NEVER);

	Radio_LogView[m_Config.LogViewEvent].SetCheck(1);

	//----------------------
	// 出力先を開くファイラ
	//----------------------
	Check_UseFiler=GetDlgItem(IDC_CHECK_USE_FILER);
	Check_UseFiler.SetCheck(0!=m_Config.Filer.UseFiler);

	//ファイラのパス
	Edit_FilerPath=GetDlgItem(IDC_EDIT_FILER_PATH);
	Edit_FilerPath.SetLimitText(_MAX_PATH);
	Edit_FilerPath.SetWindowText(m_Config.Filer.FilerPath);
	Edit_FilerPath.EnableWindow(Check_UseFiler.GetCheck());

	//パラメータ
	Edit_FilerParam=GetDlgItem(IDC_EDIT_FILER_PARAM);
	Edit_FilerParam.SetLimitText(_MAX_PATH);
	Edit_FilerParam.SetWindowText(m_Config.Filer.Param);
	Edit_FilerParam.EnableWindow(Check_UseFiler.GetCheck());

	Button_BrowseFiler=GetDlgItem(IDC_BUTTON_BROWSE_FILER);
	Button_BrowseFiler.EnableWindow(Check_UseFiler.GetCheck());

	//一時フォルダ
	Edit_TempPath=GetDlgItem(IDC_EDIT_TEMP_PATH);
	Edit_TempPath.SetLimitText(_MAX_PATH);

	//DDX情報アップデート
	DoDataExchange(FALSE);

	return TRUE;
}

LRESULT CConfigDlgGeneral::OnApply()
{
//===============================
// 設定をConfigManagerに書き戻す
//===============================
	//---------------
	// DDXデータ更新
	//---------------
	if(!DoDataExchange(TRUE)){
		return FALSE;
	}

	//----------------------------------------
	// 出力先フォルダが見つからない場合の対処
	//----------------------------------------
	for(int Type=0;Type<COUNTOF(Radio_LostDir);Type++){
		if(Radio_LostDir[Type].GetCheck()){
			m_Config.OnDirNotFound=(LOSTDIR)Type;
			break;
		}
	}

	//-------------------------------------
	// 圧縮/解凍処理のログ表示のタイミング
	//-------------------------------------
	for(int Type=0;Type<COUNTOF(Radio_LogView);Type++){
		if(Radio_LogView[Type].GetCheck()){
			m_Config.LogViewEvent=(LOGVIEW)Type;
			break;
		}
	}

	//----------------------
	// 出力先を開くファイラ
	//----------------------
	m_Config.Filer.UseFiler=Check_UseFiler.GetCheck();
	Edit_FilerPath.GetWindowText(m_Config.Filer.FilerPath);
	Edit_FilerParam.GetWindowText(m_Config.Filer.Param);

	return TRUE;
}


LRESULT CConfigDlgGeneral::OnCheckFiler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if(BN_CLICKED==wNotifyCode){
		Edit_FilerPath.EnableWindow(Check_UseFiler.GetCheck());
		Edit_FilerParam.EnableWindow(Check_UseFiler.GetCheck());
		Button_BrowseFiler.EnableWindow(Check_UseFiler.GetCheck());
	}
	return 0;
}


LRESULT CConfigDlgGeneral::OnBrowseFiler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if(BN_CLICKED==wNotifyCode){
		CString FilerPath;

		Edit_FilerPath.GetWindowText(FilerPath);

		CFileDialog dlg(TRUE, NULL, FilerPath, OFN_NOCHANGEDIR,_T("Executable File(*.exe)\0*.exe\0All Files\0*.*\0\0"));
		if(IDOK==dlg.DoModal()){
			Edit_FilerPath.SetWindowText(dlg.m_szFileName);
		}
	}
	return 0;
}


LRESULT CConfigDlgGeneral::OnBrowseTempPath(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if(BN_CLICKED==wNotifyCode){
		CString path;
		Edit_TempPath.GetWindowText(path);

		CString title(MAKEINTRESOURCE(IDS_INPUT_TEMP_PATH));
		CLFFolderDialog dlg(NULL,title,BIF_RETURNONLYFSDIRS|BIF_NEWDIALOGSTYLE);
		if(IDOK==dlg.DoModal()){
			Edit_TempPath.SetWindowText(dlg.GetFolderPath());
		}else{
			//キャンセル
			return E_ABORT;
		}
	}
	return 0;
}
