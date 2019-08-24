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
#include "ArchiverUNLHA.h"
#include "../Utilities/FileOperation.h"
#include "../resource.h"
#include "../ConfigCode/ConfigExtract.h"
#include "../ConfigCode/ConfigUNLHA.h"
#include "../Utilities/OSUtil.h"

CArchiverUNLHA::CArchiverUNLHA():
	ArchiveHandlerW(NULL),
	ArchiverCheckArchiveW(NULL),
	ArchiverGetFileNameW(NULL),
	ArchiverOpenArchiveW(NULL),
	ArchiverFindFirstW(NULL),
	ArchiverFindNextW(NULL),
	ArchiverGetFileCountW(NULL),
	ArchiverGetMethodW(NULL)
{
	m_dwInspectMode=0x00000002L;	//M_REGARDLESS_INIT_FILE:���W�X�g���̒l����
	m_nRequiredVersion=262;
	m_nRequiredSubVersion=227;
	m_strDllName=_T("UNLHA32.DLL");
	m_AstrPrefix="Unlha";
	m_AstrFindParam="*.*";
}

CArchiverUNLHA::~CArchiverUNLHA()
{
	FreeDLL();
}

void CArchiverUNLHA::FormatCompressCommand(const CConfigLZH& Config,LPCTSTR lpszMethod,CString &Param)
{
	int nType=Config.CompressType;

	if(lpszMethod && *lpszMethod!=_T('\0')){
			 if(0==_tcsicmp(lpszMethod,_T("lh0")))nType=LZH_COMPRESS_LH0;
		else if(0==_tcsicmp(lpszMethod,_T("lh1")))nType=LZH_COMPRESS_LH1;
		else if(0==_tcsicmp(lpszMethod,_T("lh5")))nType=LZH_COMPRESS_LH5;
		else if(0==_tcsicmp(lpszMethod,_T("lh6")))nType=LZH_COMPRESS_LH6;
		else if(0==_tcsicmp(lpszMethod,_T("lh7")))nType=LZH_COMPRESS_LH7;
	}

	switch(nType){
	case LZH_COMPRESS_LH5:
		Param+=_T("-jm2 ");
		break;
	case LZH_COMPRESS_LH0:
		Param+=_T("-jm0 ");
		break;
	case LZH_COMPRESS_LH1:
		Param+=_T("-jm1 ");
		break;
	case LZH_COMPRESS_LH6:
		Param+=_T("-jm3 ");
		break;
	case LZH_COMPRESS_LH7:
		Param+=_T("-jm4 ");
		break;
	}
}

