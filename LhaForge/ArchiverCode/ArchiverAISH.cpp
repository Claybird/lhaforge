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
#include "ArchiverAISH.h"
#include "../Utilities/FileOperation.h"
#include "../resource.h"
#include "../ConfigCode/ConfigExtract.h"
#include "../ConfigCode/ConfigAISH.h"

CArchiverAISH::CArchiverAISH()
{
	m_nRequiredVersion=4;
	m_strDllName=_T("AISH32.DLL");
	m_AstrPrefix="Aish";
	m_LoadLevel=LOAD_DLL_MINIMUM;
}

CArchiverAISH::~CArchiverAISH()
{
	FreeDLL();
}

bool CArchiverAISH::Compress(LPCTSTR ArcFileName,std::list<CString> &ParamList,CConfigManager &ConfMan,const PARAMETER_TYPE Type,int,LPCTSTR,LPCTSTR lpszMethod,LPCTSTR lpszLevel,CString &strLog)
{
	ASSERT(0!=_tcslen(ArcFileName));
	TRACE(_T("ArcFileName=%s\n"),ArcFileName);


	//===========================
	// DLL�ɓn���I�v�V�����̐ݒ�
	//===========================
	TRACE(_T("DLL�ɓn���I�v�V�����̐ݒ�\n"));

	CString ParamOrg;//�R�}���h���C�� �p�����[�^ �o�b�t�@

	CConfigAISH Config;
	Config.load(ConfMan);
	switch(Type){
	case PARAMETER_ISH:
		if(lpszMethod && *lpszMethod!=_T('\0')){
			ParamOrg+=_T("-");
			ParamOrg+=lpszMethod;
			ParamOrg+=_T(" ");
		}else{
			switch(Config.EncodeType){
			case ISH_SHIFT_JIS:
				ParamOrg+=_T("-s ");
				break;
			case ISH_JIS7:
				ParamOrg+=_T("-7 ");
				break;
			case ISH_JIS8:
				ParamOrg+=_T("-8 ");
				break;
			case ISH_NON_KANA_SHIFT_JIS:
				ParamOrg+=_T("-n ");
				break;
			}
		}
		break;
	case PARAMETER_UUE:
		ParamOrg+=_T("-u ");
		if(Config.UUEncodeChecksum){
			ParamOrg+=_T("-c ");
		}
		break;
	}
	ParamOrg+=_T("-o ");	//�o�̓t�@�C�����w��

	//----------------------
	// ���ԂɃt�@�C��������
	//----------------------
	//�ꎞ�o�͐�t�@�C�������X�g
	std::list<CString> TemporaryFileList;

	std::list<CString>::iterator ite;
	int Ret=0;
	strLog.Empty();
	for(ite=ParamList.begin();ite!=ParamList.end();ite++){
		//==========================
		// �e���|�����t�@�C�����擾
		//==========================

		TCHAR TempFileName[_MAX_PATH+1];
		FILL_ZERO(TempFileName);
		if(!UtilGetTemporaryFileName(TempFileName,_T("ish"))){
			strLog+=CString(MAKEINTRESOURCE(IDS_ERROR_TEMPORARY_FILE_CREATE));
			strLog+=_T("LhaForge : Failed to get temporary file.\r\n");
			Ret=-1;
			break;
		}
		ASSERT(0!=_tcslen(TempFileName));
		TemporaryFileList.push_back(TempFileName);

		CString Param=ParamOrg;
		Param+=_T("\"");
		Param+=TempFileName;
		Param+=_T("\" \".\\");
		Param+=*ite;
		Param+=_T("\"");

		ASSERT(!Param.IsEmpty());
		TRACE(_T("ArchiveHandler Commandline Parameter:%s\n"),Param);

		TRACE(_T("ArchiveHandler�Ăяo��\n"));
		//char szLog[LOG_BUFFER_SIZE]={0};
		std::vector<char> szLog(LOG_BUFFER_SIZE);
		szLog[0]='\0';
		int Ret=ArchiveHandler(NULL,CT2A(Param),&szLog[0],LOG_BUFFER_SIZE-1);
		strLog=&szLog[0];
		strLog+=_T("\r\n");
		if(Ret)break;
	}

	bool bDeleteOutput=false;	//�o�͐�t�@�C�����폜����K�v�����邩�ǂ���
	if(!Ret){
		strLog+=_T("LhaForge is Appending files...\r\n");

		HANDLE hArcFile=CreateFile(ArcFileName,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
		if(INVALID_HANDLE_VALUE==hArcFile){
			strLog+=_T("Error : Failed to open ");
			strLog+=ArcFileName;
			strLog+=_T(" for output.\r\n");
			Ret=-1;
		}
		else{
			std::list<CString>::iterator ite=TemporaryFileList.begin();
			const std::list<CString>::iterator end=TemporaryFileList.end();
			//�e���|�����t�@�C������������
			for(;ite!=end;ite++){
				HANDLE hFile=CreateFile(*ite,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
				int res=UtilAppendFile(hArcFile,hFile);
				if(0!=res){
					if(res<0){	//�������݃G���[
						strLog+=_T("\r\nError : Failed to write file ");
						strLog+=ArcFileName;
						strLog+=_T(".\r\n");
					}
					else{	//�ǂݍ��݃G���[
						strLog+=_T("\r\nError : Failed to read temporary file.\r\n");
					}
					Ret=-1;
					bDeleteOutput=true;
					break;
				}
				CloseHandle(hFile);
				strLog+=_T("o");
			}
			CloseHandle(hArcFile);
			if(!Ret){
				strLog+=_T(":done\r\n");
			}
			if(bDeleteOutput){
				DeleteFile(ArcFileName);
			}
		}
	}

	//�g�����e���|�����t�@�C���͏���
	{
		std::list<CString>::iterator ite=TemporaryFileList.begin();
		const std::list<CString>::iterator end=TemporaryFileList.end();
		for(;ite!=end;ite++){
			DeleteFile(*ite);
		}
	}

	return 0==Ret;
}

bool CArchiverAISH::Extract(LPCTSTR ArcFileName,CConfigManager&,const CConfigExtract &Config,bool,LPCTSTR OutputDir,CString &strLog)
{
	if(!IsOK()){
		return false;
	}

	//===========================
	// DLL�ɓn���I�v�V�����̐ݒ�
	//===========================
	TRACE(_T("DLL�ɓn���I�v�V�����̐ݒ�\n"));

	CString Param;//�R�}���h���C�� �p�����[�^ �o�b�t�@

	//�𓀃p�����[�^�͂Ȃ�
	if(Config.ForceOverwrite){
		//�f�t�H���g�͋����㏑��
	}else{
		Param+=_T("-f -h ");
/*	-f	���łɓ����̓W�J�t�@�C�������݂��Ă����ꍇ�A�ʃt�@�C�����ɂ�
		�Ď��o���܂��B�t�@�C�����̍Ō�Ɋg���q .00 .99 �����܂��B

	-h	-f �I�v�V�����g�p���A�t�@�C�����̍Ō�ɂł͂Ȃ��擪�� XX ����
		���ċ�ʂ��܂��B���̊g���q��ۑ��������ꍇ�Ɏg���܂��B
*/
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

bool CArchiverAISH::ExamineArchive(LPCTSTR ArcFileName,CConfigManager& ConfMan,bool,bool &bInFolder,bool &,CString &BaseDir,CString &strErr)
{
	bInFolder=false;
	return true;
}

//ish/uue�t�@�C���Ƀt�@�C������ȏ�܂܂�邩�ǂ���
bool CArchiverAISH::HasFiles(LPCTSTR ArcFileName)
{
	if(!IsOK()){
		ASSERT(!"Load DLL first!!!");
		return false;
	}

	CString Param;//�R�}���h���C�� �p�����[�^ �o�b�t�@
	Param+=_T("-l ");

	//�A�[�J�C�u�t�@�C�����w��
	Param+=_T("\"");
	Param+=ArcFileName;
	Param+=_T("\" ");

	ASSERT(!Param.IsEmpty());
	TRACE(_T("ArchiveHandler Commandline Parameter:%s\n"),Param);

	TRACE(_T("ArchiveHandler�Ăяo��\n"));
	CString strLog;
	try{
		//char szLog[LOG_BUFFER_SIZE]={0};
		std::vector<char> szLog(LOG_BUFFER_SIZE);
		szLog[0]='\0';
		int Ret=ArchiveHandler(NULL,CT2A(Param),&szLog[0],LOG_BUFFER_SIZE-1);
		strLog=&szLog[0];
		if(Ret)return false;
	}catch(...){
		return false;
	}

	//���O���
	strLog.Replace(_T("\r\n"),_T("\n"));
	int Index=strLog.Find(_T('\n'),0);
	int CopyFrom=0;
	while(true){
		if(-1==Index){
			if(strLog.IsEmpty())return false;
			else return true;
		}
		//�s�̐؂�o��
		if(Index==CopyFrom||Index+1==CopyFrom){
			CopyFrom=Index+1;
			Index=strLog.Find(_T('\n'),Index+1);
			continue;
		}
		return true;
	}
}

BOOL CArchiverAISH::CheckArchive(LPCTSTR _szFileName)
{
	if(!ArchiverCheckArchive){
		ASSERT(ArchiverCheckArchive);
		return false;
	}
	if(!ArchiverCheckArchive(CT2A(_szFileName),CHECKARCHIVE_RAPID))return FALSE;

	//AISH()��CheckArchive�̓t�@�C���̑��݂��m�F���ďI��邾���Ȃ̂ŁA��蔻��𐳊m�ɂ��邽�߂�
	//�A�[�J�C�u���Ƀt�@�C�����܂܂�Ă��邱�Ƃ��m�F����
	if(!HasFiles(_szFileName))return FALSE;
	return TRUE;
}

//�ȉ��̃R�[�h��ish�̌����͏o���邪�A�M�����������Ȃ��ߍ̗p���Ȃ�
//DEL bool CArchiverAISH::TestArchive(LPCSTR ArcFileName)
//DEL {
//DEL 	TRACE(_T("TestArchive() called.\n"));
//DEL 	ASSERT(IsOK());
//DEL 	if(!IsOK()){
//DEL 		return false;
//DEL 	}
//DEL 
//DEL 	//���ۂɓW�J���ăe�X�g����
//DEL 	CString Param=_T(
//DEL 		"-o NUL "		//���ۂɓW�J���邪�A�t�@�C���͕ۑ����Ȃ�
//DEL 
//DEL 		"\""
//DEL 	);
//DEL 	Param+=ArcFileName;
//DEL 	Param+=_T("\"");
//DEL 
//DEL 	//�t�@�C���������O�ɒǉ�
//DEL 	CString Log=ArcFileName;
//DEL 	Log+=_T("\r\n\r\n");
//DEL 	//����ǉ�
//DEL 	Log+=_T(
//DEL 		"Status Examples:\r\n"
//DEL 		"o : OK\r\n"
//DEL 		"e : NG\r\n\r\n"
//DEL 
//DEL 		"----------\r\n"
//DEL 		);
//DEL 
//DEL 	CString Temp;
//DEL 	ArchiveHandler(NULL,Param,Temp.GetBuffer(LOG_BUFFER_SIZE),LOG_BUFFER_SIZE-1);
//DEL 	Temp.ReleaseBuffer();
//DEL 
//DEL 	Log+=Temp;
//DEL 
//DEL 	//���O�\��
//DEL 	CLogDialog LogDlg;
//DEL 	LogDlg.SetData(Log);
//DEL 	LogDlg.DoModal();
//DEL 	return true;
//DEL }
