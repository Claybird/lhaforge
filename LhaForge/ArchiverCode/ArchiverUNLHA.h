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

#pragma once
#include "arc_interface.h"

//---UNICODE�p��ArchiverDLL��`
typedef struct {
	DWORD 			dwOriginalSize;		/* �t�@�C���̃T�C�Y */
 	DWORD 			dwCompressedSize;	/* ���k��̃T�C�Y */
	DWORD			dwCRC;				/* �i�[�t�@�C���̃`�F�b�N�T�� */
	UINT			uFlag;				/* �������� */
										/* Status flag */
	UINT			uOSType;			/* ���ɍ쐬�Ɏg��ꂽ OS */
	WORD			wRatio;				/* ���k�� */
	WORD			wDate;				/* �i�[�t�@�C���̓��t(DOS �`��) */
	WORD 			wTime;				/* �i�[�t�@�C���̎���(�V) */
	WCHAR			szFileName[FNAME_MAX32 + 1];	/* �i�[�t�@�C���� */
	WCHAR			dummy1[3];
	WCHAR			szAttribute[8];		/* �i�[�t�@�C���̑���(���ɌŗL) */
	WCHAR			szMode[8];			/* �i�[�t�@�C���̊i�[���[�h(�V) */
}	INDIVIDUALINFOW, *LPINDIVIDUALINFOW;


typedef int   (WINAPI *COMMON_ARCHIVER_HANDLERW)(const HWND,LPCWSTR,LPWSTR,const DWORD);
typedef BOOL  (WINAPI *COMMON_ARCHIVER_CHECKARCHIVEW)(LPCWSTR,const int);
typedef int	  (WINAPI *COMMON_ARCHIVER_GETFILENAMEW)(HARC,LPCWSTR,int);
typedef HARC  (WINAPI *COMMON_ARCHIVER_OPENARCHIVEW)(const HWND,LPCWSTR,const DWORD);
typedef int   (WINAPI *COMMON_ARCHIVER_FINDFIRSTW)(HARC,LPCWSTR,LPINDIVIDUALINFOW);
typedef int   (WINAPI *COMMON_ARCHIVER_FINDNEXTW)(HARC,LPINDIVIDUALINFOW);
typedef int	  (WINAPI *COMMON_ARCHIVER_GETFILECOUNTW)(LPCWSTR);
typedef int   (WINAPI *COMMON_ARCHIVER_GETMETHODW)(HARC,LPWSTR,const int);


enum LZH_COMPRESS_TYPE{
	LZH_COMPRESS_LH5,
	LZH_COMPRESS_LH0,
	LZH_COMPRESS_LH1,
	LZH_COMPRESS_LH6,
	LZH_COMPRESS_LH7,

	ENUM_COUNT_AND_LASTITEM(LZH_COMPRESS_TYPE),
};

struct CConfigLZH;
class CArchiverUNLHA:public CArchiverDLL{
protected:
	INDIVIDUALINFOW m_IndividualInfoW;

	COMMON_ARCHIVER_HANDLERW		ArchiveHandlerW;
	COMMON_ARCHIVER_CHECKARCHIVEW	ArchiverCheckArchiveW;
	COMMON_ARCHIVER_GETFILENAMEW	ArchiverGetFileNameW;
	COMMON_ARCHIVER_OPENARCHIVEW	ArchiverOpenArchiveW;
	COMMON_ARCHIVER_FINDFIRSTW		ArchiverFindFirstW;
	COMMON_ARCHIVER_FINDNEXTW		ArchiverFindNextW;
	COMMON_ARCHIVER_GETFILECOUNTW	ArchiverGetFileCountW;
	COMMON_ARCHIVER_GETMETHODW		ArchiverGetMethodW;

	void WriteResponceFile(HANDLE,LPCTSTR);
	void FormatCompressCommand(const CConfigLZH& Config,LPCTSTR lpszMethod,CString &Param);
public:
	CArchiverUNLHA();
	virtual ~CArchiverUNLHA();
	virtual bool Compress(LPCTSTR,std::list<CString>&,CConfigManager&,const PARAMETER_TYPE,int,LPCTSTR,LPCTSTR,LPCTSTR,CString &)override;
	virtual bool Extract(LPCTSTR,CConfigManager&,const CConfigExtract&,bool,LPCTSTR,CString &)override;
	virtual bool ExtractSpecifiedOnly(LPCTSTR ArcFileName,CConfigManager&,LPCTSTR OutputDir,std::list<CString>&,CString &,bool bUsePath=false)override;
	virtual bool ExamineArchive(LPCTSTR,CConfigManager&,bool,bool&,bool&,CString&,CString &strErr)override;

	//�A�[�J�C�u����t�@�C�����폜
	virtual bool DeleteItemFromArchive(LPCTSTR ArcFileName,CConfigManager&,const std::list<CString>&,CString &strLog)override;
	virtual bool QueryDeleteItemFromArchiveSupported(LPCTSTR ArcFileName)const override{return true;}		//DeleteFile���T�|�[�g����Ă��邩�ǂ���

	virtual ARCRESULT TestArchive(LPCTSTR,CString &)override;	//�A�[�J�C�u�����������ǂ����`�F�b�N����

	//�A�[�J�C�u�Ɏw�肵���t�@�C����ǉ�
	virtual bool AddItemToArchive(LPCTSTR ArcFileName,bool bEncrypted,const std::list<CString>&,CConfigManager&,LPCTSTR lpDestDir,CString&)override;
	virtual bool QueryAddItemToArchiveSupported(LPCTSTR ArcFileName)const override{return true;}


	//---UNICODE
	virtual LOAD_RESULT LoadDLL(CConfigManager&,CString &strErr)override;
	virtual void FreeDLL()override;
	virtual bool IsUnicodeCapable()const override{return true;}	//UNICODE�Ή�DLL�Ȃ�true��Ԃ�
	virtual BOOL CheckArchive(LPCTSTR)override;
	virtual int GetFileCount(LPCTSTR)override;	//�A�[�J�C�u���̃t�@�C������Ԃ�

	virtual bool InspectArchiveBegin(LPCTSTR,CConfigManager&)override;				//���ɓ������J�n
	//virtual bool InspectArchiveEnd();						//���ɓ������I��
	virtual bool InspectArchiveGetFileName(CString&)override;		//���ɓ��t�@�C�����擾
	virtual bool InspectArchiveNext()override;						//���ɓ����������̃t�@�C���ɐi�߂�
	//�I�[�o�[���C�h�̕K�v�Ȃ�;UNLHA32.dll�ł�INDIVIDUALINFO���Q�Ƃ��Ȃ��̂Ő��������삷��
	//bool InspectArchiveGetOriginalFileSize(LARGE_INTEGER &FileSize);
	//bool InspectArchiveGetCompressedFileSize(LARGE_INTEGER &FileSize);
	//bool InspectArchiveGetWriteTime(FILETIME &FileTime);
	virtual DWORD InspectArchiveGetCRC()override;					//���ɓ��t�@�C��CRC�擾
	virtual WORD InspectArchiveGetRatio()override;					//���ɓ��t�@�C�����k���擾
	virtual bool InspectArchiveGetMethodString(CString&)override;			//���ɓ��t�@�C���i�[���[�h�擾
};
