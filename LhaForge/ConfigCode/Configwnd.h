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
#include "../Utilities/Utility.h"
#include "../resource.h"
#include "ConfigManager.h"
#include "./Dialogs/Dlg_Base.h"
#include "./Dialogs/Dlg_version.h"
#include "./Dialogs/Dlg_general.h"
#include "./Dialogs/Dlg_compress_general.h"
#include "./Dialogs/Dlg_extract_general.h"
#include "./Dialogs/Dlg_lzh.h"
#include "./Dialogs/Dlg_zip.h"
#include "./Dialogs/Dlg_cab.h"
#include "./Dialogs/Dlg_7z.h"
#include "./Dialogs/Dlg_tar.h"
#include "./Dialogs/Dlg_jack.h"
#include "./Dialogs/Dlg_hki.h"
#include "./Dialogs/Dlg_bga.h"
#include "./Dialogs/Dlg_aish.h"
#include "./Dialogs/Dlg_dll_update.h"
#include "./Dialogs/Dlg_assoc.h"
#include "./Dialogs/Dlg_detail.h"
#include "./Dialogs/Dlg_shortcut.h"
#include "./Dialogs/Dlg_filelistwindow.h"
#include "./Dialogs/Dlg_openaction.h"
#include "./Dialogs/Dlg_shellext.h"
#include "./Dialogs/Dlg_DLL.h"

//====================================
// 設定項目をまとめる
//====================================
class CConfigDialog : public CDialogImpl<CConfigDialog>,public CDialogResize<CConfigDialog>
{
protected:
	CConfigDlgGeneral				PageGeneral;
	CConfigDlgShellExt				PageShellExt;
	CConfigDlgVersion				PageVersion;
	CConfigDlgCompressGeneral		PageCompressGeneral;
	CConfigDlgExtractGeneral		PageExtractGeneral;
	CConfigDlgLZH					PageLZH;
	CConfigDlgZIP					PageZIP;
	CConfigDlgCAB					PageCAB;
	CConfigDlg7Z					Page7Z;
	CConfigDlgTAR					PageTAR;
	CConfigDlgJACK					PageJACK;
	CConfigDlgHKI					PageHKI;
	CConfigDlgBGA					PageBGA;
	CConfigDlgAISH					PageAISH;
	CConfigDlgDLLUpdate				PageDLLUpdate;
	CConfigDlgAssociation			PageAssociation;
	CConfigDlgOpenAction			PageOpenAction;
	CConfigDlgDetail				PageDetail;
	CConfigDlgShortcut				PageShortcut;
	CConfigDlgFileListWindow		PageFileListWindow;
	CConfigDlgDLL					PageDLL;

	HWND						hActiveDialogWnd;	//アクティブなダイアログ
	CTreeViewCtrl				SelectTreeView;

	//ダイアログを貼り付けるためのスクロールコンテナ
	CScrollContainer			ScrollWindow;

	CConfigManager				&mr_Config;

	CString						m_strAssistINI;	//アシスタントを呼び出すために使うINI名
	UINT						m_nAssistRequireCount;//アシスタントを呼ぶ必要があれば非0;参照カウンタ方式


	std::set<IConfigDlgBase*> m_ConfigDlgList;
public:
	enum{IDD = IDD_DIALOG_CONFIG};

	// メッセージマップ
	BEGIN_MSG_MAP_EX(CConfigDialog)
		MSG_WM_INITDIALOG(OnInitDialog)
		NOTIFY_CODE_HANDLER_EX(TVN_SELCHANGED, OnTreeSelect)
		COMMAND_ID_HANDLER_EX(IDOK, OnOK)
		COMMAND_ID_HANDLER_EX(IDCANCEL, OnCancel)
		MSG_WM_SIZE(OnSize)
		CHAIN_MSG_MAP(CDialogResize<CConfigDialog>)
		MESSAGE_HANDLER(WM_USER_WM_SIZE,OnUserSize)
	END_MSG_MAP()

	// ダイアログリサイズマップ
	BEGIN_DLGRESIZE_MAP(CConfigDialog)
		DLGRESIZE_CONTROL(IDC_TREE_SELECT_PROPPAGE,			DLSZ_SIZE_Y)
		DLGRESIZE_CONTROL(IDC_STATIC_FRAME,					DLSZ_SIZE_X | DLSZ_SIZE_Y)
		DLGRESIZE_CONTROL(IDOK,								DLSZ_MOVE_X | DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDCANCEL,							DLSZ_MOVE_X | DLSZ_MOVE_Y)
	END_DLGRESIZE_MAP()

	CConfigDialog(CConfigManager &cfg);
	virtual ~CConfigDialog();

	LRESULT OnTreeSelect(LPNMHDR pnmh);
	LRESULT OnInitDialog(HWND hWnd, LPARAM lParam);
	void OnSize(UINT, CSize&);
	LRESULT OnUserSize(UINT, WPARAM wParam, LPARAM, BOOL& bHandled);

	void OnOK(UINT uNotifyCode, int nID, HWND hWndCtl);
	void OnCancel(UINT uNotifyCode, int nID, HWND hWndCtl);

	//LFAssist.exeに渡すINIの名前(UAC回避のため)
	LPCTSTR GetAssistantFile(){return m_strAssistINI;}
	//UAC回避のためLFAssist.exeを呼ぶことを要請(要請カウントを増やす)
	void RequireAssistant();
	//UAC回避のためLFAssist.exeを呼ぶことをやめる(要請カウントを減らす)
	void UnrequireAssistant();
};

