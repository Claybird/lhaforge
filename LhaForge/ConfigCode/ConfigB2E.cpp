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
#include "../ArchiverCode/ArchiverB2E.h"
#include "ConfigManager.h"
#include "ConfigB2E.h"

void CConfigB2E::load(CONFIG_SECTION &Config)
{
	//B2E32.dllを有効にするかどうか
	EnableDLL=Config.Data[_T("EnableDLL")].GetNParam(FALSE);
	//B2E関係のメニューを有効にするかどうか
	EnableShellMenu=Config.Data[_T("EnableShellMenu")].GetNParam(FALSE);
	//スクリプトのディレクトリ
	ScriptDirectory=Config.Data[_T("ScriptDirectory")];
	//B2EをXacRettより優先
	Priority=Config.Data[_T("Priority")].GetNParam(FALSE);
	//B2Eを優先する拡張子
	Extensions=Config.Data[_T("Extension")];
}

void CConfigB2E::store(CONFIG_SECTION &Config)const
{
	//B2E32.dllを有効にするかどうか
	Config.Data[_T("EnableDLL")]=EnableDLL;
	//B2E関係のメニューを有効にするかどうか
	Config.Data[_T("EnableShellMenu")]=EnableShellMenu;
	//スクリプトのディレクトリ
	Config.Data[_T("ScriptDirectory")]=ScriptDirectory;
	//B2EをXacRettより優先
	Config.Data[_T("Priority")]=Priority;
	//B2Eを優先する拡張子
	Config.Data[_T("Extension")]=Extensions;
}

void CConfigB2E::load(CConfigManager &ConfMan)
{
	load(ConfMan.GetSection(_T("B2E")));
}

void CConfigB2E::store(CConfigManager &ConfMan)const
{
	store(ConfMan.GetSection(_T("B2E")));
}
