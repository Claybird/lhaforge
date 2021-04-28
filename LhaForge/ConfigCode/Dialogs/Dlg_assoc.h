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
#include "Dlg_Base.h"
#include "resource.h"
#include "Utilities/FileOperation.h"
#include "Association.h"

enum ASSOC_TYPE{
	ASSOC_LZH,
	ASSOC_LZS,
	ASSOC_LHA,

	ASSOC_ZIP,
	ASSOC_JAR,
	ASSOC_CAB,

	ASSOC_7Z,

	ASSOC_ARJ,
	ASSOC_RAR,
	ASSOC_ACE,

	ASSOC_BZA,
	ASSOC_GZA,

	ASSOC_UUE,

	ASSOC_TAR,
	ASSOC_GZ,
	ASSOC_BZ2,
	ASSOC_XZ,
	ASSOC_LZMA,
	ASSOC_ZSTD,
	ASSOC_Z,
	ASSOC_TGZ,
	ASSOC_TBZ,
	ASSOC_TAR_XZ,
	ASSOC_TAR_LZMA,
	//ASSOC_TAR_ZSTD,
	ASSOC_TAZ,
	ASSOC_CPIO,
	ASSOC_RPM,
	ASSOC_DEB,
	ASSOC_ISO,

	ENUM_COUNT_AND_LASTITEM(ASSOC_TYPE),
};

const int NO_DEFAULT_ASSOCS[]={
	ASSOC_JAR,
	ASSOC_ISO,
};


class ASSOC_SETTINGS {
protected:
	CIcon		Icon;
public:
	int DefaultIconIndex;
	CStatic		Picture_Icon;
	CButton		Button_SetIcon;
	CButton		Check_SetAssoc;
	ASSOCINFO	AssocInfo;
	bool		bChanged;

	ASSOC_SETTINGS() :bChanged(false) {}
	virtual ~ASSOC_SETTINGS() {}

	void SetIconFromAssoc(CIcon &IconSystemDefault){
		if (AssocInfo.prevIconFile.empty()) {
			SetIcon(IconSystemDefault);
		} else {
			SetIcon(AssocInfo.prevIconFile, AssocInfo.prevIconIndex);
		}
	}

	void SetIcon(CIcon &icon){
		Picture_Icon.SetIcon(icon);
	}
	void SetIcon(const std::filesystem::path &path, int idx) {
		if (!Icon.IsNull())Icon.DestroyIcon();
		Icon.ExtractIcon(path.c_str(), idx);
		Picture_Icon.SetIcon(Icon);
	}
	void CheckAssociation(const std::wstring& desiredCommand) {
		bChanged = false;
		if (AssocInfo.isAssociated) {	//Not Associated
			//current association is same as desired one?
			if (desiredCommand != AssocInfo.ShellOpenCommand) {
				bChanged = true;
			}
		}
	}
};



class CConfigDialog;
class CConfigDlgAssociation : public LFConfigDialogBase<CConfigDlgAssociation>
{
protected:
	std::array<ASSOC_SETTINGS, ASSOC_TYPE_ITEM_COUNT> AssocSettings;

	//expected program path; if different, set flag to modify associated program
	std::wstring m_assocDesired;

	CIcon Icon_SystemDefault;	//file icon that has no association

	CConfigDialog	&mr_ConfigDlg;

	struct KVPAIR {
		std::wstring key, value;
	};
	std::map<std::wstring, std::vector<KVPAIR>> _assocRequests;

public:
	enum { IDD = IDD_PROPPAGE_CONFIG_ASSOCIATION };

	BEGIN_MSG_MAP_EX(CConfigDlgAssociation)
		MSG_WM_INITDIALOG(OnInitDialog)
		MESSAGE_HANDLER(WM_COMMAND, OnCommand)	//switches to suitable function
		COMMAND_RANGE_HANDLER(IDC_BUTTON_ASSOC_CHECK_ALL, IDC_BUTTON_ASSOC_SET_EXTERNAL_ICON_SINGLE, OnSetAssoc)
	END_MSG_MAP()


	LRESULT OnInitDialog(HWND hWnd, LPARAM lParam);
	LRESULT OnCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	void OnCheckAssoc(ASSOC_SETTINGS&);
	void OnChangeIcon(ASSOC_SETTINGS&);
	LRESULT OnSetAssoc(WORD,WORD,HWND,BOOL&);
	LRESULT OnApply();

	CConfigDlgAssociation(CConfigDialog &dlg):mr_ConfigDlg(dlg){}

	void LoadConfig(CConfigFile& Config){}
	void StoreConfig(CConfigFile& Config, CConfigFile& assistant);
};

#include "Utilities/OSUtil.h"
class CIconSelectDialog : public CDialogImpl<CIconSelectDialog>,public LFWinDataExchange<CIconSelectDialog>,public CDialogResize<CIconSelectDialog>
{
protected:
	ASSOCINFO *AssocInfo;
	CIconSelectDialog();
	CImageList	IconList;
	CListViewCtrl ListView;
	std::wstring IconPath;
public:
	enum {IDD = IDD_DIALOG_ICON_SELECT};

