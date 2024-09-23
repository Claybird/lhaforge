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
#include "ConfigFile.h"
#include "FileListWindow/FileListModel.h"
#include "FileListWindow/FileListFrame.h"
#include "FileListWindow/FileListTabClient.h"
#include "Utilities/StringUtil.h"
#include "Utilities/Utility.h"
#include "ConfigFileListWindow.h"
#include "resource.h"


#define FILELISTWINDOW_DEFAULT_WIDTH	760
#define FILELISTWINDOW_DEFAULT_HEIGHT	500

void CConfigFileListWindow::load_sub(const CConfigFile& Config)
{
	const auto section = L"FileListWindow";
	general.StoreSetting = Config.getBool(section, L"StoreSetting", true);
	general.ExitWithEscape = Config.getBool(section, L"ExitWithEscape", false);
	general.DisableTab = Config.getBool(section, L"DisableTab", false);
	general.KeepSingleInstance = Config.getBool(section, L"KeepSingleInstance", false);

	//---
	dimensions.StoreWindowPosition = Config.getBool(section, L"StoreWindowPosition", false);
	dimensions.Width = Config.getInt(section, L"Width", FILELISTWINDOW_DEFAULT_WIDTH);
	if (dimensions.Width < 0) {
		dimensions.Width = FILELISTWINDOW_DEFAULT_WIDTH;
	}
	dimensions.Height = Config.getInt(section, L"Height", FILELISTWINDOW_DEFAULT_HEIGHT);
	if (dimensions.Height < 0) {
		dimensions.Height = FILELISTWINDOW_DEFAULT_HEIGHT;
	}
	dimensions.TreeWidth = Config.getInt(section, L"TreeWidth", FILELISTWINDOW_DEFAULT_TREE_WIDTH);
	if (dimensions.TreeWidth < 0) {
		dimensions.TreeWidth = 175;
	}
	dimensions.WindowPos_x = Config.getInt(section, L"x", 0);
	dimensions.WindowPos_y = Config.getInt(section, L"y", 0);

	//---------
	//Default:LVS_ICON
	view.ListStyle = Config.getInt(section, L"ListStyle", 0);
	if (
		(LVS_LIST != view.ListStyle) &&
		(LVS_REPORT != view.ListStyle) &&
		(LVS_SMALLICON != view.ListStyle) &&
		(LVS_ICON != view.ListStyle)
		) {
		view.ListStyle = LVS_ICON;
	}
	view.SortColumnIndex = Config.getIntRange(section, L"SortColumn", (int)FILEINFO_TYPE::INVALID, (int)FILEINFO_TYPE::LastItem, (int)FILEINFO_TYPE::FILENAME);
	view.SortAtoZ = Config.getBool(section, L"AtoZ", true);
	view.ExpandTree = Config.getBool(section, L"ExpandTree", false);
	view.DisplayFileSizeInByte = Config.getBool(section, L"DisplayFileSizeInByte", false);
	view.DisplayPathOnly = Config.getBool(section, L"DisplayPathOnly", false);

	{
		for (int i = 0; i < (int)FILEINFO_TYPE::ItemCount; i++) {
			view.column.order[i] = i;
		}
		auto buf = Config.getText(section, L"ColumnOrder", L"");
		if (!buf.empty()) {
			std::vector<int> numArr = UtilStringToIntArray(buf);
			for (int idx = 0; idx < std::min((int)numArr.size(), (int)FILEINFO_TYPE::ItemCount); idx++) {
				int columnPosition = numArr[idx];
				if (columnPosition < 0)columnPosition = -1;
				if (columnPosition >= (int)FILEINFO_TYPE::ItemCount) {
					columnPosition = idx;
				}
				view.column.order[idx] = columnPosition;
			}
		}
	}
	{
		for (int i = 0; i < (int)FILEINFO_TYPE::ItemCount; i++) {
			view.column.width[i] = -1;
		}
		auto buf = Config.getText(section, L"ColumnWidth", L"");
		std::vector<int> numArr = UtilStringToIntArray(buf);
		for (int idx = 0; idx < std::min((int)numArr.size(), (int)FILEINFO_TYPE::ItemCount); idx++) {
			view.column.width[idx] = numArr[idx];
		}
	}
	view.strCustomToolbarImage = Config.getText(section, L"CustomToolbarImage", L"");
	view.ShowToolbar = Config.getBool(section, L"ShowToolbar", true);
	view.ShowTreeView = Config.getBool(section, L"ShowTreeView", true);

	view.OpenAssoc.Accept = Config.getText(section, L"OpenAssocAccept", UtilLoadString(IDS_FILELIST_OPENASSOC_DEFAULT_ACCEPT));
	view.OpenAssoc.Deny = Config.getText(section, L"OpenAssocDeny", UtilLoadString(IDS_FILELIST_OPENASSOC_DEFAULT_DENY));
	view.OpenAssoc.DenyExecutables = Config.getBool(section, L"DenyPathExt", true);

	//search folder
	{
		view.searchFolderItems.clear();
		auto count = Config.getInt(section, L"SearchItemCount", 0);
		for (int i = 0; i < count; i++) {
			std::wstring name = Config.getText(section, Format(L"SearchItemName%d", i), L"");
			ARCHIVE_FIND_CONDITION cond = {};
			cond.compare = (ARCHIVE_FIND_CONDITION::COMPARE)Config.getIntRange(section, Format(L"SearchItemCompare%d", i), 0, (int)ARCHIVE_FIND_CONDITION::COMPARE::LastItem, 0);
			cond.key = (ARCHIVE_FIND_CONDITION::KEY)Config.getIntRange(section, Format(L"SearchItemKey%d", i), 0, (int)ARCHIVE_FIND_CONDITION::KEY::LastItem, 0);
			switch (cond.key) {
			case ARCHIVE_FIND_CONDITION::KEY::originalSize:
				cond.st_size = Config.getInt64(section, Format(L"SearchItemSize%d", i), 0);
				break;
			case ARCHIVE_FIND_CONDITION::KEY::mdate:
			{
				auto ft = UtilUnixTimeToFileTime(Config.getInt64(section, Format(L"SearchItemMdate%d", i), 0));
				FileTimeToSystemTime(&ft, &cond.mdate);
			}
				break;
			case ARCHIVE_FIND_CONDITION::KEY::mode:
				cond.st_mode_mask = Config.getInt(section, Format(L"SearchItemModeMask%d", i), 0);
				break;
			case ARCHIVE_FIND_CONDITION::KEY::filename:	//FALLTHROUGH
			case ARCHIVE_FIND_CONDITION::KEY::fullpath:	//FALLTHROUGH
			default:
				cond.patternStr = Config.getText(section, Format(L"SearchItemPattern%d", i), L"*");
				break;
			}
			view.searchFolderItems.push_back(std::make_pair<>(name, cond));
		}
	}
}

