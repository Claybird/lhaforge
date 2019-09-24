/*
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
#include "../resource.h"
#include "ArcEntryInfo.h"
#include "../ConfigCode/ConfigManager.h"

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
	PARAMETER_HKI,
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

#define LIBARCHIVE_STATIC
#include <libarchive/archive.h>
#include <libarchive/archive_entry.h>

struct ARCHIVE_FILE {
	struct archive *_arc;
	struct archive_entry *_entry;
	ARCHIVE_FILE():_entry(NULL) {
		_arc = archive_read_new();
		archive_read_support_filter_all(_arc);
		archive_read_support_format_all(_arc);
	}
	virtual ~ARCHIVE_FILE() {
		close();
	}
	void close() {
		if (_arc) {
			archive_read_close(_arc);
			archive_read_free(_arc);
			_arc = NULL;
			_entry = NULL;
		}
	}
	operator struct archive*() { return _arc; }
	operator struct archive_entry*() { return _entry; }
	const ARCHIVE_FILE& operator=(const ARCHIVE_FILE&) = delete;
};

struct CConfigExtract;
//統合アーカイバDLLラップ用クラスのベース
class CArchiverDLL{
protected:
	//DLL固有のデータ
	//---------
	//書庫内検査の状態を記録するための変数
	bool m_bInspectFirstTime;

public:
	CArchiverDLL();
	virtual ~CArchiverDLL(){};
	virtual LOAD_RESULT LoadDLL(CConfigManager&,CString &strErr);
	virtual void FreeDLL();
	virtual WORD GetVersion()const;
	virtual WORD GetSubVersion()const;
	virtual bool IsUnicodeCapable()const{return true;}	//UNICODE対応DLLならtrueを返す
	virtual bool IsWeakCheckArchive()const{return false;}	//CheckArchiveの機能が貧弱(UNBEL/AISHのように)ならtrue
	virtual bool IsWeakErrorCheck()const{return false;}	//%Prefix%()のエラーチェックが甘い(XacRettのように)ならtrue;解凍後に削除するかの判断に使用
	virtual BOOL CheckArchive(LPCTSTR);
	virtual ARCRESULT TestArchive(LPCTSTR,CString&);	//アーカイブが正しいかどうかチェックする
	virtual bool Compress(LPCTSTR ArcFileName,std::list<CString>&,CConfigManager&,const PARAMETER_TYPE,int Options,LPCTSTR lpszFormat,LPCTSTR lpszMethod,LPCTSTR lpszLevel,CString &strLog)=0;
	virtual bool Extract(LPCTSTR ArcFileName,CConfigManager&,const CConfigExtract&,bool bSafeArchive,LPCTSTR OutputDir,CString &)=0;
	virtual bool ExtractSpecifiedOnly(LPCTSTR ArcFileName,CConfigManager&,LPCTSTR OutputDir,std::list<CString>&,CString &,bool bUsePath=false)=0;	//指定したファイルのみ解凍
	virtual bool QueryExtractSpecifiedOnlySupported(LPCTSTR)const{return true;}		//ExtractSpecifiedOnlyがサポートされているかどうか
	virtual bool GetVersionString(CString&)const;
	virtual LPCTSTR GetName()const{return L"DUMMY";}	//DLL名を返す//TODO:remove
	virtual bool isContentSingleFile(LPCTSTR)=0;	//アーカイブ中に複数ファイルが含まれていればfalse

	//アーカイブから指定したファイルを削除
	virtual bool DeleteItemFromArchive(LPCTSTR ArcFileName,CConfigManager&,const std::list<CString>&,CString &){return false;}
	virtual bool QueryDeleteItemFromArchiveSupported(LPCTSTR ArcFileName)const{return false;}		//DeleteFileがサポートされているかどうか

	//アーカイブに指定したファイルを追加
	virtual bool AddItemToArchive(LPCTSTR ArcFileName,bool bEncrypted,const std::list<CString>&,CConfigManager&,LPCTSTR lpDestDir,CString&){return false;}
	virtual bool QueryAddItemToArchiveSupported(LPCTSTR ArcFileName)const{return false;}

	virtual bool ExamineArchive(LPCTSTR,CConfigManager&,bool bSkipDir,bool &bInFolder,bool &bSafeArchive,CString&,CString &strErr)=0;
		//アーカイブされたファイルが既にフォルダ内に入っているかどうか、
		//そしてアーカイブが安全かどうかを調査する
		//bSkipDirは二重フォルダ判定が不要な場合にtrueになる。このとき、_ExamineArchiveFastは呼びださななくて済む

	virtual bool IsOK()const{return true;}	//TODO:remove		//アーカイバDLLがロードされているか

	virtual bool ExtractItems(LPCTSTR lpszArcFile,CConfigManager&,const ARCHIVE_ENTRY_INFO_TREE* lpBase,const std::list<ARCHIVE_ENTRY_INFO_TREE*>&,LPCTSTR lpszOutputBaseDir,bool bCollapseDir,CString &strLog);

	//----------------------
	// 書庫内検査用メソッド
	//----------------------
	virtual bool QueryInspectSupported()const{return true;}		//書庫内調査がサポートされているかどうか
	virtual bool InspectArchiveBegin(ARCHIVE_FILE&,LPCTSTR,CConfigManager&)=0;				//書庫内調査開始
	virtual bool InspectArchiveEnd(ARCHIVE_FILE&) = 0;						//書庫内調査終了
	virtual bool InspectArchiveGetFileName(ARCHIVE_FILE&,CString&) = 0;		//書庫内ファイル名取得
	virtual bool InspectArchiveNext(ARCHIVE_FILE&) = 0;						//書庫内調査を次のファイルに進める
	virtual int  InspectArchiveGetAttribute(ARCHIVE_FILE&) = 0;				//書庫内ファイル属性取得
	virtual bool InspectArchiveGetOriginalFileSize(ARCHIVE_FILE&,LARGE_INTEGER&) = 0;	//書庫内圧縮前ファイルサイズ取得
	virtual bool InspectArchiveGetWriteTime(ARCHIVE_FILE&,FILETIME&) = 0;		//書庫内ファイル更新日時取得
};


/*
  Compress()を呼び出す上では、
1.カレントディレクトリの設定
2.レスポンスファイルへの書き込み(出力先ファイル名の設定含む)
3.レスポンスファイルの削除
  は呼び出し側の責任で実行する。

  DLLごとのスイッチの設定の違いをCompress()が吸収する。

*/
