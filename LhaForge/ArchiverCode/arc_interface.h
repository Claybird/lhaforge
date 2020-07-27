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
#include "resource.h"
#include "ConfigCode/ConfigManager.h"
#include "Utilities/StringUtil.h"
#include "Utilities/FileOperation.h"

enum class LF_RESULT {
	OK,//successful or archive test passed
	NG,//abnormal end or archive test failed
	CANCELED,//user cancel
	NOTARCHIVE,//not an archive
	NOTIMPL,//not implemented
};

struct ARCLOG{
	virtual ~ARCLOG(){}
	void setArchivePath(const std::wstring& archivePath) {
		_archivePath = archivePath;
		_archiveFilename = std::filesystem::path(archivePath).filename().generic_wstring().c_str();
	}
	std::wstring _archivePath;
	std::wstring _archiveFilename;
	LF_RESULT overallResult;

	struct LOGENTRY {
		std::wstring entryPath;
		std::wstring message;
	};
	std::vector<LOGENTRY> logs;
	void operator()(const std::wstring& entryPath, const std::wstring& message) {
		LOGENTRY e = { entryPath,message };
		logs.push_back(e);
	}
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

struct ARCHIVE_EXCEPTION: public LF_EXCEPTION {
	int _errno;
	ARCHIVE_EXCEPTION(const std::wstring &err) :LF_EXCEPTION(err) {
		_errno = 0;
	}
	ARCHIVE_EXCEPTION(int errno_code) :LF_EXCEPTION(L"") {
		const int max_msg_len = 94;
		wchar_t work[max_msg_len];
		_errno = errno_code;
		_msg = _wcserror_s(work, _errno);
	}
	ARCHIVE_EXCEPTION(archive* arc) :LF_EXCEPTION(L"") {
		_errno = archive_errno(arc);
		auto msg = archive_error_string(arc);
		_msg = UtilUTF8toUNICODE(msg, strlen(msg));
	}
	virtual ~ARCHIVE_EXCEPTION() {}
};


struct _LIBARCHIVE_INTERNAL {
	struct archive *_arc;
	struct archive_entry *_entry;
	_LIBARCHIVE_INTERNAL() {
		_arc = nullptr;
		_entry = archive_entry_new();
	}
	virtual ~_LIBARCHIVE_INTERNAL() {
		if (_entry) {
			archive_entry_free(_entry);
			_entry = nullptr;
		}
	}
	void renew() {
		if (_entry) {
			archive_entry_free(_entry);
			_entry = nullptr;
		}
		_entry = archive_entry_new();
	}

	std::wstring get_pathname() {
		return archive_entry_pathname_w(_entry);
	}
	void set_pathname(const std::wstring& path) {
		archive_entry_copy_pathname_w(_entry, path.c_str());
	}
	void set_stat(const struct __stat64 &st) {
		archive_entry_set_size(_entry, st.st_size);
		archive_entry_set_mtime(_entry, st.st_mtime, 0/* nanosec */);
		archive_entry_set_atime(_entry, st.st_atime, 0/* nanosec */);
		archive_entry_set_ctime(_entry, st.st_ctime, 0/* nanosec */);
		archive_entry_set_mode(_entry, st.st_mode);
	}

	UINT64 get_original_filesize() {
		if (archive_entry_size_is_set(_entry)) {
			return archive_entry_size(_entry);
		} else {
			return -1;
		}
	}
	time_t get_mtime() {
		if (archive_entry_mtime_is_set(_entry)) {
			return archive_entry_mtime(_entry);
		} else {
			return 0;
		}
	};
	time_t get_atime() {
		if (archive_entry_atime_is_set(_entry)) {
			return archive_entry_atime(_entry);
		} else {
			return 0;
		}
	};
	time_t get_ctime() {
		if (archive_entry_ctime_is_set(_entry)) {
			return archive_entry_ctime(_entry);
		} else {
			return 0;
		}
	};
	unsigned short get_file_mode() {
		return archive_entry_filetype(_entry);
	}
	bool is_encrypted() {
		return archive_entry_is_encrypted(_entry);
	}
	const char* get_format_name() {
		return archive_format_name(_arc);	//differ on each entry
	}
	const char* get_mode_name() {
		return archive_entry_strmode(_entry);
	}
};

struct LF_ARCHIVE_ENTRY {
	std::wstring _pathname;
	UINT64 _original_filesize;
	__time64_t _atime, _ctime, _mtime;
	unsigned short _filemode;
	bool _is_encrypted;
	std::string _format_name;
	std::string _mode_name;

