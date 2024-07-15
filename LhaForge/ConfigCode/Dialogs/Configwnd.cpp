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
#include "ArchiverCode/archive.h"
#include "configwnd.h"
#include "Utilities/OSUtil.h"
#include "FileListWindow/MenuCommand.h"
#include "Dlg_version.h"
#include "Dlg_general.h"
#include "Dlg_compress_general.h"
#include "Dlg_extract_general.h"
#include "Dlg_assoc.h"
#include "Dlg_shortcut.h"
#include "Dlg_filelistwindow.h"
#include "Dlg_openaction.h"
#include "Dlg_format.h"


CConfigDialog::CConfigDialog(CConfigFile &cfg)
	:mr_Config(cfg),
	hActiveDialogWnd(NULL),
	m_nAssistRequireCount(0)
{
	try {
		mr_Config.load();
	} catch (const LF_EXCEPTION& e) {
		ErrorMessage(e.what());
	}
}


LRESULT CConfigDialog::OnInitDialog(HWND hWnd, LPARAM lParam)
{
	//---user common?
	if (mr_Config.isUserCommon()) {
		//set to window title
		SetWindowText(UtilLoadString(IDS_CAPTION_CONFIG_USERCOMMON).c_str());
	}

	// Large icon
	HICON hIcon = AtlLoadIconImage(IDI_APP, LR_DEFAULTCOLOR,::GetSystemMetrics(SM_CXICON),::GetSystemMetrics(SM_CYICON));
	SetIcon(hIcon, TRUE);

	// Small icon
	HICON hIconSmall = AtlLoadIconImage(IDI_APP, LR_DEFAULTCOLOR,::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON));
	SetIcon(hIconSmall, FALSE);

	//scroll container location
	CStatic StaticFrame;
	StaticFrame=GetDlgItem(IDC_STATIC_FRAME);
	RECT rect;
	StaticFrame.GetWindowRect(&rect);
	ScreenToClient(&rect);
	ScrollWindow.Create(m_hWnd, rect, NULL, WS_CHILD | WS_VISIBLE, WS_EX_CLIENTEDGE | WS_EX_CONTROLPARENT);

	//tree view; select items
	SelectTreeView = GetDlgItem(IDC_TREE_SELECT_PROPPAGE);

	auto ADD_PAGE = [&](IConfigDlgBase* _pDIALOG, HTREEITEM _ROOTITEM, const wchar_t* pTitle = nullptr) {
		m_ConfigDlgList.push_back(_pDIALOG);
		_pDIALOG->LoadConfig(mr_Config);
		_pDIALOG->Create(ScrollWindow);
		HWND hWndDlg = _pDIALOG->GetDialogHandle();
		std::wstring strTitle;
		if (pTitle) {
			strTitle = pTitle;
		} else {
			strTitle.resize(512);
			::GetWindowTextW(hWndDlg, &strTitle[0], (int)strTitle.size());
		}
		HTREEITEM hItem = SelectTreeView.InsertItem(strTitle.c_str(), _ROOTITEM, TVI_LAST);
		SelectTreeView.SetItemData(hItem, (DWORD_PTR)hWndDlg);
		return hItem;
	};

	//add dialog pages
	ADD_PAGE(new CConfigDlgGeneral(*this), TVI_ROOT);
	ADD_PAGE(new CConfigDlgShortcut,TVI_ROOT);
	ADD_PAGE(new CConfigDlgFileListWindow,TVI_ROOT);
	HTREEITEM hItemDetail = ADD_PAGE(new CConfigDlgCompressGeneral,TVI_ROOT);
	ADD_PAGE(new CConfigDlgExtractGeneral,TVI_ROOT);
	ADD_PAGE(new CConfigDlgAssociation(*this),TVI_ROOT);
	ADD_PAGE(new CConfigDlgOpenAction,TVI_ROOT);
	ADD_PAGE(new CConfigDlgVersion,TVI_ROOT);

	// format configuration
	{
		auto p = new CConfigDlgFormat;
		p->AddConfig<CConfigCompressFormatZIP>();
		ADD_PAGE(p, hItemDetail, L"ZIP");
	}
	{
		auto p = new CConfigDlgFormat;
		p->AddConfig<CConfigCompressFormat7Z>();
		ADD_PAGE(p, hItemDetail, L"7Z");
	}
	{
		auto p = new CConfigDlgFormat;
		p->AddConfig<CConfigCompressFormatTAR>();
		p->AddConfig<CConfigCompressFormatGZ>();
		p->AddConfig<CConfigCompressFormatBZ2>();
		p->AddConfig<CConfigCompressFormatXZ>();
		p->AddConfig<CConfigCompressFormatLZMA>();
		p->AddConfig<CConfigCompressFormatZSTD>();
		ADD_PAGE(p, hItemDetail, L"TAR");
	}

	// first page
	HWND hFirstPage = m_ConfigDlgList[0]->GetDialogHandle();
	::ShowWindow(hFirstPage, SW_SHOW);
	ScrollWindow.SetClient(hFirstPage);
	hActiveDialogWnd= hFirstPage;
	SelectTreeView.SetFocus();

	// init dialog resize
	DlgResize_Init(true, true, WS_THICKFRAME | WS_CLIPCHILDREN);

	CenterWindow();

	return TRUE;
}

