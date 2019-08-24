/*
 * Copyright (c) 2005-, Claybird
 * All rights reserved.

 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:

 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Claybird nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.

 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
 * THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
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
