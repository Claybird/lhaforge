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
#include "resource.h"
#include "FileListView.h"
#include "Dialogs/LogListDialog.h"
#include "ConfigCode/ConfigFileListWindow.h"
#include "Utilities/StringUtil.h"
#include "Utilities/CustomControl.h"
#include "Dialogs/TextInputDlg.h"
#include "CommonUtil.h"

struct COLUMN_DEFAULTS{
	FILEINFO_TYPE type;
	int resourceID;
	int defaultWidth;
	int align;
};

static const std::vector<COLUMN_DEFAULTS> g_defaults = {
	{FILEINFO_TYPE::FILENAME, IDS_FILELIST_COLUMN_FILENAME, 100, LVCFMT_LEFT},
	{FILEINFO_TYPE::FULLPATH, IDS_FILELIST_COLUMN_FULLPATH, 200, LVCFMT_LEFT},
	{FILEINFO_TYPE::ORIGINALSIZE, IDS_FILELIST_COLUMN_ORIGINALSIZE, 90, LVCFMT_RIGHT},
	{FILEINFO_TYPE::TYPENAME, IDS_FILELIST_COLUMN_TYPENAME, 120, LVCFMT_LEFT},
	{FILEINFO_TYPE::FILETIME, IDS_FILELIST_COLUMN_FILETIME, 120, LVCFMT_LEFT},
	{FILEINFO_TYPE::COMPRESSEDSIZE, IDS_FILELIST_COLUMN_COMPRESSEDSIZE, 90, LVCFMT_RIGHT},
	{FILEINFO_TYPE::METHOD, IDS_FILELIST_COLUMN_METHOD, 60, LVCFMT_LEFT},
	{FILEINFO_TYPE::RATIO, IDS_FILELIST_COLUMN_RATIO, 60, LVCFMT_RIGHT},
	{FILEINFO_TYPE::ATTRIBUTE, IDS_FILELIST_COLUMN_ATTRIBUTE, 60, LVCFMT_RIGHT},
};

static UINT stringID_for_type(FILEINFO_TYPE type)
{
	for (const auto& c : g_defaults) {
		if (c.type == type) {
			return c.resourceID;
		}
	}
	return -1;
}

static const std::vector<std::pair<FILEINFO_TYPE, UINT>> g_menuTable = {
	{FILEINFO_TYPE::FULLPATH,		ID_MENUITEM_LISTVIEW_COLUMN_FULLPATH},
	{FILEINFO_TYPE::ORIGINALSIZE,	ID_MENUITEM_LISTVIEW_COLUMN_ORIGINALSIZE},
	{FILEINFO_TYPE::TYPENAME,		ID_MENUITEM_LISTVIEW_COLUMN_TYPENAME},
	{FILEINFO_TYPE::FILETIME,		ID_MENUITEM_LISTVIEW_COLUMN_FILETIME},
	{FILEINFO_TYPE::COMPRESSEDSIZE,	ID_MENUITEM_LISTVIEW_COLUMN_COMPRESSEDSIZE},
	{FILEINFO_TYPE::METHOD,			ID_MENUITEM_LISTVIEW_COLUMN_METHOD},
	{FILEINFO_TYPE::RATIO,			ID_MENUITEM_LISTVIEW_COLUMN_RATIO},
	{FILEINFO_TYPE::ATTRIBUTE,		ID_MENUITEM_LISTVIEW_COLUMN_ATTRIBUTE},
};


CFileListView::CFileListView(CFileListModel& rModel, const CConfigFileListWindow &r_confFLW):
	CFileViewBase(rModel,r_confFLW),
	m_bDisplayFileSizeInByte(false),
	m_bPathOnly(false)
{
	SetFrameWnd(m_hFrameWnd);
}

