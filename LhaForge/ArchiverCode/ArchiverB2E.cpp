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
#include "ArchiverB2E.h"
#include "../Utilities/FileOperation.h"
#include "../Utilities/TemporaryDirMgr.h"
#include "../Utilities/StringUtil.h"
#include "../resource.h"
#include "../ConfigCode/ConfigExtract.h"
#include "../ConfigCode/ConfigB2E.h"

CArchiverB2E::CArchiverB2E():
	B2ESetScriptDirectory(NULL),
	B2EScriptGetCount(NULL),
	B2EScriptGetAbility(NULL),
	B2EScriptGetCompressType(NULL),
	B2EScriptGetCompressMethod(NULL),
	B2EScriptGetDefaultCompressMethod(NULL),
	B2EScriptGetExtractorIndex(NULL),
	B2EScriptGetName(NULL)
{
	m_nRequiredVersion=7;
	m_strDllName=_T("B2E32.DLL");
	m_AstrPrefix="B2E";
}

CArchiverB2E::~CArchiverB2E()
{
	FreeDLL();
}

LOAD_RESULT CArchiverB2E::LoadDLL(CConfigManager &ConfMan,CString &strErr)
{
	FreeDLL();

	//���N���X�̃��\�b�h���Ă�
	LOAD_RESULT res=CArchiverDLL::LoadDLL(ConfMan,strErr);
	if(LOAD_RESULT_OK!=res){
		return res;
	}

	//�Ǝ�����API�Q
	B2ESetScriptDirectory=(B2ESETSCRIPTDIRECTORY)GetProcAddress(m_hInstDLL,"B2ESetScriptDirectory");
	if(NULL==B2ESetScriptDirectory){
		strErr.Format(IDS_ERROR_DLL_FUNCTION_GET,m_strDllName,_T("B2ESetScriptDirectory"));
		FreeDLL();
		return LOAD_RESULT_INVALID;
	}

	B2EScriptGetCount=(B2ESCRIPTGETCOUNT)GetProcAddress(m_hInstDLL,"B2EScriptGetCount");
	if(NULL==B2EScriptGetCount){
		strErr.Format(IDS_ERROR_DLL_FUNCTION_GET,m_strDllName,_T("B2EScriptGetCount"));
		FreeDLL();
		return LOAD_RESULT_INVALID;
	}

	B2EScriptGetAbility=(B2ESCRIPTGETABILITY)GetProcAddress(m_hInstDLL,"B2EScriptGetAbility");
	if(NULL==B2EScriptGetAbility){
		strErr.Format(IDS_ERROR_DLL_FUNCTION_GET,m_strDllName,_T("B2EScriptGetAbility"));
		FreeDLL();
		return LOAD_RESULT_INVALID;
	}

	B2EScriptGetCompressType=(B2ESCRIPTGETCOMPRESSTYPE)GetProcAddress(m_hInstDLL,"B2EScriptGetCompressType");
	if(NULL==B2EScriptGetCompressType){
		strErr.Format(IDS_ERROR_DLL_FUNCTION_GET,m_strDllName,_T("B2EScriptGetCompressType"));
		FreeDLL();
		return LOAD_RESULT_INVALID;
	}

	B2EScriptGetCompressMethod=(B2ESCRIPTGETCOMPRESSMETHOD)GetProcAddress(m_hInstDLL,"B2EScriptGetCompressMethod");
	if(NULL==B2EScriptGetCompressMethod){
		strErr.Format(IDS_ERROR_DLL_FUNCTION_GET,m_strDllName,_T("B2EScriptGetCompressMethod"));
		FreeDLL();
		return LOAD_RESULT_INVALID;
	}

	B2EScriptGetDefaultCompressMethod=(B2ESCRIPTGETDEFAULTCOMPRESSMETHOD)GetProcAddress(m_hInstDLL,"B2EScriptGetDefaultCompressMethod");
	if(NULL==B2EScriptGetDefaultCompressMethod){
		strErr.Format(IDS_ERROR_DLL_FUNCTION_GET,m_strDllName,_T("B2EScriptGetDefaultCompressMethod"));
		FreeDLL();
		return LOAD_RESULT_INVALID;
	}

	B2EScriptGetExtractorIndex=(B2ESCRIPTGETEXTRACTORINDEX)GetProcAddress(m_hInstDLL,"B2EScriptGetExtractorIndex");
	if(NULL==B2EScriptGetExtractorIndex){
		strErr.Format(IDS_ERROR_DLL_FUNCTION_GET,m_strDllName,_T("B2EScriptGetExtractorIndex"));
		FreeDLL();
		return LOAD_RESULT_INVALID;
	}

	B2EScriptGetName=(B2ESCRIPTGETNAME)GetProcAddress(m_hInstDLL,"B2EScriptGetName");
	if(NULL==B2EScriptGetName){
		strErr.Format(IDS_ERROR_DLL_FUNCTION_GET,m_strDllName,_T("B2EScriptGetName"));
		FreeDLL();
		return LOAD_RESULT_INVALID;
	}

	// ���[�h����
	//===================
	// �����ݒ�
	CConfigB2E Config;
	Config.load(ConfMan);
	if(_tcslen(Config.ScriptDirectory)>0 && PathIsDirectory(Config.ScriptDirectory)){
		B2ESetScriptDirectory(CT2A(Config.ScriptDirectory));
	}

	return LOAD_RESULT_OK;
}

