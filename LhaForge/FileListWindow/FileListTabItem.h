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
#include "../Utilities/TemporaryDirMgr.h"

struct CConfigFileListWindow;
struct CFileListTabItem{
	DISALLOW_COPY_AND_ASSIGN(CFileListTabItem);
public:
	DLL_ID idForceDLL;	//強制使用するDLL
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
	bool CreateListView(HWND hParentWnd,HWND hFrameWnd,const CConfigFileListWindow&);
	bool CreateTreeView(HWND hParentWnd,HWND hFrameWnd,const CConfigFileListWindow&);
public:
	CFileListTabItem(CConfigManager &rMan);
	virtual ~CFileListTabItem(){DestroyWindow();}
	bool CreateTabItem(HWND hParentWnd,HWND hFrameWnd,const CConfigFileListWindow&);
	HRESULT OpenArchive(LPCTSTR lpszArc,DLL_ID forceID,const CConfigFileListWindow& ConfFLW,FILELISTMODE flMode,IArchiveContentUpdateHandler* lpHandler,CString &strErr);

	void DestroyWindow();
	void ShowWindow(int nCmdShow);
	void OnActivated();
	void OnDeactivated();

	int GetTreeWidth(){return Splitter.GetSplitterPos();}
	void SetTreeWidth(int width){Splitter.SetSplitterPos(width);}

	DWORD GetListViewStyle();
	void SetListViewStyle(DWORD);
	void UpdateFileListConfig(const CConfigFileListWindow&);

	void SetSortColumn(int);

	void ShowTreeView(bool);
};
