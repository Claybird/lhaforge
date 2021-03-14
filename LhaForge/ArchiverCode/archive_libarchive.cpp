#include "stdafx.h"
#include "archive_libarchive.h"

#define LIBARCHIVE_STATIC
#include <libarchive/archive.h>
#include <libarchive/archive_entry.h>
#include <errno.h>

#include "ConfigCode/ConfigManager.h"
#include "Utilities/StringUtil.h"
#include "Utilities/FileOperation.h"

template <typename T, typename S>
void convert_stat(T& dest, const S& src)
{
	dest.st_dev   = src.st_dev;
	dest.st_ino   = src.st_ino;
	dest.st_mode  = src.st_mode;
	dest.st_nlink = src.st_nlink;
	dest.st_uid   = src.st_uid;
	dest.st_gid   = src.st_gid;
	dest.st_rdev  = src.st_rdev;
	dest.st_size  = (decltype(dest.st_size))src.st_size;
	dest.st_atime = src.st_atime;
	dest.st_mtime = src.st_mtime;
	dest.st_ctime = src.st_ctime;
}


struct LA_EXCEPTION : public ARCHIVE_EXCEPTION
{
	LA_EXCEPTION(archive* arc) :ARCHIVE_EXCEPTION(L"") {
		_errno = archive_errno(arc);
		auto msg = archive_error_string(arc);
		_msg = UtilUTF8toUNICODE(msg, strlen(msg));
	}
};

const char* LF_LA_passphrase(struct archive * arc, void *_client_data)
{
	//when callback returns nullptr, it should be handled as "user cancel"
	ILFPassphrase *pc = (ILFPassphrase*)_client_data;
	if (pc) {
		auto out = (*pc)();
		if (out) {
			return out;
		} else {
			CANCEL_EXCEPTION();
		}
	} else {
		CANCEL_EXCEPTION();
	}
}

struct LA_COMPRESSION_CAPABILITY :public LF_COMPRESS_CAPABILITY {
	int mapped_libarchive_format;
};

const static std::vector<LA_COMPRESSION_CAPABILITY> g_la_capabilities = {
	{LF_FMT_ZIP, L".zip", true, {
		LF_WOPT_STANDARD,
		LF_WOPT_SFX,
		LF_WOPT_DATA_ENCRYPTION,
		LF_WOPT_SFX | LF_WOPT_DATA_ENCRYPTION}, ARCHIVE_FORMAT_ZIP, },
	{LF_FMT_7Z, L".7z", true, {
		LF_WOPT_STANDARD,
		LF_WOPT_SFX,
		//LF_WOPT_DATA_ENCRYPTION,
		//LF_WOPT_HEADER_ENCRYPTION,
		//LF_WOPT_DATA_ENCRYPTION | LF_WOPT_HEADER_ENCRYPTION,
		//LF_WOPT_SFX | LF_WOPT_DATA_ENCRYPTION
		}, ARCHIVE_FORMAT_7ZIP, },
	{LF_FMT_GZ, L"{ext}.gz", false, {LF_WOPT_STANDARD}, ARCHIVE_FORMAT_RAW | ARCHIVE_FILTER_GZIP, },
	{LF_FMT_BZ2, L"{ext}.bz2", false, {LF_WOPT_STANDARD}, ARCHIVE_FORMAT_RAW | ARCHIVE_FILTER_BZIP2, },
	{LF_FMT_LZMA, L"{ext}.lzma", false, {LF_WOPT_STANDARD}, ARCHIVE_FORMAT_RAW | ARCHIVE_FILTER_LZMA, },
	{LF_FMT_XZ, L"{ext}.xz", false, {LF_WOPT_STANDARD}, ARCHIVE_FORMAT_RAW | ARCHIVE_FILTER_XZ, },
	{LF_FMT_ZSTD, L"{ext}.zst", false, {LF_WOPT_STANDARD}, ARCHIVE_FORMAT_RAW | ARCHIVE_FILTER_ZSTD, },
	{LF_FMT_TAR, L".tar", true, {LF_WOPT_STANDARD}, ARCHIVE_FORMAT_TAR},
	{LF_FMT_TAR_GZ, L".tar.gz", true, {LF_WOPT_STANDARD}, ARCHIVE_FORMAT_TAR | ARCHIVE_FILTER_GZIP, },
	{LF_FMT_TAR_BZ2, L".tar.bz2", true, {LF_WOPT_STANDARD}, ARCHIVE_FORMAT_TAR | ARCHIVE_FILTER_BZIP2, },
	{LF_FMT_TAR_LZMA, L".tar.lzma", true, {LF_WOPT_STANDARD}, ARCHIVE_FORMAT_TAR | ARCHIVE_FILTER_LZMA, },
	{LF_FMT_TAR_XZ, L".tar.xz", true, {LF_WOPT_STANDARD}, ARCHIVE_FORMAT_TAR | ARCHIVE_FILTER_XZ, },
	{LF_FMT_TAR_ZSTD, L".tar.zst", true, {LF_WOPT_STANDARD}, ARCHIVE_FORMAT_TAR | ARCHIVE_FILTER_ZSTD, },
	{LF_FMT_UUE, L".uue", false, {LF_WOPT_STANDARD}, ARCHIVE_FORMAT_RAW | ARCHIVE_FILTER_UU, },
};


