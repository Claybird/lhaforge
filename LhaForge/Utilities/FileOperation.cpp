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
#include "FileOperation.h"
#include "Utility.h"
#include "StringUtil.h"
#include "OSUtil.h"


#if !defined(_UNICODE)&&!defined(UNICODE)
 #include <imagehlp.h> //MakeSureDirectoryPathExists()
#endif

LPCTSTR UtilGetTempPath()
{
	static CPath s_tempPath;
	if(_tcslen(s_tempPath)==0){		//�����ݒ�
		//���ϐ��擾
		std::map<stdString,stdString> envs;
		UtilGetEnvInfo(envs);
		if(!has_key(envs,_T("TMP")) && !has_key(envs,_T("TEMP"))){
			//%TMP%/%TEMP%�����݂��Ȃ���Ύ��O�̈ꎞ�t�H���_���g��(C:\Users\xxx\AppData\Roaming\LhaForge\temp)
			TCHAR szPath[_MAX_PATH+1]={0};
			SHGetFolderPath(NULL,CSIDL_APPDATA|CSIDL_FLAG_CREATE,NULL,SHGFP_TYPE_CURRENT,szPath);
			s_tempPath=szPath;
			s_tempPath.Append(_T("lhaforge\\temp\\"));
			UtilMakeSureDirectoryPathExists(s_tempPath);
		}else{
			//�ʏ�̃p�X
			std::vector<TCHAR> buffer(GetTempPath(0,NULL)+1);
			GetTempPath(buffer.size(),&buffer[0]);
			buffer.back()=_T('\0');
			s_tempPath=&buffer[0];
			s_tempPath.AddBackslash();
		}
	}
	return s_tempPath;
}

bool UtilGetTemporaryFileName(LPTSTR fname,LPCTSTR prefix)
{
	if(!GetTempFileName(UtilGetTempPath(),prefix,0,fname)){
		return false;
	}
	return true;
}

bool UtilDeletePath(LPCTSTR PathName)
{
	if( PathIsDirectory(PathName) ) {//�f�B���N�g��
		//�t�@�C���̑�����W���ɖ߂�
		if( UtilDeleteDir(PathName, true) )return true;
	} else if( PathFileExists(PathName) ) {//�t�@�C��
		//�t�@�C���̑�����W���ɖ߂�
		SetFileAttributes(PathName, FILE_ATTRIBUTE_NORMAL);
		if( DeleteFile(PathName) )return true;
	}
	return false;
}

//bDeleteParent=true�̂Ƃ��APath���g���폜����
//bDeleteParent=false�̂Ƃ��́APath�̒��g�����폜����
bool UtilDeleteDir(LPCTSTR Path,bool bDeleteParent)
{
	std::vector<WIN32_FIND_DATA> lps;

	TCHAR FindParam[_MAX_PATH+1];
	FILL_ZERO(FindParam);
	_tcsncpy_s(FindParam,Path,_MAX_PATH);
	PathAppend(FindParam, _T("*"));

	{
		WIN32_FIND_DATA fd;
		HANDLE h = FindFirstFile(FindParam, &fd);
		do {
			lps.push_back(fd);
		} while( FindNextFile(h, &fd) );
		FindClose(h);
	}

	bool bRet=true;

	for( std::vector<WIN32_FIND_DATA>::const_iterator ite = lps.begin(); ite != lps.end();++ite ) {
		const WIN32_FIND_DATA& lp = *ite;
		if( ( lp.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) && 0 != _tcscmp(lp.cFileName, _T("..")) && 0 != _tcscmp(lp.cFileName, _T(".")) ) {
			CString SubPath = Path;
			SubPath += _T("\\");
			SubPath += lp.cFileName;

			bRet = bRet&&UtilDeleteDir(SubPath, true);
		}
		if( ( lp.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) != FILE_ATTRIBUTE_DIRECTORY ) {
			// lp.cFileName�Ńt�@�C������������
			TCHAR FileName[_MAX_PATH + 1];
			FILL_ZERO(FileName);
			_tcsncpy_s(FileName, Path, _MAX_PATH);
			PathAppend(FileName, lp.cFileName);

			SetFileAttributes(FileName, FILE_ATTRIBUTE_NORMAL);
			if( !DeleteFile(FileName) )bRet = false;
		}
	}
	if(bDeleteParent){
		//bRet=bRet&&UtilDeletePath(Path);
		if(!RemoveDirectory(Path))bRet=false;
	}

	return bRet;
}


