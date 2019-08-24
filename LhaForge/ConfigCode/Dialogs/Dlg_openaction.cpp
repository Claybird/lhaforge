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

