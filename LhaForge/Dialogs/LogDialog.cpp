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
#include "LogDialog.h"
#include "../Utilities/OSUtil.h"

void CLogDialog::SetData(LPCTSTR log)
{
	LogStr=log;

	//改行コードの変換
	// \nを\r\nと置き換える
	LogStr.Replace(_T("\n"),_T("\r\n"));
	LogStr.Replace(_T("\r\r\n"),_T("\r\n"));
}


LRESULT CLogDialog::OnInitDialog(HWND hWnd, LPARAM lParam)
{
	CenterWindow();
	LogView=GetDlgItem(IDC_EDIT_LOG);
	LogView.SetWindowText(LogStr);

	// ダイアログリサイズ初期化
	DlgResize_Init(true, true, WS_THICKFRAME | WS_CLIPCHILDREN);

	//---ウィンドウをアクティブにする
	UtilSetAbsoluteForegroundWindow(hWnd);
	return TRUE;
}


