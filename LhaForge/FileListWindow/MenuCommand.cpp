/*
 * Copyright (c) 2005-2012, Claybird
 * All rights reserved.

 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:

 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Claybird nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.

 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
 * THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 */

#include "stdafx.h"
#include "menucommand.h"
#include "FileListModel.h"
#include "../Utilities/OSUtil.h"
#include "../ConfigCode/ConfigManager.h"
#include "../ConfigCode/ConfigFileListWindow.h"
#include "../resource.h"


//---「送る」メニューのコマンド
std::vector<SHORTCUTINFO> s_SendToCmd;
//「プログラムで開く」のコマンド類
std::vector<CMenuCommandItem> s_MenuCommandArray;

UINT MenuCommand_GetNumSendToCmd()
{
	return (UINT)s_SendToCmd.size();
}

const std::vector<CMenuCommandItem>& MenuCommand_GetCmdArray()
{
	return s_MenuCommandArray;
}

const std::vector<SHORTCUTINFO>& MenuCommand_GetSendToCmdArray()
{
	return s_SendToCmd;
}

void MenuCommand_MakeUserAppMenu(HMENU hMenu)
{
	ASSERT(hMenu);
	if(!hMenu)return;
	CMenuHandle cSubMenu=hMenu;
	//古いメニューの削除
	int size=cSubMenu.GetMenuItemCount();
	for(int i=0;i<size;i++){
		cSubMenu.RemoveMenu(0,MF_BYPOSITION);
	}
	if(s_MenuCommandArray.empty()){
		cSubMenu.InsertMenu(-1,MF_BYPOSITION|MF_STRING,(UINT_PTR)ID_MENUITEM_DUMMY,CString(MAKEINTRESOURCE(ID_MENUITEM_DUMMY)));
		cSubMenu.EnableMenuItem(ID_MENUITEM_DUMMY,MF_DISABLED|MF_GRAYED|MF_BYCOMMAND);
	}else{
		//LhaForge設定のコマンド
		for(UINT u=0;u<s_MenuCommandArray.size();u++){
			UINT nOffset=ID_MENUITEM_USERAPP_BEGIN+u;
			cSubMenu.InsertMenu(-1,MF_BYPOSITION|MF_STRING,(UINT_PTR)nOffset,s_MenuCommandArray[u].Caption);
		}
	}
}

void MenuCommand_MakeSendToMenu(HMENU hMenu)
{
	ASSERT(hMenu);
	if(!hMenu)return;
	CMenuHandle cSubMenu=hMenu;
	//古いメニューの削除
	int size=cSubMenu.GetMenuItemCount();
	for(int i=0;i<size;i++){
		cSubMenu.RemoveMenu(0,MF_BYPOSITION);
	}
	if(s_SendToCmd.empty()){
		cSubMenu.InsertMenu(-1,MF_BYPOSITION|MF_STRING,(UINT_PTR)ID_MENUITEM_DUMMY,CString(MAKEINTRESOURCE(ID_MENUITEM_DUMMY)));
		cSubMenu.EnableMenuItem(ID_MENUITEM_DUMMY,MF_DISABLED|MF_GRAYED|MF_BYCOMMAND);
	}else{
		//SendToのコマンド
		for(UINT u=0;u<s_SendToCmd.size();u++){
			bool bIcon=!s_SendToCmd[u].cIconBmpSmall.IsNull();

			MENUITEMINFO mii={0};
			mii.cbSize=sizeof(mii);
			mii.fMask=MIIM_ID | MIIM_STRING | MIIM_FTYPE | (bIcon ? MIIM_BITMAP : 0);
			mii.fType=MFT_STRING;
			mii.dwTypeData=(LPTSTR)(LPCTSTR)s_SendToCmd[u].strTitle;
			mii.wID=ID_MENUITEM_USERAPP_END+u;
			mii.hbmpItem=(HBITMAP)s_SendToCmd[u].cIconBmpSmall;
			cSubMenu.InsertMenuItem(-1,MF_BYPOSITION|MF_STRING,&mii);
		}
	}
}

void MenuCommand_UpdateUserAppCommands(const CConfigFileListWindow &ConfFLW)
{
	s_MenuCommandArray=ConfFLW.MenuCommandArray;
}

void MenuCommand_MakeSendToCommands()
{
	TCHAR szSendTo[_MAX_PATH];
	std::vector<CString> files;
	if(SHGetSpecialFolderPath(NULL,szSendTo,CSIDL_SENDTO,FALSE)){
		PathAddBackslash(szSendTo);
		PathAppend(szSendTo,_T("*.lnk"));
		CFindFile cFind;

		BOOL bFound=cFind.FindFile(szSendTo);
		for(;bFound;bFound=cFind.FindNextFile()){
			if(cFind.IsDots())continue;

			if(!cFind.IsDirectory()){	//サブディレクトリ検索はしない
				files.push_back(cFind.GetFilePath());
			}
		}
	}

	UtilGetShortcutInfo(files,s_SendToCmd);
}