bool CArchiverUNLHA::Compress(LPCTSTR ArcFileName,std::list<CString> &ParamList,CConfigManager &ConfMan,const PARAMETER_TYPE,int Options,LPCTSTR,LPCTSTR lpszMethod,LPCTSTR,CString &strLog)
{
	if(!IsOK()){
		return false;
	}
	bool bSFX=(0!=(Options&COMPRESS_SFX));	//���ȉ𓀍쐬�Ȃ�true

	ASSERT(0!=_tcslen(ArcFileName));
	TRACE(_T("ArcFileName=%s\n"),ArcFileName);

	//==============================================
	// ���X�|���X�t�@�C���p�e���|�����t�@�C�����擾
	//==============================================
	TCHAR ResponceFileName[_MAX_PATH+1];
	FILL_ZERO(ResponceFileName);
	if(!UtilGetTemporaryFileName(ResponceFileName,_T("lzh"))){
		strLog=CString(MAKEINTRESOURCE(IDS_ERROR_TEMPORARY_FILE_CREATE));
		return false;
	}
	ASSERT(0!=_tcslen(ResponceFileName));

	//==========================================
	// ���X�|���X�t�@�C�����ɃA�[�J�C�u�������
	// ���k�Ώۃt�@�C�������L������
	//==========================================
	{
		HANDLE hFile=CreateFile(ResponceFileName,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
		if(INVALID_HANDLE_VALUE==hFile)
		{
			strLog=CString(MAKEINTRESOURCE(IDS_ERROR_TEMPORARY_FILE_ACCESS));
			return false;
		}

		TRACE(_T("���X�|���X�t�@�C���ւ̏�������\n"));
		//���k��t�@�C������������
		WriteResponceFile(hFile,ArcFileName);

		std::list<CString>::iterator ite;
		for(ite=ParamList.begin();ite!=ParamList.end();++ite){
			WriteResponceFile(hFile,*ite);
		}
		CloseHandle(hFile);
	}

	//===========================
	// DLL�ɓn���I�v�V�����̐ݒ�
	//===========================
	TRACE(_T("DLL�ɓn���I�v�V�����̐ݒ�\n"));

	CString Param;//�R�}���h���C�� �p�����[�^ �o�b�t�@

	//���k�p�����[�^
	Param+=
		_T("a ")		//���k
		_T("-d1 ")		//�t�H���_���ƁE�S�����t�@�C����Ώ�
		_T("-+1 ")		//UNLHA32.DLL�̃��W�X�g���̐ݒ�𖳎�
		_T("-jf0 ")		//���[�g�L�����폜
		_T("-jso1 ")	//SH_DENYNO �ł̃I�[�v�����s��Ȃ�
		_T("-jsm1 ")	//�T�E���h���g��
	;
	//��ƃf�B���N�g��
	Param+=_T("\"-w");
	Param+=UtilGetTempPath();
	Param+=_T("\" ");

	CConfigLZH Config;
	Config.load(ConfMan);
	FormatCompressCommand(Config,lpszMethod,Param);
	if(bSFX){
		//���ȉ𓀍쐬
		if(Config.ConfigSFX){
			Param+=_T("-gw3 ");	//�ݒ�t��WinSFX32M
		}
		else{
			Param+=_T("-gw4 ");	//�ݒ�Ȃ�WinSFX32M
		}
	}

	//���X�|���X�t�@�C�����w��
	Param+=_T("@\"");
	Param+=ResponceFileName;
	Param+=_T("\"");

	ASSERT(!Param.IsEmpty());
	TRACE(_T("ArchiveHandler Commandline Parameter:%s\n"),Param);

	TRACE(_T("ArchiveHandler�Ăяo��\n"));
	//char szLog[LOG_BUFFER_SIZE]={0};
	std::vector<WCHAR> szLog(LOG_BUFFER_SIZE);
	szLog[0]=L'\0';
	int Ret=ArchiveHandlerW(NULL,Param,&szLog[0],LOG_BUFFER_SIZE-1);
	strLog=&szLog[0];

	//�g�������X�|���X�t�@�C���͏���
	DeleteFile(ResponceFileName);

	return 0==Ret;
}

//bSafeArchive�͖��������
bool CArchiverUNLHA::Extract(LPCTSTR ArcFileName,CConfigManager&,const CConfigExtract &Config,bool,LPCTSTR OutputDir,CString &strLog)
{
	if(!IsOK()){
		return false;
	}

	//===========================
	// DLL�ɓn���I�v�V�����̐ݒ�
	//===========================
	TRACE(_T("DLL�ɓn���I�v�V�����̐ݒ�\n"));

	//�o�͐�ړ�
	CCurrentDirManager currentDir(OutputDir);
	CString Param;//�R�}���h���C�� �p�����[�^ �o�b�t�@

	//�𓀃p�����[�^
	Param+=
		_T("e ")			//��
		_T("-x1 ")			//�p�X��L����
		_T("-jyc ")			//�t�H���_�쐬�̊m�F�L�����Z��
		_T("-a1 ")			//�t�@�C���̑�����W�J
		_T("-c1 ")			//�^�C���X�^���v�������s��
		_T("-+1 ")			//UNLHA32.DLL�̃��W�X�g���̐ݒ�𖳎�
		_T("-jf0 ")			//���[�g�L�����폜
		_T("-jso1 ")		//SH_DENYNO �ł̃I�[�v�����s��Ȃ�
		_T("-jsm1 ")		//�T�E���h���g��
		_T("-jsp23 ")		//�s���ȃp�X/���䕶��������
		_T("-jsc ")			//�W�J�ł��Ȃ������t�@�C���̐���Ԃ�
	;
	//��ƃf�B���N�g��
	Param+=_T("\"-w");
	Param+=UtilGetTempPath();
	Param+=_T("\" ");
	if(Config.ForceOverwrite){
		//�����㏑��
		Param+=_T("-jyo ");
	}

	//�A�[�J�C�u�t�@�C�����w��
	Param+=_T("\"");
	Param+=ArcFileName;
	Param+=_T("\" ");

	//�o�͐�w��
	Param+=_T("\"");
	Param+=_T(".\\");//OutputDir;
	Param+=_T("\"");

	ASSERT(!Param.IsEmpty());
	TRACE(_T("ArchiveHandler Commandline Parameter:%s\n"),Param);

	TRACE(_T("ArchiveHandler�Ăяo��\n"));
	//char szLog[LOG_BUFFER_SIZE]={0};
	std::vector<WCHAR> szLog(LOG_BUFFER_SIZE);
	szLog[0]=L'\0';
	int Ret=ArchiveHandlerW(NULL,Param,&szLog[0],LOG_BUFFER_SIZE-1);
	strLog=&szLog[0];

	return 0==Ret;
}

//���X�|���X�t�@�C���Ƀt�@�C�������G�X�P�[�v�����s������ŏ������ށB
//�L���ȃt�@�C���n���h����NULL�łȂ��t�@�C������n�����ƁB
void CArchiverUNLHA::WriteResponceFile(HANDLE hFile,LPCTSTR fname)
{
	CPath strBuffer;

	//�t�@�C�����̐擪��'-'�Ȃ�-gb(�t�@�C����)�Ƃ���
#if defined(_UNICODE)||defined(UNICODE)
	if(_T('-')==fname[0]){
#else
#error("not implemented or tested")
	if(_MBC_SINGLE==_mbsbtype((const unsigned char*)fname,0)&&_T('-')==fname[0]){
#endif//defined(_UNICODE)||defined(UNICODE)
		strBuffer=_T("-gb");
		strBuffer+=fname;
	}
	else{
		strBuffer=fname;
	}
	strBuffer.QuoteSpaces();

	DWORD dwWritten=0;
	//�t�@�C�������o��
	WriteFile(hFile,(LPCBYTE)(LPCTSTR)strBuffer,_tcslen(strBuffer)*sizeof(TCHAR),&dwWritten,NULL);

	//�t�@�C��������؂邽�߂̉��s
	TCHAR CRLF[]=_T("\r\n");
	WriteFile(hFile,&CRLF,(DWORD)_tcslen(CRLF)*sizeof(TCHAR),&dwWritten,NULL);
}


//=========================================================
// UnlhaGetFileName()�̏o�͌��ʂ���ɁA�i�[���ꂽ�t�@�C�����p�X
// ���������Ă��邩�ǂ������ʂ��A��d�t�H���_�쐬��h��
//=========================================================
bool CArchiverUNLHA::ExamineArchive(LPCTSTR ArcFileName,CConfigManager& ConfMan,bool bSkipDir,bool &bInFolder,bool &,CString &BaseDir,CString &strErr)
{
	//"-jsp" �X�C�b�`���g�����C Unicode ���䕶����s���Ƃ��Ĉ�����悤�ɂ����B
	if(bSkipDir){
		TRACE(_T("��d�t�H���_������\n"));
		return true;
	}
	return _ExamineArchiveFast(ArcFileName,ConfMan,bInFolder,BaseDir,strErr);
}

bool CArchiverUNLHA::ExtractSpecifiedOnly(LPCTSTR ArcFileName,CConfigManager&,LPCTSTR OutputDir,std::list<CString> &FileList,CString &strLog,bool bUsePath)
{
	if(!IsOK()){
		return false;
	}
	//�o�͐�ړ�
	CCurrentDirManager currentDir(OutputDir);

	//==============================================
	// ���X�|���X�t�@�C���p�e���|�����t�@�C�����擾
	//==============================================
	TCHAR ResponceFileName[_MAX_PATH+1];
	FILL_ZERO(ResponceFileName);
	if(!UtilGetTemporaryFileName(ResponceFileName,_T("lzh"))){
		strLog=CString(MAKEINTRESOURCE(IDS_ERROR_TEMPORARY_FILE_CREATE));
		return false;
	}
	ASSERT(0!=_tcslen(ResponceFileName));

	//�𓀑Ώۃt�@�C�������X�|���X�t�@�C���ɏ����o��
	{
		HANDLE hFile=CreateFile(ResponceFileName,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
		if(INVALID_HANDLE_VALUE==hFile){
			strLog=CString(MAKEINTRESOURCE(IDS_ERROR_TEMPORARY_FILE_ACCESS));
			return false;
		}

		std::list<CString>::iterator ite=FileList.begin();
		const std::list<CString>::iterator end=FileList.end();
		for(;ite!=end;++ite){
			WriteResponceFile(hFile,*ite);
		}
		CloseHandle(hFile);
	}

	//===========================
	// DLL�ɓn���I�v�V�����̐ݒ�
	//===========================
	CString Param;

	//�𓀃p�����[�^
	Param+=
		_T("e ")			//��
		_T("-p1 ")			//�S�p�X���ō��v
		_T("-jyc ")			//�t�H���_�쐬�̊m�F�L�����Z��
		_T("-a2 ")			//�t�@�C���̑�����W�J
		_T("-c1 ")			//�^�C���X�^���v�������s��
		_T("-+1 ")			//UNLHA32.DLL�̃��W�X�g���̐ݒ�𖳎�
		_T("-jf0 ")			//���[�g�L�����폜
		_T("-jso1 ")		//SH_DENYNO �ł̃I�[�v�����s��Ȃ�
		_T("-jsm1 ")		//�T�E���h���g��
		_T("-jsp23 ")		//�s���ȃp�X/���䕶��������
		_T("-jsc ")			//�W�J�ł��Ȃ������t�@�C���̐���Ԃ�
	;
	//��ƃf�B���N�g��
	Param+=_T("\"-w");
	Param+=UtilGetTempPath();
	Param+=_T("\" ");
	if(bUsePath){
		Param+=_T("-x1 ");			//�p�X��L����
	}
	else{
		Param+=_T("-x0 ");			//�p�X�𖳌���
	}
	//�A�[�J�C�u�t�@�C�����w��
	Param+=_T("\"");
	Param+=ArcFileName;
	Param+=_T("\" ");

	//�o�͐�t�H���_
	Param+=_T("\"");
	Param+=OutputDir;
	Param+=_T("\" ");

	//���X�|���X�t�@�C�����w��
	Param+=_T("@\"");
	Param+=ResponceFileName;
	Param+=_T("\"");

	TRACE(_T("ArchiveHandler�Ăяo��\nCommandline Parameter:%s\n"),Param);
	std::vector<WCHAR> szLog(LOG_BUFFER_SIZE);
	szLog[0]=L'\0';
	int Ret=ArchiveHandlerW(NULL,Param,&szLog[0],LOG_BUFFER_SIZE-1);
	strLog=&szLog[0];
	//�g�������X�|���X�t�@�C���͏���
	DeleteFile(ResponceFileName);

	return 0==Ret;
}

bool CArchiverUNLHA::DeleteItemFromArchive(LPCTSTR ArcFileName,CConfigManager&,const std::list<CString>& FileList,CString &strLog)
{
	if(!IsOK()){
		return false;
	}

	//==============================================
	// ���X�|���X�t�@�C���p�e���|�����t�@�C�����擾
	//==============================================
	TCHAR ResponceFileName[_MAX_PATH+1];
	FILL_ZERO(ResponceFileName);
	if(!UtilGetTemporaryFileName(ResponceFileName,_T("lzh"))){
		strLog=CString(MAKEINTRESOURCE(IDS_ERROR_TEMPORARY_FILE_CREATE));
		return false;
	}
	ASSERT(0!=_tcslen(ResponceFileName));

	//�폜�Ώۃt�@�C�������X�|���X�t�@�C���ɏ����o��
	{
		HANDLE hFile=CreateFile(ResponceFileName,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
		if(INVALID_HANDLE_VALUE==hFile)
		{
			strLog=CString(MAKEINTRESOURCE(IDS_ERROR_TEMPORARY_FILE_ACCESS));
			return false;
		}

		std::list<CString>::const_iterator ite=FileList.begin();
		const std::list<CString>::const_iterator end=FileList.end();
		for(;ite!=end;++ite){
			CString tmp=*ite;
			//�f�B���N�g����������/����菜��
#if defined(_UNICODE)||defined(UNICODE)
			if(_T('/')==tmp[tmp.GetLength()-1]){
				tmp.Delete(tmp.GetLength()-1);
			}
#else
			if(_MBC_SINGLE==_mbsbtype((const unsigned char*)(LPCTSTR)*ite,(*ite).GetLength()-1)){
				if(_T('/')==tmp[tmp.GetLength()-1]){
					tmp.Delete(tmp.GetLength()-1);
				}
			}
#endif//defined(_UNICODE)||defined(UNICODE)
			WriteResponceFile(hFile,tmp);
		}
		CloseHandle(hFile);
	}

	//===========================
	// DLL�ɓn���I�v�V�����̐ݒ�
	//===========================
	CString Param;

	//�폜�p�����[�^
	Param+=
		_T("d ")			//�폜
		_T("-r0 ")			//��ċA���[�h(�f�t�H���g)
		_T("-p1 ")			//�S�p�X���ō��v
		_T("-+1 ")			//UNLHA32.DLL�̃��W�X�g���̐ݒ�𖳎�
		_T("-jso1 ")		//SH_DENYNO �ł̃I�[�v�����s��Ȃ�
		_T("-jsm1 ")		//�T�E���h���g��
		_T("-d ")			//�f�B���N�g���̊i�[
	;
	//��ƃf�B���N�g��
	Param+=_T("\"-w");
	Param+=UtilGetTempPath();
	Param+=_T("\" ");
	//�A�[�J�C�u�t�@�C�����w��
	Param+=_T("\"");
	Param+=ArcFileName;
	Param+=_T("\" ");

	//���X�|���X�t�@�C�����w��
	Param+=_T("@\"");
	Param+=ResponceFileName;
	Param+=_T("\"");

	TRACE(_T("ArchiveHandler�Ăяo��\nCommandline Parameter:%s\n"),Param);
	//char szLog[LOG_BUFFER_SIZE]={0};
	std::vector<WCHAR> szLog(LOG_BUFFER_SIZE);
	szLog[0]=L'\0';
	int Ret=ArchiveHandlerW(NULL,Param,&szLog[0],LOG_BUFFER_SIZE-1);
	strLog=&szLog[0];
	//�g�������X�|���X�t�@�C���͏���
	DeleteFile(ResponceFileName);

	return 0==Ret;
}

ARCRESULT CArchiverUNLHA::TestArchive(LPCTSTR ArcFileName,CString &strLog)
{
	TRACE(_T("TestArchive() called.\n"));
	ASSERT(IsOK());
	if(!IsOK()){
		return TEST_ERROR;
	}

	//t�R�}���h�ɂ��e�X�g����������Ă���
	CString Param=
		_T("t ")			//�e�X�g
		_T("-+1 ")			//UNLHA32.DLL�̃��W�X�g���̐ݒ�𖳎�
		_T("-jso1 ")		//SH_DENYNO �ł̃I�[�v�����s��Ȃ�
		_T("-jsc ")			//�W�J�ł��Ȃ������t�@�C���̐���Ԃ�
//		_T("-jsp23 ")		//�s���ȃp�X/���䕶��������->�G���[�ƂȂ�Ȃ�

		_T("\"")
	;
	Param+=ArcFileName;
	Param+=_T("\" ");
	//��ƃf�B���N�g��
	Param+=_T("\"-w");
	Param+=UtilGetTempPath();
	Param+=_T("\" ");

	//char szLog[LOG_BUFFER_SIZE]={0};
	std::vector<WCHAR> szLog(LOG_BUFFER_SIZE);
	szLog[0]=L'\0';
	int Ret=ArchiveHandlerW(NULL,Param,&szLog[0],LOG_BUFFER_SIZE-1);
	strLog=&szLog[0];

	if(Ret==0)return TEST_OK;
	else return TEST_NG;
}

BOOL CArchiverUNLHA::CheckArchive(LPCTSTR _szFileName)
{
	if(!ArchiverCheckArchiveW){
		ASSERT(ArchiverCheckArchiveW);
		return false;
	}
	return ArchiverCheckArchiveW(_szFileName,CHECKARCHIVE_BASIC);
}

int CArchiverUNLHA::GetFileCount(LPCTSTR _szFileName)
{
	if(!ArchiverGetFileCountW){
		return -1;
	}
	return ArchiverGetFileCountW(_szFileName);
}

void CArchiverUNLHA::FreeDLL()
{
	if(m_hInstDLL){
		ArchiveHandlerW				=NULL;
		ArchiverCheckArchiveW		=NULL;
		ArchiverGetFileCountW		=NULL;
		ArchiverGetFileNameW		=NULL;
		ArchiverOpenArchiveW		=NULL;
		ArchiverFindFirstW			=NULL;
		ArchiverFindNextW			=NULL;
		ArchiverGetMethodW			=NULL;
		CArchiverDLL::FreeDLL();
	}
}

LOAD_RESULT CArchiverUNLHA::LoadDLL(CConfigManager &ConfigManager,CString &strErr)
{
	FreeDLL();

	//���N���X�̃��\�b�h���Ă�
	LOAD_RESULT res=CArchiverDLL::LoadDLL(ConfigManager,strErr);
	if(LOAD_RESULT_OK!=res){
		return res;
	}
	//ANSI�ł͊Ԉ���Ďg�����Ƃ̂Ȃ��悤�Ƀ��Z�b�g����
	ArchiveHandler				=NULL;
	ArchiverCheckArchive		=NULL;
	ArchiverGetFileCount		=NULL;
	ArchiverGetFileName			=NULL;
	ArchiverOpenArchive			=NULL;
	ArchiverFindFirst			=NULL;
	ArchiverFindNext			=NULL;
	ArchiverGetMethod			=NULL;

	ASSERT(ArchiverGetWriteTimeEx);
	if(!ArchiverGetWriteTimeEx){	//UNLHA32.DLL�ł͎�������Ă���͂�
		return LOAD_RESULT_INVALID;
	}

	//---UNICODE�ł����[�h������;�\�̓`�F�b�N�͊��ɍς�ł���̂ōs��Ȃ�

	ArchiveHandlerW=(COMMON_ARCHIVER_HANDLERW)GetProcAddress(m_hInstDLL,m_AstrPrefix+"W");
	if(NULL==ArchiveHandlerW){
		strErr.Format(IDS_ERROR_DLL_FUNCTION_GET,m_strDllName,m_AstrPrefix+"W");
		FreeDLL();
		return LOAD_RESULT_INVALID;
	}

	CStringA FunctionName=m_AstrPrefix;
	FunctionName+="CheckArchiveW";
	ArchiverCheckArchiveW=(COMMON_ARCHIVER_CHECKARCHIVEW)GetProcAddress(m_hInstDLL,FunctionName);
	if(NULL==ArchiverCheckArchiveW){
		strErr.Format(IDS_ERROR_DLL_FUNCTION_GET,m_strDllName,FunctionName);
		FreeDLL();
		return LOAD_RESULT_INVALID;
	}

	//XacRett.dll��Aish32.dll�������T�|�[�g���Ă��Ȃ�
	FunctionName=m_AstrPrefix;
	FunctionName+="GetFileCountW";
	ArchiverGetFileCountW=(COMMON_ARCHIVER_GETFILECOUNTW)GetProcAddress(m_hInstDLL,FunctionName);
	if(NULL==ArchiverGetFileCountW){
	//���[�h�Ɏ��s���Ă��G���[�Ƃ͂��Ȃ�
		TRACE(_T("Failed to Load Function %s\n"),FunctionName);
	}

	//------------
	// �ǉ��֐��Q
	//------------
	FunctionName=m_AstrPrefix;
	FunctionName+="OpenArchiveW";
	ArchiverOpenArchiveW=(COMMON_ARCHIVER_OPENARCHIVEW)GetProcAddress(m_hInstDLL,FunctionName);
	if(NULL==ArchiverOpenArchiveW){
		strErr.Format(IDS_ERROR_DLL_FUNCTION_GET,m_strDllName,FunctionName);
		FreeDLL();
		return LOAD_RESULT_INVALID;
	}

	//FindFirst
	FunctionName=m_AstrPrefix;
	FunctionName+="FindFirstW";
	ArchiverFindFirstW=(COMMON_ARCHIVER_FINDFIRSTW)GetProcAddress(m_hInstDLL,FunctionName);
	if(NULL==ArchiverFindFirstW){
		strErr.Format(IDS_ERROR_DLL_FUNCTION_GET,m_strDllName,FunctionName);
		FreeDLL();
		return LOAD_RESULT_INVALID;
	}

	//FindNext
	FunctionName=m_AstrPrefix;
	FunctionName+="FindNextW";
	ArchiverFindNextW=(COMMON_ARCHIVER_FINDNEXTW)GetProcAddress(m_hInstDLL,FunctionName);
	if(NULL==ArchiverFindNextW){
		strErr.Format(IDS_ERROR_DLL_FUNCTION_GET,m_strDllName,FunctionName);
		FreeDLL();
		return LOAD_RESULT_INVALID;
	}

	//�t�@�C�����擾
	FunctionName=m_AstrPrefix;
	FunctionName+="GetFileNameW";
	ArchiverGetFileNameW=(COMMON_ARCHIVER_GETFILENAMEW)GetProcAddress(m_hInstDLL,FunctionName);
	if(NULL==ArchiverGetFileNameW){
		strErr.Format(IDS_ERROR_DLL_FUNCTION_GET,m_strDllName,FunctionName);
		FreeDLL();
		return LOAD_RESULT_INVALID;
	}

	//�t�@�C�����k���\�b�h�擾
	FunctionName=m_AstrPrefix;
	FunctionName+="GetMethodW";
	ArchiverGetMethodW=(COMMON_ARCHIVER_GETMETHODW)GetProcAddress(m_hInstDLL,FunctionName);
	if(NULL==ArchiverGetMethodW){
		strErr.Format(IDS_ERROR_DLL_FUNCTION_GET,m_strDllName,FunctionName);
		FreeDLL();
		return LOAD_RESULT_INVALID;
	}


	TRACE(_T("Standard Function Set Load\n"));
	return LOAD_RESULT_OK;
}

bool CArchiverUNLHA::InspectArchiveBegin(LPCTSTR ArcFileName,CConfigManager&)
{
	TRACE(_T("CArchiverUNLHA::InspectArchiveBegin()\n"));
	ASSERT(ArchiverOpenArchiveW);
	if(!ArchiverOpenArchiveW){
		return false;
	}
	if(m_hInspectArchive){
		ASSERT(!"Close the Archive First!!!\n");
		return false;
	}
	m_hInspectArchive=ArchiverOpenArchiveW(NULL,ArcFileName,m_dwInspectMode);
	if(!m_hInspectArchive){
		TRACE(_T("Failed to Open Archive\n"));
		return false;
	}
	m_bInspectFirstTime=true;

	FILL_ZERO(m_IndividualInfoW);
	return true;
}

bool CArchiverUNLHA::InspectArchiveGetFileName(CString &FileName)
{
	if(!m_hInspectArchive){
		ASSERT(!"Open an Archive First!!!\n");
		return false;
	}
	if(ArchiverGetFileNameW){
		std::vector<WCHAR> szBuffer(FNAME_MAX32*2+1);
		ArchiverGetFileNameW(m_hInspectArchive,&szBuffer[0],FNAME_MAX32*2);
		FileName=&szBuffer[0];
		return true;
	}else{
		return false;
	}
}


bool CArchiverUNLHA::InspectArchiveNext()
{
	ASSERT(ArchiverFindFirstW);
	ASSERT(ArchiverFindNextW);
	if(!ArchiverFindFirstW||!ArchiverFindNextW){
		return false;
	}
	if(!m_hInspectArchive){
		ASSERT(!"Open an Archive First!!!\n");
		return false;
	}

	FILL_ZERO(m_IndividualInfoW);
	bool bRet;
	if(m_bInspectFirstTime){
		m_bInspectFirstTime=false;
		bRet=(0==ArchiverFindFirstW(m_hInspectArchive,CStringW(m_AstrFindParam),&m_IndividualInfoW));
	}
	else{
		bRet=(0==ArchiverFindNextW(m_hInspectArchive,&m_IndividualInfoW));
	}

	return bRet;
}

//���ɓ��t�@�C��CRC�擾
DWORD CArchiverUNLHA::InspectArchiveGetCRC()
{
	if(!m_hInspectArchive){
		ASSERT(!"Open an Archive First!!!\n");
		return -1;
	}
	return m_IndividualInfoW.dwCRC;
}

//���ɓ��t�@�C�����k���擾
WORD CArchiverUNLHA::InspectArchiveGetRatio()
{
	if(!m_hInspectArchive){
		ASSERT(!"Open an Archive First!!!\n");
		return -1;
	}

	return m_IndividualInfoW.wRatio;
}

//���ɓ��t�@�C���i�[���[�h�擾
bool CArchiverUNLHA::InspectArchiveGetMethodString(CString &strMethod)
{
	if(!m_hInspectArchive){
		ASSERT(!"Open an Archive First!!!\n");
		return false;
	}

	ASSERT(ArchiverGetMethodW);


	//\0�ŏI����Ă��邩�ǂ����ۏ؂ł��Ȃ�
	WCHAR szBuffer[32]={0};

	if(ArchiverGetMethodW){
		if(0!=ArchiverGetMethodW(m_hInspectArchive,szBuffer,31)){
			//�\���̂���擾
			wcsncpy_s(szBuffer,m_IndividualInfoW.szMode,8);
		}
	}
	else return false;

	//���i�[
	strMethod=szBuffer;
	return true;
}

bool CArchiverUNLHA::AddItemToArchive(LPCTSTR ArcFileName,bool bEncrypted,const std::list<CString> &FileList,CConfigManager &ConfMan,LPCTSTR lpDestDir,CString &strLog)
{
	//---�J�����g�f�B���N�g���ݒ�̂��߂�\�ŏI����_�p�X���擾
	CString strBasePath;
	UtilGetBaseDirectory(strBasePath,FileList);
	TRACE(_T("%s\n"),strBasePath);

	//�J�����g�f�B���N�g���ݒ�
	SetCurrentDirectory(strBasePath);

	//==============================================
	// ���X�|���X�t�@�C���p�e���|�����t�@�C�����擾
	//==============================================
	TCHAR ResponceFileName[_MAX_PATH+1];
	FILL_ZERO(ResponceFileName);
	if(!UtilGetTemporaryFileName(ResponceFileName,_T("lzh"))){
		strLog=CString(MAKEINTRESOURCE(IDS_ERROR_TEMPORARY_FILE_CREATE));
		return false;
	}
	ASSERT(0!=_tcslen(ResponceFileName));

	//==========================================
	// ���X�|���X�t�@�C�����ɃA�[�J�C�u�������
	// ���k�Ώۃt�@�C�������L������
	//==========================================
	{
		HANDLE hFile=CreateFile(ResponceFileName,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
		if(INVALID_HANDLE_VALUE==hFile)
		{
			strLog=CString(MAKEINTRESOURCE(IDS_ERROR_TEMPORARY_FILE_ACCESS));
			return false;
		}

		TRACE(_T("���X�|���X�t�@�C���ւ̏�������\n"));
		//���k��t�@�C������������
		WriteResponceFile(hFile,ArcFileName);

		const int BasePathLength=strBasePath.GetLength();
		std::list<CString>::const_iterator ite;
		for(ite=FileList.begin();ite!=FileList.end();++ite){
			//�x�[�X�p�X�����ɑ��΃p�X�擾 : ���ʂł�����p�X�̕������������J�b�g����
			LPCTSTR lpszPath=(LPCTSTR)(*ite)+BasePathLength;

			if(*lpszPath==_T('\0')){
				//����BasePath�Ɠ����ɂȂ��ċ�ɂȂ��Ă��܂����p�X������΁A�J�����g�f�B���N�g���w������ɓ���Ă���
				WriteResponceFile(hFile,_T("."));
			}
			else{
				WriteResponceFile(hFile,lpszPath);
			}
		}
		CloseHandle(hFile);
	}

	//===========================
	// DLL�ɓn���I�v�V�����̐ݒ�
	//===========================
	TRACE(_T("DLL�ɓn���I�v�V�����̐ݒ�\n"));

	CString Param;//�R�}���h���C�� �p�����[�^ �o�b�t�@

	//���k�p�����[�^
	Param+=
		_T("a ")		//���k
		_T("-d1 ")		//�t�H���_���ƁE�S�����t�@�C����Ώ�
		_T("-+1 ")		//UNLHA32.DLL�̃��W�X�g���̐ݒ�𖳎�
		_T("-jf0 ")		//���[�g�L�����폜
		_T("-jso1 ")	//SH_DENYNO �ł̃I�[�v�����s��Ȃ�
		_T("-jsm1 ")	//�T�E���h���g��
	;
	//��ƃf�B���N�g��
	Param+=_T("\"-w");
	Param+=UtilGetTempPath();
	Param+=_T("\" ");

	CConfigLZH Config;
	Config.load(ConfMan);
	FormatCompressCommand(Config,NULL,Param);

	//�o�͐�t�H���_�̖����̃o�b�N�X���b�V�����폜
	CPath destPath(lpDestDir);
	destPath.RemoveBackslash();
	//�ǉ����k��
	Param+=_T("-jb\"");
	Param+=(CString)destPath;
	Param+=_T("\" ");

	//���X�|���X�t�@�C�����w��
	Param+=_T("@\"");
	Param+=ResponceFileName;
	Param+=_T("\"");

	ASSERT(!Param.IsEmpty());
	TRACE(_T("ArchiveHandler Commandline Parameter:%s\n"),Param);

	TRACE(_T("ArchiveHandler�Ăяo��\n"));
	//char szLog[LOG_BUFFER_SIZE]={0};
	std::vector<WCHAR> szLog(LOG_BUFFER_SIZE);
	szLog[0]=L'\0';
	int Ret=ArchiveHandlerW(NULL,Param,&szLog[0],LOG_BUFFER_SIZE-1);
	strLog=&szLog[0];

	//�g�������X�|���X�t�@�C���͏���
	DeleteFile(ResponceFileName);

	return 0==Ret;
}
