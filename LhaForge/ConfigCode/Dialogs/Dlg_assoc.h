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

enum class ASSOC_TYPE : int {
	LZH,
	LZS,
	LHA,

	ZIP,
	CAB,
	ZIPX,

	_7Z,

	RAR,
	ACE,
	ARJ,
	BZA,
	GZA,
	JAK,

	UUE,
	ISH,

	TAR,
	GZ,
	BZ2,
	XZ,
	LZMA,
	ZSTD,
	Z,
	CPIO,
	TGZ,
	TBZ,
	TAR_XZ,
	TAR_LZMA,
	//ASSOC_TAR_ZSTD,
	TAZ,
	ISO,

	ENUM_COUNT_AND_LASTITEM,
};


class ASSOC_SETTINGS {
public:
	CIcon		Icon;
	ASSOCINFO	AssocInfo;
	bool		bChanged;
	std::wstring formatName;

	ASSOC_SETTINGS() :bChanged(false) {}
	virtual ~ASSOC_SETTINGS() {}

	void SetIconFromAssoc(){
		if (AssocInfo.prevIconFile.empty()) {
			//fall back to system default icon
			auto nSize = GetSystemDirectoryW(nullptr, 0);
			std::vector<wchar_t> buf(nSize);
			GetSystemDirectoryW(&buf[0], nSize);
			std::filesystem::path path = &buf[0];
			path /= L"shell32.dll";

			if (!Icon.IsNull())Icon.DestroyIcon();
			Icon.ExtractIconW(path.make_preferred().c_str(), 0);
		} else {
			SetIcon(AssocInfo.prevIconFile.c_str(), AssocInfo.prevIconIndex);
		}
	}
	void SetIcon(const std::filesystem::path& path, int idx) {
		if (!Icon.IsNull())Icon.DestroyIcon();
		Icon.ExtractIcon(path.c_str(), idx);
	}

	void CheckAssociation(const std::wstring& desiredCommand) {
		bChanged = false;
		if (AssocInfo.isAssociated) {	//not associated
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
	enum {
		COLUMN_STATE,
		COLUMN_EXT,
		COLUMN_FORMAT,
	};
	std::array<ASSOC_SETTINGS, (int)ASSOC_TYPE::ItemCount> AssocSettings;

	CListViewCtrl m_assocList;
	CImageListManaged m_imageList;

	//expected program path; if different, set flag to modify associated program
	std::wstring m_assocDesired;

	CConfigDialog	&mr_ConfigDlg;

	struct KVPAIR {
		std::wstring key, value;
	};
	std::map<std::wstring, std::vector<KVPAIR>> _assocRequests;

	void updateImageList();
public:
	enum { IDD = IDD_PROPPAGE_CONFIG_ASSOCIATION };

	BEGIN_MSG_MAP_EX(CConfigDlgAssociation)
		MSG_WM_INITDIALOG(OnInitDialog)
		MSG_WM_CONTEXTMENU(OnContextMenu)
		COMMAND_ID_HANDLER(IDC_BUTTON_ASSOC_CHECK_TO_DEFAULT, OnSetAssoc)
		COMMAND_ID_HANDLER(IDC_BUTTON_ASSOC_UNCHECK_ALL, OnSetAssoc)
		COMMAND_ID_HANDLER(IDC_BUTTON_ASSOC_SET_DEFAULT_ICON, OnSetIcon)
		COMMAND_ID_HANDLER(IDC_BUTTON_ASSOC_SET_DEFAULT_ICON_SINGLE, OnSetIcon)
		COMMAND_ID_HANDLER(IDC_BUTTON_ASSOC_SET_EXTERNAL_ICON_SINGLE, OnSetIcon)
		NOTIFY_CODE_HANDLER_EX(LVN_ITEMCHANGED, OnAssocStateChanged)
		NOTIFY_CODE_HANDLER_EX(NM_DBLCLK, OnChangeIcon)
		NOTIFY_CODE_HANDLER_EX(NM_RETURN, OnChangeIcon)
	END_MSG_MAP()

	LRESULT OnAssocStateChanged(LPNMHDR pnmh);
	LRESULT OnChangeIcon(LPNMHDR) { OnChangeIcon(); return 0; }
	void OnChangeIcon();
	void OnContextMenu(HWND hWndCtrl, CPoint& Point);

	LRESULT OnInitDialog(HWND hWnd, LPARAM lParam);
	LRESULT OnSetAssoc(WORD, WORD, HWND, BOOL&);
	LRESULT OnSetIcon(WORD, WORD, HWND, BOOL&);
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

