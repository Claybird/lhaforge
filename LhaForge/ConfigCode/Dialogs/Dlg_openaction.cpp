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
#include "Dlg_openaction.h"

//======================
// 関連付け動作設定画面
//======================
LRESULT CConfigDlgOpenAction::OnInitDialog(HWND hWnd, LPARAM lParam)
{
	// メッセージループにメッセージフィルタとアイドルハンドラを追加
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	pLoop->AddMessageFilter(this);

	//----------------
	// 開く動作の設定
	//----------------
	//---通常
	Radio_OpenAction[OPENACTION_EXTRACT]=GetDlgItem(IDC_RADIO_OPENACTION_EXTRACT);
	Radio_OpenAction[OPENACTION_LIST]=GetDlgItem(IDC_RADIO_OPENACTION_LIST);
	Radio_OpenAction[OPENACTION_TEST]=GetDlgItem(IDC_RADIO_OPENACTION_TEST);
	Radio_OpenAction[OPENACTION_ASK]=GetDlgItem(IDC_RADIO_OPENACTION_ASK);

	Radio_OpenAction[m_Config.OpenAction].SetCheck(true);
	//---Shift押下時
	Radio_OpenAction_Shift[OPENACTION_EXTRACT]=GetDlgItem(IDC_RADIO_OPENACTION_EXTRACT_SHIFT);
	Radio_OpenAction_Shift[OPENACTION_LIST]=GetDlgItem(IDC_RADIO_OPENACTION_LIST_SHIFT);
	Radio_OpenAction_Shift[OPENACTION_TEST]=GetDlgItem(IDC_RADIO_OPENACTION_TEST_SHIFT);
	Radio_OpenAction_Shift[OPENACTION_ASK]=GetDlgItem(IDC_RADIO_OPENACTION_ASK_SHIFT);

	Radio_OpenAction_Shift[m_Config.OpenAction_Shift].SetCheck(true);
	//---Ctrl押下時
	Radio_OpenAction_Ctrl[OPENACTION_EXTRACT]=GetDlgItem(IDC_RADIO_OPENACTION_EXTRACT_CTRL);
	Radio_OpenAction_Ctrl[OPENACTION_LIST]=GetDlgItem(IDC_RADIO_OPENACTION_LIST_CTRL);
	Radio_OpenAction_Ctrl[OPENACTION_TEST]=GetDlgItem(IDC_RADIO_OPENACTION_TEST_CTRL);
	Radio_OpenAction_Ctrl[OPENACTION_ASK]=GetDlgItem(IDC_RADIO_OPENACTION_ASK_CTRL);

	Radio_OpenAction_Ctrl[m_Config.OpenAction_Ctrl].SetCheck(true);

	return TRUE;
}

LRESULT CConfigDlgOpenAction::OnApply()
{
//===============================
// 設定をConfigManagerに書き戻す
//===============================
	//----------------
	// 開く動作の設定
	//----------------
	//---通常
	for(int i=0;i<COUNTOF(Radio_OpenAction);i++){
		if(Radio_OpenAction[i].GetCheck()){
			m_Config.OpenAction=(OPENACTION)i;
			break;
		}
	}
	//---Shift押下時
	for(int i=0;i<COUNTOF(Radio_OpenAction_Shift);i++){
		if(Radio_OpenAction_Shift[i].GetCheck()){
			m_Config.OpenAction_Shift=(OPENACTION)i;
			break;
		}
	}
	//---Ctrl押下時
	for(int i=0;i<COUNTOF(Radio_OpenAction_Ctrl);i++){
		if(Radio_OpenAction_Ctrl[i].GetCheck()){
			m_Config.OpenAction_Ctrl=(OPENACTION)i;
			break;
		}
	}

	return TRUE;
}

