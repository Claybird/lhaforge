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
#include "ArchiverJACK.h"
#include "../resource.h"
#include "../ConfigCode/ConfigExtract.h"
#include "../ConfigCode/ConfigJACK.h"
#include "../Dialogs/JackVolumeSizeDlg.h"
#include "../Utilities/OSUtil.h"

CArchiverJACK::CArchiverJACK()
{
	m_nRequiredVersion=20;
	m_strDllName=_T("Jack32.dll");
	m_AstrPrefix="Jack";
	m_LoadLevel=LOAD_DLL_MINIMUM;
}

CArchiverJACK::~CArchiverJACK()
{
	FreeDLL();
}

/*
format�̎w��́AB2E32.dll�ł̂ݗL��
level�̎w��́AB2E32.dll�ȊO�ŗL��
*/
bool CArchiverJACK::Compress(LPCTSTR ArcFileName,std::list<CString> &ParamList,CConfigManager &ConfMan,const PARAMETER_TYPE Type,int Options,LPCTSTR lpszFormat,LPCTSTR,LPCTSTR,CString &strLog)
{
	LPCTSTR lpszSplitSize = lpszFormat;

	if(!IsOK()){
		return false;
	}

	//ArcFileName�͏o�͐�t�H���_��
	ASSERT(0!=_tcslen(ArcFileName));
	TRACE(_T("ArcFileName=%s\n"),ArcFileName);

	//===========================
	// DLL�ɓn���I�v�V�����̐ݒ�
	//===========================
	TRACE(_T("DLL�ɓn���I�v�V�����̐ݒ�\n"));

	CString Param;//�R�}���h���C�� �p�����[�^ �o�b�t�@

	CConfigJACK Config;
	Config.load(ConfMan);
	Param+=_T("-r ");	//����
	if(Options&COMPRESS_SFX){
		Param+=_T("-m1 ");	//SFX
	}
	else{
		Param+=_T("-m0 ");	//�ʏ�
	}
	if(lpszSplitSize && _tcslen(lpszSplitSize)>0){
		CString Buf;
		Buf.Format(_T("-v:%s "),lpszSplitSize);
		Param+=Buf;//�����T�C�Y�w��
	}else if(Config.SpecifyVolumeSizeAtCompress){
		CJackVolumeSizeDialog vsd;
		if(IDOK!=vsd.DoModal()){
			return false;
		}
		CString Buf;
		Buf.Format(_T("-v:%d "),vsd.VolumeSize);
		Param+=Buf;//�����T�C�Y�w��
	}else{
		CString Buf;
		Buf.Format(_T("-v:%d "),Config.VolumeSize);
		Param+=Buf;//�����T�C�Y�w��
	}

	//�����Ώۃt�@�C�����w��
	Param+=_T("\"");
	Param+=*ParamList.begin();
	Param+=_T("\" ");

	//�o�̓t�H���_�w��
	Param+=_T("\"");
	Param+=ArcFileName;
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

bool CArchiverJACK::Extract(LPCTSTR ArcFileName,CConfigManager&,const CConfigExtract &Config,bool bSafeArchive,LPCTSTR OutputDir,CString &strLog)
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

	if(!Config.ForceOverwrite){//�㏑���m�F����
		CString strFileName;

		//�t�@�C����ǂ�Ŋi�[�t�@�C�������擾
		if(!GetContainedFileName(ArcFileName,strFileName)){
			//�s���ȃt�@�C��:CheckArchive()�Œe����Ă���Ɗ��҂ł���
			return false;
		}

		//--------------
		// ���݃`�F�b�N
		//--------------
		strFileName.Insert(0,OutputDir);
		if(PathFileExists(strFileName)){
			CString msg;
			msg.Format(IDS_CONFIRM_OVERWRITE_MESSAGE_SIMPLE,strFileName);
			if(IDYES!=MessageBox(NULL,msg,CString(MAKEINTRESOURCE(IDS_MESSAGE_CAPTION)),MB_YESNO|MB_ICONQUESTION)){
				return false;
			}
		}
	}


	CString Param;//�R�}���h���C�� �p�����[�^ �o�b�t�@

	//�����p�����[�^
	Param+=_T("-c ");	//����

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
	//char szLog[LOG_BUFFER_SIZE]={0};
	std::vector<char> szLog(LOG_BUFFER_SIZE);
	szLog[0]='\0';
	int Ret=ArchiveHandler(NULL,CT2A(Param),&szLog[0],LOG_BUFFER_SIZE-1);
	strLog=&szLog[0];

	return 0==Ret;
}


bool CArchiverJACK::ExtractSpecifiedOnly(LPCTSTR ArcFileName,CConfigManager&,LPCTSTR OutputDir,std::list<CString>&,CString &,bool bUsePath)
{
	return false;
}

