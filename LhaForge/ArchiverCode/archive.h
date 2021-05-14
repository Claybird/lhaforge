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

//include this file to use all archive handlers

#pragma once

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
	OUTPUT_TO_DEFAULT=-1,
	OUTPUT_TO_DESKTOP,
	OUTPUT_TO_SAME_DIR,
	OUTPUT_TO_SPECIFIC_DIR,
	OUTPUT_TO_ALWAYS_ASK_WHERE,

	ENUM_COUNT_AND_LASTITEM(OUTPUT_TO),
};
enum CREATE_OUTPUT_DIR{
	CREATE_OUTPUT_DIR_DEFAULT=-1,
	CREATE_OUTPUT_DIR_ALWAYS,
	CREATE_OUTPUT_DIR_SINGLE,
	CREATE_OUTPUT_DIR_NEVER,

	ENUM_COUNT_AND_LASTITEM(CREATE_OUTPUT_DIR)
};

enum class IGNORE_TOP_DIR :int {
	None,
	FirstTop,
	Recursive,
	ENUM_COUNT_AND_LASTITEM(IGNORE_TOP_DIR),
};

/*
//TODO
//---following are extracted other than libarchive
{LF_FMT_ACE, NOT_BY_LIBARCHIVE, false, L".ace", true, {}},
{ LF_FMT_JAK, NOT_BY_LIBARCHIVE, false, L".jak", true, {} },
{ LF_FMT_BZA, NOT_BY_LIBARCHIVE, false, L".bza", true, {} },
{ LF_FMT_GZA, NOT_BY_LIBARCHIVE, false, L".gza", true, {} },
{ LF_FMT_ISH, NOT_BY_LIBARCHIVE, false, L".ish", false, {} },
*/


enum class LF_RESULT {
	OK,//successful or archive test passed
	NG,//abnormal end or archive test failed
	CANCELED,//user cancel
	NOTARCHIVE,//not an archive
	NOTIMPL,//not implemented
};

struct ARCHIVE_EXCEPTION : public LF_EXCEPTION {
	int _errno;
	ARCHIVE_EXCEPTION(const std::wstring &err) :LF_EXCEPTION(err) {
		_errno = 0;
	}
	ARCHIVE_EXCEPTION(int errno_code) :LF_EXCEPTION(L"") {
		const int max_msg_len = 94;
		wchar_t work[max_msg_len];
		_errno = errno_code;
		_wcserror_s(work, _errno);
		_msg = work;
	}
	virtual ~ARCHIVE_EXCEPTION() {}
};

enum LF_ARCHIVE_FORMAT {
	LF_FMT_INVALID = -1,

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

	ENUM_COUNT_AND_LASTITEM(LF_ARCHIVE_FORMAT)
};

enum LF_WRITE_OPTIONS {
	LF_WOPT_STANDARD = 0x00,
	//LF_WOPT_SFX = 0x01,	users do not need this, according to poll
	LF_WOPT_DATA_ENCRYPTION = 0x02,
	//LF_WOPT_HEADER_ENCRYPTION = 0x04,	this seems not being supported
};

struct LF_COMPRESS_CAPABILITY {
	LF_ARCHIVE_FORMAT format;
	std::wstring extension;	//such as "{ext}.gz"; "{ext}" should be replaced with original extension
	bool contains_multiple_files;	//true if archive can contain multiple files.
	std::vector<int> allowed_combinations;	//combination of LF_WRITE_OPTIONS

	//returns extension with first "."
	std::wstring formatExt(const std::filesystem::path& input_filename, int option)const {
		auto org_ext = input_filename.extension();
		/*if (option & LF_WOPT_SFX) {
			if (extension.find(L"{ext}") == std::string::npos) {
				return L".exe";
			} else {
				return org_ext.wstring() + L".exe";
			}
		} else {*/
			return replace(extension, L"{ext}", org_ext);
		//}
	}
};

struct LF_BUFFER_INFO {
	//contains buffer and its size
	int64_t size;	//0 if it reaches EOF
	int64_t offset;
	const void* buffer;
	bool is_eof()const { return nullptr == buffer; }
	void make_eof() {
		size = 0;
		offset = 0;
		buffer = nullptr;
	}
};

