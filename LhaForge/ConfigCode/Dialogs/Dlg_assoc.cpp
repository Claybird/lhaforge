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
#include "Dlg_assoc.h"
#include "ConfigCode/configwnd.h"
#include "Utilities/StringUtil.h"

#define ICONINDEX_EXTERNAL_SINGLE	21


struct DLG_ASSOC_ITEM{
	ASSOC_TYPE atype;
	std::wstring ext;
	int defaultIconIndex;
	UINT resStatic, resButton, resCheck;
};

#define DLG_ASSOC_DEF(TYPE,EXT,INDEX)	\
	{ASSOC_##TYPE,	EXT, INDEX, \
	IDC_STATIC_ASSOCIATION_##TYPE, IDC_BUTTON_CHANGE_ICON_##TYPE, IDC_CHECK_ASSOCIATION_##TYPE}

const DLG_ASSOC_ITEM DLG_ASSOC_TABLE[]={
	DLG_ASSOC_DEF(LZH,	L".lzh",	0),
	DLG_ASSOC_DEF(LZS,	L".lzs",	22),
	DLG_ASSOC_DEF(LHA,	L".lha",	23),

	DLG_ASSOC_DEF(ZIP,	L".zip",	1),
	DLG_ASSOC_DEF(CAB,	L".cab",	3),
	DLG_ASSOC_DEF(ZIPX,	L".zipx",	2),

	DLG_ASSOC_DEF(7Z,	L".7z",		4),

	DLG_ASSOC_DEF(RAR,	L".rar",	5),
	DLG_ASSOC_DEF(ACE,	L".ace",	13),
	DLG_ASSOC_DEF(ARJ,	L".arj",	35),
	DLG_ASSOC_DEF(BZA,	L".bza",	15),
	DLG_ASSOC_DEF(GZA,	L".gza",	16),

	DLG_ASSOC_DEF(UUE,	L".uue",	19),
	DLG_ASSOC_DEF(UUE,	L".ish",	19),

	DLG_ASSOC_DEF(TAR,	L".tar",	24),
	DLG_ASSOC_DEF(GZ,	L".gz",		25),
	DLG_ASSOC_DEF(BZ2,	L".bz2",	27),
	DLG_ASSOC_DEF(XZ,	L".xz",		36),
	DLG_ASSOC_DEF(LZMA,	L".lzma",	38),
	DLG_ASSOC_DEF(ZSTD,	L".zst",	40),
	DLG_ASSOC_DEF(Z,	L".z",		29),
	DLG_ASSOC_DEF(CPIO,	L".cpio",	31),
	DLG_ASSOC_DEF(TGZ,	L".tgz",	26),
	DLG_ASSOC_DEF(TBZ,	L".tbz",	28),
	DLG_ASSOC_DEF(TAR_XZ,	L".txz",	37),
	DLG_ASSOC_DEF(TAR_LZMA,L".tlz",	39),
	DLG_ASSOC_DEF(TAZ,	L".taz",	30),

	DLG_ASSOC_DEF(ISO,	L".iso",	35),
};

//--------------------------------------------

LRESULT CConfigDlgAssociation::OnInitDialog(HWND hWnd, LPARAM lParam)
{
	{
		auto modulePath = UtilGetModulePath();
		std::filesystem::path fullPath;
		try {
			fullPath = UtilGetCompletePathName(modulePath);
		} catch (const LF_EXCEPTION&) {
			fullPath = modulePath;
		}
		m_assocDesired = Format(L"\"%s\"  /m \"%%1\"", fullPath.c_str());
	}

	// system default icon
	if(Icon_SystemDefault.IsNull()){
		auto nSize = GetSystemDirectoryW(nullptr, 0);
		std::vector<wchar_t> buf(nSize);
		GetSystemDirectoryW(&buf[0], nSize);
		std::filesystem::path path = &buf[0];
		path /= L"shell32.dll";
		Icon_SystemDefault.ExtractIconW(path.make_preferred().c_str(), 0);
	}

	for(const auto& item: DLG_ASSOC_TABLE){
		auto& assoc = AssocSettings[item.atype];
		const auto &ext = item.ext;
		assoc.DefaultIconIndex = item.defaultIconIndex;
		assoc.Picture_Icon = GetDlgItem(item.resStatic);
		assoc.Button_SetIcon = GetDlgItem(item.resButton);
		assoc.Check_SetAssoc = GetDlgItem(item.resCheck);
		assoc.AssocInfo.Ext = ext;
		assoc.Check_SetAssoc.SetCheck(FALSE);
		if (AssocGetAssociation(ext, assoc.AssocInfo)) {
			assoc.SetIconFromAssoc(Icon_SystemDefault);
			assoc.CheckAssociation(m_assocDesired);
			if (assoc.bChanged)mr_ConfigDlg.RequireAssistant();
			if (!assoc.AssocInfo.isAssociated) {
				assoc.Button_SetIcon.EnableWindow(false);
				assoc.Check_SetAssoc.SetCheck(FALSE);
			} else {
				assoc.Check_SetAssoc.SetCheck(TRUE);
			}
		} else {
			assoc.Picture_Icon.SetIcon(Icon_SystemDefault);
			assoc.Button_SetIcon.EnableWindow(false);
		}
	}

	return TRUE;
}

LRESULT CConfigDlgAssociation::OnCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	WORD wNotifyCode = HIWORD(wParam);
	WORD wID = LOWORD(wParam);
	HWND hWndCtl = (HWND)lParam;
	bHandled = FALSE;
	if (BN_CLICKED == wNotifyCode) {
		for (auto &item : AssocSettings) {
			if (hWndCtl == item.Check_SetAssoc) {
				OnCheckAssoc(item);
				bHandled = TRUE;
				return 0;
			} else if (hWndCtl == item.Button_SetIcon) {
				OnChangeIcon(item);
				bHandled = TRUE;
				return 0;
			}
		}
	}
	return 0;
}


