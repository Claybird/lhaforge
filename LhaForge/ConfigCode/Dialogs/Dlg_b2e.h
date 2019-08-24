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
#include "../../resource.h"
#include "../../ArchiverCode/ArchiverB2E.h"
#include "../ConfigB2E.h"

//====================================
// B2Eの設定項目
//====================================
class CConfigDlgB2E : public CDialogImpl<CConfigDlgB2E>,public CWinDataExchange<CConfigDlgB2E>,public CMessageFilter,public IConfigDlgBase
{
protected:
	CConfigB2E	m_Config;

	CButton Check_EnableB2E;

	virtual BOOL PreTranslateMessage(MSG* pMsg){
		return IsDialogMessage(pMsg);
	}

public:
	enum { IDD = IDD_PROPPAGE_CONFIG_B2E };

	// DDXマップ
	BEGIN_DDX_MAP(CConfigDlgB2E)
		DDX_TEXT(IDC_EDIT_B2E_SCRIPT_DIRECTORY, m_Config.ScriptDirectory)
		DDX_TEXT(IDC_EDIT_B2E_PRIORITY_EXTENSION, m_Config.Extensions)
		DDX_CHECK(IDC_CHECK_PRIORITIZE_B2E,m_Config.Priority)
		DDX_CHECK(IDC_CHECK_ENABLE_B2E_MENU,m_Config.EnableShellMenu)
	END_DDX_MAP()

	// メッセージマップ
	BEGIN_MSG_MAP_EX(CConfigDlgB2E)
		MSG_WM_INITDIALOG(OnInitDialog)
		COMMAND_ID_HANDLER(IDC_BUTTON_BROWSE_B2E,OnBrowse)
		COMMAND_ID_HANDLER(IDC_CHECK_ENABLE_B2E,OnCheckEnable)
		COMMAND_ID_HANDLER(IDC_BUTTON_B2E_MENUCACHE_GENERATE,OnMenuCache)
		COMMAND_ID_HANDLER(IDC_BUTTON_B2E_MENUCACHE_DELETE,OnMenuCache)
		MSG_WM_DESTROY(OnDestroy)
	END_MSG_MAP()

	LRESULT OnInitDialog(HWND hWnd, LPARAM lParam);
	LRESULT OnApply();
	LRESULT OnBrowse(WORD,WORD,HWND,BOOL&);
	LRESULT OnMenuCache(WORD,WORD,HWND,BOOL&);

	LRESULT OnDestroy(){
		CMessageLoop* pLoop = _Module.GetMessageLoop();
		pLoop->RemoveMessageFilter(this);

		return TRUE;
	}
	LRESULT OnCheckEnable(WORD,WORD,HWND,BOOL&);

	void LoadConfig(CConfigManager& Config){
		m_Config.load(Config);
	}
	void StoreConfig(CConfigManager& Config){
		m_Config.store(Config);
	}
};