void CConfigFileListWindow::loadMenuCommand(const CConfigFile &Config)
{
	view.MenuCommandArray.clear();
	for (int iIndex = 0; iIndex < USERAPP_MAX_NUM; iIndex++) {
		auto section = Format(L"UserApp%d", iIndex);
		if (!Config.hasSection(section)) {
			break;
		} else {
			CLFMenuCommandItem mci;

			mci.Path = Config.getText(section, L"Path", L"");
			mci.Param = Config.getText(section, L"Param", L"");
			mci.Dir = Config.getText(section, L"Dir", L"");
			mci.Caption = Config.getText(section, L"Caption", L"");

			view.MenuCommandArray.push_back(mci);
		}
	}
}

void CConfigFileListWindow::store_sub(CConfigFile &Config)const
{
	const auto section = L"FileListWindow";
	Config.setValue(section, L"StoreSetting", general.StoreSetting);
	if(general.StoreSetting){
		Config.setValue(section, L"Width", dimensions.Width);
		Config.setValue(section, L"Height", dimensions.Height);
		Config.setValue(section, L"TreeWidth", dimensions.TreeWidth);
		Config.setValue(section, L"ListStyle", view.ListStyle);
		Config.setValue(section, L"SortColumn", view.SortColumnIndex);
		Config.setValue(section, L"AtoZ", view.SortAtoZ);
	}
	Config.setValue(section, L"StoreWindowPosition", dimensions.StoreWindowPosition);
	if (dimensions.StoreWindowPosition) {
		Config.setValue(section, L"x", dimensions.WindowPos_x);
		Config.setValue(section, L"y", dimensions.WindowPos_y);
	}
	//----------
	Config.setValue(section, L"ExitWithEscape", general.ExitWithEscape);
	Config.setValue(section, L"DisableTab", general.DisableTab);
	Config.setValue(section, L"KeepSingleInstance", general.KeepSingleInstance);
	//---------
	Config.setValue(section, L"ExpandTree", view.ExpandTree);
	Config.setValue(section, L"DisplayFileSizeInByte", view.DisplayFileSizeInByte);
	Config.setValue(section, L"DisplayPathOnly", view.DisplayPathOnly);

	{
		std::wstring buf;
		for (int i = 0; i < (int)FILEINFO_TYPE::ItemCount; i++) {
			buf += Format(L"%d", view.column.order[i]);
			if (i != (int)FILEINFO_TYPE::ItemCount - 1) {
				buf += L",";
			}
		}
		Config.setValue(section, L"ColumnOrder", buf);
	}
	{
		std::wstring buf;
		for (int i = 0; i < (int)FILEINFO_TYPE::ItemCount; i++) {
			buf += Format(L"%d", view.column.width[i]);
			if (i != (int)FILEINFO_TYPE::ItemCount - 1) {
				buf += L",";
			}
		}
		Config.setValue(section, L"ColumnWidth", buf);
	}
	Config.setValue(section, L"OpenAssocAccept", view.OpenAssoc.Accept);
	Config.setValue(section, L"OpenAssocDeny", view.OpenAssoc.Deny);
	Config.setValue(section, L"DenyPathExt", view.OpenAssoc.DenyExecutables);

	Config.setValue(section, L"CustomToolbarImage", view.strCustomToolbarImage);
	Config.setValue(section, L"ShowToolbar", view.ShowToolbar);
	Config.setValue(section, L"ShowTreeView", view.ShowTreeView);

	//search folder
	{
		Config.setValue(section, L"SearchItemCount", (int)view.searchFolderItems.size());
		for (int i = 0; i < (int)view.searchFolderItems.size(); i++) {
			const auto& name = view.searchFolderItems[i].first;
			const auto& cond = view.searchFolderItems[i].second;
			Config.setValue(section, Format(L"SearchItemName%d", i), name);
			Config.setValue(section, Format(L"SearchItemCompare%d", i), (int)cond.compare);
			Config.setValue(section, Format(L"SearchItemKey%d", i), (int)cond.key);
			Config.setValue(section, Format(L"SearchItemSize%d", i), cond.st_size);

			FILETIME ft;
			SystemTimeToFileTime(&cond.mdate, &ft);
			Config.setValue(section, Format(L"SearchItemMdate%d", i), UtilFileTimeToUnixTime(ft));
			Config.setValue(section, Format(L"SearchItemModeMask%d", i), cond.st_mode_mask);
			Config.setValue(section, Format(L"SearchItemPattern%d", i), cond.patternStr);
		}
	}
}

