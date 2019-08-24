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
#include "Dlg_bga.h"

//=================
// BGA一般設定画面
//=================
LRESULT CConfigDlgBGA::OnInitDialog(HWND hWnd, LPARAM lParam)
{
	// メッセージループにメッセージフィルタとアイドルハンドラを追加
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	pLoop->AddMessageFilter(this);

	//--------------------------------
	// GZA圧縮レベルの設定用スライダ
	//--------------------------------
	Track_GZA_Level=GetDlgItem(IDC_SLIDER_GZA_COMPRESS_LEVEL);
	Track_GZA_Level.SetRange(GZA_COMPRESS_LEVEL_LOWEST,GZA_COMPRESS_LEVEL_HIGHEST);
	Track_GZA_Level.SetTicFreq(1);
	Track_GZA_Level.SetPageSize(1);
	Track_GZA_Level.SetLineSize(1);
	Track_GZA_Level.SetPos(m_Config.GZALevel);

	//----------------------------------
	// GZA圧縮レベルの確認用エディット
	//----------------------------------
	Edit_GZA_Level=GetDlgItem(IDC_EDIT_GZA_COMPRESS_LEVEL);
	CString Buffer;
	Buffer.Format(_T("%d"),m_Config.GZALevel);
	Edit_GZA_Level.SetWindowText(Buffer);

	//---------------------------------
	// BZA圧縮レベルの設定用スライダ
	//---------------------------------
	Track_BZA_Level=GetDlgItem(IDC_SLIDER_BZA_COMPRESS_LEVEL);
	Track_BZA_Level.SetRange(BZA_COMPRESS_LEVEL_LOWEST,BZA_COMPRESS_LEVEL_HIGHEST);
	Track_BZA_Level.SetTicFreq(1);
	Track_BZA_Level.SetPageSize(1);
	Track_BZA_Level.SetLineSize(1);
	Track_BZA_Level.SetPos(m_Config.BZALevel);

	//-----------------------------------
	// BZA圧縮レベルの確認用エディット
	//-----------------------------------
	Edit_BZA_Level=GetDlgItem(IDC_EDIT_BZA_COMPRESS_LEVEL);
	Buffer.Format(_T("%d"),m_Config.BZALevel);
	Edit_BZA_Level.SetWindowText(Buffer);

	return TRUE;
}

LRESULT CConfigDlgBGA::OnApply()
{
//===============================
// 設定をConfigManagerに書き戻す
//===============================
	//----------------
	// GZA圧縮レベル
	//----------------
	m_Config.GZALevel=Track_GZA_Level.GetPos();

	//-----------------
	// BZA圧縮レベル
	//-----------------
	m_Config.BZALevel=Track_BZA_Level.GetPos();
	return TRUE;
}


void CConfigDlgBGA::OnHScroll(int, short, HWND)
{
	CString Buffer;
	Buffer.Format(_T("%d"),Track_GZA_Level.GetPos());
	Edit_GZA_Level.SetWindowText(Buffer);
	Buffer.Format(_T("%d"),Track_BZA_Level.GetPos());
	Edit_BZA_Level.SetWindowText(Buffer);
}

