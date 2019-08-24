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
#include "ConfigOpenAction.h"

// 関連付け動作設定
void CConfigOpenAction::load(CONFIG_SECTION &Config)
{
	//動作選択
	OpenAction=(OPENACTION)Config.Data[_T("OpenAction")].GetNParam(0,OPENACTION_LAST_ITEM,OPENACTION_EXTRACT);
	OpenAction_Shift=(OPENACTION)Config.Data[_T("OpenAction_Shift")].GetNParam(0,OPENACTION_LAST_ITEM,OPENACTION_LIST);
	OpenAction_Ctrl=(OPENACTION)Config.Data[_T("OpenAction_Ctrl")].GetNParam(0,OPENACTION_LAST_ITEM,OPENACTION_TEST);
}

void CConfigOpenAction::store(CONFIG_SECTION &Config)const
{
	//動作選択
	Config.Data[_T("OpenAction")]=OpenAction;
	Config.Data[_T("OpenAction_Shift")]=OpenAction_Shift;
	Config.Data[_T("OpenAction_Ctrl")]=OpenAction_Ctrl;
}

void CConfigOpenAction::load(CConfigManager &ConfMan)
{
	load(ConfMan.GetSection(_T("OpenAction")));
}

void CConfigOpenAction::store(CConfigManager &ConfMan)const
{
	store(ConfMan.GetSection(_T("OpenAction")));
}
