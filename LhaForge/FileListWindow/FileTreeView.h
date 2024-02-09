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
	struct TREE_USER_DATA {
		TREE_USER_DATA() :pFind(nullptr), pInfo(nullptr) {}
		const ARCHIVE_FIND_CONDITION* pFind;
		const ARCHIVE_ENTRY_INFO* pInfo;
		bool isDirectory()const {
			if (pInfo) {
				if (!pInfo->_parent || pInfo->is_directory()) {
					return true;
				}
			}
			return false;
		}
	};
	std::vector<std::shared_ptr<TREE_USER_DATA>> m_GC;
protected:
	HTREEITEM m_hDropHilight;	//highlited item handle for drag-drop
protected:
	BEGIN_MSG_MAP_EX(CFileTreeView)
		MSG_WM_CREATE(OnCreate)
		MSG_WM_DESTROY(OnDestroy)
		MSG_WM_CONTEXTMENU(OnContextMenu)
		MESSAGE_HANDLER(WM_FILELIST_ARCHIVE_LOADED, OnFileListArchiveLoaded)
		MESSAGE_HANDLER(WM_FILELIST_NEWCONTENT, OnFileListNewContent)

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
	LRESULT OnCreate(LPCREATESTRUCT lpcs) {
		LRESULT lRes = DefWindowProc();
		SetFont(AtlGetDefaultGuiFont());

		mr_Model.addEventListener(m_hWnd);

		m_ImageList.Create(::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CXSMICON), ILC_COLOR32 | ILC_MASK, 1, 1);

		//directory icons
		//-close
		SHFILEINFO shfi = {};
		SHGetFileInfoW(L"dummy", FILE_ATTRIBUTE_DIRECTORY, &shfi, sizeof(shfi), SHGFI_USEFILEATTRIBUTES | SHGFI_ICON | SHGFI_SMALLICON);
		m_ImageList.AddIcon(shfi.hIcon);
		DestroyIcon(shfi.hIcon);
		//-open
		SHGetFileInfoW(L"dummy", FILE_ATTRIBUTE_DIRECTORY, &shfi, sizeof(shfi), SHGFI_USEFILEATTRIBUTES | SHGFI_ICON | SHGFI_SMALLICON | SHGFI_OPENICON);
		m_ImageList.AddIcon(shfi.hIcon);
		DestroyIcon(shfi.hIcon);

		//search icons
		CIcon ico;
		ico.LoadIcon(MAKEINTRESOURCE(IDI_ICON_SEARCH));
		m_ImageList.AddIcon(ico);

		SetImageList(m_ImageList, TVSIL_NORMAL);
		return lRes;
	}
	LRESULT OnDestroy() {
		mr_Model.removeEventListener(m_hWnd);
		m_ImageList.Destroy();
		return 0;
	}

	void AddSearchFolder() {
		//root - search folder
		HTREEITEM hParent = InsertItem(UtilLoadString(IDS_TREE_SEARCH_FOLDER).c_str(), TVI_ROOT, TVI_LAST);
		SetItemImage(hParent, searchIconIndex, searchIconIndex);
		SetItemData(hParent, NULL);
		//search conditions
		for (const auto& pair : mr_confFLW.view.searchFolderItems) {
			const auto &name = pair.first;
			const auto &cond = pair.second;

			//title only, no icon
			HTREEITEM hItem = InsertItem(name.c_str(), hParent, TVI_LAST);
			auto p = std::make_shared<TREE_USER_DATA>();
			p->pFind = &cond;
			m_GC.push_back(p);
			SetItemData(hItem, (DWORD_PTR)p.get());
		}
	}
	const TREE_USER_DATA* GetTreeItemUserData(HTREEITEM hItem)const {
		if (!hItem)return nullptr;
		auto lpData = (const TREE_USER_DATA*)GetItemData(hItem);
		return lpData;
	}

	LRESULT OnTreeSelect(LPNMHDR) {
		auto lpCurrent = mr_Model.getCurrentDir();

		HTREEITEM hItem = GetSelectedItem();
		if (!hItem)return 0;

		auto lpData = GetTreeItemUserData(hItem);
		if (lpData) {
			mr_Model.EndFindItem();
			if (lpData->pInfo) {
				if (lpData->pInfo != lpCurrent) {
					mr_Model.setCurrentDir(lpData->pInfo);
				}
			} else if(lpData->pFind){
				//find all elements
				auto lpFound = mr_Model.FindItem(*(lpData->pFind));
				mr_Model.setCurrentDir(lpFound);
			}
		}
		return 0;
	}
	LRESULT OnFileListArchiveLoaded(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
		Clear();
		AddSearchFolder();
		ConstructTree();

		if (mr_confFLW.view.ExpandTree)ExpandTree();

		EnableDropTarget(true);
		return 0;
	}
	LRESULT OnFileListNewContent(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
		UpdateCurrentNode();
		return 0;
	}
	bool UpdateCurrentNode() {
		auto lpNode = mr_Model.getCurrentDir();
		const auto ite = m_TreeItemMap.find(lpNode);
		if (m_TreeItemMap.end() == ite) {
			return false;
		}
		EnsureVisible((*ite).second);
		SelectItem((*ite).second);
		return true;
	}

	std::vector<const ARCHIVE_ENTRY_INFO*> GetSelectedItems()override {
		std::vector<const ARCHIVE_ENTRY_INFO*> items;
		HTREEITEM hItem = GetSelectedItem();
		if (hItem) {
			auto lpNode = GetTreeItemUserData(hItem);
			if (lpNode && lpNode->pInfo) {
				items.push_back(lpNode->pInfo);
			}
		}
		//always one item maximum
		ASSERT(items.size() <= 1);
		return items;
	}
	LRESULT OnRClick(LPNMHDR lpNM) {
		CPoint pt;
		GetCursorPos(&pt);
		CPoint ptClient = pt;
		ScreenToClient(&ptClient);
		SelectItem(HitTest(ptClient, nullptr));
		OnContextMenu(m_hWnd, pt);
		return 0;
	}
	enum :int {
		dirIconClosed = 0,
		dirIconOpened = 1,
		searchIconIndex = 2,
		archiveIconIndex = 3,
	};