	_LIBARCHIVE_INTERNAL  _la_internal;

	LF_ARCHIVE_ENTRY() { renew(); }
	LF_ARCHIVE_ENTRY(LF_ARCHIVE_ENTRY& a) = delete;
	LF_ARCHIVE_ENTRY(archive_entry* entry) = delete;
	const LF_ARCHIVE_ENTRY& operator=(const LF_ARCHIVE_ENTRY& a) = delete;
	virtual ~LF_ARCHIVE_ENTRY() {}
	archive_entry* la_entry() { return _la_internal._entry; }
	void renew() {
		_la_internal.renew();
		_pathname.clear();
		_original_filesize = -1;
		_mtime = 0;
		_atime = 0;
		_ctime = 0;
		_filemode = 0;
		_is_encrypted = false;
		_format_name.clear();
		_mode_name.clear();
	}
	void set_archive(archive* arc) {
		_la_internal._arc = arc;
		renew();
	}

	bool read_next() {
		int r = archive_read_next_header2(_la_internal._arc, _la_internal._entry);
		if (ARCHIVE_OK == r) {
			_pathname = _la_internal.get_pathname();
			_original_filesize = _la_internal.get_original_filesize();
			_mtime = _la_internal.get_mtime();
			_atime = _la_internal.get_atime();
			_ctime = _la_internal.get_ctime();
			_filemode = _la_internal.get_file_mode();
			_is_encrypted = _la_internal.is_encrypted();
			{
				auto p = _la_internal.get_format_name();
				_format_name = p ? p : "---";
			}
			{
				auto p = _la_internal.get_mode_name();
				_mode_name = p ? p : "---";
			}
			return true;
		} else if (ARCHIVE_EOF == r) {
			return false;
		} else {
			throw ARCHIVE_EXCEPTION(_la_internal._arc);
		}
	}

	const wchar_t* get_pathname()const { return _pathname.c_str(); }
	UINT64 get_original_filesize()const { return _original_filesize; }
	__time64_t get_mtime()const { return _mtime; }
	__time64_t get_atime()const { return _atime; }
	__time64_t get_ctime()const { return _ctime; }
	unsigned short get_file_mode()const { return _filemode; }
	bool is_encrypted()const { return _is_encrypted; }
	const char* get_format_name()const { return _format_name.c_str(); }
	const char* get_mode_name()const { return _mode_name.c_str(); }
	bool is_dir()const { return (_filemode & S_IFDIR) != 0; }

	void copy_file_stat(const std::wstring& path, const std::wstring& stored_as) {
		_pathname = stored_as;
		_la_internal.set_pathname(stored_as);
		struct __stat64 st;
		if (0 != _wstat64(path.c_str(), &st)) {
			ARCHIVE_EXCEPTION(L"Failed to stat file");
		}
		_original_filesize = st.st_size;
		_la_internal.set_stat(st);
		_mtime = st.st_mtime;
		_atime = st.st_atime;
		_ctime = st.st_ctime;
		_filemode = st.st_mode;
		_is_encrypted = false;
	}
};


struct LF_BUFFER_INFO {
	//contains buffer and its size in libarchive's internal memory
	size_t size;	//0 if it reaches EOF
	int64_t offset;
	const void* buffer;
	bool is_eof()const { return nullptr == buffer; }
	void make_eof() {
		size = 0;
		offset = 0;
		buffer = nullptr;
	}
};

struct LF_PASSPHRASE {
	std::wstring raw;
	std::string utf8;
};

struct ARCHIVE_FILE_TO_READ
{
	struct archive *_arc;
	LF_ARCHIVE_ENTRY _entry;
	LF_PASSPHRASE _passphrase;
	const ARCHIVE_FILE_TO_READ& operator=(const ARCHIVE_FILE_TO_READ&) = delete;

	ARCHIVE_FILE_TO_READ() {
		_arc = archive_read_new();
		archive_read_support_filter_all(_arc);
		archive_read_support_format_all(_arc);
	}
	ARCHIVE_FILE_TO_READ(const ARCHIVE_FILE_TO_READ&) = delete;
	virtual ~ARCHIVE_FILE_TO_READ() {
		close();
	}
	void read_open(const std::wstring& arcname,
		archive_passphrase_callback passphrase_callback) {
		int r = archive_read_set_passphrase_callback(_arc, &_passphrase, passphrase_callback);
		if (r < ARCHIVE_OK) {
			throw ARCHIVE_EXCEPTION(_arc);
		}
		r = archive_read_open_filename_w(_arc, arcname.c_str(), 10240);
		if (r < ARCHIVE_OK) {
			throw ARCHIVE_EXCEPTION(_arc);
		}
	}
	void close() {
		if (_arc) {
			archive_read_close(_arc);
			archive_read_free(_arc);
		}
		_arc = nullptr;
	}

