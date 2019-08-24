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
#include "ArchiverUNIMP.h"
#include "../Utilities/FileOperation.h"
#include "../resource.h"
#include "../ConfigCode/ConfigExtract.h"
#include "../Utilities/OSUtil.h"

CArchiverUNIMP::CArchiverUNIMP()
{
	m_nRequiredVersion=17;
	m_nRequiredSubVersion=43;
	m_strDllName=_T("UnImp32.dll");
	m_AstrPrefix="UnImp";
}

CArchiverUNIMP::~CArchiverUNIMP()
{
	FreeDLL();
}

bool CArchiverUNIMP::Compress(LPCTSTR,std::list<CString>&,CConfigManager&,const PARAMETER_TYPE,int,LPCTSTR,LPCTSTR,LPCTSTR,CString &)
{
	//IMP�͉𓀂̂�
	ASSERT(false);
	return false;
}

//bSafeArchive�͖��������
bool CArchiverUNIMP::Extract(LPCTSTR ArcFileName,CConfigManager&,const CConfigExtract &Config,bool,LPCTSTR OutputDir,CString &strLog)
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
		_T("x ")		//��
		_T("-a1 ")		//�S�����̉�
		_T("-jf1 ")		//���[�g�L���̍폜(���΃p�X�ɕϊ�)
		_T("-r1 ")		//�T�u�f�B���N�g��������
		_T("-jp1 ")		//�s���p�X(���[�U�[�₢���킹)
	;
	if(Config.ForceOverwrite){
		//�����㏑��
		Param+=_T("-u0 ");
	}
	else{
		Param+=_T("-u5 ");	//���[�U�[�₢���킹
	}

	//�A�[�J�C�u�t�@�C�����w��
	Param+=_T("-- \"");
	Param+=ArcFileName;
	Param+=_T("\" ");

	//�o�͐�w��
	Param+=_T("\"");
	Param+=_T(".\\");//OutputDir;
	Param+=_T("\"");

	ASSERT(!Param.IsEmpty());
	TRACE(_T("ArchiveHandler Commandline Parameter:%s\n"),Param);

	TRACE(_T("ArchiveHandler�Ăяo��\n"));
	std::vector<char> szLog(LOG_BUFFER_SIZE);
	szLog[0]='\0';
	int Ret=ArchiveHandler(NULL,CT2A(Param),&szLog[0],LOG_BUFFER_SIZE-1);
	strLog=&szLog[0];

	return 0==Ret;
}

//=========================================================
// UnImpGetFileName()�̏o�͌��ʂ���ɁA�i�[���ꂽ�t�@�C�����p�X
// ���������Ă��邩�ǂ������ʂ��A��d�t�H���_�쐬��h��
//=========================================================
bool CArchiverUNIMP::ExamineArchive(LPCTSTR ArcFileName,CConfigManager& ConfMan,bool bSkipDir,bool &bInFolder,bool &,CString &BaseDir,CString &strErr)
{
	if(bSkipDir){
		TRACE(_T("��d�t�H���_������\n"));
		return true;
	}
	return _ExamineArchiveFast(ArcFileName,ConfMan,bInFolder,BaseDir,strErr);
}

bool CArchiverUNIMP::InspectArchiveGetWriteTime(FILETIME &FileTime)
{
	if(!m_hInspectArchive){
		ASSERT(!"Open an Archive First!!!\n");
		return false;
	}
	//�g���Ŋ֐��Ŏ����擾
	if(ArchiverGetWriteTimeEx){
		FILETIME TempTime;
		if(!ArchiverGetWriteTimeEx(m_hInspectArchive,&TempTime))return false;
		if(!LocalFileTimeToFileTime(&TempTime,&FileTime))return false;
		return true;
	}
	//�ʏ�Ŋ֐��Ŏ����擾
	else if(ArchiverGetWriteTime){
		DWORD UnixTime=ArchiverGetWriteTime(m_hInspectArchive);
		if(-1==UnixTime){
			return false;
		}
		//time_t����FileTime�֕ϊ�
		LONGLONG ll = Int32x32To64(UnixTime, 10000000) + 116444736000000000;
		FileTime.dwLowDateTime = (DWORD) ll;
		FileTime.dwHighDateTime = (DWORD)(ll >>32);
		return true;
	}
	else{
		//INDIVIDUALINFO���玞���擾
		FILETIME TempTime;
		if(!DosDateTimeToFileTime(m_IndividualInfo.wDate,m_IndividualInfo.wTime,&TempTime))return false;
		if(!LocalFileTimeToFileTime(&TempTime,&FileTime))return false;
		return true;
	}
}