static const LA_COMPRESSION_CAPABILITY& la_get_compression_capability(LF_ARCHIVE_FORMAT fmt)
{
	for (const auto &cap : g_la_capabilities) {
		if (fmt == cap.format) {
			return cap;
		}
	}
	throw ARCHIVE_EXCEPTION(EINVAL);
}


struct LF_LA_ENTRY {
	DISALLOW_COPY_AND_ASSIGN(LF_LA_ENTRY);
	struct archive *_arc;
	struct archive_entry *_entry;

	LF_ENTRY_STAT _lf_stat;

	LF_LA_ENTRY() :_arc(nullptr), _entry(nullptr) { renew(); }
	LF_LA_ENTRY(archive_entry* entry) = delete;
	virtual ~LF_LA_ENTRY() {
		if (_entry) {
			archive_entry_free(_entry);
			_entry = nullptr;
		}
	}
	archive_entry* la_entry() { return _entry; }
	void clear_entry_stat() {
		_lf_stat = LF_ENTRY_STAT();
	}
	void renew() {
		if (_entry) {
			archive_entry_free(_entry);
			_entry = nullptr;
		}
		_entry = archive_entry_new();
		clear_entry_stat();
	}
	void set_archive(archive* arc) {
		_arc = arc;
		renew();
	}

	bool read_next() {
		clear_entry_stat();
		int r = archive_read_next_header2(_arc, _entry);
		if (ARCHIVE_OK == r) {
			auto s = archive_entry_stat(_entry);
			if (s) {
				convert_stat(_lf_stat.stat, *s);
			} else {
				clear_entry_stat();
			}
			_lf_stat.compressed_size = -1;
			_lf_stat.path = archive_entry_pathname_w(_entry);
			_lf_stat.method_name = L"---";
			_lf_stat.is_encrypted = archive_entry_is_encrypted(_entry);
			return true;
		} else if (ARCHIVE_EOF == r) {
			return false;
		} else {
			throw LA_EXCEPTION(_arc);
		}
	}

	void set_stat(const LF_ENTRY_STAT& stat) {
		_lf_stat = stat;
		archive_entry_copy_pathname_w(_entry, _lf_stat.path.c_str());
		struct stat s;
		convert_stat(s, _lf_stat.stat);
		archive_entry_copy_stat(_entry, &s);
		archive_entry_set_size(_entry, _lf_stat.stat.st_size);	//make sure file size is in 64bit
	}
};

struct LA_FILE_TO_READ
{
	DISALLOW_COPY_AND_ASSIGN(LA_FILE_TO_READ);
	struct archive *_arc;
	LF_LA_ENTRY _entry;
	struct REWIND {
		bool need_rewind;
		std::filesystem::path arcpath;
		ILFPassphrase* passphrase;
	}_rewind;

