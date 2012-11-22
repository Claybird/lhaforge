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

#pragma once
#include "../resource.h"
#include "ArcEntryInfo.h"
#include "../ConfigCode/ConfigManager.h"

typedef	HGLOBAL	HARC;
#ifndef FNAME_MAX32
#define FNAME_MAX32		512
#endif
typedef struct {
	DWORD 			dwOriginalSize;		/* �t�@�C���̃T�C�Y */
 	DWORD 			dwCompressedSize;	/* ���k��̃T�C�Y */
	DWORD			dwCRC;				/* �i�[�t�@�C���̃`�F�b�N�T�� */
	UINT			uFlag;				/* �������� */
	UINT			uOSType;			/* ���ɍ쐬�Ɏg��ꂽ�n�r */
	WORD			wRatio;				/* ���k�� */
	WORD			wDate;				/* �i�[�t�@�C���̓��t(DOS �`��) */
	WORD 			wTime;				/* �i�[�t�@�C���̎���(�V) */
	char			szFileName[FNAME_MAX32 + 1];	/* ���ɖ� */
	char			dummy1[3];
	char			szAttribute[8];		/* �i�[�t�@�C���̑���(���ɌŗL) */
	char			szMode[8];			/* �i�[�t�@�C���̊i�[���[�h(�V) */
}	INDIVIDUALINFO, *LPINDIVIDUALINFO;

typedef int   (WINAPI *COMMON_ARCHIVER_HANDLER)(const HWND,LPCSTR,LPSTR,const DWORD);
typedef WORD  (WINAPI *COMMON_ARCHIVER_GETVERSION)(VOID);	//GetVersion/GetSubVersion
typedef BOOL  (WINAPI *COMMON_ARCHIVER_CHECKARCHIVE)(LPCSTR,const int);

typedef int	  (WINAPI *COMMON_ARCHIVER_GETFILENAME)(HARC,LPCSTR,int);
typedef BOOL  (WINAPI *COMMON_ARCHIVER_QUERYFUNCTIONLIST)(const int);
typedef HARC  (WINAPI *COMMON_ARCHIVER_OPENARCHIVE)(const HWND,LPCSTR,const DWORD);
typedef HARC  (WINAPI *COMMON_ARCHIVER_OPENARCHIVE2)(const HWND,LPCSTR,const DWORD,LPCSTR);
typedef int   (WINAPI *COMMON_ARCHIVER_CLOSEARCHIVE)(HARC);
typedef int   (WINAPI *COMMON_ARCHIVER_FINDFIRST)(HARC,LPCSTR,LPINDIVIDUALINFO);
typedef int   (WINAPI *COMMON_ARCHIVER_FINDNEXT)(HARC,LPINDIVIDUALINFO);
typedef int   (WINAPI *COMMON_ARCHIVER_GETATTRIBUTE)(HARC);
typedef BOOL  (WINAPI *COMMON_ARCHIVER_GETORIGINALSIZEEX)(HARC,LARGE_INTEGER*);
typedef DWORD (WINAPI *COMMON_ARCHIVER_GETWRITETIME)(HARC);
typedef BOOL  (WINAPI *COMMON_ARCHIVER_GETWRITETIMEEX)(HARC,LPFILETIME);
typedef int	  (WINAPI *COMMON_ARCHIVER_GETFILECOUNT)(LPCSTR);
typedef int   (WINAPI *COMMON_ARCHIVER_GETMETHOD)(HARC,LPSTR,const int);

typedef BOOL   (WINAPI *COMMON_ARCHIVER_SETUNICODEMODE)(BOOL);


#define	CHECKARCHIVE_RAPID		0
#define	CHECKARCHIVE_BASIC		1
#define	CHECKARCHIVE_FULLCRC	2

