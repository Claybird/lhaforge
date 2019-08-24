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
#include "Archiver7ZIP.h"
#include "../Utilities/FileOperation.h"
#include "../Utilities/OSUtil.h"
#include "../Utilities/StringUtil.h"
#include "../Utilities/TemporaryDirMgr.h"
#include "../resource.h"
#include "../ConfigCode/ConfigExtract.h"
#include "../ConfigCode/Config7Z.h"
#include "../ConfigCode/ConfigZIP.h"
#include "../Dialogs/SevenZipVolumeSizeDlg.h"
#include "../Utilities/DummyWindow.h"

CArchiver7ZIP::CArchiver7ZIP():
	ArchiverSetUnicodeMode(NULL),
	ArchiverGetArchiveType(NULL)
{
	m_nRequiredVersion=920;
	m_nRequiredSubVersion=2;
	m_strDllName=_T("7-ZIP32.DLL");
	m_AstrPrefix="SevenZip";
}

CArchiver7ZIP::~CArchiver7ZIP()
{
	FreeDLL();
}

LOAD_RESULT CArchiver7ZIP::LoadDLL(CConfigManager &ConfMan,CString &strErr)
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
		strErr.Format(IDS_ERROR_DLL_FUNCTION_GET,m_strDllName,(LPCTSTR)CA2T(strFunctionName));
		FreeDLL();
		TRACE(_T("%s\n"),(LPCTSTR)strErr);
		return LOAD_RESULT_INVALID;
	}
	//������UNICODE���[�h�ɐݒ�
	if(!ArchiverSetUnicodeMode(TRUE)){
		//UNICODE�ɂł��Ȃ������玸�s�Ƃ���
		return LOAD_RESULT_INVALID;
	}

	//�A�[�J�C�u��ʔ���p
	strFunctionName=m_AstrPrefix+"GetArchiveType";
	ArchiverGetArchiveType=(COMMON_ARCHIVER_GETARCHIVETYPE)GetProcAddress(m_hInstDLL,strFunctionName);
	if(NULL==ArchiverGetArchiveType){
		strErr.Format(IDS_ERROR_DLL_FUNCTION_GET,m_strDllName,(LPCTSTR)CA2T(strFunctionName));
		FreeDLL();
		TRACE(_T("%s\n"),(LPCTSTR)strErr);
		return LOAD_RESULT_INVALID;
	}
	return LOAD_RESULT_OK;
}

void CArchiver7ZIP::FreeDLL()
{
	if(m_hInstDLL){
		ArchiverSetUnicodeMode=NULL;
		ArchiverGetArchiveType=NULL;
		CArchiverDLL::FreeDLL();
	}
}

bool CArchiver7ZIP::FormatCompressCommandZIP(const CConfigZIP &ConfZIP,CString &Param,bool bZIPSFX,int Options,LPCTSTR lpszMethod,LPCTSTR lpszLevel,CString &strLog)
{
	Param+=
		_T("a ")			//���k
		_T("-tzip ")		//ZIP�`���ň��k
		_T("-r0 ")			//�T�u�f�B���N�g��������
	;
	bool bBadSFX=false;
	//���k����
	if(lpszMethod && *lpszMethod!=_T('\0')){
		Param+=_T("-mm=");
		Param+=lpszMethod;
		Param+=_T(" ");
	}else{
		switch(ConfZIP.CompressType){
		case ZIP_COMPRESS_DEFLATE:
			Param+=_T("-mm=Deflate ");
			break;
		case ZIP_COMPRESS_DEFLATE64:
			Param+=_T("-mm=Deflate64 ");
			break;
		case ZIP_COMPRESS_BZIP2:
			bBadSFX=true;
			Param+=_T("-mm=BZip2 ");
			break;
		case ZIP_COMPRESS_COPY:
			Param+=_T("-mm=Copy ");
			break;
		case ZIP_COMPRESS_LZMA:
			bBadSFX=true;
			Param+=_T("-mm=LZMA ");
			break;
		case ZIP_COMPRESS_PPMD:
			bBadSFX=true;
			Param+=_T("-mm=PPMd ");
			break;
		}
	}
	//���k���x��
	if(lpszLevel && *lpszLevel!=_T('\0')){
		Param+=_T("-mx=");
		Param+=lpszLevel;
		Param+=_T(" ");
	}else{
		switch(ConfZIP.CompressLevel){
		case ZIP_COMPRESS_LEVEL0:
			Param+=_T("-mx=0 ");
			break;
		case ZIP_COMPRESS_LEVEL5:
			Param+=_T("-mx=5 ");
			break;
		case ZIP_COMPRESS_LEVEL9:
			Param+=_T("-mx=9 ");
			break;
		}
	}
	if((ZIP_COMPRESS_DEFLATE==ConfZIP.CompressType)||(ZIP_COMPRESS_DEFLATE64==ConfZIP.CompressType)){
		if(ConfZIP.SpecifyDeflateMemorySize){
			CString temp;
			temp.Format(_T("-mfb=%d "),ConfZIP.DeflateMemorySize);
			Param+=temp;
		}
		if(ConfZIP.SpecifyDeflatePassNumber){
			CString temp;
			temp.Format(_T("-mpass=%d "),ConfZIP.DeflatePassNumber);
			Param+=temp;
		}
	}
	//�����R�[�h����
	if(ConfZIP.ForceUTF8){
		bBadSFX=true;
		Param+=_T("-mcu=on ");
	}
	if(Options&COMPRESS_PASSWORD){	//�p�X���[�h�t��
		Param+=_T("-p ");

		//�Í����[�h
		switch(ConfZIP.CryptoMode){
		case ZIP_CRYPTO_ZIPCRYPTO:
			Param+=_T("-mem=ZipCrypto ");
			break;
		case ZIP_CRYPTO_AES128:
			bBadSFX=true;
			Param+=_T("-mem=AES128 ");
			break;
		case ZIP_CRYPTO_AES192:
			bBadSFX=true;
			Param+=_T("-mem=AES192 ");
			break;
		case ZIP_CRYPTO_AES256:
			bBadSFX=true;
			Param+=_T("-mem=AES256 ");
			break;
		}
	}

	if(bZIPSFX){
		//BZip2�̎��ȉ𓀂̓T�|�[�g����Ă��Ȃ�
		if(bBadSFX){
			strLog=CString(MAKEINTRESOURCE(IDS_ERROR_ZIP_SFX_DONT_SUPPORT_FORMAT));
			return false;
		}
	}

	return true;
}

