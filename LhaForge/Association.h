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

//関連付け用API

#pragma once

const LPCTSTR ASSOC_PREFIX=_T("LhaForgeArchive_");


class ASSOCINFO{
public:
	CString Ext;	//拡張子;.付きの名前を渡すこと
//	CString FileType;
	CString OrgFileType;
	bool bOrgStatus;	//関連付けされていたらtrue

	CString OrgIconFile;
	int OrgIconIndex;

	CString IconFile;
	int IconIndex;

	CString ShellOpenCommand;
	ASSOCINFO():IconIndex(0),bOrgStatus(false){}
	virtual ~ASSOCINFO(){}
};

#ifdef __LFASSIST__
//関連付けを作成
bool AssocSetAssociation(const ASSOCINFO&);
//関連付けを削除
bool AssocDeleteAssociation(LPCTSTR Ext);
#endif//__LFASSIST__
//関連付けを取得
bool AssocGetAssociation(ASSOCINFO&);

#ifdef __LFASSIST__
//レジストリ再帰的削除
bool AssocRecursiveDeleteKey(HKEY,LPCTSTR);
bool AssocRegExistKey(HKEY,LPCTSTR);
#endif//__LFASSIST__

