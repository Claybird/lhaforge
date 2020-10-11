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
#include "Utilities/OSUtil.h"

//ファイル一覧モードの右クリックメニュー「プログラムで開く」のメニューID定義
#define USERAPP_MAX_NUM				200
#define ID_MENUITEM_USERAPP_BEGIN	4000
#define ID_MENUITEM_USERAPP_END		(ID_MENUITEM_USERAPP_BEGIN+USERAPP_MAX_NUM-1)

struct CMenuCommandItem{
	virtual ~CMenuCommandItem(){}
	std::wstring Caption;
	std::wstring Path;
	std::wstring Param;
	std::wstring Dir;
};


struct CConfigFileListWindow;
void MenuCommand_MakeUserAppMenu(HMENU hMenu);
void MenuCommand_MakeSendToMenu(HMENU hMenu);
void MenuCommand_UpdateUserAppCommands(const CConfigFileListWindow &ConfFLW);
UINT MenuCommand_GetNumSendToCmd();
void MenuCommand_MakeSendToCommands();
const std::vector<CMenuCommandItem>& MenuCommand_GetCmdArray();
const std::vector<UTIL_SHORTCUTINFO>& MenuCommand_GetSendToCmdArray();
