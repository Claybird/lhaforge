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
#include "../ConfigExtract.h"
#include "../../ArchiverCode/arc_interface.h"

//====================================
// 解凍一般設定
//====================================
class CConfigDlgExtractGeneral : public CDialogImpl<CConfigDlgExtractGeneral>,public CMessageFilter,public CWinDataExchange<CConfigDlgExtractGeneral>,public IConfigDlgBase
{
protected:
	CConfigExtract m_Config;
	CButton Radio_ExtractTo[OUTPUT_TO_ITEM_COUNT];
	CEdit Edit_ExtractOutputDirPath;
	CButton Button_ExtractToFolder;
	CButton Radio_CreateDir[CREATE_OUTPUT_DIR_ITEM_COUNT];
	CUpDownCtrl UpDown_MaxExtractFileCount;
	CButton Check_LimitExtractFileCount;
	CButton Check_DeleteFileAfterExtract;
	virtual BOOL PreTranslateMessage(MSG* pMsg){
		return IsDialogMessage(pMsg);
	}

public:
	enum { IDD = IDD_PROPPAGE_CONFIG_EXTRACT_GENERAL };

	// DDXマップ
	BEGIN_DDX_MAP(CConfigGeneral)
		DDX_CHECK(IDC_CHECK_REMOVE_SYMBOL_AND_NUMBER, m_Config.RemoveSymbolAndNumber)
		DDX_CHECK(IDC_CHECK_OPEN_FOLDER_AFTER_EXTRACT, m_Config.OpenDir)
		DDX_CHECK(IDC_CHECK_FORCE_OVERWRITE, m_Config.ForceOverwrite)
		DDX_CHECK(IDC_CHECK_CREATE_NO_FOLDER_IF_SINGLE_FILE_ONLY,m_Config.CreateNoFolderIfSingleFileOnly)
		DDX_CHECK(IDC_CHECK_LIMIT_EXTRACT_FILECOUNT,m_Config.LimitExtractFileCount)
		DDX_CHECK(IDC_CHECK_DELETE_ARCHIVE_AFTER_EXTRACT,m_Config.DeleteArchiveAfterExtract)
		DDX_CHECK(IDC_CHECK_MOVETO_RECYCLE_BIN,m_Config.MoveToRecycleBin)
		DDX_CHECK(IDC_CHECK_DELETE_NOCONFIRM,m_Config.DeleteNoConfirm)
		DDX_CHECK(IDC_CHECK_FORCE_DELETE,m_Config.ForceDelete)
		DDX_CHECK(IDC_CHECK_DELETE_MULTIVOLUME,m_Config.DeleteMultiVolume)
		DDX_CHECK(IDC_CHECK_MINIMUM_PASSWORD_REQUEST,m_Config.MinimumPasswordRequest)
		DDX_TEXT(IDC_EDIT_EXTRACT_DENY_EXT,m_Config.DenyExt)
	END_DDX_MAP()

	// メッセージマップ
	BEGIN_MSG_MAP_EX(CConfigDlgExtractGeneral)
		MSG_WM_INITDIALOG(OnInitDialog)
		COMMAND_RANGE_HANDLER(IDC_RADIO_EXTRACT_TO_DESKTOP,IDC_RADIO_EXTRACT_TO_ALWAYS_ASK_WHERE, OnRadioExtractTo)
		COMMAND_RANGE_HANDLER(IDC_RADIO_CREATE_FOLDER,IDC_RADIO_CREATE_NO_FOLDER,OnRadioCreateDirectory)
		COMMAND_ID_HANDLER(IDC_BUTTON_EXTRACT_BROWSE_FOLDER,OnBrowseFolder)
		COMMAND_ID_HANDLER(IDC_CHECK_LIMIT_EXTRACT_FILECOUNT,OnCheckLimitExtractFileCount)
		COMMAND_ID_HANDLER(IDC_CHECK_DELETE_ARCHIVE_AFTER_EXTRACT,OnCheckDeleteArchive)
		MSG_WM_DESTROY(OnDestroy)
	END_MSG_MAP()

	LRESULT OnInitDialog(HWND hWnd, LPARAM lParam);
	LRESULT OnApply();
	LRESULT OnRadioExtractTo(WORD,WORD,HWND,BOOL&);
	LRESULT OnRadioCreateDirectory(WORD,WORD,HWND,BOOL&);
	LRESULT OnBrowseFolder(WORD,WORD,HWND,BOOL&);
	LRESULT OnCheckLimitExtractFileCount(WORD,WORD,HWND,BOOL&);
	LRESULT OnCheckDeleteArchive(WORD,WORD,HWND,BOOL&);

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

