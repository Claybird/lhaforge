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
#include "ConfigCode/Dialogs/configwnd.h"
#include "Utilities/StringUtil.h"

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

const ASSOC_TYPE NO_DEFAULT_ASSOCS[] = {
	ASSOC_TYPE::ISO,
};

struct DLG_ASSOC_ITEM{
	ASSOC_TYPE atype;
	std::wstring ext;
	std::wstring formatName;
};

const DLG_ASSOC_ITEM DLG_ASSOC_TABLE[]={
	{ASSOC_TYPE::LZH,	L".lzh",	L"LHA/LZH archive"},
	{ASSOC_TYPE::LZS,	L".lzs",	L"LHA/LZH archive"},
	{ASSOC_TYPE::LHA,	L".lha",	L"LHA/LZH archive"},

	{ASSOC_TYPE::ZIP,	L".zip",	L"ZIP archive"},
	{ASSOC_TYPE::CAB,	L".cab",	L"\"Microsoft Cabinet\" archive"},
	{ASSOC_TYPE::ZIPX,	L".zipx",	L"WinZip advanced ZIP archive"},

	{ASSOC_TYPE::_7Z,	L".7z",		L"7-Zip archive"},

	{ASSOC_TYPE::RAR,	L".rar",	L"WinRAR archive"},
	{ASSOC_TYPE::ACE,	L".ace",	L"WinAce archive"},
	{ASSOC_TYPE::ARJ,	L".arj",	L"ARJ archive"},
	{ASSOC_TYPE::BZA,	L".bza",	L"BGA32.dll archive"},
	{ASSOC_TYPE::GZA,	L".gza",	L"BGA32.dll archive"},
	{ASSOC_TYPE::JAK,	L".jak",	L"JACK32.dll splitted file"},

	{ASSOC_TYPE::UUE,	L".uue",	L"uuencode binary-to-text encoding"},
	{ASSOC_TYPE::ISH,	L".ish",	L"ish binary-to-text encoding"},

	{ASSOC_TYPE::TAR,	L".tar",	L"\"Tape Archives\" format"},
	{ASSOC_TYPE::GZ,		L".gz",		L"gzip compression format"},
	{ASSOC_TYPE::BZ2,	L".bz2",	L"bzip2 compression format"},
	{ASSOC_TYPE::XZ,		L".xz",		L"\"XZ Utils\" compression format"},
	{ASSOC_TYPE::LZMA,	L".lzma",	L"Lempel-Ziv-Markov chain-Algorithm compression format"},
	{ASSOC_TYPE::ZSTD,	L".zst",	L"Facebook Zstandard compression format"},
	{ASSOC_TYPE::Z,		L".z",		L"\"UNIX Compress\" format"},
	{ASSOC_TYPE::CPIO,	L".cpio",	L"UNIX cpio compression format"},
	{ASSOC_TYPE::TGZ,	L".tgz",	L"tar+gz archive"},
	{ASSOC_TYPE::TBZ,	L".tbz",	L"tar+bz2 archive"},
	{ASSOC_TYPE::TAR_XZ,	L".txz",L"tar+xz archive"},
	{ASSOC_TYPE::TAR_LZMA,L".tlz",	L"tar+lzma archive"},
	{ASSOC_TYPE::TAZ,	L".taz",	L"tar+z archive"},

	{ASSOC_TYPE::ISO,	L".iso",	L"[No default]ISO 9660 file system format"},
};

//--------------------------------------------

CConfigDlgAssociation::CConfigDlgAssociation(CConfigDialog& dlg)
	:mr_ConfigDlg(dlg)
{
	AssocSettings.resize((int)ASSOC_TYPE::ItemCount);
}

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
		auto& assoc = AssocSettings[(int)item.atype];
		const auto &ext = item.ext;
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
	m_assocList.SetExtendedListViewStyle(LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

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

	for (size_t aType = 0; aType < AssocSettings.size(); aType++) {
		auto &item = AssocSettings[aType];
		switch(wID){
		case IDC_BUTTON_ASSOC_CHECK_TO_DEFAULT:
			if(-1==index_of(NO_DEFAULT_ASSOCS, COUNTOF(NO_DEFAULT_ASSOCS), (ASSOC_TYPE)aType)){
				item.AssocInfo.isAssociated = true;
				m_assocList.SetCheckState((int)aType, item.AssocInfo.isAssociated);
				item.bChanged = true;
			}
			break;
		case IDC_BUTTON_ASSOC_UNCHECK_ALL:
			item.AssocInfo.isAssociated = false;
			m_assocList.SetCheckState((int)aType, item.AssocInfo.isAssociated);
			item.SetIconFromAssoc();
			item.bChanged = true;
			break;
		}
	}
	m_assocList.Invalidate();
	updateImageList();
	mr_ConfigDlg.RequireAssistant();
	return 0;
}


LRESULT CConfigDlgAssociation::OnSetIcon(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if (BN_CLICKED != wNotifyCode) {
		return 0;
	}

	std::filesystem::path ResourcePath = UtilGetModuleDirectoryPath() / DEFAULT_ICON_FILENAME;
	int IconIndex = 0;
	if (IDC_BUTTON_ASSOC_SET_EXTERNAL_ICON_SINGLE == wID) {
		//let user to choose icon
		ASSOCINFO ac;
		CIconSelectDialog isd(ac);
		if (IDOK != isd.DoModal()) {
			return 0;	//cancelled
		}
		ResourcePath = ac.IconFile;
		IconIndex = ac.IconIndex;
	}
	for (size_t aType = 0; aType < AssocSettings.size(); aType++) {
		auto& item = AssocSettings[aType];
		if (item.AssocInfo.isAssociated) {
			item.bChanged = true;
			switch (wID) {
			case IDC_BUTTON_ASSOC_SET_DEFAULT_ICON:	//FALLTHROUGH
				item.AssocInfo.IconIndex = 0;
				//one icon for each extension
				item.AssocInfo.IconFile = UtilGetModuleDirectoryPath() / (L"icons/archive" + item.AssocInfo.Ext + L".ico");
				item.SetIcon(item.AssocInfo.IconFile, item.AssocInfo.IconIndex);
				break;
			case IDC_BUTTON_ASSOC_SET_DEFAULT_ICON_SINGLE:	//FALLTHROUGH
			case IDC_BUTTON_ASSOC_SET_EXTERNAL_ICON_SINGLE:
				item.AssocInfo.IconIndex = IconIndex;
				item.AssocInfo.IconFile = ResourcePath;
				item.SetIcon(item.AssocInfo.IconFile, item.AssocInfo.IconIndex);
				break;
			}
		}
	}
	m_assocList.Invalidate();
	updateImageList();
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
					{L"iconfile", item.AssocInfo.IconFile.make_preferred().wstring()},
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


