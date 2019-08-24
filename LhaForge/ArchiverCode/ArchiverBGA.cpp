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
#include "ArchiverBGA.h"
#include "../Utilities/FileOperation.h"
#include "../Utilities/StringUtil.h"
#include "../Utilities/TemporaryDirMgr.h"
#include "../resource.h"
#include "../ConfigCode/ConfigExtract.h"
#include "../ConfigCode/ConfigBGA.h"
#include "../Utilities/OSUtil.h"

CArchiverBGA::CArchiverBGA()
{
	m_LoadLevel=LOAD_DLL_SIMPLE_INSPECTION;
	m_nRequiredVersion=37;
	m_strDllName=_T("Bga32.dll");
	m_AstrPrefix="Bga";
	m_AstrFindParam="*.*";
}

CArchiverBGA::~CArchiverBGA()
{ 
	FreeDLL();
}

bool CArchiverBGA::Compress(LPCTSTR ArcFileName,std::list<CString> &ParamList,CConfigManager &ConfMan,const PARAMETER_TYPE Type,int Options,LPCTSTR,LPCTSTR,LPCTSTR lpszLevel,CString &strLog)
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
	if(!UtilGetTemporaryFileName(ResponceFileName,_T("bga"))){
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
		//2�i�K�쐬����
		if(!UtilGetTemporaryFileName(SFXTemporaryFileName,_T("sfx"))){
			strLog=CString(MAKEINTRESOURCE(IDS_ERROR_TEMPORARY_FILE_CREATE));
			return false;
		}
		ASSERT(0!=_tcslen(SFXTemporaryFileName));
	}

	//====================================================
	// ���X�|���X�t�@�C�����Ɉ��k�Ώۃt�@�C�������L������
	//====================================================
	{
		HANDLE hFile=CreateFile(ResponceFileName,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
		if(INVALID_HANDLE_VALUE==hFile){
			strLog=CString(MAKEINTRESOURCE(IDS_ERROR_TEMPORARY_FILE_ACCESS));
			return false;
		}

		TRACE(_T("���X�|���X�t�@�C���ւ̏�������\n"));
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
		_T("a ")		//���k
		_T("-a ")		//�S������Ώ�
	;

	CConfigBGA Config;
	Config.load(ConfMan);
	if(lpszLevel && *lpszLevel!=_T('\0')){
		switch(Type){
		case PARAMETER_GZA:
			Param.AppendFormat(_T("-m1 -l%s "),lpszLevel);
			break;
		case PARAMETER_BZA:
			Param.AppendFormat(_T("-m2 -l%s "),lpszLevel);
			break;
		}
	}else{
		switch(Type){
		case PARAMETER_GZA:
			Param.AppendFormat(_T("-m1 -l%d "),Config.GZALevel);
			break;
		case PARAMETER_BZA:
			Param.AppendFormat(_T("-m2 -l%d "),Config.BZALevel);
			break;
		}
	}

	//�o�͐�t�@�C�����w��
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
	//char szLog[LOG_BUFFER_SIZE]={0};
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
	Param=_T("s ");		//�ϊ�

	Param+=_T("\"");
	Param+=SFXTemporaryFileName;	//�ϊ����t�@�C����
	Param+=_T("\" ");

	//------------------------------------------------------------------------------------------
	//����:
	// ���ȉ𓀃t�@�C���̃t�@�C�����̓e���|�����t�@�C�����̊g���q��.exe�ɕς������̂ɂȂ��Ă���
	//------------------------------------------------------------------------------------------

	//�o�͐�t�H���_(�e���|�����t�H���_)���w��
	CString strDir(SFXTemporaryFileName);
	UtilPathGetDirectoryPart(strDir);
	Param+=_T("\"");
	Param+=strDir;
	Param+=_T("\" ");


	ASSERT(!Param.IsEmpty());
	TRACE(_T("ArchiveHandler Commandline Parameter(SFX):%s\n"),Param);

	TRACE(_T("SFX�pArchiveHandler�Ăяo��\n"));
	std::vector<char> LogSFX(LOG_BUFFER_SIZE);
	LogSFX[0]='\0';
	Ret=ArchiveHandler(NULL,CT2A(Param),&LogSFX[0],LOG_BUFFER_SIZE-1);
	strLog+=&LogSFX[0];

	DeleteFile(SFXTemporaryFileName);
	if(!Ret){	//����I����
		//�ϊ���̎��ȉ𓀃t�@�C�������o��
		PathRemoveExtension(SFXTemporaryFileName);
		PathAddExtension(SFXTemporaryFileName,_T(".exe"));
		MoveFile(SFXTemporaryFileName,ArcFileName);
	}

	return 0==Ret;
}

bool CArchiverBGA::Extract(LPCTSTR ArcFileName,CConfigManager&,const CConfigExtract &Config,bool bSafeArchive,LPCTSTR OutputDir,CString &strLog)
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
	Param+=
		_T("x ")		//�p�X�t����
		_T("-a ")		//�S������
	;
	if(Config.ForceOverwrite){
		//�����㏑��
		Param+=_T("-o ");
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
	std::vector<char> szLog(LOG_BUFFER_SIZE);
	szLog[0]='\0';
	int Ret=ArchiveHandler(NULL,CT2A(Param),&szLog[0],LOG_BUFFER_SIZE-1);
	strLog=&szLog[0];

	return 0==Ret;
}



//=========================================================
// BgaGetFileName()�̏o�͌��ʂ���ɁA�i�[���ꂽ�t�@�C�����p�X
//���������Ă��邩�ǂ������ʂ��A��d�t�H���_�쐬��h��
// ���S�m�F���s��
//=========================================================
bool CArchiverBGA::ExamineArchive(LPCTSTR ArcFileName,CConfigManager& ConfMan,bool,bool &bInFolder,bool &bSafeArchive,CString &BaseDir,CString &strErr)
{
	return _ExamineArchive(ArcFileName,ConfMan,bInFolder,bSafeArchive,-1,BaseDir,strErr);
}

bool CArchiverBGA::ExtractSpecifiedOnly(LPCTSTR ArcFileName,CConfigManager&,LPCTSTR OutputDir,std::list<CString> &FileList,CString &strLog,bool bUsePath)
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
	if(!UtilGetTemporaryFileName(ResponceFileName,_T("bga"))){
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
		_T("x ")			//��
		_T("-a ")			//�S������
	;
	if(!bUsePath)Param+=_T("-j ");		//�p�X����
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
	std::vector<char> szLog(LOG_BUFFER_SIZE);
	szLog[0]='\0';
	int Ret=ArchiveHandler(NULL,CT2A(Param),&szLog[0],LOG_BUFFER_SIZE-1);
	strLog=&szLog[0];
	//�g�������X�|���X�t�@�C���͏���
	DeleteFile(ResponceFileName);

	return 0==Ret;
}

bool CArchiverBGA::DeleteItemFromArchive(LPCTSTR ArcFileName,CConfigManager&,const std::list<CString> &FileList,CString &strLog)
{
	if(!IsOK()){
		return false;
	}

	//==============================================
	// ���X�|���X�t�@�C���p�e���|�����t�@�C�����擾
	//==============================================
	TCHAR ResponceFileName[_MAX_PATH+1];
	FILL_ZERO(ResponceFileName);
	if(!UtilGetTemporaryFileName(ResponceFileName,_T("bga"))){
		strLog=CString(MAKEINTRESOURCE(IDS_ERROR_TEMPORARY_FILE_CREATE));
		return false;
	}
	ASSERT(0!=_tcslen(ResponceFileName));

	//�폜�Ώۃt�@�C�������X�|���X�t�@�C���ɏ����o��
	{
		HANDLE hFile=CreateFile(ResponceFileName,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
		if(INVALID_HANDLE_VALUE==hFile){
			strLog=CString(MAKEINTRESOURCE(IDS_ERROR_TEMPORARY_FILE_ACCESS));
			return false;
		}

		std::list<CString>::const_iterator ite=FileList.begin();
		const std::list<CString>::const_iterator end=FileList.end();
		for(;ite!=end;ite++){
			WriteResponceFile(hFile,*ite);
		}
		CloseHandle(hFile);
	}

	//===========================
	// DLL�ɓn���I�v�V�����̐ݒ�
	//===========================
	CString Param;

	//�폜�p�����[�^
	Param+=
		_T("d ")		//�폜
		_T("-i ")		//�m�F���Ȃ�
	;
	//�A�[�J�C�u�t�@�C�����w��
	Param+=_T("\"");
	Param+=ArcFileName;
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

bool CArchiverBGA::AddItemToArchive(LPCTSTR ArcFileName,bool bEncrypted,const std::list<CString> &FileList,CConfigManager &ConfMan,LPCTSTR lpDestDir,CString &strLog)
{
	// ���X�|���X�t�@�C���p�e���|�����t�@�C�����擾
	TCHAR ResponceFileName[_MAX_PATH+1];
	FILL_ZERO(ResponceFileName);
	if(!UtilGetTemporaryFileName(ResponceFileName,_T("bga"))){
		strLog=CString(MAKEINTRESOURCE(IDS_ERROR_TEMPORARY_FILE_CREATE));
		return false;
	}
	ASSERT(0!=_tcslen(ResponceFileName));

	//===�ꎞ�I�Ƀt�@�C�����R�s�[
	//---\�ŏI����_�p�X���擾
	CPath strBasePath;
	UtilGetBaseDirectory(strBasePath,FileList);
	TRACE(_T("%s\n"),strBasePath);

	//---�e���|�����ɑΏۃt�@�C�����R�s�[
	//�e���|��������
	CTemporaryDirectoryManager tdm(_T("lhaf"));
	CPath strDestPath(tdm.GetDirPath());
	strDestPath+=lpDestDir;
	UtilMakeSureDirectoryPathExists(strDestPath);

	// ���k�Ώۃt�@�C�������C������
	const int BasePathLength=((CString)strBasePath).GetLength();
	CString strSrcFiles;	//�R�s�[���t�@�C���̈ꗗ
	CString strDestFiles;	//�R�s�[��t�@�C���̈ꗗ
	std::list<CString>::const_iterator ite;
	for(ite=FileList.begin();ite!=FileList.end();++ite){
		//�x�[�X�p�X�����ɑ��΃p�X�擾 : ���ʂł�����p�X�̕������������J�b�g����
		LPCTSTR lpSrc((LPCTSTR)(*ite)+BasePathLength);

		//���葤�t�@�C�����w��
		strSrcFiles+=(strBasePath+lpSrc);	//PathAppend����
		strSrcFiles+=_T('|');
		//�󂯑��t�@�C�����w��
		strDestFiles+=strDestPath+lpSrc;
		strDestFiles+=_T('|');
	}
	strSrcFiles+=_T('|');
	strDestFiles+=_T('|');

	//'|'��'\0'�ɕϊ�����
	std::vector<TCHAR> srcBuf(strSrcFiles.GetLength()+1);
	UtilMakeFilterString(strSrcFiles,&srcBuf[0],srcBuf.size());
	std::vector<TCHAR> destBuf(strDestFiles.GetLength()+1);
	UtilMakeFilterString(strDestFiles,&destBuf[0],destBuf.size());

	//�t�@�C��������e
	SHFILEOPSTRUCT fileOp={0};
	fileOp.wFunc=FO_COPY;
	fileOp.fFlags=FOF_MULTIDESTFILES|FOF_NOCONFIRMATION|FOF_NOCONFIRMMKDIR|FOF_NOCOPYSECURITYATTRIBS|FOF_NO_CONNECTED_ELEMENTS;
	fileOp.pFrom=&srcBuf[0];
	fileOp.pTo=&destBuf[0];

	//�R�s�[���s
	if(::SHFileOperation(&fileOp)){
		//�G���[
		strLog=CString(MAKEINTRESOURCE(IDS_ERROR_FILE_COPY));
		return false;
	}else if(fileOp.fAnyOperationsAborted){
		//�L�����Z��
		strLog=CString(MAKEINTRESOURCE(IDS_ERROR_USERCANCEL));
		return false;
	}

	//�J�����g�f�B���N�g���ݒ�
	::SetCurrentDirectory(tdm.GetDirPath());
	// �����ɁA���X�|���X�t�@�C�����ɃA�[�J�C�u������ш��k�Ώۃt�@�C�������L������
	{
		HANDLE hFile=CreateFile(ResponceFileName,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
		if(INVALID_HANDLE_VALUE==hFile){
			strLog=CString(MAKEINTRESOURCE(IDS_ERROR_TEMPORARY_FILE_ACCESS));
			return false;
		}
		//���X�|���X�t�@�C���ւ̏�������
		//�S�Ĉ��k
		WriteResponceFile(hFile,_T("*"));
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
		_T("-a ")		//�S������Ώ�
	;

	//�t�@�C���̎�ނɉ������p�����[�^���w��
	CConfigBGA Config;
	Config.load(ConfMan);
	CString level;
	switch(CheckArchive(ArcFileName)){
	case 1:	//GZA
		TRACE(_T("This is GZA\n"));
		Param+=_T("-m1 ");
		level.Format(_T("-l%d "),Config.GZALevel);
		Param+=level;
		break;
	case 2:	//BZA
		TRACE(_T("This is BZA\n"));
		Param+=_T("-m2 ");
		level.Format(_T("-l%d "),Config.BZALevel);
		Param+=level;
		break;
	default:
		ASSERT(!"This code cannot be run");
		//�G���[�������ʓ|�Ȃ̂ŕ����Ă����B�ʂɎw�肪�����Ă�������x�͓����̂ŁB
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
	std::vector<char> szLog(LOG_BUFFER_SIZE);
	szLog[0]='\0';
	int Ret=ArchiveHandler(NULL,CT2A(Param),&szLog[0],LOG_BUFFER_SIZE-1);
	strLog=&szLog[0];

	//�g�������X�|���X�t�@�C���͏���
	DeleteFile(ResponceFileName);

	return 0==Ret;
}
