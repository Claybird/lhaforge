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
#include "../ConfigCode/ConfigFileListWindow.h"
#include "resource.h"
#include "FileListFrame.h"
#include "Dialogs/WaitDialog.h"

CFileListTabClient::CFileListTabClient(
	const CConfigFile& rConfig,
	const CConfigFileListWindow& confFLW,
	CFileListFrame& rFrame
):
	mr_Config(rConfig),
	m_confFLW(confFLW),
	m_rFrameWnd(rFrame),
	m_lpPrevTab(NULL),
	m_bShowTab(true)
{
	m_ColumnIndexArray = m_confFLW.ColumnOrderArray;
	memcpy(m_FileInfoWidth, m_confFLW.ColumnWidthArray, sizeof(m_FileInfoWidth));

	if (m_confFLW.StoreSetting) {
		m_nTreeWidth = m_confFLW.TreeWidth;
		m_bSortDescending = m_confFLW.SortDescending;
		m_nSortKeyType = m_confFLW.SortColumn;
		m_dwListStyle = m_confFLW.ListStyle;
		m_bShowTreeView = m_confFLW.ShowTreeView;
	} else {
		m_nTreeWidth = FILELISTWINDOW_DEFAULT_TREE_WIDTH;
		m_bSortDescending = true;
		m_nSortKeyType = -1;
		m_dwListStyle = LVS_ICON;
		m_bShowTreeView = true;
	}
}

BOOL CFileListTabClient::PreTranslateMessage(MSG* pMsg)
{
	CFileListTabItem* pItem=GetCurrentTab();
	if(pItem){
		if(pItem->ListView.PreTranslateMessage(pMsg))return TRUE;
		if(pItem->TreeView.PreTranslateMessage(pMsg))return TRUE;
	}
	return CTabView::PreTranslateMessage(pMsg);
}

LRESULT CFileListTabClient::OnDestroy()
{
	ClearAllTabs();
	return 0;
}

HRESULT CFileListTabClient::OpenArchiveInTab(const std::filesystem::path& arcpath, const std::wstring& mutexName, HANDLE hMutex, ARCLOG &arcLog)
{
	int idx = CreateNewTab();
	ASSERT(idx >= 0);
	if (idx < 0)return E_HANDLE;
	CFileListTabItem* pItem = (CFileListTabItem*)GetPageData(idx);
	ASSERT(pItem);
	if (!pItem)return E_HANDLE;

	//prepare duplicated window
	pItem->hMutex.Attach(hMutex);
	pItem->strMutexName = mutexName;

	if(!pItem->OpenArchive(arcpath))RemoveTab(idx);

	SetPageTitle(idx, pItem->Model.GetArchiveFileName().filename().c_str());
	// focus on treeview
	pItem->TreeView.SetFocus();
	pItem->ShowTreeView(m_bShowTreeView);
	dispatchEvent(WM_FILELIST_MODELCHANGED);
	dispatchEvent(WM_FILELIST_WND_STATE_CHANGED);

	//set property to frame window
	::SetPropW(m_rFrameWnd, mutexName.c_str(), pItem);

	FitClient();
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
	//タブ非表示の場合にウィンドウサイズをクライアント一杯に広げる
	if(!m_bShowTab){
		CRect rcTab;
		m_tab.GetWindowRect(rcTab);

		CRect rc;
		m_rFrameWnd.GetFreeClientRect(rc);
		rc.top-=rcTab.Height();
		MoveWindow(rc);
	}
}

void CFileListTabClient::FitClient()
{
//	UpdateLayout();
	UpdateClientArea();
	int idx=GetActivePage();
	if(idx>=0){
		CFileListTabItem* pTab=(CFileListTabItem*)GetPageData(idx);
		ASSERT(pTab);
		if(pTab){
			//TODO:dirty hack
			//これを行うと、リストビューのスクロールバーが復活する
			//pTab->Splitter.SetRedraw(FALSE);
			//SetRedraw(FALSE);
			int pos=pTab->Splitter.GetSplitterPos();
			pTab->Splitter.SetSplitterPos(pos+1);
			pTab->Splitter.SetSplitterPos(pos);
		}
	}
}

int CFileListTabClient::CreateNewTab()
{
	//--インスタンス確保
	auto p = std::make_shared<CFileListTabItem>(mr_Config, m_confFLW);
	auto pItem = p.get();
	if(!pItem->CreateTabItem(m_hWnd,m_rFrameWnd)){
		return -1;
	}
	m_GC.push_back(p);
	int idx=GetPageCount();

	//--現状保存
	if(idx>0){
		OnDeactivateTab((CFileListTabItem*)GetPageData(GetActivePage()));
	}

	//--タブ追加
	AddPage(pItem->Splitter,_T(""),-1,pItem);

	FitClient();

	return idx;
}

