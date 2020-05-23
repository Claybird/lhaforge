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
#include "../resource.h"

class CProgressDialog:public CDialogImpl<CProgressDialog>,public CMessageFilter
{
protected:
	CProgressBarCtrl m_fileProgress, m_entryProgress;
	CStatic m_fileInfo, m_entryInfo;
public:
	enum{IDD=IDD_DIALOG_PROGRESS};

	BEGIN_MSG_MAP_EX(CProgressDialog)
		MSG_WM_INITDIALOG(OnInitDialog)
		MSG_WM_DESTROY(OnDestroy)
		COMMAND_HANDLER(IDC_BUTTON_ABORT, BN_CLICKED, OnAbortBtn)
	END_MSG_MAP()

	LRESULT OnInitDialog(HWND hWnd, LPARAM lParam) {
		m_fileProgress = GetDlgItem(IDC_PROGRESS_FILE);
		m_fileInfo = GetDlgItem(IDC_STATIC_FILEINFO);
		m_entryProgress = GetDlgItem(IDC_PROGRESS_ENTRY);
		m_entryInfo = GetDlgItem(IDC_STATIC_ENTRY);
		m_fileProgress.SetRange32(0, 100);
		m_fileProgress.SetPos(0);
		m_entryProgress.SetRange32(0, 100);
		m_entryProgress.SetPos(0);

		//SetWindowPos(NULL, 100, 100, 0, 0, SWP_NOSIZE);
		CenterWindow();

		CMessageLoop* pLoop = _Module.GetMessageLoop();
		pLoop->AddMessageFilter(this);
		return TRUE;
	}
	void SetProgress(
		const std::wstring& archivePath,
		UINT64 fileIndex,
		UINT64 totalFiles, 
		const std::wstring& originalPath,
		UINT64 currentSize,
		UINT64 totalSize
		) {
		auto str = Format(L"%s\n%I64d / %I64d",
			archivePath.c_str(),
			fileIndex,
			totalFiles
		);
		m_fileInfo.SetWindowTextW(
			str.c_str());
		m_fileProgress.SetPos(INT32(fileIndex * 100ull / totalFiles));

		str = Format(L"%s\n%s / %s",
			originalPath.c_str(),
			UtilFormatSize(currentSize).c_str(),
			UtilFormatSize(totalSize).c_str()
		);
		m_entryInfo.SetWindowTextW(
			str.c_str());
		m_entryProgress.SetPos(INT32(currentSize * 100ull / std::max(1ull, totalSize)));
	}
	LRESULT OnAbortBtn(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		CANCEL_EXCEPTION();
		return 0;
	}
	void OnDestroy() {
		CMessageLoop* pLoop = _Module.GetMessageLoop();
		pLoop->RemoveMessageFilter(this);
	}

	virtual BOOL PreTranslateMessage(MSG* pMsg){
		return IsDialogMessage(pMsg);
	}
};
