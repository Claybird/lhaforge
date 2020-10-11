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
void CConfigOpenAction::load(const CConfigManager &Config)
{
	const auto section = L"OpenAction";
	//動作選択
	OpenAction = (OPENACTION)Config.getIntRange(section, L"OpenAction", 0, OPENACTION_LAST_ITEM, OPENACTION_EXTRACT);
	OpenAction_Shift = (OPENACTION)Config.getIntRange(section, L"OpenAction_Shift", 0, OPENACTION_LAST_ITEM, OPENACTION_LIST);
	OpenAction_Ctrl = (OPENACTION)Config.getIntRange(section, L"OpenAction_Ctrl", 0, OPENACTION_LAST_ITEM, OPENACTION_TEST);
}

void CConfigOpenAction::store(CConfigManager &Config)const
{
	const auto section = L"OpenAction";
	//動作選択
	Config.setValue(section, L"OpenAction", OpenAction);
	Config.setValue(section, L"OpenAction_Shift", OpenAction_Shift);
	Config.setValue(section, L"OpenAction_Ctrl", OpenAction_Ctrl);
}
