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

#include "Association.h"

//�֘A�t�����擾
//Ext�̏������ɁAFileType�Ƃ���Icon�ɂ��Ē��ׂ�
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
	//�g���q���������œ��ꂷ��
	AssocInfo.Ext.MakeLower();
	//�g���q�ɑΉ�����FileType�擾
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

	//�I���W�i����FileType
	/*
	1.FileType�������Ƃ��AOrgFIleType=.???\OrgFileType(NULL�ɂ��Ȃ肤��)
	2.FileType��LhaForgeArchive_*�ȊO�̎��AOrgFileType=.???\(Default)
	3.FileType��LhaForgeArchive_*�̂Ƃ��AOrgFileType=.???\OrgFileType
	*/
	//1.��3.�̂Ƃ�
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
	else{	//2.�̂Ƃ�
		AssocInfo.OrgFileType=FileType;
		::RegCloseKey(hKey);
	}

	//------------------------------
	// FileType����A�C�R�����擾
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
	//�A�C�R�����̎擾
	AssocInfo.IconFile=(TCHAR*)&Buffer.at(0);
	AssocInfo.IconIndex=PathParseIconLocation(AssocInfo.IconFile.GetBuffer(_MAX_PATH*2));
	AssocInfo.IconFile.ReleaseBuffer();
	TRACE(_T("***%d:::%s\n"),AssocInfo.IconIndex,AssocInfo.IconFile);

	AssocInfo.OrgIconFile=AssocInfo.IconFile;
	AssocInfo.OrgIconIndex=AssocInfo.IconIndex;

	//----------------------------
	// Shell\Open�̃R�}���h��ǂ�
	//----------------------------
	if(!AssocInfo.bOrgStatus){
		//�֘A�t������Ă��Ȃ�
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