public:
	DECLARE_WND_SUPERCLASS(NULL, CTreeViewCtrl::GetWndClassName())
	BOOL PreTranslateMessage(MSG* pMsg){return FALSE;}

	CFileTreeView(CFileListModel& rModel,const CConfigFileListWindow& r_confFLW):
		CFileViewBase(rModel, r_confFLW),
		m_hDropHilight(NULL)
	{}
	virtual ~CFileTreeView(){}
	bool ConstructTree(HTREEITEM hParentItem = nullptr, const ARCHIVE_ENTRY_INFO* lpNode = nullptr) {
		HTREEITEM hItem;
		if (hParentItem) {
			//directory
			hItem = InsertItem(lpNode->_entryName.c_str(), hParentItem, TVI_LAST);
			SetItemImage(hItem, dirIconClosed, dirIconOpened);
		} else {
			//---root
			//archive file icon
			m_ImageList.Remove(archiveIconIndex);	//remove old icon
			SHFILEINFO shfi = {};
			::SHGetFileInfoW(mr_Model.GetArchiveFileName().c_str(),
				0, &shfi, sizeof(shfi), SHGFI_ICON | SHGFI_SMALLICON);
			m_ImageList.AddIcon(shfi.hIcon);

			lpNode = mr_Model.GetRootNode();
			hItem = InsertItem(mr_Model.GetArchiveFileName().filename().c_str(), TVI_ROOT, TVI_LAST);
			SetItemImage(hItem, archiveIconIndex, archiveIconIndex);
		}

		m_TreeItemMap.insert({ lpNode,hItem });
		//set node pointer
		auto p = std::make_shared<TREE_USER_DATA>();
		p->pInfo = lpNode;
		m_GC.push_back(p);
		SetItemData(hItem, (DWORD_PTR)p.get());

		//process children
		UINT numItems = lpNode->getNumChildren();
		for (UINT i = 0; i < numItems; i++) {
			const auto* lpChild = lpNode->getChild(i);
			if (lpChild->is_directory()) {
				ConstructTree(hItem, lpChild);
			}
		}

		return true;
	}
	void Clear() {
		DeleteAllItems();
		m_TreeItemMap.clear();
		m_GC.clear();
	}
	void ExpandTree() {
		for (const auto &item : m_TreeItemMap) {
			Expand(item.second);
		}
	}
	void ApplyUpdatedConfig() {
		Clear();
		AddSearchFolder();
		ConstructTree();

		if (mr_confFLW.view.ExpandTree)ExpandTree();

		EnableDropTarget(true);
	}

	bool IsValidDropTarget(const HIGHLIGHT& hl)const override {
		if (hl.isValid()) {
			auto lpNode = GetTreeItemUserData(hl.itemTree);
			if (lpNode) {
				if (lpNode->isDirectory())return true;
				//cannot be dropped into a search folder
				else return false;
			}else{
				return false;
			}
		}
		return true;
	}

	//dropped
	virtual HRESULT Drop(IDataObject *lpDataObject, POINTL &pt, DWORD &dwEffect)override {
		if (m_dropHighlight.isValid()) {
			//disable drop highlight
			SetItemState(m_dropHighlight, ~LVIS_DROPHILITED, LVIS_DROPHILITED);
		}
		m_dropHighlight.invalidate();

		auto[hr, files] = m_DropTarget.GetDroppedFiles(lpDataObject);
		if (S_OK == hr) {
			//files dropped
			dwEffect = DROPEFFECT_COPY;

			//---destination
			CPoint ptTemp(pt.x, pt.y);
			ScreenToClient(&ptTemp);
			auto target = HitTest(ptTemp, nullptr);

			auto lpNode = GetTreeItemUserData(target);
			if (lpNode && lpNode->isDirectory()) {
				return AddItemsToDirectory(lpNode->pInfo, files);
			} else {
				return E_HANDLE;
			}
		} else {
			//not acceptable
			dwEffect = DROPEFFECT_NONE;
			return S_FALSE;
		}
	}
};