#define CHECKARCHIVE_RECOVERY	4   /* �j���w�b�_��ǂݔ�΂��ď��� */
#define CHECKARCHIVE_SFX		8	/* SFX ���ǂ�����Ԃ� */
#define CHECKARCHIVE_ALL		16	/* �t�@�C���̍Ō�܂Ō������� */
#define CHECKARCHIVE_ENDDATA	32	/* ���ɂ����̗]��f�[�^������ */

#define	CHECKARCHIVE_NOT_ASK_PASSWORD	64

#define ERROR_PASSWORD_FILE		0x800A


enum PARAMETER_TYPE;

enum LOAD_DLL_LEVEL{
	LOAD_DLL_STANDARD	=0x00000001L,					//�ʏ�
	LOAD_DLL_MINIMUM	=0x00000002L,					//OpenArchive*���g���Ȃ�DLL�p
	LOAD_DLL_SIMPLE_INSPECTION	=0x00000004L,				//%Prefix%(),OpenArchive(),FindFile*()�ȊO���g���Ȃ�DLL(BGA32.DLL)�p
//	LOAD_DLL_STANDARD_WITHOUT_GETATTRIBUTE=0x00000004L,	//GetAttribute���g���Ȃ�DLL�p�̒ʏ�
};

enum LOAD_RESULT{
	LOAD_RESULT_OK,			//DLL�͐���Ƀ��[�h���ꂽ
	LOAD_RESULT_NOT_FOUND,	//DLL��������Ȃ�
	LOAD_RESULT_INVALID,	//�s����DLL
	LOAD_RESULT_TOO_OLD		//DLL�̓T�|�[�g����Ă���o�[�W�������Â�
};

const int LOG_BUFFER_SIZE=512*1024;	//512KB

enum COMPRESS_MODE{
	COMPRESS_SFX				=	0x00000001L,
	COMPRESS_PASSWORD			=	0x00000002L,
	COMPRESS_PUBLIC_PASSWORD	=	0x00000004L,
	COMPRESS_SPLIT				=	0x00000008L,
};

//���k�`���p�����[�^
enum PARAMETER_TYPE{
	PARAMETER_UNDEFINED,
	PARAMETER_LZH,
	PARAMETER_ZIP,
	PARAMETER_CAB,
	PARAMETER_7Z,
	PARAMETER_JACK,
//	PARAMETER_BH,
	PARAMETER_HKI,
	PARAMETER_YZ1,
//	PARAMETER_YZ2,
	PARAMETER_BZA,
	PARAMETER_GZA,
	PARAMETER_ISH,
	PARAMETER_UUE,

	PARAMETER_TAR,
	PARAMETER_BZ2,
	PARAMETER_GZ,
	PARAMETER_TAR_GZ,	//tar.gz,tgz
	PARAMETER_TAR_BZ2,	//tar.bz2,tbz
	PARAMETER_XZ,
	PARAMETER_TAR_XZ,	//tar.xz,txz
	PARAMETER_LZMA,
	PARAMETER_TAR_LZMA,	//tar.lzma

	PARAMETER_B2E,	//B2E�͓��ʈ���

	ENUM_COUNT_AND_LASTITEM(PARAMETER),
};

//Compress/Extract/TestArchive�̖߂�l
enum ARCRESULT{
	//---�𓀌n
	EXTRACT_OK,//����I��
	EXTRACT_NG,//�ُ�I��
	EXTRACT_CANCELED,//�L�����Z��
	EXTRACT_NOTARCHIVE,//���k�t�@�C���ł͂Ȃ�
	EXTRACT_INFECTED,//�E�B���X�̉\������

	//---�����n
	TEST_OK,	//�t�@�C���͐���
	TEST_NG,	//�t�@�C���Ɉُ킠��
	TEST_NOTIMPL,//�����͎�������Ă��Ȃ�
	TEST_NOTARCHIVE,//���k�t�@�C���ł͂Ȃ�
	TEST_INFECTED,//�E�B���X�̉\������
	TEST_ERROR,	//�����G���[(DLL�����[�h����Ă��Ȃ��̂ɌĂяo���ꂽ�A��)
};

