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
#include "ArchiverYZ1.h"
#include "../Utilities/FileOperation.h"
#include "../resource.h"
#include "../ConfigCode/ConfigExtract.h"
#include "../Utilities/OSUtil.h"

CArchiverYZ1::CArchiverYZ1()
{
//	TRACE(_T("CArchiverYZ1()"));
	m_dwInspectMode=0x00400000L;	//M_ERROR_MESSAGE_ON
//	Separator=_T('\\');
	m_nRequiredVersion=29;
	m_strDllName=_T("YZ1.DLL");
	m_AstrPrefix="Yz1";
}

CArchiverYZ1::~CArchiverYZ1()
{
//	TRACE(_T("~CArchiverYZ1()\n"));
	FreeDLL();
}

bool CArchiverYZ1::Compress(LPCTSTR ArcFileName,std::list<CString> &ParamList,CConfigManager &ConfMan,const PARAMETER_TYPE,int Options,LPCTSTR,LPCTSTR,LPCTSTR,CString &strLog)
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
	if(!UtilGetTemporaryFileName(ResponceFileName,_T("yz1"))){
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
		if(INVALID_HANDLE_VALUE==hFile){
			strLog=CString(MAKEINTRESOURCE(IDS_ERROR_TEMPORARY_FILE_ACCESS));
			return false;
		}

		std::list<CString>::iterator ite;
		for(ite=ParamList.begin();ite!=ParamList.end();ite++){
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
		_T("c ")		//���k
		_T("-r2 ")		//�f�B���N�g���ċA���[�h
		_T("-x1 ")		//�f�B���N�g���i�[
		_T("-+ ")		//YZ1.DLL�̃��W�X�g���̐ݒ�𖳎�
		_T("-ea1 ")		//�p�X�C��
	;
	if(Options&COMPRESS_SFX){	//���ȉ�
		Param+=_T("-s ");
	}

	if(Options&COMPRESS_PASSWORD){
		Param+=_T("-p ");
	}else if(Options&COMPRESS_PUBLIC_PASSWORD){
		Param+=_T("-P ");
	}

	//�A�[�J�C�u�t�@�C�����w��
	Param+=_T("\"");
	Param+=ArcFileName;
	Param+=_T("\" ");

	//���X�|���X�t�@�C�����w��
	Param+=_T("\"@");
	Param+=ResponceFileName;
	Param+=_T("\"");

	ASSERT(!Param.IsEmpty());
	TRACE(_T("ArchiveHandler Commandline Parameter:%s\n"),Param);

	TRACE(_T("ArchiveHandler�Ăяo��\n"));
	//char szLog[LOG_BUFFER_SIZE]={0};
	std::vector<char> szLog(LOG_BUFFER_SIZE);
	szLog[0]='\0';
	int Ret=ArchiveHandler(NULL,CT2A(Param),&szLog[0],LOG_BUFFER_SIZE-1);
	strLog=&szLog[0];

	//�g�������X�|���X�t�@�C���͏���
	DeleteFile(ResponceFileName);

	return 0==Ret;
}

//bSafeArchive�͖��������
bool CArchiverYZ1::Extract(LPCTSTR ArcFileName,CConfigManager&,const CConfigExtract &Config,bool,LPCTSTR OutputDir,CString &strLog)
{
	if(!IsOK()){
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
	Param+=
		_T("x ")			//��
		_T("-x1 ")			//�p�X��L����
		_T("-+ ")			//YZ1.DLL�̃��W�X�g���̐ݒ�𖳎�
		_T("-ea1 ")			//�s���ȃp�X���ǂ����邩����
	;
	if(Config.ForceOverwrite){
		//�����㏑��
		Param+=_T("-c1 ");
	}else{
		Param+=_T("-c2 ");
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


//=========================================================
// Yz1GetFileName()�̏o�͌��ʂ���ɁA�i�[���ꂽ�t�@�C�����p�X
// ���������Ă��邩�ǂ������ʂ��A��d�t�H���_�쐬��h��
//=========================================================
bool CArchiverYZ1::ExamineArchive(LPCTSTR ArcFileName,CConfigManager& ConfMan,bool bSkipDir,bool &bInFolder,bool &,CString &BaseDir,CString &strErr)
{
	if(bSkipDir){
		TRACE(_T("��d�t�H���_������\n"));
		return true;
	}
	return _ExamineArchiveFast(ArcFileName,ConfMan,bInFolder,BaseDir,strErr);
}

bool CArchiverYZ1::ExtractSpecifiedOnly(LPCTSTR ArcFileName,CConfigManager&,LPCTSTR OutputDir,std::list<CString> &FileList,CString &strLog,bool bUsePath)
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
	if(!UtilGetTemporaryFileName(ResponceFileName,_T("yz1"))){
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
	Param+=
		_T("x ")		//��
//		_T("-+ ")		//YZ1.DLL�̃��W�X�g���̐ݒ�𖳎�
		_T("-ea1 ")		//�s���ȃp�X���ǂ����邩����
		_T("-c2 ")		//�㏑���m�F
		_T("-z1 ")		//�S�p�X�ň�v
	;
	if(!bUsePath)Param+=_T("-x0 ");		//�p�X�𖳌���

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

