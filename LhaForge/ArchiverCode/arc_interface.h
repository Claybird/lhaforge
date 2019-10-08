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
#include "Utilities/StringUtil.h"
#include "Utilities/FileOperation.h"

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
#include <errno.h>

struct ARCHIVE_EXCEPTION {
	std::wstring _msg;
	int _errno;
	ARCHIVE_EXCEPTION(const std::wstring &err) {
		_msg = err;
		_errno = 0;
	}
	ARCHIVE_EXCEPTION(archive* arc) {
		_errno = archive_errno(arc);
		_msg = CUTF8String(archive_error_string(arc)).toWstring();
	}
	const wchar_t* what()const { return _msg.c_str(); }
};

template<bool _readMode>
struct ARCHIVE_FILE_BASE {
	struct archive *_arc;
	ARCHIVE_FILE_BASE() {
		_arc = archive_read_new();
		archive_read_support_filter_all(_arc);
		archive_read_support_format_all(_arc);
	}
	virtual ~ARCHIVE_FILE_BASE() {
		close();
	}
	void close() {
		if (_arc) {
			if (_readMode) {
				archive_read_close(_arc);
				archive_read_free(_arc);
			} else {
				archive_write_close(_arc);
				archive_write_free(_arc);
			}
			_arc = NULL;
		}
	}
	operator struct archive*() { return _arc; }
	const ARCHIVE_FILE_BASE& operator=(const ARCHIVE_FILE_BASE&) = delete;
};

struct LF_ARCHIVE_ENTRY{
	struct archive *_arc;
	struct archive_entry *_entry;

	LF_ARCHIVE_ENTRY(){
		_entry = archive_entry_new();
	}
	LF_ARCHIVE_ENTRY(LF_ARCHIVE_ENTRY& a) = delete;
	LF_ARCHIVE_ENTRY(archive_entry* entry) = delete;
	const LF_ARCHIVE_ENTRY& operator=(const LF_ARCHIVE_ENTRY& a) = delete;
	const wchar_t* get_pathname(){
		return archive_entry_pathname_w(_entry);
	}
	LARGE_INTEGER get_original_filesize() {
		LARGE_INTEGER size;
		if (archive_entry_size_is_set(_entry)) {
			size.QuadPart = archive_entry_size(_entry);
		} else {
			size.QuadPart = -1;
		}
		return size;
	}
	FILETIME get_write_time() {
		FILETIME ft;
		if (archive_entry_mtime_is_set(_entry)) {
			const auto mtime = archive_entry_mtime(_entry);
			//archive_entry_mtime_nsec is not used, because it returns 32bit value on Windows
			UtilUnixTimeToFileTime(mtime, &ft);
		} else {
			UtilUnixTimeToFileTime(0, &ft);
		}
		return ft;
	};
	unsigned short get_file_mode() {
		const auto *stat = archive_entry_stat(_entry);
		return stat->st_mode;
	}
	bool is_encrypted() {
		return archive_entry_is_encrypted(_entry);
	}
};

struct ARCHIVE_FILE_TO_READ:public ARCHIVE_FILE_BASE<true>
{
	LF_ARCHIVE_ENTRY _entry;

	LF_ARCHIVE_ENTRY* begin() { return next(); }
	LF_ARCHIVE_ENTRY* next() {
		LF_ARCHIVE_ENTRY entry;
		if (ARCHIVE_OK == archive_read_next_header2(_arc, _entry._entry)) {
			return &_entry;
		} else {
			return NULL;
		}
	}

	void read_open(const wchar_t* arcname) {
		int r = archive_read_open_filename_w(_arc, arcname, 10240);
		if (ARCHIVE_OK != r) {
			throw ARCHIVE_EXCEPTION(_arc);
		}
	}

	struct INTERNAL_BUFFER_INFO {
		//contains buffer and its size in libarchive's internal memory
		size_t size;	//0 if it reaches EOF
		size_t offset;
		const void* buffer;
	};

	INTERNAL_BUFFER_INFO read_block() {
		INTERNAL_BUFFER_INFO info;
		int r = archive_read_data_block(_arc, &info.buffer, &info.size, &info.offset);
		if (ARCHIVE_EOF == r) {
			info.size = 0;
			info.offset = 0;
			info.buffer = NULL;
		} else if (ARCHIVE_OK != r) {
			throw ARCHIVE_EXCEPTION(_arc);
		}
		return info;
	}
};
