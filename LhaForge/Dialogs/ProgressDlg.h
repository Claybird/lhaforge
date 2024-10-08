﻿/*
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
#include "resource.h"

class CProgressDialog:public CDialogImpl<CProgressDialog>
{
protected:
	CProgressBarCtrl m_fileProgress, m_entryProgress;
	CStatic m_fileInfo, m_entryInfo, m_entrySizeInfo;
	bool m_bAbort, m_bPaused;
	int64_t m_entrySize;
	std::wstring m_entryPath;
	std::wstring m_originalTitle;
public:
	enum{IDD=IDD_DIALOG_PROGRESS};

	BEGIN_MSG_MAP_EX(CProgressDialog)
		MSG_WM_INITDIALOG(OnInitDialog)
		COMMAND_ID_HANDLER_EX(IDOK, __noop)
		COMMAND_ID_HANDLER_EX(IDCANCEL, OnAbortBtn)
		COMMAND_ID_HANDLER_EX(IDC_BUTTON_ABORT, OnAbortBtn)
		COMMAND_ID_HANDLER_EX(IDC_BUTTON_PAUSE, OnPauseBtn)
	END_MSG_MAP()

	LRESULT OnInitDialog(HWND hWnd, LPARAM lParam) {
		m_fileProgress = GetDlgItem(IDC_PROGRESS_FILE);
		m_fileInfo = GetDlgItem(IDC_STATIC_FILEINFO);
		m_entryProgress = GetDlgItem(IDC_PROGRESS_ENTRY);
		m_entryInfo = GetDlgItem(IDC_STATIC_ENTRY);
		m_entrySizeInfo = GetDlgItem(IDC_STATIC_ENTRY_SIZE);
		m_fileProgress.SetRange32(0, 100);
		m_fileProgress.SetPos(0);
		m_entryProgress.SetRange32(0, 100);
		m_entryProgress.SetPos(0);
		m_bAbort = false;
		m_bPaused = false;

		CenterWindow();
		
		CString tmp;
		GetWindowText(tmp);
		m_originalTitle = (LPCWSTR)tmp;
		return TRUE;
	}
	void SetEntry(
		const std::wstring& archivePath,
		int64_t entryIndex,
		int64_t numEntries,
		const std::wstring& entryPath,
		int64_t entrySize
	) {
		m_fileInfo.SetWindowText(archivePath.c_str());
		auto str = m_originalTitle + Format(L"[%I64d / %I64d]",
			entryIndex,
			numEntries
		);
		SetWindowText(str.c_str());
		m_fileProgress.SetPos(int(entryIndex * 100ull / std::max(1ll, numEntries)));

		m_entrySize = entrySize;
		m_entryPath = entryPath;
		m_entryInfo.SetWindowText(entryPath.c_str());
		m_entryProgress.SetPos(0);
	}
	void SetEntryProgress(int64_t currentSize) {
		if (m_entrySizeInfo.IsWindow()) {
			auto str = Format(L"%s / %s",
				UtilFormatSize(currentSize).c_str(),
				UtilFormatSize(m_entrySize).c_str()
			);
			m_entrySizeInfo.SetWindowText(str.c_str());
		}
		if (m_entryProgress.IsWindow()) {
			m_entryProgress.SetPos(int(currentSize * 100ull / std::max(1ll, m_entrySize)));
		}
	}
	void SetSpecialMessage(const std::wstring& msg) {
		if (m_entryInfo.IsWindow()) {
			m_entryInfo.SetWindowTextW(msg.c_str());
		}
	}
	LRESULT OnAbortBtn(UINT uNotifyCode, int nID, HWND hWndCtl) {
		m_bAbort = true;
		DestroyWindow();
		return 0;
	}
	LRESULT OnPauseBtn(UINT uNotifyCode, int nID, HWND hWndCtl) {
		m_bPaused = !m_bPaused;
		if (m_entryProgress.IsWindow()) {
			m_entryProgress.SetState(m_bPaused ? PBST_PAUSED : PBST_NORMAL);
		}
		return 0;
	}

	bool isAborted()const {
		return m_bAbort;
	}
	bool isPaused()const {
		return m_bPaused;
	}
};
