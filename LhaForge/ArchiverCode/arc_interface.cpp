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
#include "arc_interface.h"
#include "../Utilities/FileOperation.h"
#include "../Utilities/OSUtil.h"

CArchiverDLL::CArchiverDLL():
	m_hInspectArchive(NULL),
	m_bInspectFirstTime(false),
	m_dwInspectMode(0),
	m_nRequiredVersion(0),
	m_nRequiredSubVersion(0),
	m_LoadLevel(LOAD_DLL_STANDARD),
	m_AstrFindParam("*"),
	m_hInstDLL(NULL),
	ArchiveHandler(NULL),
	ArchiverGetVersion(NULL),
	ArchiverGetSubVersion(NULL),
	ArchiverCheckArchive(NULL),
	ArchiverGetFileCount(NULL),
	ArchiverQueryFunctionList(NULL),
	ArchiverGetFileName(NULL),
	ArchiverOpenArchive(NULL),
	ArchiverCloseArchive(NULL),
	ArchiverFindFirst(NULL),
	ArchiverFindNext(NULL),
	ArchiverGetAttribute(NULL),
	ArchiverGetOriginalSizeEx(NULL),
	ArchiverGetCompressedSizeEx(NULL),
	ArchiverGetWriteTime(NULL),
	ArchiverGetWriteTimeEx(NULL),
	ArchiverGetMethod(NULL)
{
}

WORD CArchiverDLL::GetVersion()const
{
	if(!ArchiverGetVersion){
		ASSERT(ArchiverGetVersion);
		return 0;
	}
	return ArchiverGetVersion();
}

WORD CArchiverDLL::GetSubVersion()const
{
	if(!ArchiverGetSubVersion){
//		ASSERT(ArchiverGetSubVersion);
		return 0;
	}
	return ArchiverGetSubVersion();
}

bool CArchiverDLL::GetVersionString(CString &String)const
{
	if(!m_hInstDLL||!ArchiverGetVersion){
		String.LoadString(IDS_MESSAGE_DLL_NOT_AVAILABLE);
		return false;
	}
	if(!ArchiverGetSubVersion){
		//used sprintf
		String.Format(_T("Ver.%.2f"),ArchiverGetVersion()/100.0f);
	}
	else{
		String.Format(_T("Ver.%.2f.%.2f"),ArchiverGetVersion()/100.0f,ArchiverGetSubVersion()/100.0f);
	}
	return true;
}

BOOL CArchiverDLL::CheckArchive(LPCTSTR _szFileName)
{
	if(!ArchiverCheckArchive){
		ASSERT(ArchiverCheckArchive);
		return false;
	}
	return ArchiverCheckArchive(CT2A(_szFileName),CHECKARCHIVE_BASIC);
}

int CArchiverDLL::GetFileCount(LPCTSTR _szFileName)
{
	if(!ArchiverGetFileCount){
		return -1;
	}
	return ArchiverGetFileCount(CT2A(_szFileName));
}

