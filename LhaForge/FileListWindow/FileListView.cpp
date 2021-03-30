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
#include "Dialogs/TextInputDlg.h"
#include "CommonUtil.h"

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
	const std::array<int, FILEINFO_ITEM_COUNT>& columnOrder,
	const std::array<int, FILEINFO_ITEM_COUNT>& columnWidthArray)
{
	//delete all columns
	if(GetHeader().IsWindow()){
		int nCount=GetHeader().GetItemCount();
		for(;nCount>0;nCount--){
			DeleteColumn(nCount-1);
		}
	}

	m_ColumnIndexArray.fill(-1);

	// add column to list view
	auto addColumn = [&](int columnID, int nTitileSrcID, int width, int pos)->void {
		if (-1 != index_of(columnOrder, columnID)) {
			int nIndex = InsertColumn(columnID, UtilLoadString(nTitileSrcID).c_str(), pos, width, -1);
			if (0 <= nIndex && nIndex < FILEINFO_ITEM_COUNT) {
				m_ColumnIndexArray[nIndex] = columnID;
			}
		}
	};
	
	//helper macro
	#define ADD_COLUMNITEM(x,width,pos) addColumn(FILEINFO_##x, IDS_FILELIST_COLUMN_##x, width, pos)

	ADD_COLUMNITEM(FILENAME, 100, LVCFMT_LEFT);
	ADD_COLUMNITEM(FULLPATH, 200, LVCFMT_LEFT);
	ADD_COLUMNITEM(ORIGINALSIZE, 90, LVCFMT_RIGHT);
	ADD_COLUMNITEM(TYPENAME, 120, LVCFMT_LEFT);
	ADD_COLUMNITEM(FILETIME, 120, LVCFMT_LEFT);
	ADD_COLUMNITEM(ATTRIBUTE, 60, LVCFMT_LEFT);
	ADD_COLUMNITEM(COMPRESSEDSIZE, 90, LVCFMT_RIGHT);
	ADD_COLUMNITEM(METHOD, 60, LVCFMT_LEFT);
	ADD_COLUMNITEM(RATIO, 60, LVCFMT_RIGHT);
	ADD_COLUMNITEM(CRC, 60, LVCFMT_LEFT);

	//column order
	int nValidColumns=0;
	for(; nValidColumns <FILEINFO_ITEM_COUNT; nValidColumns++){
		if(-1==columnOrder[nValidColumns])break;
	}

	//convert order
	std::array<int, FILEINFO_ITEM_COUNT> indexArray;
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
	std::array<int, FILEINFO_ITEM_COUNT>& columnOrder,
	std::array<int, FILEINFO_ITEM_COUNT>& columnWidthArray)
{
	//get column order
	int nValidCount = GetHeader().GetItemCount();
	ASSERT(nValidCount <=FILEINFO_ITEM_COUNT);

	std::array<int, FILEINFO_ITEM_COUNT> orderArray;
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
			SetItemCount(lpCurrent->getNumChildren());
			SetItemState(0, LVIS_FOCUSED, LVIS_FOCUSED);
		}
	}

	//accept file drop
	EnableDropTarget(true);
	return 0;
}

LRESULT CFileListView::OnDblClick(LPNMHDR pnmh)
{
	//----------------------------------------------------------------------
	//選択アイテムが複数の時:
	// 選択を関連付けで開く
	//Shiftが押されていた時:
	// 選択を関連付けで開く
	//選択アイテムが一つだけの時:
	// フォルダをダブルクリックした/Enterを押した場合にはそのフォルダを開く
	// 選択がファイルだった場合には関連付けで開く
	//----------------------------------------------------------------------

	//選択アイテムが複数の時
	//もしShiftが押されていたら、関連付けで開く
	if(GetKeyState(VK_SHIFT)<0||GetSelectedCount()>=2){
		auto files = extractItemToTemporary(false, GetSelectedItems());
		openAssociation(files);
		return 0;
	} else {
		//選択されたアイテムを取得
		auto items = GetSelectedItems();
		if (items.empty())return 0;
		auto lpNode = items.front();

		if (lpNode->is_directory()) {
			mr_Model.MoveDownDir(lpNode);
		} else {
			auto files = extractItemToTemporary(false, GetSelectedItems());
			openAssociation(files);
		}
	}
	return 0;
}


