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
#include "../../FileListWindow/FileListModel.h"
#include "../ConfigFileListWindow.h"

//==================================
// ファイル一覧ウィンドウの設定項目
//==================================
class CConfigDlgFileListWindow : public CDialogImpl<CConfigDlgFileListWindow>,public CWinDataExchange<CConfigDlgFileListWindow>,public CMessageFilter,public IConfigDlgBase
{
protected:
	CConfigFileListWindow	m_Config;

	virtual BOOL PreTranslateMessage(MSG* pMsg){
		return IsDialogMessage(pMsg);
	}

	void OnClearTemporary(UINT,int,HWND);	//残ってしまったテンポラリディレクトリを削除
	void OnResetExt(UINT,int,HWND);			//禁止・許可拡張子のリセット

	CButton Radio_FileListMode[FILELISTMODE_ITEM_COUNT];

	CEdit Edit_Path,Edit_Param,Edit_Dir,Edit_Caption;
	CListViewCtrl List_Command;
	void OnBrowsePath(UINT, int, HWND);
	void OnBrowseDir(UINT, int, HWND);
	void OnBrowseCustomToolbarImage(UINT, int, HWND);

	std::vector<CMenuCommandItem> m_MenuCommandArray;
	CMenuCommandItem *m_lpMenuCommandItem;	//編集中のコマンドアイテム

	LRESULT OnGetDispInfo(LPNMHDR pnmh);
	LRESULT OnSelect(LPNMHDR pnmh);
	LRESULT OnUserAppMoveUp(WORD,WORD,HWND,BOOL&);
	LRESULT OnUserAppMoveDown(WORD,WORD,HWND,BOOL&);
	LRESULT OnUserAppNew(WORD,WORD,HWND,BOOL&);
	LRESULT OnUserAppDelete(WORD,WORD,HWND,BOOL&);
	LRESULT OnCheckChanged(WORD,WORD,HWND,BOOL&);
public:
	enum { IDD = IDD_PROPPAGE_CONFIG_FILELISTWINDOW };

	// DDXマップ
	BEGIN_DDX_MAP(CConfigDlgFileListWindow)
		DDX_CHECK(IDC_CHECK_STORE_FILELISTWINDOW_SETTING, m_Config.StoreSetting)
		DDX_CHECK(IDC_CHECK_STORE_FILELISTWINDOW_POSITION, m_Config.StoreWindowPosition)
		DDX_CHECK(IDC_CHECK_IGNORE_MEANINGLESS_PATH, m_Config.IgnoreMeaninglessPath)
		DDX_CHECK(IDC_CHECK_EXPAND_TREE, m_Config.ExpandTree)
		DDX_CHECK(IDC_CHECK_DISPLAY_FILESIZE_IN_BYTE,m_Config.DisplayFileSizeInByte)
		DDX_CHECK(IDC_CHECK_EXIT_WITH_ESCAPE,m_Config.ExitWithEscape)
		DDX_CHECK(IDC_CHECK_DISABLE_TAB,m_Config.DisableTab)
		DDX_CHECK(IDC_CHECK_KEEP_SINGLE_INSTANCE,m_Config.KeepSingleInstance)
		DDX_CHECK(IDC_CHECK_DISPLAY_PATH_ONLY,m_Config.DisplayPathOnly)
		DDX_TEXT(IDC_EDIT_OPENASSOC_ACCEPT, m_Config.OpenAssoc.Accept)
		DDX_TEXT(IDC_EDIT_OPENASSOC_DENY, m_Config.OpenAssoc.Deny)
		DDX_TEXT(IDC_EDIT_CUSTOMTOOLBAR_IMAGE, m_Config.strCustomToolbarImage)
		DDX_CHECK(IDC_CHECK_SHOW_TOOLBAR,m_Config.ShowToolbar)
		DDX_CHECK(IDC_CHECK_DENY_PATHEXT,m_Config.DenyPathExt)
	END_DDX_MAP()

	// メッセージマップ
	BEGIN_MSG_MAP_EX(CConfigDlgFileListWindow)
		NOTIFY_CODE_HANDLER_EX(LVN_GETDISPINFO, OnGetDispInfo)	//仮想リストビュー
		NOTIFY_CODE_HANDLER_EX(LVN_ITEMCHANGED, OnSelect)
		COMMAND_ID_HANDLER_EX(IDC_BUTTON_CLEAR_TEMPORARY,OnClearTemporary)
		COMMAND_ID_HANDLER_EX(IDC_BUTTON_RESET_OPENASSOC_ACCEPT,OnResetExt)
		COMMAND_ID_HANDLER_EX(IDC_BUTTON_RESET_OPENASSOC_DENY,OnResetExt)

		COMMAND_ID_HANDLER(IDC_BUTTON_FILELIST_USERAPP_NEW,OnUserAppNew)
		COMMAND_ID_HANDLER(IDC_BUTTON_FILELIST_USERAPP_DELETE,OnUserAppDelete)
		COMMAND_ID_HANDLER(IDC_BUTTON_FILELIST_USERAPP_MOVEUP,OnUserAppMoveUp)
		COMMAND_ID_HANDLER(IDC_BUTTON_FILELIST_USERAPP_MOVEDOWN,OnUserAppMoveDown)

		COMMAND_ID_HANDLER_EX(IDC_BUTTON_FILELIST_USERAPP_BROWSE, OnBrowsePath)
		COMMAND_ID_HANDLER_EX(IDC_BUTTON_FILELIST_USERAPP_BROWSEDIR, OnBrowseDir)
		COMMAND_ID_HANDLER_EX(IDC_BUTTON_BROWSE_CUSTOMTOOLBAR_IMAGE, OnBrowseCustomToolbarImage)

		COMMAND_ID_HANDLER(IDC_CHECK_DISABLE_TAB, OnCheckChanged)
		MSG_WM_INITDIALOG(OnInitDialog)
		MSG_WM_DESTROY(OnDestroy)
	END_MSG_MAP()

	LRESULT OnInitDialog(HWND hWnd, LPARAM lParam);
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