bool CArchiverDLL::_ExamineArchive(LPCTSTR ArcFileName,CConfigManager& ConfMan,bool &bInFolder,bool &bSafeArchive,const int MaxRepeat,CString &BaseDir,CString &strErr)
{
	ASSERT(IsOK());
	if(!IsOK()){
		return false;
	}

	if(!InspectArchiveBegin(ArcFileName,ConfMan)){
		strErr.Format(IDS_ERROR_OPEN_ARCHIVE,ArcFileName);
		return false;
	}

	bInFolder=true;
	bool bSureDir=false;	//BaseDir�ɓ����Ă��镶���񂪊m���Ƀt�H���_�ł���Ȃ�true
	TRACE(_T("========\n"));

	while(InspectArchiveNext()){
		CString Buffer;
		InspectArchiveGetFileName(Buffer);
		Buffer.Replace(_T('\\'),_T('/'));		//�p�X��؂蕶���̒u������
		TRACE(_T("%s\n"),Buffer);

		/*
		Separator('/' or '\')�͊i�[�t�@�C���̐擪�ɂ�����܂܂�Ă��Ă��������ׂ��ł���̂ŁA
		�i�[�t�@�C�����̐擪�ɂ�����Separator�������Ă��t�H���_�Ɋi�[���ꂽ��ԂƂ͌��Ȃ��Ȃ��B
		Separator��MaxRepeat��葽���ƕs���Ƃ���
		�������AMaxRepeat��-1�̂Ƃ��̓G���[�Ƃ͂��Ȃ�
		*/
		const int Length=Buffer.GetLength();
		int StartIndex=0;
		for(;StartIndex<Length;StartIndex++){
			//�擪��'/'���Ƃ΂��Ă���
#if defined(_UNICODE)||defined(UNICODE)
			if(_T('/')!=Buffer[StartIndex])break;
#else
			if(_MBC_SINGLE==_mbsbtype((const unsigned char *)(LPCTSTR)Buffer,StartIndex)){
				if(_T('/')!=Buffer[StartIndex])break;
			}
			else{	//�S�p�����Ȃ�'/'�ł���͂����Ȃ�
				break;
			}
#endif//defined(_UNICODE)||defined(UNICODE)
			if(-1!=MaxRepeat){
				if(StartIndex>=MaxRepeat){	//'/'��MaxRepeat�ȏ㑱���ꍇ
					//�댯�ȃt�@�C���ƕ��������̂Ŋč��I��
					InspectArchiveEnd();
					bSafeArchive=false;
					bInFolder=false;
					return true;
				}
			}
		}
		if((-1!=Buffer.Find(_T("../"),StartIndex))||	//���΃p�X�w�肪������΁A����͈��S�ȃt�@�C���ł͂Ȃ�
			(-1!=Buffer.Find(_T(':'),StartIndex))){	//�h���C�u����������΁A����͈��S�ȃt�@�C���ł͂Ȃ�
			//�댯�ȃt�@�C���ƕ��������̂Ŋč��I��
			InspectArchiveEnd();
			bSafeArchive=false;
			bInFolder=false;
			return true;
		}

		//��������͓�d�f�B���N�g������
		//���łɓ�d�f�B���N�g�����肪�t���Ă���ꍇ�͈��S����݂̂ɓO����

		int FoundIndex=0;
		while(bInFolder){
			FoundIndex=Buffer.Find(_T('/'),StartIndex);
			if(-1==FoundIndex){	//'/'���i�[�t�@�C�����̐擪�ȊO�Ɋ܂܂�Ȃ��ꍇ
				if(!BaseDir.IsEmpty()&&BaseDir==Buffer){
					bSureDir=true;	//BaseDir���t�H���_�ł���Ɗm�F���ꂽ
					break;
				}
				else if(BaseDir.IsEmpty()){
					//�t�H���_���̌���'/'���t���Ȃ��A�[�J�C�o������
					//�����������̂��ŏ��ɏo�Ă����Ƃ��́A�t�H���_���Ɖ��肷��
					BaseDir=Buffer;
					bSureDir=false;
					break;
				}
			}
			CString Dir=Buffer.Mid(StartIndex,FoundIndex-StartIndex);	//Separator�̑O�܂ł̕�����(�f�B���N�g���ɑ���)�𔲂��o���Ă���
			//����܂ł̒��ׂ�Dir��Empty�ł͂Ȃ����Ƃ��ۏ؂���Ă���
			//�܂��A�댯�ł͂Ȃ����Ƃ��������Ă���
			TRACE(_T("Base=%s,Dir=%s\n"),BaseDir,Dir);

			if(_T('.')==Dir){	//./������΃f�B���N�g���w��Ƃ��Ă͖�������
				StartIndex=FoundIndex+1;
				continue;
			}
			if(BaseDir.IsEmpty()){
				BaseDir=Dir;
				bSureDir=true;
			}
			else if(BaseDir!=Dir){
				bInFolder=false;
			}
			else bSureDir=true;	//BaseDir���f�B���N�g���Ɗm�F���ꂽ
			break;
		}
	}
	TRACE(_T("========\n"));

	InspectArchiveEnd();
	bSafeArchive=true;

	//�t�H���_�ɓ����Ă���悤�ł͂��邪�A�f�B���N�g���Ɖ��肳�ꂽ�����̏ꍇ
	if(bInFolder&&!bSureDir)bInFolder=false;
	return true;
}

bool CArchiverDLL::_ExamineArchiveFast(LPCTSTR ArcFileName,CConfigManager& ConfMan,bool &bInFolder,CString &BaseDir,CString &strErr)
{
	//�s���ȃp�X�����t�@�C���͑S��DLL�����Ώ����Ă���Ă���ƍl����̂ŁA�S�ẴA�[�J�C�u�͈��S�ł���Ƃ���

	TRACE(_T("---_ExamineArchiveFast() called.\n"));
	ASSERT(IsOK());
	if(!IsOK()){
		return false;
	}

	if(!InspectArchiveBegin(ArcFileName,ConfMan)){
		strErr.Format(IDS_ERROR_OPEN_ARCHIVE,ArcFileName);
		return false;
	}

	bool bSureDir=false;	//BaseDir�ɓ����Ă��镶���񂪊m���Ƀt�H���_�ł���Ȃ�true
	bInFolder=true;
	TRACE(_T("========\n"));
	while(InspectArchiveNext()){
		CString Buffer;
		InspectArchiveGetFileName(Buffer);
		Buffer.Replace(_T('\\'),_T('/'));		//�p�X��؂蕶���̒u������
		TRACE(_T("%s\n"),Buffer);

		/*
		Separator�͊i�[�t�@�C���̐擪�ɂ�����܂܂�Ă��Ă��������ׂ��ł���̂ŁA
		�i�[�t�@�C�����̐擪�ɂ�����/�������Ă��t�H���_�Ɋi�[���ꂽ��ԂƂ͌��Ȃ��Ȃ��B
		*/
		const int Length=Buffer.GetLength();
		int StartIndex=0;
		for(;StartIndex<Length;StartIndex++){
			//�擪��Separator���Ƃ΂��Ă���
#if defined(_UNICODE)||defined(UNICODE)
			if(_T('/')!=Buffer[StartIndex])break;
#else
			if(_MBC_SINGLE==_mbsbtype((const unsigned char *)(LPCTSTR)Buffer,StartIndex)){
				if(_T('/')!=Buffer[StartIndex])break;
			}
			else{	//�S�p�����Ȃ�Separator�ł���͂����Ȃ�
				break;
			}
#endif//defined(_UNICODE)||defined(UNICODE)
		}

		int FoundIndex=0;
		while(bInFolder){
			FoundIndex=Buffer.Find(_T('/'),StartIndex);
			if(-1==FoundIndex){	//'/'���i�[�t�@�C�����̐擪�ȊO�Ɋ܂܂�Ȃ��ꍇ
				if(!BaseDir.IsEmpty()&&BaseDir==Buffer){
					bSureDir=true;	//BaseDir���t�H���_�ł���Ɗm�F���ꂽ
					break;
				}
				else if(BaseDir.IsEmpty()){
					//�t�H���_���̌���'/'���t���Ȃ��A�[�J�C�o������
					//�����������̂��ŏ��ɏo�Ă����Ƃ��́A�t�H���_���Ɖ��肷��
					BaseDir=Buffer;
					bSureDir=false;
					break;
				}
			}
			CString Dir=Buffer.Mid(StartIndex,FoundIndex-StartIndex);	//Separator�̑O�܂ł̕�����(�f�B���N�g���ɑ���)�𔲂��o���Ă���
			//����܂ł̒��ׂ�Dir��Empty�ł͂Ȃ����Ƃ��ۏ؂���Ă���
			//�܂��A�댯�ł͂Ȃ����Ƃ��������Ă���
			TRACE(_T("Base=%s,Dir=%s\n"),BaseDir,Dir);

			if(_T('.')==Dir){	//./������΃f�B���N�g���w��Ƃ��Ă͖�������
				StartIndex=FoundIndex+1;
				continue;
			}
			if(BaseDir.IsEmpty()){
				BaseDir=Dir;
				bSureDir=true;
			}
			else if(BaseDir!=Dir){
				bInFolder=false;
			}
			else bSureDir=true;	//BaseDir���f�B���N�g���Ɗm�F���ꂽ
			break;
		}
	}
	TRACE(_T("========\n"));

	InspectArchiveEnd();
	//�t�H���_�ɓ����Ă���悤�ł͂��邪�A�f�B���N�g���Ɖ��肳�ꂽ�����̏ꍇ
	if(bInFolder&&!bSureDir)bInFolder=false;
	return true;
}