//カラム表示のOn/Offを切り替える
//表示中:該当カラムを非表示にし、配列を詰める
//非表示:使われていない部分に指定カラムを追加
void _ToggleColumn(int *lpArray,size_t size,FILEINFO_TYPE type)
{
	ASSERT(lpArray);
	if(!lpArray)return;

	for(size_t i=0;i<size;i++){
		if(type==lpArray[i]){
			//配列を詰める
			for(size_t j=i;j<size-1;j++){
				lpArray[j]=lpArray[j+1];
			}
			lpArray[size-1]=-1;
			return;
		}
		else if(-1==lpArray[i]){
			lpArray[i]=type;
			return;
		}
	}
}


//カラムヘッダを左/右クリック
LRESULT CFileListView::OnColumnRClick(int /*idCtrl*/, LPNMHDR pnmh, BOOL& bHandled)
{
	if(pnmh->hwndFrom!=GetHeader()){
		//メッセージは処理しなかったことにする
		bHandled = FALSE;
		return 0;
	}

	//右クリックメニュー表示
	POINT point;
	GetCursorPos(&point);
	CMenu cMenu;
	cMenu.LoadMenu(IDR_LISTVIEW_HEADER_MENU);
	CMenuHandle cSubMenu(cMenu.GetSubMenu(0));

	//--------------------------------
	// 各メニューアイテムの有効・無効
	//--------------------------------

	std::array<int, FILEINFO_ITEM_COUNT> columnOrder;
	std::array<int, FILEINFO_ITEM_COUNT> columnWidthArray;
	GetColumnState(columnOrder, columnWidthArray);

	struct{
		FILEINFO_TYPE idx;
		UINT nMenuID;
	}menuTable[]={
		{FILEINFO_FULLPATH,			ID_MENUITEM_LISTVIEW_COLUMN_FULLPATH},
		{FILEINFO_ORIGINALSIZE,		ID_MENUITEM_LISTVIEW_COLUMN_ORIGINALSIZE},
		{FILEINFO_TYPENAME,			ID_MENUITEM_LISTVIEW_COLUMN_TYPENAME},
		{FILEINFO_FILETIME,			ID_MENUITEM_LISTVIEW_COLUMN_FILETIME},
		{FILEINFO_ATTRIBUTE,		ID_MENUITEM_LISTVIEW_COLUMN_ATTRIBUTE},
		{FILEINFO_COMPRESSEDSIZE,	ID_MENUITEM_LISTVIEW_COLUMN_COMPRESSEDSIZE},
		{FILEINFO_METHOD,			ID_MENUITEM_LISTVIEW_COLUMN_METHOD},
		{FILEINFO_RATIO,			ID_MENUITEM_LISTVIEW_COLUMN_RATIO},
		{FILEINFO_CRC,				ID_MENUITEM_LISTVIEW_COLUMN_CRC},
	};

	for(const auto &item: menuTable){
		bool bEnabled=(-1!=index_of(columnOrder, item.idx));
		cSubMenu.CheckMenuItem(item.nMenuID,MF_BYCOMMAND|(bEnabled?MF_CHECKED:MF_UNCHECKED));
	}

	//メニュー表示:選択したコマンドが返ってくる
	int nRet=cSubMenu.TrackPopupMenu(TPM_NONOTIFY|TPM_RETURNCMD|TPM_LEFTALIGN|TPM_RIGHTBUTTON,point.x, point.y, m_hWnd,NULL);
	if(0==nRet){
		//Not Selected
		return 0;
	}else if(ID_MENUITEM_LISTVIEW_COLUMN_RESET==nRet){
		//初期化
		for(size_t i=0;i<columnOrder.size();i++){
			columnOrder[i]=i;
		}
	}else{
		for(size_t i=0;i<COUNTOF(menuTable);i++){
			if(menuTable[i].nMenuID==nRet){
				_ToggleColumn(&columnOrder[0],columnOrder.size(),menuTable[i].idx);
			}
		}
	}

	SetColumnState(columnOrder, columnWidthArray);

	return 0;
}


