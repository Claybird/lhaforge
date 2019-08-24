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
#include "Dlg_dll_update.h"

//=====================
// DLLアップデート画面
//=====================
LRESULT CConfigDlgDLLUpdate::OnInitDialog(HWND hWnd, LPARAM lParam)
{
	// メッセージループにメッセージフィルタとアイドルハンドラを追加
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	pLoop->AddMessageFilter(this);

	//------------------
	// アップデート関係
	//------------------
	Check_AskUpdate=GetDlgItem(IDC_CHECK_DLL_ASK_UPDATE);
	Check_AskUpdate.SetCheck(m_Config.AskUpdate);

	Check_SilentUpdate=GetDlgItem(IDC_CHECK_DLL_SILENT_UPDATE);
	Check_SilentUpdate.SetCheck(m_Config.SilentUpdate);
	Check_SilentUpdate.EnableWindow(m_Config.AskUpdate);

	UpDown_Interval=GetDlgItem(IDC_SPIN_DLL_UPDATE_INTERVAL);
	UpDown_Interval.SetPos(m_Config.Interval);
	UpDown_Interval.SetRange(1,365);

	BOOL State=Check_AskUpdate.GetCheck();
	UpDown_Interval.EnableWindow(State);
	::EnableWindow(GetDlgItem(IDC_EDIT_DLL_UPDATE_INTERVAL),State);

	//-------------------
	// DLLバージョン情報
	//-------------------
	List_DLLVersion=GetDlgItem(IDC_LIST_DLL_VERSION);

	//リストビューにカラムを追加
	CRect rect;
	List_DLLVersion.GetClientRect(rect);
	int nScrollWidth = GetSystemMetrics(SM_CXVSCROLL);
	List_DLLVersion.InsertColumn(0, CString(MAKEINTRESOURCE(IDS_DLL_INFO_NAME)), LVCFMT_LEFT, 80, -1);
	List_DLLVersion.InsertColumn(1, CString(MAKEINTRESOURCE(IDS_DLL_INFO_VERSION)), LVCFMT_LEFT,rect.Width() - 80 - nScrollWidth, -1);

	//各DLLの情報を追加
	for(int i=0;i<DLL_ID_ITEM_COUNT;i++){
		CArchiverDLL *Archiver=ArchiverManager.GetArchiver((DLL_ID)i,true,true);
		ASSERT(Archiver);
		List_DLLVersion.AddItem(i, 0, Archiver->GetName());
		if(!Archiver->IsOK()){
			List_DLLVersion.AddItem(i, 1,CString(MAKEINTRESOURCE(IDS_MESSAGE_DLL_NOT_AVAILABLE)));
		}else{
			CString vs;
			Archiver->GetVersionString(vs);
			List_DLLVersion.AddItem(i,1,vs);
		}
	}
	//DLL開放
	ArchiverManager.Free();

	//更新ボタンにシールドアイコンをつける
	Button_SetElevationRequiredState(GetDlgItem(IDC_BUTTON_DLL_UPDATE),TRUE);

	return TRUE;
}

LRESULT CConfigDlgDLLUpdate::OnDLLUpdate(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if(BN_CLICKED==wNotifyCode){
		//--------------------
		// LFCaldixのパス取得
		//--------------------
		TCHAR ExePath[_MAX_PATH+1];
		FILL_ZERO(ExePath);
		GetModuleFileName(GetModuleHandle(NULL), ExePath, _MAX_PATH);

		PathRemoveFileSpec(ExePath);
		PathAppend(ExePath,CString(MAKEINTRESOURCE(IDS_CALDIX_EXE_NAME)));
		if(32>=(int)ShellExecute(m_hWnd,NULL,ExePath,Check_SilentUpdate.GetCheck() ? _T("/silent"): NULL,NULL,SW_RESTORE)){
			//実行エラー
			CString strLastError;
			UtilGetLastErrorMessage(strLastError);

			CString msg;
			msg.Format(IDS_ERROR_CANNOT_EXECUTE,ExePath,(LPCTSTR)strLastError);

			ErrorMessage(msg);
		}
	}
	return 0;
}

LRESULT CConfigDlgDLLUpdate::OnApply()
{
//===============================
// 設定をConfigManagerに書き戻す
//===============================
	m_Config.AskUpdate=Check_AskUpdate.GetCheck();
	m_Config.SilentUpdate=Check_SilentUpdate.GetCheck();

	m_Config.Interval=UpDown_Interval.GetPos();

	return TRUE;
}

LRESULT CConfigDlgDLLUpdate::OnCheckAskUpdate(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if(BN_CLICKED==wNotifyCode){
		BOOL State=Check_AskUpdate.GetCheck();
		UpDown_Interval.EnableWindow(State);
		::EnableWindow(GetDlgItem(IDC_EDIT_DLL_UPDATE_INTERVAL),State);
		Check_SilentUpdate.EnableWindow(State);
	}
	return 0;
}