struct ILFPassphrase {
	virtual ~ILFPassphrase() {}
	void set_passphrase(const std::wstring& p) {
		raw = p;
		utf8 = UtilToUTF8(raw);
	}

	std::wstring raw;
	std::string utf8;
	virtual const char* operator()() = 0;
};

struct ILFProgressHandler {
	int64_t numEntries;
	std::filesystem::path archivePath;

	ILFProgressHandler() { reset(); }
	virtual ~ILFProgressHandler() {}
	virtual void reset() { numEntries = 0; archivePath.clear(); }
	virtual void end() = 0;
	virtual void setArchive(const std::filesystem::path& path) { archivePath = path; }
	virtual void setNumEntries(int64_t num) { numEntries = num; }
	virtual void onNextEntry(const std::filesystem::path& entry_path, int64_t entry_size) = 0;
	virtual void onEntryIO(int64_t current_size) = 0;
};

struct ILFScanProgressHandler {
	ILFScanProgressHandler() {}
	virtual ~ILFScanProgressHandler() {}
	virtual void end() = 0;
	virtual void setArchive(const std::filesystem::path& path) = 0;
	virtual void onNextEntry(const std::filesystem::path& entry_path) = 0;
};


struct LF_ENTRY_STAT {
	LF_ENTRY_STAT() {
		stat = {};
		compressed_size = -1;
		is_encrypted = false;
	}
	virtual ~LF_ENTRY_STAT() {}
	void read_stat(const std::filesystem::path& src_path, const std::filesystem::path& stored_as) {
		int e = _wstat64(src_path.c_str(), &stat);
		if (e != 0) {
			throw ARCHIVE_EXCEPTION(errno);
		}

		compressed_size = -1;
		path = stored_as;
		method_name.clear();
		is_encrypted = false;
	}
	void write_stat(const std::filesystem::path& dest_path)const {
		struct __utimbuf64 ut;
		ut.actime = stat.st_atime;
		ut.modtime = stat.st_mtime;

		_wutime64(dest_path.c_str(), &ut);
	}

	struct _stat64 stat;
	__int64 compressed_size;	//-1 if unknown
	std::filesystem::path path;	//stored-as
	std::wstring method_name;
	bool is_encrypted;
	virtual bool is_directory()const { return stat.st_mode & S_IFDIR; }
};

struct LF_COMPRESS_ARGS;
class ILFArchiveFile
{
	DISALLOW_COPY_AND_ASSIGN(ILFArchiveFile);
public:
	ILFArchiveFile() {}
	virtual ~ILFArchiveFile() {}
	virtual void read_open(const std::filesystem::path& file, ILFPassphrase& passphrase) = 0;
	virtual void write_open(const std::filesystem::path& file, LF_ARCHIVE_FORMAT format, LF_WRITE_OPTIONS options, const LF_COMPRESS_ARGS& args, ILFPassphrase& passphrase) = 0;
	virtual void close() = 0;

	virtual bool is_modify_supported() const = 0;
	//make a copy, and returns in "write_open" state
	virtual std::unique_ptr<ILFArchiveFile> make_copy_archive(
		const std::filesystem::path& dest_path,
		const LF_COMPRESS_ARGS& args,
		std::function<bool(const LF_ENTRY_STAT&)> false_if_skip) = 0;

	//archive property
	virtual std::wstring get_format_name() = 0;	//works if file is opened
	virtual std::vector<LF_COMPRESS_CAPABILITY> get_compression_capability()const = 0;

	//entry seek; returns null if it reached EOF; valid for "read_open"ed archive
	virtual LF_ENTRY_STAT* read_entry_begin() = 0;	//rewinds to start of file
	virtual LF_ENTRY_STAT* read_entry_next() = 0;
	virtual void read_entry_end() = 0;

	virtual bool is_bypass_io_supported() const = 0;

	//read entry block; should be called until returned buffer becomes eof
	virtual LF_BUFFER_INFO read_file_entry_block() = 0;
	//read entry - bypasses decoder; can copy an entry with minimum IO cost
	virtual LF_BUFFER_INFO read_file_entry_bypass() = 0;

