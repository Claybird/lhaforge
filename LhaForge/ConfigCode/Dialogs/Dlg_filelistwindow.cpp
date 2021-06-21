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
#include "Utilities/CustomControl.h"
#include "Dialogs/FindDlg.h"

LRESULT CConfigDlgFileListWindow::OnInitDialog(HWND hWnd, LPARAM lParam)
{
	DoDataExchange(FALSE);

	m_lpMenuCommandItem = nullptr;

	CRect rect;
	List_Command = GetDlgItem(IDC_LIST_FILELIST_USERAPP);
	List_Command.GetClientRect(rect);
	List_Command.InsertColumn(0, L"", LVCFMT_LEFT, rect.Width());
	List_Command.SetExtendedListViewStyle(List_Command.GetExtendedListViewStyle() | LVS_EX_FULLROWSELECT);
	List_Command.SetItemCount(m_Config.view.MenuCommandArray.size());

	m_lpSearchItem = nullptr;

	List_Search = GetDlgItem(IDC_LIST_SEARCH_FOLDER);
	List_Search.GetClientRect(rect);
	List_Search.InsertColumn(0, UtilLoadString(IDS_SEARCH_FOLDER_NAME).c_str(), LVCFMT_LEFT, 180);
	List_Search.InsertColumn(1, UtilLoadString(IDS_SEARCH_FOLDER_CONDITION).c_str(), LVCFMT_LEFT, rect.Width() - 180);
	List_Search.SetExtendedListViewStyle(List_Search.GetExtendedListViewStyle() | LVS_EX_FULLROWSELECT);
	List_Search.SetItemCount(m_Config.view.searchFolderItems.size());

	BOOL tmp = {};
	OnCheckChanged(0, 0, nullptr, tmp);
	return TRUE;
}

LRESULT CConfigDlgFileListWindow::OnApply()
{
	if(!DoDataExchange(TRUE)){
		return FALSE;
	}

	updateUserAppEdit();
	updateSearchItemName();
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
	CLFShellFileOpenDialog dlg(path, FOS_FORCEFILESYSTEM | FOS_FILEMUSTEXIST | FOS_PATHMUSTEXIST, nullptr, filter, COUNTOF(filter));
	if (IDCANCEL != dlg.DoModal()) {
		dlg.GetFilePath(path);
		SetDlgItemText(IDC_EDIT_FILELIST_USERAPP_PATH, path);
	}
}

void CConfigDlgFileListWindow::OnBrowseDir(UINT, int, HWND)
{
	CString path;
	GetDlgItemText(IDC_EDIT_FILELIST_USERAPP_DIR, path);
	CLFShellFileOpenDialog dlg(path, FOS_FORCEFILESYSTEM | FOS_FILEMUSTEXIST | FOS_PATHMUSTEXIST | FOS_PICKFOLDERS);
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
	CLFShellFileOpenDialog dlg(path, FOS_FORCEFILESYSTEM | FOS_FILEMUSTEXIST | FOS_PATHMUSTEXIST, nullptr, filter, COUNTOF(filter));
	if (IDCANCEL != dlg.DoModal()) {
		dlg.GetFilePath(path);
		SetDlgItemText(IDC_EDIT_CUSTOMTOOLBAR_IMAGE, path);
	}
}

LRESULT CConfigDlgFileListWindow::OnGetDispInfo(LPNMHDR pnmh)
{
	LV_DISPINFO* pstLVDInfo=(LV_DISPINFO*)pnmh;

	auto& item = pstLVDInfo->item;
	if (pnmh->hwndFrom == List_Command) {
		if (item.mask & LVIF_TEXT) {
			const auto& mca = m_Config.view.MenuCommandArray;
			if (item.iItem < 0 || (unsigned)item.iItem >= mca.size())return 1;

			const CLFMenuCommandItem& mci = mca[item.iItem];
			wcsncpy_s(item.pszText, item.cchTextMax, mci.Caption.c_str(), item.cchTextMax);
		}
	} else if (pnmh->hwndFrom == List_Search) {
		if (item.mask & LVIF_TEXT) {
			const auto& sfis = m_Config.view.searchFolderItems;
			if (item.iItem < 0 || (unsigned)item.iItem >= sfis.size())return 1;

			const auto& sfi = sfis[item.iItem];
			if (item.iSubItem == 0){
				wcsncpy_s(item.pszText, item.cchTextMax, sfi.first.c_str(), item.cchTextMax);
			} else {
				wcsncpy_s(item.pszText, item.cchTextMax, sfi.second.toString().c_str(), item.cchTextMax);
			}
		}
	}
	return 0;
}

LRESULT CConfigDlgFileListWindow::OnSelect(LPNMHDR pnmh)
{
	if (pnmh->hwndFrom == List_Command) {
		updateUserAppEdit();

		auto& mca = m_Config.view.MenuCommandArray;
		int iItem = List_Command.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
		if (0 <= iItem && (unsigned)iItem < mca.size()) {
			m_lpMenuCommandItem = &mca[iItem];
			setUserAppEdit();
		}
	} else if (pnmh->hwndFrom == List_Search) {
		updateSearchItemName();

		auto& sfis = m_Config.view.searchFolderItems;
		int iItem = List_Search.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
		if (0 <= iItem && (unsigned)iItem < sfis.size()) {
			m_lpSearchItem = &sfis[iItem];
			setSearchItemName();
		}
	}
	return 0;
}

