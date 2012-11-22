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
#include "ArchiverUNBEL.h"
#include "../resource.h"
#include "../ConfigCode/ConfigExtract.h"
#include "../Utilities/FileOperation.h"
#include "../Utilities/OSUtil.h"

CArchiverUNBEL::CArchiverUNBEL()
{
	m_nRequiredVersion=45;
	m_nRequiredSubVersion=1;
	m_strDllName=_T("Unbel32.DLL");
	m_AstrPrefix="Unbel";
	m_LoadLevel=LOAD_DLL_MINIMUM;
}

CArchiverUNBEL::~CArchiverUNBEL()
{
	FreeDLL();
}

LOAD_RESULT CArchiverUNBEL::LoadDLL(CConfigManager &ConfigManager,CString &strErr)
{
	FreeDLL();

	LOAD_RESULT res=CArchiverDLL::LoadDLL(ConfigManager,strErr);
	if(LOAD_RESULT_OK!=res){
		return res;
	}

	CStringA FunctionName=m_AstrPrefix;
	FunctionName+="OpenArchive";
	ArchiverOpenArchive=(COMMON_ARCHIVER_OPENARCHIVE)GetProcAddress(m_hInstDLL,FunctionName);
	if(NULL==ArchiverOpenArchive){
		strErr.Format(IDS_ERROR_DLL_FUNCTION_GET,m_strDllName,FunctionName);
		FreeDLL();
		return LOAD_RESULT_INVALID;
	}

	FunctionName=m_AstrPrefix;
	FunctionName+="CloseArchive";
	ArchiverCloseArchive=(COMMON_ARCHIVER_CLOSEARCHIVE)GetProcAddress(m_hInstDLL,FunctionName);
	if(NULL==ArchiverCloseArchive){
		strErr.Format(IDS_ERROR_DLL_FUNCTION_GET,m_strDllName,FunctionName);
		FreeDLL();
		return LOAD_RESULT_INVALID;
	}

	//FindFirst
	FunctionName=m_AstrPrefix;
	FunctionName+="FindFirst";
	ArchiverFindFirst=(COMMON_ARCHIVER_FINDFIRST)GetProcAddress(m_hInstDLL,FunctionName);
	if(NULL==ArchiverFindFirst){
		strErr.Format(IDS_ERROR_DLL_FUNCTION_GET,m_strDllName,FunctionName);
		FreeDLL();
		return LOAD_RESULT_INVALID;
	}

	//�t�@�C�����擾
	FunctionName=m_AstrPrefix;
	FunctionName+="GetFileName";
	ArchiverGetFileName=(COMMON_ARCHIVER_GETFILENAME)GetProcAddress(m_hInstDLL,FunctionName);
	if(NULL==ArchiverGetFileName){
		strErr.Format(IDS_ERROR_DLL_FUNCTION_GET,m_strDllName,FunctionName);
		FreeDLL();
		return LOAD_RESULT_INVALID;
	}
	return LOAD_RESULT_OK;
}

bool CArchiverUNBEL::Compress(LPCTSTR,std::list<CString>&,CConfigManager&,const PARAMETER_TYPE,int,LPCTSTR,LPCTSTR,LPCTSTR,CString &)
{
	//BEL�͉𓀂̂�
	ASSERT(false);
	return false;
}

bool CArchiverUNBEL::Extract(LPCTSTR ArcFileName,CConfigManager&,const CConfigExtract&,bool bSafeArchive,LPCTSTR OutputDir,CString &strLog)
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

	//�𓀃p�����[�^�͂Ȃ�
//	if(Config.Common.Extract.ForceOverwrite){
		//�����㏑���͂ł��Ȃ�����
//	}

	//�A�[�J�C�u�t�@�C�����w��
	Param+=_T("\"");
	Param+=ArcFileName;
	Param+=_T("\" ");

	//�o�͐�w��
	Param+=_T("\"");
	Param+=_T(".\\");
	Param+=_T("\"");

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
// Belon�`���̓t�@�C��������������i�[�ł��Ȃ���A
// �t�@�C�����͊g���q3�����݂̂����ێ����Ă��Ȃ�
// �������ADTV�͋N������B
//=========================================================
bool CArchiverUNBEL::ExamineArchive(LPCTSTR ArcFileName,CConfigManager& ConfMan,bool,bool &bInFolder,bool &bSafeArchive,CString &BaseDir,CString &strErr)
{
	bInFolder=false;
	ASSERT(IsOK());
	if(!IsOK()){
		return false;
	}

	if(!InspectArchiveBegin(ArcFileName,ConfMan)){
		strErr.Format(IDS_ERROR_OPEN_ARCHIVE,ArcFileName);
		return false;
	}

	CString Buffer;
	InspectArchiveNext();		//�P��t�@�C���̂݊i�[
	InspectArchiveGetFileName(Buffer);
	InspectArchiveEnd();

	Buffer.Replace(_T('\\'),_T('/'));		//�p�X��؂蕶���̒u������
	TRACE(_T("%s\n"),Buffer);

	if(-1!=Buffer.Find(_T("../"))){
		bSafeArchive=false;
	}
	else{
		bSafeArchive=true;
	}
	return true;
}

bool CArchiverUNBEL::ExtractSpecifiedOnly(LPCTSTR ArcFileName,CConfigManager&,LPCTSTR OutputDir,std::list<CString>&,CString &,bool bUsePath)
{
	return false;
}

