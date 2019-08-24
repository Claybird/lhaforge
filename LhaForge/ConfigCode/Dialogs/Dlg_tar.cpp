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
#include "Dlg_tar.h"

//=================
// TAR一般設定画面
//=================
LRESULT CConfigDlgTAR::OnInitDialog(HWND hWnd, LPARAM lParam)
{
	// メッセージループにメッセージフィルタとアイドルハンドラを追加
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	pLoop->AddMessageFilter(this);

	CString Buffer;

#define GUI_CONFIG(NAME,name)	\
	/*--------------------------------*/\
	/* ##NAME##圧縮レベルの設定用スライダ */\
	/*--------------------------------*/\
	Track_##NAME##_Level=GetDlgItem(IDC_SLIDER_##NAME##_COMPRESS_LEVEL);\
	Track_##NAME##_Level.SetRange(##NAME##_COMPRESS_LEVEL_LOWEST,##NAME##_COMPRESS_LEVEL_HIGHEST);\
	Track_##NAME##_Level.SetTicFreq(1);\
	Track_##NAME##_Level.SetPageSize(1);\
	Track_##NAME##_Level.SetLineSize(1);\
	Track_##NAME##_Level.SetPos(m_Config.##name##CompressLevel);\
	/*----------------------------------*/\
	/* ##NAME##圧縮レベルの確認用エディット */\
	/*----------------------------------*/\
	Edit_##NAME##_Level=GetDlgItem(IDC_EDIT_##NAME##_COMPRESS_LEVEL);\
	Buffer.Format(_T("%d"),m_Config.name##CompressLevel);\
	Edit_##NAME##_Level.SetWindowText(Buffer);

	GUI_CONFIG(GZIP,Gzip)
	GUI_CONFIG(BZIP2,Bzip2)
	GUI_CONFIG(XZ,XZ)
	GUI_CONFIG(LZMA,LZMA)

#undef GUI_CONFIG

	//文字コード変換設定
	Check_ConvertCharset=GetDlgItem(IDC_CHECK_TAR_CONVERT_CHARSET);
	Check_ConvertCharset.SetCheck(m_Config.bConvertCharset);


	//ソートモード
	Radio_SortBy[TAR_SORT_BY_NONE]=GetDlgItem(IDC_RADIO_TAR_SORT_BY_NONE);
	Radio_SortBy[TAR_SORT_BY_EXT]=GetDlgItem(IDC_RADIO_TAR_SORT_BY_EXT);
	Radio_SortBy[TAR_SORT_BY_PATH]=GetDlgItem(IDC_RADIO_TAR_SORT_BY_PATH);

	Radio_SortBy[m_Config.SortBy].SetCheck(1);

	return TRUE;
}

LRESULT CConfigDlgTAR::OnApply()
{
//===============================
// 設定をConfigManagerに書き戻す
//===============================
	// GZIP圧縮レベル
	m_Config.GzipCompressLevel=Track_GZIP_Level.GetPos();

	// BZIP2圧縮レベル
	m_Config.Bzip2CompressLevel=Track_BZIP2_Level.GetPos();

	// XZ圧縮レベル
	m_Config.XZCompressLevel=Track_XZ_Level.GetPos();

	// LZMA圧縮レベル
	m_Config.LZMACompressLevel=Track_LZMA_Level.GetPos();

	//文字コード変換設定
	m_Config.bConvertCharset=Check_ConvertCharset.GetCheck();

	//ソートモード
	for(int SortBy=0;SortBy<COUNTOF(Radio_SortBy);SortBy++){
		if(Radio_SortBy[SortBy].GetCheck()){
			m_Config.SortBy=SortBy;
			break;
		}
	}
	return TRUE;
}


void CConfigDlgTAR::OnHScroll(int, short, HWND)
{
	CString Buffer;
	Buffer.Format(_T("%d"),Track_GZIP_Level.GetPos());
	Edit_GZIP_Level.SetWindowText(Buffer);
	Buffer.Format(_T("%d"),Track_BZIP2_Level.GetPos());
	Edit_BZIP2_Level.SetWindowText(Buffer);
	Buffer.Format(_T("%d"),Track_XZ_Level.GetPos());
	Edit_XZ_Level.SetWindowText(Buffer);
	Buffer.Format(_T("%d"),Track_LZMA_Level.GetPos());
	Edit_LZMA_Level.SetWindowText(Buffer);
}

