/*
 * Copyright (c) 2005-2012, Claybird
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
#include "WaitDialog.h"

LRESULT CWaitDialog::OnInitDialog(HWND hWnd, LPARAM lParam)
{
	Static_MessageString=GetDlgItem(IDC_STATIC_WAIT_MESSAGE);
	Static_Icon=GetDlgItem(IDC_STATIC_ANIMATION_ICON);

	CImageList ImageList;
	//ImageList.Create(MAKEINTRESOURCE(IDB_BITMAP_ANIMATION),32,1,RGB(128,128,128));
	ImageList.CreateFromImage(MAKEINTRESOURCE(IDB_BITMAP_ANIMATION),32,1,CLR_DEFAULT,IMAGE_BITMAP,LR_CREATEDIBSECTION);
	int IconCount=ImageList.GetImageCount();
	m_CurrentIconNumber=0;
	m_IconArray.resize(IconCount);
	for(int i=0;i<IconCount;i++){
		m_IconArray[i]=ImageList.ExtractIcon(i);
	}
	m_LastTime=0;

	// 親ウィンドウの中央に配置
	CenterWindow();

	// メッセージループにメッセージフィルタとアイドルハンドラを追加
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	pLoop->AddMessageFilter(this);
	pLoop->AddIdleHandler(this);

	m_bAborted=false;
	return TRUE;
}

void CWaitDialog::OnDestroy()
{
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	pLoop->RemoveMessageFilter(this);
	pLoop->RemoveIdleHandler(this);

	m_IconArray.clear();
}

void CWaitDialog::AnimationCall()
{
	DWORD Now=timeGetTime();
	if(Now-m_LastTime<200)return;
	m_LastTime=Now;
	m_CurrentIconNumber++;
	m_CurrentIconNumber%=m_IconArray.size();
	Static_Icon.SetIcon(m_IconArray[m_CurrentIconNumber]);
}

void CWaitDialog::Prepare(HWND hWndParent,LPCTSTR lpMsg)
{
	Create(hWndParent);
	SetMessageString(lpMsg);
	ShowWindow(SW_SHOW);
	UpdateWindow();
}
