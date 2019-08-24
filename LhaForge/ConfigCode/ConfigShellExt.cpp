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
