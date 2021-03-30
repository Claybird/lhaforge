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
#include "ShellDataManager.h"
#include "OLE/DnDSource.h"
#include "Utilities/OSUtil.h"
#include "FileViewBase.h"

struct CConfigFileListWindow;
class CFileListView:
	public CFileViewBase<CFileListView,CListViewCtrl>,
	public CCustomDraw<CFileListView>
{
public:
	DECLARE_WND_SUPERCLASS(NULL, CListViewCtrl::GetWndClassName())
	BOOL PreTranslateMessage(MSG* pMsg){return FALSE;}
protected:
	BEGIN_MSG_MAP_EX(CFileListView)
		MSG_WM_CREATE(OnCreate)
		MSG_WM_DESTROY(OnDestroy)
		MSG_WM_CONTEXTMENU(OnContextMenu)
		COMMAND_ID_HANDLER_EX(ID_MENUITEM_SELECT_ALL,OnSelectAll)
		MESSAGE_HANDLER(WM_FILELIST_ARCHIVE_LOADED, OnFileListNewContent)
		MESSAGE_HANDLER(WM_FILELIST_NEWCONTENT, OnFileListNewContent)
		MESSAGE_HANDLER(WM_FILELIST_UPDATED, OnFileListUpdated)
		NOTIFY_CODE_HANDLER(NM_RCLICK, OnColumnRClick)
		COMMAND_ID_HANDLER_EX(ID_MENUITEM_DELETE_SELECTED,OnDelete)
		COMMAND_ID_HANDLER_EX(ID_MENUITEM_EXTRACT_SELECTED,OnExtractItem)
		COMMAND_ID_HANDLER_EX(ID_MENUITEM_EXTRACT_SELECTED_SAMEDIR,OnExtractItem)
		COMMAND_ID_HANDLER_EX(ID_MENUITEM_FINDITEM,OnFindItem)
		COMMAND_ID_HANDLER_EX(ID_MENUITEM_FINDITEM_END,OnFindItem)
		COMMAND_ID_HANDLER_EX(ID_MENUITEM_ADD_FILE,OnAddItems)
		COMMAND_ID_HANDLER_EX(ID_MENUITEM_ADD_DIRECTORY,OnAddItems)
		COMMAND_ID_HANDLER_EX(ID_MENUITEM_SHOW_COLUMNHEADER_MENU,OnShowCustomizeColumn)
		COMMAND_ID_HANDLER_EX(ID_MENUITEM_OPEN_ASSOCIATION,OnOpenAssociation)
		COMMAND_ID_HANDLER_EX(ID_MENUITEM_OPEN_ASSOCIATION_OVERWRITE,OnOpenAssociation)
		COMMAND_ID_HANDLER_EX(ID_MENUITEM_EXTRACT_TEMPORARY,OnExtractTemporary)
		COMMAND_ID_HANDLER_EX(ID_MENUITEM_CLEAR_TEMPORARY,OnClearTemporary)
		COMMAND_RANGE_HANDLER_EX(ID_MENUITEM_COPY_FILENAME,ID_MENUITEM_COPY_ALL,OnCopyInfo)
		COMMAND_RANGE_HANDLER_EX(ID_MENUITEM_USERAPP_BEGIN,ID_MENUITEM_USERAPP_END+MenuCommand_GetSendToCmdArray().size(),OnOpenWithUserApp)
		REFLECTED_NOTIFY_CODE_HANDLER_EX(NM_DBLCLK, OnDblClick)
		REFLECTED_NOTIFY_CODE_HANDLER_EX(NM_RETURN, OnDblClick)
		REFLECTED_NOTIFY_CODE_HANDLER_EX(LVN_BEGINDRAG, OnBeginDrag)
		REFLECTED_NOTIFY_CODE_HANDLER_EX(LVN_GETINFOTIP, OnGetInfoTip)
		REFLECTED_NOTIFY_CODE_HANDLER_EX(LVN_GETDISPINFO, OnGetDispInfo)
		REFLECTED_NOTIFY_CODE_HANDLER_EX(LVN_ODFINDITEM, OnFindAsYouType)
		REFLECTED_NOTIFY_CODE_HANDLER_EX(LVN_COLUMNCLICK, OnSortItem)
		CHAIN_MSG_MAP_ALT(CCustomDraw<CFileListView>, 1)	// chain to CCustomDraw
		DEFAULT_REFLECTION_HANDLER()
	END_MSG_MAP()
protected:
	std::array<int/*FILEINFO_TYPE*/, FILEINFO_ITEM_COUNT> m_ColumnIndexArray;	//-1 if invisible
	
	CLFShellDataManager			m_ShellDataManager;
	CImageList					m_SortImageList;
	bool	m_bDisplayFileSizeInByte;
	bool	m_bPathOnly;

	CLFDnDSource	m_DnDSource;
protected:
	LRESULT OnCreate(LPCREATESTRUCT lpcs);
	LRESULT OnDestroy();
	LRESULT OnFileListNewContent(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnFileListUpdated(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {Invalidate();return 0;}

	LRESULT OnGetDispInfo(LPNMHDR pnmh);
	LRESULT OnGetInfoTip(LPNMHDR pnmh);
	LRESULT OnFindAsYouType(LPNMHDR pnmh);
	LRESULT OnSortItem(LPNMHDR pnmh) {
		int iCol = ((LPNMLISTVIEW)pnmh)->iSubItem;
		SortItem(iCol);
		return 0;
	}
	LRESULT OnBeginDrag(LPNMHDR pnmh);
	LRESULT OnDblClick(LPNMHDR pnmh);
	void OnSelectAll(UINT,int,HWND);
	void OnFindItem(UINT,int,HWND);
	void OnShowCustomizeColumn(UINT,int,HWND);
	void OnClearTemporary(UINT, int, HWND) {
		mr_Model.ClearTempDir();
	}
	void OnAddItems(UINT,int,HWND);

	//コマンドハンドラ
	void OnCopyInfo(UINT,int,HWND);
public:
	LRESULT OnColumnRClick(int, LPNMHDR pnmh, BOOL& bHandled);
	void SortItem(int iCol);
	DWORD OnPrePaint(int nID, LPNMCUSTOMDRAW lpnmcd) {
		if (lpnmcd->hdr.hwndFrom == m_hWnd) {
			return CDRF_NOTIFYITEMDRAW;
		} else {
			return CDRF_DODEFAULT;
		}
	}
	DWORD OnItemPrePaint(int nID, LPNMCUSTOMDRAW lpnmcd);
protected:
	//---internal functions
	void UpdateSortIcon();
public:
	CFileListView(CFileListModel& rModel, const CConfigFileListWindow &r_confFLW);
	virtual ~CFileListView(){}
	bool SetColumnState(const std::array<int, FILEINFO_ITEM_COUNT>& columnOrder, const std::array<int, FILEINFO_ITEM_COUNT>& columnWidthArray);	//リストビューのカラムをセットする
	void GetColumnState(std::array<int, FILEINFO_ITEM_COUNT>& columnOrder, std::array<int, FILEINFO_ITEM_COUNT>& columnWidthArray);

	void SetDisplayFileSizeInByte(bool b){m_bDisplayFileSizeInByte=b;}
	void SetDisplayPathOnly(bool b){m_bPathOnly=b;}

	std::vector<const ARCHIVE_ENTRY_INFO*> GetSelectedItems()override;

	bool IsValidDropTarget(const HIGHLIGHT& item)const override { 
		if (item.isValid()) {
			auto lpNode = mr_Model.GetFileListItemByIndex(item);
			if (lpNode) {		//Drop on target
				//is target directory?
				if (lpNode->is_directory()) {
					return true;
				}
			}
		}
		return false;
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
			dwEffect = DROPEFFECT_COPY;

			//---destination
			CPoint ptTemp(pt.x, pt.y);
			ScreenToClient(&ptTemp);
			auto target = HitTest(ptTemp, nullptr);

			auto lpNode = mr_Model.GetFileListItemByIndex(target);
			if (lpNode) {
				if (lpNode->is_directory()) {
					//onto directory
					return AddItemsToDirectory(lpNode, files);
				} else {
					//onto file
					return AddItemsToDirectory(mr_Model.getCurrentDir(), files);
				}
			} else {
				//onto empty space
				return AddItemsToDirectory(mr_Model.getCurrentDir(), files);
			}
		} else {
			//not acceptable
			dwEffect = DROPEFFECT_NONE;
			return S_FALSE;
		}
	}

};
