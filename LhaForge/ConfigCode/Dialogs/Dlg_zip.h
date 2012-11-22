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
#include "../../ArchiverCode/Archiver7ZIP.h"
#include "../ConfigZIP.h"

//====================================
// ZIPの設定項目
//====================================
class CConfigDlgZIP : public CDialogImpl<CConfigDlgZIP>,public CWinDataExchange<CConfigDlgZIP>,public CMessageFilter,public IConfigDlgBase
{
protected:
	CConfigZIP		m_Config;

	CButton			Radio_CompressType[ZIP_COMPRESS_TYPE_ITEM_COUNT];
	CButton			Radio_CompressLevel[ZIP_COMPRESS_LEVEL_ITEM_COUNT];

	CButton			Check_SpecifyDeflateMemorySize;
	CButton			Check_SpecifyDeflatePassNumber;
	virtual BOOL PreTranslateMessage(MSG* pMsg){
		return IsDialogMessage(pMsg);
	}

public:
	enum { IDD = IDD_PROPPAGE_CONFIG_ZIP };

	// DDXマップ
	BEGIN_DDX_MAP(CConfigDlgZIP)
		DDX_INT_RANGE(IDC_EDIT_ZIP_DEFLATE_MEMORY_SIZE, m_Config.DeflateMemorySize, (int)ZIP_DEFLATE_MEMORY_SIZE_LOWEST, (int)ZIP_DEFLATE_MEMORY_SIZE_HIGHEST)
		DDX_INT_RANGE(IDC_EDIT_ZIP_DEFLATE_PASS_NUMBER, m_Config.DeflatePassNumber, (int)ZIP_DEFLATE_PASS_NUMBER_LOWEST, (int)ZIP_DEFLATE_PASS_NUMBER_HIGHEST)
		DDX_CHECK(IDC_CHECK_ZIP_FORCE_UTF8,m_Config.ForceUTF8)
		DDX_RADIO(IDC_RADIO_ZIPCRYPTO,m_Config.CryptoMode)
	END_DDX_MAP()

	// メッセージマップ
	BEGIN_MSG_MAP_EX(CConfigDlgZIP)
		MSG_WM_INITDIALOG(OnInitDialog)
		COMMAND_ID_HANDLER(IDC_CHECK_ZIP_SPECIFY_MEMORY_SIZE,OnSpecifyDeflateMemorySize)
		COMMAND_ID_HANDLER(IDC_CHECK_ZIP_SPECIFY_PASS_NUMBER,OnSpecifyDeflatePassNumber)
		MSG_WM_DESTROY(OnDestroy)
	END_MSG_MAP()

	LRESULT OnInitDialog(HWND hWnd, LPARAM lParam);
	LRESULT OnApply();
	LRESULT OnSpecifyDeflateMemorySize(WORD,WORD,HWND,BOOL&);
	LRESULT OnSpecifyDeflatePassNumber(WORD,WORD,HWND,BOOL&);

	void OnDataValidateError(UINT nCtrlID, BOOL bSave, _XData& data){
		CString msg; 
		msg.Format(IDS_ERROR_VALUE_RANGE,data.intData.nMin, data.intData.nMax);
		ErrorMessage(msg);

		::SetFocus(GetDlgItem(nCtrlID));
		::SendMessage(GetDlgItem(nCtrlID),EM_SETSEL,0,-1);
	}

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

