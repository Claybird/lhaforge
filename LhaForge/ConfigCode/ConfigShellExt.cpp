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
#include "ConfigShellExt.h"

void CConfigShellExt::loadShellMenu(const CConfigFile &Config)
{
	const auto section = L"ShellMenu";
	// 右クリックメニューの表示・非表示
	ShellMenuCompress=Config.getBool(section, L"Compress", true);
	ShellMenuExtract=Config.getBool(section, L"Extract", true);
	ShellMenuList=Config.getBool(section, L"List", true);
	ShellMenuTest=Config.getBool(section, L"Test", true);

	// サブメニュー以下に放り込む
	ShellMenuUnderSubMenu=Config.getBool(section, L"UnderSubMenu", false);
}

void CConfigShellExt::storeShellMenu(CConfigFile &Config)const
{
	const auto section = L"ShellMenu";
	// 右クリックメニューの表示・非表示
	Config.setValue(section, L"Compress", ShellMenuCompress);
	Config.setValue(section, L"Extract", ShellMenuExtract);
	Config.setValue(section, L"List", ShellMenuList);
	Config.setValue(section, L"Test", ShellMenuTest);

	// サブメニュー以下に放り込む
	Config.setValue(section, L"UnderSubMenu", ShellMenuUnderSubMenu);
}

void CConfigShellExt::loadDragMenu(const CConfigFile &Config)
{
	const auto section = L"DragMenu";
	// 右ドラッグメニューの表示・非表示
	DragMenuCompress=Config.getBool(section, L"Compress", true);
	DragMenuExtract=Config.getBool(section, L"Extract", true);
	// サブメニュー以下に放り込む
	DragMenuUnderSubMenu=Config.getBool(section, L"UnderSubMenu", false);
}

void CConfigShellExt::storeDragMenu(CConfigFile &Config)const
{
	const auto section = L"DragMenu";
	// 右ドラッグメニューの表示・非表示
	Config.setValue(section, L"Compress", DragMenuCompress);
	Config.setValue(section, L"Extract", DragMenuExtract);
	// サブメニュー以下に放り込む
	Config.setValue(section, L"UnderSubMenu", DragMenuUnderSubMenu);
}

void CConfigShellExt::loadExtraMenu(const CConfigFile &Config)
{
	const auto section = L"ExtraMenu";
	// 拡張メニューの表示
	ForceExtraMenu=Config.getBool(section, L"ForceExtraMenu", false);
}

void CConfigShellExt::storeExtraMenu(CConfigFile &Config)const
{
	const auto section = L"ExtraMenu";
	// 拡張メニューの表示
	Config.setValue(section, L"ForceExtraMenu", ForceExtraMenu);
}