void CFileListTabClient::RemoveTab(int idx)
{
	if(idx<0)return;
	CFileListTabItem* pItem=(CFileListTabItem*)GetPageData(idx);

	//フレームウィンドウのプロパティとして登録
	::RemoveProp(m_rFrameWnd,pItem->strMutexName.c_str());

	if(idx==GetActivePage()){
		OnDeactivateTab(pItem);
	}
	m_lpPrevTab=NULL;

	RemovePage(idx);
	remove_item_if(m_GC, [&](std::shared_ptr<CFileListTabItem>& item) {return item.get() == pItem; });

	if(GetPageCount()>0){
		OnActivateTab(GetActivePage());
	}
	dispatchEvent(WM_FILELIST_WND_STATE_CHANGED);
}

void CFileListTabClient::RemoveTabExcept(int idx)
{
	if(idx!=GetActivePage()){
		CFileListTabItem* pItem=(CFileListTabItem*)GetPageData(GetActivePage());
		OnDeactivateTab(pItem);
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

void CFileListTabClient::OnActivateTab(int newIdx)
{
	UpdateLayout();

	CFileListTabItem* pItem=(CFileListTabItem*)GetPageData(newIdx);
	if(pItem){
		pItem->ShowWindow(SW_SHOW);
		pItem->ListView.SetColumnState(&m_ColumnIndexArray[0], m_FileInfoWidth);

		pItem->SetTreeWidth(m_nTreeWidth);
		pItem->Model.SetSortMode(m_bSortDescending);
		pItem->Model.SetSortKeyType(m_nSortKeyType);
		pItem->SetListViewStyle(m_dwListStyle);

		SetActivePage(newIdx);
		//m_tab.SetCurFocus(newIdx);
		pItem->OnActivated();
		pItem->ShowTreeView(m_bShowTreeView);
		m_lpPrevTab=pItem;

		FitClient();
	}

	dispatchEvent(WM_FILELIST_MODELCHANGED);
	dispatchEvent(WM_FILELIST_WND_STATE_CHANGED);
}

void CFileListTabClient::GetTabSettingsToClient(CFileListTabItem* pItem)
{
	if(pItem){
		pItem->ListView.GetColumnState(&m_ColumnIndexArray[0], m_FileInfoWidth);
		m_nTreeWidth	 =pItem->GetTreeWidth();
		m_bSortDescending=pItem->Model.GetSortMode();
		m_nSortKeyType	 =pItem->Model.GetSortKeyType();

		m_dwListStyle	 =pItem->GetListViewStyle()%(0x0004);
	}
}

void CFileListTabClient::OnDeactivateTab(CFileListTabItem* pItem)
{
	if(pItem){
		GetTabSettingsToClient(pItem);
		pItem->ShowWindow(SW_HIDE);
		pItem->OnDeactivated();
	}
}

void CFileListTabClient::ClearAllTabs()
{
	m_lpPrevTab=NULL;
	if(IsWindow())RemoveAllPages();
	m_GC.clear();
}

void CFileListTabClient::OnSize(UINT uType, CSize &size)
{
	UpdateLayout();
	FitClient();
	SetMsgHandled(false);
}

LRESULT CFileListTabClient::OnWndStateChanged(LPNMHDR)
{
	dispatchEvent(WM_FILELIST_WND_STATE_CHANGED);
	//::PostMessage(GetParent(),WM_FILELIST_WND_STATE_CHANGED,0,0);
	return 0;
}

void CFileListTabClient::StoreSettings(CConfigFileListWindow &ConfFLW)
{
	CFileListTabItem* pItem=GetCurrentTab();
	if(pItem){
		GetTabSettingsToClient(pItem);
	}

	ConfFLW.TreeWidth=m_nTreeWidth;
	//ソート設定
	ConfFLW.SortColumn=m_nSortKeyType;
	ConfFLW.SortDescending=m_bSortDescending;

	//リストビューのスタイル
	if(GetPageCount()>0){
		m_dwListStyle=GetCurrentTab()->GetListViewStyle()%(0x0004);
	}
	ConfFLW.ListStyle=m_dwListStyle;

	//カラムの並び順
	ConfFLW.ColumnOrderArray = m_ColumnIndexArray;
	//カラムの幅
	memcpy(ConfFLW.ColumnWidthArray, m_FileInfoWidth, sizeof(m_FileInfoWidth));

	//ツリービュー
	ConfFLW.ShowTreeView=m_bShowTreeView;
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

		//カレントディレクトリの保存と復元を行う
		auto currentDir = pItem->Model.getCurrentDirPath();

		//---解析
		auto path = pItem->Model.GetArchiveFileName();
		try {
			pItem->OpenArchive(path);
		} catch (const LF_EXCEPTION& e) {
			ErrorMessage(e.what());
			return E_FAIL;
		} catch (const std::exception &e) {
			ErrorMessage(UtilUTF8toUNICODE(e.what()));
			return E_FAIL;
		}
		pItem->Model.setCurrentDirPath(currentDir);

		// ツリービューにフォーカスを持たせる
//		m_lpCurrentTab->TreeView.SetFocus();
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


LRESULT CFileListTabClient::OnTabSelChanging(LPNMHDR pnmh)
{
	int sel=GetActivePage();
	if(sel>=0){
		CFileListTabItem* pPrevItem=(CFileListTabItem*)GetPageData(sel);
		//if(pPrevItem){
		//	OnDeactivateTab(pPrevItem);
		//}
		m_lpPrevTab=pPrevItem;
	}
	SetMsgHandled(FALSE);
	return 0;
}

LRESULT CFileListTabClient::OnTabSelChanged(LPNMHDR pnmh)
{
	if(m_lpPrevTab)OnDeactivateTab(m_lpPrevTab);
	int sel=GetActivePage();//pnmh->idFrom;
	if(sel>=0){
		//SetActivePage(sel);
		CFileListTabItem* pItem=(CFileListTabItem*)GetPageData(sel);
		OnActivateTab(sel);
	}
	SetMsgHandled(FALSE);
	return 0;
}

void CFileListTabClient::SetCurrentTab(int idx)
{
	if(GetActivePage()>=0)OnDeactivateTab((CFileListTabItem*)GetPageData(GetActivePage()));
	SetActivePage(idx);
	CFileListTabItem* pItem=(CFileListTabItem*)GetPageData(idx);
	OnActivateTab(idx);
}

void CFileListTabClient::SetCurrentTab(HANDLE hHandle)
{
	int size=GetPageCount();
	for(int i=0;i<size;i++){
		CFileListTabItem* pData=(CFileListTabItem*)GetPageData(i);
		if(pData==(CFileListTabItem*)hHandle){
			SetCurrentTab(i);
			break;
		}
	}
}


//コンテキストメニューを開く
LRESULT CFileListTabClient::OnContextMenu(LPNMHDR pnmh)
{
	int idx = pnmh->idFrom;
	SetCurrentTab(idx);

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
		//コマンドを他に投げる
		SendMessage(WM_COMMAND,MAKEWPARAM(nCmd,0),NULL);
		break;
	}

	return 0;
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
		//ウィンドウを使用不可に
		::EnableWindow(m_rFrameWnd,FALSE);

		bool bRet=pItem->Model.ExtractArchive(CLFProgressHandlerGUI(m_hWnd));

		//ウィンドウを使用可能に
		::EnableWindow(m_rFrameWnd,TRUE);
		SetForegroundWindow(m_rFrameWnd);

		if(bRet && nID==ID_MENUITEM_EXTRACT_ARCHIVE_AND_CLOSE){
			CloseCurrentTab();
		}
	}
}


void CFileListTabClient::OnExtractAll(UINT,int nID,HWND)
{
	//ウィンドウを使用不可に
	::EnableWindow(m_rFrameWnd,FALSE);

	int tabIndex=0;

	for(;tabIndex<GetPageCount();){
		SetCurrentTab(tabIndex);
		CFileListTabItem* pItem=GetCurrentTab();
		if(pItem){
			bool bRet=pItem->Model.ExtractArchive(CLFProgressHandlerGUI(m_hWnd));
			if(!bRet){
				break;
			}

			if(nID==ID_MENUITEM_EXTRACT_ARCHIVE_AND_CLOSE_ALL){
				CloseCurrentTab();
			}else{
				tabIndex++;
			}
		}else{
			break;
		}
	}
	if(GetPageCount()>0){
		SetCurrentTab(0);
	}
	//ウィンドウを使用可能に
	::EnableWindow(m_rFrameWnd,TRUE);
	SetForegroundWindow(m_rFrameWnd);
}

void CFileListTabClient::OnTestArchive(UINT,int,HWND)
{
	CFileListTabItem* pItem=GetCurrentTab();
	if(pItem){
		//ウィンドウを使用不可に
		::EnableWindow(m_rFrameWnd,FALSE);

		pItem->Model.TestArchive(CLFProgressHandlerGUI(m_hWnd));

		//ウィンドウを使用可能に
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
		m_bShowTreeView=!m_bShowTreeView;
		pItem->ShowTreeView(m_bShowTreeView);
	}
}

void CFileListTabClient::UpdateFileListConfig(const CConfigFileListWindow& ConfFLW)
{
	int size=GetPageCount();
	for(int i=0;i<size;i++){
		CFileListTabItem* pItem=(CFileListTabItem*)GetPageData(i);
		if(pItem){
			pItem->UpdateFileListConfig();
		}
	}
}