struct ARCLOG{	//�A�[�J�C�u����̌��ʂ��i�[����
	virtual ~ARCLOG(){}
	CString strFile;	//�A�[�J�C�u�̃t���p�X
	CString strMsg;		//���O
	ARCRESULT Result;	//����
};


enum LOGVIEW{
	LOGVIEW_ON_ERROR,
	LOGVIEW_ALWAYS,
	LOGVIEW_NEVER,

	ENUM_COUNT_AND_LASTITEM(LOGVIEW),
};
enum LOSTDIR{
	LOSTDIR_ASK_TO_CREATE,
	LOSTDIR_FORCE_CREATE,
	LOSTDIR_ERROR,

	ENUM_COUNT_AND_LASTITEM(LOSTDIR),
};
enum OUTPUT_TO{
	OUTPUT_TO_DESKTOP,
	OUTPUT_TO_SAME_DIR,
	OUTPUT_TO_SPECIFIC_DIR,
	OUTPUT_TO_ALWAYS_ASK_WHERE,

	ENUM_COUNT_AND_LASTITEM(OUTPUT_TO),
};
enum CREATE_OUTPUT_DIR{
	CREATE_OUTPUT_DIR_ALWAYS,
	CREATE_OUTPUT_DIR_SINGLE,
	CREATE_OUTPUT_DIR_NEVER,

	ENUM_COUNT_AND_LASTITEM(CREATE_OUTPUT_DIR)
};

struct CConfigExtract;
//�����A�[�J�C�oDLL���b�v�p�N���X�̃x�[�X
class CArchiverDLL{
protected:
	//DLL�ŗL�̃f�[�^
	DWORD			m_dwInspectMode;		//OpenArchive�̃��[�h
	CString			m_strDllName;			//DLL��
	CStringA		m_AstrPrefix;				//�֐��v���t�B�b�N�X
	CStringA		m_AstrFindParam;			//InspectArchiveFileNext()�̈���
	LOAD_DLL_LEVEL	m_LoadLevel;			//LoadDLL�ŗv������֐��̃��x��
	WORD			m_nRequiredVersion;	//LhaForge���T�|�[�g����DLL�̍Œ�o�[�W����
	WORD			m_nRequiredSubVersion;	//LhaForge���T�|�[�g����DLL�̍Œ�T�u�o�[�W����

	//---------
	//���ɓ������̏�Ԃ��L�^���邽�߂̕ϐ�
	HARC m_hInspectArchive;
	bool m_bInspectFirstTime;
	INDIVIDUALINFO m_IndividualInfo;

	//---------
	HINSTANCE	m_hInstDLL;			//DLL�C���X�^���X
	COMMON_ARCHIVER_HANDLER			ArchiveHandler;	//Un???�֐�
	COMMON_ARCHIVER_GETVERSION		ArchiverGetVersion;
	COMMON_ARCHIVER_GETVERSION		ArchiverGetSubVersion;
	COMMON_ARCHIVER_CHECKARCHIVE	ArchiverCheckArchive;
	COMMON_ARCHIVER_GETFILECOUNT	ArchiverGetFileCount;
	//�ȉ��̊֐��͓�d�t�H���_���肨��ъ댯�A�[�J�C�u����ȂǃA�[�J�C�u�������Ɏg��
	COMMON_ARCHIVER_QUERYFUNCTIONLIST		ArchiverQueryFunctionList;
	COMMON_ARCHIVER_GETFILENAME				ArchiverGetFileName;
	COMMON_ARCHIVER_OPENARCHIVE				ArchiverOpenArchive;
	COMMON_ARCHIVER_CLOSEARCHIVE			ArchiverCloseArchive;
	COMMON_ARCHIVER_FINDFIRST				ArchiverFindFirst;
	COMMON_ARCHIVER_FINDNEXT				ArchiverFindNext;
	COMMON_ARCHIVER_GETATTRIBUTE			ArchiverGetAttribute;
	COMMON_ARCHIVER_GETORIGINALSIZEEX		ArchiverGetOriginalSizeEx;
	COMMON_ARCHIVER_GETORIGINALSIZEEX		ArchiverGetCompressedSizeEx;
	COMMON_ARCHIVER_GETWRITETIME			ArchiverGetWriteTime;
	COMMON_ARCHIVER_GETWRITETIMEEX			ArchiverGetWriteTimeEx;
	COMMON_ARCHIVER_GETMETHOD				ArchiverGetMethod;