//�ǂݎ�茳�t�@�C�����������ݐ�t�@�C����seek�̂���ӏ������ɃR�s�[���ď�������
//�߂�l�̓G���[�Ȃ��Ȃ�0,�ǂݎ��G���[��1,�������݃G���[��-1
int UtilAppendFile(HANDLE hWriteTo,HANDLE hReadFrom)
{
	//16KB���R�s�[
	DWORD dwRead=0,dwWrite=0;
	std::vector<BYTE> Buffer(16*1024);
	for(;;){
		if(!ReadFile(hReadFrom,&Buffer.at(0),16*1024,&dwRead,NULL)){
			return 1;
		}
		if(0==dwRead){
			break;
		}
		if(!WriteFile(hWriteTo,&Buffer.at(0),dwRead,&dwWrite,NULL)){
			return -1;
		}
		if(dwRead!=dwWrite){
			return -1;
		}
	}
	return 0;
}

void UtilModifyPath(CString &strPath)
{
	// �p�X�̏C��
	strPath.Replace(_T("/"),_T("\\"));	//�p�X��؂蕶��

	int Ret=0;
	do{
		Ret=strPath.Replace(_T("\\\\"),_T("\\"));	//\\��\�ɒu�������Ă���
	}while(Ret!=0);
	strPath.Replace(_T("..\\"),_T("__\\"));	//���΃p�X�w��..��__�ɒu��������
	strPath.Replace(_T(":"),_T("__"));	//�h���C�u�����u��������(C:����C__��)
}

BOOL UtilMoveFileToRecycleBin(LPCTSTR lpFileName)
{
	ASSERT(lpFileName);
	if(!lpFileName)return false;
	const UINT len=_tcslen(lpFileName);
	std::vector<TCHAR> Buffer(len+2);
	_tcsncpy_s(&Buffer[0],len+1,lpFileName,len+1);
	Buffer.back()=_T('\0');

	SHFILEOPSTRUCT shfo={0};
	shfo.wFunc=FO_DELETE;	//�폜
	shfo.pFrom=&Buffer[0];//�t�@�C�����̃��X�g������\0\0�ŏI���K�v�L��
	shfo.fFlags=FOF_SILENT/*�i���󋵂�\�����Ȃ�*/|FOF_ALLOWUNDO/*UNDO���t��;���ݔ���*/|FOF_NOCONFIRMATION/*�m�F�_�C�A���O��\�����Ȃ�*/;
	return SHFileOperation(&shfo);
}

BOOL UtilMoveFileToRecycleBin(const std::list<CString> &fileList)
{
	ASSERT(!fileList.empty());
	if(fileList.empty())return false;

	//�����쐬
	CString Param;
	for(std::list<CString>::const_iterator ite=fileList.begin();ite!=fileList.end();++ite){
		Param+=*ite;
		Param+=_T('|');
	}
	Param+=_T('|');
	std::vector<TCHAR> Buffer(Param.GetLength()+1);

	UtilMakeFilterString(Param,&Buffer[0],Buffer.size());

	SHFILEOPSTRUCT shfo={0};
	shfo.wFunc=FO_DELETE;	//�폜
	shfo.pFrom=&Buffer[0];//�t�@�C�����̃��X�g������\0\0�ŏI���K�v�L��
	shfo.fFlags=FOF_SILENT/*�i���󋵂�\�����Ȃ�*/|FOF_ALLOWUNDO/*UNDO���t��;���ݔ���*/|FOF_NOCONFIRMATION/*�m�F�_�C�A���O��\�����Ȃ�*/;
	return SHFileOperation(&shfo);
}

