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