	BEGIN_DLGRESIZE_MAP(CIconSelectDialog)
		DLGRESIZE_CONTROL(IDC_BUTTON_BROWSE_DEFAULT_ICON,	DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_EDIT_ICON_PATH,				DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_BUTTON_BROWSE_ICON,			DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_LIST_ICON,					DLSZ_SIZE_X | DLSZ_SIZE_Y)
		DLGRESIZE_CONTROL(IDOK,								DLSZ_MOVE_X | DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDCANCEL,							DLSZ_MOVE_X | DLSZ_MOVE_Y)
	END_DLGRESIZE_MAP()

	BEGIN_DDX_MAP(CIconSelectDialog)
		DDX_TEXT(IDC_EDIT_ICON_PATH,IconPath)
	END_DDX_MAP()

	BEGIN_MSG_MAP_EX(CIconSelectDialog)
		MSG_WM_INITDIALOG(OnInitDialog)
		COMMAND_ID_HANDLER_EX(IDC_BUTTON_BROWSE_ICON, OnBrowse)
		COMMAND_ID_HANDLER_EX(IDC_BUTTON_BROWSE_DEFAULT_ICON, OnBrowseDefault)
		COMMAND_ID_HANDLER_EX(IDOK, OnOK)
		COMMAND_ID_HANDLER_EX(IDCANCEL, OnCancel)
		CHAIN_MSG_MAP(CDialogResize<CIconSelectDialog>)
	END_MSG_MAP()

	CIconSelectDialog(ASSOCINFO &ai){
		AssocInfo = &ai;
		if (AssocInfo->IconFile.empty()) {
			auto strResourcePath = UtilGetModuleDirectoryPath();
			strResourcePath /= UtilLoadString(IDS_ICON_FILE_NAME_DEFAULT);
			IconPath = strResourcePath;
		} else {
			IconPath = AssocInfo->IconFile;
		}
	}
	bool UpdateIcon() {
		ListView.DeleteAllItems();
		IconList.Destroy();

		//number of icons
		long IconCount = (long)ExtractIconW(GetModuleHandleW(nullptr), IconPath.c_str(), -1);
		if (0 == IconCount) {
			ListView.EnableWindow(false);
			return false;
		}
		ListView.EnableWindow(true);
		IconList.Create(32, 32, ILC_COLOR32 | ILC_MASK, IconCount, 1);
		for (long i = 0; i < IconCount; i++) {
			CIcon Icon;
			Icon.ExtractIcon(IconPath.c_str(), i);
			IconList.AddIcon(Icon);
		}
		ListView.SetImageList(IconList, LVSIL_NORMAL);
		for (long i = 0; i < IconCount; i++) {
			ListView.AddItem(i, 0, L"", i);
		}
		return true;
	}

	LRESULT OnInitDialog(HWND hWnd, LPARAM lParam) {
		ASSERT(AssocInfo);
		CenterWindow();
		ListView = GetDlgItem(IDC_LIST_ICON);
		DoDataExchange(FALSE);

		UpdateIcon();
		ListView.SetItemState(AssocInfo->IconIndex, LVIS_SELECTED, 1);

		DlgResize_Init(true, true, WS_THICKFRAME | WS_CLIPCHILDREN);
		return TRUE;
	}
	void OnBrowse(UINT uNotifyCode, int nID, HWND hWndCtl) {
		const COMDLG_FILTERSPEC filter[] = {
			{ L"Icon File", L"*.dll;*.exe;*.ico;*.ocx;*.cpl;*.vbx;*.scr;*.icl" },
			{ L"All Files", L"*.*" },
		};

		if (!DoDataExchange(TRUE))return;
		LFShellFileOpenDialog dlg(IconPath.c_str(),
			FOS_FORCEFILESYSTEM | FOS_FILEMUSTEXIST | FOS_PATHMUSTEXIST,
			nullptr, filter, COUNTOF(filter));
		if (IDCANCEL != dlg.DoModal()) {
			CString tmp;
			dlg.GetFilePath(tmp);
			IconPath = tmp.operator LPCWSTR();
			DoDataExchange(FALSE);
			UpdateIcon();
		}
	}
	void OnBrowseDefault(UINT uNotifyCode, int nID, HWND hWndCtl) {
		auto ResourcePath = UtilGetModuleDirectoryPath();
		ResourcePath /= UtilLoadString(IDS_ICON_FILE_NAME_DEFAULT);
		IconPath = ResourcePath.make_preferred().c_str();

		DoDataExchange(FALSE);
		UpdateIcon();
	}
	void OnOK(UINT uNotifyCode, int nID, HWND hWndCtl) {
		if (!DoDataExchange(TRUE))return;
		AssocInfo->IconFile = IconPath;
		const int ItemCount = ListView.GetItemCount();
		for (int i = 0; i < ItemCount; i++) {
			if (ListView.GetItemState(i, LVIS_SELECTED)) {
				AssocInfo->IconIndex = i;
				break;
			}
		}
		EndDialog(nID);
	}
	void OnCancel(UINT uNotifyCode, int nID, HWND hWndCtl){
		EndDialog(nID);
	}
};