//�t�H���_���t�@�C��(�f�B���N�g���͏���)���ċA����
bool UtilRecursiveEnumFile(LPCTSTR lpszRoot,std::list<CString> &rFileList)
{
	CFindFile cFindFile;
	TCHAR szPath[_MAX_PATH+1];
	_tcsncpy_s(szPath,lpszRoot,_MAX_PATH);
	PathAppend(szPath,_T("*"));

	BOOL bContinue=cFindFile.FindFile(szPath);
	while(bContinue){
		if(!cFindFile.IsDots()){
			if(cFindFile.IsDirectory()){
				UtilRecursiveEnumFile(cFindFile.GetFilePath(),rFileList);
			}else{
				rFileList.push_back(cFindFile.GetFilePath());
			}
		}
		bContinue=cFindFile.FindNextFile();
	}

	return !rFileList.empty();
}

//�t���p�X����΃p�X�̎擾
PATHERROR UtilGetCompletePathName(CString &_FullPath,LPCTSTR lpszFileName)
{
	ASSERT(lpszFileName&&_tcslen(lpszFileName)>0);
	if(!lpszFileName||_tcslen(lpszFileName)<=0){
		TRACE(_T("�t�@�C�������w�肳��Ă��Ȃ�\n"));
		return PATHERROR_INVALID;
	}

	//---��΃p�X�擾
	TCHAR szAbsPath[_MAX_PATH+1]={0};
	{
		TCHAR Buffer[_MAX_PATH+1]={0};	//���[�g���ǂ����̃`�F�b�N���s��
		_tcsncpy_s(Buffer,lpszFileName,_MAX_PATH);
		PathAddBackslash(Buffer);
		if(PathIsRoot(Buffer)){
			//�h���C�u���������w�肳��Ă���ꍇ�A
			//_tfullpath�͂��̃h���C�u�̃J�����g�f�B���N�g�����擾���Ă��܂�
			_tcsncpy_s(szAbsPath,lpszFileName,_MAX_PATH);
		}
		else if(!_tfullpath(szAbsPath,lpszFileName,_MAX_PATH)){
			TRACE(_T("��΃p�X�擾���s\n"));
			return PATHERROR_ABSPATH;
		}
	}

	if(!PathFileExists(szAbsPath)&&!PathIsDirectory(szAbsPath)){
		//�p�X���t�@�C���������̓f�B���N�g���Ƃ��đ��݂��Ȃ��Ȃ�A�G���[�Ƃ���
		TRACE(_T("�t�@�C�������݂��Ȃ�\n"));
		return PATHERROR_NOTFOUND;
	}
	if(!GetLongPathName(szAbsPath,szAbsPath,_MAX_PATH)){
		TRACE(_T("�����O�t�@�C�����擾���s\n"));
		return PATHERROR_LONGNAME;
	}
	_FullPath=szAbsPath;
	return PATHERROR_NONE;
}

//��΃p�X�̎擾
bool UtilGetAbsPathName(CString &_FullPath,LPCTSTR lpszFileName)
{
	ASSERT(lpszFileName&&_tcslen(lpszFileName)>0);
	if(!lpszFileName||_tcslen(lpszFileName)<=0){
		TRACE(_T("�t�@�C�������w�肳��Ă��Ȃ�\n"));
		return false;
	}

	//---��΃p�X�擾
	TCHAR szAbsPath[_MAX_PATH+1]={0};
	{
		TCHAR Buffer[_MAX_PATH+1]={0};	//���[�g���ǂ����̃`�F�b�N���s��
		_tcsncpy_s(Buffer,lpszFileName,_MAX_PATH);
		PathAddBackslash(Buffer);
		if(PathIsRoot(Buffer)){
			//�h���C�u���������w�肳��Ă���ꍇ�A
			//_tfullpath�͂��̃h���C�u�̃J�����g�f�B���N�g�����擾���Ă��܂�
			_tcsncpy_s(szAbsPath,lpszFileName,_MAX_PATH);
		}
		else if(!_tfullpath(szAbsPath,lpszFileName,_MAX_PATH)){
			TRACE(_T("��΃p�X�擾���s\n"));
			return false;
		}
	}

	_FullPath=szAbsPath;
	return true;
}

