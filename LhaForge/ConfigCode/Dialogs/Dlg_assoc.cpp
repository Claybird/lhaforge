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
#include <CommonControls.h>

#define ICONINDEX_EXTERNAL_SINGLE	21


struct DLG_ASSOC_ITEM{
	ASSOC_TYPE atype;
	std::wstring ext;
	std::wstring formatName;
	int defaultIconIndex;	//TODO: remove
};

const DLG_ASSOC_ITEM DLG_ASSOC_TABLE[]={
	{ASSOC_LZH,	L".lzh",	L"LHA/LZH archive",	0},
	{ASSOC_LZS,	L".lzs",	L"LHA/LZH archive",	22},
	{ASSOC_LHA,	L".lha",	L"LHA/LZH archive",	23},

	{ASSOC_ZIP,	L".zip",	L"ZIP archive",	1},
	{ASSOC_CAB,	L".cab",	L"\"Microsoft Cabinet\" archive",	3},
	{ASSOC_ZIPX,	L".zipx",	L"WinZip advanced ZIP archive",	2},

	{ASSOC_7Z,	L".7z",		L"7-Zip archive",	4},

	{ASSOC_RAR,	L".rar",	L"WinRAR archive",	5},
	{ASSOC_ACE,	L".ace",	L"WinAce archive",	13},
	{ASSOC_ARJ,	L".arj",	L"ARJ archive",	35},
	{ASSOC_BZA,	L".bza",	L"BGA32.dll archive",	15},
	{ASSOC_GZA,	L".gza",	L"BGA32.dll archive",	16},

	{ASSOC_UUE,	L".uue",	L"uuencode binary-to-text encoding",	19},
	{ASSOC_ISH,	L".ish",	L"ish binary-to-text encoding",	19},

	{ASSOC_TAR,	L".tar",	L"\"Tape Archives\" format",	24},
	{ASSOC_GZ,	L".gz",		L"gzip compression format",	25},
	{ASSOC_BZ2,	L".bz2",	L"bzip2 compression format",	27},
	{ASSOC_XZ,	L".xz",		L"\"XZ Utils\" compression format",	36},
	{ASSOC_LZMA,	L".lzma",	L"Lempel-Ziv-Markov chain-Algorithm compression format",	38},
	{ASSOC_ZSTD,	L".zst",	L"Facebook Zstandard compression format",	40},
	{ASSOC_Z,	L".z",		L"\"UNIX Compress\" format",	29},
	{ASSOC_CPIO,	L".cpio",	L"UNIX cpio compression format",	31},
	{ASSOC_TGZ,	L".tgz",	L"tar+gz archive",	26},
	{ASSOC_TBZ,	L".tbz",	L"tar+bz2 archive",	28},
	{ASSOC_TAR_XZ,	L".txz",L"tar+xz archive",	37},
	{ASSOC_TAR_LZMA,L".tlz",	L"tar+lzma archive",	39},
	{ASSOC_TAZ,	L".taz",	L"tar+z archive",	30},

	{ASSOC_ISO,	L".iso",	L"[No default]ISO 9660 file system format",	35},
};

//--------------------------------------------

