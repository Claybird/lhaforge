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
	DWORD 			dwOriginalSize;		/* ファイルのサイズ */
 	DWORD 			dwCompressedSize;	/* 圧縮後のサイズ */
	DWORD			dwCRC;				/* 格納ファイルのチェックサム */
	UINT			uFlag;				/* 処理結果 */
	UINT			uOSType;			/* 書庫作成に使われたＯＳ */
	WORD			wRatio;				/* 圧縮率 */
	WORD			wDate;				/* 格納ファイルの日付(DOS 形式) */
	WORD 			wTime;				/* 格納ファイルの時刻(〃) */
	char			szFileName[FNAME_MAX32 + 1];	/* 書庫名 */
	char			dummy1[3];
	char			szAttribute[8];		/* 格納ファイルの属性(書庫固有) */
	char			szMode[8];			/* 格納ファイルの格納モード(〃) */
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

#define CHECKARCHIVE_RECOVERY	4   /* 破損ヘッダを読み飛ばして処理 */
#define CHECKARCHIVE_SFX		8	/* SFX かどうかを返す */
#define CHECKARCHIVE_ALL		16	/* ファイルの最後まで検索する */
#define CHECKARCHIVE_ENDDATA	32	/* 書庫より後ろの余剰データを許可 */

#define	CHECKARCHIVE_NOT_ASK_PASSWORD	64

#define ERROR_PASSWORD_FILE		0x800A


enum PARAMETER_TYPE;

enum LOAD_DLL_LEVEL{
	LOAD_DLL_STANDARD	=0x00000001L,					//通常
	LOAD_DLL_MINIMUM	=0x00000002L,					//OpenArchive*が使えないDLL用
	LOAD_DLL_SIMPLE_INSPECTION	=0x00000004L,				//%Prefix%(),OpenArchive(),FindFile*()以外が使えないDLL(BGA32.DLL)用
//	LOAD_DLL_STANDARD_WITHOUT_GETATTRIBUTE=0x00000004L,	//GetAttributeが使えないDLL用の通常
};

enum LOAD_RESULT{
	LOAD_RESULT_OK,			//DLLは正常にロードされた
	LOAD_RESULT_NOT_FOUND,	//DLLが見つからない
	LOAD_RESULT_INVALID,	//不正なDLL
	LOAD_RESULT_TOO_OLD		//DLLはサポートされているバージョンより古い
};

const int LOG_BUFFER_SIZE=512*1024;	//512KB

enum COMPRESS_MODE{
	COMPRESS_SFX				=	0x00000001L,
	COMPRESS_PASSWORD			=	0x00000002L,
	COMPRESS_PUBLIC_PASSWORD	=	0x00000004L,
	COMPRESS_SPLIT				=	0x00000008L,
};

//圧縮形式パラメータ
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

	PARAMETER_B2E,	//B2Eは特別扱い

	ENUM_COUNT_AND_LASTITEM(PARAMETER),
};

//Compress/Extract/TestArchiveの戻り値
enum ARCRESULT{
	//---解凍系
	EXTRACT_OK,//正常終了
	EXTRACT_NG,//異常終了
	EXTRACT_CANCELED,//キャンセル
	EXTRACT_NOTARCHIVE,//圧縮ファイルではない
	EXTRACT_INFECTED,//ウィルスの可能性あり

	//---検査系
	TEST_OK,	//ファイルは正常
	TEST_NG,	//ファイルに異常あり
	TEST_NOTIMPL,//検査は実装されていない
	TEST_NOTARCHIVE,//圧縮ファイルではない
	TEST_INFECTED,//ウィルスの可能性あり
	TEST_ERROR,	//内部エラー(DLLがロードされていないのに呼び出された、等)
};

struct ARCLOG{	//アーカイブ操作の結果を格納する
	virtual ~ARCLOG(){}
	CString strFile;	//アーカイブのフルパス
	CString strMsg;		//ログ
	ARCRESULT Result;	//結果
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
//統合アーカイバDLLラップ用クラスのベース
class CArchiverDLL{
protected:
	//DLL固有のデータ
	DWORD			m_dwInspectMode;		//OpenArchiveのモード
	CString			m_strDllName;			//DLL名
	CStringA		m_AstrPrefix;				//関数プリフィックス
	CStringA		m_AstrFindParam;			//InspectArchiveFileNext()の引数
	LOAD_DLL_LEVEL	m_LoadLevel;			//LoadDLLで要求する関数のレベル
	WORD			m_nRequiredVersion;	//LhaForgeがサポートするDLLの最低バージョン
	WORD			m_nRequiredSubVersion;	//LhaForgeがサポートするDLLの最低サブバージョン

	//---------
	//書庫内検査の状態を記録するための変数
	HARC m_hInspectArchive;
	bool m_bInspectFirstTime;
	INDIVIDUALINFO m_IndividualInfo;

	//---------
	HINSTANCE	m_hInstDLL;			//DLLインスタンス
	COMMON_ARCHIVER_HANDLER			ArchiveHandler;	//Un???関数
	COMMON_ARCHIVER_GETVERSION		ArchiverGetVersion;
	COMMON_ARCHIVER_GETVERSION		ArchiverGetSubVersion;
	COMMON_ARCHIVER_CHECKARCHIVE	ArchiverCheckArchive;
	COMMON_ARCHIVER_GETFILECOUNT	ArchiverGetFileCount;
	//以下の関数は二重フォルダ判定および危険アーカイブ判定などアーカイブ内調査に使う
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

