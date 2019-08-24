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