void CArchiverDLL::FreeDLL()
{
	ASSERT(NULL==m_hInspectArchive);
	ArchiveHandler				=NULL;
	ArchiverGetVersion			=NULL;
	ArchiverGetSubVersion		=NULL;
	ArchiverCheckArchive		=NULL;
	ArchiverGetFileCount		=NULL;
	ArchiverGetFileName			=NULL;
	ArchiverQueryFunctionList	=NULL;
	ArchiverOpenArchive			=NULL;
	ArchiverCloseArchive		=NULL;
	ArchiverFindFirst			=NULL;
	ArchiverFindNext			=NULL;
	ArchiverGetAttribute		=NULL;
	ArchiverGetOriginalSizeEx	=NULL;
	ArchiverGetCompressedSizeEx	=NULL;
	ArchiverGetWriteTime		=NULL;
	ArchiverGetWriteTimeEx		=NULL;
	ArchiverGetMethod			=NULL;

	if(m_hInstDLL){
		FreeLibrary(m_hInstDLL);
		m_hInstDLL=NULL;
	}
}

LOAD_RESULT CArchiverDLL::LoadDLL(CConfigManager&,CString &strErr)
{
	FreeDLL();

	//���S�ȃp�X�Ɉړ�;DLL�ǂݍ��ݑ΍�
	CCurrentDirManager cdm(UtilGetModuleDirectoryPath());

	if(NULL==(m_hInstDLL=LoadLibrary(m_strDllName))){
		strErr.Format(IDS_ERROR_DLL_LOAD,m_strDllName);
		TRACE(_T("%s\n"),(LPCTSTR)strErr);
		return LOAD_RESULT_NOT_FOUND;
	}

	ArchiveHandler=(COMMON_ARCHIVER_HANDLER)GetProcAddress(m_hInstDLL,m_AstrPrefix);
	if(NULL==ArchiveHandler){
		strErr.Format(IDS_ERROR_DLL_FUNCTION_GET,m_strDllName,(LPCTSTR)CA2T(m_AstrPrefix));
		FreeDLL();
		TRACE(_T("%s\n"),(LPCTSTR)strErr);
		return LOAD_RESULT_INVALID;
	}

	CStringA FunctionName=m_AstrPrefix;
	FunctionName+="GetVersion";
	ArchiverGetVersion=(COMMON_ARCHIVER_GETVERSION)GetProcAddress(m_hInstDLL,FunctionName);
	if(NULL==ArchiverGetVersion){
		strErr.Format(IDS_ERROR_DLL_FUNCTION_GET,m_strDllName,(LPCTSTR)CA2T(FunctionName));
		FreeDLL();
		TRACE(_T("%s\n"),(LPCTSTR)strErr);
		return LOAD_RESULT_INVALID;
	}

	//�T�u�o�[�W�����̎擾�ł���DLL�͌����Ă���
	FunctionName=m_AstrPrefix;
	FunctionName+="GetSubVersion";
	ArchiverGetSubVersion=(COMMON_ARCHIVER_GETVERSION)GetProcAddress(m_hInstDLL,FunctionName);
	if(NULL==ArchiverGetSubVersion){
	//���[�h�Ɏ��s���Ă��G���[�Ƃ͂��Ȃ�
		TRACE(_T("Failed to Load Function %s\n"),(LPCTSTR)CA2T(FunctionName));
	}
	//-------------------------
	// DLL�o�[�W�����̃`�F�b�N
	//-------------------------
	if((ArchiverGetVersion()<m_nRequiredVersion)){
		//���W���[�o�[�W�������Ⴂ
		strErr.Format(IDS_ERROR_DLL_TOO_OLD,m_strDllName);
		FreeDLL();
		TRACE(_T("%s\n"),(LPCTSTR)strErr);
		return LOAD_RESULT_TOO_OLD;
	}else if((ArchiverGetVersion()==m_nRequiredVersion)&&(NULL!=ArchiverGetSubVersion)&&(ArchiverGetSubVersion()<m_nRequiredSubVersion)){
		//�}�C�i�[�o�[�W�������Ⴂ
		strErr.Format(IDS_ERROR_DLL_TOO_OLD,(LPCTSTR)m_strDllName);
		FreeDLL();
		TRACE(_T("%s\n"),(LPCTSTR)strErr);
		return LOAD_RESULT_TOO_OLD;
	}

	FunctionName=m_AstrPrefix;
	FunctionName+="CheckArchive";
	ArchiverCheckArchive=(COMMON_ARCHIVER_CHECKARCHIVE)GetProcAddress(m_hInstDLL,FunctionName);
	if(NULL==ArchiverCheckArchive){
		strErr.Format(IDS_ERROR_DLL_FUNCTION_GET,m_strDllName,(LPCTSTR)CA2T(FunctionName));
		FreeDLL();
		TRACE(_T("%s\n"),(LPCTSTR)strErr);
		return LOAD_RESULT_INVALID;
	}

	//XacRett.dll��Aish32.dll�������T�|�[�g���Ă��Ȃ�
	FunctionName=m_AstrPrefix;
	FunctionName+="GetFileCount";
	ArchiverGetFileCount=(COMMON_ARCHIVER_GETFILECOUNT)GetProcAddress(m_hInstDLL,FunctionName);
	if(NULL==ArchiverGetFileCount){
	//���[�h�Ɏ��s���Ă��G���[�Ƃ͂��Ȃ�
		TRACE(_T("Failed to Load Function %s\n"),(LPCTSTR)CA2T(FunctionName));
	}

	if(LOAD_DLL_MINIMUM==m_LoadLevel){
		TRACE(_T("Minimum Function Set Load\n"));
		return LOAD_RESULT_OK;
	}

	//------------
	// �ǉ��֐��Q
	//------------
	FunctionName=m_AstrPrefix;
	FunctionName+="QueryFunctionList";
	ArchiverQueryFunctionList=(COMMON_ARCHIVER_QUERYFUNCTIONLIST)GetProcAddress(m_hInstDLL,FunctionName);
	if(NULL==ArchiverQueryFunctionList){
		strErr.Format(IDS_ERROR_DLL_FUNCTION_GET,m_strDllName,(LPCTSTR)CA2T(FunctionName));
		FreeDLL();
		TRACE(_T("%s\n"),(LPCTSTR)strErr);
		return LOAD_RESULT_INVALID;
	}

	FunctionName=m_AstrPrefix;
	FunctionName+="OpenArchive";
	if(!ArchiverQueryFunctionList(23)){//23:???OpenArchive
		strErr.Format(IDS_ERROR_UNAVAILABLE_API,(LPCTSTR)CA2T(FunctionName));
		FreeDLL();
		TRACE(_T("%s\n"),(LPCTSTR)strErr);
		return LOAD_RESULT_INVALID;
	}
	ArchiverOpenArchive=(COMMON_ARCHIVER_OPENARCHIVE)GetProcAddress(m_hInstDLL,FunctionName);
	if(NULL==ArchiverOpenArchive){
		strErr.Format(IDS_ERROR_DLL_FUNCTION_GET,m_strDllName,(LPCTSTR)CA2T(FunctionName));
		FreeDLL();
		TRACE(_T("%s\n"),(LPCTSTR)strErr);
		return LOAD_RESULT_INVALID;
	}

	FunctionName=m_AstrPrefix;
	FunctionName+="CloseArchive";
	if(!ArchiverQueryFunctionList(24)){//24:???CloseArchive
		strErr.Format(IDS_ERROR_UNAVAILABLE_API,(LPCTSTR)CA2T(FunctionName));
		FreeDLL();
		TRACE(_T("%s\n"),(LPCTSTR)strErr);
		return LOAD_RESULT_INVALID;
	}
	ArchiverCloseArchive=(COMMON_ARCHIVER_CLOSEARCHIVE)GetProcAddress(m_hInstDLL,FunctionName);
	if(NULL==ArchiverCloseArchive){
		strErr.Format(IDS_ERROR_DLL_FUNCTION_GET,m_strDllName,(LPCTSTR)CA2T(FunctionName));
		FreeDLL();
		TRACE(_T("%s\n"),(LPCTSTR)strErr);
		return LOAD_RESULT_INVALID;
	}

	//FindFirst
	FunctionName=m_AstrPrefix;
	FunctionName+="FindFirst";
	if(!ArchiverQueryFunctionList(25)){//25:???FindFirst
		strErr.Format(IDS_ERROR_UNAVAILABLE_API,(LPCTSTR)CA2T(FunctionName));
		FreeDLL();
		TRACE(_T("%s\n"),(LPCTSTR)strErr);
		return LOAD_RESULT_INVALID;
	}
	ArchiverFindFirst=(COMMON_ARCHIVER_FINDFIRST)GetProcAddress(m_hInstDLL,FunctionName);
	if(NULL==ArchiverFindFirst){
		strErr.Format(IDS_ERROR_DLL_FUNCTION_GET,m_strDllName,(LPCTSTR)CA2T(FunctionName));
		FreeDLL();
		TRACE(_T("%s\n"),(LPCTSTR)strErr);
		return LOAD_RESULT_INVALID;
	}

	//FindNext
	FunctionName=m_AstrPrefix;
	FunctionName+="FindNext";
	if(!ArchiverQueryFunctionList(26)){//26:???FindNext
		strErr.Format(IDS_ERROR_UNAVAILABLE_API,(LPCTSTR)CA2T(FunctionName));
		FreeDLL();
		TRACE(_T("%s\n"),(LPCTSTR)strErr);
		return LOAD_RESULT_INVALID;
	}
	ArchiverFindNext=(COMMON_ARCHIVER_FINDNEXT)GetProcAddress(m_hInstDLL,FunctionName);
	if(NULL==ArchiverFindNext){
		strErr.Format(IDS_ERROR_DLL_FUNCTION_GET,m_strDllName,(LPCTSTR)CA2T(FunctionName));
		FreeDLL();
		TRACE(_T("%s\n"),(LPCTSTR)strErr);
		return LOAD_RESULT_INVALID;
	}

	if(LOAD_DLL_SIMPLE_INSPECTION==m_LoadLevel){
		TRACE(_T("Simple Inspection Set Load\n"));
		return LOAD_RESULT_OK;
	}

	//�t�@�C�����擾
	FunctionName=m_AstrPrefix;
	FunctionName+="GetFileName";
	if(!ArchiverQueryFunctionList(57)){//57:???GetFileName
		strErr.Format(IDS_ERROR_UNAVAILABLE_API,FunctionName);
		FreeDLL();
		TRACE(_T("%s\n"),(LPCTSTR)strErr);
		return LOAD_RESULT_INVALID;
	}
	ArchiverGetFileName=(COMMON_ARCHIVER_GETFILENAME)GetProcAddress(m_hInstDLL,FunctionName);
	if(NULL==ArchiverGetFileName){
		strErr.Format(IDS_ERROR_DLL_FUNCTION_GET,m_strDllName,(LPCTSTR)CA2T(FunctionName));
		FreeDLL();
		TRACE(_T("%s\n"),(LPCTSTR)strErr);
		return LOAD_RESULT_INVALID;
	}

	//�t�@�C�������擾
	FunctionName=m_AstrPrefix;
	FunctionName+="GetAttribute";
	//CAB32.DLL�ł�CabGetAttribute()��CabQueryFunctionList()�ŗ��p�s�ƕ񍐂���邪
	//���ۂ͗��p�\�ł��邽�߁A�ΏǗÖ@�I�ɂ����𖳌��ɂ����B
/*	if(!ArchiverQueryFunctionList(64)){//64:???GetAttribute
		TRACE(_T("Function %s is not Available\n"),FunctionName);
	}
	else{*/
		ArchiverGetAttribute=(COMMON_ARCHIVER_GETATTRIBUTE)GetProcAddress(m_hInstDLL,FunctionName);
		if(NULL==ArchiverGetAttribute){
			TRACE(_T("Failed to Load Function %s\n"),(LPCTSTR)CA2T(FunctionName));
		}
//	}

	//�t�@�C���T�C�Y�擾(���k�O)
	FunctionName=m_AstrPrefix;
	FunctionName+="GetOriginalSizeEx";
	if(!ArchiverQueryFunctionList(85)){//85:???GetOriginalSizeEx
		TRACE(_T("Function %s is not Available\n"),(LPCTSTR)CA2T(FunctionName));
	}
	else{
		ArchiverGetOriginalSizeEx=(COMMON_ARCHIVER_GETORIGINALSIZEEX)GetProcAddress(m_hInstDLL,FunctionName);
		if(NULL==ArchiverGetOriginalSizeEx){
			TRACE(_T("Failed to Load Function %s\n"),(LPCTSTR)CA2T(FunctionName));
		}
	}

	//�t�@�C���T�C�Y�擾(���k��)
	FunctionName=m_AstrPrefix;
	FunctionName+="GetCompressedSizeEx";
	if(!ArchiverQueryFunctionList(86)){//86:???GetCompressedSizeEx
		TRACE(_T("Function %s is not Available\n"),(LPCTSTR)CA2T(FunctionName));
	}
	else{
		ArchiverGetCompressedSizeEx=(COMMON_ARCHIVER_GETORIGINALSIZEEX)GetProcAddress(m_hInstDLL,FunctionName);
		if(NULL==ArchiverGetCompressedSizeEx){
			TRACE(_T("Failed to Load Function %s\n"),(LPCTSTR)CA2T(FunctionName));
		}
	}

	//�t�@�C�������擾(�g��)
	FunctionName=m_AstrPrefix;
	FunctionName+="GetWriteTimeEx";
	if(!ArchiverQueryFunctionList(70)){//70:???GetWriteTimeEx
		TRACE(_T("Function %s is not Available\n"),(LPCTSTR)CA2T(FunctionName));
	}
	else{
		ArchiverGetWriteTimeEx=(COMMON_ARCHIVER_GETWRITETIMEEX)GetProcAddress(m_hInstDLL,FunctionName);
		if(NULL==ArchiverGetWriteTimeEx){
			TRACE(_T("Failed to Load Function %s\n"),(LPCTSTR)CA2T(FunctionName));
		}
	}
	//�t�@�C�������擾(�ʏ��;�g���ł��g���Ȃ��Ƃ��̂�)
	if(NULL==ArchiverGetWriteTimeEx){
		FunctionName=m_AstrPrefix;
		FunctionName+="GetWriteTime";
		if(!ArchiverQueryFunctionList(67)){//67:???GetWriteTime
			TRACE(_T("Function %s is not Available\n"),(LPCTSTR)CA2T(FunctionName));
		}
		else{
			ArchiverGetWriteTime=(COMMON_ARCHIVER_GETWRITETIME)GetProcAddress(m_hInstDLL,FunctionName);
			if(NULL==ArchiverGetWriteTime){
				TRACE(_T("Failed to Load Function %s\n"),(LPCTSTR)CA2T(FunctionName));
			}
		}
	}

	//�t�@�C�����k���\�b�h�擾
	FunctionName=m_AstrPrefix;
	FunctionName+="GetMethod";
	if(!ArchiverQueryFunctionList(66)){//66:???GetMethod
		TRACE(_T("Function %s is not Available\n"),(LPCTSTR)CA2T(FunctionName));
	}
	else{
		ArchiverGetMethod=(COMMON_ARCHIVER_GETMETHOD)GetProcAddress(m_hInstDLL,FunctionName);
		if(NULL==ArchiverGetMethod){
			TRACE(_T("Failed to Load Function %s\n"),(LPCTSTR)CA2T(FunctionName));
		}
	}


	if(LOAD_DLL_STANDARD==m_LoadLevel){
		TRACE(_T("Standard Function Set Load\n"));
		return LOAD_RESULT_OK;
	}

	ASSERT(!"Unknown Load Set!!!");
	return LOAD_RESULT_INVALID;
}

