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
#include "../ConfigCompress.h"
#include "../../ArchiverCode/arc_interface.h"

//====================================
// 圧縮一般設定
//====================================
class CConfigDlgCompressGeneral : public CDialogImpl<CConfigDlgCompressGeneral>,public CMessageFilter,public CWinDataExchange<CConfigDlgCompressGeneral>,public IConfigDlgBase
{
protected:
	CConfigCompress	m_Config;
	CButton Radio_CompressTo[OUTPUT_TO_ITEM_COUNT];
	CEdit Edit_CompressOutputDirPath;
	CButton Button_CompressToFolder;
	CUpDownCtrl UpDown_MaxCompressFileCount;
	CButton Check_LimitCompressFileCount;
	CButton Check_UseDefaultParameter;
	CButton Check_DeleteAfterCompress;
	CEdit Edit_DefaultParameterInfo;
	virtual BOOL PreTranslateMessage(MSG* pMsg){
		return IsDialogMessage(pMsg);
	}

	void SetParameterInfo();//Editに現在のパラメータの情報を表示する
public:
	enum { IDD = IDD_PROPPAGE_CONFIG_COMPRESS_GENERAL };

	// DDXマップ
	BEGIN_DDX_MAP(CConfigGeneral)
		DDX_CHECK(IDC_CHECK_ALWAYS_SPEFICY_OUTPUT_FILENAME, m_Config.SpecifyOutputFilename)
		DDX_CHECK(IDC_CHECK_OPEN_FOLDER_AFTER_COMPRESS, m_Config.OpenDir)
		DDX_CHECK(IDC_CHECK_LIMIT_COMPRESS_FILECOUNT,m_Config.LimitCompressFileCount)
		DDX_CHECK(IDC_CHECK_USE_DEFAULTPARAMETER,m_Config.UseDefaultParameter)
		DDX_CHECK(IDC_CHECK_DELETE_AFTER_COMPRESS,m_Config.DeleteAfterCompress)
		DDX_CHECK(IDC_CHECK_MOVETO_RECYCLE_BIN,m_Config.MoveToRecycleBin)
		DDX_CHECK(IDC_CHECK_DELETE_NOCONFIRM,m_Config.DeleteNoConfirm)
		DDX_CHECK(IDC_CHECK_FORCE_DELETE,m_Config.ForceDelete)
		DDX_CHECK(IDC_CHECK_IGNORE_TOP_DIRECTORY,m_Config.IgnoreTopDirectory)
	END_DDX_MAP()

	// メッセージマップ
	BEGIN_MSG_MAP_EX(CConfigDlgCompressGeneral)
		MSG_WM_INITDIALOG(OnInitDialog)
		COMMAND_RANGE_HANDLER(IDC_RADIO_COMPRESS_TO_DESKTOP,IDC_RADIO_COMPRESS_TO_ALWAYS_ASK_WHERE, OnRadioCompressTo)
		COMMAND_ID_HANDLER(IDC_BUTTON_COMPRESS_BROWSE_FOLDER,OnBrowseFolder)
		COMMAND_ID_HANDLER(IDC_CHECK_LIMIT_COMPRESS_FILECOUNT,OnCheckLimitCompressFileCount)
		COMMAND_ID_HANDLER(IDC_CHECK_USE_DEFAULTPARAMETER,OnCheckUseDefaultParameter)
		COMMAND_ID_HANDLER(IDC_BUTTON_SELECT_DEFAULTPARAMETER,OnSelectDefaultParameter)
		COMMAND_ID_HANDLER(IDC_CHECK_DELETE_AFTER_COMPRESS,OnCheckDelete)
		MSG_WM_DESTROY(OnDestroy)
	END_MSG_MAP()

	LRESULT OnInitDialog(HWND hWnd, LPARAM lParam);
	LRESULT OnApply();
	LRESULT OnRadioCompressTo(WORD,WORD,HWND,BOOL&);
	LRESULT OnBrowseFolder(WORD,WORD,HWND,BOOL&);
	LRESULT OnCheckLimitCompressFileCount(WORD,WORD,HWND,BOOL&);
	LRESULT OnCheckUseDefaultParameter(WORD,WORD,HWND,BOOL&);
	LRESULT OnSelectDefaultParameter(WORD,WORD,HWND,BOOL&);
	LRESULT OnCheckDelete(WORD,WORD,HWND,BOOL&);

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

