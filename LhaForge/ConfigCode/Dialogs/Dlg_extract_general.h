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
#include "../ConfigExtract.h"
#include "../../ArchiverCode/arc_interface.h"

//====================================
// 解凍一般設定
//====================================
class CConfigDlgExtractGeneral : public LFConfigDialogBase<CConfigDlgExtractGeneral>
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
	CString DenyExt;
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
		DDX_CHECK(IDC_CHECK_MINIMUM_PASSWORD_REQUEST,m_Config.MinimumPasswordRequest)
		DDX_TEXT(IDC_EDIT_EXTRACT_DENY_EXT,DenyExt)
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
		DenyExt = m_Config.DenyExt.c_str();
	}
	void StoreConfig(CConfigManager& Config){
		m_Config.store(Config);
		m_Config.DenyExt = (const wchar_t*)DenyExt;
	}
};

