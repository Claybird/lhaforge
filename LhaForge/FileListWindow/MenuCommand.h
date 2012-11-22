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

#pragma once
//ファイル一覧モードの右クリックメニュー「プログラムで開く」のメニューID定義
#define USERAPP_MAX_NUM				200
#define ID_MENUITEM_USERAPP_BEGIN	4000
#define ID_MENUITEM_USERAPP_END		(ID_MENUITEM_USERAPP_BEGIN+USERAPP_MAX_NUM-1)

struct CMenuCommandItem{
	virtual ~CMenuCommandItem(){}
	CString Caption;
	CString Path;
	CString Param;
	CString Dir;
};


struct CConfigFileListWindow;
struct SHORTCUTINFO;
void MenuCommand_MakeUserAppMenu(HMENU hMenu);
void MenuCommand_MakeSendToMenu(HMENU hMenu);
void MenuCommand_UpdateUserAppCommands(const CConfigFileListWindow &ConfFLW);
UINT MenuCommand_GetNumSendToCmd();
void MenuCommand_MakeSendToCommands();
const std::vector<CMenuCommandItem>& MenuCommand_GetCmdArray();
const std::vector<SHORTCUTINFO>& MenuCommand_GetSendToCmdArray();
