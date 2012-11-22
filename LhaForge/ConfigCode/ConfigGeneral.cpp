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
#include "../ArchiverCode/arc_interface.h"
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
}

void CConfigGeneral::storeGeneral(CONFIG_SECTION &Config)const
{
	Config.Data[_T("NotifyShell")]=NotifyShellAfterProcess;
	Config.Data[_T("ProcessPriority")]=ProcessPriority;
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
