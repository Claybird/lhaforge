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
#include "FileListTabItem.h"
#include "../ConfigCode/ConfigFileListWindow.h"
#include "../resource.h"
#include "../Utilities/StringUtil.h"

CFileListTabItem::CFileListTabItem(CConfigManager &rMan):
	Model(rMan),
	ListView(rMan,Model),
	TreeView(Model),
	hMutex(NULL)
{
}

bool CFileListTabItem::CreateListView(HWND hParentWnd,HWND hFrameWnd,const CConfigFileListWindow &ConfFLW)
{
	//--�t�@�C���ꗗ�E�B���h�E�쐬
	ListView.Create(hParentWnd,CWindow::rcDefault,NULL,WS_CHILD | /*WS_VISIBLE | */WS_CLIPSIBLINGS | WS_CLIPCHILDREN | LVS_OWNERDATA|LVS_AUTOARRANGE|LVS_SHAREIMAGELISTS|LVS_SHOWSELALWAYS, WS_EX_CLIENTEDGE);
	//�t���[���E�B���h�E�̃n���h����������
	ListView.SetFrameWnd(hFrameWnd);

	//�X�^�C���ݒ�
	ListView.SetExtendedListViewStyle(LVS_EX_INFOTIP | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES|LVS_EX_HEADERDRAGDROP);

	//���X�g�r���[�ɃJ�����ǉ�
	if(!ListView.SetColumnState(ConfFLW.ColumnOrderArray, ConfFLW.ColumnWidthArray))return false;

	//�\���ݒ�
	UpdateFileListConfig(ConfFLW);

	//�\�[�g�ݒ�
	if(ConfFLW.StoreSetting){
		Model.SetSortKeyType(ConfFLW.SortColumn);
		Model.SetSortMode(0!=ConfFLW.SortDescending);
	}else{
		Model.SetSortKeyType(-1);
		Model.SetSortMode(true);
	}

	//���X�g�r���[�X�^�C���̐ݒ�
	if(ConfFLW.StoreSetting){
		DWORD Style=ListView.GetWindowLong(GWL_STYLE);
		Style&=~(LVS_ICON|LVS_REPORT|LVS_SMALLICON|LVS_LIST);
		ListView.SetWindowLong(GWL_STYLE,Style|ConfFLW.ListStyle);
	}else{
		DWORD Style=ListView.GetWindowLong(GWL_STYLE);
		Style&=~(LVS_ICON|LVS_REPORT|LVS_SMALLICON|LVS_LIST);
		ListView.SetWindowLong(GWL_STYLE,Style|LVS_ICON);
	}
	return true;
}

void CFileListTabItem::UpdateFileListConfig(const CConfigFileListWindow& ConfFLW)
{
	//�\���ݒ�
	ListView.SetDisplayFileSizeInByte(BOOL2bool(ConfFLW.DisplayFileSizeInByte));
	ListView.SetDisplayPathOnly(BOOL2bool(ConfFLW.DisplayPathOnly));
	ListView.Invalidate();
}

bool CFileListTabItem::CreateTreeView(HWND hParentWnd,HWND hFrameWnd,const CConfigFileListWindow &ConfFLW)
{
	//�c���[�r���[�쐬
	TreeView.Create(hParentWnd,CWindow::rcDefault,NULL,WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN|TVS_HASBUTTONS | TVS_HASLINES | TVS_LINESATROOT | TVS_SHOWSELALWAYS,WS_EX_CLIENTEDGE);

	//�t���[���E�B���h�E�̃n���h����������
	TreeView.SetFrameWnd(hFrameWnd);
	return true;
}

bool CFileListTabItem::CreateTabItem(HWND hParentWnd,HWND hFrameWnd,const CConfigFileListWindow &ConfFLW)
{
	m_hFrameWnd=hFrameWnd;

	// �X�v���b�^�E�B���h�E���쐬
	CRect rc;
	GetClientRect(hParentWnd,rc);
	Splitter.Create(hParentWnd,rc,NULL,WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS);
	// �X�v���b�^�E�B���h�E�g���X�^�C����ݒ�
	Splitter.SetSplitterExtendedStyle(0);

	//---�c���[�r���[
	if(!CreateTreeView(Splitter,hFrameWnd,ConfFLW))return false;
	//---���X�g�r���[
	if(!CreateListView(Splitter,hFrameWnd,ConfFLW))return false;

	//�����E�B���h�E�ɐݒ�
	Splitter.SetSplitterPanes(TreeView,ListView);
	// �����o�[�̈ʒu��ݒ�
	Splitter.SetSplitterPos(ConfFLW.TreeWidth);
	Splitter.UpdateSplitterLayout();
	return true;
}


HRESULT CFileListTabItem::OpenArchive(LPCTSTR lpszArc,DLL_ID forceID,const CConfigFileListWindow& ConfFLW,FILELISTMODE flMode,IArchiveContentUpdateHandler* lpHandler,CString &strErr)
{
	//---���
	idForceDLL=forceID;
	HRESULT hr=Model.OpenArchiveFile(lpszArc,forceID,flMode,strErr,lpHandler);

	if(SUCCEEDED(hr)){
		//�c���[�\�z
		TreeView.ConstructTree();
		while(UtilDoMessageLoop())continue;	//�����Ń��b�Z�[�W���[�v���񂳂Ȃ��ƃc���[�A�C�e�����L���ɂȂ�Ȃ�
		if(ConfFLW.ExpandTree)TreeView.ExpandTree();
	}
	return hr;
}

void CFileListTabItem::DestroyWindow()
{
	if(TreeView.IsWindow())TreeView.DestroyWindow();
	if(ListView.IsWindow())ListView.DestroyWindow();
	if(Splitter.IsWindow())Splitter.DestroyWindow();

	if(hMutex){
		CloseHandle(hMutex);
		hMutex=NULL;
	}
}

void CFileListTabItem::ShowWindow(int nCmdShow)
{
	TreeView.ShowWindow(nCmdShow);
	ListView.ShowWindow(nCmdShow);
	Splitter.ShowWindow(nCmdShow);
}

void CFileListTabItem::OnActivated()
{
	//---�t���[���E�B���h�E���C�x���g���X�i�ɓo�^
	Model.addEventListener(m_hFrameWnd);
}

void CFileListTabItem::OnDeactivated()
{
	//---�t���[���E�B���h�E���C�x���g���X�i�������
	Model.removeEventListener(m_hFrameWnd);
}

DWORD CFileListTabItem::GetListViewStyle()
{
	return ListView.GetWindowLong(GWL_STYLE);
}

void CFileListTabItem::SetListViewStyle(DWORD dwStyleNew)
{
	DWORD dwStyle=ListView.GetWindowLong(GWL_STYLE);
	dwStyle&=~(LVS_ICON|LVS_REPORT|LVS_SMALLICON|LVS_LIST);
	ListView.SetWindowLong(GWL_STYLE,dwStyle|dwStyleNew);
}

void CFileListTabItem::SetSortColumn(int iCol)
{
	ListView.SortItem(iCol);
}

void CFileListTabItem::ShowTreeView(bool bShow)
{
	if(bShow){
		Splitter.SetSinglePaneMode(SPLIT_PANE_NONE);    // ���y�C���\��
	}else{
		Splitter.SetSinglePaneMode(SPLIT_PANE_RIGHT);   // �E�y�C���̂ݕ\��
	}
}
