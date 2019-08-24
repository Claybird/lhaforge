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
#include "../ArchiverCode/ArchiverXACRETT.h"
#include "ConfigManager.h"
#include "ConfigXACRETT.h"

// XacRett設定
void CConfigXACRETT::load(CONFIG_SECTION &Config)
{
	//XacRettを有効にするかどうか
	EnableDLL=Config.Data[_T("EnableDLL")].GetNParam(FALSE);
	//XacRett関係のメニューを有効にするかどうか
	EnableShellMenu=Config.Data[_T("EnableShellMenu")].GetNParam(FALSE);
	//XacRettを優先する拡張子
	Extensions=Config.Data[_T("Extension")];
}

void CConfigXACRETT::store(CONFIG_SECTION &Config)const
{
	//XacRettを有効にするかどうか
	Config.Data[_T("EnableDLL")]=EnableDLL;
	//XacRett関係のメニューを有効にするかどうか
	Config.Data[_T("EnableShellMenu")]=EnableShellMenu;
	//XacRettを優先する拡張子
	Config.Data[_T("Extension")]=Extensions;
}

void CConfigXACRETT::load(CConfigManager &ConfMan)
{
	load(ConfMan.GetSection(_T("XacRett")));
}

void CConfigXACRETT::store(CConfigManager &ConfMan)const
{
	store(ConfMan.GetSection(_T("XacRett")));
}