//���C���h�J�[�h�̓W�J
bool UtilPathExpandWild(std::list<CString> &r_outList,const std::list<CString> &r_inList)
{
	std::list<CString> tempList;
	std::list<CString>::const_iterator ite=r_inList.begin();
	const std::list<CString>::const_iterator end=r_inList.end();
	for(;ite!=end;++ite){
		if(-1==(*ite).FindOneOf(_T("*?"))){	//���C���h�W�J�\�ȕ����͂Ȃ�
			tempList.push_back(*ite);
		}else{
			//���C���h�W�J
			CFindFile cFindFile;
			BOOL bContinue=cFindFile.FindFile(*ite);
			while(bContinue){
				if(!cFindFile.IsDots()){
					tempList.push_back(cFindFile.GetFilePath());
				}
				bContinue=cFindFile.FindNextFile();
			}
		}
	}
	r_outList=tempList;
	return true;
}


bool UtilPathExpandWild(std::list<CString> &r_outList,const CString &r_inParam)
{
	std::list<CString> tempList;
	if(-1==r_inParam.FindOneOf(_T("*?"))){	//���C���h�W�J�\�ȕ����͂Ȃ�
		tempList.push_back(r_inParam);
	}else{
		//���C���h�W�J
		CFindFile cFindFile;
		BOOL bContinue=cFindFile.FindFile(r_inParam);
		while(bContinue){
			if(!cFindFile.IsDots()){
				tempList.push_back(cFindFile.GetFilePath());
			}
			bContinue=cFindFile.FindNextFile();
		}
	}
	r_outList=tempList;
	return true;
}

//�p�X�̃f�B���N�g���������������o��
void UtilPathGetDirectoryPart(CString &str)
{
	LPTSTR lpszBuf=str.GetBuffer(_MAX_PATH+1);
	PathRemoveFileSpec(lpszBuf);
	PathAddBackslash(lpszBuf);
	str.ReleaseBuffer();
}

//�����̃v���O�����̃t�@�C������Ԃ�
LPCTSTR UtilGetModulePath()
{
	static TCHAR s_szExePath[_MAX_PATH+1]={0};
	if(s_szExePath[0]!=_T('\0'))return s_szExePath;

	GetModuleFileName(GetModuleHandle(NULL), s_szExePath, _MAX_PATH);	//�{�̂̃p�X�擾
	return s_szExePath;
}

//�����̃v���O�����̂����Ă���f�B���N�g���̃p�X����Ԃ�
LPCTSTR UtilGetModuleDirectoryPath()
{
	static TCHAR s_szDirPath[_MAX_PATH+1]={0};
	if(s_szDirPath[0]!=_T('\0'))return s_szDirPath;

	GetModuleFileName(GetModuleHandle(NULL), s_szDirPath, _MAX_PATH);	//�{�̂̃p�X�擾
	PathRemoveFileSpec(s_szDirPath);
	return s_szDirPath;
}