	//write entry
	virtual void add_file_entry(const LF_ENTRY_STAT&, std::function<LF_BUFFER_INFO()> dataProvider) = 0;
	//write entry - bypasses encoder; can copy an entry with minimum IO cost
	virtual void add_file_entry_bypass(const LF_ENTRY_STAT&, std::function<LF_BUFFER_INFO()> dataProvider) = 0;
	virtual void add_directory_entry(const LF_ENTRY_STAT&) = 0;
};



struct ARCLOG {
	ARCLOG() :_overallResult(LF_RESULT::OK) {}
	virtual ~ARCLOG() {}
	void setArchivePath(const std::filesystem::path& archivePath) {
		_archivePath = archivePath;
		_archiveFilename = std::filesystem::path(archivePath).filename().generic_wstring().c_str();
	}
	std::filesystem::path _archivePath;
	std::filesystem::path _archiveFilename;
	LF_RESULT _overallResult;

	struct LOGENTRY {
		std::filesystem::path entryPath;
		std::wstring message;
	};
	std::vector<LOGENTRY> logs;

	void operator()(const std::filesystem::path& entryPath, const std::wstring& message) {
		LOGENTRY e = { entryPath,message };
		logs.push_back(e);
	}
	std::wstring toString()const {
		std::wstring out;
		for (const auto& log : logs) {
			out += log.entryPath.wstring() + L" -> " + log.message + L"\r\n";
		}
		return out;
	}
	void logException(const LF_USER_CANCEL_EXCEPTION& e) {
		_overallResult = LF_RESULT::CANCELED;
		operator()(L"", e.what());
	}
	void logException(const LF_EXCEPTION& e) {
		_overallResult = LF_RESULT::NG;
		operator()(L"", e.what());
	}
	void logException(const ARCHIVE_EXCEPTION& e) {
		_overallResult = LF_RESULT::NOTARCHIVE;
		operator()(L"", e.what());
	}
};


//open with automatic backend selection
#define _LFA_SAFE_CALL(call)	\
if (m_ptr) {				\
	return m_ptr->call;		\
} else {					\
	RAISE_EXCEPTION(L"Archive is not opened");\
}

#define _LFA_SAFE_CALL_VOID(call)	\
if (m_ptr) {				\
	m_ptr->call;			\
} else {					\
	RAISE_EXCEPTION(L"Archive is not opened");\
}

class CLFArchive : public ILFArchiveFile
{
	DISALLOW_COPY_AND_ASSIGN(CLFArchive);
	std::unique_ptr<ILFArchiveFile> m_ptr;
	int64_t m_numEntries;
public:
	CLFArchive() :m_numEntries(-1) {}
	virtual ~CLFArchive() {}
	void read_open(const std::filesystem::path& file, ILFPassphrase& passphrase)override;
	void write_open(const std::filesystem::path& file, LF_ARCHIVE_FORMAT format, LF_WRITE_OPTIONS options, const LF_COMPRESS_ARGS& args, ILFPassphrase& passphrase)override;
	void close()override {
		if (m_ptr) {
			m_ptr->close();
		}
		m_ptr = nullptr;
		m_numEntries = -1;
	}

	bool is_modify_supported() const override { _LFA_SAFE_CALL(is_modify_supported()); }
	//make a copy, and returns in "write_open" state
	std::unique_ptr<ILFArchiveFile> make_copy_archive(
		const std::filesystem::path& dest_path,
		const LF_COMPRESS_ARGS& args,
		std::function<bool(const LF_ENTRY_STAT&)> false_if_skip)override {
		_LFA_SAFE_CALL(make_copy_archive(dest_path, args, false_if_skip));
	}

	//archive property
	std::wstring get_format_name()override { _LFA_SAFE_CALL(get_format_name()); }
	std::vector<LF_COMPRESS_CAPABILITY> get_compression_capability()const override;
	static LF_COMPRESS_CAPABILITY get_compression_capability(LF_ARCHIVE_FORMAT format);

