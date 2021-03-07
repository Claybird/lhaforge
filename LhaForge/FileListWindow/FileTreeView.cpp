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
#include "FileListModel.h"
#include "Dialogs/LogListDialog.h"
#include "Utilities/StringUtil.h"
#include "Utilities/OSUtil.h"
#include "Utilities/Utility.h"
#include "CommonUtil.h"
#include "FileTreeView.h"

CFileTreeView::CFileTreeView(CFileListModel &rModel):
	m_bSelfAction(false),
	m_hDropHilight(NULL)
{
}

LRESULT CFileTreeView::OnCreate(LPCREATESTRUCT lpcs)
{
	LRESULT lRes=DefWindowProc();
	SetFont(AtlGetDefaultGuiFont());

	mr_Model.addEventListener(m_hWnd);

	m_ImageList.Create(::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CXSMICON), ILC_COLOR32 | ILC_MASK, 8, 1);
	SetImageList(m_ImageList,TVSIL_NORMAL);

	//directory icons
	//-close
	SHFILEINFO shfi;
	SHGetFileInfoW(L"dummy",FILE_ATTRIBUTE_DIRECTORY,&shfi,sizeof(shfi),SHGFI_USEFILEATTRIBUTES|SHGFI_ICON|SHGFI_SMALLICON);
	m_ImageList.AddIcon(shfi.hIcon);
	DestroyIcon(shfi.hIcon);
	//-open
	SHGetFileInfoW(L"dummy",FILE_ATTRIBUTE_DIRECTORY,&shfi,sizeof(shfi),SHGFI_USEFILEATTRIBUTES|SHGFI_ICON|SHGFI_SMALLICON|SHGFI_OPENICON);
	m_ImageList.AddIcon(shfi.hIcon);
	DestroyIcon(shfi.hIcon);

	return lRes;
}

bool CFileTreeView::UpdateCurrentNode()
{
	auto lpNode=mr_Model.getCurrentDir();
	const auto ite=m_TreeItemMap.find(lpNode);
	if(m_TreeItemMap.end()==ite){
		return false;
	}
	EnsureVisible((*ite).second);
	SelectItem((*ite).second);
	return true;
}


bool CFileTreeView::ConstructTree(HTREEITEM hParentItem, const ARCHIVE_ENTRY_INFO* lpNode)
{
	const int dirIconClosed = 0;
	const int dirIconOpened = 1;
	const int archiveIconIndex = 2;
	BeginSelfAction();
	HTREEITEM hItem;
	if(hParentItem){
		//directory
		hItem=InsertItem(lpNode->_entryName.c_str(),hParentItem,TVI_LAST);
		SetItemImage(hItem, dirIconClosed, dirIconOpened);
	} else {
		//---root
		//archive file icon
		m_ImageList.Remove(archiveIconIndex);	//remove old icon
		SHFILEINFO shfi;
		::SHGetFileInfoW(mr_Model.GetArchiveFileName(), 0, &shfi, sizeof(shfi), SHGFI_ICON | SHGFI_SMALLICON);
		m_ImageList.AddIcon(shfi.hIcon);

		lpNode = mr_Model.GetRootNode();
		hItem = InsertItem(PathFindFileName(mr_Model.GetArchiveFileName()), TVI_ROOT, TVI_LAST);
		SetItemImage(hItem, archiveIconIndex, archiveIconIndex);
	}

	m_TreeItemMap.insert({ lpNode,hItem });
	//set node pointer
	SetItemData(hItem,(DWORD_PTR)lpNode);

	//process children
	UINT numItems=lpNode->getNumChildren();
	for(UINT i=0;i<numItems;i++){
		const auto* lpChild=lpNode->getChild(i);
		if(lpChild->is_directory()){
			ConstructTree(hItem, lpChild);
		}
	}
	while (UtilDoMessageLoop())continue;
	return true;
}

LRESULT CFileTreeView::OnTreeSelect(LPNMHDR)
{
	if(IsSelfAction()){
		EndSelfAction();
	}else{
		auto lpCurrent=mr_Model.getCurrentDir();

		HTREEITEM hItem=GetSelectedItem();
		if(!hItem)return 0;

		auto lpNode=(ARCHIVE_ENTRY_INFO*)GetItemData(hItem);
		ASSERT(lpNode);

		if(!mr_Model.IsRoot())mr_Model.EndFindItem();
		if(lpNode && lpNode!=lpCurrent){
			mr_Model.setCurrentDir(lpNode);
		}
	}
	return 0;
}



void CFileTreeView::OnExtractItem(UINT,int nID,HWND)
{
	//アーカイブと同じフォルダに解凍する場合はtrue
	const bool bSameDir=(ID_MENUITEM_EXTRACT_SELECTED_SAMEDIR==nID);

	if(!mr_Model.IsOK()){
		return;// false;
	}

	//選択されたアイテムを列挙
	std::vector<ARCHIVE_ENTRY_INFO*> items;
	GetSelectedItems(items);
	if(items.empty()){
		//選択されたファイルがない
		ErrorMessage((const wchar_t*)CString(MAKEINTRESOURCE(IDS_ERROR_FILELIST_NOT_SELECTED)));
		return;// false;
	}

	TODO
	if (!bSameDir) {	//出力先をダイアログで選ばせる
		LFShellFileOpenDialog dlg(pathOutputBaseDir, FOS_FORCEFILESYSTEM | FOS_FILEMUSTEXIST | FOS_PATHMUSTEXIST | FOS_PICKFOLDERS);
		if (IDOK != dlg.DoModal()) {
			return S_FALSE;//E_ABORT;
		}
		dlg.GetFilePath(pathOutputBaseDir);
		pathOutputBaseDir.AddBackslash();
	}

	//解凍
	CString strLog;
	HRESULT hr=mr_Model.ExtractItems(m_hFrameWnd,bSameDir,items,(*items.begin())->_parent,strLog);

	SetForegroundWindow(m_hFrameWnd);

	if(FAILED(hr)){
		//TODO
		CLogListDialog LogDlg(L"Log");
		std::vector<ARCLOG> logs;
		logs.resize(1);
		logs.back().logs.resize(1);
		logs.back().logs.back().entryPath = mr_Model.GetArchiveFileName();
		logs.back().logs.back().message = strLog;
		LogDlg.SetLogArray(logs);
		LogDlg.DoModal(m_hFrameWnd);
	}
}

std::vector<const ARCHIVE_ENTRY_INFO*> CFileTreeView::GetSelectedItems()
{
	std::vector<const ARCHIVE_ENTRY_INFO*> items;
	HTREEITEM hItem = GetSelectedItem();
	if(hItem){
		const ARCHIVE_ENTRY_INFO* lpNode=(const ARCHIVE_ENTRY_INFO*)GetItemData(hItem);
		items.push_back(lpNode);
	}
	//always one item maximum
	ASSERT(items.size()<=1);
	return items;
}

