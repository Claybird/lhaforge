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
#include "FileListModel.h"
#include "FileListView.h"
#include "FileTreeView.h"
#include "ConfigCode/ConfigFileListWindow.h"

#define FILELISTWINDOW_DEFAULT_TREE_WIDTH	175

struct CConfigFileListWindow;
struct CFileListTabItem{
	DISALLOW_COPY_AND_ASSIGN(CFileListTabItem);
protected:
	struct COMMON {
		struct LISTVIEW {
			std::array<int, FILEINFO_ITEM_COUNT> columnOrder;
			std::array<int, FILEINFO_ITEM_COUNT> columnWidth;
			DWORD style;
		}listview;
		struct SPLITTER {
			bool bShowTreeView;
			int treeWidth;
		}splitter;
		bool initialized;

		COMMON():initialized(false){}
		virtual ~COMMON() {}
		void initialize(const CConfigFileListWindow& confFLW) {
			listview.columnOrder = confFLW.ColumnOrderArray;
			listview.columnWidth = confFLW.ColumnWidthArray;

			if (confFLW.StoreSetting) {
				splitter.bShowTreeView = confFLW.ShowTreeView;
				splitter.treeWidth = confFLW.TreeWidth;
				listview.style = confFLW.ListStyle;
			} else {
				splitter.bShowTreeView = true;
				splitter.treeWidth = FILELISTWINDOW_DEFAULT_TREE_WIDTH;
				listview.style = LVS_ICON;
			}
			initialized = true;
		}
	};
	static COMMON _common;
//---
	CLFPassphraseGUI _passphrase;
	const CConfigFileListWindow &_confFLW;

	HWND			m_hFrameWnd;


public:
	CFileListModel	Model;
	CSplitterWindow	Splitter;
	CFileListView	ListView;
	CFileTreeView	TreeView;

	//for detect same file opened in different window/tab
	CHandle hMutex;
	std::wstring strMutexName;
protected:
	//---internal functions
	bool CreateListView(HWND hParentWnd, HWND hFrameWnd) {
		ListView.Create(hParentWnd, CWindow::rcDefault, NULL, WS_CHILD | /*WS_VISIBLE | */WS_CLIPSIBLINGS | WS_CLIPCHILDREN | LVS_OWNERDATA | LVS_AUTOARRANGE | LVS_SHAREIMAGELISTS | LVS_SHOWSELALWAYS, WS_EX_CLIENTEDGE);
		ListView.SetFrameWnd(hFrameWnd);
		ListView.SetExtendedListViewStyle(LVS_EX_INFOTIP | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_HEADERDRAGDROP);
		ApplyListViewState();

		if (_confFLW.StoreSetting) {
			Model.SetSortKeyType(_confFLW.SortColumn);
			Model.SetSortMode(_confFLW.SortDescending);
		} else {
			Model.SetSortKeyType(-1);
			Model.SetSortMode(true);
		}
		return true;
	}
	bool CreateTreeView(HWND hParentWnd, HWND hFrameWnd) {
		TreeView.Create(
			hParentWnd,
			CWindow::rcDefault,
			NULL,
			WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | TVS_HASBUTTONS | TVS_HASLINES | TVS_LINESATROOT | TVS_SHOWSELALWAYS,
			WS_EX_CLIENTEDGE);
		TreeView.SetFrameWnd(hFrameWnd);
		return true;
	}