void CConfigFileListWindow::storeMenuCommand(CConfigFile &Config)const
{
	//---delete old sections
	for (int iIndex = 0; iIndex < USERAPP_MAX_NUM; iIndex++) {
		auto section = Format(L"UserApp%d", iIndex);
		if (!Config.hasSection(section)) {
			break;
		} else {
			Config.deleteSection(section);
		}
	}
	//---store new sections
	for (size_t iIndex = 0; iIndex < view.MenuCommandArray.size(); iIndex++) {
		auto section = Format(L"UserApp%d", iIndex);
		const auto& mci = view.MenuCommandArray[iIndex];

		Config.setValue(section, L"Path", mci.Path);
		Config.setValue(section, L"Param", mci.Param);
		Config.setValue(section, L"Dir", mci.Dir);
		Config.setValue(section, L"Caption", mci.Caption);
	}
}

//checks file extension whether file is allowed to be opened.
bool CConfigFileListWindow::isPathAcceptableToOpenAssoc(const std::filesystem::path& path, bool bDenyOnly)const
{
	auto denyExt = view.OpenAssoc.Deny;
	if (view.OpenAssoc.DenyExecutables) {
		auto envs = UtilGetEnvInfo();
		denyExt += L";" + envs[L"PATHEXT"];
	}

	const auto denyList = UtilSplitString(denyExt, L";");
	for (const auto& deny : denyList) {
		if (UtilExtMatchSpec(path, deny)) {
			return false;
		}
	}
	if (bDenyOnly) {
		return true;
	} else {
		const auto acceptList = UtilSplitString(view.OpenAssoc.Accept, L";");
		for (const auto& accept : acceptList) {
			if (UtilExtMatchSpec(path, accept)) {
				return true;
			}
		}
	}
	return false;
}

#ifdef UNIT_TEST
TEST(config, CConfigFileListWindow)
{
	CConfigFile emptyFile;
	CConfigFileListWindow conf;
	conf.load(emptyFile);
	conf.view.OpenAssoc.Deny = L".exe;.bat";
	conf.view.OpenAssoc.Accept = L".txt";

	EXPECT_TRUE(conf.isPathAcceptableToOpenAssoc(L"path/to/file.txt", true));
	EXPECT_TRUE(conf.isPathAcceptableToOpenAssoc(L"path/to/file.bmp", true));
	EXPECT_FALSE(conf.isPathAcceptableToOpenAssoc(L"path/to/file.exe", true));
	EXPECT_FALSE(conf.isPathAcceptableToOpenAssoc(L"path/to/file.bat", true));

	EXPECT_TRUE(conf.isPathAcceptableToOpenAssoc(L"path/to/file.txt", false));
	EXPECT_FALSE(conf.isPathAcceptableToOpenAssoc(L"path/to/file.bmp", false));
	EXPECT_FALSE(conf.isPathAcceptableToOpenAssoc(L"path/to/file.exe", false));
	EXPECT_FALSE(conf.isPathAcceptableToOpenAssoc(L"path/to/file.bat", false));
}
#endif
