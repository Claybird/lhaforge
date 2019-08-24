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
#include "../../ArchiverCode/Archiver7ZIP.h"
#include "../Config7Z.h"
#include "../../resource.h"

//====================================
// 7Zの設定項目
//====================================
class CConfigDlg7Z : public CDialogImpl<CConfigDlg7Z>,public CWinDataExchange<CConfigDlg7Z>,public CMessageFilter,public IConfigDlgBase
{
protected:
	CConfig7Z		m_Config;

	CButton			Check_UsePreset;
	CButton			Radio_CompressType[SEVEN_ZIP_COMPRESS_TYPE_ITEM_COUNT];
	CButton			Radio_CompressLevel[SEVEN_ZIP_COMPRESS_LEVEL_ITEM_COUNT];
	CButton			Radio_LZMA_Mode[SEVEN_ZIP_LZMA_MODE_ITEM_COUNT];

	CButton			Check_SpecifyPPMdModelSize;
	CButton			Check_HeaderCompression;
	CButton			Check_SpecifySplitSize;

	CComboBox Combo_SizeUnit;

	virtual BOOL PreTranslateMessage(MSG* pMsg){
		return IsDialogMessage(pMsg);
	}

public:
	enum { IDD = IDD_PROPPAGE_CONFIG_7Z };

	// DDXマップ
	BEGIN_DDX_MAP(CConfigDlg7Z)
		DDX_INT_RANGE(IDC_EDIT_7Z_PPMD_MODEL_SIZE, m_Config.PPMdModelSize, (int)SEVEN_ZIP_PPMD_MODEL_SIZE_LOWEST, (int)SEVEN_ZIP_PPMD_MODEL_SIZE_HIGHEST)
		DDX_CHECK(IDC_CHECK_7Z_SOLID_MODE, m_Config.SolidMode)
		DDX_CHECK(IDC_CHECK_7Z_HEADER_COMPRESSION, m_Config.HeaderCompression)
		//DDX_CHECK(IDC_CHECK_7Z_FULL_HEADER_COMPRESSION, m_Config.FullHeaderCompression)
		DDX_CHECK(IDC_CHECK_7Z_HEADER_ENCRYPT, m_Config.HeaderEncryption)
		DDX_CHECK(IDC_CHECK_7Z_SPECIFY_SPLIT_SIZE, m_Config.SpecifySplitSize)
		DDX_INT_RANGE(IDC_EDIT_7Z_SPLIT_SIZE, m_Config.SplitSize, 1, INT_MAX)
	END_DDX_MAP()

	// メッセージマップ
	BEGIN_MSG_MAP_EX(CConfigDlg7Z)
		MSG_WM_INITDIALOG(OnInitDialog)
		COMMAND_ID_HANDLER(IDC_CHECK_7Z_HEADER_COMPRESSION,OnHeaderCompression)
		COMMAND_ID_HANDLER(IDC_CHECK_7Z_USE_PRESET,OnUsePreset)
		COMMAND_ID_HANDLER(IDC_CHECK_7Z_SPECIFY_SPLIT_SIZE,OnSpecifySplitSize)
		COMMAND_ID_HANDLER(IDC_CHECK_7Z_SPECIFY_PPMD_MODEL_SIZE,OnSpecifyPPMdModelSize)
		COMMAND_RANGE_HANDLER(IDC_RADIO_7Z_METHOD_LZMA,IDC_RADIO_7Z_METHOD_LZMA2, OnSelectCompressType)
		MSG_WM_DESTROY(OnDestroy)
	END_MSG_MAP()

	LRESULT OnInitDialog(HWND hWnd, LPARAM lParam);
	LRESULT OnApply();
	LRESULT OnHeaderCompression(WORD,WORD,HWND,BOOL&);
	LRESULT OnUsePreset(WORD,WORD,HWND,BOOL&);
	LRESULT OnSpecifyPPMdModelSize(WORD,WORD,HWND,BOOL&);
	LRESULT OnSelectCompressType(WORD,WORD,HWND,BOOL&);
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