	virtual bool _ExamineArchive(LPCTSTR,CConfigManager&,bool &bInFolder,bool &bSafeArchive,const int,CString&,CString &strErr);
	virtual bool _ExamineArchiveFast(LPCTSTR,CConfigManager&,bool &bInFolder,CString&,CString &strErr);

	virtual bool ExtractSubDirectories(LPCTSTR lpszArcFile,CConfigManager&,const ARCHIVE_ENTRY_INFO_TREE* lpBase,const std::list<ARCHIVE_ENTRY_INFO_TREE*>&,LPCTSTR lpszOutputDir,bool bCollapseDir,CString &strLog);
	virtual bool ExtractDirectoryEntry(LPCTSTR lpszArcFile,CConfigManager&,const ARCHIVE_ENTRY_INFO_TREE* lpBase,const ARCHIVE_ENTRY_INFO_TREE* lpDir,LPCTSTR lpszOutputBaseDir,bool bCollapseDir,CString &strLog);

	//���X�|���X�t�@�C���Ƀf�[�^����������:MBCS,�G�X�P�[�v��K�v�Ƃ��Ȃ����̌���
	virtual void WriteResponceFile(HANDLE,LPCTSTR,bool bQuoteSpaces=true);
public:
	CArchiverDLL();
	virtual ~CArchiverDLL(){};
	virtual LOAD_RESULT LoadDLL(CConfigManager&,CString &strErr);
	virtual void FreeDLL();
	virtual WORD GetVersion()const;
	virtual WORD GetSubVersion()const;
	virtual bool IsUnicodeCapable()const{return false;}	//UNICODE�Ή�DLL�Ȃ�true��Ԃ�
	virtual bool IsWeakCheckArchive()const{return false;}	//CheckArchive�̋@�\���n��(UNBEL/AISH�̂悤��)�Ȃ�true
	virtual bool IsWeakErrorCheck()const{return false;}	//%Prefix%()�̃G���[�`�F�b�N���Â�(XacRett�̂悤��)�Ȃ�true;�𓀌�ɍ폜���邩�̔��f�Ɏg�p
	virtual BOOL CheckArchive(LPCTSTR);
	virtual ARCRESULT TestArchive(LPCTSTR,CString&);	//�A�[�J�C�u�����������ǂ����`�F�b�N����
	virtual bool Compress(LPCTSTR ArcFileName,std::list<CString>&,CConfigManager&,const PARAMETER_TYPE,int Options,LPCTSTR lpszFormat,LPCTSTR lpszMethod,LPCTSTR lpszLevel,CString &strLog)=0;
	virtual bool Extract(LPCTSTR ArcFileName,CConfigManager&,const CConfigExtract&,bool bSafeArchive,LPCTSTR OutputDir,CString &)=0;
	virtual bool ExtractSpecifiedOnly(LPCTSTR ArcFileName,CConfigManager&,LPCTSTR OutputDir,std::list<CString>&,CString &,bool bUsePath=false)=0;	//�w�肵���t�@�C���̂݉�
	virtual bool QueryExtractSpecifiedOnlySupported(LPCTSTR)const{return true;}		//ExtractSpecifiedOnly���T�|�[�g����Ă��邩�ǂ���
	virtual bool GetVersionString(CString&)const;
	virtual LPCTSTR GetName()const{return m_strDllName;}	//DLL����Ԃ�
	virtual int GetFileCount(LPCTSTR);	//�A�[�J�C�u���̃t�@�C������Ԃ�

	//�A�[�J�C�u����w�肵���t�@�C�����폜
	virtual bool DeleteItemFromArchive(LPCTSTR ArcFileName,CConfigManager&,const std::list<CString>&,CString &){return false;}
	virtual bool QueryDeleteItemFromArchiveSupported(LPCTSTR ArcFileName)const{return false;}		//DeleteFile���T�|�[�g����Ă��邩�ǂ���

	//�A�[�J�C�u�Ɏw�肵���t�@�C����ǉ�
	virtual bool AddItemToArchive(LPCTSTR ArcFileName,const std::list<CString>&,CConfigManager&,LPCTSTR lpDestDir,CString&){return false;}
	virtual bool QueryAddItemToArchiveSupported(LPCTSTR ArcFileName)const{return false;}

	virtual bool ExamineArchive(LPCTSTR,CConfigManager&,bool bSkipDir,bool &bInFolder,bool &bSafeArchive,CString&,CString &strErr)=0;
		//�A�[�J�C�u���ꂽ�t�@�C�������Ƀt�H���_���ɓ����Ă��邩�ǂ����A
		//�����ăA�[�J�C�u�����S���ǂ����𒲍�����
		//bSkipDir�͓�d�t�H���_���肪�s�v�ȏꍇ��true�ɂȂ�B���̂Ƃ��A_ExamineArchiveFast�͌Ăт����ȂȂ��čς�

	virtual bool IsOK()const{return NULL!=m_hInstDLL;}		//�A�[�J�C�oDLL�����[�h����Ă��邩

	virtual bool ExtractItems(LPCTSTR lpszArcFile,CConfigManager&,const ARCHIVE_ENTRY_INFO_TREE* lpBase,const std::list<ARCHIVE_ENTRY_INFO_TREE*>&,LPCTSTR lpszOutputBaseDir,bool bCollapseDir,CString &strLog);

	//----------------------
	// ���ɓ������p���\�b�h
	//----------------------
	virtual bool QueryInspectSupported()const{return true;}		//���ɓ��������T�|�[�g����Ă��邩�ǂ���
	virtual bool InspectArchiveBegin(LPCTSTR,CConfigManager&);				//���ɓ������J�n
	virtual bool InspectArchiveEnd();						//���ɓ������I��
	virtual bool InspectArchiveGetFileName(CString&);		//���ɓ��t�@�C�����擾
	virtual bool InspectArchiveNext();						//���ɓ����������̃t�@�C���ɐi�߂�
	virtual int  InspectArchiveGetAttribute();				//���ɓ��t�@�C�������擾
	virtual bool InspectArchiveGetOriginalFileSize(LARGE_INTEGER&);	//���ɓ����k�O�t�@�C���T�C�Y�擾
	virtual bool InspectArchiveGetCompressedFileSize(LARGE_INTEGER&);	//���ɓ����k��t�@�C���T�C�Y�擾
	virtual bool InspectArchiveGetWriteTime(FILETIME&);		//���ɓ��t�@�C���X�V�����擾
	virtual DWORD InspectArchiveGetCRC();					//���ɓ��t�@�C��CRC�擾
	virtual WORD InspectArchiveGetRatio();					//���ɓ��t�@�C�����k���擾
	virtual bool InspectArchiveGetMethodString(CString&);	//���ɓ��t�@�C���i�[���[�h�擾
};


/*
  Compress()���Ăяo����ł́A
1.�J�����g�f�B���N�g���̐ݒ�
2.���X�|���X�t�@�C���ւ̏�������(�o�͐�t�@�C�����̐ݒ�܂�)
3.���X�|���X�t�@�C���̍폜
  �͌Ăяo�����̐ӔC�Ŏ��s����B

  DLL���Ƃ̃X�C�b�`�̐ݒ�̈Ⴂ��Compress()���z������B

*/
