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

	CComboBox Combo_SizeUnit;

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
		DDX_CHECK(IDC_CHECK_ZIP_SPECIFY_SPLIT_SIZE,m_Config.SpecifySplitSize)
		DDX_INT_RANGE(IDC_EDIT_ZIP_SPLIT_SIZE,m_Config.SplitSize,1,INT_MAX)
	END_DDX_MAP()

	// メッセージマップ
	BEGIN_MSG_MAP_EX(CConfigDlgZIP)
		MSG_WM_INITDIALOG(OnInitDialog)
		COMMAND_ID_HANDLER(IDC_CHECK_ZIP_SPECIFY_MEMORY_SIZE,OnSpecifyDeflateMemorySize)
		COMMAND_ID_HANDLER(IDC_CHECK_ZIP_SPECIFY_PASS_NUMBER,OnSpecifyDeflatePassNumber)
		COMMAND_ID_HANDLER(IDC_CHECK_ZIP_SPECIFY_SPLIT_SIZE,OnSpecifySplitSize)
		MSG_WM_DESTROY(OnDestroy)
	END_MSG_MAP()

	LRESULT OnInitDialog(HWND hWnd, LPARAM lParam);
	LRESULT OnApply();
	LRESULT OnSpecifyDeflateMemorySize(WORD,WORD,HWND,BOOL&);
	LRESULT OnSpecifyDeflatePassNumber(WORD,WORD,HWND,BOOL&);
	LRESULT OnSpecifySplitSize(WORD,WORD,HWND,BOOL&);

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