//�����K�w�̃f�B���N�g������C�ɍ쐬����
BOOL UtilMakeSureDirectoryPathExists(LPCTSTR _lpszPath)
{
#if defined(_UNICODE)||defined(UNICODE)
	CPath path(_lpszPath);
	path.RemoveFileSpec();
	path.AddBackslash();

	//TODO:UNICODE�ł݂̂Ń`�F�b�N�����Ă���̂�ANSI�r���h���ɂ͓K�X�����������ׂ�
	CString tmp(path);
	if(-1!=tmp.Find(_T(" \\"))||-1!=tmp.Find(_T(".\\"))){	//�p�X�Ƃ��ď����ł��Ȃ��t�@�C����������
		ASSERT(!"Unacceptable Directory Name");
		return FALSE;
	}

	//UNICODE�ł݂̂ŕK�v�ȃ`�F�b�N
	if(path.IsRoot())return TRUE;	//�h���C�u���[�g�Ȃ�쐬���Ȃ�(�ł��Ȃ�)

	int nRet=SHCreateDirectoryEx(NULL,path,NULL);
	switch(nRet){
	case ERROR_SUCCESS:
		return TRUE;
	case ERROR_ALREADY_EXISTS:
		if(path.IsDirectory())return TRUE;	//���łɃf�B���N�g��������ꍇ�����Z�[�t�Ƃ���
		else return FALSE;
	default:
		return FALSE;
	}
#else//defined(_UNICODE)||defined(UNICODE)
  #pragma comment(lib, "Dbghelp.lib")
	return MakeSureDirectoryPathExists(_lpszPath);
#endif//defined(_UNICODE)||defined(UNICODE)
}

//TCHAR�t�@�C������SJIS�t�@�C�����ɕϊ�����B�������ϊ��ł��Ȃ��ꍇ�ɂ́Afalse��Ԃ�
bool UtilPathT2A(CStringA &strA,LPCTSTR lpPath,bool bOnDisk)
{
#if defined(_UNICODE)||defined(UNICODE)
	CStringA strTemp(lpPath);
	if(strTemp==lpPath){
		//���������ϊ��ł���
		strA=strTemp;
		return true;
	}
	if(!bOnDisk){
		//�f�B�X�N��̃t�@�C���ł͂Ȃ��̂ŁA�ȍ~�̑�֎�i�͎���Ȃ�
		return false;
	}
	//�V���[�g�t�@�C�����ő�p���Ă݂�
	WCHAR szPath[_MAX_PATH+1];
	GetShortPathNameW(lpPath,szPath,_MAX_PATH+1);

	//��������SJIS�ɕϊ��ł��Ă��邩�ǂ����`�F�b�N����
	return UtilPathT2A(strA,szPath,false);

#else//defined(_UNICODE)||defined(UNICODE)
	//SJIS���̂܂�
	strA=lpPath;
	return true;
#endif//defined(_UNICODE)||defined(UNICODE)
}