bool CArchiver7ZIP::FormatCompressCommand7Z(const CConfig7Z &Conf7Z,CString &Param,int Options,LPCTSTR lpszMethod,LPCTSTR lpszLevel,CString &strLog)
{
	Param+=
		_T("a ")			//���k
		_T("-t7z ")			//7-Zip�`���ň��k
		_T("-r0 ")			//�T�u�f�B���N�g��������
	;
	if(lpszMethod && *lpszMethod!=_T('\0')){
		Param+=_T("-m0=");
		Param+=lpszMethod;
		Param+=_T(" ");

		if(lpszLevel && *lpszLevel!=_T('\0')){
			Param+=_T("-mx=");
			Param+=lpszLevel;
			Param+=_T(" ");
		}
	}else{
		if(lpszLevel && *lpszLevel!=_T('\0')){
			Param+=_T("-mx=");
			Param+=lpszLevel;
			Param+=_T(" ");
		}else if(Conf7Z.UsePreset){
			//�v���Z�b�g���k���[�h
			switch(Conf7Z.CompressLevel){	//���k���x��
			case SEVEN_ZIP_COMPRESS_LEVEL0:
				Param+=_T("-mx=0 ");
				break;
			case SEVEN_ZIP_COMPRESS_LEVEL1:
				Param+=_T("-mx=1 ");
				break;
			case SEVEN_ZIP_COMPRESS_LEVEL5:
				Param+=_T("-mx=5 ");
				break;
			case SEVEN_ZIP_COMPRESS_LEVEL7:
				Param+=_T("-mx=7 ");
				break;
			case SEVEN_ZIP_COMPRESS_LEVEL9:
				Param+=_T("-mx=9 ");
				break;
			}
		}
		if(!Conf7Z.UsePreset){
			switch(Conf7Z.CompressType){	//���k����
			case SEVEN_ZIP_COMPRESS_LZMA:
				Param+=_T("-m0=LZMA:a=");
				switch(Conf7Z.LZMA_Mode){	//LZMA���k���[�h
				case SEVEN_ZIP_LZMA_MODE0:
					Param+=_T("0 ");
					break;
				case SEVEN_ZIP_LZMA_MODE1:
					Param+=_T("1 ");
					break;
				//case SEVEN_ZIP_LZMA_MODE2:
				//	Param+=_T("2 ");
				//	break;
				}
				break;
			case SEVEN_ZIP_COMPRESS_PPMD:
				Param+=_T("-m0=PPMd ");
				break;
			case SEVEN_ZIP_COMPRESS_BZIP2:
				Param+=_T("-m0=BZip2 ");
				break;
			case SEVEN_ZIP_COMPRESS_DEFLATE:
				Param+=_T("-m0=Deflate ");
				break;
			case SEVEN_ZIP_COMPRESS_COPY:
				Param+=_T("-m0=Copy ");
				break;
			case SEVEN_ZIP_COMPRESS_LZMA2:
				Param+=_T("-m0=LZMA2:a=");
				switch(Conf7Z.LZMA_Mode){	//LZMA���k���[�h
				case SEVEN_ZIP_LZMA_MODE0:
					Param+=_T("0 ");
					break;
				case SEVEN_ZIP_LZMA_MODE1:
					Param+=_T("1 ");
					break;
				}
			}
		}
	}
	if(Conf7Z.SolidMode){	//�\���b�h���[�h
		Param+=_T("-ms=on ");
	}else{
		Param+=_T("-ms=off ");
	}
	//------------
	// �w�b�_���k
	//------------
	if(Conf7Z.HeaderCompression){	//�w�b�_���k
		Param+=_T("-mhc=on ");
	}else{
		Param+=_T("-mhc=off ");
		Param+=_T("-mhcf=off ");
	}

	if(Options&COMPRESS_PASSWORD){	//�p�X���[�h�t��
		Param+=_T("-p ");
		if(Conf7Z.HeaderEncryption){	//�w�b�_�Í���
			Param+=_T("-mhe=on ");
		}
		else{
			Param+=_T("-mhe=off ");
		}
	}
	if(Options&COMPRESS_SFX){
		//BZip2�̎��ȉ𓀂̓T�|�[�g����Ă��Ȃ�
		if(!Conf7Z.UsePreset && SEVEN_ZIP_COMPRESS_BZIP2==Conf7Z.CompressType){
			strLog=CString(MAKEINTRESOURCE(IDS_ERROR_7Z_SFX_DONT_SUPPORT_BZIP2));
			return false;
		}
		Param+=_T("-sfx ");
	}
	return true;
}

