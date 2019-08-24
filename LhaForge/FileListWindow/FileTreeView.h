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