bool CArchiverDLL::InspectArchiveBegin(LPCTSTR ArcFileName,CConfigManager&)
{
	TRACE(_T("CArchiverDLL::InspectArchiveBegin()\n"));
	ASSERT(ArchiverOpenArchive);
	if(!ArchiverOpenArchive){
		return false;
	}
	if(m_hInspectArchive){
		ASSERT(!"Close the Archive First!!!\n");
		return false;
	}
	m_hInspectArchive=ArchiverOpenArchive(NULL,CT2A(ArcFileName),m_dwInspectMode);
	if(!m_hInspectArchive){
		TRACE(_T("Failed to Open Archive\n"));
		return false;
	}
	m_bInspectFirstTime=true;

	FILL_ZERO(m_IndividualInfo);
	return true;
}

bool CArchiverDLL::InspectArchiveEnd()
{
	ASSERT(ArchiverCloseArchive);
	if(!ArchiverCloseArchive){
		return false;
	}
	if(!m_hInspectArchive){
		ASSERT(!"The Archive is Already Closed!!!\n");
		return false;
	}
	ArchiverCloseArchive(m_hInspectArchive);
	m_hInspectArchive=NULL;
	return true;
}

bool CArchiverDLL::InspectArchiveGetFileName(CString &FileName)
{
	if(!m_hInspectArchive){
		ASSERT(!"Open an Archive First!!!\n");
		return false;
	}
	if(ArchiverGetFileName){
		char szBuffer[FNAME_MAX32*2+1]={0};
		ArchiverGetFileName(m_hInspectArchive,szBuffer,FNAME_MAX32*2);
		FileName=szBuffer;
		return true;
	}
	else{
		ASSERT(LOAD_DLL_STANDARD!=m_LoadLevel);
		FileName=m_IndividualInfo.szFileName;
		return true;
	}
}

