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