LRESULT CFileListView::OnCreate(LPCREATESTRUCT lpcs)
{
	m_ShellDataManager.Init();
	SetImageList(m_ShellDataManager.GetImageList(true),LVSIL_NORMAL);
	SetImageList(m_ShellDataManager.GetImageList(false),LVSIL_SMALL);

	//sort icon on column header
	m_SortImageList.CreateFromImage(
		MAKEINTRESOURCE(IDB_BITMAP_SORTMARK), 16, 1,
		CLR_DEFAULT, IMAGE_BITMAP, LR_CREATEDIBSECTION);

	mr_Model.addEventListener(m_hWnd);

	//to invoke default callback
	SetMsgHandled(FALSE);
	return 0;
}

LRESULT CFileListView::OnDestroy()
{
	mr_Model.removeEventListener(m_hWnd);
	return 0;
}

bool CFileListView::SetColumnState(
	const std::array<int, (int)FILEINFO_TYPE::ItemCount>& columnOrder,
	const std::array<int, (int)FILEINFO_TYPE::ItemCount>& columnWidthArray)
{
	//delete all columns
	if(GetHeader().IsWindow()){
		int nCount=GetHeader().GetItemCount();
		for(;nCount>0;nCount--){
			DeleteColumn(nCount-1);
		}
	}

	m_ColumnIndexArray.fill(-1);

	// add default column to list view
	for (const auto& item : g_defaults) {
		if (-1 != index_of(columnOrder, (int)item.type)) {
			int nIndex = InsertColumn((int)item.type, UtilLoadString(item.resourceID).c_str(), item.align, item.defaultWidth, -1);
			m_ColumnIndexArray[nIndex] = (int)item.type;
		}
	}

	//column order
	int nValidColumns=0;
	for(; nValidColumns <(int)FILEINFO_TYPE::ItemCount; nValidColumns++){
		if(-1==columnOrder[nValidColumns])break;
	}

	//convert order
	std::array<int, (int)FILEINFO_TYPE::ItemCount> indexArray;
	indexArray.fill(-1);
	for (int i = 0; i < nValidColumns; i++) {
		int nIndex = index_of(m_ColumnIndexArray, columnOrder[i]);
		ASSERT(-1 != nIndex);
		if (-1 != nIndex) {
			indexArray[i] = nIndex;
		}
	}
	SetColumnOrderArray(nValidColumns,&indexArray[0]);
	for( int i = 0; i < nValidColumns; i++ ) {
		SetColumnWidth(indexArray[i], columnWidthArray[i]);
	}

	//sort icon on column header
	if(GetHeader().IsWindow()){
		GetHeader().SetImageList(m_SortImageList);
	}
	UpdateSortIcon();
	return true;
}

void CFileListView::GetColumnState(
	std::array<int, (int)FILEINFO_TYPE::ItemCount>& columnOrder,
	std::array<int, (int)FILEINFO_TYPE::ItemCount>& columnWidthArray)
{
	//get column order
	int nValidCount = GetHeader().GetItemCount();
	ASSERT(nValidCount <=(int)FILEINFO_TYPE::ItemCount);

	std::array<int, (int)FILEINFO_TYPE::ItemCount> orderArray;
	orderArray.fill(-1);
	GetColumnOrderArray(nValidCount, &orderArray[0]);
	//convert order
	columnOrder.fill(-1);
	for(int i=0;i< nValidCount;i++){
		columnOrder[i] = m_ColumnIndexArray[orderArray[i]];
	}

	for( int i = 0; i < nValidCount; i++ ) {
		columnWidthArray[i] = GetColumnWidth(orderArray[i]);
	}
}

std::vector<const ARCHIVE_ENTRY_INFO*> CFileListView::GetSelectedItems()
{
	std::vector<const ARCHIVE_ENTRY_INFO*> items;
	int nIndex = -1;
	for (;;) {
		nIndex = GetNextItem(nIndex, LVNI_ALL | LVNI_SELECTED);
		if (-1 == nIndex)break;
		const ARCHIVE_ENTRY_INFO* lpNode = mr_Model.GetFileListItemByIndex(nIndex);

		ASSERT(lpNode);

		items.push_back(lpNode);
	}
	return items;
}