	LF_ARCHIVE_ENTRY* begin() {
		_entry.set_archive(_arc);
		return next();
	}
	LF_ARCHIVE_ENTRY* next() {
		if (_entry.read_next()) {
			return &_entry;
		} else {
			return nullptr;
		}
	}

	operator struct archive*() { return _arc; }

	LF_BUFFER_INFO read_block() {
		LF_BUFFER_INFO ibi;
		int r = archive_read_data_block(_arc, &ibi.buffer, &ibi.size, &ibi.offset);
		if (ARCHIVE_EOF == r) {
			ibi.make_eof();
		} else if (r < ARCHIVE_OK) {
			throw ARCHIVE_EXCEPTION(_arc);
		}
		return ibi;
	}
};

enum LF_WRITE_OPTIONS {
	LF_WOPT_STANDARD = 0x00,
	LF_WOPT_SFX = 0x01,
	LF_WOPT_DATA_ENCRYPTION = 0x02,
	//LF_WOPT_HEADER_ENCRYPTION = 0x04,	this seems not being supported
};

enum LF_ARCHIVE_FORMAT {
	LF_FMT_INVALID=-1,

	LF_FMT_ZIP,
	LF_FMT_7Z,
	LF_FMT_GZ,
	LF_FMT_BZ2,
	LF_FMT_LZMA,
	LF_FMT_XZ,
	LF_FMT_ZSTD,
	LF_FMT_TAR,
	LF_FMT_TAR_GZ,
	LF_FMT_TAR_BZ2,
	LF_FMT_TAR_LZMA,
	LF_FMT_TAR_XZ,
	LF_FMT_TAR_ZSTD,
	LF_FMT_UUE,

