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

class CArchiver7ZIP:public CArchiverDLL{
public:
	CArchiver7ZIP();
	virtual ~CArchiver7ZIP();
	virtual LOAD_RESULT LoadDLL(CConfigManager&,CString &strErr)override;
	virtual void FreeDLL()override;
	virtual bool Compress(LPCTSTR,std::list<CString>&,CConfigManager&,const PARAMETER_TYPE,int,LPCTSTR,LPCTSTR,LPCTSTR,CString &)override;
	virtual bool Extract(LPCTSTR,CConfigManager&,const CConfigExtract&,bool,LPCTSTR,CString &)override;
	virtual bool ExtractSpecifiedOnly(LPCTSTR ArcFileName,CConfigManager&,LPCTSTR OutputDir,std::list<CString>&,CString &,bool bUsePath=false)override;
	virtual bool ExamineArchive(LPCTSTR,CConfigManager&,bool,bool&,bool&,CString&,CString &strErr)override;

	//アーカイブからファイルを削除
	virtual bool DeleteItemFromArchive(LPCTSTR ArcFileName,CConfigManager&,const std::list<CString>&,CString &)override;
	virtual bool QueryDeleteItemFromArchiveSupported(LPCTSTR ArcFileName)const override{return true;}		//DeleteFileがサポートされているかどうか

	//アーカイブに指定したファイルを追加
	virtual bool AddItemToArchive(LPCTSTR ArcFileName,bool bEncrypted,const std::list<CString>&,CConfigManager&,LPCTSTR lpDestDir,CString&)override;
	virtual bool QueryAddItemToArchiveSupported(LPCTSTR ArcFileName)const override{return true;}

	//-------------------------------
	//---UNICODE版をオーバーライド---
	//-------------------------------
	virtual BOOL CheckArchive(LPCTSTR)override;	//TODO:構成を変える
	virtual bool isContentSingleFile(LPCTSTR)override;

	// 書庫内検査用メソッド
	virtual bool InspectArchiveBegin(ARCHIVE_FILE&,LPCTSTR,CConfigManager&)override;				//書庫内調査開始
	virtual bool InspectArchiveGetFileName(ARCHIVE_FILE&,CString&)override;		//書庫内ファイル名取得
	virtual bool InspectArchiveEnd(ARCHIVE_FILE&)override;						//書庫内調査終了
	virtual bool InspectArchiveNext(ARCHIVE_FILE&)override;						//書庫内調査を次のファイルに進める
	virtual bool InspectArchiveGetOriginalFileSize(ARCHIVE_FILE&,LARGE_INTEGER&);	//書庫内圧縮前ファイルサイズ取得
	int InspectArchiveGetAttribute(ARCHIVE_FILE&)override;
	virtual bool InspectArchiveGetWriteTime(ARCHIVE_FILE&,FILETIME&);		//書庫内ファイル更新日時取得
};


enum SEVEN_ZIP_COMPRESS_TYPE{
	SEVEN_ZIP_COMPRESS_LZMA,
	SEVEN_ZIP_COMPRESS_PPMD,
	SEVEN_ZIP_COMPRESS_BZIP2,
	SEVEN_ZIP_COMPRESS_DEFLATE,
	SEVEN_ZIP_COMPRESS_COPY,
	SEVEN_ZIP_COMPRESS_LZMA2,

	ENUM_COUNT_AND_LASTITEM(SEVEN_ZIP_COMPRESS_TYPE),
};
enum SEVEN_ZIP_COMPRESS_LEVEL{
	SEVEN_ZIP_COMPRESS_LEVEL5,
	SEVEN_ZIP_COMPRESS_LEVEL0,
	SEVEN_ZIP_COMPRESS_LEVEL1,
	SEVEN_ZIP_COMPRESS_LEVEL7,
	SEVEN_ZIP_COMPRESS_LEVEL9,

	ENUM_COUNT_AND_LASTITEM(SEVEN_ZIP_COMPRESS_LEVEL),
};
enum SEVEN_ZIP_LZMA_MODE{
	SEVEN_ZIP_LZMA_MODE1,
	SEVEN_ZIP_LZMA_MODE0,

	ENUM_COUNT_AND_LASTITEM(SEVEN_ZIP_LZMA_MODE)
};

#define SEVEN_ZIP_PPMD_MODEL_SIZE_LOWEST	2
#define SEVEN_ZIP_PPMD_MODEL_SIZE_HIGHEST	32

//----------
enum ZIP_COMPRESS_TYPE{
	ZIP_COMPRESS_DEFLATE,
	ZIP_COMPRESS_DEFLATE64,
	ZIP_COMPRESS_BZIP2,
	ZIP_COMPRESS_COPY,
	ZIP_COMPRESS_LZMA,
	ZIP_COMPRESS_PPMD,

	ENUM_COUNT_AND_LASTITEM(ZIP_COMPRESS_TYPE),
};
enum ZIP_COMPRESS_LEVEL{
	ZIP_COMPRESS_LEVEL5,
	ZIP_COMPRESS_LEVEL0,
	ZIP_COMPRESS_LEVEL9,

	ENUM_COUNT_AND_LASTITEM(ZIP_COMPRESS_LEVEL)
};

enum ZIP_CRYPTO_MODE{
	ZIP_CRYPTO_ZIPCRYPTO,
	ZIP_CRYPTO_AES128,
	ZIP_CRYPTO_AES192,
	ZIP_CRYPTO_AES256,

	ENUM_COUNT_AND_LASTITEM(ZIP_CRYPTO_MODE)
};

#define ZIP_DEFLATE_MEMORY_SIZE_LOWEST	3
#define ZIP_DEFLATE_MEMORY_SIZE_HIGHEST	255
#define ZIP_DEFLATE_PASS_NUMBER_LOWEST	1
#define ZIP_DEFLATE_PASS_NUMBER_HIGHEST	4

