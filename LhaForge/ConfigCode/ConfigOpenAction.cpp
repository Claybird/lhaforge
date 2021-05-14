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
#include "ConfigOpenAction.h"

void CConfigOpenAction::load(const CConfigFile &Config)
{
	const auto section = L"OpenAction";
	OpenAction = (OPENACTION)Config.getIntRange(section, L"OpenAction", 0, OPENACTION_LAST_ITEM, OPENACTION_EXTRACT);
	OpenAction_Shift = (OPENACTION)Config.getIntRange(section, L"OpenAction_Shift", 0, OPENACTION_LAST_ITEM, OPENACTION_LIST);
	OpenAction_Ctrl = (OPENACTION)Config.getIntRange(section, L"OpenAction_Ctrl", 0, OPENACTION_LAST_ITEM, OPENACTION_TEST);
}

void CConfigOpenAction::store(CConfigFile &Config)const
{
	const auto section = L"OpenAction";
	Config.setValue(section, L"OpenAction", OpenAction);
	Config.setValue(section, L"OpenAction_Shift", OpenAction_Shift);
	Config.setValue(section, L"OpenAction_Ctrl", OpenAction_Ctrl);
}

#ifdef UNIT_TEST
TEST(config, CConfigOpenAction)
{
	CConfigFile emptyFile;
	CConfigOpenAction conf;
	conf.load(emptyFile);

	EXPECT_EQ(OPENACTION_EXTRACT, conf.OpenAction);
	EXPECT_EQ(OPENACTION_LIST, conf.OpenAction_Shift);
	EXPECT_EQ(OPENACTION_TEST, conf.OpenAction_Ctrl);
}
#endif
