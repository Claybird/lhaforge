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
#include "../../Utilities/FileOperation.h"
//#include "../Association.h"
#include "../AssocSettings.h"

//====================================
// 関連付け情報
//====================================

enum ASSOC_TYPE{
	ASSOC_LZH,
	ASSOC_LZS,
	ASSOC_LHA,
	ASSOC_ZIP,
	ASSOC_JAR,
	ASSOC_CAB,
	ASSOC_7Z,
	ASSOC_ARJ,
	ASSOC_RAR,
	ASSOC_JAK,
	ASSOC_GCA,
	ASSOC_YZ1,
	ASSOC_IMP,
	ASSOC_ACE,
	ASSOC_HKI,
	ASSOC_BZA,
	ASSOC_GZA,
	ASSOC_ISH,
	ASSOC_UUE,
	ASSOC_BEL,
	ASSOC_TAR,
	ASSOC_GZ,
	ASSOC_TGZ,
	ASSOC_BZ2,
	ASSOC_TBZ,
	ASSOC_XZ,
	ASSOC_TAR_XZ,
	ASSOC_LZMA,
	ASSOC_TAR_LZMA,
	ASSOC_Z,
	ASSOC_TAZ,
	ASSOC_CPIO,
	ASSOC_A,
	ASSOC_LIB,
	ASSOC_RPM,
	ASSOC_DEB,
	ASSOC_ISO,

	ENUM_COUNT_AND_LASTITEM(ASSOC_TYPE),
};

const int NO_DEFAULT_ASSOCS[]={
	ASSOC_JAR,
	ASSOC_A,
	ASSOC_LIB,
	ASSOC_ISO,
};

class CConfigDialog;
class CConfigManager;
class CConfigDlgAssociation : public CDialogImpl<CConfigDlgAssociation>,public CMessageFilter,public IConfigDlgBase
{
protected:
	ASSOC_SETTINGS AssocSettings[ASSOC_TYPE_ITEM_COUNT];
	virtual BOOL PreTranslateMessage(MSG* pMsg){
		return IsDialogMessage(pMsg);
	}
	CString m_strAssocDesired;	//標準的な関連付けパス;これと異なっていた場合には関連付け情報の変更フラグをたてておく

	CIcon Icon_SystemDefault;	//関連付けがない時のアイコン

	CConfigDialog	&mr_ConfigDlg;
public:
	enum { IDD = IDD_PROPPAGE_CONFIG_ASSOCIATION };

	// メッセージマップ
	BEGIN_MSG_MAP_EX(CConfigDlgAssociation)
		MSG_WM_INITDIALOG(OnInitDialog)
		COMMAND_RANGE_HANDLER(IDC_CHECK_ASSOCIATION_LZH,IDC_CHECK_ASSOCIATION_ISO, OnCheckAssoc)
		COMMAND_RANGE_HANDLER(IDC_BUTTON_CHANGE_ICON_LZH,IDC_BUTTON_CHANGE_ICON_ISO, OnChangeIcon)
		COMMAND_RANGE_HANDLER(IDC_BUTTON_ASSOC_CHECK_ALL,IDC_BUTTON_ASSOC_SET_EXTERNAL_ICON_SINGLE, OnSetAssoc)
		MSG_WM_DESTROY(OnDestroy)
	END_MSG_MAP()

	LRESULT OnInitDialog(HWND hWnd, LPARAM lParam);
	LRESULT OnCheckAssoc(WORD,WORD,HWND,BOOL&);
	LRESULT OnChangeIcon(WORD,WORD,HWND,BOOL&);
	LRESULT OnSetAssoc(WORD,WORD,HWND,BOOL&);
	LRESULT OnApply();

	CConfigDlgAssociation(CConfigDialog &dlg):
		mr_ConfigDlg(dlg)
	{
		TRACE(_T("CConfigDlgAssociation()\n"));
	}

	LRESULT OnDestroy(){
		CMessageLoop* pLoop = _Module.GetMessageLoop();
		pLoop->RemoveMessageFilter(this);

		return TRUE;
	}
	void LoadConfig(CConfigManager& Config){}
	void StoreConfig(CConfigManager& Config){}
};


class CIconSelectDialog : public CDialogImpl<CIconSelectDialog>,public CWinDataExchange<CIconSelectDialog>,public CDialogResize<CIconSelectDialog>
{
protected:
	ASSOCINFO *AssocInfo;
	CIconSelectDialog();
	CImageList	IconList;
	CListViewCtrl ListView;
	CString IconPath;
public:
	enum {IDD = IDD_DIALOG_ICON_SELECT};

	// ダイアログリサイズマップ
	BEGIN_DLGRESIZE_MAP(CIconSelectDialog)
		DLGRESIZE_CONTROL(IDC_BUTTON_BROWSE_DEFAULT_ICON,	DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_EDIT_ICON_PATH,				DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_BUTTON_BROWSE_ICON,			DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_LIST_ICON,					DLSZ_SIZE_X | DLSZ_SIZE_Y)
		DLGRESIZE_CONTROL(IDOK,								DLSZ_MOVE_X | DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDCANCEL,							DLSZ_MOVE_X | DLSZ_MOVE_Y)
	END_DLGRESIZE_MAP()

	// DDXマップ
	BEGIN_DDX_MAP(CIconSelectDialog)
		DDX_TEXT(IDC_EDIT_ICON_PATH,IconPath)
	END_DDX_MAP()

	// メッセージマップ
	BEGIN_MSG_MAP_EX(CIconSelectDialog)
		MSG_WM_INITDIALOG(OnInitDialog)
		COMMAND_ID_HANDLER_EX(IDC_BUTTON_BROWSE_ICON, OnBrowse)
		COMMAND_ID_HANDLER_EX(IDC_BUTTON_BROWSE_DEFAULT_ICON, OnBrowseDefault)
		COMMAND_ID_HANDLER_EX(IDOK, OnOK)
		COMMAND_ID_HANDLER_EX(IDCANCEL, OnCancel)
		CHAIN_MSG_MAP(CDialogResize<CIconSelectDialog>)    // CDialogResizeクラスへのチェーン
	END_MSG_MAP()

	CIconSelectDialog(ASSOCINFO&);
	bool UpdateIcon();

	LRESULT OnInitDialog(HWND hWnd, LPARAM lParam);
	void OnBrowse(UINT uNotifyCode, int nID, HWND hWndCtl);
	void OnBrowseDefault(UINT uNotifyCode, int nID, HWND hWndCtl);
	void OnOK(UINT uNotifyCode, int nID, HWND hWndCtl);
	void OnCancel(UINT uNotifyCode, int nID, HWND hWndCtl){
		EndDialog(nID);
	}
};

