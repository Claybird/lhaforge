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
#include "Dlg_Base.h"
#include "../../resource.h"
#include "../ConfigDLL.h"

class CConfigDialog;

//====================================
// DLLの有効/無効
//====================================
class CConfigDlgDLL : public CDialogImpl<CConfigDlgDLL>,public CMessageFilter,public CWinDataExchange<CConfigDlgDLL>,public IConfigDlgBase
{
protected:
	CConfigDLL m_Config;

	virtual BOOL PreTranslateMessage(MSG* pMsg){
		return IsDialogMessage(pMsg);
	}

	LRESULT OnInitDialog(HWND hWnd, LPARAM lParam);
public:
	enum { IDD = IDD_PROPPAGE_CONFIG_DLL };

#define MY_DDX_ENTRY(x)	DDX_CHECK(IDC_CHECK_ENABLE_##x,m_Config.EnableDLL[DLL_ID_##x])
	// DDXマップ
	BEGIN_DDX_MAP(CConfigDlgDLL)
		MY_DDX_ENTRY(UNLHA)
		MY_DDX_ENTRY(7ZIP)
		MY_DDX_ENTRY(CAB)
		MY_DDX_ENTRY(TAR)
		MY_DDX_ENTRY(JACK)
		MY_DDX_ENTRY(YZ1)
		MY_DDX_ENTRY(UNARJ)
		MY_DDX_ENTRY(UNGCA)
		MY_DDX_ENTRY(UNRAR)
		MY_DDX_ENTRY(UNACE)
		MY_DDX_ENTRY(UNIMP)
		MY_DDX_ENTRY(UNBEL)
		MY_DDX_ENTRY(UNHKI)
		MY_DDX_ENTRY(BGA)
		MY_DDX_ENTRY(AISH)
		MY_DDX_ENTRY(UNISO)
	END_DDX_MAP()
#undef MY_DDX_ENTRY

	// メッセージマップ
	BEGIN_MSG_MAP_EX(CConfigDlgDLL)
		MSG_WM_INITDIALOG(OnInitDialog)
		MSG_WM_DESTROY(OnDestroy)
	END_MSG_MAP()

	LRESULT OnApply();

	CConfigDlgDLL(){}

	LRESULT OnDestroy(){
		CMessageLoop* pLoop = _Module.GetMessageLoop();
		pLoop->RemoveMessageFilter(this);

		return TRUE;
	}

	void LoadConfig(CConfigManager& Config){
		m_Config.load(Config);
	}
	void StoreConfig(CConfigManager& Config){
		m_Config.store(Config);
	}
};

