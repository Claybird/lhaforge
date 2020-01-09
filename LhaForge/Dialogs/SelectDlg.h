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

#pragma once
#include "../resource.h"

//圧縮形式の選択
class CSelectDialog : public CDialogImpl<CSelectDialog>,public CWinDataExchange<CSelectDialog>
{
protected:
#pragma message("Need to be updated")
	bool bSFX;
	bool bSplit;
	bool bPassword;
	bool bPublicPassword;
	bool bSingleCompression;
	bool bDeleteAfterCompress;

	// DDXマップ
	BEGIN_DDX_MAP(CSelectDialog)
		DDX_CHECK(IDC_CHECK_COMPRESS_SFX,bSFX)
		DDX_CHECK(IDC_CHECK_COMPRESS_SPLIT,bSplit)
		DDX_CHECK(IDC_CHECK_COMPRESS_PASSWORD,bPassword)
		DDX_CHECK(IDC_CHECK_COMPRESS_PUBLIC_PASSWORD,bPublicPassword)
		DDX_CHECK(IDC_CHECK_SINGLE_COMPRESSION,bSingleCompression)
		DDX_CHECK(IDC_CHECK_DELETE_AFTER_COMPRESS,bDeleteAfterCompress)
	END_DDX_MAP()

	// メッセージマップ
	BEGIN_MSG_MAP_EX(CSelectDialog)
		MSG_WM_INITDIALOG(OnInitDialog)
		COMMAND_ID_HANDLER(IDC_CHECK_COMPRESS_SFX, OnSFX)
		COMMAND_ID_HANDLER(IDC_CHECK_COMPRESS_PASSWORD, OnPassword)
		MSG_WM_COMMAND(OnCommand)
	END_MSG_MAP()

	LRESULT OnPassword(WORD,WORD,HWND,BOOL&);
	LRESULT OnSFX(WORD,WORD,HWND,BOOL&);
	LRESULT OnInitDialog(HWND hWnd, LPARAM lParam);
	void OnCommand(UINT nCode, int nID, HWND hWnd);

public:
	enum {IDD = IDD_DIALOG_SELECT_COMPRESS_TYPE};
	CSelectDialog():bSFX(false),bSplit(false),bPassword(false),bPublicPassword(false),bSingleCompression(false),bDeleteAfterCompress(false){}
	virtual ~CSelectDialog(){}

	int GetOptions();
	bool IsSingleCompression(){return bSingleCompression;}
	bool GetDeleteAfterCompress(){return bDeleteAfterCompress;}
	void SetDeleteAfterCompress(bool b){bDeleteAfterCompress=b;}
};

//-----------------------------------

//圧縮形式選択:キャンセルでLF_FMT_INVALIDが返る
//SelectDialogのラッパ
enum LF_ARCHIVE_FORMAT;
LF_ARCHIVE_FORMAT SelectCompressType(int &Options,bool &bSingleCompression);