	//---following are decompress only
	LF_FMT_LZH,
	LF_FMT_CAB,
	LF_FMT_RAR,
	LF_FMT_ISO,
	LF_FMT_AR,
	LF_FMT_XAR,
	LF_FMT_WARC,
	LF_FMT_CPIO,
	/*LF_FMT_CPIO_GZ,
	LF_FMT_CPIO_BZ2,
	LF_FMT_CPIO_LZMA,
	LF_FMT_CPIO_XZ,
	LF_FMT_CPIO_Z,*/
	//---following are extracted other than libarchive
	LF_FMT_ACE,
	LF_FMT_JAK,
	LF_FMT_BZA,
	LF_FMT_GZA,
	LF_FMT_ISH,
};

#define NOT_BY_LIBARCHIVE -1

struct LF_ARCHIVE_CAPABILITY {
	LF_ARCHIVE_FORMAT format;
	int mapped_libarchive_format;
	bool need_original_ext;	//TODO: update to include original ext as "{ext}.gz"
	const std::wstring extension;
	bool multi_file_archive;	//true if archive can contain multiple files.
	std::vector<int> allowed_combinations;	//empty if read only
};

WEAK_SYMBOL std::vector<LF_ARCHIVE_CAPABILITY> g_capabilities = {
	{LF_FMT_ZIP, ARCHIVE_FORMAT_ZIP, false, L".zip", true, {
		LF_WOPT_STANDARD,
		LF_WOPT_SFX,
		LF_WOPT_DATA_ENCRYPTION,
		LF_WOPT_SFX | LF_WOPT_DATA_ENCRYPTION}},
	{LF_FMT_7Z, ARCHIVE_FORMAT_7ZIP, false, L".7z", true, {
		//TODO
		LF_WOPT_STANDARD,
		LF_WOPT_SFX,
		//LF_WOPT_DATA_ENCRYPTION,
		//LF_WOPT_HEADER_ENCRYPTION,
		//LF_WOPT_DATA_ENCRYPTION | LF_WOPT_HEADER_ENCRYPTION,
		//LF_WOPT_SFX | LF_WOPT_DATA_ENCRYPTION
		}},
	{LF_FMT_GZ, ARCHIVE_FORMAT_RAW | ARCHIVE_FILTER_GZIP, true, L".gz", false, {LF_WOPT_STANDARD}},	//archive_compressor_gzip_options
	{LF_FMT_BZ2, ARCHIVE_FORMAT_RAW | ARCHIVE_FILTER_BZIP2, true, L".bz2", false, {LF_WOPT_STANDARD}},	//archive_compressor_bzip2_options
	{LF_FMT_LZMA, ARCHIVE_FORMAT_RAW | ARCHIVE_FILTER_LZMA, true, L".lzma", false, {LF_WOPT_STANDARD}},		//archive_compressor_xz_options
	{LF_FMT_XZ, ARCHIVE_FORMAT_RAW | ARCHIVE_FILTER_XZ, true, L".xz", false, {LF_WOPT_STANDARD}},	//archive_compressor_xz_options
	{LF_FMT_ZSTD, ARCHIVE_FORMAT_RAW | ARCHIVE_FILTER_ZSTD, true, L".zst", false, {LF_WOPT_STANDARD}},	//archive_compressor_zstd_options
	{LF_FMT_TAR, ARCHIVE_FORMAT_TAR_GNUTAR, false, L".tar", true, {LF_WOPT_STANDARD}},
	{LF_FMT_TAR_GZ, ARCHIVE_FORMAT_TAR_GNUTAR | ARCHIVE_FILTER_GZIP, false, L".tar.gz", true, {LF_WOPT_STANDARD}},
	{LF_FMT_TAR_BZ2, ARCHIVE_FORMAT_TAR_GNUTAR | ARCHIVE_FILTER_BZIP2, false, L".tar.bz2", true, {LF_WOPT_STANDARD}},
	{LF_FMT_TAR_LZMA, ARCHIVE_FORMAT_TAR_GNUTAR | ARCHIVE_FILTER_LZMA, false, L".tar.lzma", true, {LF_WOPT_STANDARD}},
	{LF_FMT_TAR_XZ, ARCHIVE_FORMAT_TAR_GNUTAR | ARCHIVE_FILTER_XZ, false, L".tar.xz", true, {LF_WOPT_STANDARD}},
	{LF_FMT_TAR_ZSTD, ARCHIVE_FORMAT_TAR_GNUTAR | ARCHIVE_FILTER_ZSTD, false, L".tar.zst", true, {LF_WOPT_STANDARD}},
	/*{LF_FMT_CPIO, ARCHIVE_FORMAT_CPIO, false, L".cpio", true, {LF_WOPT_STANDARD}},
	{LF_FMT_CPIO_GZ, ARCHIVE_FORMAT_CPIO | ARCHIVE_FILTER_GZIP, L".cpio.gz", true, {LF_WOPT_STANDARD}},
	{LF_FMT_CPIO_BZ2, ARCHIVE_FORMAT_CPIO | ARCHIVE_FILTER_BZIP2, L".cpio.bz2", true, {LF_WOPT_STANDARD}},
	{LF_FMT_CPIO_LZMA, ARCHIVE_FORMAT_CPIO | ARCHIVE_FILTER_LZMA, L".cpio.lzma", true, {LF_WOPT_STANDARD}},
	{LF_FMT_CPIO_XZ, ARCHIVE_FORMAT_CPIO | ARCHIVE_FILTER_XZ, L".cpio.xz", true, {LF_WOPT_STANDARD}},
	{LF_FMT_CPIO_Z, ARCHIVE_FORMAT_CPIO | ARCHIVE_FILTER_ZSTD, L".cpio.zstd", true, {LF_WOPT_STANDARD}},*/
	{LF_FMT_UUE, ARCHIVE_FORMAT_RAW | ARCHIVE_FILTER_UU, false, L".uue", false, {LF_WOPT_STANDARD}},

	//---following are decompress only
	{LF_FMT_LZH, ARCHIVE_FORMAT_LHA, false, L".lzh", true, {}},
	{LF_FMT_CAB, ARCHIVE_FORMAT_CAB, false, L".cab", true, {}},
	{LF_FMT_RAR, ARCHIVE_FORMAT_RAR, false, L".rar", true, {}},
	{LF_FMT_ISO, ARCHIVE_FORMAT_ISO9660, false, L".iso", true, {}},	//create is supported, but disabled intentionally; lhaforge is not designed as a iso file writer
	//{ARCHIVE_FORMAT_SHAR},	disabled because it is irrelevant to windows users
	{LF_FMT_AR, ARCHIVE_FORMAT_AR, false, L".ar", true, {}},
	//{ARCHIVE_FORMAT_MTREE},	disabled because it contains only metadata, i.e., no actual data
	{LF_FMT_XAR, ARCHIVE_FORMAT_XAR, false, L".xar", true, {}},	// create is supported, but disabled intentionally; xar is not popular on windows
	{LF_FMT_WARC, ARCHIVE_FORMAT_WARC, false, L".warc", true, {}},	//create is supported, but disabled intentionally; lhaforge is not designed as a web archive writer
	{LF_FMT_CPIO, ARCHIVE_FORMAT_CPIO, false, L".cpio", true, {}},
	//---following are extracted other than libarchive
	{LF_FMT_ACE, NOT_BY_LIBARCHIVE, false, L".ace", true, {}},
	{LF_FMT_JAK, NOT_BY_LIBARCHIVE, false, L".jak", true, {}},
	{LF_FMT_BZA, NOT_BY_LIBARCHIVE, false, L".bza", true, {}},
	{LF_FMT_GZA, NOT_BY_LIBARCHIVE, false, L".gza", true, {}},
	{LF_FMT_ISH, NOT_BY_LIBARCHIVE, false, L".ish", false, {}},
};


inline const LF_ARCHIVE_CAPABILITY& get_archive_capability(LF_ARCHIVE_FORMAT fmt)
{
	for (const auto &cap : g_capabilities) {
		if (fmt == cap.format) {
			return cap;
		}
	}
	throw ARCHIVE_EXCEPTION(EINVAL);
}

struct ARCHIVE_FILE_TO_WRITE
{
	struct archive *_arc;
	LF_ARCHIVE_ENTRY _entry;
	LF_PASSPHRASE _passphrase;
	const ARCHIVE_FILE_TO_WRITE& operator=(const ARCHIVE_FILE_TO_WRITE&) = delete;

