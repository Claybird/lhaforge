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