LRESULT CFileListView::OnFileListNewContent(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	DeleteAllItems();
	SetItemCount(0);
	if (mr_Model.IsOK()) {
		auto lpCurrent = mr_Model.getCurrentDir();
		ASSERT(lpCurrent);
		if (lpCurrent) {
			SetItemCount((int)lpCurrent->getNumChildren());
			SetItemState(0, LVIS_FOCUSED, LVIS_FOCUSED);
		}
	}

	//accept file drop
	EnableDropTarget(true);
	return 0;
}

LRESULT CFileListView::OnDblClick(LPNMHDR pnmh)
{
	auto items = GetSelectedItems();
	if (!items.empty()) {
		if (GetKeyState(VK_SHIFT) < 0 || items.size() >= 2) {
			//open by associated app, if shift key is pressed, or multiple files are selected
			auto files = extractItemToTemporary(false, items);
			openAssociation(files);
		} else {
			auto lpNode = items.front();

			if (lpNode->is_directory()) {
				mr_Model.MoveDownDir(lpNode);
			} else {
				auto files = extractItemToTemporary(false, GetSelectedItems());
				openAssociation(files);
			}
		}
	}
	return 0;
}

LRESULT CFileListView::OnColumnRClick(int /*idCtrl*/, LPNMHDR pnmh, BOOL& bHandled)
{
	if(pnmh->hwndFrom!=GetHeader()){
		bHandled = FALSE;
		return 0;
	}

	//show context menu
	POINT point;
	GetCursorPos(&point);
	CMenu cMenu;
	cMenu.LoadMenu(IDR_LISTVIEW_HEADER_MENU);
	CMenuHandle cSubMenu(cMenu.GetSubMenu(0));

	std::array<int, (int)FILEINFO_TYPE::ItemCount> columnOrder;
	std::array<int, (int)FILEINFO_TYPE::ItemCount> columnWidthArray;
	GetColumnState(columnOrder, columnWidthArray);

	for (const auto [type, nMenu] : g_menuTable) {
		bool bEnabled=(-1!=index_of(columnOrder, (int)type));
		cSubMenu.CheckMenuItem(nMenu, MF_BYCOMMAND|(bEnabled?MF_CHECKED:MF_UNCHECKED));
	}

	int nRet = cSubMenu.TrackPopupMenu(
		TPM_NONOTIFY | TPM_RETURNCMD | TPM_LEFTALIGN | TPM_RIGHTBUTTON,
		point.x, point.y, m_hWnd, NULL);
	if(0==nRet){
		//Not selected
		return 1;
	}else if(ID_MENUITEM_LISTVIEW_COLUMN_RESET==nRet){
		//reset
		for (size_t i = 0; i < g_defaults.size();i++) {
			columnOrder[i] = (int)g_defaults[i].type;
			columnWidthArray[i] = g_defaults[i].defaultWidth;
		}
	}else{
		auto size = columnOrder.size();
		for (const auto [_type, nMenu] : g_menuTable) {
			if (nMenu == nRet) {
				auto type = (int)_type;
				for (size_t i = 0; i < size; i++) {
					if (type == columnOrder[i]) {
						//disable existing, slide items
						for (size_t j = i; j < size - 1; j++) {
							columnOrder[j] = columnOrder[j + 1];
							columnWidthArray[j] = columnWidthArray[j + 1];
						}
						columnOrder[size - 1] = -1;
						break;
					} else if (-1 == columnOrder[i]) {
						//enable adding default
						columnOrder[i] = type;
						for (const auto &d : g_defaults) {
							if ((int)d.type == type) {
								columnWidthArray[i] = d.defaultWidth;
								break;
							}
						}
						break;
					}
				}
			}
		}
	}

	SetColumnState(columnOrder, columnWidthArray);

	return 1;
}