/*
format�̎w��́AB2E32.dll�ł̂ݗL��
level�̎w��́AB2E32.dll�ȊO�ŗL��
*/
bool CArchiver7ZIP::Compress(LPCTSTR ArcFileName,std::list<CString> &ParamList,CConfigManager &ConfMan,const PARAMETER_TYPE Type,int Options,LPCTSTR lpszFormat,LPCTSTR lpszMethod,LPCTSTR lpszLevel,CString &strLog)
{
	LPCTSTR lpszSplitSize = lpszFormat;

	if(!IsOK()){
		return false;
	}

	ASSERT(0!=_tcslen(ArcFileName));
	TRACE(_T("ArcFileName=%s\n"),ArcFileName);

	//�������ɂ̏ꍇ�͎��ȉ𓀂͏o���Ȃ�
	if((Options&COMPRESS_SPLIT)&&(Options&COMPRESS_SFX)){
		ErrorMessage(CString(MAKEINTRESOURCE(IDS_ERROR_CANNOT_SPLIT_SFX)));
		strLog=CString(MAKEINTRESOURCE(IDS_ERROR_CANNOT_SPLIT_SFX));
		Options&=~COMPRESS_SFX;
	}

	//==============================================
	// ���X�|���X�t�@�C���p�e���|�����t�@�C�����擾
	//==============================================
	TCHAR ResponceFileName[_MAX_PATH+1];
	FILL_ZERO(ResponceFileName);
	if(!UtilGetTemporaryFileName(ResponceFileName,_T("zip"))){
		strLog=CString(MAKEINTRESOURCE(IDS_ERROR_TEMPORARY_FILE_CREATE));
		return false;
	}
	ASSERT(0!=_tcslen(ResponceFileName));

	//============================================
	// ���ȉ𓀃t�@�C���p�e���|�����t�@�C�����擾
	//============================================
	TCHAR SFXTemporaryFileName[_MAX_PATH+1];
	FILL_ZERO(SFXTemporaryFileName);
	TCHAR SFXModulePath[_MAX_PATH+1];
	FILL_ZERO(SFXModulePath);
	bool bZIPSFX=((0!=(Options&COMPRESS_SFX))&&(PARAMETER_ZIP==Type));
	if(bZIPSFX){
		//2�i�K�쐬����
		if(!UtilGetTemporaryFileName(SFXTemporaryFileName,_T("sfx"))){
			strLog=CString(MAKEINTRESOURCE(IDS_ERROR_TEMPORARY_FILE_CREATE));
			return false;
		}
		ASSERT(0!=_tcslen(SFXTemporaryFileName));
		DeleteFile(SFXTemporaryFileName);//�S�~�t�@�C������

		//---SFX���W���[��
		LPTSTR lptemp;
		{
			//���S�ȃp�X�Ɉړ�;DLL�ǂݍ��ݑ΍�
			CCurrentDirManager cdm(UtilGetModuleDirectoryPath());
			if(!SearchPath(NULL,_T("SFX32GUI.DAT"),NULL,_MAX_PATH,SFXModulePath,&lptemp)){
				strLog.Format(IDS_ERROR_SFX_MODULE_NOT_FOUND,_T("SFX32GUI.DAT"));
				return false;
			}
		}
	}

	//====================================================
	// ���X�|���X�t�@�C�����Ɉ��k�Ώۃt�@�C�������L������
	// �A�[�J�C�u�t�@�C�����̓R�}���h���C���Œ��ڎw�肷��
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
			CPath strPath=*ite;

			if(strPath.IsDirectory()){
				strPath.Append(_T("*"));
			}
			WriteResponceFile(hFile,strPath);
		}
		CloseHandle(hFile);
	}

	//===========================
	// DLL�ɓn���I�v�V�����̐ݒ�
	//===========================
	TRACE(_T("DLL�ɓn���I�v�V�����̐ݒ�\n"));

	CString Param;//�R�}���h���C�� �p�����[�^ �o�b�t�@

	CConfigZIP confZIP;
	CConfig7Z  conf7Z;
	switch(Type){
	case PARAMETER_ZIP:	//ZIP�`���ň��k
		confZIP.load(ConfMan);
		break;
	case PARAMETER_7Z:	//7z�`���ň��k
		conf7Z.load(ConfMan);
		break;
	}

	switch(Type){
	case PARAMETER_ZIP:	//ZIP�`���ň��k
		if(!FormatCompressCommandZIP(confZIP,Param,bZIPSFX,Options,lpszMethod,lpszLevel,strLog)){
			DeleteFile(ResponceFileName);
			return false;
		}
		break;
	case PARAMETER_7Z:	//7z�`���ň��k
		if(!FormatCompressCommand7Z(conf7Z,Param,Options,lpszMethod,lpszLevel,strLog)){
			DeleteFile(ResponceFileName);
			return false;
		}
		break;
	}
	//����
	if(Options&COMPRESS_SPLIT){
		if(lpszSplitSize && _tcslen(lpszSplitSize)>0){
			CString temp;
			temp.Format(_T("-v%s "),lpszSplitSize);
			Param+=temp;
		}else{
			int unitIndex=-1;
			int size=-1;
			switch(Type){
			case PARAMETER_ZIP:	//ZIP�`���ň��k
				if(confZIP.SpecifySplitSize){
					unitIndex = confZIP.SplitSizeUnit;
					size = confZIP.SplitSize;
				}
				break;
			case PARAMETER_7Z:	//7z�`���ň��k
				if(conf7Z.SpecifySplitSize){
					unitIndex = conf7Z.SplitSizeUnit;
					size = conf7Z.SplitSize;
				}
				break;
			}

			if(size==-1 && unitIndex==-1){
				C7Zip32VolumeSizeDialog vsd;
				if(IDOK!=vsd.DoModal())return false;
				unitIndex = vsd.SelectIndex;
				size = vsd.VolumeSize;
			}

			CString temp;
			temp.Format(_T("-v%d%s "),size,ZIP_VOLUME_UNIT[unitIndex].ParamName);
			Param+=temp;
		}
	}

	Param+=_T("-scsUTF-8 ");	//���X�|���X�t�@�C���̃R�[�h�y�[�W�w��

	//��ƃf�B���N�g��
	Param+=_T("\"-w");
	Param+=UtilGetTempPath();
	Param+=_T("\" ");

	//���k��t�@�C�����w��
	if(bZIPSFX){
		Param+=_T("\"");
		Param+=SFXTemporaryFileName;
		Param+=_T("\" ");
	}
	else{
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
	std::vector<BYTE> szLog(LOG_BUFFER_SIZE);
	szLog[0]='\0';
	int Ret=ArchiveHandler(NULL,C2UTF8(Param),(LPSTR)&szLog[0],LOG_BUFFER_SIZE-1);
	CString strTmp;
	UtilToUNICODE(strTmp,&szLog[0],szLog.size()-1,UTILCP_UTF8);
	//strLog=&szLog[0];
	strLog=strTmp;

	//�g�������X�|���X�t�@�C���͏���
	DeleteFile(ResponceFileName);

	//�G���[�����O�o��
	if(!bZIPSFX||0!=Ret){
		if(bZIPSFX){
			DeleteFile(SFXTemporaryFileName);
		}
		return 0==Ret;
	}

	//==================================
	// ���ȉ𓀏��ɂɕϊ�(�o�C�i������)
	//==================================
	//�o�͐�t�@�C�����J��
	HANDLE hArcFile=CreateFile(ArcFileName,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
	if(INVALID_HANDLE_VALUE==hArcFile){
		strLog.Format(IDS_ERROR_ACCESS_OUTPUT_FILE,ArcFileName);
		DeleteFile(ArcFileName);
		DeleteFile(SFXTemporaryFileName);
		return false;
	}
	{
		//SFX���W���[����ǂݎ�胂�[�h�ŊJ��
		HANDLE hSFXModuleFile=CreateFile(SFXModulePath,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
		if(INVALID_HANDLE_VALUE==hSFXModuleFile){
			strLog.Format(IDS_ERROR_SFX_MODULE_CANNOT_ACCESS,SFXModulePath);
			CloseHandle(hArcFile);
			DeleteFile(ArcFileName);
			DeleteFile(SFXTemporaryFileName);
			return false;
		}
		//SFX���W���[���̒��g���R�s�[
		int Result=UtilAppendFile(hArcFile,hSFXModuleFile);
		if(Result>0){	//�ǂݎ��G���[
			strLog.Format(IDS_ERROR_SFX_MODULE_CANNOT_ACCESS,SFXModulePath);
			CloseHandle(hArcFile);
			CloseHandle(hSFXModuleFile);
			DeleteFile(ArcFileName);
			DeleteFile(SFXTemporaryFileName);
			return false;
		}
		else if(Result<0){	//�������݃G���[
			strLog.Format(IDS_ERROR_ACCESS_OUTPUT_FILE,ArcFileName);
			CloseHandle(hArcFile);
			CloseHandle(hSFXModuleFile);
			DeleteFile(ArcFileName);
			DeleteFile(SFXTemporaryFileName);
			return false;
		}
		CloseHandle(hSFXModuleFile);
	}
	{
		//�e���|�����t�@�C����ǂݎ�胂�[�h�ŊJ��
		HANDLE hSFXTempFile=CreateFile(SFXTemporaryFileName,GENERIC_READ,0,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
		if(INVALID_HANDLE_VALUE==hSFXTempFile){
			strLog=CString(MAKEINTRESOURCE(IDS_ERROR_TEMPORARY_FILE_ACCESS));
			CloseHandle(hArcFile);
			DeleteFile(ArcFileName);
			DeleteFile(SFXTemporaryFileName);
			return false;
		}
		//�e���|�����t�@�C���̒��g���R�s�[
		int Result=UtilAppendFile(hArcFile,hSFXTempFile);
		if(Result>0){	//�ǂݎ��G���[
			strLog=CString(MAKEINTRESOURCE(IDS_ERROR_TEMPORARY_FILE_ACCESS));
			CloseHandle(hArcFile);
			CloseHandle(hSFXTempFile);
			DeleteFile(ArcFileName);
			DeleteFile(SFXTemporaryFileName);
			return false;
		}
		else if(Result<0){	//�������݃G���[
			strLog.Format(IDS_ERROR_ACCESS_OUTPUT_FILE,ArcFileName);
			CloseHandle(hArcFile);
			CloseHandle(hSFXTempFile);
			DeleteFile(ArcFileName);
			DeleteFile(SFXTemporaryFileName);
			return false;
		}
		CloseHandle(hSFXTempFile);
		DeleteFile(SFXTemporaryFileName);
	}
	CloseHandle(hArcFile);

	return true;
}

bool CArchiver7ZIP::Extract(LPCTSTR ArcFileName,CConfigManager&,const CConfigExtract& Config,bool bSafeArchive,LPCTSTR OutputDir,CString &strLog)
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
	Param+=_T("x ");			//��
	if(Config.ForceOverwrite){
		//�����㏑��
		Param+=_T("-aoa ");
	}

	Param+=_T("-scsUTF-8 ");	//���X�|���X�t�@�C���̃R�[�h�y�[�W�w��

	//��ƃf�B���N�g��
	Param+=_T("\"-w");
	Param+=UtilGetTempPath();
	Param+=_T("\" ");

	//�A�[�J�C�u�t�@�C�����w��
	Param+=_T("\"");
	Param+=ArcFileName;
	Param+=_T("\" ");

	ASSERT(!Param.IsEmpty());
	TRACE(_T("ArchiveHandler Commandline Parameter:%s\n"),Param);

	TRACE(_T("ArchiveHandler�Ăяo��\n"));
	std::vector<BYTE> szLog(LOG_BUFFER_SIZE);
	szLog[0]='\0';
	int Ret=ArchiveHandler(NULL,C2UTF8(Param),(LPSTR)&szLog[0],LOG_BUFFER_SIZE-1);
	CString strTmp;
	UtilToUNICODE(strTmp,&szLog[0],szLog.size()-1,UTILCP_UTF8);
	//strLog=&szLog[0];
	strLog=strTmp;

	return 0==Ret;
}

//���X�|���X�t�@�C���Ƀt�@�C�������G�X�P�[�v�����s������ŏ������ށB
//�L���ȃt�@�C���n���h����NULL�łȂ��t�@�C������n�����ƁB
void CArchiver7ZIP::WriteResponceFile(HANDLE hFile,LPCTSTR fname)
{
	CPath strPath=fname;

	strPath.QuoteSpaces();

	DWORD dwWritten=0;
	//�t�@�C����+���s���o��
	std::vector<BYTE> cArray;
	UtilToUTF8(cArray,strPath+_T("\r\n"));
	WriteFile(hFile,&cArray[0],(cArray.size()-1)*sizeof(BYTE),&dwWritten,NULL);
}


//=============================================================
// SevenZipGetFileName()�̏o�͌��ʂ���ɁA�i�[���ꂽ�t�@�C����
// �p�X���������Ă��邩�ǂ������ʂ��A��d�t�H���_�쐬��h��
//=============================================================
bool CArchiver7ZIP::ExamineArchive(LPCTSTR ArcFileName,CConfigManager& ConfMan,bool bSkipDir,bool &bInFolder,bool &bSafeArchive,CString &BaseDir,CString &strErr)
{
	//UNICODE���������S���ǂ������肷��
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
		}
		if(!UtilIsSafeUnicode((LPCTSTR)Buffer+StartIndex)){	//�댯��UNICODE�w�肪������΁A�댯�ȃt�@�C���ƌ��Ȃ�
			//�č��I��
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

bool CArchiver7ZIP::ExtractSpecifiedOnly(LPCTSTR ArcFileName,CConfigManager&,LPCTSTR OutputDir,std::list<CString> &FileList,CString &strLog,bool bUsePath)
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
	if(!UtilGetTemporaryFileName(ResponceFileName,_T("7zp"))){
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
			TRACE(_T("%s=>%s\n"),LPCTSTR(*ite),OutputDir);
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
		Param+=_T("x ");	//�p�X�t����
	}else{
		Param+=_T("e ");		//�f�B���N�g���Ȃ��ŉ�
	}
	Param+=
		_T("-r- ")		//�ċA��������
		_T("-scsUTF-8 ")	//���X�|���X�t�@�C���̃R�[�h�y�[�W�w��
	;

	//��ƃf�B���N�g��
	Param+=_T("\"-w");
	Param+=UtilGetTempPath();
	Param+=_T("\" ");

	//�A�[�J�C�u�t�@�C�����w��
	Param+=_T("\"");
	Param+=ArcFileName;
	Param+=_T("\" ");

	//�o�͐�w��
	Param+=_T("-o\"");
	Param+=OutputDir;
	Param+=_T("\" ");

	//���X�|���X�t�@�C�����w��
	Param+=_T("\"@");
	Param+=ResponceFileName;
	Param+=_T("\"");

	TRACE(_T("ArchiveHandler�Ăяo��\nCommandline Parameter:%s\n"),Param);
	std::vector<BYTE> szLog(LOG_BUFFER_SIZE);
	szLog[0]='\0';
	int Ret=ArchiveHandler(NULL,C2UTF8(Param),(LPSTR)&szLog[0],LOG_BUFFER_SIZE-1);
	CString strTmp;
	UtilToUNICODE(strTmp,&szLog[0],szLog.size()-1,UTILCP_UTF8);
	//strLog=&szLog[0];
	strLog=strTmp;
	//�g�������X�|���X�t�@�C���͏���
	DeleteFile(ResponceFileName);

	return 0==Ret;
}


bool CArchiver7ZIP::DeleteItemFromArchive(LPCTSTR ArcFileName,CConfigManager&,const std::list<CString>& FileList,CString &strLog)
{
	if(!IsOK()){
		return false;
	}

	//==============================================
	// ���X�|���X�t�@�C���p�e���|�����t�@�C�����擾
	//==============================================
	TCHAR ResponceFileName[_MAX_PATH+1];
	FILL_ZERO(ResponceFileName);
	if(!UtilGetTemporaryFileName(ResponceFileName,_T("7zp"))){
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
			CString tmp=*ite;
			if(tmp[tmp.GetLength()-1]==_T('\\'))tmp.Delete(tmp.GetLength()-1);
			WriteResponceFile(hFile,tmp);
		}
		CloseHandle(hFile);
	}

	//===========================
	// DLL�ɓn���I�v�V�����̐ݒ�
	//===========================
	CString Param;

	//�폜�p�����[�^
	Param+=
		_T("d ")	//�폜
		_T("-r- ")	//�ċA��������
		_T("-scsUTF-8 ")	//���X�|���X�t�@�C���̃R�[�h�y�[�W�w��
	;

	//��ƃf�B���N�g��
	Param+=_T("\"-w");
	Param+=UtilGetTempPath();
	Param+=_T("\" ");

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
	std::vector<BYTE> szLog(LOG_BUFFER_SIZE);
	szLog[0]='\0';
	int Ret=ArchiveHandler(NULL,C2UTF8(Param),(LPSTR)&szLog[0],LOG_BUFFER_SIZE-1);
	CString strTmp;
	UtilToUNICODE(strTmp,&szLog[0],szLog.size()-1,UTILCP_UTF8);
	//strLog=&szLog[0];
	strLog=strTmp;
	//�g�������X�|���X�t�@�C���͏���
	DeleteFile(ResponceFileName);

	return 0==Ret;
}

ARCRESULT CArchiver7ZIP::TestArchive(LPCTSTR ArcFileName,CString &strLog)
{
	TRACE(_T("TestArchive() called.\n"));
	ASSERT(IsOK());
	if(!IsOK()){
		return TEST_ERROR;
	}

	//t�R�}���h�ɂ��e�X�g����������Ă���
	CString Param=
		_T("t ")			//�e�X�g
		_T("\"")
	;
	Param+=ArcFileName;
	Param+=_T("\"");

	//char szLog[LOG_BUFFER_SIZE]={0};
	std::vector<BYTE> szLog(LOG_BUFFER_SIZE);
	szLog[0]='\0';
	int Ret=ArchiveHandler(NULL,C2UTF8(Param),(LPSTR)&szLog[0],LOG_BUFFER_SIZE-1);
	CString strTmp;
	UtilToUNICODE(strTmp,&szLog[0],szLog.size()-1,UTILCP_UTF8);
	//strLog=&szLog[0];
	strLog=strTmp;



	if(Ret==0)return TEST_OK;
	else return TEST_NG;
}


//-------------------------------
//---UNICODE�ł��I�[�o�[���C�h---
//-------------------------------
BOOL CArchiver7ZIP::CheckArchive(LPCTSTR _szFileName)
{
	if(!ArchiverCheckArchive){
		ASSERT(ArchiverCheckArchive);
		return false;
	}
	//7-Zip32.dll�́A�w�b�_�Í����t�@�C����CheckArchive���A�E�B���h�E�n���h�������݂��Ȃ��Ǝ��s����
	CDummyWindow dummy;
	dummy.Create(NULL,CWindow::rcDefault);
	BOOL bRet=ArchiverCheckArchive(C2UTF8(_szFileName),CHECKARCHIVE_BASIC);
	dummy.DestroyWindow();
	return bRet;
}

int CArchiver7ZIP::GetFileCount(LPCTSTR _szFileName)
{
	if(!ArchiverGetFileCount){
		return -1;
	}
	return ArchiverGetFileCount(C2UTF8(_szFileName));
}


bool CArchiver7ZIP::InspectArchiveBegin(LPCTSTR ArcFileName,CConfigManager&)
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
	m_hInspectArchive=ArchiverOpenArchive(NULL,C2UTF8(ArcFileName),m_dwInspectMode);
	if(!m_hInspectArchive){
		TRACE(_T("Failed to Open Archive\n"));
		return false;
	}
	m_bInspectFirstTime=true;

	FILL_ZERO(m_IndividualInfo);
	return true;
}

