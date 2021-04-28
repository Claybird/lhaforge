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

#include "stdafx.h"
#include "Dlg_shortcut.h"
#include "Compress.h"
#include "Dialogs/SelectDlg.h"
#include "Utilities/OSUtil.h"


std::tuple<std::wstring/*arg*/, std::wstring/*fname*/>
GetCompressShortcutInfo(HWND hWnd)
{
	if (IDYES == UtilMessageBox(hWnd,
		UtilLoadString(IDS_ASK_SHORTCUT_COMPRESS_TYPE_ALWAYS_ASK), MB_YESNO | MB_ICONQUESTION)) {
		int Options = -1;

		//choose format
		auto[format, options, singleCompression, deleteAfterCompress] = GUI_SelectCompressType();
		if (format == LF_FMT_INVALID)CANCEL_EXCEPTION();

		//find args
		try {
			const auto &args = get_archive_format_args(format, options);
			UINT resID;
			auto arg = L"/c:" + args.name;
			if (singleCompression) {
				resID = IDS_SHORTCUT_NAME_COMPRESS_EX_SINGLE;
				arg += L" /s";
			} else {
				resID = IDS_SHORTCUT_NAME_COMPRESS_EX;
			}
			auto fname = Format(UtilLoadString(resID), UtilLoadString(args.FormatName).c_str());

			if (deleteAfterCompress) {
				//ignored intentionally
			}
			return { arg,fname };
		} catch (const ARCHIVE_EXCEPTION&) {
			//unsupported format
			throw LF_EXCEPTION(UtilLoadString(IDS_ERROR_ILLEGAL_FORMAT_TYPE));
		}
	} else {
		//Select on every compression
		auto fname = UtilLoadString(IDS_SHORTCUT_NAME_COMPRESS);
		auto arg = L"/c";
		return { arg,fname };
	}
}


LRESULT CConfigDlgShortcut::OnCreateShortcut(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if (BN_CLICKED == wNotifyCode) {
		const auto ExePath = UtilGetModulePath();

		std::filesystem::path shortcutPath;

		// destination directory
		switch (wID) {
		case IDC_BUTTON_CREATE_COMPRESS_SHORTCUT_DESKTOP://FALLTHROUGH
		case IDC_BUTTON_CREATE_EXTRACT_SHORTCUT_DESKTOP://FALLTHROUGH
		case IDC_BUTTON_CREATE_AUTOMATIC_SHORTCUT_DESKTOP://FALLTHROUGH
		case IDC_BUTTON_CREATE_LIST_SHORTCUT_DESKTOP://FALLTHROUGH
		case IDC_BUTTON_CREATE_TESTARCHIVE_SHORTCUT_DESKTOP:
			// on desktop
			shortcutPath = UtilGetDesktopPath();
			break;
		case IDC_BUTTON_CREATE_COMPRESS_SHORTCUT_SENDTO://FALLTHROUGH
		case IDC_BUTTON_CREATE_EXTRACT_SHORTCUT_SENDTO://FALLTHROUGH
		case IDC_BUTTON_CREATE_AUTOMATIC_SHORTCUT_SENDTO://FALLTHROUGH
		case IDC_BUTTON_CREATE_LIST_SHORTCUT_SENDTO://FALLTHROUGH
		case IDC_BUTTON_CREATE_TESTARCHIVE_SHORTCUT_SENDTO:
			// in "sendto"
			shortcutPath = UtilGetSendToPath();
			break;
		default:ASSERT(!"OnCreateShortcut:this code must not be run."); return 0;
		}

		//parameters
		std::wstring arg;
		int IconIndex = -1;
		WORD DescriptionID;	//resource id for description
		switch (wID) {
		case IDC_BUTTON_CREATE_COMPRESS_SHORTCUT_DESKTOP://FALLTHROUGH
		case IDC_BUTTON_CREATE_COMPRESS_SHORTCUT_SENDTO:
			//Compress
			try {
				std::wstring filename;
				std::tie(arg, filename) = GetCompressShortcutInfo(m_hWnd);
				shortcutPath /= filename;
			} catch (const LF_USER_CANCEL_EXCEPTION&) {
				return 0;
			} catch (const LF_EXCEPTION& e) {
				ErrorMessage(e.what());
				return 0;
			}
			DescriptionID = IDS_SHORTCUT_DESCRIPTION_COMPRESS;
			IconIndex = 1;
			break;

		case IDC_BUTTON_CREATE_EXTRACT_SHORTCUT_DESKTOP://FALLTHROUGH
		case IDC_BUTTON_CREATE_EXTRACT_SHORTCUT_SENDTO:
			//Extract
			shortcutPath /= UtilLoadString(IDS_SHORTCUT_NAME_EXTRACT);
			arg = L"/e";
			DescriptionID = IDS_SHORTCUT_DESCRIPTION_EXTRACT;
			IconIndex = 2;
			break;

		case IDC_BUTTON_CREATE_AUTOMATIC_SHORTCUT_DESKTOP://FALLTHROUGH
		case IDC_BUTTON_CREATE_AUTOMATIC_SHORTCUT_SENDTO:
			//automatic detect
			shortcutPath /= UtilLoadString(IDS_SHORTCUT_NAME_AUTOMATIC);
			arg = L"";
			DescriptionID = IDS_SHORTCUT_DESCRIPTION_AUTOMATIC;
			IconIndex = 0;
			break;

		case IDC_BUTTON_CREATE_LIST_SHORTCUT_DESKTOP://FALLTHROUGH
		case IDC_BUTTON_CREATE_LIST_SHORTCUT_SENDTO:
			//List
			shortcutPath /= UtilLoadString(IDS_SHORTCUT_NAME_LIST);
			arg = L"/l";
			DescriptionID = IDS_SHORTCUT_DESCRIPTION_LIST;
			IconIndex = 3;
			break;

		case IDC_BUTTON_CREATE_TESTARCHIVE_SHORTCUT_DESKTOP://FALLTHROUGH
		case IDC_BUTTON_CREATE_TESTARCHIVE_SHORTCUT_SENDTO:
			//test
			shortcutPath /= UtilLoadString(IDS_SHORTCUT_NAME_TESTARCHIVE);
			arg = L"/t";
			DescriptionID = IDS_SHORTCUT_DESCRIPTION_TESTARCHIVE;
			IconIndex = 4;
			break;

		default:ASSERT(!"OnCreateShortcut:this code must not be run."); return 0;
		}
		//extension
		shortcutPath += L".lnk";

		if (FAILED(UtilCreateShortcut(shortcutPath, ExePath, arg, ExePath, IconIndex, UtilLoadString(DescriptionID)))) {
			ErrorMessage(UtilLoadString(IDS_ERROR_CREATE_SHORTCUT));
		} else {
			//beep when success
			MessageBeep(MB_ICONASTERISK);
		}
	}
	return 0;
}
