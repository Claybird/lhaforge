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
#include "Dlg_B2E.h"
#include "../../ArchiverManager.h"
#include "../../Dialogs/LFFolderDialog.h"

//=================
// B2E設定画面
//=================
LRESULT CConfigDlgB2E::OnInitDialog(HWND hWnd, LPARAM lParam)
{
	// メッセージループにメッセージフィルタとアイドルハンドラを追加
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	pLoop->AddMessageFilter(this);

	//---------------------------
	// B2E.DLLを使うかどうか
	//---------------------------
	Check_EnableB2E=GetDlgItem(IDC_CHECK_ENABLE_B2E);
	Check_EnableB2E.SetCheck(m_Config.EnableDLL);

	//DDX情報アップデート
	DoDataExchange(FALSE);

	//有効/無効
	::EnableWindow(GetDlgItem(IDC_CHECK_PRIORITIZE_B2E),Check_EnableB2E.GetCheck());
	::EnableWindow(GetDlgItem(IDC_EDIT_B2E_PRIORITY_EXTENSION),Check_EnableB2E.GetCheck());


	//アクティブなDLL
	{
#if !defined(_UNICODE)&&!defined(UNICODE)
#error("UNICODE版のみ実装")
#endif
		CListViewCtrl List_B2E=GetDlgItem(IDC_LIST_ACTIVE_B2E);
		//リストビューにカラムを追加
		CRect rect;
		List_B2E.GetClientRect(rect);
		int nScrollWidth = GetSystemMetrics(SM_CXVSCROLL);
		List_B2E.InsertColumn(0, CString(MAKEINTRESOURCE(IDS_B2E_SCRIPT_NAME)), LVCFMT_LEFT, rect.Width()-nScrollWidth, -1);

		CArchiverB2E &ArcB2E=CArchiverDLLManager::GetInstance().GetB2EHandler();
		if(ArcB2E.IsOK()){
			//B2E情報取得
			std::vector<CString> ScriptNames;
			if(!ArcB2E.EnumActiveB2EScriptNames(ScriptNames))ScriptNames.clear();

			//各B2Eの情報を追加
			for(size_t i=0;i<ScriptNames.size();i++){
				List_B2E.AddItem(i, 0, ScriptNames[i]);
			}
		}
	}

	return TRUE;
}

LRESULT CConfigDlgB2E::OnApply()
{
//===============================
// 設定をConfigManagerに書き戻す
//===============================
	//---------------------------
	// B2E.DLLを使うかどうか
	//---------------------------
	m_Config.EnableDLL=Check_EnableB2E.GetCheck();

	//---------------
	// DDXデータ更新
	//---------------
	if(!DoDataExchange(TRUE)){
		return FALSE;
	}

	return TRUE;
}


LRESULT CConfigDlgB2E::OnBrowse(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if(BN_CLICKED==wNotifyCode){
		//DDX取得
		DoDataExchange(TRUE);
		CLFFolderDialog dlg(m_hWnd,NULL,BIF_RETURNONLYFSDIRS|BIF_NEWDIALOGSTYLE);
		dlg.SetInitialFolder(m_Config.ScriptDirectory);
		if(IDOK==dlg.DoModal()){
			m_Config.ScriptDirectory=dlg.GetFolderPath();
		}
		//DDX情報アップデート
		DoDataExchange(FALSE);
	}
	return 0;
}

LRESULT CConfigDlgB2E::OnCheckEnable(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if(BN_CLICKED==wNotifyCode){
		//有効/無効
		::EnableWindow(GetDlgItem(IDC_CHECK_PRIORITIZE_B2E),Check_EnableB2E.GetCheck());
		::EnableWindow(GetDlgItem(IDC_EDIT_B2E_PRIORITY_EXTENSION),Check_EnableB2E.GetCheck());
	}
	return 0;
}


//B2E圧縮メニューキャッシュの生成と削除を行う
LRESULT CConfigDlgB2E::OnMenuCache(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if(BN_CLICKED!=wNotifyCode){
		return 0;
	}
	bool bDummy;	//ダミー
	CPath strPath;	//ファイルパス
	//メニューキャッシュファイル名取得
	UtilGetDefaultFilePath(strPath,PROGRAMDIR_NAME,_T("B2EMenu.dat"),bDummy);
	if(IDC_BUTTON_B2E_MENUCACHE_GENERATE==wID){
		//メニューキャッシュ生成
		//既存のキャッシュは削除
		if(strPath.FileExists()){
			::DeleteFile(strPath);
		}

		//---------------
		// B2Eの情報取得
		//---------------
		//B2Eハンドラ
		CArchiverB2E &ArcB2E=CArchiverDLLManager::GetInstance().GetB2EHandler();
		if(!ArcB2E.IsOK()){
			CString msg;
			msg.Format(IDS_ERROR_DLL_LOAD,ArcB2E.GetName());
			ErrorMessage(msg);
			return 0;
		}

		//B2E情報取得
		std::vector<B2ESCRIPTINFO> ScriptInfoArray;
		if(!ArcB2E.EnumCompressB2EScript(ScriptInfoArray))ScriptInfoArray.clear();

#if !defined(_UNICODE)&&!defined(UNICODE)
 #error("UNICODE版のみ実装")
#endif
		//使えるB2Eがあれば
		if(!ScriptInfoArray.empty()){
			HANDLE hFile=CreateFile(strPath,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
			if(INVALID_HANDLE_VALUE==hFile){
				CString msg;
				msg.Format(IDS_ERROR_ACCESS_OUTPUT_FILE,strPath);
				ErrorMessage(msg);
				return false;
			}

			//---形式
			for(UINT uFmt=0;uFmt<ScriptInfoArray.size();uFmt++){
				CString strFormatInfo;	//形式の情報([形式名]\t[メソッド名])
				strFormatInfo=CA2T(ScriptInfoArray[uFmt].szFormat);
				//---メソッド
				if(ScriptInfoArray[uFmt].MethodArray.empty()){
					strFormatInfo+=_T("\tDefault");
				}
				else{
					for(UINT uMthd=0;uMthd<ScriptInfoArray[uFmt].MethodArray.size();uMthd++){
						strFormatInfo+=_T("\t");
						strFormatInfo+=CA2T(ScriptInfoArray[uFmt].MethodArray[uMthd]);
					}
				}
				strFormatInfo+=_T("\r\n");
				//ファイルに出力
				DWORD dwWritten=0;
				WriteFile(hFile,(LPCBYTE)(LPCTSTR)strFormatInfo,_tcslen(strFormatInfo)*sizeof(TCHAR),&dwWritten,NULL);
			}
			CloseHandle(hFile);
		}
	}else{
		//メニューキャッシュ削除
		if(strPath.FileExists()){
			::DeleteFile(strPath);
		}
	}
	return 0;
}
