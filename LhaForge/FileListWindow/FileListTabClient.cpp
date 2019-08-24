/*
 * Copyright (c) 2005-, Claybird
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
#include "FileListTabClient.h"
#include "FileTreeView.h"
#include "../ConfigCode/ConfigFileListWindow.h"
#include "../resource.h"
#include "AnimationUpdater.h"
#include "FileListFrame.h"

CFileListTabClient::CFileListTabClient(CConfigManager& rConfig,CFileListFrame& rFrame):
	m_rConfig(rConfig),
	m_rFrameWnd(rFrame),
	m_lpPrevTab(NULL),
	m_bShowTab(true)
{
	CConfigFileListWindow ConfFLW;
	ConfFLW.load(rConfig);
	ASSERT(sizeof(m_ColumnIndexArray)==sizeof(ConfFLW.ColumnOrderArray));
	memcpy(m_ColumnIndexArray, ConfFLW.ColumnOrderArray, sizeof(ConfFLW.ColumnOrderArray));
	memcpy(m_FileInfoWidth, ConfFLW.ColumnWidthArray, sizeof(m_FileInfoWidth));

	if(ConfFLW.StoreSetting){
		m_nTreeWidth=ConfFLW.TreeWidth;
		m_bSortDescending=BOOL2bool(ConfFLW.SortDescending);
		m_nSortKeyType=ConfFLW.SortColumn;
		m_dwListStyle=ConfFLW.ListStyle;
		m_ListMode=ConfFLW.FileListMode;
		m_bShowTreeView=BOOL2bool(ConfFLW.ShowTreeView);
	}else{
		m_nTreeWidth=FILELISTWINDOW_DEFAULT_TREE_WIDTH;
		m_bSortDescending=true;
		m_nSortKeyType=-1;
		m_dwListStyle=LVS_ICON;
		m_ListMode=FILELIST_TREE;
		m_bShowTreeView=true;
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

HRESULT CFileListTabClient::OpenArchiveInTab(LPCTSTR lpszArc,DLL_ID forceID,const CConfigFileListWindow& ConfFLW,LPCTSTR lpMutexName,HANDLE hMutex,CString &strErr)
{
	int idx=CreateNewTab(ConfFLW);
	ASSERT(idx>=0);
	if(idx<0)return E_HANDLE;
	CFileListTabItem* pItem=(CFileListTabItem*)GetPageData(idx);
	ASSERT(pItem);
	if(!pItem)return E_HANDLE;

	//�d���I�[�v���h�~�I�u�W�F�N�g���Z�b�g
	pItem->hMutex=hMutex;
	pItem->strMutexName=lpMutexName;

	//�u���҂����������v�_�C�A���O��\��
	CWaitDialog WaitDialog;
	WaitDialog.Prepare(m_rFrameWnd,CString(MAKEINTRESOURCE(IDS_FILELIST_SEARCH)));
	AnimationUpdater au;
	au.lpDialog=&WaitDialog;

	m_rFrameWnd.EnableWindow(FALSE);
	//---���
	HRESULT hr=pItem->OpenArchive(lpszArc,forceID,ConfFLW,ConfFLW.FileListMode,&au,strErr);
	if(FAILED(hr)){
		m_rFrameWnd.EnableWindow(TRUE);
		WaitDialog.DestroyWindow();
		RemoveTab(idx);
	}else{
		m_rFrameWnd.EnableWindow(TRUE);
		WaitDialog.DestroyWindow();
		SetPageTitle(idx,PathFindFileName(pItem->Model.GetArchiveFileName()));
		// �c���[�r���[�Ƀt�H�[�J�X����������
		pItem->TreeView.SetFocus();
		pItem->ShowTreeView(m_bShowTreeView);
		dispatchEvent(WM_FILELIST_MODELCHANGED);
		dispatchEvent(WM_FILELIST_WND_STATE_CHANGED);

		//�t���[���E�B���h�E�̃v���p�e�B�Ƃ��ēo�^
		::SetProp(m_rFrameWnd,lpMutexName,pItem);

		FitClient();
	}
	return hr;
}

void CFileListTabClient::ShowTabCtrl(bool bShow)
{
	m_bShowTab=bShow;
	ShowTabControl(bShow);
	UpdateClientArea();
}

void CFileListTabClient::UpdateClientArea()
{
	//�^�u��\���̏ꍇ�ɃE�B���h�E�T�C�Y���N���C�A���g��t�ɍL����
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
			//������s���ƁA���X�g�r���[�̃X�N���[���o�[����������
			//pTab->Splitter.SetRedraw(FALSE);
			//SetRedraw(FALSE);
			int pos=pTab->Splitter.GetSplitterPos();
			pTab->Splitter.SetSplitterPos(pos+1);
			pTab->Splitter.SetSplitterPos(pos);
		}
	}
}

int CFileListTabClient::CreateNewTab(const CConfigFileListWindow& ConfFLW)
{
	//--�C���X�^���X�m��
	CFileListTabItem* pItem=new CFileListTabItem(m_rConfig);
	m_GC.Add(pItem);
	if(!pItem->CreateTabItem(m_hWnd,m_rFrameWnd,ConfFLW)){
		m_GC.Delete(pItem);
		return -1;
	}
	int idx=GetPageCount();

	//--����ۑ�
	if(idx>0){
		OnDeactivateTab((CFileListTabItem*)GetPageData(GetActivePage()));
	}

	//--�^�u�ǉ�
	AddPage(pItem->Splitter,_T(""),-1,pItem);

	FitClient();

	return idx;
}

void CFileListTabClient::RemoveTab(int idx)
{
	if(idx<0)return;
	CFileListTabItem* pItem=(CFileListTabItem*)GetPageData(idx);

	//�t���[���E�B���h�E�̃v���p�e�B�Ƃ��ēo�^
	::RemoveProp(m_rFrameWnd,pItem->strMutexName);

	if(idx==GetActivePage()){
		OnDeactivateTab(pItem);
	}
	m_lpPrevTab=NULL;

	RemovePage(idx);
	m_GC.Delete(pItem);

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
		m_GC.Delete(pItem);
	}

	for(int i=0;i<idx;i++){
		CFileListTabItem* pItem=(CFileListTabItem*)GetPageData(i);
		RemovePage(i);
		m_GC.Delete(pItem);
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
		pItem->ListView.SetColumnState(m_ColumnIndexArray, m_FileInfoWidth);

		pItem->SetTreeWidth(m_nTreeWidth);
		pItem->Model.SetSortMode(m_bSortDescending);
		pItem->Model.SetSortKeyType(m_nSortKeyType);
		pItem->SetListViewStyle(m_dwListStyle);

		SetActivePage(newIdx);
		//m_tab.SetCurFocus(newIdx);
		pItem->OnActivated();
		pItem->ShowTreeView(m_bShowTreeView);
		m_lpPrevTab=pItem;

		m_ListMode=pItem->Model.GetListMode();

		FitClient();
	}

	dispatchEvent(WM_FILELIST_MODELCHANGED);
	dispatchEvent(WM_FILELIST_WND_STATE_CHANGED);
}

void CFileListTabClient::GetTabSettingsToClient(CFileListTabItem* pItem)
{
	if(pItem){
		pItem->ListView.GetColumnState(m_ColumnIndexArray, m_FileInfoWidth);
		m_nTreeWidth	 =pItem->GetTreeWidth();
		m_bSortDescending=pItem->Model.GetSortMode();
		m_nSortKeyType	 =pItem->Model.GetSortKeyType();

		m_dwListStyle	 =pItem->GetListViewStyle()%(0x0004);
		m_ListMode		 =pItem->Model.GetListMode();
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
	m_GC.DeleteAll();
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
	//�\�[�g�ݒ�
	ConfFLW.SortColumn=m_nSortKeyType;
	ConfFLW.SortDescending=m_bSortDescending;

	//���X�g�r���[�̃X�^�C��
	if(GetPageCount()>0){
		m_dwListStyle=GetCurrentTab()->GetListViewStyle()%(0x0004);
	}
	ConfFLW.ListStyle=m_dwListStyle;

	ConfFLW.FileListMode=m_ListMode;
	//�J�����̕��я�
	memcpy(ConfFLW.ColumnOrderArray, m_ColumnIndexArray, sizeof(m_ColumnIndexArray));
	//�J�����̕�
	memcpy(ConfFLW.ColumnWidthArray, m_FileInfoWidth, sizeof(m_FileInfoWidth));

	//�c���[�r���[
	ConfFLW.ShowTreeView=m_bShowTreeView;
}

HRESULT CFileListTabClient::ReopenArchiveFile(FILELISTMODE flMode,int nPage)
{
	if(GetPageCount()<=0)return S_FALSE;
	if(nPage==-1)nPage=GetActivePage();
	CFileListTabItem* pItem=(CFileListTabItem*)GetPageData(nPage);
	if(pItem){
		pItem->TreeView.DeleteAllItems();
		pItem->ListView.DeleteAllItems();
		pItem->ListView.SetItemCount(0);

		//�J�����g�f�B���N�g���̕ۑ��ƕ������s��
		std::stack<CString> dirStack;
		pItem->Model.GetDirStack(dirStack);

		//�u���҂����������v�_�C�A���O��\��
		CWaitDialog WaitDialog;
		WaitDialog.Prepare(m_rFrameWnd,CString(MAKEINTRESOURCE(IDS_FILELIST_SEARCH)));
		AnimationUpdater au;
		au.lpDialog=&WaitDialog;

		m_rFrameWnd.EnableWindow(FALSE);
		//---���
		CString strErr;
		HRESULT hr=pItem->Model.ReopenArchiveFile(flMode,strErr,&au);
		m_rFrameWnd.EnableWindow(TRUE);
		WaitDialog.DestroyWindow();
		if(FAILED(hr)){
			ErrorMessage(strErr);
			return hr;
		}
		pItem->Model.SetDirStack(dirStack);

		// �c���[�r���[�Ƀt�H�[�J�X����������
//		m_lpCurrentTab->TreeView.SetFocus();
	}
	return S_OK;
}

bool CFileListTabClient::ReopenArchiveFileAll()
{
	//�u���҂����������v�_�C�A���O��\��
	CWaitDialog WaitDialog;
	WaitDialog.Prepare(m_rFrameWnd,CString(MAKEINTRESOURCE(IDS_FILELIST_SEARCH)));
	AnimationUpdater au;
	au.lpDialog=&WaitDialog;

	m_rFrameWnd.EnableWindow(FALSE);
	int size=GetPageCount();
	for(int i=0;i<size;i++){
		CFileListTabItem* pItem=(CFileListTabItem*)GetPageData(i);
		if(pItem){
			pItem->ListView.DeleteAllItems();
			pItem->ListView.SetItemCount(0);

			//�J�����g�f�B���N�g���̕ۑ��ƕ������s��
			std::stack<CString> dirStack;
			pItem->Model.GetDirStack(dirStack);

			//---���
			CString strErr;
			HRESULT hr=pItem->Model.ReopenArchiveFile(m_ListMode,strErr,&au);
			if(FAILED(hr)){
				if(hr==E_ABORT){
					m_rFrameWnd.EnableWindow(TRUE);
					WaitDialog.DestroyWindow();
					return false;
				}
				//WaitDialog.DestroyWindow();
				//return false;
				continue;
			}
			pItem->Model.SetDirStack(dirStack);
		}
	}
	m_rFrameWnd.EnableWindow(TRUE);
	WaitDialog.DestroyWindow();
	return true;
}

FILELISTMODE CFileListTabClient::GetFileListMode()
{
	CFileListTabItem* pItem=GetCurrentTab();
	if(pItem){
		return pItem->Model.GetListMode();
	}else return FILELIST_TREE;
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

void CFileListTabClient::ReloadArchiverIfLost()
{
	CString strErr;
	size_t count=GetPageCount();
	for(size_t idx=0;idx<count;idx++){
		CFileListTabItem* pItem=(CFileListTabItem*)GetPageData(idx);
		ASSERT(pItem);
		pItem->Model.ReloadArchiverIfLost(strErr);
		//ErrorMessage(strErr);
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


//�R���e�L�X�g���j���[���J��
LRESULT CFileListTabClient::OnContextMenu(LPNMHDR pnmh)
{
	int idx = pnmh->idFrom;
	SetCurrentTab(idx);

	CMenu cMenu;
	cMenu.LoadMenu(IDM_TABMENU);
	CMenuHandle cSubMenu(cMenu.GetSubMenu(0));

	CFileListTabItem* pItem=(CFileListTabItem*)GetPageData(idx);

	TBVCONTEXTMENUINFO* pInfo=(TBVCONTEXTMENUINFO*)pnmh;

	UINT nCmd = (UINT)cSubMenu.TrackPopupMenu(TPM_RETURNCMD | TPM_LEFTALIGN,pInfo->pt.x,pInfo->pt.y,m_hWnd);	//���j���[�\��
	switch(nCmd){
	case ID_TABMENU_CLOSE:
		RemoveTab(idx);
		break;
	case ID_TABMENU_CLOSEOTHERS:
		RemoveTabExcept(idx);
		break;
	case ID_TABMENU_RELOAD:
		ReopenArchiveFile(m_ListMode,idx);
		break;
	case ID_TABMENU_RELOADALL:
		ReopenArchiveFileAll();
		break;
	case 0:
		//canceled
		break;
	default:
		//�R�}���h�𑼂ɓ�����
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
		//�E�B���h�E���g�p�s��
		::EnableWindow(m_rFrameWnd,FALSE);

		bool bRet=pItem->Model.ExtractArchive();

		//�E�B���h�E���g�p�\��
		::EnableWindow(m_rFrameWnd,TRUE);
		SetForegroundWindow(m_rFrameWnd);

		if(bRet && nID==ID_MENUITEM_EXTRACT_ARCHIVE_AND_CLOSE){
			CloseCurrentTab();
		}
	}
}


void CFileListTabClient::OnExtractAll(UINT,int nID,HWND)
{
	//�E�B���h�E���g�p�s��
	::EnableWindow(m_rFrameWnd,FALSE);

	int tabIndex=0;

	for(;tabIndex<GetPageCount();){
		SetCurrentTab(tabIndex);
		CFileListTabItem* pItem=GetCurrentTab();
		if(pItem){
			bool bRet=pItem->Model.ExtractArchive();
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
	//�E�B���h�E���g�p�\��
	::EnableWindow(m_rFrameWnd,TRUE);
	SetForegroundWindow(m_rFrameWnd);
}

void CFileListTabClient::OnTestArchive(UINT,int,HWND)
{
	CFileListTabItem* pItem=GetCurrentTab();
	if(pItem){
		//�E�B���h�E���g�p�s��
		::EnableWindow(m_rFrameWnd,FALSE);

		pItem->Model.TestArchive();

		//�E�B���h�E���g�p�\��
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
			pItem->UpdateFileListConfig(ConfFLW);
		}
	}
}
