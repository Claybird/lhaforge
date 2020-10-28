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

#include "stdafx.h"
#include "Dlg_filelistwindow.h"
#include "Utilities/StringUtil.h"
#include "Utilities/FileOperation.h"

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
	Edit_Param	=GetDlgItem(IDC_EDIT_FILELIST_USERAPP_PARAM);
	Edit_Dir	=GetDlgItem(IDC_EDIT_FILELIST_USERAPP_DIR);
	Edit_Caption=GetDlgItem(IDC_EDIT_FILELIST_USERAPP_CAPTION);
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
		getUserAppEdit();
	}
	m_Config.MenuCommandArray=m_MenuCommandArray;
	return TRUE;
}

void CConfigDlgFileListWindow::OnClearTemporary(UINT,int,HWND)
{
	//残ってしまったテンポラリディレクトリを削除
	if(!UtilDeleteDir(UtilGetTempPath().c_str(), false))MessageBeep(MB_ICONHAND);
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
	const COMDLG_FILTERSPEC filter[] = {
		{L"Program", L"*.exe;*.com;*.bat;*.cmd"},
		{L"All Files",L"*.*"},
	};

	CString path;
	Edit_Path.GetWindowTextW(path);
	LFShellFileOpenDialog dlg(path, FOS_FORCEFILESYSTEM | FOS_FILEMUSTEXIST | FOS_PATHMUSTEXIST, nullptr, filter, COUNTOF(filter));
	if(IDCANCEL==dlg.DoModal()){	//キャンセル
		return;
	}
	dlg.GetFilePath(path);
	Edit_Path.SetWindowTextW(path);
}

//フォルダを参照
void CConfigDlgFileListWindow::OnBrowseDir(UINT, int, HWND)
{
	CString path;
	Edit_Dir.GetWindowText(path);
	LFShellFileOpenDialog dlg(path, FOS_FORCEFILESYSTEM | FOS_FILEMUSTEXIST | FOS_PATHMUSTEXIST | FOS_PICKFOLDERS);

	if(IDCANCEL==dlg.DoModal()){	//キャンセル
		return;
	}
	dlg.GetFilePath(path);
	Edit_Dir.SetWindowTextW(path);
}

//カスタムツールバー画像を参照
void CConfigDlgFileListWindow::OnBrowseCustomToolbarImage(UINT, int, HWND)
{
	const COMDLG_FILTERSPEC filter[] = {
		{L"Bitmap(*.bmp)", L"*.bmp"},
		{L"All Files(*.*)", L"*.*"},
	};

	CString path;
	GetDlgItem(IDC_EDIT_CUSTOMTOOLBAR_IMAGE).GetWindowTextW(path);
	LFShellFileOpenDialog dlg(path, FOS_FORCEFILESYSTEM | FOS_FILEMUSTEXIST | FOS_PATHMUSTEXIST, nullptr, filter, COUNTOF(filter));
	if(IDCANCEL==dlg.DoModal()){	//キャンセル
		return;
	}
	dlg.GetFilePath(path);
	GetDlgItem(IDC_EDIT_CUSTOMTOOLBAR_IMAGE).SetWindowTextW(path);
}

//仮想リストビューのアイテム取得
LRESULT CConfigDlgFileListWindow::OnGetDispInfo(LPNMHDR pnmh)
{
	LV_DISPINFO* pstLVDInfo=(LV_DISPINFO*)pnmh;

	if(pstLVDInfo->item.iItem<0||pstLVDInfo->item.iItem>=(signed)m_MenuCommandArray.size())return 1;
	CMenuCommandItem &mci=m_MenuCommandArray[pstLVDInfo->item.iItem];

	if(pstLVDInfo->item.mask & LVIF_TEXT){
		_tcsncpy_s(pstLVDInfo->item.pszText,pstLVDInfo->item.cchTextMax,mci.Caption.c_str(),pstLVDInfo->item.cchTextMax);
	}
	return 0;
}

void CConfigDlgFileListWindow::setUserAppEdit()
{
	Edit_Path.SetWindowText(m_lpMenuCommandItem->Path.c_str());
	Edit_Param.SetWindowText(m_lpMenuCommandItem->Param.c_str());
	Edit_Dir.SetWindowText(m_lpMenuCommandItem->Dir.c_str());
	Edit_Caption.SetWindowText(m_lpMenuCommandItem->Caption.c_str());
}

void CConfigDlgFileListWindow::getUserAppEdit()
{
	CString buf;
	Edit_Path.GetWindowText(buf);
	m_lpMenuCommandItem->Path = buf;
	Edit_Param.GetWindowText(buf);
	m_lpMenuCommandItem->Param = buf;
	Edit_Dir.GetWindowText(buf);
	m_lpMenuCommandItem->Dir = buf;
	Edit_Caption.GetWindowText(buf);
	m_lpMenuCommandItem->Caption = buf;
}

//アイテム選択変更
LRESULT CConfigDlgFileListWindow::OnSelect(LPNMHDR pnmh)
{
	if(m_lpMenuCommandItem){
		getUserAppEdit();
	}

	int iItem=List_Command.GetNextItem(-1,LVNI_ALL|LVNI_SELECTED);
	if(-1!=iItem&&iItem<(signed)m_MenuCommandArray.size()){
		m_lpMenuCommandItem=&m_MenuCommandArray[iItem];
		setUserAppEdit();
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
	setUserAppEdit();

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
	setUserAppEdit();

	List_Command.EnsureVisible(iItem+1,FALSE);
	List_Command.SetItemState(iItem+1,LVIS_SELECTED, LVIS_SELECTED);
	return TRUE;
}

//項目の新規作成
LRESULT CConfigDlgFileListWindow::OnUserAppNew(WORD,WORD,HWND,BOOL&)
{
	//旧項目の保存
	if(m_lpMenuCommandItem){
		getUserAppEdit();
	}

	//新項目の設定
	CMenuCommandItem mci;
	mci.Caption=_T("UserApp");
	mci.Param=_T("%S");
	m_MenuCommandArray.push_back(mci);
	int iItem=m_MenuCommandArray.size()-1;
	m_lpMenuCommandItem=&m_MenuCommandArray[iItem];

	setUserAppEdit();
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
	setUserAppEdit();

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
