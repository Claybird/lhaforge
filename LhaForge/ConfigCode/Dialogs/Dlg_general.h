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
#include "ArchiverCode/archive.h"
#include "../ConfigGeneral.h"

//====================================
// 一般設定項目
//====================================
class CConfigDlgGeneral : public LFConfigDialogBase<CConfigDlgGeneral>
{
protected:
	CConfigGeneral	m_Config;
	CButton Radio_LostDir[LOSTDIR_ITEM_COUNT];
	CButton Radio_LogView[LOGVIEW_ITEM_COUNT];

	CButton Check_UseFiler;
	CEdit Edit_FilerPath;
	CEdit Edit_FilerParam;
	CEdit Edit_TempPath;
	CButton Button_BrowseFiler;

	virtual BOOL PreTranslateMessage(MSG* pMsg){
		return IsDialogMessage(pMsg);
	}

	LRESULT OnInitDialog(HWND hWnd, LPARAM lParam);
	LRESULT OnCheckFiler(WORD,WORD,HWND,BOOL&);
	LRESULT OnBrowseFiler(WORD,WORD,HWND,BOOL&);
	LRESULT OnBrowseTempPath(WORD,WORD,HWND,BOOL&);
public:
	enum { IDD = IDD_PROPPAGE_CONFIG_GENERAL };
	// DDXマップ
	BEGIN_DDX_MAP(CConfigDlgGeneral)
		DDX_CHECK(IDC_CHECK_WARN_NETWORK,m_Config.WarnNetwork)
		DDX_CHECK(IDC_CHECK_WARN_REMOVABLE,m_Config.WarnRemovable)
		DDX_CHECK(IDC_CHECK_NOTIFY_SHELL,m_Config.NotifyShellAfterProcess)
		DDX_RADIO(IDC_RADIO_PRIORITY_DEFAULT,m_Config.ProcessPriority)
		DDX_TEXT(IDC_EDIT_TEMP_PATH, m_Config.TempPath)
	END_DDX_MAP()

	// メッセージマップ
	BEGIN_MSG_MAP_EX(CConfigDlgGeneral)
		MSG_WM_INITDIALOG(OnInitDialog)
		COMMAND_ID_HANDLER(IDC_CHECK_USE_FILER,OnCheckFiler)
		COMMAND_ID_HANDLER(IDC_BUTTON_BROWSE_FILER,OnBrowseFiler)
		COMMAND_ID_HANDLER(IDC_BUTTON_BROWSE_TEMP,OnBrowseTempPath)
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

