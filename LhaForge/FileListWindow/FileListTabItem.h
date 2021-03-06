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

struct CConfigFileListWindow;
struct CFileListTabItem{
	DISALLOW_COPY_AND_ASSIGN(CFileListTabItem);
public:
	CLFPassphraseGUI _passphrase;
	CFileListModel	Model;

	CSplitterWindow	Splitter;
	CFileListView	ListView;
	CFileTreeView	TreeView;

	HWND			m_hFrameWnd;

	//for detect same file opened in different window/tab
	HANDLE hMutex;
	std::wstring strMutexName;
protected:
	//---internal functions
	bool CreateListView(HWND hParentWnd, HWND hFrameWnd, const CConfigFileListWindow& ConfFLW) {
		ListView.Create(hParentWnd, CWindow::rcDefault, NULL, WS_CHILD | /*WS_VISIBLE | */WS_CLIPSIBLINGS | WS_CLIPCHILDREN | LVS_OWNERDATA | LVS_AUTOARRANGE | LVS_SHAREIMAGELISTS | LVS_SHOWSELALWAYS, WS_EX_CLIENTEDGE);
		ListView.SetFrameWnd(hFrameWnd);

		ListView.SetExtendedListViewStyle(LVS_EX_INFOTIP | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_HEADERDRAGDROP);

		if (!ListView.SetColumnState(ConfFLW.ColumnOrderArray, ConfFLW.ColumnWidthArray))return false;

		UpdateFileListConfig(ConfFLW);

		if (ConfFLW.StoreSetting) {
			Model.SetSortKeyType(ConfFLW.SortColumn);
			Model.SetSortMode(0 != ConfFLW.SortDescending);
		} else {
			Model.SetSortKeyType(-1);
			Model.SetSortMode(true);
		}

		if (ConfFLW.StoreSetting) {
			DWORD Style = ListView.GetWindowLong(GWL_STYLE);
			Style &= ~(LVS_ICON | LVS_REPORT | LVS_SMALLICON | LVS_LIST);
			ListView.SetWindowLong(GWL_STYLE, Style | ConfFLW.ListStyle);
		} else {
			DWORD Style = ListView.GetWindowLong(GWL_STYLE);
			Style &= ~(LVS_ICON | LVS_REPORT | LVS_SMALLICON | LVS_LIST);
			ListView.SetWindowLong(GWL_STYLE, Style | LVS_ICON);
		}
		return true;
	}
	bool CreateTreeView(HWND hParentWnd, HWND hFrameWnd, const CConfigFileListWindow&) {
		TreeView.Create(hParentWnd, CWindow::rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | TVS_HASBUTTONS | TVS_HASLINES | TVS_LINESATROOT | TVS_SHOWSELALWAYS, WS_EX_CLIENTEDGE);
		TreeView.SetFrameWnd(hFrameWnd);
		return true;
	}
public:
	CFileListTabItem(CConfigManager &rMan): Model(rMan, _passphrase),
		ListView(rMan, Model),
		TreeView(Model),
		hMutex(NULL){}
	virtual ~CFileListTabItem(){DestroyWindow();}
	bool CreateTabItem(HWND hParentWnd, HWND hFrameWnd, const CConfigFileListWindow &ConfFLW) {
		m_hFrameWnd = hFrameWnd;

		CRect rc;
		GetClientRect(hParentWnd, rc);
		Splitter.Create(hParentWnd, rc, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS);
		Splitter.SetSplitterExtendedStyle(0);

		if (!CreateTreeView(Splitter, hFrameWnd, ConfFLW))return false;
		if (!CreateListView(Splitter, hFrameWnd, ConfFLW))return false;

		Splitter.SetSplitterPanes(TreeView, ListView);
		Splitter.SetSplitterPos(ConfFLW.TreeWidth);
		Splitter.UpdateSplitterLayout();
		return true;
	}
	void OpenArchive(const std::filesystem::path &arcpath, const CConfigFileListWindow& ConfFLW) {
		CLFScanProgressHandlerGUI progress(m_hFrameWnd);
		Model.Open(arcpath, progress);

		//construct tree view structure
		TreeView.ConstructTree();
		if (ConfFLW.ExpandTree)TreeView.ExpandTree();
	}

	void DestroyWindow() {
		if (TreeView.IsWindow())TreeView.DestroyWindow();
		if (ListView.IsWindow())ListView.DestroyWindow();
		if (Splitter.IsWindow())Splitter.DestroyWindow();

		if (hMutex) {
			CloseHandle(hMutex);
			hMutex = NULL;
		}
	}
	void ShowWindow(int nCmdShow) {
		TreeView.ShowWindow(nCmdShow);
		ListView.ShowWindow(nCmdShow);
		Splitter.ShowWindow(nCmdShow);
	}
	void OnActivated() {
		Model.addEventListener(m_hFrameWnd);
	}
	void OnDeactivated() {
		Model.removeEventListener(m_hFrameWnd);
	}

	int GetTreeWidth(){return Splitter.GetSplitterPos();}
	void SetTreeWidth(int width){Splitter.SetSplitterPos(width);}

	DWORD GetListViewStyle()const {
		return ListView.GetWindowLong(GWL_STYLE);
	}
	void SetListViewStyle(DWORD dwStyleNew) {
		DWORD dwStyle = ListView.GetWindowLong(GWL_STYLE);
		dwStyle &= ~(LVS_ICON | LVS_REPORT | LVS_SMALLICON | LVS_LIST);
		ListView.SetWindowLong(GWL_STYLE, dwStyle | dwStyleNew);
	}
	void UpdateFileListConfig(const CConfigFileListWindow& ConfFLW) {
		ListView.SetDisplayFileSizeInByte(ConfFLW.DisplayFileSizeInByte);
		ListView.SetDisplayPathOnly(ConfFLW.DisplayPathOnly);
		ListView.Invalidate();
	}

	void SetSortColumn(int iCol) {
		ListView.SortItem(iCol);
	}

	void ShowTreeView(bool bShow) {
		if (bShow) {
			Splitter.SetSinglePaneMode(SPLIT_PANE_NONE);    // show both pane
		} else {
			Splitter.SetSinglePaneMode(SPLIT_PANE_RIGHT);   // right pane only
		}
	}
};
