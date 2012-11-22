/*
 * Copyright (c) 2005-2012, Claybird
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

#pragma once
#include "FileListTabItem.h"
#include "../resource.h"

#define FILELISTWINDOW_DEFAULT_TREE_WIDTH	175

class CFileListFrame;
enum DLL_ID;
struct CConfigFileListWindow;
class CFileListTabClient:public CTabView,public CEventDispatcher
{
public:
	BOOL PreTranslateMessage(MSG* pMsg);
protected:
	CSmartPtrCollection<CFileListTabItem> m_GC;
	//HWND				m_hFrameWnd;
	CFileListFrame&		m_rFrameWnd;
	CConfigManager&		m_rConfig;
	int					m_ColumnOrderArray[FILEINFO_ITEM_COUNT];	//���X�g�r���[�J�����̕��я�
	bool				m_bShowTab;			//�^�u�\��?
	bool				m_bShowTreeView;	//�c���[�r���[�\���Ȃ�True

	CFileListTabItem*	m_lpPrevTab;

	//�e���X�g�r���[�ŋ��ʉ�����ݒ�
	int		m_nTreeWidth;
	bool	m_bSortDescending;
	int		m_nSortKeyType;
	DWORD	m_dwListStyle;
	FILELISTMODE	m_ListMode;//�^�u�Ԃŋ��L�͂����A�ۑ��̂��߂����Ɏg�p����
protected:
	BEGIN_MSG_MAP_EX(CFileListTabClient)
		//MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MSG_WM_DESTROY(OnDestroy)
		MSG_WM_SIZE(OnSize)
		NOTIFY_CODE_HANDLER_EX(NM_SETFOCUS, OnWndStateChanged)
		NOTIFY_CODE_HANDLER_EX(LVN_ITEMCHANGED, OnWndStateChanged)
		REFLECTED_NOTIFY_CODE_HANDLER_EX(TCN_SELCHANGING, OnTabSelChanging)
		REFLECTED_NOTIFY_CODE_HANDLER_EX(TCN_SELCHANGE, OnTabSelChanged)
		REFLECTED_NOTIFY_CODE_HANDLER_EX(TBVN_PAGEACTIVATED, OnTabSelChanged)
		REFLECTED_NOTIFY_CODE_HANDLER_EX(TBVN_CONTEXTMENU,OnContextMenu)	//�^�u���j���[
		COMMAND_ID_HANDLER_EX(ID_MENUITEM_TOGGLE_TREEVIEW,OnToggleTreeView)
		COMMAND_ID_HANDLER_EX(ID_MENUITEM_EXTRACT_ARCHIVE,OnExtractArchive)
		COMMAND_ID_HANDLER_EX(ID_MENUITEM_EXTRACT_ARCHIVE_AND_CLOSE,OnExtractArchive)
		COMMAND_ID_HANDLER_EX(ID_MENUITEM_TEST_ARCHIVE,OnTestArchive)
		COMMAND_ID_HANDLER_EX(ID_MENUITEM_EXTRACT_ARCHIVE_ALL,OnExtractAll);
		COMMAND_ID_HANDLER_EX(ID_MENUITEM_EXTRACT_ARCHIVE_AND_CLOSE_ALL,OnExtractAll);
		COMMAND_RANGE_HANDLER_EX(ID_MENUITEM_SORT_FILENAME,ID_MENUITEM_SORT_CRC,OnSortItemMenu)
	if(GetActivePage()!=-1)CHAIN_COMMANDS_MEMBER(GetCurrentTab()->ListView)
		CHAIN_MSG_MAP(CTabView)
		REFLECT_NOTIFICATIONS()
		DEFAULT_REFLECTION_HANDLER()
	ALT_MSG_MAP(1)	//�^�u�R���g���[��
		MSG_WM_MBUTTONUP(OnMButtonUp)	//�}�E�X�z�C�[��(���{�^��)�N���b�N
		REFLECTED_NOTIFY_CODE_HANDLER_EX(TCN_SELCHANGING, OnTabSelChanging)
		REFLECTED_NOTIFY_CODE_HANDLER_EX(TCN_SELCHANGE, OnTabSelChanged)
		CHAIN_MSG_MAP_ALT(CTabView,1)
	END_MSG_MAP()
protected:
	//---�C�x���g�n���h��
	LRESULT OnDestroy();
	void OnSize(UINT uType, CSize &size);
	LRESULT OnTabSelChanging(LPNMHDR pnmh);
	LRESULT OnTabSelChanged(LPNMHDR pnmh);
	LRESULT OnWndStateChanged(LPNMHDR pnmh);
	LRESULT OnContextMenu(LPNMHDR pnmh);
	void OnMButtonUp(UINT, CPoint&);
	void OnExtractArchive(UINT,int,HWND);	//�A�[�J�C�u�����ׂĉ�
	void OnTestArchive(UINT,int,HWND);	//�A�[�J�C�u������
	void OnSortItemMenu(UINT,int,HWND);
	void OnToggleTreeView(UINT,int,HWND);
	void OnExtractAll(UINT,int,HWND);
protected:
	int CreateNewTab(const CConfigFileListWindow& ConfFLW);
	void ClearAllTabs();
	void RemoveTab(int);
	void RemoveTabExcept(int);
	void FitClient();
	void OnActivateTab(int newIdx);
	void OnDeactivateTab(CFileListTabItem*);
	void GetTabSettingsToClient(CFileListTabItem*);	//�w�肵���^�u�̐ݒ�������o�ϐ��ɓǂݍ���
	void UpdateClientArea();
public:
	CFileListTabClient(CConfigManager&,CFileListFrame&);
	virtual ~CFileListTabClient(){ClearAllTabs();}
	//void SetFrameWnd(HWND hWnd){m_hFrameWnd=hWnd;}
	CFileListTabItem* GetCurrentTab();

	void ReloadArchiverIfLost();

	HRESULT OpenArchiveInTab(LPCTSTR lpszArc,DLL_ID forceID,const CConfigFileListWindow& ConfFLW,LPCTSTR lpMutexName,HANDLE hMutex,CString &strErr);
	HRESULT ReopenArchiveFile(FILELISTMODE,int nPage=-1);
	void UpdateFileListConfig(const CConfigFileListWindow& ConfFLW);
	bool ReopenArchiveFileAll();
	FILELISTMODE GetFileListMode();

	void StoreSettings(CConfigFileListWindow&);

	void SetCurrentTab(int idx);
	void SetCurrentTab(HANDLE);
	void CloseCurrentTab(){RemoveTab(GetActivePage());}

	DWORD GetListViewStyle();
	void SetListViewStyle(DWORD);

	void ShowTabCtrl(bool bShow);
	bool IsTabEnabled()const{return m_bShowTab;}
};