LRESULT CFileListView::OnFindAsYouType(LPNMHDR pnmh)
{
	auto lpCurrent = mr_Model.getCurrentDir();
	ASSERT(lpCurrent);
	if (!lpCurrent)return -1;

	int iCount = (int)lpCurrent->getNumChildren();
	if (iCount <= 0)return -1;

	LPNMLVFINDITEM lpFindInfo = (LPNMLVFINDITEM)pnmh;
	int iStart = lpFindInfo->iStart;
	if (iStart < 0)iStart = 0;
	if (lpFindInfo->lvfi.flags & LVFI_STRING || lpFindInfo->lvfi.flags & LVFI_PARTIAL) {
		auto lpFindString = lpFindInfo->lvfi.psz;
		size_t nLength = wcslen(lpFindString);

		//search from current position
		for (int i = iStart; i < iCount; i++) {
			auto lpNode = mr_Model.GetFileListItemByIndex(i);
			ASSERT(lpNode);
			if (0 == _wcsnicmp(lpFindString, lpNode->_entryName.c_str(), nLength)) {
				return i;
			}
		}
		if (lpFindInfo->lvfi.flags & LVFI_WRAP) {
			for (int i = 0; i < iStart; i++) {
				auto lpNode = mr_Model.GetFileListItemByIndex(i);
				ASSERT(lpNode);
				if (0 == _wcsnicmp(lpFindString, lpNode->_entryName.c_str(), nLength)) {
					return i;
				}
			}
		}
		return -1;
	} else {
		return -1;
	}
}

void CFileListView::SortItem(int iCol)
{
	if (!(iCol >= 0 && iCol < (int)FILEINFO_TYPE::ItemCount))return;

	if (iCol == mr_Model.GetSortKeyType()) {
		if (mr_Model.IsSortAtoZ()) {
			mr_Model.SetSortAtoZ(false);
		} else {	//disable sort
			mr_Model.SetSortKeyType((int)FILEINFO_TYPE::INVALID);
			mr_Model.SetSortAtoZ(true);
		}
	} else {
		mr_Model.SetSortKeyType(iCol);
		mr_Model.SetSortAtoZ(true);
	}

	UpdateSortIcon();
}

void CFileListView::UpdateSortIcon()
{
	const int sortIconAscending = 0;
	const int sortIconDescending = 1;

	if (!IsWindow())return;

	CHeaderCtrl hc = GetHeader();
	ASSERT(hc.IsWindow());
	if (hc.IsWindow()) {
		int count = hc.GetItemCount();
		//unset icon
		for (int i = 0; i < count; i++) {
			HDITEM hdi = { 0 };
			hdi.mask = HDI_FORMAT;
			hc.GetItem(i, &hdi);
			if ((hdi.fmt & HDF_IMAGE)) {
				hdi.fmt &= ~HDF_IMAGE;
				hc.SetItem(i, &hdi);
			}
		}

		//set icon
		int iCol = mr_Model.GetSortKeyType();
		if (iCol != (int)FILEINFO_TYPE::INVALID && iCol < count) {
			HDITEM hdi = { 0 };
			hdi.mask = HDI_FORMAT;

			hc.GetItem(iCol, &hdi);
			hdi.mask |= HDI_FORMAT | HDI_IMAGE;
			hdi.fmt |= HDF_IMAGE | HDF_BITMAP_ON_RIGHT;
			hdi.iImage = mr_Model.IsSortAtoZ() ? sortIconAscending : sortIconDescending;
			hc.SetItem(iCol, &hdi);
		}
	}
}

DWORD CFileListView::OnItemPrePaint(int nID, LPNMCUSTOMDRAW lpnmcd)
{
	if (lpnmcd->hdr.hwndFrom == m_hWnd) {
		auto lpnmlv = (LPNMLVCUSTOMDRAW)lpnmcd;

		auto lpNode = mr_Model.GetFileListItemByIndex((int)lpnmcd->dwItemSpec);
		if (lpNode) {
			if (!UtilIsSafeUnicode(lpNode->_entry.path)) {
				//not a safe entry
				lpnmlv->clrText = RGB(255, 255, 255);
				lpnmlv->clrTextBk = RGB(255, 0, 0);
			}
		}
	}
	return CDRF_DODEFAULT;
}