LRESULT CFileListView::OnFindAsYouType(LPNMHDR pnmh)
{
	auto lpCurrent = mr_Model.getCurrentDir();
	ASSERT(lpCurrent);
	if (!lpCurrent)return -1;

	int iCount = lpCurrent->getNumChildren();
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
	if (!(iCol >= 0 && iCol < FILEINFO_ITEM_COUNT))return;

	if (iCol == mr_Model.GetSortKeyType()) {
		if (mr_Model.GetSortMode()) {
			mr_Model.SetSortMode(false);
		} else {	//disable sort
			mr_Model.SetSortKeyType(FILEINFO_INVALID);
			mr_Model.SetSortMode(true);
		}
	} else {
		mr_Model.SetSortKeyType(iCol);
		mr_Model.SetSortMode(true);
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
		if (iCol != FILEINFO_INVALID && iCol < count) {
			HDITEM hdi = { 0 };
			hdi.mask = HDI_FORMAT;

			hc.GetItem(iCol, &hdi);
			hdi.mask |= HDI_FORMAT | HDI_IMAGE;
			hdi.fmt |= HDF_IMAGE | HDF_BITMAP_ON_RIGHT;
			hdi.iImage = mr_Model.GetSortMode() ? sortIconAscending : sortIconDescending;
			hc.SetItem(iCol, &hdi);
		}
	}
}

