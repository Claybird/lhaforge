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
#include "ArchiverXACRETT.h"
#include "../Utilities/FileOperation.h"
#include "../resource.h"
#include "../ConfigCode/ConfigExtract.h"
#include "../ConfigCode/ConfigXACRETT.h"
#include "../Utilities/OsUtil.h"

CArchiverXACRETT::CArchiverXACRETT()
{
	m_LoadLevel=LOAD_DLL_SIMPLE_INSPECTION;
	m_nRequiredVersion=49;
	m_nRequiredSubVersion=17;
	m_strDllName=_T("XacRett.DLL");
	m_AstrPrefix="XacRett";
	m_AstrFindParam="*";
}

CArchiverXACRETT::~CArchiverXACRETT()
{ 
	FreeDLL();
}

bool CArchiverXACRETT::Compress(LPCTSTR,std::list<CString> &,CConfigManager &,const PARAMETER_TYPE,int,LPCTSTR,LPCTSTR,LPCTSTR,CString &)
{
	//XACRETT�͉𓀂̂�
	ASSERT(false);
	return false;
}

bool CArchiverXACRETT::Extract(LPCTSTR ArcFileName,CConfigManager&,const CConfigExtract &Config,bool,LPCTSTR OutputDir,CString &strLog)
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
	Param+=_T("x ");		//��
	if(Config.ForceOverwrite){
		//�����㏑��
		Param+=_T("-o1 ");
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
// �i�[���ꂽ�t�@�C�����p�X���������Ă��邩�ǂ������ʂ��A
//��d�t�H���_�쐬��h��
//=========================================================
bool CArchiverXACRETT::ExamineArchive(LPCTSTR ArcFileName,CConfigManager& ConfMan,bool bSkipDir,bool &bInFolder,bool &,CString &BaseDir,CString &strErr)
{
	if(bSkipDir){
		TRACE(_T("��d�t�H���_������\n"));
		return true;
	}
	return _ExamineArchiveFast(ArcFileName,ConfMan,bInFolder,BaseDir,strErr);
}

bool CArchiverXACRETT::ExtractSpecifiedOnly(LPCTSTR ArcFileName,CConfigManager&,LPCTSTR OutputDir,std::list<CString> &FileList,CString &strLog,bool bUsePath)
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
	if(!UtilGetTemporaryFileName(ResponceFileName,_T("xac"))){
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
	Param+=_T("x ");			//��
	if(!bUsePath)Param+=_T("-j ");			//�p�X����
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

//DLL��GetFileCount���T�|�[�g���Ă��Ȃ��̂ŁA�@�\���G�~�����[�g����
int CArchiverXACRETT::GetFileCount(LPCTSTR _szFileName)
{
	if(!IsOK()){
		ASSERT(!"Load DLL First!!!");
		return -1;
	}

	if(!InspectArchiveBegin(_szFileName,CConfigManager())){
		return -1;
	}

	int ItemCount=0;
	while(InspectArchiveNext()){
		ItemCount++;
	}

	InspectArchiveEnd();

	return ItemCount;
}


