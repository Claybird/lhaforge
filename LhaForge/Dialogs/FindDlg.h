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
#include "resource.h"
#include "Utilities/CustomControl.h"
#include "FileListWindow/ArcFileContent.h"

enum class FIND_CONDITION:int {
	filename_match,
	filepath_match,
	original_size_equal,
	original_size_equal_or_less,
	original_size_equal_or_greater,
	mdate_equal,
	mdate_equal_or_newer,
	mdate_equal_or_older,
	all_files,
	all_directories,
	everything,
};

class CLFFindDialog : public CDialogImpl<CLFFindDialog>, public LFWinDataExchange<CLFFindDialog>
{
#ifdef UNIT_TEST
	FRIEND_TEST(CLFFindDialog, getCondition);
	FRIEND_TEST(CLFFindDialog, setCondition);
#endif
protected:
	std::wstring _path;
	std::wstring _sizeStr;
	SYSTEMTIME _date;
	int/*FIND_CONDITION*/ _condition;

	CLFBytesEdit _bytesEdit;
	CDateTimePickerCtrl _datePicker;
protected:
	void updateDialog() {
		if (_condition == (int)FIND_CONDITION::filename_match
			|| _condition == (int)FIND_CONDITION::filepath_match) {
			::EnableWindow(GetDlgItem(IDC_CONDITION_PATH), TRUE);
		} else {
			::EnableWindow(GetDlgItem(IDC_CONDITION_PATH), FALSE);
		}

		if (_condition == (int)FIND_CONDITION::original_size_equal
			|| _condition == (int)FIND_CONDITION::original_size_equal_or_less
			|| _condition == (int)FIND_CONDITION::original_size_equal_or_greater) {
			::EnableWindow(GetDlgItem(IDC_CONDITION_SIZE), TRUE);
		} else {
			::EnableWindow(GetDlgItem(IDC_CONDITION_SIZE), FALSE);
		}

		if (_condition == (int)FIND_CONDITION::mdate_equal
			|| _condition == (int)FIND_CONDITION::mdate_equal_or_newer
			|| _condition == (int)FIND_CONDITION::mdate_equal_or_older) {
			::EnableWindow(GetDlgItem(IDC_CONDITION_DATE), TRUE);
		} else {
			::EnableWindow(GetDlgItem(IDC_CONDITION_DATE), FALSE);
		}
	}
public:
	enum { IDD = IDD_DIALOG_FIND };
	BEGIN_DDX_MAP(CLFFindDialog)
		DDX_TEXT(IDC_CONDITION_PATH, _path)
		DDX_TEXT(IDC_CONDITION_SIZE, _sizeStr)
		DDX_RADIO(IDC_BY_FILENAME, _condition)
	END_DDX_MAP()

	BEGIN_MSG_MAP_EX(CLFFindDialog)
		MSG_WM_INITDIALOG(OnInitDialog)
		COMMAND_ID_HANDLER_EX(IDOK, OnButton)
		COMMAND_ID_HANDLER_EX(IDCANCEL, OnButton)
		COMMAND_ID_HANDLER_EX(IDC_BY_FILENAME, OnRadios)
		COMMAND_ID_HANDLER_EX(IDC_BY_FULLPATH, OnRadios)
		COMMAND_ID_HANDLER_EX(IDC_BY_FILESIZE_EQUAL, OnRadios)
		COMMAND_ID_HANDLER_EX(IDC_BY_FILESIZE_EQUAL_OR_LESS, OnRadios)
		COMMAND_ID_HANDLER_EX(IDC_BY_FILESIZE_EQUAL_OR_GREATER, OnRadios)
		COMMAND_ID_HANDLER_EX(IDC_BY_MTIME_EQUAL, OnRadios)
		COMMAND_ID_HANDLER_EX(IDC_BY_MTIME_EQUAL_OR_GREATER, OnRadios)
		COMMAND_ID_HANDLER_EX(IDC_BY_MTIME_EQUAL_OR_OLDER, OnRadios)
		COMMAND_ID_HANDLER_EX(IDC_BY_CONDITION_FILE, OnRadios)
		COMMAND_ID_HANDLER_EX(IDC_BY_CONDITION_DIRECTORY, OnRadios)
		COMMAND_ID_HANDLER_EX(IDC_BY_CONDITION_EVERYTHING, OnRadios)
		REFLECT_COMMAND_CODE(EN_UPDATE)	//CLFBytesEdit
	END_MSG_MAP()

	CLFFindDialog(){
		GetLocalTime(&_date);
		_condition = 0;
	}

