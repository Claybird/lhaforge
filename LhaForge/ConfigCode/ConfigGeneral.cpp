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
#include "ArchiverCode/arc_interface.h"
#include "Utilities/OSUtil.h"
#include "ConfigManager.h"
#include "ConfigGeneral.h"

void CConfigGeneral::loadOutput(const CConfigManager &Config)
{
	const auto section = L"Output";
	// 出力先種類に対する警告
	WarnNetwork=Config.getBool(section ,L"WarnNetwork", false);
	WarnRemovable=Config.getBool(section, L"WarnRemovable", false);

	// 出力先が見つからない場合の設定
	OnDirNotFound = (LOSTDIR)Config.getIntRange(section, L"OnDirNotFound", 0, LOSTDIR_LAST_ITEM, LOSTDIR_ERROR);
}

void CConfigGeneral::storeOutput(CConfigManager &Config)const
{
	const auto section = L"Output";
	// 出力先種類に対する警告
	Config.setValue(section, L"WarnNetwork", WarnNetwork);
	Config.setValue(section, L"WarnRemovable", WarnRemovable);

	// 出力先が見つからない場合の設定
	Config.setValue(section, L"OnDirNotFound", OnDirNotFound);
}

void CConfigGeneral::loadLogView(const CConfigManager &Config)
{
	const auto section = L"LogView";
	LogViewEvent = (LOGVIEW)Config.getIntRange(section, L"LogViewEvent", 0, LOGVIEW_LAST_ITEM, LOGVIEW_ON_ERROR);
}

void CConfigGeneral::storeLogView(CConfigManager &Config)const
{
	const auto section = L"LogView";
	Config.setValue(section, L"LogViewEvent", LogViewEvent);
}

void CConfigGeneral::loadFiler(const CConfigManager &Config)
{
	const auto section = L"Filer";
	Filer.UseFiler = Config.getBool(section, L"UseFiler", false);

	Filer.FilerPath = Config.getText(section, L"FilerPath", L"");
	Filer.Param = Config.getText(section, L"Param", L"");
}

void CConfigGeneral::storeFiler(CConfigManager &Config)const
{
	const auto section = L"Filer";
	Config.setValue(section, L"UseFiler", Filer.UseFiler);

	Config.setValue(section, L"FilerPath", Filer.FilerPath);
	Config.setValue(section, L"Param", Filer.Param);
}

void CConfigGeneral::loadGeneral(const CConfigManager &Config)
{
	const auto section = L"General";
	NotifyShellAfterProcess=Config.getBool(section, L"NotifyShell", false);
	ProcessPriority = (LFPROCESS_PRIORITY)Config.getIntRange(section, L"ProcessPriority", LFPRIOTITY_DEFAULT, LFPRIOTITY_MAX_NUM, LFPRIOTITY_DEFAULT);

	TempPath = Config.getText(section, L"TempPath", L"");
}

void CConfigGeneral::storeGeneral(CConfigManager &Config)const
{
	const auto section = L"General";
	Config.setValue(section, L"NotifyShell", NotifyShellAfterProcess);
	Config.setValue(section, L"ProcessPriority", ProcessPriority);

	Config.setValue(section, L"TempPath", TempPath);
}
