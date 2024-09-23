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
			std::array<int, (int)FILEINFO_TYPE::ItemCount> columnOrder;
			std::array<int, (int)FILEINFO_TYPE::ItemCount> columnWidth;
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
			listview.columnOrder = confFLW.view.column.order;
			listview.columnWidth = confFLW.view.column.width;

			if (confFLW.general.StoreSetting) {
				splitter.bShowTreeView = confFLW.view.ShowTreeView;
				splitter.treeWidth = confFLW.dimensions.TreeWidth;
				listview.style = confFLW.view.ListStyle;
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
	std::shared_ptr<CLFPassphraseGUI> _passphrase;
	const CConfigFileListWindow &_confFLW;

	HWND			m_hFrameWnd;


public:
	CFileListModel	Model;
	CSplitterWindow	Splitter;
	CFileListView	ListView;
	CFileTreeView	TreeView;

	//for detect same file opened in different window/tab
	CHandle _hMutex;
	std::wstring _strMutexName;
protected:
	//---internal functions
	bool CreateListView(HWND hParentWnd, HWND hFrameWnd) {
		ListView.Create(hParentWnd, CWindow::rcDefault, NULL, WS_CHILD | /*WS_VISIBLE | */WS_CLIPSIBLINGS | WS_CLIPCHILDREN | LVS_OWNERDATA | LVS_AUTOARRANGE | LVS_SHAREIMAGELISTS | LVS_SHOWSELALWAYS, WS_EX_CLIENTEDGE);
		ListView.SetFrameWnd(hFrameWnd);
		ListView.SetExtendedListViewStyle(LVS_EX_INFOTIP | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_HEADERDRAGDROP);
		ApplyListViewState();

		if (_confFLW.general.StoreSetting) {
			Model.SetSortKeyType(_confFLW.view.SortColumnIndex);
			Model.SetSortAtoZ(_confFLW.view.SortAtoZ);
		} else {
			Model.SetSortKeyType(-1);
			Model.SetSortAtoZ(true);
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
		ListView.SetColumnState(_common.listview.columnOrder, _common.listview.columnWidth);
		ListView.SetDisplayFileSizeInByte(_confFLW.view.DisplayFileSizeInByte);
		ListView.SetDisplayPathOnly(_confFLW.view.DisplayPathOnly);
		ListView.Invalidate();
		//NOTE: sort column/key settings are not shared; these should be configured independently on each tab
	}
	void ApplySplitterState() {
		Splitter.SetSplitterPos(std::max(100, _common.splitter.treeWidth));
		Splitter.UpdateSplitterLayout();
	}
	void CopyCurrentViewState() {
		ListView.GetColumnState(_common.listview.columnOrder, _common.listview.columnWidth);
		_common.listview.style = GetListViewStyle();

		_common.splitter.treeWidth = Splitter.GetSplitterPos();
		_common.splitter.bShowTreeView = IsTreeViewVisible();
	}

public:
	CFileListTabItem(const CConfigFileListWindow &confFLW, const LF_COMPRESS_ARGS& compressArgs):
		_confFLW(confFLW),
		_passphrase(std::make_shared<CLFPassphraseGUI>()),
		Model(compressArgs, _passphrase),
		ListView(Model, confFLW),
		TreeView(Model, confFLW)
	{
		_common.initialize(confFLW);
	}
	virtual ~CFileListTabItem() { DestroyWindow(); }
	bool CreateTabItem(HWND hTabViewWnd, HWND hFrameWnd) {
		m_hFrameWnd = hFrameWnd;

		CRect rc;
		GetClientRect(hTabViewWnd, rc);
		Splitter.Create(hTabViewWnd, rc, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS);
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
		TreeView.ApplyUpdatedConfig();
	}
	bool OpenArchive(const std::filesystem::path &arcpath, const std::wstring& mutexName, HANDLE hMutex) {
		{
			CLFScanProgressHandlerGUI progress(m_hFrameWnd);
			try {
				Model.Open(arcpath, progress);
			} catch (const LF_EXCEPTION& e) {
				ErrorMessage(e.what());
				return false;
			}

			//set property to frame window
			if (hMutex) {
				//when re-opening an archive, hMutex=nullptr
				::SetPropW(m_hFrameWnd, mutexName.c_str(), this);
				_hMutex.Attach(hMutex);
				_strMutexName = mutexName;
			}

			//tree view is updated via CFileTreeView::OnFileListArchiveLoaded
		}
		SetForegroundWindow(m_hFrameWnd);
		return true;
	}

	void DestroyWindow() {
		if (TreeView.IsWindow())TreeView.DestroyWindow();
		if (ListView.IsWindow())ListView.DestroyWindow();
		if (Splitter.IsWindow())Splitter.DestroyWindow();

		_hMutex.Close();
		::RemovePropW(m_hFrameWnd, _strMutexName.c_str());
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
		ConfFLW.dimensions.TreeWidth = _common.splitter.treeWidth;
		ConfFLW.view.ShowTreeView = _common.splitter.bShowTreeView;

		//list view style
		ConfFLW.view.ListStyle = _common.listview.style;
		//column
		ConfFLW.view.column.order = _common.listview.columnOrder;
		ConfFLW.view.column.width = _common.listview.columnWidth;

		//sort status
		ConfFLW.view.SortColumnIndex = Model.GetSortKeyType();
		ConfFLW.view.SortAtoZ = Model.IsSortAtoZ();
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
