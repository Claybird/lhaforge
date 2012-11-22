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

#include "stdafx.h"
#include "Dlg_filelistwindow.h"
#include "../../Dialogs/LFFolderDialog.h"
#include "../../Utilities/TemporaryDirMgr.h"
#include "../../Utilities/StringUtil.h"

//================================
// ファイル一覧ウィンドウ設定画面
//================================
LRESULT CConfigDlgFileListWindow::OnInitDialog(HWND hWnd, LPARAM lParam)
{
	// メッセージループにメッセージフィルタとアイドルハンドラを追加
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	pLoop->AddMessageFilter(this);

	//DDX情報アップデート
	DoDataExchange(FALSE);

	//ファイル一覧モード
	Radio_FileListMode[FILELIST_TREE]=GetDlgItem(IDC_RADIO_FILELIST_TREE);
	Radio_FileListMode[FILELIST_FLAT]=GetDlgItem(IDC_RADIO_FILELIST_FLAT);
	Radio_FileListMode[FILELIST_FLAT_FILESONLY]=GetDlgItem(IDC_RADIO_FILELIST_FLAT_FILESONLY);

	Radio_FileListMode[m_Config.FileListMode].SetCheck(true);

	//「プログラムで開く」コマンド関連
	m_MenuCommandArray=m_Config.MenuCommandArray;

	Edit_Path	=GetDlgItem(IDC_EDIT_FILELIST_USERAPP_PATH);
	Edit_Path.SetLimitText(_MAX_PATH);
	Edit_Param	=GetDlgItem(IDC_EDIT_FILELIST_USERAPP_PARAM);
	Edit_Param.SetLimitText(_MAX_PATH*3);
	Edit_Dir	=GetDlgItem(IDC_EDIT_FILELIST_USERAPP_DIR);
	Edit_Dir.SetLimitText(_MAX_PATH);
	Edit_Caption=GetDlgItem(IDC_EDIT_FILELIST_USERAPP_CAPTION);
	Edit_Caption.SetLimitText(_MAX_PATH);
	List_Command=GetDlgItem(IDC_LIST_FILELIST_USERAPP);

	CRect rect;
	List_Command.GetClientRect(rect);
	List_Command.InsertColumn(0, _T(""),LVCFMT_LEFT,rect.Width());
	List_Command.SetExtendedListViewStyle(List_Command.GetExtendedListViewStyle()|LVS_EX_FULLROWSELECT);
	List_Command.SetItemCount(m_MenuCommandArray.size());

	m_lpMenuCommandItem=NULL;

	//状態更新
	BOOL tmp;
	OnCheckChanged(0,0,NULL,tmp);
	return TRUE;
}

LRESULT CConfigDlgFileListWindow::OnApply()
{
//===============================
// 設定をConfigManagerに書き戻す
//===============================
	//---------------
	// DDXデータ更新
	//---------------
	if(!DoDataExchange(TRUE)){
		return FALSE;
	}
	for(int i=0;i<COUNTOF(Radio_FileListMode);i++){
		if(Radio_FileListMode[i].GetCheck()){
			m_Config.FileListMode=(FILELISTMODE)i;
			break;
		}
	}
	//「プログラムで開く」コマンドの更新
	if(m_lpMenuCommandItem){
		Edit_Path.GetWindowText(m_lpMenuCommandItem->Path);
		Edit_Param.GetWindowText(m_lpMenuCommandItem->Param);
		Edit_Dir.GetWindowText(m_lpMenuCommandItem->Dir);
		Edit_Caption.GetWindowText(m_lpMenuCommandItem->Caption);
	}
	m_Config.MenuCommandArray=m_MenuCommandArray;
	return TRUE;
}

void CConfigDlgFileListWindow::OnClearTemporary(UINT,int,HWND)
{
	//残ってしまったテンポラリディレクトリを削除
	if(!CTemporaryDirectoryManager::DeleteAllTemporaryDir(_T("lhaf")))MessageBeep(MB_ICONHAND);
}

void CConfigDlgFileListWindow::OnResetExt(UINT,int nID,HWND)
{
	switch(nID){
	case IDC_BUTTON_RESET_OPENASSOC_ACCEPT:
		::SetWindowText(GetDlgItem(IDC_EDIT_OPENASSOC_ACCEPT),CString(MAKEINTRESOURCE(IDS_FILELIST_OPENASSOC_DEFAULT_ACCEPT)));
		break;
	case IDC_BUTTON_RESET_OPENASSOC_DENY:
		::SetWindowText(GetDlgItem(IDC_EDIT_OPENASSOC_DENY),CString(MAKEINTRESOURCE(IDS_FILELIST_OPENASSOC_DEFAULT_DENY)));
		break;
	}
}