//�p�X�ɋ��ʂ��镔�������o���A���p�X�����o��
void UtilGetBaseDirectory(CString &BasePath,const std::list<CString> &PathList)
{
	bool bFirst=true;
	size_t ElementsCount=0;	//���ʍ��ڐ�
	std::vector<CString> PathElements;	//���ʍ��ڂ̔z��

	std::list<CString>::const_iterator ite,end;
	end=PathList.end();
	for(ite=PathList.begin();ite!=end;++ite){
		if(!bFirst&&0==ElementsCount){
			//���ɋ��ʂ��Ă���v�f���z����ɑ��݂��Ȃ��Ƃ�
			break;
		}

		//�e�f�B���N�g���܂łŏI���p�X�����
		TCHAR Path[_MAX_PATH+1];
		FILL_ZERO(Path);
		_tcsncpy_s(Path,*ite,_MAX_PATH);
		PathRemoveFileSpec(Path);
		PathAddBackslash(Path);

		CString buffer;
		size_t iElement=0;	//��v���Ă���p�X�̗v�f�̃J�E���g�p(���[�v��)
		size_t iIndex=0;	//�p�X���̕����̃C���f�b�N�X
		const size_t Length=_tcslen(Path);
		for(;iIndex<Length;iIndex++){
#if !defined(_UNICODE)&&!defined(UNICODE)
			if(_MBC_SINGLE==_mbsbtype((const unsigned char *)Path,iIndex)){
#endif
				if(_T('\\')==Path[iIndex]){
					//����Ȃ�p�X���l�ߍ��݁A�����łȂ���Ηv�f���r
					if(bFirst){
						PathElements.push_back(buffer);
						buffer.Empty();
						continue;
					}
					else{
						//�啶����������ʂ����ɔ�r
						if(0==PathElements[iElement].CompareNoCase(buffer)){
							//�v�f�����ʂ��Ă��邤����OK
							iElement++;
							if(iElement>=ElementsCount)break;
							else{
								buffer.Empty();
								continue;
							}
						}
						else
						{
							//�v�f�������炷
							//0�I���W����i�Ԗڂŕs��v�Ȃ��i�܂ň�v
							ElementsCount=iElement;
							break;
						}
					}
				}
#if !defined(_UNICODE)&&!defined(UNICODE)
			}
#endif
			buffer+=Path[iIndex];
		}
		if(bFirst){
			bFirst=false;
			ElementsCount=PathElements.size();
		}
		else if(ElementsCount>iElement){
			//�p�X���Z�������ꍇ�A�s��v�Ȃ��̂܂܏������I������ꍇ������
			ElementsCount=iElement;
		}
	}

	BasePath.Empty();
	for(size_t i=0;i<ElementsCount;i++){
		BasePath+=PathElements[i];
		BasePath+=_T("\\");
	}
	TRACE(_T("BasePath=%s\n"),BasePath);
}

const LPCTSTR c_InvalidPathChar=_T("\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1a\x1b\x1c\x1d\x1e\x1f\"<>|:*?\\/\b\t");

//�t�@�C�����Ɏg���Ȃ��������u��������
void UtilFixFileName(CString &rStr,LPCTSTR lpszOrg,TCHAR replace)
{
	rStr=lpszOrg;
	int length=_tcslen(c_InvalidPathChar);
	for(int i=0;i<length;i++){
		rStr.Replace(c_InvalidPathChar[i],replace);
	}
}


LPCTSTR UtilPathNextSeparator(LPCTSTR lpStr)
{
	for(;*lpStr!=_T('\0');lpStr++){
		if(*lpStr==_T('/') || *lpStr==_T('\\')){
			break;
		}
	}
	//�I�[
	return lpStr;
}

bool UtilPathNextSection(LPCTSTR lpStart,LPCTSTR& r_lpStart,LPCTSTR& r_lpEnd,bool bSkipMeaningless)
{
	LPCTSTR lpEnd=UtilPathNextSeparator(lpStart);
	if(bSkipMeaningless){
		while(true){
			//---�����ȃp�X���ǂ����`�F�b�N
			//2�����ȏ�̃p�X�͗L���ł���ƌ��Ȃ�

			int length=lpEnd-lpStart;
			if(length==1){
				if(_T('.')==*lpStart || _T('\\')==*lpStart || _T('/')==*lpStart){
					//�����ȃp�X
					//���̗v�f������Ă���
					lpStart=lpEnd;
					lpEnd=UtilPathNextSeparator(lpStart);
				}else{
					break;
				}
			}else if(length==0){
				if(_T('\0')==*lpEnd){	//�����p�X�̗v�f���Ȃ��Ȃ����̂ŕԂ�
					return false;
				}else{
					lpEnd++;
					//���̗v�f������Ă���
					lpStart=lpEnd;
					lpEnd=UtilPathNextSeparator(lpStart);
				}
			}else{
				break;
			}
		}
	}
	r_lpStart=lpStart;
	r_lpEnd=lpEnd;
	return true;
}

//Path��'/'��������'\\'�ŏI����Ă���Ȃ�true
bool UtilPathEndWithSeparator(LPCTSTR lpPath)
{
	UINT length=_tcslen(lpPath);
	TCHAR c=lpPath[length-1];
	return (_T('/')==c || _T('\\')==c);
}

