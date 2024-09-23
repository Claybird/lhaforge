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
#include "Dlg_compress_general.h"
#include "ArchiverCode/archive.h"
#include "Dialogs/selectdlg.h"
#include "compress.h"
#include "Utilities/CustomControl.h"

LRESULT CConfigDlgCompressGeneral::OnInitDialog(HWND hWnd, LPARAM lParam)
{
	SetDlgItemText(IDC_EDIT_COMPRESS_TO_SPECIFIC_DIR, m_Config.OutputDirUserSpecified.c_str());

	bool bActive=((int)OUTPUT_TO::SpecificDir==m_Config.OutputDirType);
	::EnableWindow(GetDlgItem(IDC_EDIT_COMPRESS_TO_SPECIFIC_DIR), bActive);
	::EnableWindow(GetDlgItem(IDC_BUTTON_COMPRESS_BROWSE_FOLDER), bActive);

	UpDown_MaxCompressFileCount=GetDlgItem(IDC_SPIN_MAX_COMPRESS_FILECOUNT);
	UpDown_MaxCompressFileCount.SetPos(m_Config.MaxCompressFileCount);
	UpDown_MaxCompressFileCount.SetRange(1,32767);
	UpDown_MaxCompressFileCount.EnableWindow(m_Config.LimitCompressFileCount);
	::EnableWindow(GetDlgItem(IDC_EDIT_MAX_COMPRESS_FILECOUNT),m_Config.LimitCompressFileCount);

	::EnableWindow(GetDlgItem(IDC_BUTTON_SELECT_DEFAULTPARAMETER),m_Config.UseDefaultParameter);

	SetParameterInfo();

	::EnableWindow(GetDlgItem(IDC_CHECK_MOVETO_RECYCLE_BIN), m_Config.DeleteAfterCompress);
	::EnableWindow(GetDlgItem(IDC_CHECK_DELETE_NOCONFIRM), m_Config.DeleteAfterCompress);

	DoDataExchange(FALSE);

	return TRUE;
}

LRESULT CConfigDlgCompressGeneral::OnApply()
{
	CString buf;
	GetDlgItemText(IDC_EDIT_COMPRESS_TO_SPECIFIC_DIR, buf);
	m_Config.OutputDirUserSpecified = (const wchar_t*)buf;

	m_Config.MaxCompressFileCount=UpDown_MaxCompressFileCount.GetPos();

	if(!DoDataExchange(TRUE)){
		return FALSE;
	}
	return TRUE;
}

LRESULT CConfigDlgCompressGeneral::OnRadioCompressTo(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if(BN_CLICKED==wNotifyCode){
		bool bActive=(0!=CButton(GetDlgItem(IDC_RADIO_COMPRESS_TO_SPECIFIC_DIR)).GetCheck());
		::EnableWindow(GetDlgItem(IDC_EDIT_COMPRESS_TO_SPECIFIC_DIR), bActive);
		::EnableWindow(GetDlgItem(IDC_BUTTON_COMPRESS_BROWSE_FOLDER), bActive);
	}
	return 0;
}

LRESULT CConfigDlgCompressGeneral::OnBrowseFolder(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if(BN_CLICKED==wNotifyCode){
		CString FolderPath;
		GetDlgItemText(IDC_EDIT_COMPRESS_TO_SPECIFIC_DIR, FolderPath);

		CLFShellFileOpenDialog dlg(FolderPath, FOS_FORCEFILESYSTEM | FOS_FILEMUSTEXIST | FOS_PATHMUSTEXIST | FOS_PICKFOLDERS);
		if(IDOK==dlg.DoModal()){
			dlg.GetFilePath(FolderPath);
			SetDlgItemText(IDC_EDIT_COMPRESS_TO_SPECIFIC_DIR, FolderPath);
		}
	}
	return 0;
}

LRESULT CConfigDlgCompressGeneral::OnCheckLimitCompressFileCount(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if(BN_CLICKED==wNotifyCode){
		BOOL State=CButton(GetDlgItem(IDC_CHECK_LIMIT_COMPRESS_FILECOUNT)).GetCheck();
		UpDown_MaxCompressFileCount.EnableWindow(State);
		::EnableWindow(GetDlgItem(IDC_EDIT_MAX_COMPRESS_FILECOUNT),State);
	}
	return 0;
}

LRESULT CConfigDlgCompressGeneral::OnCheckUseDefaultParameter(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if(BN_CLICKED==wNotifyCode){
		BOOL State=CButton(GetDlgItem(IDC_CHECK_USE_DEFAULTPARAMETER)).GetCheck();
		::EnableWindow(GetDlgItem(IDC_BUTTON_SELECT_DEFAULTPARAMETER),State);
	}
	return 0;
}

//Choose default compression parameter
LRESULT CConfigDlgCompressGeneral::OnSelectDefaultParameter(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if(BN_CLICKED==wNotifyCode){
		auto[format, options, _, __] = GUI_SelectCompressType(false, false);
		if(format == LF_ARCHIVE_FORMAT::INVALID)return 1;	//cancel

		//find suitable options
		try {
			const auto caps = CLFArchive::get_compression_capability(format);
			if (!isIn(caps.allowed_combinations, options)) {
				throw ARCHIVE_EXCEPTION(EINVAL);
			}
		} catch (const ARCHIVE_EXCEPTION& ) {
			//unknown format or unacceptable option
			ErrorMessage(UtilLoadString(IDS_ERROR_ILLEGAL_FORMAT_TYPE));
			return 1;
		}
		//save defaults
		m_Config.DefaultType=format;
		m_Config.DefaultOptions=options;

		SetParameterInfo();
		return 0;
	}
	return 0;
}


void CConfigDlgCompressGeneral::SetParameterInfo()
{
	if(LF_ARCHIVE_FORMAT::INVALID==m_Config.DefaultType){
		SetDlgItemText(IDC_EDIT_DEFAULTPARAMETER, L"");
	}else{
		try {
			const auto &args = get_archive_format_args(m_Config.DefaultType, m_Config.DefaultOptions);
			SetDlgItemText(IDC_EDIT_DEFAULTPARAMETER, UtilLoadString(args.FormatName).c_str());
		} catch (const ARCHIVE_EXCEPTION&) {
			SetDlgItemText(IDC_EDIT_DEFAULTPARAMETER, UtilLoadString(IDS_ERROR_ILLEGAL_FORMAT_TYPE).c_str());
		}
	}
}

LRESULT CConfigDlgCompressGeneral::OnCheckDelete(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if(BN_CLICKED==wNotifyCode){
		BOOL State=CButton(GetDlgItem(IDC_CHECK_DELETE_AFTER_COMPRESS)).GetCheck();
		::EnableWindow(GetDlgItem(IDC_CHECK_MOVETO_RECYCLE_BIN),State);
		::EnableWindow(GetDlgItem(IDC_CHECK_DELETE_NOCONFIRM),State);
	}
	return 0;
}