//プログラムの場所を参照
void CConfigDlgFileListWindow::OnBrowsePath(UINT, int, HWND)
{
	TCHAR filter[_MAX_PATH+2]={0};
	UtilMakeFilterString(
		_T("Program(*.exe;*.com;*.bat;*.cmd)|")
		_T("*.exe;*.com;*.bat;*.cmd|")
		_T("All Files(*.*)|")
		_T("*.*"),
		filter,_MAX_PATH+2);

	TCHAR szPath[_MAX_PATH+1];
	Edit_Path.GetWindowText(szPath,_MAX_PATH);
	CFileDialog dlg(TRUE, NULL, szPath, OFN_HIDEREADONLY|OFN_NOCHANGEDIR,filter);
	if(IDCANCEL==dlg.DoModal()){	//キャンセル
		return;
	}
	Edit_Path.SetWindowText(dlg.m_szFileName);
}

//フォルダを参照
void CConfigDlgFileListWindow::OnBrowseDir(UINT, int, HWND)
{
	TCHAR szPath[_MAX_PATH+1];
	Edit_Dir.GetWindowText(szPath,_MAX_PATH);
	CLFFolderDialog dlg(NULL,NULL,BIF_RETURNONLYFSDIRS|BIF_NEWDIALOGSTYLE);
	dlg.SetInitialFolder(szPath);

	if(IDCANCEL==dlg.DoModal()){	//キャンセル
		return;
	}
	Edit_Dir.SetWindowText(dlg.GetFolderPath());
}

//カスタムツールバー画像を参照
void CConfigDlgFileListWindow::OnBrowseCustomToolbarImage(UINT, int, HWND)
{
	TCHAR filter[_MAX_PATH+2]={0};
	UtilMakeFilterString(
		_T("Bitmap(*.bmp)|")
		_T("*.bmp|")
		_T("All Files(*.*)|")
		_T("*.*"),
		filter,_MAX_PATH+2);

	TCHAR szPath[_MAX_PATH+1];
	::GetWindowText(GetDlgItem(IDC_EDIT_CUSTOMTOOLBAR_IMAGE),szPath,_MAX_PATH);
	CFileDialog dlg(TRUE, NULL, szPath, OFN_HIDEREADONLY|OFN_NOCHANGEDIR,filter);
	if(IDCANCEL==dlg.DoModal()){	//キャンセル
		return;
	}
	::SetWindowText(GetDlgItem(IDC_EDIT_CUSTOMTOOLBAR_IMAGE),dlg.m_szFileName);
}

//仮想リストビューのアイテム取得
LRESULT CConfigDlgFileListWindow::OnGetDispInfo(LPNMHDR pnmh)
{
	LV_DISPINFO* pstLVDInfo=(LV_DISPINFO*)pnmh;

	if(pstLVDInfo->item.iItem<0||pstLVDInfo->item.iItem>=(signed)m_MenuCommandArray.size())return 1;
	CMenuCommandItem &mci=m_MenuCommandArray[pstLVDInfo->item.iItem];

	if(pstLVDInfo->item.mask & LVIF_TEXT){
		_tcsncpy_s(pstLVDInfo->item.pszText,pstLVDInfo->item.cchTextMax,mci.Caption,pstLVDInfo->item.cchTextMax);
	}
	return 0;
}

//アイテム選択変更
LRESULT CConfigDlgFileListWindow::OnSelect(LPNMHDR pnmh)
{
	if(m_lpMenuCommandItem){
		Edit_Path.GetWindowText(m_lpMenuCommandItem->Path);
		Edit_Param.GetWindowText(m_lpMenuCommandItem->Param);
		Edit_Dir.GetWindowText(m_lpMenuCommandItem->Dir);
		Edit_Caption.GetWindowText(m_lpMenuCommandItem->Caption);
	}

	int iItem=List_Command.GetNextItem(-1,LVNI_ALL|LVNI_SELECTED);
	if(-1!=iItem&&iItem<(signed)m_MenuCommandArray.size()){
		m_lpMenuCommandItem=&m_MenuCommandArray[iItem];
		Edit_Path.SetWindowText(m_lpMenuCommandItem->Path);
		Edit_Param.SetWindowText(m_lpMenuCommandItem->Param);
		Edit_Dir.SetWindowText(m_lpMenuCommandItem->Dir);
		Edit_Caption.SetWindowText(m_lpMenuCommandItem->Caption);
	}
	return 0;
}

//項目を上へ移動
LRESULT CConfigDlgFileListWindow::OnUserAppMoveUp(WORD,WORD,HWND,BOOL&)
{
	int iItem=List_Command.GetNextItem(-1,LVNI_ALL|LVNI_SELECTED);
	if(-1==iItem||iItem>=(signed)m_MenuCommandArray.size())return FALSE;
	if(0==iItem){
		MessageBeep(MB_ICONASTERISK);
		return TRUE;
	}
	CMenuCommandItem mci=m_MenuCommandArray[iItem];
	m_MenuCommandArray[iItem]=m_MenuCommandArray[iItem-1];
	m_MenuCommandArray[iItem-1]=mci;
	m_lpMenuCommandItem=&m_MenuCommandArray[iItem-1];
	Edit_Path.SetWindowText(m_lpMenuCommandItem->Path);
	Edit_Param.SetWindowText(m_lpMenuCommandItem->Param);
	Edit_Dir.SetWindowText(m_lpMenuCommandItem->Dir);
	Edit_Caption.SetWindowText(m_lpMenuCommandItem->Caption);

	List_Command.EnsureVisible(iItem-1,FALSE);
	List_Command.SetItemState(iItem-1,LVIS_SELECTED, LVIS_SELECTED);
	return TRUE;
}

