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
#include "ConfigManager.h"
#include "ConfigShellExt.h"

void CConfigShellExt::load(CONFIG_SECTION&){ASSERT(!"This code cannot be run");}

void CConfigShellExt::store(CONFIG_SECTION&)const{ASSERT(!"This code cannot be run");}

void CConfigShellExt::loadShellMenu(CONFIG_SECTION &Config)
{
	// 右クリックメニューの表示・非表示
	ShellMenuCompress=Config.Data[_T("Compress")].GetNParam(TRUE);
	ShellMenuExtract=Config.Data[_T("Extract")].GetNParam(TRUE);
	ShellMenuList=Config.Data[_T("List")].GetNParam(TRUE);
	ShellMenuTest=Config.Data[_T("Test")].GetNParam(TRUE);

	// サブメニュー以下に放り込む
	ShellMenuUnderSubMenu=Config.Data[_T("UnderSubMenu")].GetNParam(FALSE);
}

void CConfigShellExt::storeShellMenu(CONFIG_SECTION &Config)const
{
	// 右クリックメニューの表示・非表示
	Config.Data[_T("Compress")]=ShellMenuCompress;
	Config.Data[_T("Extract")]=ShellMenuExtract;
	Config.Data[_T("List")]=ShellMenuList;
	Config.Data[_T("Test")]=ShellMenuTest;

	// サブメニュー以下に放り込む
	Config.Data[_T("UnderSubMenu")]=ShellMenuUnderSubMenu;
}

void CConfigShellExt::loadDragMenu(CONFIG_SECTION &Config)
{
	// 右ドラッグメニューの表示・非表示
	DragMenuCompress=Config.Data[_T("Compress")].GetNParam(TRUE);
	DragMenuExtract=Config.Data[_T("Extract")].GetNParam(TRUE);
	// サブメニュー以下に放り込む
	DragMenuUnderSubMenu=Config.Data[_T("UnderSubMenu")].GetNParam(FALSE);
}

void CConfigShellExt::storeDragMenu(CONFIG_SECTION &Config)const
{
	// 右ドラッグメニューの表示・非表示
	Config.Data[_T("Compress")]=DragMenuCompress;
	Config.Data[_T("Extract")]=DragMenuExtract;
	// サブメニュー以下に放り込む
	Config.Data[_T("UnderSubMenu")]=DragMenuUnderSubMenu;
}

void CConfigShellExt::loadExtraMenu(CONFIG_SECTION &Config)
{
	// 拡張メニューの表示
	ForceExtraMenu=Config.Data[_T("ForceExtraMenu")].GetNParam(FALSE);
}

void CConfigShellExt::storeExtraMenu(CONFIG_SECTION &Config)const
{
	// 拡張メニューの表示
	Config.Data[_T("ForceExtraMenu")]=ForceExtraMenu;
}

void CConfigShellExt::loadCustomMenu(CONFIG_SECTION &Config)
{
	// カスタマイズメニューの使用
	UseCustomMenu=Config.Data[_T("UseCustom")].GetNParam(TRUE);
}

void CConfigShellExt::storeCustomMenu(CONFIG_SECTION &Config)const
{
	// カスタマイズメニューの使用
	Config.Data[_T("UseCustom")]=UseCustomMenu;
}

void CConfigShellExt::load(CConfigManager &ConfMan)
{
	loadShellMenu(ConfMan.GetSection(_T("ShellMenu")));
	loadDragMenu(ConfMan.GetSection(_T("DragMenu")));
	loadExtraMenu(ConfMan.GetSection(_T("ExtraMenu")));
	loadCustomMenu(ConfMan.GetSection(_T("CustomMenu")));
}

void CConfigShellExt::store(CConfigManager &ConfMan)const
{
	storeShellMenu(ConfMan.GetSection(_T("ShellMenu")));
	storeDragMenu(ConfMan.GetSection(_T("DragMenu")));
	storeExtraMenu(ConfMan.GetSection(_T("ExtraMenu")));
	storeCustomMenu(ConfMan.GetSection(_T("CustomMenu")));
}