std::wstring CFileListView::fileInfoHelper(
	FILEINFO_TYPE type,
	const ARCHIVE_ENTRY_INFO* lpNode,
	bool followConfig
)//NOTE: cannot set "const" because of m_ShellDataManager
{
	switch (type) {
	case FILEINFO_TYPE::FILENAME:
		return lpNode->_entryName;
	case FILEINFO_TYPE::FULLPATH:
		if (followConfig && m_bPathOnly) {
			return lpNode->_entry.path.parent_path();
		} else {
			return lpNode->_entry.path;
		}
		break;
	case FILEINFO_TYPE::ORIGINALSIZE:
		if (!followConfig || m_bDisplayFileSizeInByte) {
			return Format(L"%llu", lpNode->_originalSize);
		} else {
			return UtilFormatSize(lpNode->_originalSize);
		}
		break;
	case FILEINFO_TYPE::TYPENAME:
		return m_ShellDataManager.GetTypeName(lpNode->getExt().c_str());
	case FILEINFO_TYPE::FILETIME:
		if (lpNode->_entry.path.empty()) {
			return L"---";
		} else {
			return UtilFormatTime(lpNode->_entry.stat.st_mtime);
		}
		break;
	case FILEINFO_TYPE::COMPRESSEDSIZE:
		if (lpNode->_entry.compressed_size == -1) {
			return L"---";
		} else {
			if (!followConfig || m_bDisplayFileSizeInByte) {
				return Format(L"%llu", lpNode->_entry.compressed_size);
			} else {
				return UtilFormatSize(lpNode->_entry.compressed_size);
			}
		}
		break;
	case FILEINFO_TYPE::METHOD:
		return lpNode->_entry.method_name;
	case FILEINFO_TYPE::RATIO:
		if (lpNode->_entry.compressed_size == -1) {
			return L"---";
		} else {
			return Format(L"%.2f%%", lpNode->compress_ratio());
		}
		break;
	case FILEINFO_TYPE::ATTRIBUTE:
	{
		std::wstring info = L"";
		if (lpNode->_entry.stat.st_mode & _S_IFDIR) {
			info += L'D';
		} else {
			info += L'-';
		}
		if (lpNode->_entry.stat.st_mode & _S_IWRITE) {
			info += L'-';
		} else {
			info += L'R';
		}
		return info;
	}
	default:
		return L"";
#ifndef NDEBUG
		RAISE_EXCEPTION(L"Not implemented");
#endif
	}
}

//item information
LRESULT CFileListView::OnGetDispInfo(LPNMHDR pnmh)
{
	auto pstLVDInfo=(LV_DISPINFO*)pnmh;

	auto lpNode = mr_Model.GetFileListItemByIndex(pstLVDInfo->item.iItem);
	if(!lpNode)return 0;

	ASSERT(pstLVDInfo->item.iSubItem>=0 && pstLVDInfo->item.iSubItem<(int)FILEINFO_TYPE::ItemCount);
	if (pstLVDInfo->item.iSubItem < 0 || pstLVDInfo->item.iSubItem >= (int)FILEINFO_TYPE::ItemCount)return 0;

	auto type = (FILEINFO_TYPE)(m_ColumnIndexArray[pstLVDInfo->item.iSubItem]);
	if(type == FILEINFO_TYPE::FILENAME){
		if (pstLVDInfo->item.mask & LVIF_IMAGE) {
			pstLVDInfo->item.iImage = m_ShellDataManager.GetIconIndex(lpNode->getExt().c_str());
		}
	}

	if (pstLVDInfo->item.mask & LVIF_TEXT) {
		auto info = fileInfoHelper(type, lpNode, true);
		wcsncpy_s(pstLVDInfo->item.pszText, pstLVDInfo->item.cchTextMax, info.c_str(), pstLVDInfo->item.cchTextMax);
	}
	return 0;
}