DWORD CFileListView::OnItemPrePaint(int nID, LPNMCUSTOMDRAW lpnmcd)
{
	if (lpnmcd->hdr.hwndFrom == m_hWnd) {
		auto lpnmlv = (LPNMLVCUSTOMDRAW)lpnmcd;

		auto lpNode = mr_Model.GetFileListItemByIndex(lpnmcd->dwItemSpec);
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


//item information
LRESULT CFileListView::OnGetDispInfo(LPNMHDR pnmh)
{
	auto pstLVDInfo=(LV_DISPINFO*)pnmh;

	auto lpNode = mr_Model.GetFileListItemByIndex(pstLVDInfo->item.iItem);
	if(!lpNode)return 0;

	ASSERT(pstLVDInfo->item.iSubItem>=0 && pstLVDInfo->item.iSubItem<FILEINFO_ITEM_COUNT);
	if (pstLVDInfo->item.iSubItem < 0 || pstLVDInfo->item.iSubItem >= FILEINFO_ITEM_COUNT)return 0;

	std::wstring info;
	switch(m_ColumnIndexArray[pstLVDInfo->item.iSubItem]){
	case FILEINFO_FILENAME:
		if (pstLVDInfo->item.mask & LVIF_TEXT)info = lpNode->_entryName;
		if (pstLVDInfo->item.mask & LVIF_IMAGE)pstLVDInfo->item.iImage = m_ShellDataManager.GetIconIndex(lpNode->getExt().c_str());
		break;
	case FILEINFO_FULLPATH:
		if(pstLVDInfo->item.mask & LVIF_TEXT){
			if(m_bPathOnly){
				info = lpNode->_entry.path.parent_path();
			}else{
				info = lpNode->_entry.path;
			}
		}
		break;
	case FILEINFO_ORIGINALSIZE:
		if(pstLVDInfo->item.mask & LVIF_TEXT){
			if (m_bDisplayFileSizeInByte) {
				info = Format(L"%llu Bytes", lpNode->_originalSize);
			}else{
				info = UtilFormatSize(lpNode->_originalSize);
			}
		}
		break;
	case FILEINFO_TYPENAME:
		if (pstLVDInfo->item.mask & LVIF_TEXT) {
			info = m_ShellDataManager.GetTypeName(lpNode->getExt().c_str());
		}
		break;
	case FILEINFO_FILETIME:
		if(pstLVDInfo->item.mask & LVIF_TEXT){
			if (lpNode->_entry.path.empty()) {
				info = L"---";
			} else {
				info = UtilFormatTime(lpNode->_entry.stat.st_mtime);
			}
		}
		break;
	//TODO
	case FILEINFO_ATTRIBUTE:
	case FILEINFO_METHOD:
	case FILEINFO_RATIO:
	case FILEINFO_CRC:
	case FILEINFO_COMPRESSEDSIZE:
		if (pstLVDInfo->item.mask & LVIF_TEXT) {
			info = L"";
		}
		break;
	}

	if (pstLVDInfo->item.mask & LVIF_TEXT) {
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

	//filename
	strInfo += UtilLoadString(IDS_FILELIST_COLUMN_FILENAME);
	strInfo += L" : " + lpNode->_entryName + L"\n";
	//path
	strInfo += UtilLoadString(IDS_FILELIST_COLUMN_FULLPATH);
	strInfo += L" : " + lpNode->_entry.path.wstring() + L"\n";
	//original size
	strInfo += UtilLoadString(IDS_FILELIST_COLUMN_ORIGINALSIZE);
	strInfo += L" : " + UtilFormatSize(lpNode->_originalSize) + L"\n";
	//filetype
	strInfo += UtilLoadString(IDS_FILELIST_COLUMN_TYPENAME);
	strInfo += L" : " + m_ShellDataManager.GetTypeName(lpNode->getExt().c_str()) + L"\n";
	//filetime
	strInfo += UtilLoadString(IDS_FILELIST_COLUMN_FILETIME);
	if (lpNode->_entry.path.empty()) {
		strInfo += L" : ---\n";
	}else{
		strInfo += L" : " + UtilFormatTime(lpNode->_entry.stat.st_mtime) + L"\n";
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

		LFShellFileOpenDialog dlg(nullptr, FOS_FORCEFILESYSTEM | FOS_FILEMUSTEXIST | FOS_PATHMUSTEXIST | FOS_ALLOWMULTISELECT, nullptr, filter, COUNTOF(filter));
		if(IDOK==dlg.DoModal()){
			files = dlg.GetMultipleFiles();
		}
	}else{
		//add directory
		LFShellFileOpenDialog dlg(nullptr, FOS_FORCEFILESYSTEM | FOS_FILEMUSTEXIST | FOS_PATHMUSTEXIST | FOS_PICKFOLDERS);
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
		ErrorMessage(UtilLoadString(IDS_ERROR_CANT_CLEAR_TEMPDIR));
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

	std::wstring info;

	switch(nID){
	case ID_MENUITEM_COPY_FILENAME:
		for(const auto &item: items){
			info += item->_entryName + L"\n";
		}
		break;
	case ID_MENUITEM_COPY_PATH:
		for (const auto &item : items) {
			info += item->_entry.path.wstring() + L"\n";
		}
		break;
	case ID_MENUITEM_COPY_ORIGINAL_SIZE:
		for (const auto &item : items) {
			info += Format(L"%I64d\n", item->_originalSize);
		}
		break;
	case ID_MENUITEM_COPY_FILETYPE:
		for (const auto &item : items) {
			info += m_ShellDataManager.GetTypeName(item->getExt().c_str()) + L"\n";
		}
		break;
	case ID_MENUITEM_COPY_FILETIME:
		for (const auto &item : items) {
			info += UtilFormatTime(item->_entry.stat.st_mtime) + L"\n";
		}
		break;
	case ID_MENUITEM_COPY_ATTRIBUTE:
	case ID_MENUITEM_COPY_COMPRESSED_SIZE:
	case ID_MENUITEM_COPY_METHOD:
	case ID_MENUITEM_COPY_COMPRESSION_RATIO:
	case ID_MENUITEM_COPY_CRC:
#pragma message("FIXME!")
		//TODO
		break;
	case ID_MENUITEM_COPY_ALL:
		info=L"FileName\tFullPath\tOriginalSize\tFileType\tFileTime\tAttribute\tCompressedSize\tMethod\tCompressionRatio\tCRC\n";
		for (const auto &item : items) {
			info += Format(L"%s\t%s\t%I64d\t%s\t%s\n",
				item->_entryName.c_str(),
				item->_entry.path.c_str(),
				item->_originalSize,
				m_ShellDataManager.GetTypeName(item->getExt().c_str()).c_str(),
				UtilFormatTime(item->_entry.stat.st_mtime).c_str());
		}
		break;
	default:
		ASSERT(!"Unknown command");
	}
	UtilSetTextOnClipboard(info);
}

void CFileListView::OnFindItem(UINT uNotifyCode,int nID,HWND hWndCtrl)
{
	if(ID_MENUITEM_FINDITEM_END==nID){
		mr_Model.EndFindItem();
	}else{
		CTextInputDialog dlg(CString(MAKEINTRESOURCE(IDS_INPUT_FIND_PARAM)));

		if(IDOK == dlg.DoModal()){
			mr_Model.EndFindItem();
			auto lpFound = mr_Model.FindItem(dlg.GetInputText(), mr_Model.getCurrentDir());
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
