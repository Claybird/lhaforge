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
	const CConfigFileListWindow &_confFLW;
	const CConfigFile &_confMan;

	CSplitterWindow	Splitter;
	CFileListView	ListView;
	CFileTreeView	TreeView;

	HWND			m_hFrameWnd;

	//for detect same file opened in different window/tab
	CHandle hMutex;
	std::wstring strMutexName;
protected:
	//---internal functions
	bool CreateListView(HWND hParentWnd, HWND hFrameWnd) {
		ListView.Create(hParentWnd, CWindow::rcDefault, NULL, WS_CHILD | /*WS_VISIBLE | */WS_CLIPSIBLINGS | WS_CLIPCHILDREN | LVS_OWNERDATA | LVS_AUTOARRANGE | LVS_SHAREIMAGELISTS | LVS_SHOWSELALWAYS, WS_EX_CLIENTEDGE);
		ListView.SetFrameWnd(hFrameWnd);

		ListView.SetExtendedListViewStyle(LVS_EX_INFOTIP | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_HEADERDRAGDROP);

		if (!ListView.SetColumnState(&_confFLW.ColumnOrderArray[0], _confFLW.ColumnWidthArray))return false;

		UpdateFileListConfig();

		if (_confFLW.StoreSetting) {
			Model.SetSortKeyType(_confFLW.SortColumn);
			Model.SetSortMode(0 != _confFLW.SortDescending);
		} else {
			Model.SetSortKeyType(-1);
			Model.SetSortMode(true);
		}

		if (_confFLW.StoreSetting) {
			SetListViewStyle(_confFLW.ListStyle);
		} else {
			SetListViewStyle(LVS_ICON);
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
public:
	CFileListTabItem(const CConfigFile &rMan, const CConfigFileListWindow &confFLW):
		_confMan(rMan),
		_confFLW(confFLW),
		Model(rMan, _passphrase),
		ListView(Model, confFLW),
		TreeView(Model, confFLW){}
	virtual ~CFileListTabItem(){DestroyWindow();}
	bool CreateTabItem(HWND hParentWnd, HWND hFrameWnd) {
		m_hFrameWnd = hFrameWnd;

		CRect rc;
		GetClientRect(hParentWnd, rc);
		Splitter.Create(hParentWnd, rc, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS);
		Splitter.SetSplitterExtendedStyle(0);

		if (!CreateTreeView(Splitter, hFrameWnd))return false;
		if (!CreateListView(Splitter, hFrameWnd))return false;

		Splitter.SetSplitterPanes(TreeView, ListView);
		Splitter.SetSplitterPos(_confFLW.TreeWidth);
		Splitter.UpdateSplitterLayout();
		return true;
	}
	bool OpenArchive(const std::filesystem::path &arcpath) {
		CLFScanProgressHandlerGUI progress(m_hFrameWnd);
		try {
			Model.Open(arcpath, progress);
		} catch(const LF_EXCEPTION& e){
			ErrorMessage(e.what());
			return false;
		}

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
	}
	void ShowWindow(int nCmdShow) {
		TreeView.ShowWindow(nCmdShow);
		ListView.ShowWindow(nCmdShow);
		Splitter.ShowWindow(nCmdShow);
	}
	void OnActivated() { Model.addEventListener(m_hFrameWnd); }
	void OnDeactivated() { Model.removeEventListener(m_hFrameWnd); }

	int GetTreeWidth() { return Splitter.GetSplitterPos(); }
	void SetTreeWidth(int width) { Splitter.SetSplitterPos(width); }

	DWORD GetListViewStyle()const { return ListView.GetWindowLong(GWL_STYLE); }
	void SetListViewStyle(DWORD dwStyleNew) {
		DWORD dwStyle = ListView.GetWindowLong(GWL_STYLE);
		dwStyle &= ~(LVS_ICON | LVS_REPORT | LVS_SMALLICON | LVS_LIST);
		ListView.SetWindowLong(GWL_STYLE, dwStyle | dwStyleNew);
	}
	void UpdateFileListConfig() {
		ListView.SetDisplayFileSizeInByte(_confFLW.DisplayFileSizeInByte);
		ListView.SetDisplayPathOnly(_confFLW.DisplayPathOnly);
		ListView.Invalidate();
	}

	void SetSortColumn(int iCol) { ListView.SortItem(iCol); }

	void ShowTreeView(bool bShow) {
		if (bShow) {
			Splitter.SetSinglePaneMode(SPLIT_PANE_NONE);    // show both pane
		} else {
			Splitter.SetSinglePaneMode(SPLIT_PANE_RIGHT);   // right pane only
		}
	}
};
