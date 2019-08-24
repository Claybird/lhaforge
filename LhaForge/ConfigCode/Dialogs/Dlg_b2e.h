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