bool CArchiverDLL::InspectArchiveNext()
{
	ASSERT(ArchiverFindFirst);
	ASSERT(ArchiverFindNext);
	if(!ArchiverFindFirst||!ArchiverFindNext){
		return false;
	}
	if(!m_hInspectArchive){
		ASSERT(!"Open an Archive First!!!\n");
		return false;
	}

	FILL_ZERO(m_IndividualInfo);
	bool bRet;
	if(m_bInspectFirstTime){
		m_bInspectFirstTime=false;
		bRet=(0==ArchiverFindFirst(m_hInspectArchive,m_AstrFindParam,&m_IndividualInfo));
	}
	else{
		bRet=(0==ArchiverFindNext(m_hInspectArchive,&m_IndividualInfo));
	}

	return bRet;
}

int CArchiverDLL::InspectArchiveGetAttribute()
{
//	ASSERT(ArchiverGetAttribute);
	//�Ή����Ă��Ȃ�DLL(CAB32.dll�Ȃ�)�ł�NULL�ɂȂ��Ă���
	if(!ArchiverGetAttribute){
		return -1;
	}
	if(!m_hInspectArchive){
		ASSERT(!"Open an Archive First!!!\n");
		return -1;
	}

	return ArchiverGetAttribute(m_hInspectArchive);
}