	//レスポンスファイルにデータを書き込む:MBCS,エスケープを必要としないもの限定
	virtual void WriteResponceFile(HANDLE,LPCTSTR,bool bQuoteSpaces=true);
public:
	CArchiverDLL();
	virtual ~CArchiverDLL(){};
	virtual LOAD_RESULT LoadDLL(CConfigManager&,CString &strErr);
	virtual void FreeDLL();
	virtual WORD GetVersion()const;
	virtual WORD GetSubVersion()const;
	virtual bool IsUnicodeCapable()const{return false;}	//UNICODE対応DLLならtrueを返す
	virtual bool IsWeakCheckArchive()const{return false;}	//CheckArchiveの機能が貧弱(UNBEL/AISHのように)ならtrue
	virtual bool IsWeakErrorCheck()const{return false;}	//%Prefix%()のエラーチェックが甘い(XacRettのように)ならtrue;解凍後に削除するかの判断に使用
	virtual BOOL CheckArchive(LPCTSTR);
	virtual ARCRESULT TestArchive(LPCTSTR,CString&);	//アーカイブが正しいかどうかチェックする
	virtual bool Compress(LPCTSTR ArcFileName,std::list<CString>&,CConfigManager&,const PARAMETER_TYPE,int Options,LPCTSTR lpszFormat,LPCTSTR lpszMethod,LPCTSTR lpszLevel,CString &strLog)=0;
	virtual bool Extract(LPCTSTR ArcFileName,CConfigManager&,const CConfigExtract&,bool bSafeArchive,LPCTSTR OutputDir,CString &)=0;
	virtual bool ExtractSpecifiedOnly(LPCTSTR ArcFileName,CConfigManager&,LPCTSTR OutputDir,std::list<CString>&,CString &,bool bUsePath=false)=0;	//指定したファイルのみ解凍
	virtual bool QueryExtractSpecifiedOnlySupported(LPCTSTR)const{return true;}		//ExtractSpecifiedOnlyがサポートされているかどうか
	virtual bool GetVersionString(CString&)const;
	virtual LPCTSTR GetName()const{return m_strDllName;}	//DLL名を返す
	virtual int GetFileCount(LPCTSTR);	//アーカイブ中のファイル数を返す

	//アーカイブから指定したファイルを削除
	virtual bool DeleteItemFromArchive(LPCTSTR ArcFileName,CConfigManager&,const std::list<CString>&,CString &){return false;}
	virtual bool QueryDeleteItemFromArchiveSupported(LPCTSTR ArcFileName)const{return false;}		//DeleteFileがサポートされているかどうか

	//アーカイブに指定したファイルを追加
	virtual bool AddItemToArchive(LPCTSTR ArcFileName,const std::list<CString>&,CConfigManager&,LPCTSTR lpDestDir,CString&){return false;}
	virtual bool QueryAddItemToArchiveSupported(LPCTSTR ArcFileName)const{return false;}

	virtual bool ExamineArchive(LPCTSTR,CConfigManager&,bool bSkipDir,bool &bInFolder,bool &bSafeArchive,CString&,CString &strErr)=0;
		//アーカイブされたファイルが既にフォルダ内に入っているかどうか、
		//そしてアーカイブが安全かどうかを調査する
		//bSkipDirは二重フォルダ判定が不要な場合にtrueになる。このとき、_ExamineArchiveFastは呼びださななくて済む

	virtual bool IsOK()const{return NULL!=m_hInstDLL;}		//アーカイバDLLがロードされているか

	virtual bool ExtractItems(LPCTSTR lpszArcFile,CConfigManager&,const ARCHIVE_ENTRY_INFO_TREE* lpBase,const std::list<ARCHIVE_ENTRY_INFO_TREE*>&,LPCTSTR lpszOutputBaseDir,bool bCollapseDir,CString &strLog);

	//----------------------
	// 書庫内検査用メソッド
	//----------------------
	virtual bool QueryInspectSupported()const{return true;}		//書庫内調査がサポートされているかどうか
	virtual bool InspectArchiveBegin(LPCTSTR,CConfigManager&);				//書庫内調査開始
	virtual bool InspectArchiveEnd();						//書庫内調査終了
	virtual bool InspectArchiveGetFileName(CString&);		//書庫内ファイル名取得
	virtual bool InspectArchiveNext();						//書庫内調査を次のファイルに進める
	virtual int  InspectArchiveGetAttribute();				//書庫内ファイル属性取得
	virtual bool InspectArchiveGetOriginalFileSize(LARGE_INTEGER&);	//書庫内圧縮前ファイルサイズ取得
	virtual bool InspectArchiveGetCompressedFileSize(LARGE_INTEGER&);	//書庫内圧縮後ファイルサイズ取得
	virtual bool InspectArchiveGetWriteTime(FILETIME&);		//書庫内ファイル更新日時取得
	virtual DWORD InspectArchiveGetCRC();					//書庫内ファイルCRC取得
	virtual WORD InspectArchiveGetRatio();					//書庫内ファイル圧縮率取得
	virtual bool InspectArchiveGetMethodString(CString&);	//書庫内ファイル格納モード取得
};


/*
  Compress()を呼び出す上では、
1.カレントディレクトリの設定
2.レスポンスファイルへの書き込み(出力先ファイル名の設定含む)
3.レスポンスファイルの削除
  は呼び出し側の責任で実行する。

  DLLごとのスイッチの設定の違いをCompress()が吸収する。

*/
