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
#include "StringUtil.h"
#include "Utility.h"

void UtilTrimString(CStringW &strTarget,LPCWSTR lpszSubject)
{
	const std::wstring strSubject(lpszSubject);
	int idx;
	for(idx=strTarget.GetLength()-1;idx>=0;idx--){
		wchar_t ch=strTarget.operator[](idx);
		if(-1==index_of(strSubject,ch))break;
	}
	strTarget=strTarget.Left(idx+1);
}

//MFC�X�^�C����CFileDialog�̃t�B���^�[����������
void UtilMakeFilterString(LPCTSTR lpszIn,LPTSTR lpszOut,int outSize)
{
	ASSERT(outSize>=2);
	if(outSize<2)return;
	_tcsncpy_s(lpszOut,outSize,lpszIn,outSize);
	lpszOut[outSize-1]=_T('\0');
	lpszOut[outSize-2]=_T('\0');

	LPTSTR lpChar=lpszOut;
	for(;*lpChar!=_T('\0');lpChar++){
#if !defined(_UNICODE)&&!defined(UNICODE)
		if(_MBC_SINGLE==_mbsbtype((LPCBYTE)lpChar,0))
#endif
			if(*lpChar==_T('|')){
				*lpChar=_T('\0');
			}
	}
	*lpChar++;
	*lpChar=_T('\0');
}

//TCHAR�t�@�C������SJIS�t�@�C�����ŕ\���ł��邩�`�F�b�N����
bool UtilCheckT2A(LPCTSTR lpPath)
{
#if defined(_UNICODE)||defined(UNICODE)
	//���������ϊ��ł��Ă��邩�ǂ����`�F�b�N����
	CStringA strTempA(lpPath);
	CStringW strTempW(strTempA);
	return (strTempW==lpPath);
#else//defined(_UNICODE)||defined(UNICODE)
	//SJIS���̂܂�
	return true;
#endif//defined(_UNICODE)||defined(UNICODE)
}

//�����t�@�C���̂����A��ł�UNICODE��p�t�@�C�����������false
bool UtilCheckT2AList(const std::list<CString> &strList)
{
	for(std::list<CString>::const_iterator ite=strList.begin();ite!=strList.end();++ite){
		if(!UtilCheckT2A(*ite))return false;
	}
	return true;
}


//�K���ȕ����R�[�h->UNICODE
bool UtilToUNICODE(CString &strRet,LPCBYTE lpcByte,DWORD dwSize,UTIL_CODEPAGE uSrcCodePage)
{
#if defined(_UNICODE)||defined(UNICODE)
	switch(uSrcCodePage){
	case UTILCP_SJIS:
		strRet=CStringA((LPCSTR)lpcByte);
		return true;
	case UTILCP_UTF16:
		if(*((LPCWSTR)lpcByte)==0xFEFF){
			//UTF16LE
			strRet=((LPCWSTR)lpcByte)+1;
		}else if(*((LPCWSTR)lpcByte)==0xFFFE){
			//UTF16BE
			//�G���f�B�A���ϊ�
			std::vector<BYTE> cArray(dwSize-2);
			_swab((char*)const_cast<LPBYTE>(lpcByte+2),(char*)&cArray[0],dwSize-2);
			strRet=(LPCWSTR)&cArray[0];
			return true;
		}
		else{
			strRet=(LPCWSTR)lpcByte;
		}
		return true;
	case UTILCP_UTF8:
		{
			if(lpcByte[0]==0xEF && lpcByte[1]==0xBB && lpcByte[2]==0xBF){	//BOM check
				lpcByte+=3;
			}
			std::vector<wchar_t> buf(::MultiByteToWideChar(CP_UTF8,0,(LPCSTR)lpcByte,-1,NULL,0));	//�o�b�t�@�m��
			//�ϊ�
			if(!::MultiByteToWideChar(CP_UTF8,0,(LPCSTR)lpcByte,-1,&buf[0],buf.size())){
				TRACE(_T("�����R�[�h�ϊ����s(UTF8->UTF16)"));
				return false;
			}
			strRet=(LPCWSTR)&buf[0];
			return true;
		}
	default:
		ASSERT(!"This code cannot be run");
		return false;
	}
#else//defined(_UNICODE)||defined(UNICODE)
#error("not implemented")
	return false;
#endif//defined(_UNICODE)||defined(UNICODE)
}

