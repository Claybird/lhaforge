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
#include "../ArchiverCode/ArchiverB2E.h"

//圧縮形式の選択
class CSelectDialog : public CDialogImpl<CSelectDialog>,public CWinDataExchange<CSelectDialog>
{
protected:
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

//B2Eで圧縮形式の選択
class CB2ESelectDialog : public CDialogImpl<CB2ESelectDialog>,public CWinDataExchange<CB2ESelectDialog>
{
protected:
	bool m_bSFX;
	bool m_bSingleCompression;
	CComboBox Combo_Format;		//圧縮形式指定
	CComboBox Combo_Method;		//圧縮メソッド指定
	CString m_strFormat;
	CString m_strMethod;
	std::vector<B2ESCRIPTINFO> m_ScriptInfoArray;

	// DDXマップ
	BEGIN_DDX_MAP(CSelectDialog)
		DDX_CHECK(IDC_CHECK_COMPRESS_SFX,m_bSFX)
		DDX_CHECK(IDC_CHECK_SINGLE_COMPRESSION,m_bSingleCompression)
		DDX_CONTROL_HANDLE(IDC_COMBO_B2E_FORMAT,Combo_Format)
		DDX_CONTROL_HANDLE(IDC_COMBO_B2E_METHOD,Combo_Method)
	END_DDX_MAP()

	// メッセージマップ
	BEGIN_MSG_MAP_EX(CB2ESelectDialog)
		MSG_WM_INITDIALOG(OnInitDialog)
		COMMAND_HANDLER_EX(IDC_COMBO_B2E_FORMAT, CBN_SELCHANGE, OnComboFormat)
		MSG_WM_COMMAND(OnCommand)
	END_MSG_MAP()

	LRESULT OnInitDialog(HWND hWnd, LPARAM lParam);
	void OnCommand(UINT nCode, int nID, HWND hWnd);
	void OnComboFormat(UINT uNotifyCode, int nID, HWND hWndCtl);
public:
	enum {IDD = IDD_DIALOG_SELECT_COMPRESS_TYPE_B2E};
	CB2ESelectDialog():m_bSFX(false),m_bSingleCompression(false){}
	virtual ~CB2ESelectDialog(){}

	bool IsSFX(){return m_bSFX;}
	bool IsSingleCompression(){return m_bSingleCompression;}

	LPCTSTR GetFormat(){return m_strFormat;}
	LPCTSTR GetMethod(){return m_strMethod;}
};


//圧縮形式選択:キャンセルでPARAMETER_UNDEFINEDが返る
//SelectDialogのラッパ
PARAMETER_TYPE SelectCompressType(int &Options,bool &bSingleCompression,CString &strB2EFormat,CString &strB2EMethod,bool &bB2ESFX);