void CConfigDlgAssociation::updateImageList()
{
	if (m_imageList.IsNull()) {
		m_imageList.Create(32, 32, ILC_COLOR32, 0, 1);
		for (const auto& assoc : AssocSettings) {
			m_imageList.AddIcon(assoc.Icon);
		}
	} else {
		int idx = 0;
		for (const auto& assoc : AssocSettings) {
			m_imageList.ReplaceIcon(idx, assoc.Icon);
			idx++;
		}
	}
}

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
		m_assocDesired = Format(L"\"%s\" /m \"%%1\"", fullPath.c_str());
	}

	for(const auto& item: DLG_ASSOC_TABLE){
		auto& assoc = AssocSettings[item.atype];
		const auto &ext = item.ext;
		assoc.DefaultIconIndex = item.defaultIconIndex;
		assoc.AssocInfo.Ext = ext;
		assoc.formatName = item.formatName;
		if (AssocGetAssociation(ext, assoc.AssocInfo)) {
			assoc.CheckAssociation(m_assocDesired);
			if (assoc.bChanged)mr_ConfigDlg.RequireAssistant();
		}
		assoc.SetIconFromAssoc();
	}

	//create default image list
	updateImageList();

	m_assocList = GetDlgItem(IDC_LIST_ASSOC);
	SIZE iconSize = {};
	m_imageList.GetIconSize(iconSize);
	m_assocList.InsertColumn(COLUMN_STATE, L"", LVCFMT_LEFT, iconSize.cx * 2, -1);
	m_assocList.InsertColumn(COLUMN_EXT, UtilLoadString(IDS_EXTENSION).c_str(), LVCFMT_LEFT, 160, -1);
	CRect rc;
	m_assocList.GetClientRect(rc);
	m_assocList.InsertColumn(COLUMN_FORMAT, UtilLoadString(IDS_FORMAT_NAME).c_str(), LVCFMT_LEFT, rc.Width() - iconSize.cx * 2 - 160 - 20, -1);

	//icons - make sure to use large icons only
	m_assocList.SetImageList(m_imageList, LVSIL_NORMAL);
	m_assocList.SetImageList(m_imageList, LVSIL_SMALL);
	m_assocList.SetExtendedListViewStyle(LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_HEADERDRAGDROP);

	// report mode
	DWORD Style = m_assocList.GetWindowLong(GWL_STYLE);
	Style &= ~(LVS_ICON | LVS_REPORT | LVS_SMALLICON | LVS_LIST);
	m_assocList.SetWindowLong(GWL_STYLE, Style | LVS_REPORT | LVS_SINGLESEL);

	// set number of items
	for (int i = 0; i < (int)AssocSettings.size();i++) {
		const auto& item = AssocSettings[i];
		UINT columns[] = { COLUMN_STATE, COLUMN_EXT, COLUMN_FORMAT };

		//icon & check box
		LVITEM li = {};
		li.iItem = i;
		li.cColumns = COUNTOF(columns);
		li.puColumns = columns;
		li.mask |= LVIF_IMAGE | LVIF_STATE | LVIF_COLUMNS;
		li.iImage = i;
		m_assocList.InsertItem(&li);

		m_assocList.SetItemText(i, COLUMN_EXT, item.AssocInfo.Ext.c_str());
		m_assocList.SetItemText(i, COLUMN_FORMAT, item.formatName.c_str());

		m_assocList.SetCheckState(i, item.AssocInfo.isAssociated);
	}
	return TRUE;
}

LRESULT CConfigDlgAssociation::OnAssocStateChanged(LPNMHDR pnmh)
{
	NM_LISTVIEW* pView = (NM_LISTVIEW*)pnmh;
	if (pView->uOldState & LVIS_STATEIMAGEMASK) {
		if (pView->uNewState & LVIS_STATEIMAGEMASK) {
			if (0 <= pView->iItem && (unsigned int)pView->iItem < AssocSettings.size()) {
				auto& item = AssocSettings[pView->iItem];
				bool newState = ((pView->uNewState & LVIS_STATEIMAGEMASK) == INDEXTOSTATEIMAGEMASK(2));

				if (!newState) {
					item.SetIconFromAssoc();
					updateImageList();
				}
				if (newState ^ item.AssocInfo.isAssociated) {
					mr_ConfigDlg.RequireAssistant();
				}
				item.AssocInfo.isAssociated = newState;
				item.bChanged = true;
			}
		}
	}
	return 0;
}


void CConfigDlgAssociation::OnChangeIcon()
{
	int nSelected = m_assocList.GetSelectedIndex();
	if (nSelected != -1) {
		auto& item = AssocSettings[nSelected];
		if (item.AssocInfo.isAssociated) {
			CIconSelectDialog isd(item.AssocInfo);
			if (IDOK == isd.DoModal()) {
				item.SetIcon(item.AssocInfo.IconFile, item.AssocInfo.IconIndex);
				item.bChanged = true;
				updateImageList();
				mr_ConfigDlg.RequireAssistant();
			}
		}
	}
}

