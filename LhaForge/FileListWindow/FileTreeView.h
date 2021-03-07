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
#include "FileViewBase.h"

struct ARCHIVE_ENTRY_INFO;
class CFileTreeView:public CFileViewBase<CFileTreeView,CTreeViewCtrl>
{
protected:
	CImageList	m_ImageList;
	std::map<const ARCHIVE_ENTRY_INFO*,HTREEITEM> m_TreeItemMap;
	bool m_bSelfAction;
protected:
	HTREEITEM m_hDropHilight;	//highlited item handle for drag-drop
protected:
	BEGIN_MSG_MAP_EX(CFileTreeView)
		MSG_WM_CREATE(OnCreate)
		MSG_WM_DESTROY(OnDestroy)
		MSG_WM_CONTEXTMENU(OnContextMenu)
		MESSAGE_HANDLER(WM_FILELIST_ARCHIVE_LOADED, OnFileListArchiveLoaded)
		MESSAGE_HANDLER(WM_FILELIST_NEWCONTENT, OnFileListNewContent)
		MESSAGE_HANDLER(WM_FILELIST_UPDATED, OnFileListUpdated)

		COMMAND_ID_HANDLER_EX(ID_MENUITEM_DELETE_SELECTED,OnDelete)
		COMMAND_ID_HANDLER_EX(ID_MENUITEM_EXTRACT_SELECTED,OnExtractItem)
		COMMAND_ID_HANDLER_EX(ID_MENUITEM_EXTRACT_SELECTED_SAMEDIR,OnExtractItem)
		COMMAND_ID_HANDLER_EX(ID_MENUITEM_OPEN_ASSOCIATION,OnOpenAssociation)
		COMMAND_ID_HANDLER_EX(ID_MENUITEM_OPEN_ASSOCIATION_OVERWRITE,OnOpenAssociation)
		COMMAND_ID_HANDLER_EX(ID_MENUITEM_EXTRACT_TEMPORARY,OnExtractTemporary)
		COMMAND_RANGE_HANDLER_EX(ID_MENUITEM_USERAPP_BEGIN,ID_MENUITEM_USERAPP_END+MenuCommand_GetSendToCmdArray().size(),OnOpenWithUserApp)

		REFLECTED_NOTIFY_CODE_HANDLER_EX(NM_RCLICK, OnRClick)
		REFLECTED_NOTIFY_CODE_HANDLER_EX(TVN_SELCHANGED, OnTreeSelect)
		DEFAULT_REFLECTION_HANDLER()
	END_MSG_MAP()

protected:
	//---internal functions
	LRESULT OnCreate(LPCREATESTRUCT lpcs);
	LRESULT OnDestroy() {
		mr_Model.removeEventListener(m_hWnd);
		m_ImageList.Destroy();
		return 0;
	}

	LRESULT OnTreeSelect(LPNMHDR);
	LRESULT OnFileListArchiveLoaded(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
		Clear();
		ConstructTree();

		EnableDropTarget(true);
		return 0;
	}
	LRESULT OnFileListNewContent(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
		UpdateCurrentNode();
		return 0;
	}
	LRESULT OnFileListUpdated(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
		//nothing to do
		return 0;
	}
	bool UpdateCurrentNode();

	//flags to prevent actios while ConstructTree()
	bool IsSelfAction(){return m_bSelfAction;}
	void BeginSelfAction(){m_bSelfAction=true;}
	void EndSelfAction(){m_bSelfAction=false;}

	void OnExtractItem(UINT,int nID,HWND);
	void OnExtractTemporary(UINT uNotifyCode,int nID,HWND hWndCtrl);

	std::vector<const ARCHIVE_ENTRY_INFO*> GetSelectedItems()override;
	LRESULT OnRClick(LPNMHDR lpNM) {
		CPoint pt;
		GetCursorPos(&pt);
		CPoint ptClient = pt;
		ScreenToClient(&ptClient);
		SelectItem(HitTest(ptClient, nullptr));
		OnContextMenu(m_hWnd, pt);
		return 0;
	}

public:
	DECLARE_WND_SUPERCLASS(NULL, CTreeViewCtrl::GetWndClassName())
	BOOL PreTranslateMessage(MSG* pMsg){return FALSE;}

	CFileTreeView(CFileListModel&);
	virtual ~CFileTreeView(){}
	bool ConstructTree(HTREEITEM hParentItem = nullptr, const ARCHIVE_ENTRY_INFO* lpNode = nullptr);
	void Clear() {
		DeleteAllItems();
		m_TreeItemMap.clear();
	}
	void ExpandTree() {
		for (const auto &item : m_TreeItemMap) {
			Expand(item.second);
		}
	}
};