	LA_FILE_TO_READ() :_arc(nullptr) {}
	virtual ~LA_FILE_TO_READ() {
		close();
	}
	void open(const std::filesystem::path& arcpath, ILFPassphrase& passphrase) {
		close();
		_rewind.arcpath = arcpath;
		_rewind.need_rewind = false;
		_rewind.passphrase = &passphrase;
		_arc = archive_read_new();
		archive_read_support_filter_all(_arc);
		archive_read_support_format_all(_arc);

		int r = archive_read_set_passphrase_callback(_arc, &passphrase, LF_LA_passphrase);
		if (r < ARCHIVE_OK) {
			throw LA_EXCEPTION(_arc);
		}
		r = archive_read_open_filename_w(_arc, arcpath.c_str(), 10240);
		if (r < ARCHIVE_OK) {
			//retry enabling archive_read_support_format_raw
			close();
			if (is_known_format(arcpath)) {
				_arc = archive_read_new();
				r = archive_read_set_passphrase_callback(_arc, &passphrase, LF_LA_passphrase);
				if (r < ARCHIVE_OK) {
					throw LA_EXCEPTION(_arc);
				}
				archive_read_support_filter_all(_arc);
				archive_read_support_format_raw(_arc);
				r = archive_read_open_filename_w(_arc, arcpath.c_str(), 10240);
				if (r < ARCHIVE_OK) {
					throw LA_EXCEPTION(_arc);
				}
			} else {
				throw ARCHIVE_EXCEPTION(L"Invalid format");
			}
		}
	}
	void close() {
		if (_arc) {
			archive_read_close(_arc);
			archive_read_free(_arc);
		}
		_arc = nullptr;
	}
	void rewind() {
		if (_arc){
			if (_rewind.need_rewind) {
				open(_rewind.arcpath, *_rewind.passphrase);
			}
		} else {
			throw ARCHIVE_EXCEPTION(EFAULT);
		}
	}
	LF_LA_ENTRY* begin() {
		rewind();
		_entry.set_archive(_arc);
		return next();
	}
	LF_LA_ENTRY* next() {
		_rewind.need_rewind = true;
		if (_entry.read_next()) {
			return &_entry;
		} else {
			return nullptr;
		}
	}

	operator struct archive*() { return _arc; }

	LF_BUFFER_INFO read_block() {
		LF_BUFFER_INFO ibi;
		size_t size;
		int r = archive_read_data_block(_arc, &ibi.buffer, &size, &ibi.offset);
		ibi.size = size;
		if (ARCHIVE_EOF == r) {
			ibi.make_eof();
		} else if (r < ARCHIVE_OK) {
			throw LA_EXCEPTION(_arc);
		}
		return ibi;
	}
	static bool is_known_format(const std::filesystem::path &arcname) {
		const size_t readSize = 10;
		try {
			if (std::filesystem::file_size(arcname) < readSize)return false;
		} catch (...) {
			return false;
		}

		CAutoFile fp;
		fp.open(arcname);
		if (!fp.is_opened())return false;
		std::vector<unsigned char> header(readSize);
		if (readSize != fread(&header[0], 1, readSize, fp)) {
			return false;
		}

		//check header for known format
		//gzip: RFC 1952
		if (header[0] == 0x1f && header[1] == 0x8b) {
			return true;
		}
		//bz2: https://github.com/dsnet/compress/blob/master/doc/bzip2-format.pdf
		if (header[0] == 'B' && header[1] == 'Z' && header[2] == 'h' &&
			'0' <= header[3] && header[3] <= '9' &&
			(
				//compressed_magic
			(header[4] == 0x31 && header[5] == 0x41 && header[6] == 0x59 &&
				header[7] == 0x26 && header[8] == 0x53 && header[9] == 0x59) ||
				//eos_magic
				(header[4] == 0x17 && header[5] == 0x72 && header[6] == 0x45 &&
					header[7] == 0x38 && header[8] == 0x50 && header[9] == 0x90)
				)
			) {
			return true;
		}
		//lzma: lzma-file-format.txt in XZ Utils[https://tukaani.org/xz/]
		{
			uint8_t prop = header[0];
			if (prop <= (4 * 5 + 4) * 9 + 8) {
				return true;
			}
		}
		//xz: xz-file-format.txt in XZ Utils[https://tukaani.org/xz/]
		if (header[0] == 0xFD && header[1] == '7' && header[2] == 'z' &&
			header[3] == 'X' && header[4] == 'Z' && header[5] == 0x00) {
			return true;
		}

		//zstd: https://tools.ietf.org/id/draft-kucherawy-dispatch-zstd-00.html
		if (header[0] == 0xFD && header[1] == 0x2F && header[2] == 0xB5 && header[3] == 0x28) {
			return true;
		}

		return false;
	}
};

