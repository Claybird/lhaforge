/*
 * Copyright (c) 2005-, Claybird
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
#include "shellmanager.h"
#include "../Utilities/Utility.h"
#include "../Utilities/OSUtil.h"
#include "../resource.h"


const int CLSID_STRING_SIZE=(39 + 1);

//---IA32
//�E�N���b�N���j���[�n���h��
// {713B479F-6F2B-48e9-B545-5591CCFE398F}
static const GUID CLSID_ShellExtShellMenu32 = 
{ 0x713b479f, 0x6f2b, 0x48e9, { 0xb5, 0x45, 0x55, 0x91, 0xcc, 0xfe, 0x39, 0x8f } };

//�E�h���b�O���j���[�n���h��
// {5E5B692B-D6ED-4103-A1FA-9A71A93DAC88}
static const GUID CLSID_ShellExtDragMenu32 = 
{ 0x5e5b692b, 0xd6ed, 0x4103, { 0xa1, 0xfa, 0x9a, 0x71, 0xa9, 0x3d, 0xac, 0x88 } };

//---AMD64
//�E�N���b�N���j���[�n���h��
// {B7584D74-DE0C-4db5-80DD-42EEEDF42665}
static const GUID CLSID_ShellExtShellMenu64 = 
{ 0xb7584d74, 0xde0c, 0x4db5, { 0x80, 0xdd, 0x42, 0xee, 0xed, 0xf4, 0x26, 0x65 } };

//�E�h���b�O���j���[�n���h��
// {00521ADB-148D-45c9-8021-7446EE35609D}
static const GUID CLSID_ShellExtDragMenu64 = 
{ 0x521adb, 0x148d, 0x45c9, { 0x80, 0x21, 0x74, 0x46, 0xee, 0x35, 0x60, 0x9d } };


//-------------------------------------------------------------------------
// ShellRegisterServer			�g���V�F����o�^
//-------------------------------------------------------------------------
bool ShellRegisterServer(HWND hWnd,LPCTSTR inDllPath)
{
	// ���C�u���������[�h
	HINSTANCE theDllH = ::LoadLibrary(inDllPath);
	if(!theDllH){
		// ���[�h�o���܂���ł���
		CString msg;
		msg.Format(IDS_ERROR_DLL_LOAD,inDllPath);
		MessageBox(hWnd,msg,CString(MAKEINTRESOURCE(IDS_MESSAGE_CAPTION)),MB_OK|MB_ICONSTOP);
		return false;
	}

	// �G���g���[�|�C���g��T���Ď��s
	FARPROC lpDllEntryPoint;
	(FARPROC&)lpDllEntryPoint = ::GetProcAddress(theDllH,"DllRegisterServer");
	if(lpDllEntryPoint){
		// �o�^
		(*lpDllEntryPoint)();
//		MessageBeep(MB_OK);
	}else{
		// DllRegisterServer ��������܂���
		CString msg;
		msg.Format(IDS_ERROR_DLL_FUNCTION_GET,inDllPath,_T("DllRegisterServer"));
		MessageBox(hWnd,msg,CString(MAKEINTRESOURCE(IDS_MESSAGE_CAPTION)),MB_OK|MB_ICONSTOP);
		::FreeLibrary(theDllH);
		return false;
	}
	::FreeLibrary(theDllH);
	return true;
}

//-------------------------------------------------------------------------
// ShellUnregisterServer			�g���V�F��������
//-------------------------------------------------------------------------
bool ShellUnregisterServer(HWND hWnd,LPCTSTR inDllPath)
{
	// ���C�u���������[�h
	HINSTANCE theDllH = ::LoadLibrary(inDllPath);
	if(!theDllH){
		// ۰�ޏo���܂���ł���
		CString msg;
		msg.Format(IDS_ERROR_DLL_LOAD,inDllPath);
		MessageBox(hWnd,msg,CString(MAKEINTRESOURCE(IDS_MESSAGE_CAPTION)),MB_OK|MB_ICONSTOP);
		return false;
	}

	// �G���g���[�|�C���g��T���Ď��s
	FARPROC	lpDllEntryPoint;
	(FARPROC&)lpDllEntryPoint = ::GetProcAddress(theDllH,"DllUnregisterServer");
	if(lpDllEntryPoint){
		// �o�^
		(*lpDllEntryPoint)();
//		MessageBeep(MB_OK);
	}else{
		// DllUnregisterServer ��������܂���
		CString msg;
		msg.Format(IDS_ERROR_DLL_FUNCTION_GET,inDllPath,_T("DllUnregisterServer"));
		MessageBox(hWnd,msg,CString(MAKEINTRESOURCE(IDS_MESSAGE_CAPTION)),MB_OK|MB_ICONSTOP);
		::FreeLibrary(theDllH);
		return false;
	}
	::FreeLibrary(theDllH);
	return true;
}


/*-------------------------------------------------------------------------*/
// CLSIDtoSTRING			�N���XID�𕶎���ɕϊ�
/*-------------------------------------------------------------------------*/
void CLSIDtoSTRING(REFCLSID inClsid,CString &outCLSID)
{
	WCHAR theCLSID[CLSID_STRING_SIZE];

	// �N���XID�𕶎���ɕϊ�����B
	StringFromGUID2(inClsid, theCLSID,CLSID_STRING_SIZE);
	outCLSID=theCLSID;
}

//-------------------------------------------------------------------------
// ShellRegistCheck		�g��Shell�����ݓo�^����Ă��邩�`�F�b�N
//-------------------------------------------------------------------------
bool _ShellRegistCheck(const GUID inGUID)
{
	bool Result=false;
	// �N���XID����A���W�X�g���L�[�����쐬����
	CString theCLSID;
	CLSIDtoSTRING(inGUID, theCLSID);

	// �L�[���쐬
	CString theKey=_T("CLSID\\")+theCLSID;

	// ���W�X�g���ɃL�[�����邩�ǂ����T��

	// �w��L�[�̃I�[�v��
	HKEY theKeyChildH;
	int flag=KEY_READ;
	if(UtilIsWow64())flag|=KEY_WOW64_64KEY;
	LONG theRes=::RegOpenKeyEx(HKEY_CLASSES_ROOT, theKey, 0, flag, &theKeyChildH);
	//�w��L�[�̃I�[�v��
	if(theRes != ERROR_SUCCESS){
		return Result;
	}

	//�w��L�[�ɕ������֘A�Â����Ă���Α��`�F�b�N�˔j
	DWORD dwLength=0;
	::RegQueryValueEx(theKeyChildH, NULL, NULL, NULL, NULL,&dwLength);
	if(dwLength>1){
		//���`�F�b�N
		// �q�G���g���̗�
		TCHAR		theBuffer[256];
		DWORD		theSize = 256;
		FILETIME	theTime;
		DWORD		theIdx = 0;
		while(S_OK==::RegEnumKeyEx(theKeyChildH, theIdx, theBuffer, &theSize, NULL, NULL, NULL, &theTime)){
			if(_tcsicmp(theBuffer,_T("InprocServer32")) == 0){
				// �r���S�I
				Result = true;
				break;
			}
			theSize = 256;
			theIdx++;
		}
	}

	// Close the child.
	::RegCloseKey(theKeyChildH);

	return Result;
}


bool ShellRegistCheck()
{
	return (_ShellRegistCheck(CLSID_ShellExtShellMenu32) || _ShellRegistCheck(CLSID_ShellExtShellMenu64));
}