	int64_t get_num_entries();	//-1 if no information is given
	//entry seek; returns null if it reached EOF; valid for "read_open"ed archive
	LF_ENTRY_STAT* read_entry_begin()override { _LFA_SAFE_CALL(read_entry_begin()); }//rewinds to start of file
	LF_ENTRY_STAT* read_entry_next()override { _LFA_SAFE_CALL(read_entry_next()); }
	void read_entry_end()override { _LFA_SAFE_CALL(read_entry_end()); }
	bool is_bypass_io_supported() const override { _LFA_SAFE_CALL(is_bypass_io_supported()); }

	//read entry block; should be called until returned buffer becomes eof
	LF_BUFFER_INFO read_file_entry_block()override {
		_LFA_SAFE_CALL(read_file_entry_block());
	}
	//read entry - bypasses decoder; can copy an entry with minimum IO cost
	LF_BUFFER_INFO read_file_entry_bypass()override {
		_LFA_SAFE_CALL(read_file_entry_bypass());
	}

	//write entry
	void add_file_entry(const LF_ENTRY_STAT& entry, std::function<LF_BUFFER_INFO()> dataProvider)override {
		_LFA_SAFE_CALL_VOID(add_file_entry(entry, dataProvider));
	}
	//write entry - bypasses encoder; can copy an entry with minimum IO cost
	void add_file_entry_bypass(const LF_ENTRY_STAT& entry, std::function<LF_BUFFER_INFO()> dataProvider)override {
		_LFA_SAFE_CALL_VOID(add_file_entry_bypass(entry, dataProvider));
	}
	void add_directory_entry(const LF_ENTRY_STAT& entry)override {
		_LFA_SAFE_CALL_VOID(add_directory_entry(entry));
	}

	//---
	static bool is_known_format(const std::filesystem::path& path);
};
#undef _LFA_SAFE_CALL
#undef _LFA_SAFE_CALL_VOID

//dummy interface
class CLFArchiveNULL : public ILFArchiveFile
{
public:
	CLFArchiveNULL() {}
	virtual ~CLFArchiveNULL() {}
	void read_open(const std::filesystem::path& file, ILFPassphrase& passphrase)override {}
	void write_open(const std::filesystem::path& file, LF_ARCHIVE_FORMAT format, LF_WRITE_OPTIONS options, const LF_COMPRESS_ARGS& args, ILFPassphrase& passphrase)override {}
	void close()override {}

	bool is_modify_supported()const override { return false; }
	//make a copy, and returns in "write_open" state
	std::unique_ptr<ILFArchiveFile> make_copy_archive(
		const std::filesystem::path& dest_path,
		const LF_COMPRESS_ARGS& args,
		std::function<bool(const LF_ENTRY_STAT&)> false_if_skip)override {
		return std::make_unique<CLFArchiveNULL>();
	}

	//archive property
	std::wstring get_format_name()override { return L"dummy"; }
	std::vector<LF_COMPRESS_CAPABILITY> get_compression_capability()const override { return {}; }

	int64_t get_num_entries() { return -1; }	//-1 if no information is given
	//entry seek; returns null if it reached EOF; valid for "read_open"ed archive
	LF_ENTRY_STAT* read_entry_begin()override { return nullptr; }//rewinds to start of file
	LF_ENTRY_STAT* read_entry_next()override { return nullptr; }
	void read_entry_end()override {}
	bool is_bypass_io_supported() const override { return false; }
	//read entry block; should be called until returned buffer becomes eof
	LF_BUFFER_INFO read_file_entry_block()override {
		LF_BUFFER_INFO buf;
		buf.make_eof();
		return buf;
	}
	//read entry - bypasses decoder; can copy an entry with minimum IO cost
	LF_BUFFER_INFO read_file_entry_bypass()override {
		LF_BUFFER_INFO buf;
		buf.make_eof();
		return buf;
	}

	//write entry
	void add_file_entry(const LF_ENTRY_STAT& entry, std::function<LF_BUFFER_INFO()> dataProvider)override {}
	//write entry - bypasses encoder; can copy an entry with minimum IO cost
	void add_file_entry_bypass(const LF_ENTRY_STAT& entry, std::function<LF_BUFFER_INFO()> dataProvider)override {}
	void add_directory_entry(const LF_ENTRY_STAT& entry)override {}
};
