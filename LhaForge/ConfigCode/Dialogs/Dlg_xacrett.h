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
#include "../ConfigManager.h"
#include "../../ArchiverCode/ArchiverXACRETT.h"
#include "../ConfigXACRETT.h"
#include "../../resource.h"

//====================================
// XACRETTの設定項目
//====================================
class CConfigDlgXACRETT : public CDialogImpl<CConfigDlgXACRETT>,public CWinDataExchange<CConfigDlgXACRETT>,public CMessageFilter,public IConfigDlgBase
{
protected:
	CConfigXACRETT	m_Config;

	CButton Check_EnableXacrett;

	virtual BOOL PreTranslateMessage(MSG* pMsg){
		return IsDialogMessage(pMsg);
	}

public:
	enum { IDD = IDD_PROPPAGE_CONFIG_XACRETT };

	// DDXマップ
	BEGIN_DDX_MAP(CConfigDlgXACRETT)
		DDX_TEXT(IDC_EDIT_XACRETT_PRIORITY_EXTENSION, m_Config.Extensions)
		DDX_CHECK(IDC_CHECK_ENABLE_XACRETT_MENU, m_Config.EnableShellMenu)
	END_DDX_MAP()

	// メッセージマップ
	BEGIN_MSG_MAP_EX(CConfigDlgXACRETT)
		MSG_WM_INITDIALOG(OnInitDialog)
		COMMAND_ID_HANDLER(IDC_CHECK_ENABLE_XACRETT,OnCheckEnableXacrett)
		MSG_WM_DESTROY(OnDestroy)
	END_MSG_MAP()

	LRESULT OnInitDialog(HWND hWnd, LPARAM lParam);
	LRESULT OnApply();
	LRESULT OnCheckEnableXacrett(WORD,WORD,HWND,BOOL&);

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

