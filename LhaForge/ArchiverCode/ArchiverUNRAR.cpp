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
#include "ArchiverUNRAR.h"
#include "../Utilities/FileOperation.h"
#include "../Utilities/StringUtil.h"
#include "../resource.h"
#include "../ConfigCode/ConfigExtract.h"
#include "../Utilities/OSUtil.h"

CArchiverUNRAR::CArchiverUNRAR()
{
	m_nRequiredVersion=12;
	m_strDllName=_T("unrar32.dll");
	m_AstrPrefix="Unrar";
	m_bUTF8=false;

	m_strDllDisplayName=m_strDllName;
}

CArchiverUNRAR::~CArchiverUNRAR()
{
	FreeDLL();
}

LOAD_RESULT CArchiverUNRAR::LoadDLL(CConfigManager &ConfMan,CString &strErr)
{
	FreeDLL();

	//���N���X�̃��\�b�h���Ă�
	LOAD_RESULT res=CArchiverDLL::LoadDLL(ConfMan,strErr);
	if(LOAD_RESULT_OK!=res){
		return res;
	}

	//UNICODE���[�h�ݒ�p
	CStringA strFunctionName;
	strFunctionName=m_AstrPrefix+"SetUnicodeMode";
	ArchiverSetUnicodeMode=(COMMON_ARCHIVER_SETUNICODEMODE)GetProcAddress(m_hInstDLL,strFunctionName);
	if(NULL==ArchiverSetUnicodeMode){
		m_bUTF8=false;
	}
	//������UNICODE���[�h�ɐݒ�: UNICODE��Ή���DLL�Ȃ牽�����Ȃ�
	if(ArchiverSetUnicodeMode && ArchiverSetUnicodeMode(TRUE)){
		m_bUTF8=true;
	}else{
		m_bUTF8=false;
	}

	m_strDllDisplayName=m_bUTF8 ? m_strDllName+_T("[UTF8]") : m_strDllName;

	return LOAD_RESULT_OK;
}

void CArchiverUNRAR::FreeDLL()
{
	if(m_hInstDLL){
		ArchiverSetUnicodeMode=NULL;
		m_strDllDisplayName=m_strDllName;
		CArchiverDLL::FreeDLL();
	}
}

//���X�|���X�t�@�C���Ƀt�@�C�������G�X�P�[�v�����s������ŏ������ށB
//�L���ȃt�@�C���n���h����NULL�łȂ��t�@�C������n�����ƁB
void CArchiverUNRAR::WriteResponceFile(HANDLE hFile,LPCTSTR fname)
{
	if(m_bUTF8){
		CPath strPath=fname;

		strPath.QuoteSpaces();

		DWORD dwWritten=0;
		//�t�@�C����+���s���o��
		std::vector<BYTE> cArray;
		UtilToUTF8(cArray,strPath+_T("\r\n"));
		WriteFile(hFile,&cArray[0],(cArray.size()-1)*sizeof(BYTE),&dwWritten,NULL);
	}else{
		CArchiverDLL::WriteResponceFile(hFile,fname);
	}
}

bool CArchiverUNRAR::Compress(LPCTSTR,std::list<CString>&,CConfigManager&,const PARAMETER_TYPE,int,LPCTSTR,LPCTSTR,LPCTSTR,CString &)
{
	//RAR�͉𓀂̂�
	ASSERT(false);
	return false;
}

//bSafeArchive�͖��������
bool CArchiverUNRAR::Extract(LPCTSTR ArcFileName,CConfigManager&,const CConfigExtract &Config,bool,LPCTSTR OutputDir,CString &strLog)
{
	if(!IsOK()){
		return false;
	}

	//===========================
	// DLL�ɓn���I�v�V�����̐ݒ�
	//===========================
	TRACE(_T("DLL�ɓn���I�v�V�����̐ݒ�\n"));

	CString Param;//�R�}���h���C�� �p�����[�^ �o�b�t�@
	//�o�͐�ړ�
	CCurrentDirManager currentDir(OutputDir);

	//�𓀃p�����[�^
	Param+=
		_T("-x ")		//��
		_T("-e2 ")		//�Z�L�����e�B���x��(��f�B���N�g���ȊO�ɓW�J�����悤�ȃp�X���\�Ȍ���ǂ��ɂ����܂�)
	;
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
	std::vector<BYTE> szLog(LOG_BUFFER_SIZE);
	szLog[0]='\0';
	int Ret;
	if(m_bUTF8){
		Ret=ArchiveHandler(NULL,C2UTF8(Param),(LPSTR)&szLog[0],LOG_BUFFER_SIZE-1);
		CString strTmp;
		UtilToUNICODE(strTmp,&szLog[0],szLog.size()-1,UTILCP_UTF8);
		strLog=strTmp;
	}else{
		Ret=ArchiveHandler(NULL,CT2A(Param),(LPSTR)&szLog[0],LOG_BUFFER_SIZE-1);
		strLog=&szLog[0];
	}

	return 0==Ret;
}

