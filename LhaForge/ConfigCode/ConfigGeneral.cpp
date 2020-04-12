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

void CConfigGeneral::loadOutput(CONFIG_SECTION &Config)
{
	// 出力先種類に対する警告
	WarnNetwork=Config.Data[_T("WarnNetwork")].GetNParam(FALSE);
	WarnRemovable=Config.Data[_T("WarnRemovable")].GetNParam(FALSE);

	// 出力先が見つからない場合の設定
	OnDirNotFound=(LOSTDIR)Config.Data[_T("OnDirNotFound")].GetNParam(0,LOSTDIR_LAST_ITEM,LOSTDIR_ERROR);
}

void CConfigGeneral::storeOutput(CONFIG_SECTION &Config)const
{
	// 出力先種類に対する警告
	Config.Data[_T("WarnNetwork")]=WarnNetwork;
	Config.Data[_T("WarnRemovable")]=WarnRemovable;

	// 出力先が見つからない場合の設定
	Config.Data[_T("OnDirNotFound")]=OnDirNotFound;
}

void CConfigGeneral::loadLogView(CONFIG_SECTION &Config)
{
	LogViewEvent=(LOGVIEW)Config.Data[_T("LogViewEvent")].GetNParam(0,LOGVIEW_LAST_ITEM,LOGVIEW_ON_ERROR);
}

void CConfigGeneral::storeLogView(CONFIG_SECTION &Config)const
{
	Config.Data[_T("LogViewEvent")]=LogViewEvent;
}

void CConfigGeneral::loadFiler(CONFIG_SECTION &Config)
{
	Filer.UseFiler=Config.Data[_T("UseFiler")].GetNParam(FALSE);

	Filer.FilerPath=Config.Data[_T("FilerPath")];
	Filer.Param=Config.Data[_T("Param")];
}

void CConfigGeneral::storeFiler(CONFIG_SECTION &Config)const
{
	Config.Data[_T("UseFiler")]=Filer.UseFiler;

	Config.Data[_T("FilerPath")]=Filer.FilerPath;
	Config.Data[_T("Param")]=Filer.Param;
}

void CConfigGeneral::loadGeneral(CONFIG_SECTION &Config)
{
	NotifyShellAfterProcess=Config.Data[_T("NotifyShell")];
	ProcessPriority=(LFPROCESS_PRIORITY)Config.Data[_T("ProcessPriority")].GetNParam(LFPRIOTITY_DEFAULT,LFPRIOTITY_MAX_NUM,LFPRIOTITY_DEFAULT);

	TempPath=Config.Data[_T("TempPath")];
}

void CConfigGeneral::storeGeneral(CONFIG_SECTION &Config)const
{
	Config.Data[_T("NotifyShell")]=NotifyShellAfterProcess;
	Config.Data[_T("ProcessPriority")]=ProcessPriority;

	Config.Data[_T("TempPath")]=TempPath;
}

void CConfigGeneral::load(CConfigManager &ConfMan)
{
	loadOutput(ConfMan.GetSection(_T("Output")));
	loadLogView(ConfMan.GetSection(_T("LogView")));
	loadFiler(ConfMan.GetSection(_T("Filer")));
	loadGeneral(ConfMan.GetSection(_T("General")));
}

void CConfigGeneral::store(CConfigManager &ConfMan)const
{
	storeOutput(ConfMan.GetSection(_T("Output")));
	storeLogView(ConfMan.GetSection(_T("LogView")));
	storeFiler(ConfMan.GetSection(_T("Filer")));
	storeGeneral(ConfMan.GetSection(_T("General")));
}
