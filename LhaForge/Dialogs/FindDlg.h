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
protected:
	std::wstring _path;
	std::wstring _sizeStr;
	int64_t _size;
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
		MSG_WM_COMMAND(OnRadios)
		REFLECT_COMMAND_CODE(EN_UPDATE)	//CLFBytesEdit
	END_MSG_MAP()

	LRESULT OnInitDialog(HWND hWnd, LPARAM lParam) {
		_bytesEdit.SetSubjectWindow(GetDlgItem(IDC_CONDITION_SIZE));
		_datePicker = GetDlgItem(IDC_CONDITION_DATE);
		_condition = 0;
		Button_SetCheck(GetDlgItem(IDC_BY_FILENAME), TRUE);
		DoDataExchange(FALSE);
		updateDialog();
		return TRUE;
	}
	void OnButton(UINT uNotifyCode, int nID, HWND hWndCtl) {
		DoDataExchange(TRUE);
		_datePicker.GetSystemTime(&_date);
		_size = CLFBytesEdit::ParseSize(_sizeStr);
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
		ARCHIVE_FIND_CONDITION afc = {};
		switch ((FIND_CONDITION)_condition) {
		case FIND_CONDITION::filename_match:
			afc.setFindByFilename(_path);
			break;
		case FIND_CONDITION::filepath_match:
			afc.setFindByFullpath(_path);
			break;
		case FIND_CONDITION::original_size_equal:
			afc.setFindByOriginalSize(_size, ARCHIVE_FIND_CONDITION::COMPARE::equal);
			break;
		case FIND_CONDITION::original_size_equal_or_less:
			afc.setFindByOriginalSize(_size, ARCHIVE_FIND_CONDITION::COMPARE::equalOrLess);
			break;
		case FIND_CONDITION::original_size_equal_or_greater:
			afc.setFindByOriginalSize(_size, ARCHIVE_FIND_CONDITION::COMPARE::equalOrGreater);
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
};