void CConfigDialog::OnOK(UINT uNotifyCode, int nID, HWND hWndCtl)
{
	//reload latest configuration in case the file was modified by something, such as menu editor
	try {
		mr_Config.load();
	} catch(...) {
		//ignore errors
	}

	//apply settings
	bool bRet=true;
	CConfigFile assistINI;	//ini file passed to LFAssistant

	for (auto &item : m_ConfigDlgList) {
		bRet = bRet && item->OnApply();
		item->StoreConfig(mr_Config, assistINI);
	}

	//update "TEMP" environment variable
	{
		CConfigGeneral ConfGeneral;
		ConfGeneral.load(mr_Config);
		LF_setProcessTempPath(ConfGeneral.TempPath);
	}

	//request delete
	assistINI.setValue(L"PostProcess", L"DeleteMe", L"Please_Delete_Me");

	//Assistant requested to handle UAC
	if(m_nAssistRequireCount>0){
		auto tempFile = UtilGetTemporaryFileName().wstring();
		assistINI.setPath(tempFile);
		assistINI.save();

		auto strExePath = UtilGetModuleDirectoryPath() / L"LFAssist64.exe";

		std::wstring buf = (L'"' + strExePath.wstring() + L'"');

		SHELLEXECUTEINFOW shei = { 0 };
		shei.fMask = SEE_MASK_FLAG_DDEWAIT;
		shei.cbSize = sizeof(shei);
		shei.lpFile = &buf[0];
		shei.lpParameters = &tempFile[0];
		shei.nShow = SW_SHOW;
		if (!ShellExecuteExW(&shei)) {
			//failed with error
			auto strLastError = UtilGetLastErrorMessage();
			auto msg = Format(
				UtilLoadString(IDS_ERROR_CANNOT_EXECUTE),
				strExePath.c_str(),
				strLastError.c_str());
			ErrorMessage(msg);
		}

		//notify shell of association change
		Sleep(100);
		::SHChangeNotify(SHCNE_ASSOCCHANGED,SHCNF_FLUSH,NULL,NULL);
	}

	if(bRet)EndDialog(nID);
}

LRESULT CConfigDialog::OnTreeSelect(LPNMHDR pnmh)
{
	if(pnmh->hwndFrom==SelectTreeView){
		HTREEITEM hItem=SelectTreeView.GetSelectedItem();
		if(!hItem)return 0;

		HWND hSelectedDialogWnd = (HWND)SelectTreeView.GetItemData(hItem);
		if (hSelectedDialogWnd && hSelectedDialogWnd != hActiveDialogWnd) {
			SetRedraw(FALSE);
			::ShowWindow(hActiveDialogWnd,SW_HIDE);
			//ScrollWindow.SetClient(NULL);
			ScrollWindow.SetClient(hSelectedDialogWnd);
			::ShowWindow(hSelectedDialogWnd,SW_SHOW);
			hActiveDialogWnd = hSelectedDialogWnd;
			SetRedraw(TRUE);
			RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_FRAME | RDW_UPDATENOW | RDW_ALLCHILDREN);
		}
	}
	return 0;
}

LRESULT CConfigDialog::OnUserSize(UINT, WPARAM, LPARAM, BOOL& bHandled)
{
	//scroll container location
	CStatic StaticFrame;
	StaticFrame=GetDlgItem(IDC_STATIC_FRAME);
	RECT rect;
	StaticFrame.GetWindowRect(&rect);
	ScreenToClient(&rect);

	//move container window
	ScrollWindow.MoveWindow(&rect);
	return 0;
}
