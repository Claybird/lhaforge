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

#pragma once
#include "Dlg_Base.h"
#include "../ConfigManager.h"
#include "../../resource.h"
#include "../../ArchiverCode/arc_interface.h"
#include "../ConfigGeneral.h"

//====================================
// 一般設定項目
//====================================
class CConfigDlgGeneral : public CDialogImpl<CConfigDlgGeneral>,public CMessageFilter,public CWinDataExchange<CConfigDlgGeneral>,public IConfigDlgBase
{
protected:
	CConfigGeneral	m_Config;
	CButton Radio_LostDir[LOSTDIR_ITEM_COUNT];
	CButton Radio_LogView[LOGVIEW_ITEM_COUNT];

	CButton Check_UseFiler;
	CEdit Edit_FilerPath;
	CEdit Edit_FilerParam;
	CButton Button_BrowseFiler;

	virtual BOOL PreTranslateMessage(MSG* pMsg){
		return IsDialogMessage(pMsg);
	}

	LRESULT OnInitDialog(HWND hWnd, LPARAM lParam);
	LRESULT OnCheckFiler(WORD,WORD,HWND,BOOL&);
	LRESULT OnBrowseFiler(WORD,WORD,HWND,BOOL&);
public:
	enum { IDD = IDD_PROPPAGE_CONFIG_GENERAL };
	// DDXマップ
	BEGIN_DDX_MAP(CConfigDlgGeneral)
		DDX_CHECK(IDC_CHECK_WARN_NETWORK,m_Config.WarnNetwork)
		DDX_CHECK(IDC_CHECK_WARN_REMOVABLE,m_Config.WarnRemovable)
		DDX_CHECK(IDC_CHECK_NOTIFY_SHELL,m_Config.NotifyShellAfterProcess)
		DDX_RADIO(IDC_RADIO_PRIORITY_DEFAULT,m_Config.ProcessPriority)
	END_DDX_MAP()

	// メッセージマップ
	BEGIN_MSG_MAP_EX(CConfigDlgGeneral)
		MSG_WM_INITDIALOG(OnInitDialog)
		COMMAND_ID_HANDLER(IDC_CHECK_USE_FILER,OnCheckFiler)
		COMMAND_ID_HANDLER(IDC_BUTTON_BROWSE_FILER,OnBrowseFiler)
		MSG_WM_DESTROY(OnDestroy)
	END_MSG_MAP()

	LRESULT OnApply();

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

