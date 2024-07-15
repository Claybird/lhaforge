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
#include "FileListTabClient.h"
#include "FileTreeView.h"
#include "ConfigCode/ConfigFileListWindow.h"
#include "FileListFrame.h"
#include "Dialogs/WaitDialog.h"

CFileListTabClient::CFileListTabClient(
	const CConfigFileListWindow& confFLW,
	const LF_COMPRESS_ARGS& compressArgs,
	CFileListFrame& rFrame
):
	m_confFLW(confFLW),
	mr_compressArgs(compressArgs),
	m_rFrameWnd(rFrame),
	m_bShowTab(true)
{}

BOOL CFileListTabClient::PreTranslateMessage(MSG* pMsg)
{
	CFileListTabItem* pItem = GetCurrentTab();
	if(pItem){
		if(pItem->ListView.PreTranslateMessage(pMsg))return TRUE;
		if(pItem->TreeView.PreTranslateMessage(pMsg))return TRUE;
	}
	return CTabView::PreTranslateMessage(pMsg);
}

HRESULT CFileListTabClient::OpenArchiveInTab(const std::filesystem::path& arcpath, const std::wstring& mutexName, HANDLE hMutex, ARCLOG &arcLog)
{
	int idx = CreateNewTab();
	ASSERT(idx >= 0);
	if (idx < 0)return E_HANDLE;
	CFileListTabItem* pItem = (CFileListTabItem*)GetPageData(idx);
	ASSERT(pItem);
	if (!pItem)return E_HANDLE;

	if (!pItem->OpenArchive(arcpath, mutexName, hMutex)) {
		RemoveTab(idx);
		return E_FAIL;
	}

	SetPageTitle(idx, pItem->Model.GetArchiveFileName().filename().c_str());
	dispatchEvent(WM_FILELIST_MODELCHANGED);
	dispatchEvent(WM_FILELIST_WND_STATE_CHANGED);

	UpdateClientArea();
	return S_OK;
}

void CFileListTabClient::ShowTabCtrl(bool bShow)
{
	m_bShowTab=bShow;
	ShowTabControl(bShow);
	UpdateClientArea();
}

void CFileListTabClient::UpdateClientArea()
{
	if(!m_bShowTab){
		//expand to full client if tab is disabled
		CRect rcTab;
		m_tab.GetWindowRect(rcTab);

		CRect rc;
		m_rFrameWnd.GetFreeClientRect(rc);
		rc.top-=rcTab.Height();
		MoveWindow(rc);
	}
}

int CFileListTabClient::CreateNewTab()
{
	auto p = std::make_shared<CFileListTabItem>(m_confFLW, mr_compressArgs);
	auto pItem = p.get();
	if(!pItem->CreateTabItem(m_hWnd,m_rFrameWnd)){
		return -1;
	}
	m_GC.push_back(p);

	//--save current
	OnDeactivatingTab(GetActivePage());

	int prevCount = GetPageCount();
	//--add tab
	AddPage(pItem->Splitter,L"",-1,pItem);

	UpdateClientArea();

	return prevCount;
}

void CFileListTabClient::RemoveTab(int idx)
{
	if(idx<0)return;
	CFileListTabItem* pItem=(CFileListTabItem*)GetPageData(idx);

	if (idx == GetActivePage()) {
		OnDeactivatingTab(idx);
	}
	RemovePage(idx);
	remove_item_if(m_GC, [&](std::shared_ptr<CFileListTabItem>& item) {return item.get() == pItem; });

	OnActivateTab(GetActivePage());

	dispatchEvent(WM_FILELIST_WND_STATE_CHANGED);
}

void CFileListTabClient::RemoveTabExcept(int idx)
{
	if (idx != GetActivePage()) {
		OnDeactivatingTab(GetActivePage());
	}

	int size=GetPageCount();
	for(int i=size-1;i>idx;i--){
		CFileListTabItem* pItem=(CFileListTabItem*)GetPageData(i);
		RemovePage(i);
		remove_item_if(m_GC, [&](std::shared_ptr<CFileListTabItem>& item) {return item.get() == pItem; });
	}

	for(int i=0;i<idx;i++){
		CFileListTabItem* pItem=(CFileListTabItem*)GetPageData(i);
		RemovePage(i);
		remove_item_if(m_GC, [&](std::shared_ptr<CFileListTabItem>& item) {return item.get() == pItem; });
	}

	dispatchEvent(WM_FILELIST_WND_STATE_CHANGED);
}