//項目を下へ移動
LRESULT CConfigDlgFileListWindow::OnUserAppMoveDown(WORD,WORD,HWND,BOOL&)
{
	int iItem=List_Command.GetNextItem(-1,LVNI_ALL|LVNI_SELECTED);
	if(-1==iItem||iItem>=(signed)m_MenuCommandArray.size())return FALSE;
	if((signed)m_MenuCommandArray.size()-1==iItem){
		MessageBeep(MB_ICONASTERISK);
		return TRUE;
	}
	CMenuCommandItem mci=m_MenuCommandArray[iItem];
	m_MenuCommandArray[iItem]=m_MenuCommandArray[iItem+1];
	m_MenuCommandArray[iItem+1]=mci;
	m_lpMenuCommandItem=&m_MenuCommandArray[iItem+1];
	Edit_Path.SetWindowText(m_lpMenuCommandItem->Path);
	Edit_Param.SetWindowText(m_lpMenuCommandItem->Param);
	Edit_Dir.SetWindowText(m_lpMenuCommandItem->Dir);
	Edit_Caption.SetWindowText(m_lpMenuCommandItem->Caption);

	List_Command.EnsureVisible(iItem+1,FALSE);
	List_Command.SetItemState(iItem+1,LVIS_SELECTED, LVIS_SELECTED);
	return TRUE;
}

//項目の新規作成
LRESULT CConfigDlgFileListWindow::OnUserAppNew(WORD,WORD,HWND,BOOL&)
{
	//旧項目の保存
	if(m_lpMenuCommandItem){
		Edit_Path.GetWindowText(m_lpMenuCommandItem->Path);
		Edit_Param.GetWindowText(m_lpMenuCommandItem->Param);
		Edit_Dir.GetWindowText(m_lpMenuCommandItem->Dir);
		Edit_Caption.GetWindowText(m_lpMenuCommandItem->Caption);
	}

	//新項目の設定
	CMenuCommandItem mci;
	mci.Caption=_T("UserApp");
	mci.Param=_T("%S");
	m_MenuCommandArray.push_back(mci);
	int iItem=m_MenuCommandArray.size()-1;
	m_lpMenuCommandItem=&m_MenuCommandArray[iItem];

	Edit_Path.SetWindowText(m_lpMenuCommandItem->Path);
	Edit_Param.SetWindowText(m_lpMenuCommandItem->Param);
	Edit_Dir.SetWindowText(m_lpMenuCommandItem->Dir);
	Edit_Caption.SetWindowText(m_lpMenuCommandItem->Caption);
	List_Command.SetItemCount(m_MenuCommandArray.size());
	List_Command.EnsureVisible(iItem,FALSE);
	List_Command.SetItemState(iItem,LVIS_SELECTED, LVIS_SELECTED);
	return TRUE;
}

//項目を削除
LRESULT CConfigDlgFileListWindow::OnUserAppDelete(WORD,WORD,HWND,BOOL&)
{
	int iItem=List_Command.GetNextItem(-1,LVNI_ALL|LVNI_SELECTED);
	if(-1==iItem||iItem>=(signed)m_MenuCommandArray.size())return FALSE;
	std::vector<CMenuCommandItem>::iterator ite=m_MenuCommandArray.begin();

	//削除対象アイテムの検索
	ite+=iItem;
	m_MenuCommandArray.erase(ite);

	List_Command.SetItemCount(m_MenuCommandArray.size());

	if(m_MenuCommandArray.empty()){
		m_lpMenuCommandItem=NULL;
		return TRUE;
	}
	if(iItem!=0)iItem--;
	m_lpMenuCommandItem=&m_MenuCommandArray[iItem];
	Edit_Path.SetWindowText(m_lpMenuCommandItem->Path);
	Edit_Param.SetWindowText(m_lpMenuCommandItem->Param);
	Edit_Dir.SetWindowText(m_lpMenuCommandItem->Dir);
	Edit_Caption.SetWindowText(m_lpMenuCommandItem->Caption);

	List_Command.EnsureVisible(iItem,FALSE);
	List_Command.SetItemState(iItem,LVIS_SELECTED, LVIS_SELECTED);
	return TRUE;
}

LRESULT CConfigDlgFileListWindow::OnCheckChanged(WORD,WORD,HWND,BOOL&)
{
	CButton btn=GetDlgItem(IDC_CHECK_DISABLE_TAB);
	::EnableWindow(GetDlgItem(IDC_CHECK_KEEP_SINGLE_INSTANCE),!btn.GetCheck());
	return 0;
}
