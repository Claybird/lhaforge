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

#include "Association.h"
#include "Utilities/OSUtil.h"

//関連付けを取得
//Extの情報を元に、FileTypeとそのIconについて調べる
bool AssocGetAssociation(ASSOCINFO &AssocInfo)
{
	if(AssocInfo.Ext.IsEmpty()){
		TRACE(_T("AssocGetAssociation():Ext is Empty\n"));
		return false;
	}
	HKEY hKey;
	LONG Ret = ::RegOpenKeyEx(HKEY_CLASSES_ROOT, AssocInfo.Ext, NULL, KEY_READ, &hKey);
	if(ERROR_SUCCESS!=Ret){
		TRACE(_T("AssocGetAssociation():Failed to Open '%s'\n"),AssocInfo.Ext);
		return false;
	}
	//拡張子を小文字で統一する
	AssocInfo.Ext.MakeLower();
	//拡張子に対応するFileType取得
	std::vector<BYTE> Buffer;
	DWORD dwRead=0;
	::RegQueryValueEx(hKey, NULL, NULL, NULL, NULL,&dwRead);
	Buffer.assign(dwRead+1,0);
	Ret = ::RegQueryValueEx(hKey, NULL, NULL, NULL, &Buffer.at(0),&dwRead);
	if(ERROR_SUCCESS!=Ret){
		::RegCloseKey(hKey);
		TRACE(_T("AssocGetAssociation():Failed to Get FileType\n"));
		return false;
	}
	CString FileType=(TCHAR*)&Buffer.at(0);

	//オリジナルのFileType
	/*
	1.FileTypeが無いとき、OrgFIleType=.???\OrgFileType(NULLにもなりうる)
	2.FileTypeがLhaForgeArchive_*以外の時、OrgFileType=.???\(Default)
	3.FileTypeがLhaForgeArchive_*のとき、OrgFileType=.???\OrgFileType
	*/
	//1.と3.のとき
	if(0==FileType.Left(_tcslen(ASSOC_PREFIX)).CompareNoCase(ASSOC_PREFIX)){
		AssocInfo.bOrgStatus=true;
	}
	else{
		AssocInfo.bOrgStatus=false;
	}
	if(FileType.IsEmpty()||(0==FileType.Left(_tcslen(ASSOC_PREFIX)).CompareNoCase(ASSOC_PREFIX))){
		::RegQueryValueEx(hKey, _T("LhaForgeOrgFileType"), NULL, NULL, NULL,&dwRead);
		Buffer.assign(dwRead+1,0);
		Ret = ::RegQueryValueEx(hKey, _T("LhaForgeOrgFileType"), NULL, NULL, &Buffer.at(0),&dwRead);
		::RegCloseKey(hKey);
		if(ERROR_SUCCESS!=Ret){
			AssocInfo.OrgFileType.Empty();
			TRACE(_T("AssocGetAssociation():Failed to Get OrgFileType\n"));
		}
		else{
			AssocInfo.OrgFileType=(TCHAR*)&Buffer.at(0);
		}
	}
	else{	//2.のとき
		AssocInfo.OrgFileType=FileType;
		::RegCloseKey(hKey);
	}

	//------------------------------
	// FileTypeからアイコン情報取得
	//------------------------------
	CString KeyNameBuffer=FileType;
	KeyNameBuffer+=_T("\\DefaultIcon");
	Ret = ::RegOpenKeyEx(HKEY_CLASSES_ROOT, KeyNameBuffer, NULL, KEY_READ, &hKey);
	if(ERROR_SUCCESS!=Ret){
		TRACE(_T("AssocGetAssociation('%s'):Failed to Open '%s' for Default Icon\n"),AssocInfo.Ext,KeyNameBuffer);
		return false;
	}

	dwRead=0;
	::RegQueryValueEx(hKey, NULL, NULL, NULL, NULL,&dwRead);
	Buffer.assign(dwRead+1,0);
	Ret = ::RegQueryValueEx(hKey, NULL, NULL, NULL, &Buffer.at(0),&dwRead);
	::RegCloseKey(hKey);
	if(ERROR_SUCCESS!=Ret){
		TRACE(_T("AssocGetAssociation():Failed to Get IconInfo\n"));
		return false;
	}
	//アイコン情報の取得
	auto path_and_index = UtilPathParseIconLocation((const wchar_t*)&Buffer.at(0));
	AssocInfo.IconFile = path_and_index.first.c_str();
	AssocInfo.IconIndex = path_and_index.second;
	TRACE(_T("***%d:::%s\n"),AssocInfo.IconIndex,AssocInfo.IconFile);

	AssocInfo.OrgIconFile=AssocInfo.IconFile;
	AssocInfo.OrgIconIndex=AssocInfo.IconIndex;

	//----------------------------
	// Shell\Openのコマンドを読む
	//----------------------------
	if(!AssocInfo.bOrgStatus){
		//関連付けされていない
		return true;
	}
	KeyNameBuffer=FileType;
	KeyNameBuffer+=_T("\\shell\\open\\command");
	Ret = ::RegOpenKeyEx(HKEY_CLASSES_ROOT, KeyNameBuffer, NULL, KEY_READ, &hKey);
	if(ERROR_SUCCESS!=Ret){
		TRACE(_T("AssocGetAssociation():Failed to Open '%s' for Open Command\n"),KeyNameBuffer);
		return false;
	}

	dwRead=0;
	::RegQueryValueEx(hKey, NULL, NULL, NULL, NULL,&dwRead);
	Buffer.assign(dwRead+1,0);
	Ret = ::RegQueryValueEx(hKey, NULL, NULL, /*&ValueType*/NULL, &Buffer.at(0),&dwRead);
	::RegCloseKey(hKey);
	if(ERROR_SUCCESS!=Ret){
		TRACE(_T("AssocGetAssociation():Failed to Get ShellOpenCommand\n"));
		return false;
	}
	AssocInfo.ShellOpenCommand=(TCHAR*)&Buffer.at(0);

	return true;
}
