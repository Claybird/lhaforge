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
#include "../resource.h"
#include "Utility.h"
#include "StringUtil.h"
#include "FileOperation.h"
#include "../Dialogs/TextInputDlg.h"

#if !defined(_UNICODE)&&!defined(UNICODE)
 #include <imagehlp.h> //MakeSureDirectoryPathExists()
#endif

#if defined(DEBUG) || defined(_DEBUG)
void UtilDebugTrace(LPCTSTR pszFormat, ...)
{
	va_list	args;

	va_start(args, pszFormat);
	CString str;
	str.FormatV(pszFormat,args);
	va_end(args);

	OutputDebugString(str);
}

void TraceLastError()
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, GetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // �f�t�H���g����
		(LPTSTR)&lpMsgBuf, 0, NULL);
	TRACE(_T("API Error : %s"),lpMsgBuf);
	LocalFree(lpMsgBuf);
}

#endif

//�G���[���b�Z�[�W�\��
int ErrorMessage(LPCTSTR msg)
{
	TRACE(_T("ErrorMessage:")),TRACE(msg),TRACE(_T("\n"));
	return MessageBox(NULL,msg,UtilGetMessageCaption(),MB_OK|MB_ICONSTOP);
}

//���b�Z�[�W�L���v�V�������擾
LPCTSTR UtilGetMessageCaption()
{
	const static CString strCaption(MAKEINTRESOURCE(IDS_MESSAGE_CAPTION));
	return strCaption;
}

void UtilGetLastErrorMessage(CString &strMsg)
{
	//���s�G���[
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,GetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);

	strMsg=(LPCTSTR)lpMsgBuf;
	LocalFree(lpMsgBuf);
}


//�z��̒��Ɏw�肳�ꂽ�������L��΂��̈ʒu��Ԃ�
int UtilCheckNumberArray(const int *lpcArray,int size,int c)
{
	for(int i=0;i<size;i++){
		if(lpcArray[i]==c)return i;
	}
	return -1;
}

//�t�@�C�������w�肵���p�^�[���ɓ��Ă͂܂��true
bool UtilExtMatchSpec(LPCTSTR lpszPath,LPCTSTR lpPattern)
{
	const CString strBuf=lpPattern;
	int Index=0,CopyFrom=0;

	while(true){
		Index=strBuf.Find(_T(';'),Index);
		CString strMatchSpec;
		if(-1==Index){
			strMatchSpec=strBuf.Mid(CopyFrom);
		}else{
			strMatchSpec=strBuf.Mid(CopyFrom,Index-CopyFrom);
		}
		if(!strMatchSpec.IsEmpty()){	//�g���q�m�F
			if(strMatchSpec[0]!=L'.')strMatchSpec=L'.'+strMatchSpec;	//.����n�܂�悤�ɕ₤
			strMatchSpec.Insert(0,_T("*"));	//.ext->*.ext
			if(PathMatchSpec(lpszPath,strMatchSpec)){
				return true;
			}
		}
		if(-1==Index){	//�����I��
			break;
		}else{
			Index++;
			CopyFrom=Index;
		}
	}
	return false;
}

//�t�@�C�������w�肵��2�̏�����[����]����邩�ǂ���;���ۂ��D��
bool UtilPathAcceptSpec(LPCTSTR lpszPath,LPCTSTR lpDeny,LPCTSTR lpAccept,bool bDenyOnly)
{
	if(UtilExtMatchSpec(lpszPath,lpDeny)){
		return false;
	}
	if(bDenyOnly){
		return true;
	}else{
		if(UtilExtMatchSpec(lpszPath,lpAccept)){
			return true;
		}
	}
	return false;
}