CFileListTabItem* CFileListTabClient::GetCurrentTab()
{
	int currentIdx=GetActivePage();
	if(currentIdx>=0){
		return (CFileListTabItem*)GetPageData(currentIdx);
	}else{
		return NULL;
	}
}

void CFileListTabClient::ClearAllTabs()
{
	if(IsWindow())RemoveAllPages();
	m_GC.clear();
}

void CFileListTabClient::OnSize(UINT uType, CSize &size)
{
	UpdateLayout();
	UpdateClientArea();
	SetMsgHandled(false);
}

void CFileListTabClient::StoreSettings(CConfigFileListWindow &ConfFLW)
{
	CFileListTabItem* pItem=GetCurrentTab();
	if(pItem){
		pItem->StoreSettings(ConfFLW);
	}
}

HRESULT CFileListTabClient::ReopenArchiveFile(int nPage)
{
	if(GetPageCount()<=0)return S_FALSE;
	if(nPage==-1)nPage=GetActivePage();
	CFileListTabItem* pItem=(CFileListTabItem*)GetPageData(nPage);
	if(pItem){
		pItem->TreeView.DeleteAllItems();
		pItem->ListView.DeleteAllItems();
		pItem->ListView.SetItemCount(0);

		//store current directory
		auto currentDir = pItem->Model.getCurrentDirPath();

		try {
			pItem->OpenArchive(pItem->Model.GetArchiveFileName(), L"", nullptr);
		} catch (const LF_EXCEPTION& e) {
			ErrorMessage(e.what());
			return E_FAIL;
		} catch (const std::exception &e) {
			ErrorMessage(UtilUTF8toUNICODE(e.what()));
			return E_FAIL;
		}
		//restore
		pItem->Model.setCurrentDirPath(currentDir);
	}
	return S_OK;
}

bool CFileListTabClient::ReopenArchiveFileAll()
{
	int size=GetPageCount();
	for(int i=0;i<size;i++){
		ReopenArchiveFile(i);
	}
	return true;
}

DWORD CFileListTabClient::GetListViewStyle()
{
	CFileListTabItem* pItem=GetCurrentTab();
	if(pItem){
		return pItem->GetListViewStyle();
	}else return 0;
}

void CFileListTabClient::SetListViewStyle(DWORD dwStyle)
{
	CFileListTabItem* pItem=GetCurrentTab();
	if(pItem){
		pItem->SetListViewStyle(dwStyle);
	}
}

void CFileListTabClient::OnDeactivatingTab(int page)
{
	if (page >= 0) {
		auto pItem = (CFileListTabItem*)GetPageData(page);
		if (pItem) {
			pItem->OnDeactivating();
			pItem->ShowWindow(SW_HIDE);
		}
	}
}

void CFileListTabClient::OnActivateTab(int page)
{
	if(page>=0){
		UpdateLayout();

		auto pItem = (CFileListTabItem*)GetPageData(page);
		if (pItem) {
			pItem->ShowWindow(SW_SHOW);
			pItem->OnActivated();

			UpdateClientArea();
		}

		dispatchEvent(WM_FILELIST_MODELCHANGED);
		dispatchEvent(WM_FILELIST_WND_STATE_CHANGED);
	}
}

void CFileListTabClient::SetActivePage(HANDLE hHandle)
{
	int size=GetPageCount();
	for(int i=0;i<size;i++){
		CFileListTabItem* pData=(CFileListTabItem*)GetPageData(i);
		if(pData==(CFileListTabItem*)hHandle){
			SetActivePage(i);
			break;
		}
	}
}