//�K���ȓ��͕����̕����R�[�h�𐄒肵�AUNICODE(UTF16)�ɕϊ�
void UtilGuessToUNICODE(CString &strRet,LPCBYTE lpcByte,DWORD dwSize)
{
#if defined(_UNICODE)||defined(UNICODE)
	if(*((LPCWSTR)lpcByte)==0xFEFF){
		//UTF16LE
		TRACE(_T("���̓e�L�X�g��UTF-16LE�Ɛ��肳���\n"));
		strRet=((LPCWSTR)lpcByte)+1;
	}else if(*((LPCWSTR)lpcByte)==0xFFFE){
		//UTF16BE
		//�G���f�B�A���ϊ�
		TRACE(_T("���̓e�L�X�g��UTF-16BE�Ɛ��肳���\n"));
		std::vector<BYTE> cArray(dwSize-2);
		_swab((char*)const_cast<LPBYTE>(lpcByte+2),(char*)&cArray[0],dwSize-2);
		strRet=(LPCWSTR)&cArray[0];
	}else if(lpcByte[0]==0xEF && lpcByte[1]==0xBB && lpcByte[2]==0xBF){	//BOM check
		TRACE(_T("���̓e�L�X�g��UTF-8 with BOM�Ɛ��肳���\n"));
		UtilToUNICODE(strRet,lpcByte,dwSize,UTILCP_UTF8);
	}else{
		TRACE(_T("���̓e�L�X�g�͔�UNICODE(UTF16)�Ɛ��肳���\n"));
		strRet=CStringA((LPCSTR)lpcByte);
	}
#else//defined(_UNICODE)||defined(UNICODE)
#error("not implemented")
	return false;
#endif//defined(_UNICODE)||defined(UNICODE)
}

//UNICODE->UTF8
bool UtilToUTF8(std::vector<BYTE> &cArray,const CStringW &strSrc)
{
#if defined(_UNICODE)||defined(UNICODE)
	cArray.resize(::WideCharToMultiByte(CP_UTF8,0,strSrc,-1,NULL,0,NULL,NULL));	//�o�b�t�@�m��
	//�ϊ�
	if(!::WideCharToMultiByte(CP_UTF8,0,strSrc,-1,(LPSTR)&cArray[0],cArray.size(),NULL,NULL)){
		TRACE(_T("�����R�[�h�ϊ����s(UTF16->UTF8)"));
		return false;
	}
	return true;
#else//defined(_UNICODE)||defined(UNICODE)
#error("not implemented")
	return false;
#endif//defined(_UNICODE)||defined(UNICODE)
}

inline bool between(WCHAR a,WCHAR begin,WCHAR end){
	return (begin<=a && a<=end);
}

//UNICODE�Ƃ��Ĉ��S�Ȃ�true
bool UtilIsSafeUnicode(LPCTSTR lpChar)
{
#if defined(_UNICODE)||defined(UNICODE)
	ASSERT(lpChar);
	if(!lpChar){
		return true;
	}
	const static WCHAR chars[]={
		0x001e,
		0x001f,
		0x00ad,
		0xFEFF,
		0xFFF9,
		0xFFFA,
		0xFFFB,
		0xFFFE,
	};
	const static UINT len=COUNTOF(chars);
	for(;*lpChar!=L'\0';lpChar++){
		WCHAR c=*lpChar;
		//�P�ƕ����Ƃ̔�r
		for(UINT i=0;i<len;i++){
			if(c==chars[i])return false;
		}

		//�͈͂̂��镶���Ƃ̔�r
		if(between(c,0x200b,0x200f))return false;
		if(between(c,0x202a,0x202e))return false;
		if(between(c,0x2060,0x2063))return false;
		if(between(c,0x206a,0x206f))return false;
	}
	return true;

#else//defined(_UNICODE)||defined(UNICODE)
	return true;
#endif//defined(_UNICODE)||defined(UNICODE)
}


//�w�肳�ꂽ�t�H�[�}�b�g�ŏ����ꂽ�������W�J����
void UtilExpandTemplateString(CString &strOut,LPCTSTR lpszFormat,const std::map<stdString,CString> &env)
{
	strOut=lpszFormat;
	for(std::map<stdString,CString>::const_iterator ite=env.begin();ite!=env.end();++ite){
		//%�Ŏn�܂�u���b�N�͊��ϐ��ƌ��Ȃ��̂�{}�ɂ��C���͍s��Ȃ�
		stdString strKey=( (*ite).first[0]==L'%' ? (*ite).first : _T("{")+(*ite).first+_T("}") );
		strOut.Replace(strKey.c_str(),(*ite).second);
	}
}

void UtilAssignSubString(CString &strOut,LPCTSTR lpStart,LPCTSTR lpEnd)
{
	ASSERT(lpStart);
	ASSERT(lpEnd);
	strOut=_T("");
	for(;lpStart!=lpEnd;++lpStart){
		strOut+=*lpStart;
	}
}
