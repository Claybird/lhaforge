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

LRESULT CConfigDlgFileListWindow::OnInitDialog(HWND hWnd, LPARAM lParam)
{
	DoDataExchange(FALSE);

	m_MenuCommandArray = m_Config.MenuCommandArray;
	m_lpMenuCommandItem = nullptr;

	CRect rect;
	List_Command = GetDlgItem(IDC_LIST_FILELIST_USERAPP);
	List_Command.GetClientRect(rect);
	List_Command.InsertColumn(0, L"", LVCFMT_LEFT, rect.Width());
	List_Command.SetExtendedListViewStyle(List_Command.GetExtendedListViewStyle() | LVS_EX_FULLROWSELECT);
	List_Command.SetItemCount(m_MenuCommandArray.size());

	BOOL tmp = {};
	OnCheckChanged(0, 0, nullptr, tmp);
	return TRUE;
}

LRESULT CConfigDlgFileListWindow::OnApply()
{
	if(!DoDataExchange(TRUE)){
		return FALSE;
	}

	if(m_lpMenuCommandItem){
		getUserAppEdit();
	}
	m_Config.MenuCommandArray = m_MenuCommandArray;
	return TRUE;
}

void CConfigDlgFileListWindow::OnBrowsePath(UINT, int, HWND)
{
	const COMDLG_FILTERSPEC filter[] = {
		{L"Program", L"*.exe;*.com;*.bat;*.cmd"},
		{L"All Files",L"*.*"},
	};

	CString path;
	GetDlgItemText(IDC_EDIT_FILELIST_USERAPP_PATH, path);
	LFShellFileOpenDialog dlg(path, FOS_FORCEFILESYSTEM | FOS_FILEMUSTEXIST | FOS_PATHMUSTEXIST, nullptr, filter, COUNTOF(filter));
	if (IDCANCEL != dlg.DoModal()) {
		dlg.GetFilePath(path);
		SetDlgItemText(IDC_EDIT_FILELIST_USERAPP_PATH, path);
	}
}

void CConfigDlgFileListWindow::OnBrowseDir(UINT, int, HWND)
{
	CString path;
	GetDlgItemText(IDC_EDIT_FILELIST_USERAPP_DIR, path);
	LFShellFileOpenDialog dlg(path, FOS_FORCEFILESYSTEM | FOS_FILEMUSTEXIST | FOS_PATHMUSTEXIST | FOS_PICKFOLDERS);
	if (IDCANCEL != dlg.DoModal()) {
		dlg.GetFilePath(path);
		SetDlgItemText(IDC_EDIT_FILELIST_USERAPP_DIR, path);
	}
}

void CConfigDlgFileListWindow::OnBrowseCustomToolbarImage(UINT, int, HWND)
{
	const COMDLG_FILTERSPEC filter[] = {
		{L"Bitmap(*.bmp)", L"*.bmp"},
		{L"All Files(*.*)", L"*.*"},
	};

	CString path;
	GetDlgItemText(IDC_EDIT_CUSTOMTOOLBAR_IMAGE, path);
	LFShellFileOpenDialog dlg(path, FOS_FORCEFILESYSTEM | FOS_FILEMUSTEXIST | FOS_PATHMUSTEXIST, nullptr, filter, COUNTOF(filter));
	if (IDCANCEL != dlg.DoModal()) {
		dlg.GetFilePath(path);
		SetDlgItemText(IDC_EDIT_CUSTOMTOOLBAR_IMAGE, path);
	}
}

LRESULT CConfigDlgFileListWindow::OnGetDispInfo(LPNMHDR pnmh)
{
	LV_DISPINFO* pstLVDInfo=(LV_DISPINFO*)pnmh;

	auto &item = pstLVDInfo->item;

	if (item.iItem < 0 || (unsigned)item.iItem >= m_MenuCommandArray.size())return 1;
	CLFMenuCommandItem &mci=m_MenuCommandArray[item.iItem];

	if(item.mask & LVIF_TEXT){
		wcsncpy_s(item.pszText, item.cchTextMax, mci.Caption.c_str(), item.cchTextMax);
	}
	return 0;
}

LRESULT CConfigDlgFileListWindow::OnSelect(LPNMHDR pnmh)
{
	if (m_lpMenuCommandItem) {
		getUserAppEdit();
	}

	int iItem = List_Command.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if (0 <= iItem && (unsigned)iItem < m_MenuCommandArray.size()) {
		m_lpMenuCommandItem = &m_MenuCommandArray[iItem];
		setUserAppEdit();
	}
	return 0;
}

LRESULT CConfigDlgFileListWindow::OnUserAppMoveUpDown(WORD, WORD wID, HWND, BOOL&)
{
	if (m_lpMenuCommandItem) {
		getUserAppEdit();
	}
	bool up = (IDC_BUTTON_FILELIST_USERAPP_MOVEUP == wID);
	int iItem = List_Command.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if (iItem < 0 || (unsigned)iItem >= m_MenuCommandArray.size())return FALSE;

	int swapTarget;
	if (up) {
		if (0 == iItem) {
			MessageBeep(MB_ICONASTERISK);
			return TRUE;
		}
		swapTarget = iItem - 1;
	} else {
		if (m_MenuCommandArray.size() - 1 == (unsigned)iItem) {
			MessageBeep(MB_ICONASTERISK);
			return TRUE;
		}
		swapTarget = iItem + 1;
	}
	std::swap(m_MenuCommandArray[iItem], m_MenuCommandArray[swapTarget]);
	setUserAppEdit();

	List_Command.EnsureVisible(swapTarget, FALSE);
	List_Command.SetItemState(swapTarget, LVIS_SELECTED, LVIS_SELECTED);
	return TRUE;
}

LRESULT CConfigDlgFileListWindow::OnUserAppNew(WORD, WORD, HWND, BOOL&)
{
	//save old item
	if (m_lpMenuCommandItem) {
		getUserAppEdit();
	}

	CLFMenuCommandItem mci;
	mci.Caption = L"UserApp";
	mci.Param = L"%S";
	m_MenuCommandArray.push_back(mci);
	int iItem = m_MenuCommandArray.size() - 1;
	m_lpMenuCommandItem = &m_MenuCommandArray[iItem];

	setUserAppEdit();
	List_Command.SetItemCount(m_MenuCommandArray.size());
	List_Command.EnsureVisible(iItem, FALSE);
	List_Command.SetItemState(iItem, LVIS_SELECTED, LVIS_SELECTED);
	return TRUE;
}

LRESULT CConfigDlgFileListWindow::OnUserAppDelete(WORD,WORD,HWND,BOOL&)
{
	int iItem=List_Command.GetNextItem(-1,LVNI_ALL|LVNI_SELECTED);
	if(iItem<0||(unsigned)iItem>=m_MenuCommandArray.size())return FALSE;

	auto ite=m_MenuCommandArray.begin();
	m_MenuCommandArray.erase(m_MenuCommandArray.begin() + iItem);

	List_Command.SetItemCount(m_MenuCommandArray.size());
	if(m_MenuCommandArray.empty()){
		m_lpMenuCommandItem = nullptr;
	} else {
		if (iItem > 0)iItem--;
		m_lpMenuCommandItem = &m_MenuCommandArray[iItem];
		setUserAppEdit();

		List_Command.EnsureVisible(iItem, FALSE);
		List_Command.SetItemState(iItem, LVIS_SELECTED, LVIS_SELECTED);
	}
	return TRUE;
}