void CArchiverB2E::FreeDLL()
{
	if(m_hInstDLL){
		B2ESetScriptDirectory=NULL;
		B2EScriptGetCount=NULL;
		B2EScriptGetAbility=NULL;
		B2EScriptGetCompressType=NULL;
		B2EScriptGetCompressMethod=NULL;
		B2EScriptGetDefaultCompressMethod=NULL;
		B2EScriptGetExtractorIndex=NULL;
		CArchiverDLL::FreeDLL();
	}
}

bool CArchiverB2E::Compress(LPCTSTR ArcFileName,std::list<CString> &ParamList,CConfigManager &ConfigManager,const PARAMETER_TYPE,int Options,LPCTSTR lpszFormat,LPCTSTR lpszMethod,LPCTSTR,CString &strLog)
{
	return B2ECompress(ArcFileName,ParamList,ConfigManager,lpszFormat,lpszMethod,(Options & COMPRESS_SFX) !=0 ,strLog);
}

bool CArchiverB2E::B2ECompress(LPCTSTR ArcFileName,std::list<CString> &ParamList,CConfigManager &ConfigManager,LPCTSTR lpszFormat,LPCTSTR lpszMethod,bool bSFX,CString &strLog)
{
	if(!IsOK()||!lpszFormat){
		return false;
	}

	ASSERT(0!=_tcslen(ArcFileName));
	TRACE(_T("ArcFileName=%s\n"),ArcFileName);

	//==============================================
	// ���X�|���X�t�@�C���p�e���|�����t�@�C�����擾
	//==============================================
	TCHAR ResponceFileName[_MAX_PATH+1];
	FILL_ZERO(ResponceFileName);
	if(!UtilGetTemporaryFileName(ResponceFileName,_T("b2e"))){
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

		TRACE(_T("���X�|���X�t�@�C���ւ̏�������\n"));
		//���k��t�@�C������������
		/*
		 * B2E�p�̏o�̓t�@�C�����ɂ́A�g���q.archive���t�������̂��w�肳���B
		 * ����́A�����̃t�@�C��������č폜����Ȃ��悤�ɂ��邽�߂ł���
		 * �����ŉ��߂ė]�v�Ȋg���q���폜����
		 */
		CPath tmpName=ArcFileName;
		tmpName.RemoveExtension();
		WriteResponceFile(hFile,tmpName);

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

	//---���k�p�����[�^
	//���k�`��
	Param =_T("\"-c");
	Param+=lpszFormat;
	Param+=_T("\" ");

	//���k���\�b�h
	if(lpszMethod&&_tcslen(lpszMethod)>0){
		Param+=_T("\"-m");
		Param+=lpszMethod;
		Param+=_T("\" ");
	}

	//���ȉ�
	if(bSFX){
		Param+=_T("-s ");
	}

	//���X�|���X�t�@�C�����w��
	Param+=_T("\"-@");
	Param+=ResponceFileName;
	Param+=_T("\"");

	ASSERT(!Param.IsEmpty());
	TRACE(_T("ArchiveHandler Commandline Parameter:%s\n"),Param);

	TRACE(_T("ArchiveHandler�Ăяo��\n"));

	//---���s
	std::vector<char> szLog(LOG_BUFFER_SIZE);
	szLog[0]='\0';
	int Ret=ArchiveHandler(NULL,CT2A(Param),&szLog[0],LOG_BUFFER_SIZE-1);
	strLog=&szLog[0];

	//�g�������X�|���X�t�@�C���͏���
	DeleteFile(ResponceFileName);

	return 0==Ret;
}

bool CArchiverB2E::Extract(LPCTSTR ArcFileName,CConfigManager&,const CConfigExtract &Config,bool bSafeArchive,LPCTSTR OutputDir,CString &strLog)
{
	if(!IsOK()){
		return false;
	}

	if(!bSafeArchive){
		//�s���ȃp�X������΁A���s���邩�ǂ����m�F����
		//�������A�p�X�`�F�b�N�s�\�ȂƂ��ɂ͂��̌x������o�Ȃ��B
		CString msg;
		msg.Format(IDS_ASK_CONTINUE_EXTRACT,ArcFileName);
		if(IDYES!=MessageBox(NULL,msg,UtilGetMessageCaption(),MB_YESNO|MB_ICONQUESTION|MB_DEFBUTTON2)){
			return false;
		}
	}

	//===========================
	// DLL�ɓn���I�v�V�����̐ݒ�
	//===========================
	CString Param;//�R�}���h���C�� �p�����[�^ �o�b�t�@

	//�𓀃p�����[�^
	Param+=_T("-e ");	//��
	//�����㏑���ɂ͖��Ή�

	//�o�͐�w��
	Param+=_T("\"-o");
	Param+=OutputDir;
	Param+=_T("\" ");

	//�A�[�J�C�u�t�@�C�����w��
	Param+=_T("\"");
	Param+=ArcFileName;
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
// B2EGetFileName()�̏o�͌��ʂ���ɁA�i�[���ꂽ�t�@�C�����p�X
// ���������Ă��邩�ǂ������ʂ��A��d�t�H���_�쐬��h��
//=========================================================
bool CArchiverB2E::ExamineArchive(LPCTSTR ArcFileName,CConfigManager& ConfMan,bool,bool &bInFolder,bool &bSafeArchive,CString &BaseDir,CString &strErr)
{
	if(!IsOK()){
		return false;
	}
/*	UINT uIndex=B2EScriptGetExtractorIndex(ArcFileName);
	if(uIndex==0xFFFFFFFF)return false;	//���Ή��`�����G���[

	WORD wAbility;
	if(!B2EScriptGetAbility(uIndex,&wAbility))return false;
	if(!(wAbility&B2EABILITY_LIST))return false;	//�����ł��Ȃ�*/

	//------
	//�����ł��Ȃ��Ƃ��ɂ́AExamineArchive�����̎|�񍐂��Ă����̂ł���������������K�v�͂Ȃ�

	return _ExamineArchive(ArcFileName,ConfMan,bInFolder,bSafeArchive,0,BaseDir,strErr);
}

bool CArchiverB2E::ExtractSpecifiedOnly(LPCTSTR ArcFileName,CConfigManager&,LPCTSTR OutputDir,std::list<CString> &FileList,CString &strLog,bool bUsePath)
{
	if(!IsOK()){
		return false;
	}

	//==============================================
	// ���X�|���X�t�@�C���p�e���|�����t�@�C�����擾
	//==============================================
	TCHAR ResponceFileName[_MAX_PATH+1];
	FILL_ZERO(ResponceFileName);
	if(!UtilGetTemporaryFileName(ResponceFileName,_T("b2e"))){
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
	Param+=_T("-e ");			//��
	if(!bUsePath)Param+=_T("-g ");		//�p�X�𖳌���

	//�A�[�J�C�u�t�@�C�����w��
	Param+=_T("\"");
	Param+=ArcFileName;
	Param+=_T("\" ");

	//�o�͐�t�H���_
	Param+=_T("\"-o");
	Param+=OutputDir;
	Param+=_T("\" ");

	//���X�|���X�t�@�C�����w��
	Param+=_T("\"-@");
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

bool CArchiverB2E::QueryExtractSpecifiedOnlySupported(LPCTSTR ArcFileName)const
{
	UINT uIndex=B2EScriptGetExtractorIndex(CT2A(ArcFileName));
	if(uIndex==0xFFFFFFFF)return false;	//���Ή��`�����G���[

	WORD wAbility;
	if(!B2EScriptGetAbility(uIndex,&wAbility))return false;
	if(wAbility&B2EABILITY_MELT_EACH)return true;	//�ʉ𓀂ł��Ȃ�
	return false;
}

bool CArchiverB2E::EnumCompressB2EScript(std::vector<B2ESCRIPTINFO> &ScriptInfoArray)
{
	ScriptInfoArray.clear();

	//�X�N���v�g�̐����擾
	const UINT uScriptCount=B2EScriptGetCount();

	if(uScriptCount==-1)return false;	//�G���[

	for(UINT uIndex=0;uIndex<uScriptCount;uIndex++){
		B2ESCRIPTINFO Info;
		Info.uIndex=uIndex;

		//�\�͂��擾
		B2EScriptGetAbility(uIndex,&Info.wAbility);

		if(Info.wAbility&B2EABILITY_COMPRESS){	//---���k�\�͂���
			//���k�`�������擾
			B2EScriptGetCompressType(uIndex,Info.szFormat,_MAX_PATH);
			//���k���\�b�h�����擾
			{
				int i=0;
				char szBuffer[_MAX_PATH+1]={0};
				while(B2EScriptGetCompressMethod(uIndex,i,szBuffer,_MAX_PATH)){
					Info.MethodArray.push_back(szBuffer);
					i++;
				}
			}

			//�f�t�H���g���k���\�b�h�ԍ����擾
			Info.nDefaultMethod=B2EScriptGetDefaultCompressMethod(uIndex);
			//---���ǉ�
			ScriptInfoArray.push_back(Info);
		}
	}
	return true;
}

bool CArchiverB2E::EnumActiveB2EScriptNames(std::vector<CString> &ScriptNames)
{
	ScriptNames.clear();

	//�X�N���v�g�̐����擾
	const UINT uScriptCount=B2EScriptGetCount();

	if(uScriptCount==-1)return false;	//�G���[

	for(UINT uIndex=0;uIndex<uScriptCount;uIndex++){
		B2ESCRIPTINFO Info;
		Info.uIndex=uIndex;

		//�\�͂��擾
		B2EScriptGetAbility(uIndex,&Info.wAbility);

		if(Info.wAbility){	//---�L���ȃX�N���v�g
			char buffer[_MAX_PATH];
			B2EScriptGetName(uIndex,buffer,COUNTOF(buffer));
			ScriptNames.push_back(CString(buffer));
		}
	}
	return true;
}


bool CArchiverB2E::AddItemToArchive(LPCTSTR ArcFileName,bool bEncrypted,const std::list<CString> &FileList,CConfigManager &ConfMan,LPCTSTR lpDestDir,CString &strLog)
{
	// ���X�|���X�t�@�C���p�e���|�����t�@�C�����擾
	TCHAR ResponceFileName[_MAX_PATH+1];
	FILL_ZERO(ResponceFileName);
	if(!UtilGetTemporaryFileName(ResponceFileName,_T("b2e"))){
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
		WriteResponceFile(hFile,_T("*.*"));
		CloseHandle(hFile);
	}

	//===========================
	// DLL�ɓn���I�v�V�����̐ݒ�
	//===========================
	TRACE(_T("DLL�ɓn���I�v�V�����̐ݒ�\n"));

	CString Param;//�R�}���h���C�� �p�����[�^ �o�b�t�@

	//---���k�p�����[�^
	//���k�`��
	Param =_T("-a ");

	//�A�[�J�C�u�t�@�C�����w��
	Param+=_T("\"");
	Param+=ArcFileName;
	Param+=_T("\" ");

	//���X�|���X�t�@�C�����w��
	Param+=_T("\"-@");
	Param+=ResponceFileName;
	Param+=_T("\"");

	ASSERT(!Param.IsEmpty());
	TRACE(_T("ArchiveHandler Commandline Parameter:%s\n"),Param);

	//---���s
	std::vector<char> szLog(LOG_BUFFER_SIZE);
	szLog[0]='\0';
	int Ret=ArchiveHandler(NULL,CT2A(Param),&szLog[0],LOG_BUFFER_SIZE-1);
	strLog=&szLog[0];

	//�g�������X�|���X�t�@�C���͏���
	DeleteFile(ResponceFileName);

	return 0==Ret;
}

bool CArchiverB2E::QueryAddItemToArchiveSupported(LPCTSTR ArcFileName)const
{
	LPCTSTR lpExt=PathFindExtension(ArcFileName);
	if(!lpExt)return false;
	CString strExt=lpExt;
	strExt+=_T('.');	//".ext."�̌`�ɂ���

	const int ScriptCount=B2EScriptGetCount();  //�X�N���v�g�̐����擾

	for(int i=0;i<ScriptCount;i++){
		WORD ability;
		B2EScriptGetAbility(i,&ability);        //�\�͂��擾
		if(ability & B2EABILITY_ADD){
			char Buffer[_MAX_PATH+1];
			B2EScriptGetName(i,Buffer,_MAX_PATH);   //�X�N���v�g�����擾
			CString tmp=Buffer;
			tmp.Replace(_T("#"),_T(""));
			tmp=_T('.')+tmp;
			if(tmp.Find(strExt)!=-1){
				//�Ή��g���q(".ext.")����
				return true;
			}
		}
	}
	return false;
}

bool CArchiverB2E::DeleteItemFromArchive(LPCTSTR ArcFileName,CConfigManager &ConfMan,const std::list<CString> &FileList,CString &strLog)
{
	if(!IsOK()){
		return false;
	}

	//==============================================
	// ���X�|���X�t�@�C���p�e���|�����t�@�C�����擾
	//==============================================
	TCHAR ResponceFileName[_MAX_PATH+1];
	FILL_ZERO(ResponceFileName);
	if(!UtilGetTemporaryFileName(ResponceFileName,_T("b2e"))){
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

	//�p�����[�^
	Param+=_T("-d ");			//�폜

	//�A�[�J�C�u�t�@�C�����w��
	Param+=_T("\"");
	Param+=ArcFileName;
	Param+=_T("\" ");

	//���X�|���X�t�@�C�����w��
	Param+=_T("\"-@");
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

bool CArchiverB2E::QueryDeleteItemFromArchiveSupported(LPCTSTR ArcFileName)const
{
	LPCTSTR lpExt=PathFindExtension(ArcFileName);
	if(!lpExt)return false;
	CString strExt=lpExt;
	strExt+=_T('.');	//".ext."�̌`�ɂ���

	const int ScriptCount=B2EScriptGetCount();  //�X�N���v�g�̐����擾
	
	for(int i=0;i<ScriptCount;i++){
		WORD ability;
		B2EScriptGetAbility(i,&ability);        //�\�͂��擾
		if(ability & B2EABILITY_DELETE){
			char Buffer[_MAX_PATH+1];
			B2EScriptGetName(i,Buffer,_MAX_PATH);   //�X�N���v�g�����擾
			CString tmp=Buffer;
			tmp.Replace(_T("#"),_T(""));
			tmp=_T('.')+tmp;
			if(tmp.Find(strExt)!=-1){
				//�Ή��g���q(".ext.")����
				return true;
			}
		}
	}
	return false;
}