void CConfigDlgAssociation::OnCheckAssoc(ASSOC_SETTINGS& item)
{
	BOOL bEnabled = item.Check_SetAssoc.GetCheck();
	item.Button_SetIcon.EnableWindow(bEnabled);
	if (!bEnabled) {
		item.SetIconFromAssoc(Icon_SystemDefault);
	}
	mr_ConfigDlg.RequireAssistant();
}


void CConfigDlgAssociation::OnChangeIcon(ASSOC_SETTINGS& item)
{
	CIconSelectDialog isd(item.AssocInfo);
	if (IDOK == isd.DoModal()) {
		item.SetIcon(item.AssocInfo.IconFile, item.AssocInfo.IconIndex);
		item.bChanged = true;
		mr_ConfigDlg.RequireAssistant();
	}
}

LRESULT CConfigDlgAssociation::OnSetAssoc(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if(BN_CLICKED!=wNotifyCode){
		return 0;
	}

	std::filesystem::path ResourcePath;
	int IconIndex=-1;
	if(IDC_BUTTON_ASSOC_SET_DEFAULT_ICON==wID||IDC_BUTTON_ASSOC_SET_DEFAULT_ICON_SINGLE==wID){
		ResourcePath = UtilGetModuleDirectoryPath() / UtilLoadString(IDS_ICON_FILE_NAME_DEFAULT);
	}else if(IDC_BUTTON_ASSOC_SET_EXTERNAL_ICON==wID||IDC_BUTTON_ASSOC_SET_EXTERNAL_ICON_SINGLE==wID){
		//let user to choose icon
		ASSOCINFO ac;
		CIconSelectDialog isd(ac);
		if(IDOK!=isd.DoModal()){
			return 0;
		}
		ResourcePath = ac.IconFile;
		IconIndex = ac.IconIndex;
		if (IDC_BUTTON_ASSOC_SET_EXTERNAL_ICON_SINGLE == wID && -1 == IconIndex) {
			//icon not selected
			return 0;
		}
	}
	for (size_t i = 0; i < AssocSettings.size();i++) {
		auto &item = AssocSettings[i];
		switch(wID){
		case IDC_BUTTON_ASSOC_CHECK_TO_DEFAULT:
			if(-1==index_of(NO_DEFAULT_ASSOCS, COUNTOF(NO_DEFAULT_ASSOCS), i)){
				item.Button_SetIcon.EnableWindow(TRUE);
				item.Check_SetAssoc.SetCheck(TRUE);
			}
			break;
		case IDC_BUTTON_ASSOC_CHECK_ALL:
			item.Button_SetIcon.EnableWindow(TRUE);
			item.Check_SetAssoc.SetCheck(TRUE);
			break;
		case IDC_BUTTON_ASSOC_UNCHECK_ALL:
			item.Button_SetIcon.EnableWindow(FALSE);
			item.Check_SetAssoc.SetCheck(FALSE);
			item.SetIconFromAssoc(Icon_SystemDefault);
			break;
		case IDC_BUTTON_ASSOC_SET_DEFAULT_ICON:	//FALLTHROUGH
		case IDC_BUTTON_ASSOC_SET_EXTERNAL_ICON:
			if(item.Check_SetAssoc.GetCheck()){
				item.AssocInfo.IconIndex=item.DefaultIconIndex;
				item.AssocInfo.IconFile = ResourcePath.make_preferred().c_str();
				item.SetIcon(item.AssocInfo.IconFile, item.AssocInfo.IconIndex);
				item.bChanged=true;
			}
			break;
		case IDC_BUTTON_ASSOC_SET_DEFAULT_ICON_SINGLE:
			if(item.Check_SetAssoc.GetCheck()){
				item.AssocInfo.IconIndex=ICONINDEX_EXTERNAL_SINGLE;
				item.AssocInfo.IconFile = ResourcePath.make_preferred().c_str();
				item.SetIcon(item.AssocInfo.IconFile, item.AssocInfo.IconIndex);
				item.bChanged=true;
			}
			break;
		case IDC_BUTTON_ASSOC_SET_EXTERNAL_ICON_SINGLE:
			if(item.Check_SetAssoc.GetCheck()){
				item.AssocInfo.IconIndex=IconIndex;
				item.AssocInfo.IconFile = ResourcePath.make_preferred().c_str();
				item.SetIcon(item.AssocInfo.IconFile, item.AssocInfo.IconIndex);
				item.bChanged=true;
			}
			break;
		}
	}
	mr_ConfigDlg.RequireAssistant();
	return 0;
}

LRESULT CConfigDlgAssociation::OnApply()
{
	_assocRequests.clear();
	for (auto &item: AssocSettings) {
		bool Checked = (0 != item.Check_SetAssoc.GetCheck());
		if ((item.AssocInfo.isAssociated^Checked) || item.bChanged) {
			if (item.AssocInfo.isAssociated && !Checked) {
				//unset association
				_assocRequests[item.AssocInfo.Ext] = { {L"set", 0} };
			} else {
				//set association
				_assocRequests[item.AssocInfo.Ext] = {
					{L"set", L"1"},
					{L"iconfile", item.AssocInfo.IconFile.wstring()},
					{L"iconindex", Format(L"%d",item.AssocInfo.IconIndex)},
				};
			}
		}
	}

	return TRUE;
}

void CConfigDlgAssociation::StoreConfig(CConfigFile& Config, CConfigFile& assistant)
{
	for (const auto &item : _assocRequests) {
		auto ext = item.first;
		for (const auto &value : item.second) {
			assistant.setValue(ext, value.key, value.value);
		}
	}
}