//move item in list. return new position
template<typename T>
int updown_base(std::vector<T>& subject, bool up, CListViewCtrl& listView)
{
	int iItem = listView.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if (iItem < 0 || (unsigned)iItem >= subject.size())return iItem;

	int newPos;
	if (up) {
		if (0 == iItem) {
			MessageBeep(MB_ICONASTERISK);
			return iItem;
		}
		newPos = iItem - 1;
	} else {
		if (subject.size() - 1 == (unsigned)iItem) {
			MessageBeep(MB_ICONASTERISK);
			return iItem;
		}
		newPos = iItem + 1;
	}
	std::swap(subject[iItem], subject[newPos]);

	listView.EnsureVisible(newPos, FALSE);
	listView.SetItemState(newPos, LVIS_SELECTED, LVIS_SELECTED);

	return newPos;
}

LRESULT CConfigDlgFileListWindow::OnUserAppMoveUpDown(WORD, WORD wID, HWND, BOOL&)
{
	updateUserAppEdit();

	auto& mca = m_Config.view.MenuCommandArray;
	bool up = (IDC_BUTTON_FILELIST_USERAPP_MOVEUP == wID);
	auto newPos = updown_base(mca, up, List_Command);
	m_lpMenuCommandItem = &mca[newPos];
	setUserAppEdit();
	return 0;
}

LRESULT CConfigDlgFileListWindow::OnSearchItemMoveUpDown(WORD, WORD wID, HWND, BOOL&)
{
	updateSearchItemName();

	auto& sfis = m_Config.view.searchFolderItems;
	bool up = (IDC_BUTTON_SEARCH_ITEM_MOVEUP == wID);
	auto newPos = updown_base(sfis, up, List_Search);
	m_lpSearchItem = &sfis[newPos];
	setSearchItemName();
	return 0;
}

LRESULT CConfigDlgFileListWindow::OnUserAppNew(WORD, WORD, HWND, BOOL&)
{
	//save old item
	updateUserAppEdit();

	auto& mca = m_Config.view.MenuCommandArray;
	CLFMenuCommandItem mci;
	mci.Caption = L"UserApp";
	mci.Param = L"%S";
	mca.push_back(mci);
	m_lpMenuCommandItem = &mca.back();

	setUserAppEdit();
	auto& lv = List_Command;
	lv.SetItemCount(mca.size());
	int iItem = mca.size() - 1;
	lv.EnsureVisible(iItem, FALSE);
	lv.SetItemState(iItem, LVIS_SELECTED, LVIS_SELECTED);
	return 0;
}

LRESULT CConfigDlgFileListWindow::OnSearchItemNew(WORD, WORD, HWND, BOOL&)
{
	//save old item
	updateSearchItemName();

	CLFFindDialog dlg;
	if (IDOK == dlg.DoModal()) {
		auto& sfis = m_Config.view.searchFolderItems;
		auto sfi = dlg.getCondition();
		sfis.push_back(std::make_pair(L"New Search", sfi));
		m_lpSearchItem = &sfis.back();

		setSearchItemName();
		auto& lv = List_Search;
		lv.SetItemCount(sfis.size());
		int iItem = sfis.size() - 1;
		lv.EnsureVisible(iItem, FALSE);
		lv.SetItemState(iItem, LVIS_SELECTED, LVIS_SELECTED);
	}
	return 0;
}

LRESULT CConfigDlgFileListWindow::OnUserAppDelete(WORD,WORD,HWND,BOOL&)
{
	auto& mca = m_Config.view.MenuCommandArray;
	auto& lv = List_Command;
	int iItem=lv.GetNextItem(-1,LVNI_ALL|LVNI_SELECTED);
	if(iItem<0||(unsigned)iItem>= mca.size())return 0;

	mca.erase(mca.begin() + iItem);

	lv.SetItemCount(mca.size());
	if(mca.empty()){
		m_lpMenuCommandItem = nullptr;
	} else {
		if (iItem > 0)iItem--;
		m_lpMenuCommandItem = &mca[iItem];
		setUserAppEdit();

		lv.EnsureVisible(iItem, FALSE);
		lv.SetItemState(iItem, LVIS_SELECTED, LVIS_SELECTED);
	}
	return 0;
}

LRESULT CConfigDlgFileListWindow::OnSearchItemDelete(WORD, WORD, HWND, BOOL&)
{
	auto& sfis = m_Config.view.searchFolderItems;
	auto& lv = List_Search;
	int iItem = lv.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if (iItem < 0 || (unsigned)iItem >= sfis.size())return 0;

	sfis.erase(sfis.begin() + iItem);

	lv.SetItemCount(sfis.size());
	if (sfis.empty()) {
		m_lpSearchItem = nullptr;
	} else {
		if (iItem > 0)iItem--;
		m_lpSearchItem = &sfis[iItem];
		setSearchItemName();

		lv.EnsureVisible(iItem, FALSE);
		lv.SetItemState(iItem, LVIS_SELECTED, LVIS_SELECTED);
	}
	return 0;
}

LRESULT CConfigDlgFileListWindow::OnSearchItemEdit(WORD, WORD, HWND, BOOL&)
{
	updateSearchItemName();
	if (m_lpSearchItem) {
		CLFFindDialog dlg;
		dlg.setCondition(m_lpSearchItem->second);
		if (IDOK == dlg.DoModal()) {
			m_lpSearchItem->second = dlg.getCondition();
			List_Search.Invalidate();
		}
	}
	return 0;
}