bool CArchiverDLL::InspectArchiveGetOriginalFileSize(LARGE_INTEGER &FileSize)
{
	if(!m_hInspectArchive){
		ASSERT(!"Open an Archive First!!!\n");
		return false;
	}
	if(ArchiverGetOriginalSizeEx){
		return (FALSE!=ArchiverGetOriginalSizeEx(m_hInspectArchive,&FileSize));
	}
	else{
		//API�ɂ��t�@�C���T�C�Y�擾�Ɏ��s�����̂ō\���̂̃f�[�^�𗘗p����
		if(-1==m_IndividualInfo.dwOriginalSize)return false;
		FileSize.HighPart=0;
		FileSize.LowPart=m_IndividualInfo.dwOriginalSize;
		return true;
	}
}

bool CArchiverDLL::InspectArchiveGetCompressedFileSize(LARGE_INTEGER &FileSize)
{
	if(!m_hInspectArchive){
		ASSERT(!"Open an Archive First!!!\n");
		return false;
	}
	if(ArchiverGetCompressedSizeEx){
		return (FALSE!=ArchiverGetCompressedSizeEx(m_hInspectArchive,&FileSize));
	}
	else{
		//API�ɂ��t�@�C���T�C�Y�擾�Ɏ��s�����̂ō\���̂̃f�[�^�𗘗p����
		if(-1==m_IndividualInfo.dwCompressedSize)return false;
		FileSize.HighPart=0;
		FileSize.LowPart=m_IndividualInfo.dwCompressedSize;
		return true;
	}
}