//=========================================================
// UnrarGetFileName()�̏o�͌��ʂ���ɁA�i�[���ꂽ�t�@�C�����p�X
// ���������Ă��邩�ǂ������ʂ��A��d�t�H���_�쐬��h��
//=========================================================
bool CArchiverUNRAR::ExamineArchive(LPCTSTR ArcFileName,CConfigManager& ConfMan,bool bSkipDir,bool &bInFolder,bool &,CString &BaseDir,CString &strErr)
{
	if(bSkipDir){
		TRACE(_T("��d�t�H���_������\n"));
		return true;
	}
	if(IsHeaderEncrypted(ArcFileName)){
		//�w�b�_�Í����ς�?
		CConfigExtract ConfExtract;
		ConfExtract.load(ConfMan);
		if(ConfExtract.MinimumPasswordRequest){
			//�p�X���[�h���͉񐔂��ŏ�����;��d������X�L�b�v����
			bInFolder=false;
			return true;
		}
	}
	return _ExamineArchiveFast(ArcFileName,ConfMan,bInFolder,BaseDir,strErr);
}


bool CArchiverUNRAR::ExtractSpecifiedOnly(LPCTSTR ArcFileName,CConfigManager&,LPCTSTR OutputDir,std::list<CString> &FileList,CString &strLog,bool bUsePath)
{
	if(!IsOK()){
		return false;
	}

	//==============================================
	// ���X�|���X�t�@�C���p�e���|�����t�@�C�����擾
	//==============================================
	TCHAR ResponceFileName[_MAX_PATH+1];
	FILL_ZERO(ResponceFileName);
	if(!UtilGetTemporaryFileName(ResponceFileName,_T("rar"))){
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
			CPath target = *ite;
			WriteResponceFile(hFile,*ite);
		}
		CloseHandle(hFile);
	}
	//�o�͐�ړ�
	CCurrentDirManager currentDir(OutputDir);

	//===========================
	// DLL�ɓn���I�v�V�����̐ݒ�
	//===========================
	CString Param;

	//�𓀃p�����[�^
	if(bUsePath){
		Param+=_T("-x ");	//�p�X�t����
	}else{
		Param+=_T("-e ");		//�p�X������
	}
	Param+=
		_T("-s ")		//�����ɔ�r
		_T("-e2 ")		//�Z�L�����e�B���x��(��f�B���N�g���ȊO�ɓW�J�����悤�ȃp�X���\�Ȍ���ǂ��ɂ����܂�)
	;

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
	std::vector<BYTE> szLog(LOG_BUFFER_SIZE);
	szLog[0]='\0';
	int Ret;
	if(m_bUTF8){
		Ret=ArchiveHandler(NULL,C2UTF8(Param),(LPSTR)&szLog[0],LOG_BUFFER_SIZE-1);
		CString strTmp;
		UtilToUNICODE(strTmp,&szLog[0],szLog.size()-1,UTILCP_UTF8);
		strLog=strTmp;
	}else{
		Ret=ArchiveHandler(NULL,CT2A(Param),(LPSTR)&szLog[0],LOG_BUFFER_SIZE-1);
		strLog=&szLog[0];
	}
	//�g�������X�|���X�t�@�C���͏���
	DeleteFile(ResponceFileName);

	return 0==Ret;
}

ARCRESULT CArchiverUNRAR::TestArchive(LPCTSTR ArcFileName,CString &strLog)
{
	TRACE(_T("TestArchive() called.\n"));
	ASSERT(IsOK()&&ArchiverCheckArchive);
	if(!IsOK()||!ArchiverCheckArchive){
		return TEST_ERROR;
	}

	//CheckArchive�ɂ��e�X�g����������Ă���
	strLog.Format(IDS_TESTARCHIVE_WITH_CHECKARCHIVE,ArcFileName,GetName());	//API�ɂ��`�F�b�N
	strLog+=_T("\r\n\r\n");

	if(ArchiverCheckArchive(m_bUTF8 ? (LPCSTR)C2UTF8(ArcFileName) : CT2A(ArcFileName),CHECKARCHIVE_FULLCRC|CHECKARCHIVE_NOT_ASK_PASSWORD)){
		//����
		strLog+=CString(MAKEINTRESOURCE(IDS_TESTARCHIVE_RESULT_OK));
		return TEST_OK;
	}else{
		//�ُ�
		strLog+=CString(MAKEINTRESOURCE(IDS_TESTARCHIVE_RESULT_NG));
		return TEST_NG;
	}
}