	void ApplyListViewState() {
		SetListViewStyle(_common.listview.style);
		ListView.SetColumnState(&_common.listview.columnOrder[0], &_common.listview.columnWidth[0]);
		ListView.SetDisplayFileSizeInByte(_confFLW.DisplayFileSizeInByte);
		ListView.SetDisplayPathOnly(_confFLW.DisplayPathOnly);
		ListView.Invalidate();
		//NOTE: sort column/key settings are not shared; these should be configured independently on each tab
	}
	void ApplySplitterState() {
		Splitter.SetSplitterPos(_common.splitter.treeWidth);
		Splitter.UpdateSplitterLayout();
	}
	void CopyCurrentViewState() {
		ListView.GetColumnState(&_common.listview.columnOrder[0], &_common.listview.columnWidth[0]);
		_common.listview.style = GetListViewStyle();

		_common.splitter.treeWidth = Splitter.GetSplitterPos();
		_common.splitter.bShowTreeView = IsTreeViewVisible();
	}

public:
	CFileListTabItem(const CConfigFileListWindow &confFLW, const LF_COMPRESS_ARGS& compressArgs):
		_confFLW(confFLW),
		Model(compressArgs, _passphrase),
		ListView(Model, confFLW),
		TreeView(Model, confFLW)
	{
		_common.initialize(confFLW);
	}
	virtual ~CFileListTabItem() { DestroyWindow(); }
	bool CreateTabItem(HWND hParentWnd, HWND hFrameWnd) {
		m_hFrameWnd = hFrameWnd;

		CRect rc;
		GetClientRect(hParentWnd, rc);
		Splitter.Create(hParentWnd, rc, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS);
		Splitter.SetSplitterExtendedStyle(0);

		if (!CreateTreeView(Splitter, hFrameWnd))return false;
		if (!CreateListView(Splitter, hFrameWnd))return false;

		Splitter.SetSplitterPanes(TreeView, ListView);
		ApplySplitterState();

		TreeView.SetFocus();
		return true;
	}
	void ApplyUpdatedConfig() {
		ApplyListViewState();
		ApplySplitterState();
	}
	bool OpenArchive(const std::filesystem::path &arcpath) {
		CLFScanProgressHandlerGUI progress(m_hFrameWnd);
		try {
			Model.Open(arcpath, progress);
		} catch(const LF_EXCEPTION& e){
			ErrorMessage(e.what());
			return false;
		}

		//set property to frame window
		::SetPropW(m_hFrameWnd, strMutexName.c_str(), this);

		//construct tree view structure
		TreeView.ConstructTree();
		if (_confFLW.ExpandTree)TreeView.ExpandTree();
		return true;
	}

	void DestroyWindow() {
		if (TreeView.IsWindow())TreeView.DestroyWindow();
		if (ListView.IsWindow())ListView.DestroyWindow();
		if (Splitter.IsWindow())Splitter.DestroyWindow();

		if (hMutex)hMutex.Close();
		::RemovePropW(m_hFrameWnd, strMutexName.c_str());
	}
	void ShowWindow(int nCmdShow) {
		TreeView.ShowWindow(nCmdShow);
		ListView.ShowWindow(nCmdShow);
		Splitter.ShowWindow(nCmdShow);
	}
	void OnActivated() {
		Model.addEventListener(m_hFrameWnd);
		ApplyListViewState();
		ApplySplitterState();
	}
	void OnDeactivating() {
		CopyCurrentViewState();
		Model.removeEventListener(m_hFrameWnd);
	}
	void StoreSettings(CConfigFileListWindow& ConfFLW) {
		CopyCurrentViewState();
		ConfFLW.TreeWidth = _common.splitter.treeWidth;
		ConfFLW.ShowTreeView = _common.splitter.bShowTreeView;

		//list view style
		ConfFLW.ListStyle = _common.listview.style;
		//column
		ConfFLW.ColumnOrderArray = _common.listview.columnOrder;
		ConfFLW.ColumnWidthArray = _common.listview.columnWidth;

		//sort status
		ConfFLW.SortColumn = Model.GetSortKeyType();
		ConfFLW.SortDescending = Model.GetSortMode();
	}

	DWORD GetListViewStyle()const { return ListView.GetWindowLong(GWL_STYLE) & LVS_TYPEMASK; }
	void SetListViewStyle(DWORD dwStyleNew) {
		DWORD dwStyle = ListView.GetWindowLong(GWL_STYLE);
		dwStyle &= ~(LVS_ICON | LVS_REPORT | LVS_SMALLICON | LVS_LIST);
		ListView.SetWindowLong(GWL_STYLE, dwStyle | dwStyleNew);
	}

	void SetSortColumn(int iCol) { ListView.SortItem(iCol); }

	void ShowTreeView(bool bShow) {
		if (bShow) {
			Splitter.SetSinglePaneMode(SPLIT_PANE_NONE);    // show both pane
		} else {
			Splitter.SetSinglePaneMode(SPLIT_PANE_RIGHT);   // right pane only
		}
	}
	bool IsTreeViewVisible() {
		return SPLIT_PANE_NONE == Splitter.GetSinglePaneMode();
	}
};
WEAK_SYMBOL CFileListTabItem::COMMON CFileListTabItem::_common;