struct LA_FILE_TO_WRITE
{
	DISALLOW_COPY_AND_ASSIGN(LA_FILE_TO_WRITE);
	struct archive *_arc;
	LF_LA_ENTRY _entry;

	LA_FILE_TO_WRITE() :_arc(nullptr) {}
	virtual ~LA_FILE_TO_WRITE() {
		close();
	}

	void open(const std::filesystem::path& arcname,
		LF_ARCHIVE_FORMAT fmt,
		const std::map<std::string, std::string> &archive_options,
		ILFPassphrase& passphrase) {
		const auto& cap = la_get_compression_capability(fmt);

		int la_filter = cap.mapped_libarchive_format & ~ARCHIVE_FORMAT_BASE_MASK;
		int la_fmt = cap.mapped_libarchive_format & ARCHIVE_FORMAT_BASE_MASK;

		write_open_la(arcname, la_fmt, { la_filter }, archive_options, passphrase);
	}
	void write_open_la(const std::wstring& arcname,
		int la_fmt,
		const std::vector<int> &filters,
		const std::map<std::string, std::string> &archive_options,
		ILFPassphrase &passphrase) {
		close();
		_arc = archive_write_new();

		for (auto f : filters) {
			int r = archive_write_add_filter(_arc, f);
			if (r < ARCHIVE_OK) {
				throw LA_EXCEPTION(_arc);
			}
		}

		int r = archive_write_set_format(_arc, la_fmt);
		if (r < ARCHIVE_OK) {
			throw LA_EXCEPTION(_arc);
		}

		r = archive_write_set_passphrase_callback(_arc, &passphrase, LF_LA_passphrase);
		if (r < ARCHIVE_OK) {
			throw LA_EXCEPTION(_arc);
		}
		for (auto &ite : archive_options) {
			int r = archive_write_set_option(_arc, nullptr, ite.first.c_str(), ite.second.c_str());
			if (r < ARCHIVE_OK) {
				throw LA_EXCEPTION(_arc);
			}
		}
		r = archive_write_open_filename_w(_arc, arcname.c_str());
		if (r < ARCHIVE_OK) {
			throw LA_EXCEPTION(_arc);
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
	void add_entry(LF_LA_ENTRY& entry, T& dataProvider) {
		int r = archive_write_header(_arc, entry.la_entry());
		if (r < ARCHIVE_OK) {
			throw LA_EXCEPTION(_arc);
		}

		while (true) {
			LF_BUFFER_INFO ibi = dataProvider();
			if (ibi.is_eof()) {
				break;
			} else {
				archive_write_data(_arc, ibi.buffer, (size_t)ibi.size);
			}
		}
	}
	void add_directory(LF_LA_ENTRY& entry) {
		int r = archive_write_header(_arc, entry.la_entry());
		if (r < ARCHIVE_OK) {
			throw LA_EXCEPTION(_arc);
		}
	}
	operator struct archive*() { return _arc; }


	//get most similar option
	static std::tuple<int/*la_format*/, std::vector<int>/*filters*/, bool /*is_encrypted*/>
	mimic_archive_property(LA_FILE_TO_READ& src_archive) {
		//scan for file content; to know archive information
		bool is_src_encrypted = false;
		for (LF_LA_ENTRY* entry = src_archive.begin(); entry; entry = src_archive.next()) {
			if (entry->_lf_stat.is_encrypted) {
				is_src_encrypted = true;
				break;
			}
		}

		int la_format = archive_format(src_archive);

		std::vector<int> filters;
		auto filter_count = archive_filter_count(src_archive);
		for (int i = 0; i < filter_count; i++) {
			auto code = archive_filter_code(src_archive, i);
			filters.push_back(code);
		}

		src_archive.rewind();

		return { la_format, filters, is_src_encrypted };
	}
};

#include "compress.h"

std::map<std::string, std::string> getLAOptionsFromConfig(
	int la_format,
	const std::vector<int> &la_filters,
	bool encrypt,
	const LF_COMPRESS_ARGS &args)
{
	std::map<std::string, std::string> params;
	//formats
	switch (la_format & ARCHIVE_FORMAT_BASE_MASK) {
	case ARCHIVE_FORMAT_ZIP:
	{
		merge_map(params, args.formats.zip.params);
		if (!encrypt) {
			params.erase("encryption");
		}
	}
		break;
	case ARCHIVE_FORMAT_7ZIP:
		merge_map(params, args.formats.sevenzip.params);
		break;
	case ARCHIVE_FORMAT_TAR:
		merge_map(params, args.formats.tar.params);
		break;
	case ARCHIVE_FORMAT_RAW:
		//nothing to do
		break;
	}

	//filters
	for (auto la_filter : la_filters) {
		switch (la_filter & ~ARCHIVE_FORMAT_BASE_MASK) {
		case ARCHIVE_FILTER_GZIP:
			merge_map(params, args.formats.gz.params);
			break;
		case ARCHIVE_FILTER_BZIP2:
			merge_map(params, args.formats.bz2.params);
			break;
		case ARCHIVE_FILTER_LZMA:
			merge_map(params, args.formats.lzma.params);
			break;
		case ARCHIVE_FILTER_XZ:
			merge_map(params, args.formats.xz.params);
			break;
		case ARCHIVE_FILTER_ZSTD:
			merge_map(params, args.formats.zstd.params);
			break;
		}
	}
	return params;
}

std::map<std::string, std::string> getLAOptionsFromConfig(
	const LF_COMPRESS_ARGS &args,
	LF_ARCHIVE_FORMAT format,
	LF_WRITE_OPTIONS options)
{
	const auto& cap = la_get_compression_capability(format);
	int la_format = cap.mapped_libarchive_format & ARCHIVE_FORMAT_BASE_MASK;
	std::vector<int> la_filters = { cap.mapped_libarchive_format & ~ARCHIVE_FORMAT_BASE_MASK };

	bool encrypt = (options & LF_WOPT_DATA_ENCRYPTION) != 0;
	return getLAOptionsFromConfig(la_format, la_filters, encrypt, args);
}

//------

CLFArchiveLA::CLFArchiveLA() {}
CLFArchiveLA::~CLFArchiveLA() {}

void CLFArchiveLA::read_open(const std::filesystem::path& file, ILFPassphrase& passhprase)
{
	close();
	_arc_read = std::make_unique<LA_FILE_TO_READ>();
	_arc_read->open(file, passhprase);
}

void CLFArchiveLA::write_open(
	const std::filesystem::path& file,
	LF_ARCHIVE_FORMAT format,
	LF_WRITE_OPTIONS options,
	const LF_COMPRESS_ARGS& args,
	ILFPassphrase& passphrase)
{
	_arc_write = std::make_unique<LA_FILE_TO_WRITE>();

	auto flags = getLAOptionsFromConfig(args, format, options);
	_arc_write->open(file, format, flags, passphrase);
}

void CLFArchiveLA::close()
{
	if (_arc_read) {
		_arc_read->close();
		_arc_read.reset();
	}
	if (_arc_write) {
		_arc_write->close();
		_arc_write.reset();
	}
}

//archive property
std::wstring CLFArchiveLA::get_format_name()
{
	if (_arc_read) {
		auto p = archive_format_name(*_arc_read);	//differ on each entry
		if (p)return UtilUTF8toUNICODE(p);
	}
	if (_arc_write) {
		auto p = archive_format_name(*_arc_write);	//differ on each entry
		if (p)return UtilUTF8toUNICODE(p);
	}
	return L"---";
}

std::vector<LF_COMPRESS_CAPABILITY> CLFArchiveLA::get_compression_capability()const
{
	std::vector<LF_COMPRESS_CAPABILITY> caps(g_la_capabilities.begin(), g_la_capabilities.end());
	return caps;
}

LF_ENTRY_STAT* CLFArchiveLA::read_entry_begin()
{
	if (_arc_read) {
		auto p = _arc_read->begin();
		if (p) {
			return &p->_lf_stat;
		} else {
			return nullptr;
		}
	} else {
		throw ARCHIVE_EXCEPTION(EFAULT);
	}
}

LF_ENTRY_STAT* CLFArchiveLA::read_entry_next()
{
	if (_arc_read) {
		auto p = _arc_read->next();
		if (p) {
			return &p->_lf_stat;
		} else {
			return nullptr;
		}
	} else {
		throw ARCHIVE_EXCEPTION(EFAULT);
	}
}

void CLFArchiveLA::read_entry_end()
{
	if (_arc_read) {
		_arc_read->rewind();
	} else {
		throw ARCHIVE_EXCEPTION(EFAULT);
	}
}


//read file entry
LF_BUFFER_INFO CLFArchiveLA::read_file_entry_block()
{
	if (_arc_read) {
		return _arc_read->read_block();
	} else {
		throw ARCHIVE_EXCEPTION(EFAULT);
	}
}

//write entry
void CLFArchiveLA::add_file_entry(
	const LF_ENTRY_STAT& lf_stat,
	std::function<LF_BUFFER_INFO()> dataProvider)
{
	if (_arc_write) {
		LF_LA_ENTRY la_entry;
		la_entry.set_archive(*_arc_write);
		la_entry.set_stat(lf_stat);
		_arc_write->add_entry(la_entry, dataProvider);
	} else {
		throw ARCHIVE_EXCEPTION(EFAULT);
	}
}

void CLFArchiveLA::add_directory_entry(const LF_ENTRY_STAT& lf_stat)
{
	if (_arc_write) {
		LF_LA_ENTRY la_entry;
		la_entry.set_archive(*_arc_write);
		la_entry.set_stat(lf_stat);
		_arc_write->add_directory(la_entry);
	} else {
		throw ARCHIVE_EXCEPTION(EFAULT);
	}
}


//make a copy, and returns in "write_open" state
std::unique_ptr<ILFArchiveFile> CLFArchiveLA::make_copy_archive(
	const std::filesystem::path& dest_path,
	const LF_COMPRESS_ARGS& args,
	std::function<bool(const LF_ENTRY_STAT&)> false_if_skip)
{
	if (_arc_read) {
		ASSERT(_arc_read->_rewind.passphrase);
		auto[la_format, filters, is_encrypted] = LA_FILE_TO_WRITE::mimic_archive_property(*_arc_read);
		std::unique_ptr<CLFArchiveLA> dest_archive = std::make_unique<CLFArchiveLA>();
		dest_archive->_arc_write = std::make_unique<LA_FILE_TO_WRITE>();

		//- open an output archive in most similar option
		auto options = getLAOptionsFromConfig(la_format, filters, is_encrypted, args);
		dest_archive->_arc_write->write_open_la(dest_path, la_format, filters, options, *_arc_read->_rewind.passphrase);

		//- then, copy entries if filter returns true
		//this would need overhead of extract on read and compress on write
		//there seems no way to get raw data
		_arc_read->rewind();
		for (LF_LA_ENTRY* entry = _arc_read->begin(); entry; entry = _arc_read->next()) {
			if (false_if_skip(entry->_lf_stat)) {
				if (entry->_lf_stat.is_directory()) {
					dest_archive->_arc_write->add_directory(*entry);
				} else {
					dest_archive->_arc_write->add_entry(*entry, [&]() {
						while (UtilDoMessageLoop())continue;	//TODO
						//TODO progress handler
						return _arc_read->read_block();
					});
				}
			}
		}

		//- copy finished. now the caller can add extra files
		return dest_archive;
	} else {
		throw ARCHIVE_EXCEPTION(EFAULT);
	}
}

#include "CommonUtil.h"

bool CLFArchiveLA::is_known_format(const std::filesystem::path &arcname)
{
	if (LA_FILE_TO_READ::is_known_format(arcname)) {
		return true;
		/*
		the following test is goes too deep into file. checking header should be enough
		LA_FILE_TO_READ arc;
		try {
			arc.open(arcname, CLFPassphraseNULL());
			for (auto* entry = arc.begin(); entry; entry = arc.next()) {
				continue;
			}
			return true;
		} catch (const LF_USER_CANCEL_EXCEPTION&) {
			//encrypted, but passphrase is not provided
			return true;
		} catch (const ARCHIVE_EXCEPTION&) {
			return false;
		}*/
	} else {
		return false;
	}
}


#ifdef UNIT_TEST
#include <gtest/gtest.h>
#include "ArchiverCode/archive.h"

TEST(CLFArchiveLA, isKnownArchive)
{
	const auto dir = LF_PROJECT_DIR() / L"ArchiverCode/test";
	EXPECT_TRUE(LA_FILE_TO_READ::is_known_format(dir / L"empty.gz"));
	EXPECT_TRUE(LA_FILE_TO_READ::is_known_format(dir / L"empty.bz2"));
	EXPECT_TRUE(LA_FILE_TO_READ::is_known_format(dir / L"empty.xz"));
	EXPECT_TRUE(LA_FILE_TO_READ::is_known_format(dir / L"empty.lzma"));
	EXPECT_TRUE(LA_FILE_TO_READ::is_known_format(dir / L"empty.zst"));

	EXPECT_TRUE(LA_FILE_TO_READ::is_known_format(dir / L"abcde.gz"));
	EXPECT_TRUE(LA_FILE_TO_READ::is_known_format(dir / L"abcde.bz2"));
	EXPECT_TRUE(LA_FILE_TO_READ::is_known_format(dir / L"abcde.xz"));
	EXPECT_TRUE(LA_FILE_TO_READ::is_known_format(dir / L"abcde.lzma"));
	EXPECT_TRUE(LA_FILE_TO_READ::is_known_format(dir / L"abcde.zst"));

	EXPECT_FALSE(LA_FILE_TO_READ::is_known_format(__FILEW__));
	EXPECT_FALSE(LA_FILE_TO_READ::is_known_format(L"some_non_existing_file"));
}

TEST(CLFArchiveLA, getLAOptionsFromConfig)
{
	LF_COMPRESS_ARGS fake_args;
	CConfigManager mngr;
	fake_args.load(mngr);
	{
		auto la_options = getLAOptionsFromConfig(fake_args, LF_FMT_ZIP, LF_WOPT_STANDARD);
		EXPECT_EQ(4, la_options.size());
		EXPECT_EQ("deflate", la_options.at("compression"));
		EXPECT_EQ("9", la_options.at("compression-level"));
		//EXPECT_EQ("ZipCrypt", la_options.at("encryption"));
		EXPECT_EQ("UTF-8", la_options.at("hdrcharset"));
		EXPECT_EQ("enabled", la_options.at("zip64"));
	}
	{
		auto la_options = getLAOptionsFromConfig(fake_args, LF_FMT_ZIP, LF_WOPT_DATA_ENCRYPTION);
		EXPECT_EQ(5, la_options.size());
		EXPECT_EQ("deflate", la_options.at("compression"));
		EXPECT_EQ("9", la_options.at("compression-level"));
		EXPECT_EQ("ZipCrypt", la_options.at("encryption"));
		EXPECT_EQ("UTF-8", la_options.at("hdrcharset"));
		EXPECT_EQ("enabled", la_options.at("zip64"));
	}
	{
		auto la_options = getLAOptionsFromConfig(fake_args, LF_FMT_7Z, LF_WOPT_STANDARD);
		EXPECT_EQ(2, la_options.size());
		EXPECT_EQ("deflate", la_options.at("compression"));
		EXPECT_EQ("9", la_options.at("compression-level"));
	}

	{
		auto la_options = getLAOptionsFromConfig(fake_args, LF_FMT_TAR, LF_WOPT_STANDARD);
		EXPECT_EQ(1, la_options.size());
		EXPECT_EQ("UTF-8", la_options.at("hdrcharset"));
	}

	{
		auto la_options = getLAOptionsFromConfig(fake_args, LF_FMT_GZ, LF_WOPT_STANDARD);
		EXPECT_EQ(1, la_options.size());
		EXPECT_EQ("9", la_options.at("compression-level"));
	}

	{
		auto la_options = getLAOptionsFromConfig(fake_args, LF_FMT_BZ2, LF_WOPT_STANDARD);
		EXPECT_EQ(1, la_options.size());
		EXPECT_EQ("9", la_options.at("compression-level"));
	}

	{
		auto la_options = getLAOptionsFromConfig(fake_args, LF_FMT_XZ, LF_WOPT_STANDARD);
		EXPECT_EQ(2, la_options.size());
		EXPECT_EQ("9", la_options.at("compression-level"));
		EXPECT_EQ("0", la_options.at("threads"));
	}

	{
		auto la_options = getLAOptionsFromConfig(fake_args, LF_FMT_LZMA, LF_WOPT_STANDARD);
		EXPECT_EQ(1, la_options.size());
		EXPECT_EQ("9", la_options.at("compression-level"));
	}

	{
		auto la_options = getLAOptionsFromConfig(fake_args, LF_FMT_ZSTD, LF_WOPT_STANDARD);
		EXPECT_EQ(1, la_options.size());
		EXPECT_EQ("3", la_options.at("compression-level"));
	}
}

TEST(CLFArchiveLA, mimic_archive_property)
{
	{
		auto fileToRead = LF_PROJECT_DIR() / L"test/test_extract.zip";
		LA_FILE_TO_READ src;
		src.open(fileToRead, CLFPassphraseNULL());
		src.begin();	//need to scan
		auto[la_format, filters, is_encrypted] = LA_FILE_TO_WRITE::mimic_archive_property(src);
		EXPECT_EQ(la_format, ARCHIVE_FORMAT_ZIP);
		EXPECT_EQ(filters.size(), 1);
		EXPECT_EQ(filters.back(), ARCHIVE_FILTER_NONE);
		EXPECT_FALSE(is_encrypted);
	}
	{
		auto fileToRead = LF_PROJECT_DIR() / L"test/test_gzip.gz";
		LA_FILE_TO_READ src;
		src.open(fileToRead, CLFPassphraseNULL());
		src.begin();	//need to scan
		auto[la_format, filters, is_encrypted] = LA_FILE_TO_WRITE::mimic_archive_property(src);
		EXPECT_EQ(la_format, ARCHIVE_FORMAT_RAW);
		EXPECT_EQ(filters.size(), 2);
		EXPECT_EQ(filters[0], ARCHIVE_FILTER_GZIP);
		EXPECT_EQ(filters[1], ARCHIVE_FILTER_NONE);
		EXPECT_FALSE(is_encrypted);
	}
	{
		auto fileToRead = LF_PROJECT_DIR() / L"test/test.tar.gz";
		LA_FILE_TO_READ src;
		src.open(fileToRead, CLFPassphraseNULL());
		src.begin();	//need to scan
		auto[la_format, filters, is_encrypted] = LA_FILE_TO_WRITE::mimic_archive_property(src);
		EXPECT_TRUE(la_format & ARCHIVE_FORMAT_TAR);
		EXPECT_EQ(filters.size(), 2);
		EXPECT_EQ(filters[0], ARCHIVE_FILTER_GZIP);
		EXPECT_EQ(filters[1], ARCHIVE_FILTER_NONE);
		EXPECT_FALSE(is_encrypted);
	}
}

#endif
