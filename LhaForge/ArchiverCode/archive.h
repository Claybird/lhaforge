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
#include "resource.h"

enum class LOGVIEW : int {
	OnError,
	Always,
	Never,

	ENUM_COUNT_AND_LASTITEM
};

enum class LF_RESULT :int {
	OK,//successful or archive test passed
	NG,//abnormal end or archive test failed
	CANCELED,//user cancel
	NOTARCHIVE,//not an archive
	NOTIMPL,//not implemented
};

enum class LOSTDIR : int {
	AskToCreate,
	ForceCreate,
	Error,

	ENUM_COUNT_AND_LASTITEM
};
enum class OUTPUT_TO :int {
	NoOverride = -1,
	Desktop,
	SameDir,
	SpecificDir,
	AlwaysAsk,

	ENUM_COUNT_AND_LASTITEM
};


struct ARCHIVE_EXCEPTION : public LF_EXCEPTION {
	int _errno;
	ARCHIVE_EXCEPTION(const std::wstring &err) :LF_EXCEPTION(err) {
		_errno = 0;
	}
	ARCHIVE_EXCEPTION(int errno_code) :LF_EXCEPTION(L"") {
		const int max_msg_len = 94;
		wchar_t work[max_msg_len] = {};
		_errno = errno_code;
		_wcserror_s(work, _errno);
		_msg = work;
	}
	virtual ~ARCHIVE_EXCEPTION() {}
};

enum class LF_ARCHIVE_FORMAT :int {
	INVALID = -1,
	ZIP,
	_7Z,
	GZ,
	BZ2,
	LZMA,
	XZ,
	ZSTD,
	TAR,
	TAR_GZ,
	TAR_BZ2,
	TAR_LZMA,
	TAR_XZ,
	TAR_ZSTD,

	//---read only format
	READONLY,

	ENUM_COUNT_AND_LASTITEM
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

struct offset_info {
	uint64_t offset;	//absolute position
	//int origin;
};

struct LF_BUFFER_INFO {
	const void* buffer;
	size_t size;
	const offset_info* offset;
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
	virtual void setSpecialMessage(const std::wstring& msg) = 0;
	virtual void poll() = 0;	//detect cancel
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
	virtual void read_open(const std::filesystem::path& file, std::shared_ptr<ILFPassphrase> passphrase) = 0;
	virtual void write_open(const std::filesystem::path& file, LF_ARCHIVE_FORMAT format, LF_WRITE_OPTIONS options, const LF_COMPRESS_ARGS& args, std::shared_ptr<ILFPassphrase> passphrase) = 0;
	virtual void close() = 0;
	virtual std::filesystem::path get_archive_path()const = 0;

	virtual bool is_modify_supported() const = 0;
	//make a copy, and returns in "write_open" state
	virtual std::unique_ptr<ILFArchiveFile> make_copy_archive(
		const std::filesystem::path& dest_path,
		const LF_COMPRESS_ARGS& args,
		std::function<bool(const LF_ENTRY_STAT&)> false_to_skip) = 0;

	//archive property
	virtual LF_ARCHIVE_FORMAT get_format() = 0;	//works if file is opened
	virtual std::wstring get_format_name() = 0;	//works if file is opened
	virtual std::vector<LF_COMPRESS_CAPABILITY> get_compression_capability()const = 0;

	//entry seek; returns null if it reached EOF; valid for "read_open"ed archive
	virtual LF_ENTRY_STAT* read_entry_begin() = 0;	//rewinds to start of file
	virtual LF_ENTRY_STAT* read_entry_next() = 0;
	virtual void read_entry_end() = 0;

	//read entry block; should be called until returned buffer becomes eof
	virtual void read_file_entry_block(std::function<void(const void*, size_t, const offset_info*)> data_receiver) = 0;

	//write entry
	virtual void add_file_entry(const LF_ENTRY_STAT&, std::function<LF_BUFFER_INFO()> dataProvider) = 0;
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


#define _LFA_SAFE_CALL(call)	\
if (m_ptr) {				\
	return m_ptr->call;		\
} else {					\
	RAISE_EXCEPTION(UtilLoadString(IDS_ARCHIVE_IS_NOT_OPENED));\
}

#define _LFA_SAFE_CALL_VOID(call)	\
if (m_ptr) {				\
	m_ptr->call;			\
} else {					\
	RAISE_EXCEPTION(UtilLoadString(IDS_ARCHIVE_IS_NOT_OPENED));\
}

//open with automatic backend selection
class CLFArchive : public ILFArchiveFile
{
	DISALLOW_COPY_AND_ASSIGN(CLFArchive);
	std::unique_ptr<ILFArchiveFile> m_ptr;
	int64_t m_numEntries;
public:
	CLFArchive() :m_numEntries(-1) {}
	virtual ~CLFArchive() {}

