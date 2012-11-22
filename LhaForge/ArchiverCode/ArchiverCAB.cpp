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
#include "ArchiverCAB.h"
#include "../Utilities/FileOperation.h"
#include "../resource.h"
#include "../ConfigCode/ConfigExtract.h"
#include "../ConfigCode/ConfigCAB.h"
#include "../Utilities/OSUtil.h"

CArchiverCAB::CArchiverCAB()
{
	m_nRequiredVersion=98;
	m_strDllName=_T("CAB32.DLL");
	m_AstrPrefix="Cab";
	m_AstrFindParam="**";
}

CArchiverCAB::~CArchiverCAB()
{
	FreeDLL();
}

bool CArchiverCAB::Compress(LPCTSTR ArcFileName,std::list<CString> &ParamList,CConfigManager &ConfMan,const PARAMETER_TYPE,int Options,LPCTSTR,LPCTSTR lpszMethod,LPCTSTR lpszLevel,CString &strLog)
{
	if(!IsOK()){
		return false;
	}

	ASSERT(0!=_tcslen(ArcFileName));
	TRACE(_T("ArcFileName=%s\n"),ArcFileName);

	//==============================================
	// ���X�|���X�t�@�C���p�e���|�����t�@�C�����擾
	//==============================================
	TCHAR ResponceFileName[_MAX_PATH+1];
	FILL_ZERO(ResponceFileName);
	if(!UtilGetTemporaryFileName(ResponceFileName,_T("cab"))){
		strLog=CString(MAKEINTRESOURCE(IDS_ERROR_TEMPORARY_FILE_CREATE));
		return false;
	}
	ASSERT(0!=_tcslen(ResponceFileName));

	//============================================
	// ���ȉ𓀃t�@�C���p�e���|�����t�@�C�����擾
	//============================================
	TCHAR SFXTemporaryFileName[_MAX_PATH+1];
	FILL_ZERO(SFXTemporaryFileName);
	bool bSFX=(0!=(Options&COMPRESS_SFX));
	if(bSFX){
		//3�i�K�쐬����
		if(!UtilGetTemporaryFileName(SFXTemporaryFileName,_T("sfx"))){
			strLog=CString(MAKEINTRESOURCE(IDS_ERROR_TEMPORARY_FILE_CREATE));
			return false;
		}
		ASSERT(0!=_tcslen(SFXTemporaryFileName));
		DeleteFile(SFXTemporaryFileName);//�S�~�t�@�C������

		PathRemoveExtension(SFXTemporaryFileName);
		PathAddExtension(SFXTemporaryFileName,_T(".cab"));
	}

	//======================================================
	// ���X�|���X�t�@�C�����Ɉ��k�Ώۃt�@�C�������L������
	// �A�[�J�C�u�t�@�C�����̓R�}���h���C���Œ��ڎw�肷��
	// �T�u�t�H���_���̃t�@�C�������ڎw�肵�Ă��BDLL��
	// �ċN�����ɗ���Ɨ\��O�̃t�@�C���܂ň��k�Ώۂɂ����
	//======================================================
	{
		HANDLE hFile=CreateFile(ResponceFileName,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
		if(INVALID_HANDLE_VALUE==hFile){
			strLog=CString(MAKEINTRESOURCE(IDS_ERROR_TEMPORARY_FILE_ACCESS));
			return false;
		}

		TRACE(_T("���X�|���X�t�@�C���ւ̏�������\n"));
		std::list<CString>::iterator ite;
		for(ite=ParamList.begin();ite!=ParamList.end();ite++){
			if(PathIsDirectory(*ite)){
				EnumAndWriteSubDir(hFile,*ite);
			}
			else WriteResponceFile(hFile,*ite);
		}
		CloseHandle(hFile);
	}

	//===========================
	// DLL�ɓn���I�v�V�����̐ݒ�
	//===========================
	TRACE(_T("DLL�ɓn���I�v�V�����̐ݒ�\n"));

	CString Param;//�R�}���h���C�� �p�����[�^ �o�b�t�@

	//���k�p�����[�^
	Param+=_T("-a ");	//���k
//	Param+=_T("-r ");	//�ċN�I�ɏ���	�����LhaForge���Ńt�@�C������񋓂��邱�ƂőΏ�

	CConfigCAB Config;
	Config.load(ConfMan);
	int nCompressType=Config.CompressType;
	int LZX_Level=Config.LZX_Level;
	if(lpszMethod && *lpszMethod!=_T('\0')){
		if(0==_tcsicmp(lpszMethod,_T("mszip"))){
			nCompressType=CAB_COMPRESS_MSZIP;
		}else if(0==_tcsicmp(lpszMethod,_T("lzx"))){
			nCompressType=CAB_COMPRESS_LZX;
		}else if(0==_tcsicmp(lpszMethod,_T("plain"))){
			nCompressType=CAB_COMPRESS_PLAIN;
		}else{
			nCompressType=-1;
		}
	}
	if(lpszLevel && *lpszLevel!=_T('\0')){
		LZX_Level=_ttoi(lpszLevel);
	}


	switch(nCompressType){
	case -1:
		Param.AppendFormat(_T("-m%s "),lpszMethod);	//�R�}���h���C���ɏ]��
		break;
	case CAB_COMPRESS_MSZIP:
		Param+=_T("-mz ");	//MSZIP�`���ň��k
		break;
	case CAB_COMPRESS_LZX:
		Param+=_T("-ml:");	//LZX�`��
		{
			CString buf;
			buf.Format(_T("%d"),LZX_Level);
			Param+=buf;
		}
		Param+=_T(" ");
		break;
	case CAB_COMPRESS_PLAIN:
		Param+=_T("-ms ");	//�����k
		break;
	}

	//���k��t�@�C�����w��
	if(bSFX){
		Param+=_T("\"");
		Param+=SFXTemporaryFileName;
		Param+=_T("\" ");
	}else{
		Param+=_T("\"");
		Param+=ArcFileName;
		Param+=_T("\" ");
	}

	//���X�|���X�t�@�C�����w��
	Param+=_T("\"@");
	Param+=ResponceFileName;
	Param+=_T("\"");

	ASSERT(!Param.IsEmpty());
	TRACE(_T("ArchiveHandler Commandline Parameter:%s\n"),Param);

	TRACE(_T("ArchiveHandler�Ăяo��\n"));
	std::vector<char> szLog(LOG_BUFFER_SIZE);
	szLog[0]='\0';
	int Ret=ArchiveHandler(NULL,CT2A(Param),&szLog[0],LOG_BUFFER_SIZE-1);
	strLog=&szLog[0];

	//�g�������X�|���X�t�@�C���͏���
	DeleteFile(ResponceFileName);

	//�G���[�����O�o��
	if(!bSFX||0!=Ret){
		if(bSFX){
			DeleteFile(SFXTemporaryFileName);
		}
		return 0==Ret;
	}

	//====================
	// ���ȉ𓀏��ɂɕϊ�
	//====================
	//�ϊ��p�����[�^
	Param=_T("-f ");		//�ϊ�
	Param+=_T("\"");
	Param+=SFXTemporaryFileName;	//�ϊ����t�@�C����
	Param+=_T("\" ");

	//----------
	//�o�͐�t�H���_��
	TCHAR Buffer[_MAX_PATH+1];
	FILL_ZERO(Buffer);
	GetTempPath(_MAX_PATH,Buffer);
	PathAddBackslash(Buffer);

	Param+=_T(" \"");
	Param+=Buffer;
	Param+=_T("\" ");

	ASSERT(!Param.IsEmpty());
	TRACE(_T("ArchiveHandler Commandline Parameter(SFX):%s\n"),Param);

	TRACE(_T("SFX�pArchiveHandler�Ăяo��\n"));
	std::vector<char> LogSFX(LOG_BUFFER_SIZE);
	LogSFX[0]='\0';
	Ret=ArchiveHandler(NULL,CT2A(Param),&LogSFX[0],LOG_BUFFER_SIZE-1);
	strLog+=&LogSFX[0];
	DeleteFile(SFXTemporaryFileName);

	if(!Ret){	//���������ꍇ�A�t�@�C������ύX
		//�܂��͕ϊ����ꂽ���ȉ𓀃t�@�C�������쐬
		PathStripPath(SFXTemporaryFileName);
		PathRemoveExtension(SFXTemporaryFileName);
		PathAddExtension(SFXTemporaryFileName,_T(".exe"));
		PathAppend(Buffer,SFXTemporaryFileName);
		//�t�@�C���ړ�
		MoveFile(Buffer,ArcFileName);
	}

	return 0==Ret;
}

//�t�@�C�����𓀁G�ʏ��
bool CArchiverCAB::Extract(LPCTSTR ArcFileName,CConfigManager &ConfMan,const CConfigExtract &Config,bool bSafeArchive,LPCTSTR OutputDir,CString &strLog)
{
	if(!IsOK()){
		return false;
	}

	if(!bSafeArchive){
		strLog.Format(IDS_ERROR_DANGEROUS_ARCHIVE,ArcFileName);
		return false;
	}

	//�o�͐�ړ�
	CCurrentDirManager currentDir(OutputDir);
	//===========================
	// DLL�ɓn���I�v�V�����̐ݒ�
	//===========================
	TRACE(_T("DLL�ɓn���I�v�V�����̐ݒ�\n"));

	CString Param;//�R�}���h���C�� �p�����[�^ �o�b�t�@

	//�𓀃p�����[�^
	Param+=_T("-x ");		//��
//	Param+=_T("-k ");		//�}���`�{�����[��;��߂���������

	if(Config.ForceOverwrite){
		//�����㏑��
		Param+=_T("-o ");
	}

	//�A�[�J�C�u�t�@�C�����w��
	Param+=_T("\"");
	Param+=ArcFileName;
	Param+=_T("\" ");

	ASSERT(!Param.IsEmpty());
	TRACE(_T("ArchiveHandler Commandline Parameter:%s\n"),Param);

	TRACE(_T("ArchiveHandler�Ăяo��\n"));
	//char szLog[LOG_BUFFER_SIZE]={0};
	std::vector<char> szLog(LOG_BUFFER_SIZE);
	szLog[0]='\0';
	int Ret=ArchiveHandler(NULL,CT2A(Param),&szLog[0],LOG_BUFFER_SIZE-1);
	strLog=&szLog[0];

	return 0==Ret;
}


//============================================================
// CabGetFileName()�̏o�͌��ʂ���ɁA�i�[���ꂽ�t�@�C�����p�X
// ���������Ă��邩�ǂ������ʂ��A��d�t�H���_�쐬��h��
// �𓀂��Ă����S���ǂ��������f����
//============================================================
bool CArchiverCAB::ExamineArchive(LPCTSTR ArcFileName,CConfigManager& ConfMan,bool,bool &bInFolder,bool &bSafeArchive,CString &BaseDir,CString &strErr)
{
	return _ExamineArchive(ArcFileName,ConfMan,bInFolder,bSafeArchive,2,BaseDir,strErr);
}


// �T�u�f�B���N�g������т��̒��̃t�@�C����񋓂��A
// �t�@�C���������X�|���X�t�@�C���ɏ�������
void CArchiverCAB::EnumAndWriteSubDir(HANDLE hFile,LPCTSTR Path)
{
	WIN32_FIND_DATA lp;

	CString Buffer=Path;
	Buffer+=_T("\\*");
	HANDLE h=FindFirstFile(Buffer,&lp);

	do{
		if((lp.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)&& 0!=_tcscmp(lp.cFileName,_T("..")) && 0!=_tcscmp(lp.cFileName,_T(".")))
		{
			CString SubPath=Path;
			SubPath+=_T("\\");
			SubPath+=lp.cFileName;

			EnumAndWriteSubDir(hFile,SubPath);
		}
		if((lp.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)!=FILE_ATTRIBUTE_DIRECTORY)
		{
			// lp.cFileName�Ńt�@�C������������
			// wsprintf(�o�͕�����,"%s%s",temp,lp.cFileName);�Ƃ����
			// �t�@�C���̃t���p�X��������
			CString FileName=Path;
			FileName+=_T("\\");
			FileName+=lp.cFileName;

			if(0==_tcsnccmp(FileName,_T(".\\"),2)){
				//�J�����g�f�B���N�g���w�肪����΍폜���Ă���
				FileName.Delete(0,2);
			}

			WriteResponceFile(hFile,FileName);
		}
	}while(FindNextFile(h,&lp));

	FindClose(h);
}

bool CArchiverCAB::ExtractSpecifiedOnly(LPCTSTR ArcFileName,CConfigManager&,LPCTSTR OutputDir,std::list<CString> &FileList,CString &strLog,bool bUsePath)
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
	if(!UtilGetTemporaryFileName(ResponceFileName,_T("cab"))){
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
		for(;ite!=end;ite++){
			WriteResponceFile(hFile,*ite);
		}
		CloseHandle(hFile);
	}

	//===========================
	// DLL�ɓn���I�v�V�����̐ݒ�
	//===========================
	CString Param;

	//�𓀃p�����[�^
	Param+=_T("-x ");		//��
	if(!bUsePath){
		Param+=_T("-j ");		//�t�H���_�𖳎����Ĉ��k�܂��͉�
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
	Param+=_T("\"@");
	Param+=ResponceFileName;
	Param+=_T("\"");

	//�o�͐�f�B���N�g�����쐬���Ă���
	if(!UtilMakeSureDirectoryPathExists(OutputDir)){
		strLog.Format(IDS_ERROR_CANNOT_MAKE_DIR,OutputDir);
		return false;
	}

	TRACE(_T("ArchiveHandler�Ăяo��\nCommandline Parameter:%s\n"),Param);
	//char szLog[LOG_BUFFER_SIZE]={0};
	std::vector<char> szLog(LOG_BUFFER_SIZE);
	szLog[0]='\0';
	int Ret=ArchiveHandler(NULL,CT2A(Param),&szLog[0],LOG_BUFFER_SIZE-1);
	strLog=&szLog[0];
	//�g�������X�|���X�t�@�C���͏���
	DeleteFile(ResponceFileName);

	return 0==Ret;
}


ARCRESULT CArchiverCAB::TestArchive(LPCTSTR ArcFileName,CString &strLog)
{
	TRACE(_T("TestArchive() called.\n"));
	ASSERT(IsOK());
	if(!IsOK()){
		return TEST_ERROR;
	}

	//t�R�}���h�ɂ��e�X�g����������Ă���
	CString Param=
		_T("-t ")			//�e�X�g
		_T("\"")
	;
	Param+=ArcFileName;
	Param+=_T("\"");

	std::vector<char> szLog(LOG_BUFFER_SIZE);
	szLog[0]='\0';
	int Ret=ArchiveHandler(NULL,CT2A(Param),&szLog[0],LOG_BUFFER_SIZE-1);
	strLog=&szLog[0];

	if(0==Ret){
		strLog+=_T("\r\n");
		strLog+=CString(MAKEINTRESOURCE(IDS_TESTARCHIVE_RESULT_OK));
		return TEST_OK;
	}
	else return TEST_NG;
}
