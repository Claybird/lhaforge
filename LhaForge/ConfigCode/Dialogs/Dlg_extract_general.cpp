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
#include "Dlg_extract_general.h"
#include "Utilities/CustomControl.h"
#include "extract.h"

LRESULT CConfigDlgExtractGeneral::OnInitDialog(HWND hWnd, LPARAM lParam)
{
	{
		bool bActive = ((int)OUTPUT_TO::SpecificDir == m_Config.OutputDirType);
		SetDlgItemText(IDC_EDIT_EXTRACT_TO_SPECIFIC_DIR, m_Config.OutputDirUserSpecified.c_str());
		::EnableWindow(GetDlgItem(IDC_EDIT_EXTRACT_TO_SPECIFIC_DIR), bActive);
		::EnableWindow(GetDlgItem(IDC_BUTTON_EXTRACT_BROWSE_FOLDER), bActive);
	}

	{
		bool bActive = (int(EXTRACT_CREATE_DIR::Never) != m_Config.CreateDir);
		::EnableWindow(GetDlgItem(IDC_CHECK_REMOVE_SYMBOL_AND_NUMBER), bActive);
	}

	UpDown_MaxExtractFileCount=GetDlgItem(IDC_SPIN_MAX_EXTRACT_FILECOUNT);
	UpDown_MaxExtractFileCount.SetPos(m_Config.MaxExtractFileCount);
	UpDown_MaxExtractFileCount.SetRange(1,32767);
	UpDown_MaxExtractFileCount.EnableWindow(m_Config.LimitExtractFileCount);
	::EnableWindow(GetDlgItem(IDC_EDIT_MAX_EXTRACT_FILECOUNT),m_Config.LimitExtractFileCount);

	//delete archive after extract
	::EnableWindow(GetDlgItem(IDC_CHECK_MOVETO_RECYCLE_BIN), m_Config.DeleteArchiveAfterExtract);
	::EnableWindow(GetDlgItem(IDC_CHECK_DELETE_NOCONFIRM), m_Config.DeleteArchiveAfterExtract);

	DoDataExchange(FALSE);
	return TRUE;
}

LRESULT CConfigDlgExtractGeneral::OnApply()
{
	CString buf;
	GetDlgItemText(IDC_EDIT_EXTRACT_TO_SPECIFIC_DIR, buf);
	m_Config.OutputDirUserSpecified = (const wchar_t*)buf;

	m_Config.MaxExtractFileCount=UpDown_MaxExtractFileCount.GetPos();

	if(!DoDataExchange(TRUE)){
		return FALSE;
	}

	return TRUE;
}

LRESULT CConfigDlgExtractGeneral::OnRadioExtractTo(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if(BN_CLICKED==wNotifyCode){
		bool bActive = (0 != CButton(GetDlgItem(IDC_RADIO_EXTRACT_TO_SPECIFIC_DIR)).GetCheck());
		::EnableWindow(GetDlgItem(IDC_EDIT_EXTRACT_TO_SPECIFIC_DIR), bActive);
		::EnableWindow(GetDlgItem(IDC_BUTTON_EXTRACT_BROWSE_FOLDER), bActive);
	}
	return 0;
}

LRESULT CConfigDlgExtractGeneral::OnBrowseFolder(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if(BN_CLICKED==wNotifyCode){
		CString FolderPath;
		GetDlgItemText(IDC_EDIT_EXTRACT_TO_SPECIFIC_DIR, FolderPath);

		CLFShellFileOpenDialog dlg(FolderPath, FOS_FORCEFILESYSTEM | FOS_FILEMUSTEXIST | FOS_PATHMUSTEXIST | FOS_PICKFOLDERS);
		if(IDOK==dlg.DoModal()){
			dlg.GetFilePath(FolderPath);
			SetDlgItemText(IDC_EDIT_EXTRACT_TO_SPECIFIC_DIR, FolderPath);
		}
	}
	return 0;
}


LRESULT CConfigDlgExtractGeneral::OnCheckLimitExtractFileCount(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if(BN_CLICKED==wNotifyCode){
		BOOL State = CButton(GetDlgItem(IDC_CHECK_LIMIT_EXTRACT_FILECOUNT)).GetCheck();
		UpDown_MaxExtractFileCount.EnableWindow(State);
		::EnableWindow(GetDlgItem(IDC_EDIT_MAX_EXTRACT_FILECOUNT),State);
	}
	return 0;
}

LRESULT CConfigDlgExtractGeneral::OnRadioCreateDirectory(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if(BN_CLICKED==wNotifyCode){
		bool bActive = (!CButton(GetDlgItem(IDC_RADIO_CREATE_NO_FOLDER)).GetCheck());
		::EnableWindow(GetDlgItem(IDC_CHECK_REMOVE_SYMBOL_AND_NUMBER),bActive);
	}
	return 0;
}

LRESULT CConfigDlgExtractGeneral::OnCheckDeleteArchive(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if(BN_CLICKED==wNotifyCode){
		BOOL State = CButton(GetDlgItem(IDC_CHECK_DELETE_ARCHIVE_AFTER_EXTRACT)).GetCheck();
		::EnableWindow(GetDlgItem(IDC_CHECK_MOVETO_RECYCLE_BIN),State);
		::EnableWindow(GetDlgItem(IDC_CHECK_DELETE_NOCONFIRM),State);
	}
	return 0;
}