//=========================================================
// DTV�̉\���̂���t�@�C�����ǂ������ڊm�F����
//=========================================================
bool CArchiverJACK::ExamineArchive(LPCTSTR ArcFileName,CConfigManager& ConfMan,bool,bool &bInFolder,bool &bSafeArchive,CString &BaseDir,CString &strErr)
{
	bInFolder=false;
	bSafeArchive=false;

	CString strFileName;

	//�t�@�C����ǂ�Ŋi�[�t�@�C�������擾
	if(!GetContainedFileName(ArcFileName,strFileName))return false;
	if(-1==strFileName.FindOneOf(_T(":\\/"))){//�p�X�w��̕������܂܂�Ă��Ȃ��̂ň��S
		bSafeArchive=true;
	}

/*
	���̃R�[�h�ł͉𓀂���t�@�C���A������t�@�C�����e���m�F���Ă��Ȃ��B
	����ł���肪�Ȃ����R�́AJACK32.dll�͓W�J���ɏo�̓t�@�C�����̈�ѐ����`�F�b�N���Ă���͗l������ł���B
	[����]
	�En-1�Ԗڂ܂ł̃t�@�C���ɂ͍׍H������Ă��Ȃ�
		�En�Ԗڂ̃t�@�C�����׍H����Ă��Ȃ�
			������𓀁An++
		�En�Ԗڂ̃t�@�C�����׍H����Ă���
			�En-1�Ԗڂ܂ł̃t�@�C����LhaForge�ɗ^����
				�����S���Ƃ��ĉ𓀂��n�߂����̂́AJAK�̓��ꐫ�`�F�b�N�ɂ�����
			�En�Ԗڂ̃t�@�C����LhaForge�ɗ^����
				��LhaForge��DTV�`�F�b�N�ɂ�����

	[���ӓ_]
	���݂̃R�[�h�ł́A�㏑���m�F�@�\���g���Ƃ��A����JAK�t�@�C�����Q��ǂނ��ƂɂȂ�B
	���������߂�Ȃ�A�����̃R�[�h�𖳌������āAExtract()�����ň��S���ǂ����`�F�b�N����悤�ɂ���΂悢�B
	���܂̂Ƃ���́A�����������ʂ��̗ǂ���D�悵�Ă���B
*/

	return true;
}

//�w�b�_����
// Original:JakTool.cpp/XacRett #49/(C)k.inaba
int CArchiverJACK::FindHeader(const BYTE* hdr,DWORD size)
{
	static const char Magic[]="This_Is_Jack_File";

	if(size<sizeof_JakHeader)
		return -1;

	if(0==strcmp((char*)hdr,Magic)){
		if(sizeof_JakHeader+((JakHeader*)hdr)->FileNameLen<size)
			return 0;
	}
	else if(hdr[0]=='M' && hdr[1]=='Z')	//���ȉ�
	{
		DWORD prev = 0xffffffff;
		for( DWORD i=0; i<size-sizeof_JakHeader; i++ )
		{
			if( hdr[i]!='T' )continue;
			if( hdr[i+1]!='h' )continue;
			if( hdr[(++i)+1]!='i' )continue;
			if( hdr[(++i)+1]!='s' )continue;
			if( hdr[(++i)+1]!='_' )continue;
			if( 0!=strcmp((char*)hdr+(++i)+1,Magic+5) )continue;

			// �X�^�u���̕�����Ɉ��������邱�Ƃ�����̂ŁA
			// ���"This_Is_..."������Ƃ��͈��Skip
			if( prev==0xffffffff )
				prev = (i-4);
			else
				return (i-4);
		}
		if(prev!=0xffffffff)
			return prev;
	}
	return -1;
}

bool CArchiverJACK::GetContainedFileName(LPCTSTR ArcFileName,CString &strFileName)
{
	strFileName.Empty();

	//�t�@�C�����e�o�b�t�@
	std::vector<BYTE> Buffer;
	//�t�@�C���I�[�v��
	HANDLE hFile=CreateFile(ArcFileName,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
	if(INVALID_HANDLE_VALUE==hFile)return false;

	DWORD dwSize=GetFileSize(hFile,NULL);
		//4GB���傫�ȃt�@�C���T�C�Y�͎擾�ł��Ȃ����A�����I�ɂ͂��̂悤�ȑ傫�ȃt�@�C���͈���Ȃ�(JACK�͕����p)
	Buffer.resize(dwSize);

	DWORD dwRead=0;
	if(dwSize<0){
		CloseHandle(hFile);
		return false;
	}

	//�ǂݎ��
	ReadFile(hFile,&Buffer[0],dwSize,&dwRead,NULL);
	CloseHandle(hFile);

	//�w�b�_����
	int iPos=FindHeader(&Buffer[0],dwRead);
	if(-1==iPos){
		//Not Found
		return false;
	}

	JakHeader* lpHeader=(JakHeader*)&Buffer[iPos];

	for(unsigned int i=0;i<lpHeader->FileNameLen;i++){
		strFileName+=Buffer[iPos+sizeof_JakHeader+i];
	}
	return true;
}