	ARCHIVE_FILE_TO_WRITE() :_arc(nullptr) {}
	ARCHIVE_FILE_TO_WRITE(const ARCHIVE_FILE_TO_WRITE&) = delete;
	virtual ~ARCHIVE_FILE_TO_WRITE() {
		close();
	}

	void write_open(const std::wstring& arcname,
		LF_ARCHIVE_FORMAT fmt,
		const std::map<std::string, std::string> &archive_options,
		archive_passphrase_callback passphrase_callback) {
		close();
		_arc = archive_write_new();
		const auto& cap = get_archive_capability(fmt);

		int la_filter = cap.mapped_libarchive_format & ~ARCHIVE_FORMAT_BASE_MASK;
		if (la_filter != ARCHIVE_FILTER_NONE) {
			int r = archive_write_add_filter(_arc, la_filter);
			if (r < ARCHIVE_OK) {
				throw ARCHIVE_EXCEPTION(_arc);
			}
		}

		int la_fmt = cap.mapped_libarchive_format & ARCHIVE_FORMAT_BASE_MASK;
		int r = archive_write_set_format(_arc, la_fmt);
		if (r < ARCHIVE_OK) {
			throw ARCHIVE_EXCEPTION(_arc);
		}

		r = archive_write_set_passphrase_callback(_arc, &_passphrase, passphrase_callback);
		if (r < ARCHIVE_OK) {
			throw ARCHIVE_EXCEPTION(_arc);
		}
		for(auto &ite: archive_options){
			int r = archive_write_set_option(_arc, nullptr, ite.first.c_str(), ite.second.c_str());
			if (r < ARCHIVE_OK) {
				throw ARCHIVE_EXCEPTION(_arc);
			}
		}
		r = archive_write_open_filename_w(_arc, arcname.c_str());
		if (r < ARCHIVE_OK) {
			throw ARCHIVE_EXCEPTION(_arc);
		}
	}
	void close() {
		if (_arc) {
			archive_write_close(_arc);
			archive_write_free(_arc);
		}
		_arc = nullptr;
	}

	template<typename T>
	void add_entry(LF_ARCHIVE_ENTRY& entry, T& dataProvider) {
		int r = archive_write_header(_arc, entry.la_entry());
		if (r < ARCHIVE_OK) {
			throw ARCHIVE_EXCEPTION(_arc);
		}

		while (true) {
			LF_BUFFER_INFO ibi = dataProvider();
			if (ibi.is_eof()) {
				break;
			} else {
				archive_write_data(_arc, ibi.buffer, ibi.size);
			}
		}
		//archive_write_finish_entry(_arc);	Ordinarily, clients never need to call this
		//, as it is called automatically by archive_write_next_header() and archive_write_close() as needed. 
	}
	void add_directory(LF_ARCHIVE_ENTRY& entry) {
		int r = archive_write_header(_arc, entry.la_entry());
		if (r < ARCHIVE_OK) {
			throw ARCHIVE_EXCEPTION(_arc);
		}
		//archive_write_finish_entry(_arc);	Ordinarily, clients never need to call this
		//, as it is called automatically by archive_write_next_header() and archive_write_close() as needed. 
	}
};
