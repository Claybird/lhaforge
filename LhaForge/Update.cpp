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
#include "ConfigCode/ConfigManager.h"
#include "ConfigCode/ConfigUpdate.h"
#include "Update.h"
#include "resource.h"

bool CheckUpdateArchiverDLLRequired(const CConfigUpdate& ConfUpdate)
{
	if(!ConfUpdate.AskUpdate)return false;
	if(difftime(time(NULL),ConfUpdate.LastTime)<ConfUpdate.Interval*60*60*24){
		return false;
	}else return true;
}

void DoUpdateArchiverDLL(CConfigManager &ConfigManager)
{
	if(IDYES!=MessageBox(NULL,CString(MAKEINTRESOURCE(IDS_ASK_UPDATE_DLL)),UtilGetMessageCaption(),MB_YESNO|MB_ICONQUESTION)){
		//アップデートしないなら更新日時を上書き
		ConfigManager.WriteUpdateTime();
		return;
	}
	//------------------------
	// LhaForge本体のパス取得
	//------------------------
	TCHAR ExePath[_MAX_PATH+1];
	FILL_ZERO(ExePath);
	GetModuleFileName(GetModuleHandle(NULL), ExePath, _MAX_PATH);

	CConfigUpdate ConfUpdate;
	ConfUpdate.load(ConfigManager);

	PathRemoveFileSpec(ExePath);
	PathAppend(ExePath,CString(MAKEINTRESOURCE(IDS_CALDIX_EXE_NAME)));
	if(32>=(int)ShellExecute(NULL,NULL,ExePath, ConfUpdate.SilentUpdate ? _T("/silent") : NULL,NULL,SW_RESTORE)){
		//実行エラー
		CString strLastError;
		UtilGetLastErrorMessage(strLastError);

		CString msg;
		msg.Format(IDS_ERROR_CANNOT_EXECUTE,ExePath,(LPCTSTR)strLastError);

		ErrorMessage(msg);
	}
}
