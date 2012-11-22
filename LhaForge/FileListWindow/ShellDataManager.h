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

#pragma once

//#include <hash_map>
//#include <string>
//ファイルのアイコンなどを管理する

#if defined(_UNICODE)||defined(UNICODE)
 typedef std::wstring StlString;
#else
 typedef std::string StlString;
#endif

class SHELLDATA{
public:
	int IconIndex;
	CString TypeName;
};

//シェル情報管理クラス
//アイコン、ファイルタイプ名など
class CShellDataManager
{
protected:
	HIMAGELIST			ImageListSmall;
	HIMAGELIST			ImageListLarge;
	std::hash_map<StlString,SHELLDATA> ShellDataMap;
	std::hash_map<StlString,SHELLDATA>::iterator CShellDataManager::RegisterData(LPCTSTR Ext,DWORD Attribute=FILE_ATTRIBUTE_NORMAL);
public:
	virtual ~CShellDataManager(){}
	void Init();
	HIMAGELIST GetImageList(bool bLarge);
	int GetIconIndex(LPCTSTR);
	LPCTSTR GetTypeName(LPCTSTR);
};