LRESULT CFileListView::OnGetInfoTip(LPNMHDR pnmh)
{
	LPNMLVGETINFOTIP pGetInfoTip = (LPNMLVGETINFOTIP)pnmh;

	auto lpNode = mr_Model.GetFileListItemByIndex(pGetInfoTip->iItem);
	ASSERT(lpNode);
	if (!lpNode)return 0;
	std::wstring strInfo;

	if (!UtilIsSafeUnicode(lpNode->_entry.path)) {
		strInfo += UtilLoadString(IDS_ERROR_UNICODECONTROL) + L"\n";
	}

	for (const auto [type, nMenuID] : g_menuTable) {
		strInfo += UtilLoadString(stringID_for_type(type));
		strInfo += L" : " + fileInfoHelper(type, lpNode, true) + L"\n";
	}

	if (lpNode->is_directory()) {
		//show number of children
		strInfo += L"\n";
		strInfo += UtilLoadString(IDS_FILELIST_SUBITEM);
		strInfo += L" : ";
		strInfo += Format(UtilLoadString(IDS_FILELIST_ITEMCOUNT), lpNode->getNumChildren());
	}

	wcsncpy_s(pGetInfoTip->pszText, pGetInfoTip->cchTextMax, strInfo.c_str(), pGetInfoTip->cchTextMax);
	return 0;
}

//-------
void CFileListView::OnAddItems(UINT uNotifyCode,int nID,HWND hWndCtrl)
{
	ASSERT(mr_Model.IsModifySupported());
	if(!mr_Model.IsModifySupported())return;

	auto dest = mr_Model.getCurrentDir();

	std::vector<std::filesystem::path> files;
	if(nID==ID_MENUITEM_ADD_FILE){
		//add file
		const COMDLG_FILTERSPEC filter[] = {
			{ L"All Files", L"*.*" },
		};

		CLFShellFileOpenDialog dlg(nullptr, FOS_FORCEFILESYSTEM | FOS_FILEMUSTEXIST | FOS_PATHMUSTEXIST | FOS_ALLOWMULTISELECT, nullptr, filter, COUNTOF(filter));
		if(IDOK==dlg.DoModal()){
			files = dlg.GetMultipleFiles();
		}
	}else{
		//add directory
		CLFShellFileOpenDialog dlg(nullptr, FOS_FORCEFILESYSTEM | FOS_FILEMUSTEXIST | FOS_PATHMUSTEXIST | FOS_PICKFOLDERS);
		if(IDOK==dlg.DoModal()){
			CString path;
			dlg.GetFilePath(path);
			files.push_back(path.operator LPCWSTR());
		}
	}

	if(!files.empty()){
		AddItemsToDirectory(dest, files);
	}
}

// Extract items invoked by DnD
LRESULT CFileListView::OnBeginDrag(LPNMHDR pnmh)
{
	if(!mr_Model.CheckArchiveExists())return 0;

	auto items = GetSelectedItems();
	if(items.empty()){
		ASSERT(!"This code cannot be run");
		return 0;
	}

	if(!mr_Model.ClearTempDir()){
		//temporary directory clean-up failed
		//ErrorMessage(UtilLoadString(IDS_ERROR_CANT_CLEAR_TEMPDIR));
		return 0;
	}

	::EnableWindow(m_hFrameWnd,FALSE);

	ARCLOG arcLog;
	HRESULT hr=m_DnDSource.DoDragDrop(
		mr_Model,
		items,
		mr_Model.getCurrentDir(),
		mr_Model.getTempDir(),
		m_hFrameWnd,
		arcLog);
	::EnableWindow(m_hFrameWnd, TRUE);
	::SetForegroundWindow(m_hFrameWnd);

	if(FAILED(hr)){
		//TODO
		CLogListDialog LogDlg(L"Log");
		std::vector<ARCLOG> logs = { arcLog };
		LogDlg.SetLogArray(logs);
		LogDlg.DoModal(m_hFrameWnd);
	}

	return 0;
}