	LRESULT OnInitDialog(HWND hWnd, LPARAM lParam) {
		_bytesEdit.SetSubjectWindow(GetDlgItem(IDC_CONDITION_SIZE));
		_datePicker = GetDlgItem(IDC_CONDITION_DATE);
		_datePicker.SetSystemTime(GDT_VALID, &_date);
		DoDataExchange(FALSE);
		updateDialog();
		return TRUE;
	}
	void OnButton(UINT uNotifyCode, int nID, HWND hWndCtl) {
		DoDataExchange(TRUE);
		_datePicker.GetSystemTime(&_date);
		int64_t size = CLFBytesEdit::ParseSize(_sizeStr);
		if(nID==IDOK && size == -1) {
			if ((int)FIND_CONDITION::original_size_equal == _condition ||
				(int)FIND_CONDITION::original_size_equal_or_greater == _condition ||
				(int)FIND_CONDITION::original_size_equal_or_less == _condition) {
				MessageBeep(MB_ICONASTERISK);
				return;
			}
		}
		EndDialog(nID);
	}
	void OnRadios(UINT uNotifyCode, int nID, HWND hWndCtl) {
		auto prev = _condition;
		DoDataExchange(TRUE);
		if (_condition != prev) {
			//radio
			updateDialog();
		} else {
			SetMsgHandled(FALSE);
		}
	}
	ARCHIVE_FIND_CONDITION getCondition()const {		//valid after closed with IDOK
		int64_t size = CLFBytesEdit::ParseSize(_sizeStr);
		ARCHIVE_FIND_CONDITION afc = {};
		switch ((FIND_CONDITION)_condition) {
		case FIND_CONDITION::filename_match:
			afc.setFindByFilename(_path);
			break;
		case FIND_CONDITION::filepath_match:
			afc.setFindByFullpath(_path);
			break;
		case FIND_CONDITION::original_size_equal:
			afc.setFindByOriginalSize(size, ARCHIVE_FIND_CONDITION::COMPARE::equal);
			break;
		case FIND_CONDITION::original_size_equal_or_less:
			afc.setFindByOriginalSize(size, ARCHIVE_FIND_CONDITION::COMPARE::equalOrLess);
			break;
		case FIND_CONDITION::original_size_equal_or_greater:
			afc.setFindByOriginalSize(size, ARCHIVE_FIND_CONDITION::COMPARE::equalOrGreater);
			break;
		case FIND_CONDITION::mdate_equal:
			afc.setFindByMDate(_date, ARCHIVE_FIND_CONDITION::COMPARE::equal);
			break;
		case FIND_CONDITION::mdate_equal_or_newer:
			afc.setFindByMDate(_date, ARCHIVE_FIND_CONDITION::COMPARE::equalOrGreater);
			break;
		case FIND_CONDITION::mdate_equal_or_older:
			afc.setFindByMDate(_date, ARCHIVE_FIND_CONDITION::COMPARE::equalOrLess);
			break;
		case FIND_CONDITION::all_files:
			afc.setFindByMode(S_IFREG);
			break;
		case FIND_CONDITION::all_directories:
			afc.setFindByMode(S_IFDIR);
			break;
		case FIND_CONDITION::everything:
		default:
			afc.setFindByFilename(L"*");
			break;
		}

		return afc;
	}
	void setCondition(const ARCHIVE_FIND_CONDITION& afc) {
		switch (afc.key) {
		case ARCHIVE_FIND_CONDITION::KEY::filename:
			if (afc.patternStr == L"*" || afc.patternStr == L"*.*") {
				_condition = (int)FIND_CONDITION::everything;
			} else {
				_condition = (int)FIND_CONDITION::filename_match;
				_path = afc.patternStr;
			}
			break;
		case ARCHIVE_FIND_CONDITION::KEY::fullpath:
			if (afc.patternStr == L"*" || afc.patternStr == L"*.*") {
				_condition = (int)FIND_CONDITION::everything;
			} else {
				_condition = (int)FIND_CONDITION::filepath_match;
				_path = afc.patternStr;
			}
			break;
		case ARCHIVE_FIND_CONDITION::KEY::originalSize:
			switch (afc.compare) {
			case ARCHIVE_FIND_CONDITION::COMPARE::equalOrGreater:
				_condition = (int)FIND_CONDITION::original_size_equal_or_greater;
				break;
			case ARCHIVE_FIND_CONDITION::COMPARE::equalOrLess:
				_condition = (int)FIND_CONDITION::original_size_equal_or_less;
				break;
			case ARCHIVE_FIND_CONDITION::COMPARE::equal:
			default:
				_condition = (int)FIND_CONDITION::original_size_equal;
				break;
			}
			_sizeStr = UtilFormatSizeStrict(afc.st_size);
			break;
		case ARCHIVE_FIND_CONDITION::KEY::mdate:
			switch (afc.compare) {
			case ARCHIVE_FIND_CONDITION::COMPARE::equalOrGreater:
				_condition = (int)FIND_CONDITION::mdate_equal_or_newer;
				break;
			case ARCHIVE_FIND_CONDITION::COMPARE::equalOrLess:
				_condition = (int)FIND_CONDITION::mdate_equal_or_older;
				break;
			case ARCHIVE_FIND_CONDITION::COMPARE::equal:
			default:
				_condition = (int)FIND_CONDITION::mdate_equal;
				break;
			}
			_date = afc.mdate;
			break;
		case ARCHIVE_FIND_CONDITION::KEY::mode:
			if(afc.st_mode_mask == S_IFDIR) {
				_condition = (int)FIND_CONDITION::all_directories;
			} else {
				_condition = (int)FIND_CONDITION::all_files;
			}
			break;
		}
	}
};


