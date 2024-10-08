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

class CConfirmOverwriteInArchiveDialog :public CDialogImpl<CConfirmOverwriteInArchiveDialog>, public CWinDataExchange<CConfirmOverwriteInArchiveDialog>
{
protected:
	CString _extracting_filename, _extracting_filesize, _extracting_filetime;
	CString _existing_filename, _existing_filesize, _existing_filetime;
public:
	enum{IDD=IDD_DIALOG_CONFIRM_OVERWRITE_IN_ARCHIVE};

	// DDX map
	BEGIN_DDX_MAP(CConfirmOverwriteInArchiveDialog)
		DDX_TEXT(IDC_EXTRACTING_FILENAME, _extracting_filename)
		DDX_TEXT(IDC_EXTRACTING_FILESIZE, _extracting_filesize)
		DDX_TEXT(IDC_EXTRACTING_FILETIME, _extracting_filetime)
		DDX_TEXT(IDC_EXISTING_FILENAME, _existing_filename)
		DDX_TEXT(IDC_EXISTING_FILESIZE, _existing_filesize)
		DDX_TEXT(IDC_EXISTING_FILETIME, _existing_filetime)
	END_DDX_MAP()

	BEGIN_MSG_MAP_EX(CConfirmOverwriteInArchiveDialog)
		MSG_WM_INITDIALOG(OnInitDialog)
		COMMAND_ID_HANDLER_EX(IDC_BUTTON_EXTRACT_OVERWRITE, OnButton)
		COMMAND_ID_HANDLER_EX(IDC_BUTTON_EXTRACT_OVERWRITE_ALL, OnButton)
		COMMAND_ID_HANDLER_EX(IDC_BUTTON_EXTRACT_SKIP, OnButton)
		COMMAND_ID_HANDLER_EX(IDC_BUTTON_EXTRACT_SKIP_ALL, OnButton)
		COMMAND_ID_HANDLER_EX(IDC_BUTTON_EXTRACT_ABORT, OnButton)
	END_MSG_MAP()

	LRESULT OnInitDialog(HWND hWnd, LPARAM lParam) {
		CenterWindow();
		//apply data to dialog control
		DoDataExchange(FALSE);
		return TRUE;
	}
	void OnButton(UINT uNotifyCode, int nID, HWND hWndCtl) {
		EndDialog(nID);
	}

	void SetFileInfo(
		const std::wstring& extracting_file_path,
		UINT64 extracting_file_size,
		__time64_t extracting_file_mtime,
		const std::wstring& existing_file_path,
		UINT64 existing_file_size,
		__time64_t existing_file_mtime
	) {
		_extracting_filename = extracting_file_path.c_str();
		_extracting_filesize = (
			UtilFormatSize(extracting_file_size) + Format(L" (%llu Bytes)", extracting_file_size)
			).c_str();
		_extracting_filetime = UtilFormatTime(extracting_file_mtime).c_str();
		_existing_filename = existing_file_path.c_str();
		_existing_filesize =(
			UtilFormatSize(existing_file_size) + Format(L" (%llu Bytes)", existing_file_size)
			).c_str();
		_existing_filetime = UtilFormatTime(existing_file_mtime).c_str();
	}
};