//�p�X���̍Ō�̕��������o��
void UtilPathGetLastSection(CString &strSection,LPCTSTR lpPath)
{
	CString strPath=lpPath;
	strPath.Replace(_T('\\'),_T('/'));
	while(true){
		int idx=strPath.ReverseFind(_T('/'));
		if(-1==idx){	//���̂܂�
			strSection=lpPath;
			return;
		}else if(idx<strPath.GetLength()-1){
			//������Separator�ł͂Ȃ�
			strSection=lpPath+idx+1;
			return;
		}else{	//������Separator
			//���������->�������̓��[�v�ōēx����;strSection�ɂ�Separator�t���̕������i�[�����
			strPath.Delete(idx,strPath.GetLength());
		}
	}
}



//�t�@�C�����ۂ��ƁA�������͎w�肳�ꂽ�Ƃ���܂œǂݍ���(-1�Ŋۂ���)
bool UtilReadFile(LPCTSTR lpFile,std::vector<BYTE> &cReadBuffer,DWORD dwLimit)
{
	ASSERT(lpFile);
	if(!lpFile)return false;
	if(!PathFileExists(lpFile))return false;

	//Open
	HANDLE hFile=CreateFile(lpFile,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
	if(INVALID_HANDLE_VALUE==hFile)return false;

	//4GB�z���t�@�C���͈���Ȃ��̂Ńt�@�C���T�C�Y�擾�͂���ł悢
	DWORD dwSize=GetFileSize(hFile,NULL);

	//�ǂݍ��ݔ͈�
	if(-1!=dwLimit){	//�͈͐�������Ă���
		dwSize=min(dwSize,dwLimit);
	}

	cReadBuffer.resize(dwSize);
	DWORD dwRead,dwIndex=0;
	//---�ǂݍ���
	while(true){
		const DWORD BLOCKSIZE=1024*10;	//10KB���Ƃɓǂݍ���
		DWORD readsize=BLOCKSIZE;
		if(dwIndex+readsize>dwSize){
			readsize=dwSize-dwIndex;
		}
		if(!ReadFile(hFile,&cReadBuffer[dwIndex],readsize,&dwRead,NULL)){
			CloseHandle(hFile);
			return false;
		}
		dwIndex+=dwRead;
		if(readsize<BLOCKSIZE)break;	//�ǂݐ؂���
	}
	CloseHandle(hFile);

	if(dwSize!=dwIndex){
		return false;
	}

	return true;
}

bool UtilReadFileSplitted(LPCTSTR lpFile,FILELINECONTAINER &container)
{
	//�s�o�b�t�@���N���A
	container.data.clear();
	container.lines.clear();

	//---�ǂݍ���
	std::vector<BYTE> cReadBuffer;
	if(!UtilReadFile(lpFile,cReadBuffer))return false;
	//�I�[��0�ǉ�
	cReadBuffer.resize(cReadBuffer.size()+2);
	cReadBuffer[cReadBuffer.size()-1];
	cReadBuffer[cReadBuffer.size()-2];

	{
		CStringW strData;
		UtilGuessToUNICODE(strData,&cReadBuffer[0],cReadBuffer.size());
		container.data.assign((LPCWSTR)strData,(LPCWSTR)strData+strData.GetLength());
		container.data.push_back(L'\0');
	}


	LPWSTR p=&container.data[0];
	const LPCWSTR end=p+container.data.size();
	LPCWSTR lastHead=p;

	//����
	for(;p!=end && *p!=L'\0';p++){
		if(*p==_T('\n')||*p==_T('\r')){
			if(lastHead<p){	//��s�͔�΂�
				container.lines.push_back(lastHead);
			}
			lastHead=p+1;
			*p=L'\0';
		}
	}
	return true;
}