void CConfigDlgAssociation::OnContextMenu(HWND hWndCtrl, CPoint& Point)
{
	int nSelected = m_assocList.GetSelectedIndex();
	if (nSelected != -1) {
		if (-1 == Point.x && -1 == Point.y) {
			//from keyboard
			m_assocList.GetItemPosition(nSelected, &Point);
			m_assocList.ClientToScreen(&Point);
		}

		CMenu cMenu;
		CMenuHandle cSubMenu;

		cMenu.LoadMenu(IDR_ASSOC_POPUP);
		cSubMenu = cMenu.GetSubMenu(0);
		auto& item = AssocSettings[nSelected];
		if (!item.AssocInfo.isAssociated) {
			cSubMenu.EnableMenuItem(ID_MENUITEM_ASSOC_ICON, MF_BYCOMMAND | MF_GRAYED);
		}

		UINT nCmd = (UINT)cSubMenu.TrackPopupMenu(TPM_RETURNCMD | TPM_LEFTALIGN, Point.x, Point.y, m_hWnd, nullptr);
		if (nCmd == ID_MENUITEM_ASSOC_ICON) {
			OnChangeIcon();
		}
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
	for (size_t aType = 0; aType < AssocSettings.size(); aType++) {
		auto &item = AssocSettings[aType];
		switch(wID){
		case IDC_BUTTON_ASSOC_CHECK_TO_DEFAULT:
			if(-1==index_of(NO_DEFAULT_ASSOCS, COUNTOF(NO_DEFAULT_ASSOCS), aType)){
				item.AssocInfo.isAssociated = true;
				m_assocList.SetCheckState(aType, item.AssocInfo.isAssociated);
				item.bChanged = true;
			}
			break;
		case IDC_BUTTON_ASSOC_UNCHECK_ALL:
			item.AssocInfo.isAssociated = false;
			m_assocList.SetCheckState(aType, item.AssocInfo.isAssociated);
			item.SetIconFromAssoc();
			updateImageList();
			item.bChanged = true;
			break;
		case IDC_BUTTON_ASSOC_SET_DEFAULT_ICON:	//FALLTHROUGH
		case IDC_BUTTON_ASSOC_SET_EXTERNAL_ICON:
			if(item.AssocInfo.isAssociated){
				item.AssocInfo.IconIndex = item.DefaultIconIndex;
				item.AssocInfo.IconFile = ResourcePath;
				item.SetIcon(item.AssocInfo.IconFile, item.AssocInfo.IconIndex);
				updateImageList();
				item.bChanged=true;
			}
			break;
		case IDC_BUTTON_ASSOC_SET_DEFAULT_ICON_SINGLE:
			if (item.AssocInfo.isAssociated) {
				item.AssocInfo.IconIndex = ICONINDEX_EXTERNAL_SINGLE;
				item.AssocInfo.IconFile = ResourcePath;
				item.SetIcon(item.AssocInfo.IconFile, item.AssocInfo.IconIndex);
				updateImageList();
				item.bChanged=true;
			}
			break;
		case IDC_BUTTON_ASSOC_SET_EXTERNAL_ICON_SINGLE:
			if (item.AssocInfo.isAssociated) {
				item.AssocInfo.IconIndex=IconIndex;
				item.AssocInfo.IconFile = ResourcePath;
				item.SetIcon(item.AssocInfo.IconFile, item.AssocInfo.IconIndex);
				updateImageList();
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
		if (item.bChanged) {
			if (item.AssocInfo.isAssociated) {
				//set association
				_assocRequests[item.AssocInfo.Ext] = {
					{L"set", L"1"},
					{L"iconfile", item.AssocInfo.IconFile.wstring()},
					{L"iconindex", Format(L"%d",item.AssocInfo.IconIndex)},
				};
			} else {
				//unset association
				_assocRequests[item.AssocInfo.Ext] = { {L"set", 0} };
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