void CFileListView::OnSelectAll(UINT,int,HWND)
{
	SetItemState(-1, LVIS_SELECTED, LVIS_SELECTED);
	SetFocus();
}

void CFileListView::OnCopyInfo(UINT uNotifyCode,int nID,HWND hWndCtrl)
{
	auto items = GetSelectedItems();

	struct COPYSUBJECTS {
		UINT nID;
		FILEINFO_TYPE type;
	};
	const std::vector<COPYSUBJECTS> copySubjects = {
		{ ID_MENUITEM_COPY_FILENAME, FILEINFO_TYPE::FILENAME},
		{ ID_MENUITEM_COPY_PATH, FILEINFO_TYPE::FULLPATH},
		{ ID_MENUITEM_COPY_ORIGINAL_SIZE, FILEINFO_TYPE::ORIGINALSIZE},
		{ ID_MENUITEM_COPY_FILETYPE, FILEINFO_TYPE::TYPENAME },
		{ ID_MENUITEM_COPY_FILETIME, FILEINFO_TYPE::FILETIME },
		{ ID_MENUITEM_COPY_METHOD, FILEINFO_TYPE::METHOD},
		{ ID_MENUITEM_COPY_COMPRESSED_SIZE, FILEINFO_TYPE::COMPRESSEDSIZE},
		{ ID_MENUITEM_COPY_COMPRESSION_RATIO, FILEINFO_TYPE::RATIO},
		{ ID_MENUITEM_COPY_ATTRIBUTE, FILEINFO_TYPE::ATTRIBUTE},
		//{ ID_MENUITEM_COPY_ALL, FILEINFO_TYPE::INVALID}
	};

	std::wstring info;
	if (nID == ID_MENUITEM_COPY_ALL) {
		//header
		for (size_t i = 0; i < copySubjects.size(); i++) {
			info += UtilLoadString(stringID_for_type(copySubjects[i].type));
			if (i == copySubjects.size() - 1) {
				info += L"\n";
			} else {
				info += L",";
			}
		}
		//content
		for (const auto& item : items) {
			for (size_t i = 0; i < copySubjects.size(); i++) {
				info += fileInfoHelper(copySubjects[i].type, item, false);
				if (i == copySubjects.size() - 1) {
					info += L"\n";
				} else {
					info += L",";
				}
			}
		}
	} else {
		for (const auto & cs : copySubjects) {
			if (cs.nID == nID) {
				info += UtilLoadString(stringID_for_type(cs.type));
				info += L"\n";
				for (const auto& item : items) {
					info += fileInfoHelper(cs.type, item, false) + L"\n";
				}
				break;
			}
		}
	}

	UtilSetTextOnClipboard(info);
}

#include "Dialogs/FindDlg.h"
void CFileListView::OnFindItem(UINT uNotifyCode,int nID,HWND hWndCtrl)
{
	if(ID_MENUITEM_FINDITEM_END==nID){
		mr_Model.EndFindItem();
	}else{
		CLFFindDialog dlg;
		if (IDOK == dlg.DoModal()) {
			auto afc = dlg.getCondition();
			mr_Model.EndFindItem();
			auto lpFound = mr_Model.FindItem(afc, mr_Model.getCurrentDir());
			mr_Model.setCurrentDir(lpFound);
		}
	}
}

void CFileListView::OnShowCustomizeColumn(UINT,int,HWND)
{
	//enumlate column header R-click
	BOOL bTemp;
	NMHDR nmhdr;
	nmhdr.hwndFrom=GetHeader();
	OnColumnRClick(0, &nmhdr, bTemp);
}