bool CArchiver7ZIP::InspectArchiveGetFileName(CString &FileName)
{
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
}

bool CArchiver7ZIP::InspectArchiveGetMethodString(CString &strMethod)
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
	UtilToUNICODE(strMethod,(LPCBYTE)szBuffer,9,UTILCP_UTF8);
	return true;
}

bool CArchiver7ZIP::AddItemToArchive(LPCTSTR ArcFileName,bool bEncrypted,const std::list<CString> &FileList,CConfigManager &ConfMan,LPCTSTR lpDestDir,CString &strLog)
{
	// ���X�|���X�t�@�C���p�e���|�����t�@�C�����擾
	TCHAR ResponceFileName[_MAX_PATH+1];
	FILL_ZERO(ResponceFileName);
	if(!UtilGetTemporaryFileName(ResponceFileName,_T("zip"))){
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

	int option=0;
	if(bEncrypted){
		option |= COMPRESS_PASSWORD;
	}
	CConfigZIP confZIP;
	CConfig7Z  conf7Z;
	ASSERT(ArchiverGetArchiveType);
	switch(ArchiverGetArchiveType(C2UTF8(ArcFileName))){
	case 1:	//ZIP�`���ň��k
		confZIP.load(ConfMan);
		if(!FormatCompressCommandZIP(confZIP,Param,false,option,NULL,NULL,strLog)){
			DeleteFile(ResponceFileName);
			return false;
		}
		break;
	case 2:	//7z�`���ň��k
		conf7Z.load(ConfMan);
		if(!FormatCompressCommand7Z(conf7Z,Param,option,NULL,NULL,strLog)){
			DeleteFile(ResponceFileName);
			return false;
		}
		break;
	default:
		ASSERT(!"This code cannot be run");
		//�G���[�������ʓ|�Ȃ̂ŕ����Ă����B
		return false;
	}

	Param+=_T("-scsUTF-8 ");	//���X�|���X�t�@�C���̃R�[�h�y�[�W�w��

	//��ƃf�B���N�g��
	Param+=_T("\"-w");
	Param+=UtilGetTempPath();
	Param+=_T("\" ");

	//���k��t�@�C�����w��
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
	std::vector<BYTE> szLog(LOG_BUFFER_SIZE);
	szLog[0]='\0';
	int Ret=ArchiveHandler(NULL,C2UTF8(Param),(LPSTR)&szLog[0],LOG_BUFFER_SIZE-1);
	CString strTmp;
	UtilToUNICODE(strTmp,&szLog[0],szLog.size()-1,UTILCP_UTF8);
	//strLog=&szLog[0];
	strLog=strTmp;

	//�g�������X�|���X�t�@�C���͏���
	DeleteFile(ResponceFileName);

	return 0==Ret;
}


//�w�肳�ꂽ�f�B���N�g��������W�J����;��������
bool CArchiver7ZIP::ExtractDirectoryEntry(LPCTSTR lpszArcFile,CConfigManager &ConfMan,const ARCHIVE_ENTRY_INFO_TREE* lpBase,const ARCHIVE_ENTRY_INFO_TREE* lpDir,LPCTSTR lpszOutputBaseDir,bool bCollapseDir,CString &strLog)
{
	//---�ꎞ�t�H���_���ɂ܂Ƃ߂ēW�J���A�ォ��t�H���_�\����؂�o��
	std::list<CString> files;

	CString strPath;

	bool bRestoreDir=false;
	if(lpDir->strFullPath.IsEmpty()){
		//�f�B���N�g�����o�^����Ă��Ȃ��̂Ńp�X�����Z�o����
		ArcEntryInfoTree_GetNodePathRelative(lpDir,lpBase,strPath);
		strPath.Replace(_T('/'),_T('\\'));

		CPath tmpPath(strPath);
		tmpPath.RemoveBackslash();	//�f�B���N�g���������症�ɂ���
		tmpPath.RemoveFileSpec();	//�e�f�B���N�g���܂Ő؂�߂�
		tmpPath.Append(_T("*"));	//����f�B���N�g���ȉ��̑S�Ẵt�@�C����W�J
		files.push_back(tmpPath);

		bRestoreDir=true;
	}else{
		//�f�B���N�g�����A�[�J�C�u���ɃG���g���Ƃ��ēo�^����Ă���Ȃ�o�͂���
		CString tmpPath(lpDir->strFullPath);
		if(tmpPath[tmpPath.GetLength()-1]==_T('\\')||tmpPath[tmpPath.GetLength()-1]==_T('/')){
			//������\��������/���폜
			tmpPath.Delete(tmpPath.GetLength()-1);
		}
		files.push_back(tmpPath);

		strPath=lpDir->strFullPath;
		strPath.Replace(_T('/'),_T('\\'));
	}

	//--------------------------------------
	// �C�����ꂽ�o�̓f�B���N�g���p�X���Z�o
	//--------------------------------------
	//---�{���̏o�͐�
	CString strOutputDir=lpszOutputBaseDir+strPath;
	if(strOutputDir.GetLength()>_MAX_PATH){
		//�t�H���_���������Ȃ肷����
		strLog=CString(MAKEINTRESOURCE(IDS_ERROR_MAX_PATH));
		return false;
	}

	//FileList�ɂ͂P�����������Ă��Ȃ��͂�
	ASSERT(files.size()==1);

	//�ꎞ�t�H���_
	CTemporaryDirectoryManager tdm(_T("lhaf"));
	CPath strTempOutput(tdm.GetDirPath());
	if(bRestoreDir){	//�f�B���N�g�������Ƃŕ���
		strTempOutput+=strPath;
		strTempOutput.AddBackslash();
		TRACE(_T("���Ƃł̃f�B���N�g������:%s\n"),(LPCTSTR)strTempOutput);
		if(!UtilMakeSureDirectoryPathExists(strTempOutput)){
			strLog.Format(IDS_ERROR_CANNOT_MAKE_DIR,strTempOutput);
			return false;
		}
	}
	//---�ꎞ�t�H���_���ɂ܂Ƃ߂ēW�J���A�ォ��t�H���_�\����؂�o��
	// �t�@�C����W�J
	if(!ExtractSpecifiedOnly(lpszArcFile,ConfMan,strTempOutput,files,strLog,true)){
		return false;
	}

	//���葤�t�@�C�����w��
	CPath tmp(strTempOutput);
	tmp+=(LPCTSTR)strPath;	//PathAppend����
	tmp.RemoveBackslash();
	CString strSrcFiles(tmp);
	strSrcFiles+=_T("||");
	//�󂯑��t�@�C�����w��
	tmp=lpszOutputBaseDir;
	{
		CString strTmp;
		ArcEntryInfoTree_GetNodePathRelative(lpDir,lpBase,strTmp);
		strTmp.Replace(_T('/'),_T('\\'));
		tmp+=(LPCTSTR)strTmp;
	}
	tmp.AddBackslash();
	CString strDestFiles(tmp);
	strDestFiles+=_T("||");

	//'|'��'\0'�ɕϊ�����
	std::vector<TCHAR> srcBuf(strSrcFiles.GetLength()+1);
	UtilMakeFilterString(strSrcFiles,&srcBuf[0],srcBuf.size());
	std::vector<TCHAR> destBuf(strDestFiles.GetLength()+1);
	UtilMakeFilterString(strDestFiles,&destBuf[0],destBuf.size());

	//�t�@�C��������e
	SHFILEOPSTRUCT fileOp={0};
	fileOp.wFunc=FO_MOVE;
	fileOp.fFlags=FOF_MULTIDESTFILES|/*FOF_NOCONFIRMATION|*/FOF_NOCONFIRMMKDIR|FOF_NOCOPYSECURITYATTRIBS|FOF_NO_CONNECTED_ELEMENTS;
	fileOp.pFrom=&srcBuf[0];
	fileOp.pTo=&destBuf[0];

	//�ړ����s
	if(::SHFileOperation(&fileOp)){
		//�G���[
		strLog=CString(MAKEINTRESOURCE(IDS_ERROR_FILE_MOVE));
		return false;
	}else if(fileOp.fAnyOperationsAborted){
		//�L�����Z��
		strLog=CString(MAKEINTRESOURCE(IDS_ERROR_USERCANCEL));
		return false;
	}

	return true;

}

