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
#include "ProgressDlg.h"

LRESULT CProgressDialog::OnInitDialog(HWND hWnd, LPARAM lParam)
{
	m_Info=GetDlgItem(IDC_STATIC_INFO);
	m_Progress=GetDlgItem(IDC_PROGRESS_STATUS);
	m_Info.GetWindowText(m_strInfo);
	GetWindowText(m_strTitle);
	m_TotalFiles=0;
	m_CurrentIndex=0;
	m_Progress.SetPos(0);

	//ダイアログの場所を画面左上にする
	SetWindowPos(NULL,100,100,0,0,SWP_NOSIZE);


	//メッセージフィルタの設定
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	pLoop->AddMessageFilter(this);
	return TRUE;
}

void CProgressDialog::OnDestroy()
{
	//メッセージフィルタの除去
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	pLoop->RemoveMessageFilter(this);
}

void CProgressDialog::SetTotalFileCount(int nFiles)
{
	m_TotalFiles=nFiles;
	m_Progress.SetRange(0,nFiles);
	m_Progress.SetPos(0);
}

void CProgressDialog::SetNextState(LPCTSTR lpszFile)
{
	m_CurrentIndex++;
	//処理中のファイルを明示
	CString tmp;
	tmp.Format(m_strInfo,lpszFile);
	m_Info.SetWindowText(tmp);

	//処理中のファイル数を表示
	tmp.Format(m_strTitle,m_CurrentIndex,m_TotalFiles);
	SetWindowText(tmp);

	//プログレスバーを設定
	m_Progress.SetPos(m_CurrentIndex-1);
	m_Progress.UpdateWindow();
}
