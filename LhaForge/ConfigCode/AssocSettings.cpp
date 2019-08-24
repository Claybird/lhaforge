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
#include "AssocSettings.h"
#include "../Utilities/FileOperation.h"

ASSOC_SETTINGS::ASSOC_SETTINGS()
	:bChanged(false)
{
}


ASSOC_SETTINGS::~ASSOC_SETTINGS()
{
}

//関連付け情報が、LhaForgeの物と一致しているかどうかチェック
bool ASSOC_SETTINGS::CheckAssociation(LPCTSTR lpShellOpenCommand_Desired)
{
#ifdef ASSOC_NOCHECK
	//いちいち関連付けのチェックを行わない
	return true;
#endif

	if(!AssocInfo.bOrgStatus){
		//もともと関連付けされていなかった
		TRACE(_T("Not Associated\n"));
		return true;
	}

	ASSERT(lpShellOpenCommand_Desired);
	if(!lpShellOpenCommand_Desired)return false;

	TRACE(_T("AssocInfo.ShellOpenCommand=%s\n"),AssocInfo.ShellOpenCommand);

	if(lpShellOpenCommand_Desired!=AssocInfo.ShellOpenCommand){
		TRACE(_T("Not Match\n"));
		bChanged=true;
	}
	return true;
}

//関連付け情報からアイコンを取得
void ASSOC_SETTINGS::SetIconFromAssoc(CIcon &IconSystemDefault)
{
	if(AssocInfo.OrgIconFile.IsEmpty()){
		SetIcon(IconSystemDefault);
	}
	else{
		SetIcon(AssocInfo.OrgIconFile,AssocInfo.OrgIconIndex);
	}
}

void ASSOC_SETTINGS::SetIcon(CIcon &icon)
{
	Picture_Icon.SetIcon(icon);
}

void ASSOC_SETTINGS::SetIcon(LPCTSTR path,int idx)
{
	if(!Icon.IsNull())Icon.DestroyIcon();
	Icon.ExtractIcon(path,idx);
	Picture_Icon.SetIcon(Icon);
}
