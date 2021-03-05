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
	CLFScanProgressHandlerGUI _progress;
	CFileListModel	Model;

	CSplitterWindow	Splitter;	// スプリッタウィンドウ
	CFileListView	ListView;
	CFileTreeView	TreeView;	// フォルダのツリービュー

	HWND			m_hFrameWnd;

	//重複open阻止用
	HANDLE hMutex;	//NULL以外なら、自動的に閉じる
	CString strMutexName;
protected:
	//---internal functions
	bool CreateListView(HWND hParentWnd, HWND hFrameWnd, const CConfigFileListWindow& ConfFLW) {
		//--ファイル一覧ウィンドウ作成
		ListView.Create(hParentWnd, CWindow::rcDefault, NULL, WS_CHILD | /*WS_VISIBLE | */WS_CLIPSIBLINGS | WS_CLIPCHILDREN | LVS_OWNERDATA | LVS_AUTOARRANGE | LVS_SHAREIMAGELISTS | LVS_SHOWSELALWAYS, WS_EX_CLIENTEDGE);
		//フレームウィンドウのハンドルを教える
		ListView.SetFrameWnd(hFrameWnd);

		//スタイル設定
		ListView.SetExtendedListViewStyle(LVS_EX_INFOTIP | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_HEADERDRAGDROP);

		//リストビューにカラム追加
		if (!ListView.SetColumnState(ConfFLW.ColumnOrderArray, ConfFLW.ColumnWidthArray))return false;

		//表示設定
		UpdateFileListConfig(ConfFLW);

		//ソート設定
		if (ConfFLW.StoreSetting) {
			Model.SetSortKeyType(ConfFLW.SortColumn);
			Model.SetSortMode(0 != ConfFLW.SortDescending);
		} else {
			Model.SetSortKeyType(-1);
			Model.SetSortMode(true);
		}

		//リストビュースタイルの設定
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
		//ツリービュー作成
		TreeView.Create(hParentWnd, CWindow::rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | TVS_HASBUTTONS | TVS_HASLINES | TVS_LINESATROOT | TVS_SHOWSELALWAYS, WS_EX_CLIENTEDGE);

		//フレームウィンドウのハンドルを教える
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

		// スプリッタウィンドウを作成
		CRect rc;
		GetClientRect(hParentWnd, rc);
		Splitter.Create(hParentWnd, rc, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS);
		// スプリッタウィンドウ拡張スタイルを設定
		Splitter.SetSplitterExtendedStyle(0);

		//---ツリービュー
		if (!CreateTreeView(Splitter, hFrameWnd, ConfFLW))return false;
		//---リストビュー
		if (!CreateListView(Splitter, hFrameWnd, ConfFLW))return false;

		//分割ウィンドウに設定
		Splitter.SetSplitterPanes(TreeView, ListView);
		// 分割バーの位置を設定
		Splitter.SetSplitterPos(ConfFLW.TreeWidth);
		Splitter.UpdateSplitterLayout();
		return true;
	}
	HRESULT OpenArchive(const std::filesystem::path &arcpath, const CConfigFileListWindow& ConfFLW) {
		//---解析
		HRESULT hr = Model.Open(arcpath, _progress);

		if (SUCCEEDED(hr)) {
			//ツリー構築
			TreeView.ConstructTree();
			while (UtilDoMessageLoop())continue;	//ここでメッセージループを回さないとツリーアイテムが有効にならない
			if (ConfFLW.ExpandTree)TreeView.ExpandTree();
		}
		return hr;
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
		//---フレームウィンドウをイベントリスナに登録
		Model.addEventListener(m_hFrameWnd);
	}
	void OnDeactivated() {
		//---フレームウィンドウをイベントリスナから解除
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
		//表示設定
		ListView.SetDisplayFileSizeInByte(BOOL2bool(ConfFLW.DisplayFileSizeInByte));
		ListView.SetDisplayPathOnly(BOOL2bool(ConfFLW.DisplayPathOnly));
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