//���X�|���X�t�@�C���Ƀt�@�C�������������ށB
//�L���ȃt�@�C���n���h����NULL�łȂ��t�@�C������n�����ƁB
void CArchiverUNIMP::WriteResponceFile(HANDLE hFile,LPCTSTR fname)
{
	CArchiverDLL::WriteResponceFile(hFile,fname,false);
}

bool CArchiverUNIMP::ExtractSpecifiedOnly(LPCTSTR ArcFileName,CConfigManager&,LPCTSTR OutputDir,std::list<CString> &FileList,CString &strLog,bool bUsePath)
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
	if(!UtilGetTemporaryFileName(ResponceFileName,_T("imp"))){
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
	if(bUsePath){
		Param+=_T("x ");	//�p�X�����
	}else{
		Param+=_T("e ");	//�p�X������
	}
	Param+=
		_T("-a1 ")		//�S�����̉�
		_T("-jf1 ")		//���[�g�L���̍폜(���΃p�X�ɕϊ�)
		_T("-r0 ")		//�T�u�f�B���N�g���͌������Ȃ�
		_T("-jp1 ")		//�s���p�X(���[�U�[�₢���킹)
		_T("-u5 ")		//�㏑��(���[�U�[�₢���킹)
	;

	//�A�[�J�C�u�t�@�C�����w��
	Param+=_T("-- \"");
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

ARCRESULT CArchiverUNIMP::TestArchive(LPCTSTR ArcFileName,CString &strLog)
{
	TRACE(_T("TestArchive() called.\n"));
	ASSERT(IsOK());
	if(!IsOK()){
		return TEST_ERROR;
	}

	//����DLL�̃`�F�b�N�\�͂͒Ⴂ

/*	//CheckArchive�ɂ��e�X�g����������Ă���
	strLog.Format(IDS_TESTARCHIVE_WITH_CHECKARCHIVE,ArcFileName,GetName());	//API�ɂ��`�F�b�N
	strLog+=_T("\r\n\r\n");

	const int CHECKARCHIVE_FULLCRC=2;
	if(ArchiverCheckArchive(ArcFileName,CHECKARCHIVE_FULLCRC)){
		//����
		strLog+=CString(MAKEINTRESOURCE(IDS_TESTARCHIVE_RESULT_OK));
		return TEST_OK;
	}
	else{
		//�ُ�
		strLog+=CString(MAKEINTRESOURCE(IDS_TESTARCHIVE_RESULT_NG));
		return TEST_NG;
	}*/

	//t�R�}���h�ɂ��e�X�g����������Ă���
	CString Param=
		_T("t ")			//�e�X�g
		_T("-- \"")
	;
	Param+=ArcFileName;
	Param+=_T("\"");

	TRACE(_T("%s\n"),Param);

	//�t�@�C���������O�ɒǉ�
	strLog=ArcFileName;
	strLog+=_T("\n\n");

	//char szLog[LOG_BUFFER_SIZE]={0};
	std::vector<char> szLog(LOG_BUFFER_SIZE);
	szLog[0]='\0';
	int Ret=ArchiveHandler(NULL,CT2A(Param),&szLog[0],LOG_BUFFER_SIZE-1);
	strLog+=&szLog[0];

	if(Ret==0)return TEST_OK;
	else return TEST_NG;
}