	std::filesystem::path get_archive_path()const override {
		if (m_ptr.get()) {
			return m_ptr->get_archive_path();
		} else {
			return L"";
		}
	}
	void read_open(const std::filesystem::path& file, std::shared_ptr<ILFPassphrase> passphrase)override;
	void write_open(const std::filesystem::path& file, LF_ARCHIVE_FORMAT format, LF_WRITE_OPTIONS options, const LF_COMPRESS_ARGS& args, std::shared_ptr<ILFPassphrase> passphrase)override;
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
		std::function<bool(const LF_ENTRY_STAT&)> false_to_skip)override {
		_LFA_SAFE_CALL(make_copy_archive(dest_path, args, false_to_skip));
	}

	//archive property
	LF_ARCHIVE_FORMAT get_format()override { _LFA_SAFE_CALL(get_format()); }
	std::wstring get_format_name()override { _LFA_SAFE_CALL(get_format_name()); }
	std::vector<LF_COMPRESS_CAPABILITY> get_compression_capability()const override;
	static LF_COMPRESS_CAPABILITY get_compression_capability(LF_ARCHIVE_FORMAT format);

	int64_t get_num_entries();	//-1 if no information is given
	//entry seek; returns null if it reached EOF; valid for "read_open"ed archive
	LF_ENTRY_STAT* read_entry_begin()override { _LFA_SAFE_CALL(read_entry_begin()); }//rewinds to start of file
	LF_ENTRY_STAT* read_entry_next()override { _LFA_SAFE_CALL(read_entry_next()); }
	void read_entry_end()override { _LFA_SAFE_CALL(read_entry_end()); }

	//read entry block; should be called until returned buffer becomes eof
	void read_file_entry_block(std::function<void(const void*, size_t/*data size*/, const offset_info*)> data_receiver)override {
		_LFA_SAFE_CALL(read_file_entry_block(data_receiver));
	}

	//write entry
	void add_file_entry(const LF_ENTRY_STAT& entry, std::function<LF_BUFFER_INFO()> dataProvider)override {
		_LFA_SAFE_CALL_VOID(add_file_entry(entry, dataProvider));
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
	std::filesystem::path get_archive_path()const override { return L""; }
	void read_open(const std::filesystem::path& file, std::shared_ptr<ILFPassphrase> passphrase)override {}
	void write_open(const std::filesystem::path& file, LF_ARCHIVE_FORMAT format, LF_WRITE_OPTIONS options, const LF_COMPRESS_ARGS& args, std::shared_ptr<ILFPassphrase> passphrase)override {}
	void close()override {}

	bool is_modify_supported()const override { return false; }
	//make a copy, and returns in "write_open" state
	std::unique_ptr<ILFArchiveFile> make_copy_archive(
		const std::filesystem::path& dest_path,
		const LF_COMPRESS_ARGS& args,
		std::function<bool(const LF_ENTRY_STAT&)> false_to_skip)override {
		return std::make_unique<CLFArchiveNULL>();
	}

	//archive property
	LF_ARCHIVE_FORMAT get_format()override { return LF_ARCHIVE_FORMAT::INVALID; }
	std::wstring get_format_name()override { return L"dummy"; }
	std::vector<LF_COMPRESS_CAPABILITY> get_compression_capability()const override { return {}; }

	int64_t get_num_entries() { return -1; }	//-1 if no information is given
	//entry seek; returns null if it reached EOF; valid for "read_open"ed archive
	LF_ENTRY_STAT* read_entry_begin()override { return nullptr; }//rewinds to start of file
	LF_ENTRY_STAT* read_entry_next()override { return nullptr; }
	void read_entry_end()override {}

	//read entry block; should be called until returned buffer becomes eof
	void read_file_entry_block(std::function<void(const void*, size_t/*data size*/, const offset_info*)> data_receiver)override {
		data_receiver(nullptr, 0, 0);
	}

	//write entry
	void add_file_entry(const LF_ENTRY_STAT& entry, std::function<LF_BUFFER_INFO()> dataProvider)override {}
	void add_directory_entry(const LF_ENTRY_STAT& entry)override {}
};