bool CArchiverDLL::InspectArchiveGetWriteTime(FILETIME &FileTime)
{
	if(!m_hInspectArchive){
		ASSERT(!"Open an Archive First!!!\n");
		return false;
	}
	//�g���Ŋ֐��Ŏ����擾
	if(ArchiverGetWriteTimeEx){
		if(!ArchiverGetWriteTimeEx(m_hInspectArchive,&FileTime))return false;
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

//���ɓ��t�@�C��CRC�擾
DWORD CArchiverDLL::InspectArchiveGetCRC()
{
	if(!m_hInspectArchive){
		ASSERT(!"Open an Archive First!!!\n");
		return -1;
	}
	return m_IndividualInfo.dwCRC;
}

//���ɓ��t�@�C�����k���擾
WORD CArchiverDLL::InspectArchiveGetRatio()
{
	if(!m_hInspectArchive){
		ASSERT(!"Open an Archive First!!!\n");
		return -1;
	}

	return m_IndividualInfo.wRatio;
}

//���ɓ��t�@�C���i�[���[�h�擾
//ANSI��
bool CArchiverDLL::InspectArchiveGetMethodString(CString &strMethod)
{
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
	strMethod=szBuffer;
	return true;
}

ARCRESULT CArchiverDLL::TestArchive(LPCTSTR lpszFile,CString &strMsg)
{
	ASSERT(IsOK());
	if(!IsOK())return TEST_ERROR;
	TRACE(_T("TestArchive() called.\n"));
	//�f�t�H���g�ł̓e�X�g�͎�������Ă��Ȃ�

	strMsg.Format(IDS_TESTARCHIVE_UNAVAILABLE,lpszFile,GetName());
	return TEST_NOTIMPL;
}


//���X�|���X�t�@�C���Ƀt�@�C�������G�X�P�[�v�����s������ŏ������ށB
//�L���ȃt�@�C���n���h����NULL�łȂ��t�@�C������n�����ƁB
void CArchiverDLL::WriteResponceFile(HANDLE hFile,LPCTSTR fname,bool bQuoteSpaces)
{
	CPath path(fname);
	if(bQuoteSpaces){
		path.QuoteSpaces();
	}
	char szBuffer[_MAX_PATH+1]={0};

	strncpy_s(szBuffer,CT2A(path),_MAX_PATH);

	TRACE(_T("%s\n"),CA2T(szBuffer));

	DWORD dwWritten=0;
	//�t�@�C�������o��
	WriteFile(hFile,szBuffer,strlen(szBuffer)*sizeof(char),&dwWritten,NULL);

	//�t�@�C��������؂邽�߂̉��s
	char CRLF[]="\r\n";
	WriteFile(hFile,&CRLF,strlen(CRLF)*sizeof(char),&dwWritten,NULL);
}

bool CArchiverDLL::ExtractItems(LPCTSTR lpszArcFile,CConfigManager &ConfMan,const ARCHIVE_ENTRY_INFO_TREE *lpBase, const std::list<ARCHIVE_ENTRY_INFO_TREE *> &items, LPCTSTR lpszOutputBaseDir, bool bCollapseDir, CString &strLog)
{
	//�A�[�J�C�u�̑��݂͊��Ɋm�F�ς݂Ƃ���
	//�����R�[�h�֌W�͊��Ɋm�F�ς݂Ƃ���

	//---���X�g����t�@�C���ƃf�B���N�g���𕪗�
	ARCHIVE_ENTRY_INFO_TREE *lpFileParent=NULL;	//�t�@�C���̐e
	std::list<CString> files;			//�t�@�C����p
	std::list<ARCHIVE_ENTRY_INFO_TREE*> dirs;	//�f�B���N�g����p

	std::list<ARCHIVE_ENTRY_INFO_TREE*>::const_iterator ite=items.begin();
	const std::list<ARCHIVE_ENTRY_INFO_TREE*>::const_iterator end=items.end();
	for(;ite!=end;++ite){
		ARCHIVE_ENTRY_INFO_TREE* lpNode=*ite;
		if(lpNode->bDir){
			dirs.push_back(lpNode);
		}else{
			lpFileParent=lpNode->lpParent;
			files.push_back(lpNode->strFullPath);
		}
	}

	// �t�@�C��������
	if(!files.empty()){
		CString pathOutputDir;
		if(!bCollapseDir){
			ArcEntryInfoTree_GetNodePathRelative(lpFileParent,lpBase,pathOutputDir);
		}
		pathOutputDir=lpszOutputBaseDir+pathOutputDir;
		bool bRet=ExtractSpecifiedOnly(lpszArcFile,ConfMan,pathOutputDir,files,strLog);
		if(!bRet){
			return false;
		}
	}

	// �f�B���N�g��������
	if(!dirs.empty()){
		if(!ExtractSubDirectories(lpszArcFile,ConfMan,lpBase,dirs,lpszOutputBaseDir,bCollapseDir,strLog))return false;
	}
	return true;
}

//�w�肳�ꂽ�f�B���N�g��������W�J����;��{����
bool CArchiverDLL::ExtractSubDirectories(LPCTSTR lpszArcFile,CConfigManager &ConfMan,const ARCHIVE_ENTRY_INFO_TREE* lpBase,const std::list<ARCHIVE_ENTRY_INFO_TREE*> &dirs,LPCTSTR lpszOutputDir,bool bCollapseDir,CString &strLog)
{
	std::list<ARCHIVE_ENTRY_INFO_TREE*>::const_iterator ite=dirs.begin();
	const std::list<ARCHIVE_ENTRY_INFO_TREE*>::const_iterator end=dirs.end();
	for(;ite!=end;++ite){
		if(!ExtractDirectoryEntry(lpszArcFile,ConfMan,bCollapseDir?(*ite)->lpParent:lpBase,*ite,lpszOutputDir,bCollapseDir,strLog))return false;
	}
	return true;
}

//�w�肳�ꂽ�f�B���N�g��������W�J����;��{����
bool CArchiverDLL::ExtractDirectoryEntry(LPCTSTR lpszArcFile,CConfigManager &ConfMan,const ARCHIVE_ENTRY_INFO_TREE* lpBase,const ARCHIVE_ENTRY_INFO_TREE* lpDir,LPCTSTR lpszOutputBaseDir,bool bCollapseDir,CString &strLog)
{
	//�f�B���N�g�����̃t�@�C�����
	std::list<CString> files;			//�t�@�C����p
	std::list<ARCHIVE_ENTRY_INFO_TREE*> dirs;	//�f�B���N�g����p

	//�f�B���N�g�����A�[�J�C�u���ɃG���g���Ƃ��ēo�^����Ă��Ă���������
	//->�ʏ�ʂ�o�͂���Əo�͐�ƌ������邱�Ƃ�����
	UINT nItems=lpDir->GetNumChildren();
	for(UINT i=0;i<nItems;i++){
		ARCHIVE_ENTRY_INFO_TREE* lpNode=lpDir->GetChild(i);

		if(lpNode->bDir){
			dirs.push_back(lpNode);
		}else{
			files.push_back(lpNode->strFullPath);
		}
	}

	//--------------------------------------
	// �C�����ꂽ�o�̓f�B���N�g���p�X���Z�o
	//--------------------------------------
	CString pathOutputDir;
	ArcEntryInfoTree_GetNodePathRelative(lpDir,lpBase,pathOutputDir);

	//�o�̓f�B���N�g�����ƌ���
	pathOutputDir=lpszOutputBaseDir+pathOutputDir;

	if(pathOutputDir.GetLength()>_MAX_PATH){
		//�t�H���_���������Ȃ肷����
		strLog=CString(MAKEINTRESOURCE(IDS_ERROR_MAX_PATH));
		return false;
	}

	if(files.empty() && dirs.empty() && lpDir->strFullPath.IsEmpty()){	//��t�H���_���A�[�J�C�u���ɑ��݂����ꍇ
		//��f�B���N�g���G���g�������Ƃŕ�������
		if(!UtilMakeSureDirectoryPathExists(pathOutputDir)){
			strLog.Format(IDS_ERROR_CANNOT_MAKE_DIR,(LPCTSTR)pathOutputDir);
			return false;
		}
		return true;
	}

	//---�ċA�I�ɏ���

	//----------------
	// �t�@�C��������
	//----------------
	if(!files.empty()){
		if(!ExtractSpecifiedOnly(lpszArcFile,ConfMan,pathOutputDir,files,strLog)){
			return false;
		}
	}

	//--------------------
	// �f�B���N�g��������
	//--------------------
	if(!dirs.empty()){
		ExtractSubDirectories(lpszArcFile,ConfMan,bCollapseDir?lpDir:lpBase,dirs,lpszOutputBaseDir,bCollapseDir,strLog);
	}
	return true;
}
