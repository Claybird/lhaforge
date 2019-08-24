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

#pragma once
#include "ArcFileContent.h"
#include "FileListModel.h"
#include "OLE/DropTarget.h"	//ドロップ受け入れ,IDropCommunicator
#include "FileListMessages.h"
#include "../resource.h"

class CFileTreeView:public CWindowImpl<CFileTreeView,CTreeViewCtrl>,public IDropCommunicator//自前のインターフェイス
{
protected:
	CImageList	m_ImageList;
	typedef std::map<ARCHIVE_ENTRY_INFO_TREE*,HTREEITEM> ITEMDICT;
	ITEMDICT m_TreeItemMap;
	CFileListModel &mr_Model;
	bool m_bSelfAction;
	HWND m_hFrameWnd;
protected:
	//---ドロップ受け入れ
	CDropTarget m_DropTarget;	//ドロップ受け入れに使う
	HTREEITEM m_hDropHilight;	//ドロップハイライト状態にあるアイテムのハンドル
	//IDropCommunicatorの実装
	HRESULT DragEnter(IDataObject*,POINTL&,DWORD&);
	HRESULT DragLeave();
	HRESULT DragOver(IDataObject*,POINTL&,DWORD&);
	HRESULT Drop(IDataObject*,POINTL&,DWORD&);
protected:
	BEGIN_MSG_MAP_EX(CFileTreeView)
		MSG_WM_CREATE(OnCreate)
		MSG_WM_DESTROY(OnDestroy)
		MSG_WM_CONTEXTMENU(OnContextMenu)	//右クリックメニュー
		MESSAGE_HANDLER(WM_FILELIST_ARCHIVE_LOADED, OnFileListArchiveLoaded)
		MESSAGE_HANDLER(WM_FILELIST_NEWCONTENT, OnFileListNewContent)
		MESSAGE_HANDLER(WM_FILELIST_UPDATED, OnFileListUpdated)

		COMMAND_ID_HANDLER_EX(ID_MENUITEM_DELETE_SELECTED,OnDelete)
		COMMAND_ID_HANDLER_EX(ID_MENUITEM_EXTRACT_SELECTED,OnExtractItem)
		COMMAND_ID_HANDLER_EX(ID_MENUITEM_EXTRACT_SELECTED_SAMEDIR,OnExtractItem)
		COMMAND_ID_HANDLER_EX(ID_MENUITEM_OPEN_ASSOCIATION,OnOpenAssociation)
		COMMAND_ID_HANDLER_EX(ID_MENUITEM_OPEN_ASSOCIATION_OVERWRITE,OnOpenAssociation)
		COMMAND_ID_HANDLER_EX(ID_MENUITEM_EXTRACT_TEMPORARY,OnExtractTemporary)
		COMMAND_RANGE_HANDLER_EX(ID_MENUITEM_USERAPP_BEGIN,ID_MENUITEM_USERAPP_END+MenuCommand_GetNumSendToCmd(),OnOpenWithUserApp)

		REFLECTED_NOTIFY_CODE_HANDLER_EX(NM_RCLICK, OnRClick)
		REFLECTED_NOTIFY_CODE_HANDLER_EX(TVN_SELCHANGED, OnTreeSelect)
		DEFAULT_REFLECTION_HANDLER()
	END_MSG_MAP()

protected:
	//---internal functions
	LRESULT OnCreate(LPCREATESTRUCT lpcs);
	LRESULT OnDestroy();
	void OnContextMenu(HWND,CPoint&);

	LRESULT OnRClick(LPNMHDR);
	LRESULT OnTreeSelect(LPNMHDR);
	LRESULT OnFileListArchiveLoaded(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnFileListNewContent(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnFileListUpdated(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	bool UpdateCurrentNode();

	bool IsSelfAction(){return m_bSelfAction;}
	void BeginSelfAction(){m_bSelfAction=true;}
	void EndSelfAction(){m_bSelfAction=false;}

	void OnDelete(UINT uNotifyCode,int nID,HWND hWndCtrl);
	void OnOpenWithUserApp(UINT uNotifyCode,int nID,HWND hWndCtrl);
	bool OnUserApp(const std::vector<CMenuCommandItem> &menuCommandArray,UINT nID);
	bool OnSendToApp(UINT nID);
	void OnExtractItem(UINT,int nID,HWND);
	void GetSelectedItems(std::list<ARCHIVE_ENTRY_INFO_TREE*> &items);
	void OnOpenAssociation(UINT uNotifyCode,int nID,HWND hWndCtrl);
	void OnExtractTemporary(UINT uNotifyCode,int nID,HWND hWndCtrl);
	bool OpenAssociation(bool bOverwrite,bool bOpen);
	void OpenAssociation(const std::list<CString> &filesList);

public:
	DECLARE_WND_SUPERCLASS(NULL, CTreeViewCtrl::GetWndClassName())
	BOOL PreTranslateMessage(MSG* pMsg){return FALSE;}

	CFileTreeView(CFileListModel&);
	virtual ~CFileTreeView(){}
	bool ConstructTree(HTREEITEM hParentItem=NULL,ARCHIVE_ENTRY_INFO_TREE* lpNode=NULL);
	void Clear();
	void ExpandTree();

	void EnableDropTarget(bool bEnable);
	void SetFrameWnd(HWND hWnd){m_hFrameWnd=hWnd;}
};
