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