LRESULT CFileListTabClient::OnContextMenu(LPNMHDR pnmh)
{
	int idx = (int)pnmh->idFrom;
	OnDeactivatingTab(GetActivePage());
	SetActivePage(idx);
	OnActivateTab(idx);

	CMenu cMenu;
	cMenu.LoadMenu(IDM_TABMENU);
	CMenuHandle cSubMenu(cMenu.GetSubMenu(0));

	CFileListTabItem* pItem=(CFileListTabItem*)GetPageData(idx);

	TBVCONTEXTMENUINFO* pInfo=(TBVCONTEXTMENUINFO*)pnmh;

	UINT nCmd = (UINT)cSubMenu.TrackPopupMenu(TPM_RETURNCMD | TPM_LEFTALIGN,pInfo->pt.x,pInfo->pt.y,m_hWnd);	//メニュー表示
	switch(nCmd){
	case ID_TABMENU_CLOSE:
		RemoveTab(idx);
		break;
	case ID_TABMENU_CLOSEOTHERS:
		RemoveTabExcept(idx);
		break;
	case ID_TABMENU_RELOAD:
		ReopenArchiveFile(idx);
		break;
	case ID_TABMENU_RELOADALL:
		ReopenArchiveFileAll();
		break;
	case 0:
		//canceled
		break;
	default:
		//processed by owner window
		SendMessage(WM_COMMAND,MAKEWPARAM(nCmd,0),NULL);
		break;
	}

	return 0;
}

LRESULT CFileListTabClient::OnTabCloseBtn(LPNMHDR pnmh)
{
	int idx = (int)pnmh->idFrom;
	if (idx < 0)return 0;
	CFileListTabItem* pItem = (CFileListTabItem*)GetPageData(idx);

	if (idx == GetActivePage()) {
		OnDeactivatingTab(idx);
	}
	//RemovePage(idx);
	remove_item_if(m_GC, [&](std::shared_ptr<CFileListTabItem>& item) {return item.get() == pItem; });

	dispatchEvent(WM_FILELIST_WND_STATE_CHANGED);
	return 0;	//let CTabView to call RemovePage()
}

void CFileListTabClient::OnMButtonUp(UINT, CPoint &pt)
{
	TCHITTESTINFO hti={0};
	hti.pt=pt;
	int nItem = m_tab.HitTest(&hti);
	if(nItem != -1){
		RemoveTab(nItem);
	}
}

void CFileListTabClient::OnExtractArchive(UINT,int nID,HWND)
{
	CFileListTabItem* pItem=GetCurrentTab();
	if(pItem){
		::EnableWindow(m_rFrameWnd,FALSE);

		bool bRet=pItem->Model.ExtractArchive(CLFProgressHandlerGUI(m_hWnd));

		::EnableWindow(m_rFrameWnd,TRUE);
		SetForegroundWindow(m_rFrameWnd);
	}
}

void CFileListTabClient::OnTestArchive(UINT,int,HWND)
{
	CFileListTabItem* pItem=GetCurrentTab();
	if(pItem){
		::EnableWindow(m_rFrameWnd,FALSE);

		pItem->Model.TestArchive(CLFProgressHandlerGUI(m_hWnd));

		::EnableWindow(m_rFrameWnd,TRUE);
		SetForegroundWindow(m_rFrameWnd);
	}
}

void CFileListTabClient::OnSortItemMenu(UINT uNotifyCode,int nID,HWND hWndCtrl)
{
	int iCol=nID-ID_MENUITEM_SORT_FILENAME;
	CFileListTabItem* pItem=GetCurrentTab();
	if(pItem){
		pItem->SetSortColumn(iCol);
	}
}


void CFileListTabClient::OnToggleTreeView(UINT,int,HWND)
{
	CFileListTabItem* pItem=GetCurrentTab();
	if(pItem){
		pItem->ShowTreeView(!pItem->IsTreeViewVisible());
	}
}

void CFileListTabClient::UpdateFileListConfig(const CConfigFileListWindow& ConfFLW)
{
	int size=GetPageCount();
	for(int i=0;i<size;i++){
		CFileListTabItem* pItem=(CFileListTabItem*)GetPageData(i);
		if(pItem){
			pItem->ApplyUpdatedConfig();
		}
	}
}
