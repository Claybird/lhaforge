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
#include "menucommand.h"
#include "FileListModel.h"
#include "../Utilities/OSUtil.h"
#include "../ConfigCode/ConfigManager.h"
#include "../ConfigCode/ConfigFileListWindow.h"
#include "../resource.h"


//---SendTo commands
std::vector<UTIL_SHORTCUTINFO> s_SendToCmd;
//---OpenWith commands
std::vector<CMenuCommandItem> s_MenuCommandArray;

UINT MenuCommand_GetNumSendToCmd()
{
	return (UINT)s_SendToCmd.size();
}

const std::vector<CMenuCommandItem>& MenuCommand_GetCmdArray()
{
	return s_MenuCommandArray;
}

const std::vector<UTIL_SHORTCUTINFO>& MenuCommand_GetSendToCmdArray()
{
	return s_SendToCmd;
}

void MenuCommand_MakeUserAppMenu(HMENU hMenu)
{
	ASSERT(hMenu);
	if (!hMenu)return;
	CMenuHandle cSubMenu = hMenu;
	//remove old menu
	int size = cSubMenu.GetMenuItemCount();
	for (int i = 0; i < size; i++) {
		cSubMenu.RemoveMenu(0, MF_BYPOSITION);
	}
	if (s_MenuCommandArray.empty()) {
		cSubMenu.InsertMenu(-1, MF_BYPOSITION | MF_STRING, (UINT_PTR)ID_MENUITEM_DUMMY, CString(MAKEINTRESOURCE(ID_MENUITEM_DUMMY)));
		cSubMenu.EnableMenuItem(ID_MENUITEM_DUMMY, MF_DISABLED | MF_GRAYED | MF_BYCOMMAND);
	} else {
		//add custom command
		for (UINT u = 0; u < s_MenuCommandArray.size(); u++) {
			UINT nOffset = ID_MENUITEM_USERAPP_BEGIN + u;
			cSubMenu.InsertMenu(-1, MF_BYPOSITION | MF_STRING, (UINT_PTR)nOffset, s_MenuCommandArray[u].Caption.c_str());
		}
	}
}

void MenuCommand_MakeSendToMenu(HMENU hMenu)
{
	ASSERT(hMenu);
	if (!hMenu)return;
	CMenuHandle cSubMenu = hMenu;
	//remove old menu
	int size = cSubMenu.GetMenuItemCount();
	for (int i = 0; i < size; i++) {
		cSubMenu.RemoveMenu(0, MF_BYPOSITION);
	}
	if (s_SendToCmd.empty()) {
		cSubMenu.InsertMenu(-1, MF_BYPOSITION | MF_STRING, (UINT_PTR)ID_MENUITEM_DUMMY, CString(MAKEINTRESOURCE(ID_MENUITEM_DUMMY)));
		cSubMenu.EnableMenuItem(ID_MENUITEM_DUMMY, MF_DISABLED | MF_GRAYED | MF_BYCOMMAND);
	} else {
		//add SendTo command
		for (UINT u = 0; u < s_SendToCmd.size(); u++) {
			bool bIcon = !s_SendToCmd[u].cIconBmpSmall.IsNull();

			MENUITEMINFO mii = { 0 };
			mii.cbSize = sizeof(mii);
			mii.fMask = MIIM_ID | MIIM_STRING | MIIM_FTYPE | (bIcon ? MIIM_BITMAP : 0);
			mii.fType = MFT_STRING;
			mii.dwTypeData = (LPTSTR)(LPCTSTR)s_SendToCmd[u].title.c_str();
			mii.wID = ID_MENUITEM_USERAPP_END + u;
			mii.hbmpItem = (HBITMAP)s_SendToCmd[u].cIconBmpSmall;
			cSubMenu.InsertMenuItem(-1, MF_BYPOSITION | MF_STRING, &mii);
		}
	}
}

void MenuCommand_UpdateUserAppCommands(const CConfigFileListWindow &ConfFLW)
{
	s_MenuCommandArray=ConfFLW.MenuCommandArray;
}

void MenuCommand_MakeSendToCommands()
{
	std::filesystem::path szSendTo;
	std::vector<std::wstring> files;
	szSendTo = UtilGetSendToPath();
	szSendTo /= L"*.lnk";
	CFindFile cFind;

	BOOL bFound=cFind.FindFile(szSendTo.c_str());
	for(;bFound;bFound=cFind.FindNextFile()){
		if(cFind.IsDots())continue;

		if(!cFind.IsDirectory()){
			files.push_back(cFind.GetFilePath().operator LPCWSTR());
		}
	}

	s_SendToCmd.resize(files.size());
	for (size_t i = 0; i < files.size(); i++) {
		UtilGetShortcutInfo(files[i], s_SendToCmd[i]);
	}
}


