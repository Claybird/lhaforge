﻿/*
* MIT License

* Copyright (c) 2005- Claybird

* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:

* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.

* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/

#pragma once
#include "arc_interface.h"

//---UNICODE用のArchiverDLL定義
typedef struct {
	DWORD 			dwOriginalSize;		/* ファイルのサイズ */
 	DWORD 			dwCompressedSize;	/* 圧縮後のサイズ */
	DWORD			dwCRC;				/* 格納ファイルのチェックサム */
	UINT			uFlag;				/* 処理結果 */
										/* Status flag */
	UINT			uOSType;			/* 書庫作成に使われた OS */
	WORD			wRatio;				/* 圧縮率 */
	WORD			wDate;				/* 格納ファイルの日付(DOS 形式) */
	WORD 			wTime;				/* 格納ファイルの時刻(〃) */
	WCHAR			szFileName[FNAME_MAX32 + 1];	/* 格納ファイル名 */
	WCHAR			dummy1[3];
	WCHAR			szAttribute[8];		/* 格納ファイルの属性(書庫固有) */
	WCHAR			szMode[8];			/* 格納ファイルの格納モード(〃) */
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

	//アーカイブからファイルを削除
	virtual bool DeleteItemFromArchive(LPCTSTR ArcFileName,CConfigManager&,const std::list<CString>&,CString &strLog)override;
	virtual bool QueryDeleteItemFromArchiveSupported(LPCTSTR ArcFileName)const override{return true;}		//DeleteFileがサポートされているかどうか

	virtual ARCRESULT TestArchive(LPCTSTR,CString &)override;	//アーカイブが正しいかどうかチェックする

	//アーカイブに指定したファイルを追加
	virtual bool AddItemToArchive(LPCTSTR ArcFileName,bool bEncrypted,const std::list<CString>&,CConfigManager&,LPCTSTR lpDestDir,CString&)override;
	virtual bool QueryAddItemToArchiveSupported(LPCTSTR ArcFileName)const override{return true;}


	//---UNICODE
	virtual LOAD_RESULT LoadDLL(CConfigManager&,CString &strErr)override;
	virtual void FreeDLL()override;
	virtual bool IsUnicodeCapable()const override{return true;}	//UNICODE対応DLLならtrueを返す
	virtual BOOL CheckArchive(LPCTSTR)override;
	virtual int GetFileCount(LPCTSTR)override;	//アーカイブ中のファイル数を返す

	virtual bool InspectArchiveBegin(LPCTSTR,CConfigManager&)override;				//書庫内調査開始
	//virtual bool InspectArchiveEnd();						//書庫内調査終了
	virtual bool InspectArchiveGetFileName(CString&)override;		//書庫内ファイル名取得
	virtual bool InspectArchiveNext()override;						//書庫内調査を次のファイルに進める
	//オーバーライドの必要なし;UNLHA32.dllではINDIVIDUALINFOを参照しないので正しく動作する
	//bool InspectArchiveGetOriginalFileSize(LARGE_INTEGER &FileSize);
	//bool InspectArchiveGetCompressedFileSize(LARGE_INTEGER &FileSize);
	//bool InspectArchiveGetWriteTime(FILETIME &FileTime);
	virtual DWORD InspectArchiveGetCRC()override;					//書庫内ファイルCRC取得
	virtual WORD InspectArchiveGetRatio()override;					//書庫内ファイル圧縮率取得
	virtual bool InspectArchiveGetMethodString(CString&)override;			//書庫内ファイル格納モード取得
};