//���X�|���X�t�@�C����ǂݎ��
bool UtilReadFromResponceFile(LPCTSTR lpszRespFile,UTIL_CODEPAGE uSrcCodePage,std::list<CString> &FileList)
{
	ASSERT(lpszRespFile);
	if(!lpszRespFile)return false;
	if(!PathFileExists(lpszRespFile))return false;

	HANDLE hFile=CreateFile(lpszRespFile,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
	if(INVALID_HANDLE_VALUE==hFile)return false;

	//4GB�z���t�@�C���͈���Ȃ�
	const DWORD dwSize=GetFileSize(hFile,NULL);
	std::vector<BYTE> cReadBuffer;
	cReadBuffer.resize(dwSize+2);
	DWORD dwRead;
	//---�ǂݍ���
	if(!ReadFile(hFile,&cReadBuffer[0],dwSize,&dwRead,NULL)||dwSize!=dwRead){
		CloseHandle(hFile);
		return false;
	}
	CloseHandle(hFile);

	//---�����R�[�h�ϊ�
	//�I�[�����ǉ�
	switch(uSrcCodePage){
	case UTILCP_SJIS:
	case UTILCP_UTF8:	//FALLTHROUGH
		cReadBuffer[dwSize]='\0';
		break;
	case UTILCP_UTF16:
		*((LPWSTR)&cReadBuffer[dwSize])=L'\0';
		break;
	default:
		ASSERT(!"This code canno be run");
		return false;
	}
	//�����R�[�h�ϊ�
	CString strBuffer;
	if(!UtilToUNICODE(strBuffer,&cReadBuffer[0],cReadBuffer.size(),uSrcCodePage))return false;

	LPCTSTR p=strBuffer;
	const LPCTSTR end=p+strBuffer.GetLength()+1;
	//����
	CString strLine;
	for(;p!=end;p++){
		if(*p==_T('\n')||*p==_T('\r')||*p==_T('\0')){
			if(!strLine.IsEmpty()){
				CPath tmpPath(strLine);
				tmpPath.UnquoteSpaces();	//""���O��
				FileList.push_back(tmpPath);
			}
			strLine.Empty();
		}
		else 
			strLine+=*p;
	}
	return true;
}

//INI�ɐ����𕶎���Ƃ��ď�������
BOOL UtilWritePrivateProfileInt(LPCTSTR lpAppName,LPCTSTR lpKeyName,LONG nData,LPCTSTR lpFileName)
{
	TCHAR Buffer[32]={0};
	wsprintf(Buffer,_T("%ld"),nData);
	return ::WritePrivateProfileString(lpAppName,lpKeyName,Buffer,lpFileName);
}


//INI�Ɏw�肳�ꂽ�Z�N�V����������Ȃ�true��Ԃ�
bool UtilCheckINISectionExists(LPCTSTR lpAppName,LPCTSTR lpFileName)
{
	TCHAR szBuffer[10];
	DWORD dwRead=GetPrivateProfileSection(lpAppName,szBuffer,9,lpFileName);
	return dwRead>0;
}

//���������͂�����
bool UtilInputText(LPCTSTR lpszMessage,CString &strInput)
{
	CInputDialog dlg(lpszMessage,strInput);
	return IDOK==dlg.DoModal();
}


//�^����ꂽ�t�@�C�������}���`�{�����[�����ɂƌ��Ȃ���Ȃ�true��Ԃ�
bool UtilIsMultiVolume(LPCTSTR lpszPath,CString &r_strFindParam)
{
	//������
	r_strFindParam.Empty();

	CPath strPath(lpszPath);
	if(strPath.IsDirectory())return false;	//�f�B���N�g���Ȃ疳�����ɕԂ�

	strPath.StripPath();	//�t�@�C�����݂̂�
	int nExt=strPath.FindExtension();	//�g���q��.�̈ʒu
	if(-1==nExt)return false;	//�g���q�͌����炸

	CString strExt((LPCTSTR)strPath+nExt);
	strExt.MakeLower();	//��������
	if(strExt==_T(".rar")){
		//---RAR
		if(strPath.MatchSpec(_T("*.part*.rar"))){
			//����������̍쐬
			CPath tempPath(lpszPath);
			tempPath.RemoveExtension();	//.rar�̍폜
			tempPath.RemoveExtension();	//.part??�̍폜
			tempPath.AddExtension(_T(".part*.rar"));

			r_strFindParam=(CString)tempPath;
			return true;
		}else{
			return false;
		}
	}
	//TODO:�g�p�p�x�Ǝ����̊ȕւ����l����rar�̂ݑΉ��Ƃ���
	return false;
}


//�����I�Ƀ��b�Z�[�W���[�v����
bool UtilDoMessageLoop()
{
	MSG msg;
	if(PeekMessage (&msg,NULL,0,0,PM_NOREMOVE)){
		if(!GetMessage (&msg,NULL,0,0)){
			return false;
		}

		TranslateMessage(&msg);
		DispatchMessage(&msg);
		return true;
	}
	return false;
}

VOID CALLBACK UtilMessageLoopTimerProc(HWND,UINT,UINT,DWORD)
{
	while(UtilDoMessageLoop())continue;
}


//�W���̐ݒ�t�@�C���̃p�X���擾
//bUserCommon�̓��[�U�[�Ԃŋ��ʐݒ���g���ꍇ��true����������
//lpszDir��ApplicationData�ɓ����Ƃ��ɕK�v�ȃf�B���N�g����
//lpszFile�͒T���t�@�C����
void UtilGetDefaultFilePath(CString &strPath,LPCTSTR lpszDir,LPCTSTR lpszFile,bool &bUserCommon)
{
	//---���[�U�[�Ԃŋ��ʂ̐ݒ��p����
	//LhaForge�t�H���_�Ɠ����ꏊ��INI������Ύg�p����
	{
		TCHAR szCommonIniPath[_MAX_PATH+1]={0};
		_tcsncpy_s(szCommonIniPath,UtilGetModuleDirectoryPath(),_MAX_PATH);
		PathAppend(szCommonIniPath,lpszFile);
		if(PathFileExists(szCommonIniPath)){
			//���ʐݒ�
			bUserCommon=true;
			strPath=szCommonIniPath;
			TRACE(_T("Common INI(Old Style) '%s' found.\n"),strPath);
			return;
		}
	}
	//CSIDL_COMMON_APPDATA��INI������Ύg�p����
	{
		TCHAR szCommonIniPath[_MAX_PATH+1]={0};
		SHGetFolderPath(NULL,CSIDL_COMMON_APPDATA|CSIDL_FLAG_CREATE,NULL,SHGFP_TYPE_CURRENT,szCommonIniPath);
		PathAppend(szCommonIniPath,lpszDir);
		PathAppend(szCommonIniPath,lpszFile);
		if(PathFileExists(szCommonIniPath)){
			//���ʐݒ�
			bUserCommon=true;
			strPath=szCommonIniPath;
			TRACE(_T("Common INI '%s' found.\n"),strPath);
			return;
		}
	}

	//--------------------

	//---���[�U�[�ʐݒ��p����
	//LhaForge�C���X�g�[���t�H���_�ȉ��Ƀt�@�C�������݂���ꍇ�A������g�p
	{
		//���[�U�[���擾
		TCHAR UserName[UNLEN+1]={0};
		DWORD Length=UNLEN;
		GetUserName(UserName,&Length);

		TCHAR szIniPath[_MAX_PATH+1];
		_tcsncpy_s(szIniPath,UtilGetModuleDirectoryPath(),_MAX_PATH);
		PathAppend(szIniPath,UserName);
		PathAddBackslash(szIniPath);
		//MakeSureDirectoryPathExists(szIniPath);

		PathAppend(szIniPath,lpszFile);

		if(PathFileExists(szIniPath)){
			bUserCommon=false;
			strPath=szIniPath;
			TRACE(_T("Personal INI(Old Style) '%s' found.\n"),strPath);
			return;
		}
	}
	//---�f�t�H���g
	//CSIDL_APPDATA��INI������Ύg�p����:Vista�ł͂���ȊO�̓A�N�Z�X�����s���ɂȂ�\��������
	TCHAR szIniPath[_MAX_PATH+1]={0};
	SHGetFolderPath(NULL,CSIDL_APPDATA|CSIDL_FLAG_CREATE,NULL,SHGFP_TYPE_CURRENT,szIniPath);
	PathAppend(szIniPath,lpszDir);
	PathAddBackslash(szIniPath);
	UtilMakeSureDirectoryPathExists(szIniPath);
	PathAppend(szIniPath,lpszFile);
	bUserCommon=false;
	strPath=szIniPath;
	TRACE(_T("Personal INI '%s' found.\n"),strPath);
	return;
}




