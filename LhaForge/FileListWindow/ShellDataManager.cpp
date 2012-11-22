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
#include "ShellDataManager.h"
#include "../ArchiverCode/ArcEntryInfo.h"

int CShellDataManager::GetIconIndex(LPCTSTR Ext)
{
	std::hash_map<StlString,SHELLDATA>::iterator ite=ShellDataMap.find(Ext);
	if(ite!=ShellDataMap.end()){
		//既に登録されていた
		return (*ite).second.IconIndex;
	}
	//マップに未登録だったら登録する

	ite=RegisterData(Ext);
	return (*ite).second.IconIndex;
}

LPCTSTR CShellDataManager::GetTypeName(LPCTSTR Ext)
{
	std::hash_map<StlString,SHELLDATA>::iterator ite=ShellDataMap.find(Ext);
	if(ite!=ShellDataMap.end()){
		//既に登録されていた
		return (*ite).second.TypeName;
	}
	//マップに未登録だったら登録する

	ite=RegisterData(Ext);
	return (*ite).second.TypeName;
}

std::hash_map<StlString,SHELLDATA>::iterator CShellDataManager::RegisterData(LPCTSTR Ext,DWORD Attribute)
{
	SHELLDATA ShellData;
	//ファイル アイコン インデックス取得
	SHFILEINFO shfi;
	SHGetFileInfo(Ext ? Ext : _T("dummy"),Attribute,&shfi,sizeof(shfi),SHGFI_USEFILEATTRIBUTES|SHGFI_ICON|SHGFI_LARGEICON|SHGFI_SYSICONINDEX);
	ShellData.IconIndex=shfi.iIcon;

	//ファイル形式名取得
	SHGetFileInfo(Ext ? Ext : _T("dummy"),Attribute,&shfi,sizeof(shfi),SHGFI_USEFILEATTRIBUTES|SHGFI_TYPENAME);
	ShellData.TypeName=shfi.szTypeName;

	std::pair<std::hash_map<StlString,SHELLDATA>::iterator,bool> Result;

	if(!Ext)Ext=FOLDER_EXTENSION_STRING;
	Result=ShellDataMap.insert(std::pair<StlString,SHELLDATA>(Ext,ShellData));
	return Result.first;
}


void CShellDataManager::Init()
{
	SHFILEINFO shfi;
	ImageListLarge=(HIMAGELIST)SHGetFileInfo(_T(""),0,&shfi,sizeof(shfi),SHGFI_SYSICONINDEX | SHGFI_ICON | SHGFI_LARGEICON);
	ImageListSmall=(HIMAGELIST)SHGetFileInfo(_T(""),0,&shfi,sizeof(shfi),SHGFI_SYSICONINDEX | SHGFI_ICON | SHGFI_SMALLICON);
	//アイコン(フォルダ用)を抽出
	RegisterData(NULL,FILE_ATTRIBUTE_DIRECTORY);
}

HIMAGELIST CShellDataManager::GetImageList(bool bLarge)
{
	if(bLarge){
		return ImageListLarge;
	}
	else{
		return ImageListSmall;
	}
}

