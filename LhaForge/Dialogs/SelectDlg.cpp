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
#include "SelectDlg.h"
#include "compress.h"
#include "resource.h"

//select compression mode
class CSelectDialog : public CDialogImpl<CSelectDialog>, public CWinDataExchange<CSelectDialog>
{
protected:
	bool bPassword;
	bool bSingleCompression;
	bool bDeleteAfterCompress;
	const bool bEnableSingleCompression;
	const bool bEnableDeletion;

	BEGIN_DDX_MAP(CSelectDialog)
		DDX_CHECK(IDC_CHECK_COMPRESS_PASSWORD, bPassword)
		DDX_CHECK(IDC_CHECK_SINGLE_COMPRESSION, bSingleCompression)
		DDX_CHECK(IDC_CHECK_DELETE_AFTER_COMPRESS, bDeleteAfterCompress)
	END_DDX_MAP()

	BEGIN_MSG_MAP_EX(CSelectDialog)
		MSG_WM_INITDIALOG(OnInitDialog)
		COMMAND_ID_HANDLER(IDC_CHECK_COMPRESS_PASSWORD, OnPassword)
		MSG_WM_COMMAND(OnCommand)
	END_MSG_MAP()

	LRESULT OnPassword(WORD, WORD, HWND, BOOL&) {
		DoDataExchange(TRUE);
		return 0;
	}
	LRESULT OnInitDialog(HWND hWnd, LPARAM lParam) {
		DoDataExchange(FALSE);

		::EnableWindow(GetDlgItem(IDC_CHECK_SINGLE_COMPRESSION), bEnableSingleCompression);
		::EnableWindow(GetDlgItem(IDC_CHECK_DELETE_AFTER_COMPRESS), bEnableDeletion);

		CenterWindow();
		return TRUE;
	}
	void OnCommand(UINT nCode, int nID, HWND hWnd) {
#define BUTTON_PARAM(x,y) case IDC_BUTTON_FORMAT_##x: format=LF_ARCHIVE_FORMAT::##y;break
		DoDataExchange(TRUE);
		LF_ARCHIVE_FORMAT format;
		switch (nID) {
			BUTTON_PARAM(ZIP, ZIP);
			BUTTON_PARAM(7Z, _7Z);
			BUTTON_PARAM(GZ, GZ);
			BUTTON_PARAM(BZ2, BZ2);
			BUTTON_PARAM(LZMA, LZMA);
			BUTTON_PARAM(XZ, XZ);
			BUTTON_PARAM(ZSTD, ZSTD);
			BUTTON_PARAM(LZ4, LZ4);
			BUTTON_PARAM(TAR, TAR);
			BUTTON_PARAM(TAR_GZ, TAR_GZ);
			BUTTON_PARAM(TAR_BZ2, TAR_BZ2);
			BUTTON_PARAM(TAR_LZMA, TAR_LZMA);
			BUTTON_PARAM(TAR_XZ, TAR_XZ);
			BUTTON_PARAM(TAR_ZSTD, TAR_ZSTD);
			BUTTON_PARAM(TAR_LZ4, TAR_LZ4);
		case IDCANCEL:
			format = LF_ARCHIVE_FORMAT::INVALID;
			break;
		default:
			return;
		}
		EndDialog((int)format);
	}

public:
	enum { IDD = IDD_DIALOG_SELECT_COMPRESS_TYPE };
	CSelectDialog(bool enableSingleCompression, bool enableDeletion) :
		bPassword(false),
		bSingleCompression(false),
		bDeleteAfterCompress(false),
		bEnableSingleCompression(enableSingleCompression),
		bEnableDeletion(enableDeletion){}
	virtual ~CSelectDialog() {}

	LF_WRITE_OPTIONS GetOptions() {
		int Options = LF_WOPT_STANDARD;
		if(bPassword){
			Options|= LF_WOPT_DATA_ENCRYPTION;
		}
		return (LF_WRITE_OPTIONS)Options;
	}
	bool IsSingleCompression()const { return bSingleCompression; }
	bool GetDeleteAfterCompress()const { return bDeleteAfterCompress; }
};

//---------------------------------------------------------------

std::tuple<LF_ARCHIVE_FORMAT,
	LF_WRITE_OPTIONS,
	bool /*singleCompression*/,
	bool /*deleteAfterCompress*/>
GUI_SelectCompressType(bool enableSingleCompression, bool enableDeletion)
{
	CSelectDialog SelDlg(enableSingleCompression, enableDeletion);
	auto format = (LF_ARCHIVE_FORMAT)SelDlg.DoModal();
	return { format, SelDlg.GetOptions(), SelDlg.IsSingleCompression(), SelDlg.GetDeleteAfterCompress() };
}
