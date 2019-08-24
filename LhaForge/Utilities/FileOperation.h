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

#pragma once

LPCTSTR UtilGetTempPath();
bool UtilGetTemporaryFileName(LPTSTR fname,LPCTSTR prefix);
bool UtilDeletePath(LPCTSTR PathName);
bool UtilDeleteDir(LPCTSTR Path,bool);
int UtilAppendFile(HANDLE hWriteTo,HANDLE hReadFrom);
void UtilModifyPath(CString&);	//DTV���N�����\���̂���p�X���C������

BOOL UtilMoveFileToRecycleBin(LPCTSTR);	//�t�@�C�������ݔ��Ɉړ�
BOOL UtilMoveFileToRecycleBin(const std::list<CString>&);	//�t�@�C�������ݔ��Ɉړ�

//�t�H���_���t�@�C��(�f�B���N�g���͏���)���ċA����
bool UtilRecursiveEnumFile(LPCTSTR lpszRoot,std::list<CString>&);

//�t���p�X����΃p�X�̎擾
enum PATHERROR{
	PATHERROR_NONE,		//����
	PATHERROR_INVALID,	//�p�����[�^�w�肪�s��
	PATHERROR_ABSPATH,	//��΃p�X�̎擾�Ɏ��s
	PATHERROR_NOTFOUND,	//�t�@�C���������̓t�H���_��������Ȃ�
	PATHERROR_LONGNAME,	//�����O�t�@�C�����擾���s
};
PATHERROR UtilGetCompletePathName(CString &_FullPath,LPCTSTR lpszFileName);
//��΃p�X�̎擾
bool UtilGetAbsPathName(CString &_FullPath,LPCTSTR lpszFileName);

//���C���h�J�[�h�̓W�J
bool UtilPathExpandWild(std::list<CString> &r_outList,const std::list<CString> &r_inList);
bool UtilPathExpandWild(std::list<CString> &r_outList,const CString &r_inParam);

//�p�X�̃f�B���N�g���������������o��
void UtilPathGetDirectoryPart(CString&);

//�����̃v���O�����̃t�@�C������Ԃ�
LPCTSTR UtilGetModulePath();

//�����̃v���O�����̂����Ă���f�B���N�g���̃p�X����Ԃ�
LPCTSTR UtilGetModuleDirectoryPath();

//�����K�w�̃f�B���N�g������C�ɍ쐬����
BOOL UtilMakeSureDirectoryPathExists(LPCTSTR lpszPath);

//TCHAR�t�@�C������SJIS�t�@�C�����ɕϊ�����B�������ϊ��ł��Ȃ��ꍇ�ɂ́Afalse��Ԃ�
bool UtilPathT2A(CStringA&,LPCTSTR,bool bOnDisk);

//�p�X�ɋ��ʂ��镔�������o���A���p�X�����o��
void UtilGetBaseDirectory(CString &BasePath,const std::list<CString> &PathList);

//�t�@�C�����Ɏg���Ȃ��������u��������
void UtilFixFileName(CString &,LPCTSTR lpszOrg,TCHAR replace);

LPCTSTR UtilPathNextSeparator(LPCTSTR lpStr);
bool UtilPathNextSection(LPCTSTR lpStart,LPCTSTR& r_lpStart,LPCTSTR& r_lpEnd,bool bSkipMeaningless);
//Path��'/'��������'\\'�ŏI����Ă���Ȃ�true
bool UtilPathEndWithSeparator(LPCTSTR lpPath);
void UtilPathGetLastSection(CString &strSection,LPCTSTR lpPath);

//�t�@�C�����ۂ��ƁA�������͎w�肳�ꂽ�Ƃ���܂œǂݍ���(-1�Ŋۂ���)
bool UtilReadFile(LPCTSTR lpFile,std::vector<BYTE> &cReadBuffer,DWORD dwLimit=-1);

struct FILELINECONTAINER{
	virtual ~FILELINECONTAINER(){}
	std::vector<WCHAR> data;
	std::vector<LPCWSTR> lines;
};
bool UtilReadFileSplitted(LPCTSTR lpFile,FILELINECONTAINER&);
