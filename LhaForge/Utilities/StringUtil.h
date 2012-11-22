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
//---�����񏈗�

//�����R�[�h
enum UTIL_CODEPAGE{
	UTILCP_SJIS,
	UTILCP_UTF8,
	UTILCP_UTF16
};


//MFC�X�^�C����CFileDialog�̃t�B���^�[����������
void UtilMakeFilterString(LPCTSTR,LPTSTR,int);


//�K���ȕ����R�[�h->UNICODE
//dwSize��UTILCP_UTF16�̂Ƃ��̂ݕK�v
bool UtilToUNICODE(CString &strRet,LPCBYTE lpcByte,DWORD dwSize,UTIL_CODEPAGE uSrcCodePage);
//UTF16-BE/UTF16-LE/SJIS���������肵��UNICODE��
void UtilGuessToUNICODE(CString &strRet,LPCBYTE lpcByte,DWORD dwSize);
//UNICODE->UTF8
bool UtilToUTF8(std::vector<BYTE> &cArray,const CStringW &strSrc);

//UNICODE��UTF8�ɕϊ����邽�߂̃A�_�v�^�N���X
class C2UTF8{
protected:
	std::vector<BYTE> m_cArray;
public:
	C2UTF8(const CStringW &str){
		UtilToUTF8(m_cArray,str);
	}
	virtual ~C2UTF8(){}
	operator LPCSTR(){return (LPCSTR)&m_cArray[0];}
};

//UNICODE�Ƃ��Ĉ��S�Ȃ�true
bool UtilIsSafeUnicode(LPCTSTR);

//TCHAR�t�@�C������SJIS�t�@�C�����ŕ\���ł���Ȃ�true
bool UtilCheckT2A(LPCTSTR);
bool UtilCheckT2AList(const std::list<CString>&);	//�����t�@�C���̂����A��ł�UNICODE��p�t�@�C�����������false

//��������w�肳�ꂽ���������
void UtilTrimString(CStringW&,LPCWSTR lpszSubject);

//�w�肳�ꂽ�t�H�[�}�b�g�ŏ����ꂽ�������W�J����
void UtilExpandTemplateString(CString &strOut,LPCTSTR lpszFormat,const std::map<stdString,CString> &env);

void UtilAssignSubString(CString &strOut,LPCTSTR lpStart,LPCTSTR lpEnd);
