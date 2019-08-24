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
#include "Dlg_xacrett.h"

//=================
// XACRETT設定画面
//=================
LRESULT CConfigDlgXACRETT::OnInitDialog(HWND hWnd, LPARAM lParam)
{
	// メッセージループにメッセージフィルタとアイドルハンドラを追加
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	pLoop->AddMessageFilter(this);

	//---------------------------
	// XacRett.DLLを使うかどうか
	//---------------------------
	Check_EnableXacrett=GetDlgItem(IDC_CHECK_ENABLE_XACRETT);
	Check_EnableXacrett.SetCheck(m_Config.EnableDLL);

	//DDX情報アップデート
	DoDataExchange(FALSE);

	//有効/無効
	::EnableWindow(GetDlgItem(IDC_EDIT_XACRETT_PRIORITY_EXTENSION),Check_EnableXacrett.GetCheck());
	return TRUE;
}

LRESULT CConfigDlgXACRETT::OnApply()
{
//===============================
// 設定をConfigManagerに書き戻す
//===============================
	//---------------------------
	// XacRett.DLLを使うかどうか
	//---------------------------
	m_Config.EnableDLL=Check_EnableXacrett.GetCheck();

	//---------------
	// DDXデータ更新
	//---------------
	if(!DoDataExchange(TRUE)){
		return FALSE;
	}

	return TRUE;
}

LRESULT CConfigDlgXACRETT::OnCheckEnableXacrett(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if(BN_CLICKED==wNotifyCode){
		//有効/無効
		::EnableWindow(GetDlgItem(IDC_EDIT_XACRETT_PRIORITY_EXTENSION),Check_EnableXacrett.GetCheck());
	}
	return 0;
}

