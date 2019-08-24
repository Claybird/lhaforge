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