BOOL CArchiverUNRAR::CheckArchive(LPCTSTR _szFileName)
{
	if(!ArchiverCheckArchive){
		ASSERT(ArchiverCheckArchive);
		return false;
	}
	return ArchiverCheckArchive(m_bUTF8 ? (LPCSTR)C2UTF8(_szFileName) : CT2A(_szFileName),CHECKARCHIVE_BASIC|CHECKARCHIVE_NOT_ASK_PASSWORD);
}

bool CArchiverUNRAR::IsHeaderEncrypted(LPCTSTR _szFileName)
{
	return ERROR_PASSWORD_FILE==CheckArchive(_szFileName);
}

int CArchiverUNRAR::GetFileCount(LPCTSTR _szFileName)
{
	if(!ArchiverGetFileCount){
		return -1;
	}
	return ArchiverGetFileCount(m_bUTF8 ? (LPCSTR)C2UTF8(_szFileName) : CT2A(_szFileName));
}

bool CArchiverUNRAR::InspectArchiveBegin(LPCTSTR ArcFileName,CConfigManager& config)
{
	if(m_bUTF8){
		ASSERT(ArchiverOpenArchive);
		if(!ArchiverOpenArchive){
			return false;
		}
		if(m_hInspectArchive){
			ASSERT(!"Close the Archive First!!!\n");
			return false;
		}
		m_hInspectArchive=ArchiverOpenArchive(NULL,m_bUTF8 ? (LPCSTR)C2UTF8(ArcFileName) : CT2A(ArcFileName),m_dwInspectMode);
		if(!m_hInspectArchive){
			return false;
		}
		m_bInspectFirstTime=true;

		FILL_ZERO(m_IndividualInfo);
		return true;
	}else{
		return __super::InspectArchiveBegin(ArcFileName, config);
	}
}

bool CArchiverUNRAR::InspectArchiveGetFileName(CString &FileName)
{
	if(m_bUTF8){
		if(ArchiverGetFileName){
			if(!m_hInspectArchive){
				ASSERT(!"Open an Archive First!!!\n");
				return false;
			}
			std::vector<BYTE> szBuffer(FNAME_MAX32*2+1);
			ArchiverGetFileName(m_hInspectArchive,(LPCSTR)&szBuffer[0],FNAME_MAX32*2);
			UtilToUNICODE(FileName,&szBuffer[0],szBuffer.size(),UTILCP_UTF8);
			return true;
		}
		else{
			ASSERT(LOAD_DLL_STANDARD!=m_LoadLevel);
			if(!m_hInspectArchive){
				ASSERT(!"Open an Archive First!!!\n");
				return false;
			}
			UtilToUNICODE(FileName,(LPCBYTE)m_IndividualInfo.szFileName,sizeof(m_IndividualInfo.szFileName),UTILCP_UTF8);
			return true;
		}
	}else{
		return __super::InspectArchiveGetFileName(FileName);
	}
}

bool CArchiverUNRAR::InspectArchiveGetMethodString(CString &strMethod)
{
	if(m_bUTF8){
		if(!m_hInspectArchive){
			ASSERT(!"Open an Archive First!!!\n");
			return false;
		}

		//\0�ŏI����Ă��邩�ǂ����ۏ؂ł��Ȃ�
		char szBuffer[32]={0};

		if(ArchiverGetMethod){
			if(0!=ArchiverGetMethod(m_hInspectArchive,szBuffer,31)){
				//�\���̂���擾
				strncpy_s(szBuffer,m_IndividualInfo.szMode,8);
			}
		}
		else{
			//�\���̂���擾
			strncpy_s(szBuffer,m_IndividualInfo.szMode,8);
		}

		//���i�[
		UtilToUNICODE(strMethod,(LPCBYTE)szBuffer,9,UTILCP_UTF8);
		return true;
	}else{
		return __super::InspectArchiveGetMethodString(strMethod);
	}
}
