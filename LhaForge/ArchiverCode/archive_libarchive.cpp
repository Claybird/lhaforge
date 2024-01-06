#include "stdafx.h"
#include "archive_libarchive.h"

#define LIBARCHIVE_STATIC
#include <libarchive/archive.h>
#include <libarchive/archive_entry.h>
#include <errno.h>

#include "ConfigCode/ConfigFile.h"
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

//#include <libarchive/archive_platform.h>
struct LA_EXCEPTION : public ARCHIVE_EXCEPTION
{
protected:
	std::wstring error_string(int la_errno)const {
		switch (la_errno) {
		case ARCHIVE_EOF:
			return L"End of archive";
		case ARCHIVE_OK:
			return L"Operation was successful";
		case ARCHIVE_RETRY:
			return L"Retry might succeed";
		case ARCHIVE_WARN:
			return L"Warning: Partial success";
		case ARCHIVE_FAILED:
			return L"Failed: Current operation cannot complete";
		case ARCHIVE_FATAL:
			return L"Fatal Error: No more operations are possible";
		case EILSEQ/*ARCHIVE_ERRNO_FILE_FORMAT*/:	//<libarchive/archive_platform.h>
			return L"Unrecognized or invalid file format";
		case EINVAL/*ARCHIVE_ERRNO_PROGRAMMER*/:	//<libarchive/archive_platform.h>
			return L"Illegal usage of the library";
		case -1/*ARCHIVE_ERRNO_MISC*/:	//<libarchive/archive_platform.h>
			return L"Unknown or unclassified error";
		default:
			return L"Unknown error";
		}
	}
public:
	LA_EXCEPTION(int la_errno) :ARCHIVE_EXCEPTION(L"") {
		_errno = la_errno;
		auto msg = error_string(la_errno);
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
	{LF_ARCHIVE_FORMAT::ZIP, L".zip", true, {
		LF_WOPT_STANDARD,
		LF_WOPT_DATA_ENCRYPTION
		}, ARCHIVE_FORMAT_ZIP, },
	{LF_ARCHIVE_FORMAT::_7Z, L".7z", true, {
		LF_WOPT_STANDARD,
		//LF_WOPT_DATA_ENCRYPTION,
		//LF_WOPT_HEADER_ENCRYPTION,
		//LF_WOPT_DATA_ENCRYPTION | LF_WOPT_HEADER_ENCRYPTION,
		//LF_WOPT_SFX | LF_WOPT_DATA_ENCRYPTION
		}, ARCHIVE_FORMAT_7ZIP, },
	{LF_ARCHIVE_FORMAT::GZ, L"{ext}.gz", false, {LF_WOPT_STANDARD}, ARCHIVE_FORMAT_RAW | ARCHIVE_FILTER_GZIP, },
	{LF_ARCHIVE_FORMAT::BZ2, L"{ext}.bz2", false, {LF_WOPT_STANDARD}, ARCHIVE_FORMAT_RAW | ARCHIVE_FILTER_BZIP2, },
	{LF_ARCHIVE_FORMAT::LZMA, L"{ext}.lzma", false, {LF_WOPT_STANDARD}, ARCHIVE_FORMAT_RAW | ARCHIVE_FILTER_LZMA, },
	{LF_ARCHIVE_FORMAT::XZ, L"{ext}.xz", false, {LF_WOPT_STANDARD}, ARCHIVE_FORMAT_RAW | ARCHIVE_FILTER_XZ, },
	{LF_ARCHIVE_FORMAT::ZSTD, L"{ext}.zst", false, {LF_WOPT_STANDARD}, ARCHIVE_FORMAT_RAW | ARCHIVE_FILTER_ZSTD, },
	{LF_ARCHIVE_FORMAT::TAR, L".tar", true, {LF_WOPT_STANDARD}, ARCHIVE_FORMAT_TAR},
	{LF_ARCHIVE_FORMAT::TAR_GZ, L".tar.gz", true, {LF_WOPT_STANDARD}, ARCHIVE_FORMAT_TAR | ARCHIVE_FILTER_GZIP, },
	{LF_ARCHIVE_FORMAT::TAR_BZ2, L".tar.bz2", true, {LF_WOPT_STANDARD}, ARCHIVE_FORMAT_TAR | ARCHIVE_FILTER_BZIP2, },
	{LF_ARCHIVE_FORMAT::TAR_LZMA, L".tar.lzma", true, {LF_WOPT_STANDARD}, ARCHIVE_FORMAT_TAR | ARCHIVE_FILTER_LZMA, },
	{LF_ARCHIVE_FORMAT::TAR_XZ, L".tar.xz", true, {LF_WOPT_STANDARD}, ARCHIVE_FORMAT_TAR | ARCHIVE_FILTER_XZ, },
	{LF_ARCHIVE_FORMAT::TAR_ZSTD, L".tar.zst", true, {LF_WOPT_STANDARD}, ARCHIVE_FORMAT_TAR | ARCHIVE_FILTER_ZSTD, },
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
			throw LA_EXCEPTION(r);
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
		std::shared_ptr<ILFPassphrase> passphrase;
	}_rewind;

	LA_FILE_TO_READ() :_arc(nullptr) {}
	virtual ~LA_FILE_TO_READ() {
		close();
	}
	void open(const std::filesystem::path& arcpath, std::shared_ptr<ILFPassphrase> passphrase) {
		close();
		_rewind.arcpath = arcpath;
		_rewind.need_rewind = false;
		_rewind.passphrase = passphrase;
		_arc = archive_read_new();
		archive_read_support_filter_all(_arc);
		archive_read_support_format_all(_arc);

		int r = archive_read_set_passphrase_callback(_arc, passphrase.get(), LF_LA_passphrase);
		if (r < ARCHIVE_OK) {
			throw LA_EXCEPTION(r);
		}
		r = archive_read_open_filename_w(_arc, arcpath.c_str(), 10240);
		if (r < ARCHIVE_OK) {
			//retry enabling archive_read_support_format_raw
			close();
			if (check_format(arcpath) != LF_ARCHIVE_FORMAT::INVALID) {
				_arc = archive_read_new();
				r = archive_read_set_passphrase_callback(_arc, passphrase.get(), LF_LA_passphrase);
				if (r < ARCHIVE_OK) {
					throw LA_EXCEPTION(r);
				}
				archive_read_support_filter_all(_arc);
				archive_read_support_format_raw(_arc);
				r = archive_read_open_filename_w(_arc, arcpath.c_str(), 10240);
				if (r < ARCHIVE_OK) {
					throw LA_EXCEPTION(r);
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
				open(_rewind.arcpath, _rewind.passphrase);
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
			if (L"data" == _entry._lf_stat.path && archive_format(_arc) == ARCHIVE_FORMAT_RAW) {
				_entry._lf_stat.path = _rewind.arcpath.stem();
			}

			return &_entry;
		} else {
			return nullptr;
		}
	}

	operator struct archive*() { return _arc; }

	void read_block(std::function<void(const void*, size_t/*data size*/, const offset_info*/*offset*/)> data_receiver) {
		const void* buf;
		size_t size;
		la_int64_t offset = 0;
		int r = archive_read_data_block(_arc, &buf, &size, &offset);
		if (ARCHIVE_EOF == r) {
			data_receiver(nullptr, 0, 0);
		} else if (r < ARCHIVE_OK) {
			throw LA_EXCEPTION(r);
		} else {
			offset_info oi = { (uint64_t)offset };
			data_receiver(buf, size, &oi);
		}
	}
	static LF_ARCHIVE_FORMAT check_format(const std::filesystem::path &arcname) {
		try {
			if (std::filesystem::file_size(arcname) == 0)return LF_ARCHIVE_FORMAT::INVALID;
		} catch (...) {
			return LF_ARCHIVE_FORMAT::INVALID;
		}

		CAutoFile fp;
		fp.open(arcname);
		if (!fp.is_opened())return LF_ARCHIVE_FORMAT::INVALID;
		const size_t bufSize = 10;
		std::vector<unsigned char> header(bufSize);
		size_t read = fread(&header[0], 1, bufSize, fp);
		if (read < 1) {
			return LF_ARCHIVE_FORMAT::INVALID;
		}

		//check header for known format; does not care if its tar or not
		//gzip: RFC 1952
		if (read > 2 && 
			header[0] == 0x1f && header[1] == 0x8b) {
			return LF_ARCHIVE_FORMAT::GZ;
		}
		//bz2: https://github.com/dsnet/compress/blob/master/doc/bzip2-format.pdf
		if (read >= 10 && 
			header[0] == 'B' && header[1] == 'Z' && header[2] == 'h' &&
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
			return LF_ARCHIVE_FORMAT::BZ2;
		}
		//zstd: https://github.com/facebook/zstd/blob/dev/doc/zstd_compression_format.md
		if (read > 4 &&
			header[0] == 0x28 && header[1] == 0xB5 && header[2] == 0x2F && header[3] == 0xFD) {
			return LF_ARCHIVE_FORMAT::ZSTD;
		}

		//xz: xz-file-format.txt in XZ Utils[https://tukaani.org/xz/]
		if (read > 6 && 
			header[0] == 0xFD && header[1] == '7' && header[2] == 'z' &&
			header[3] == 'X' && header[4] == 'Z' && header[5] == 0x00) {
			return LF_ARCHIVE_FORMAT::XZ;
		}
		//lzma: lzma-file-format.txt in XZ Utils[https://tukaani.org/xz/]
		{
			uint8_t prop = header[0];
			if (prop <= (4 * 5 + 4) * 9 + 8) {
				return LF_ARCHIVE_FORMAT::LZMA;
			}
		}


		return LF_ARCHIVE_FORMAT::INVALID;
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
		std::shared_ptr<ILFPassphrase> passphrase) {
		const auto& cap = la_get_compression_capability(fmt);

		int la_filter = cap.mapped_libarchive_format & ~ARCHIVE_FORMAT_BASE_MASK;
		int la_fmt = cap.mapped_libarchive_format & ARCHIVE_FORMAT_BASE_MASK;

		write_open_la(arcname, la_fmt, { la_filter }, archive_options, passphrase);
	}
	void write_open_la(const std::wstring& arcname,
		int la_fmt,
		const std::vector<int> &filters,
		const std::map<std::string, std::string> &archive_options,
		std::shared_ptr<ILFPassphrase> passphrase) {
		close();
		_arc = archive_write_new();

		for (auto f : filters) {
			int r = archive_write_add_filter(_arc, f);
			if (r < ARCHIVE_OK) {
				throw LA_EXCEPTION(r);
			}
		}

		int r = archive_write_set_format(_arc, la_fmt);
		if (r < ARCHIVE_OK) {
			throw LA_EXCEPTION(r);
		}

		r = archive_write_set_passphrase_callback(_arc, passphrase.get(), LF_LA_passphrase);
		if (r < ARCHIVE_OK) {
			throw LA_EXCEPTION(r);
		}
		for (auto &ite : archive_options) {
			int r = archive_write_set_option(_arc, nullptr, ite.first.c_str(), ite.second.c_str());
			if (r < ARCHIVE_OK) {
				throw LA_EXCEPTION(r);
			}
		}
		r = archive_write_open_filename_w(_arc, arcname.c_str());
		if (r < ARCHIVE_OK) {
			throw LA_EXCEPTION(r);
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
			throw LA_EXCEPTION(r);
		}

		while (true) {
			LF_BUFFER_INFO ibi = dataProvider();
			if (ibi.size) {
				archive_write_data(_arc, ibi.buffer, (size_t)ibi.size);
			} else {
				break;
			}
		}
	}
	void add_directory(LF_LA_ENTRY& entry) {
		int r = archive_write_header(_arc, entry.la_entry());
		if (r < ARCHIVE_OK) {
			throw LA_EXCEPTION(r);
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

#ifdef UNIT_TEST
#include "CommonUtil.h"
TEST(CLFArchiveLA, mimic_archive_property)
{
	{
		auto fileToRead = LF_PROJECT_DIR() / L"test/test_extract.zip";
		LA_FILE_TO_READ src;
		auto pp = std::make_shared<CLFPassphraseNULL>();
		src.open(fileToRead, pp);
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
		auto pp = std::make_shared<CLFPassphraseNULL>();
		src.open(fileToRead, pp);
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
		auto pp = std::make_shared<CLFPassphraseNULL>();
		src.open(fileToRead, pp);
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
		//ZIP should be handled by minizip-ng
	{
		merge_map(params, args.formats.zip.params);
		if (encrypt) {
			if (params["encryption"] == "zipcrypto") {
				params["encryption"] = "ZipCrypt";
			}
		} else {
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

	if (!isIn(cap.allowed_combinations, options)) {
		throw ARCHIVE_EXCEPTION(EINVAL);
	}

	int la_format = cap.mapped_libarchive_format & ARCHIVE_FORMAT_BASE_MASK;
	std::vector<int> la_filters = { cap.mapped_libarchive_format & ~ARCHIVE_FORMAT_BASE_MASK };

	bool encrypt = (options & LF_WOPT_DATA_ENCRYPTION) != 0;
	return getLAOptionsFromConfig(la_format, la_filters, encrypt, args);
}

#ifdef UNIT_TEST
TEST(archive_libarchive, getLAOptionsFromConfig)
{
	LF_COMPRESS_ARGS fake_args;
	CConfigFile mngr;
	fake_args.load(mngr);
	{
		auto la_options = getLAOptionsFromConfig(fake_args, LF_ARCHIVE_FORMAT::ZIP, LF_WOPT_STANDARD);
		EXPECT_EQ(2, la_options.size());
		EXPECT_EQ("deflate", la_options.at("compression"));
		EXPECT_EQ("9", la_options.at("compression-level"));
		//EXPECT_EQ("ZipCrypt", la_options.at("encryption"));
		//EXPECT_EQ("UTF-8", la_options.at("hdrcharset"));
		//EXPECT_EQ("", la_options.at("zip64"));
	}
	{
		auto la_options = getLAOptionsFromConfig(fake_args, LF_ARCHIVE_FORMAT::ZIP, LF_WOPT_DATA_ENCRYPTION);
		EXPECT_EQ(3, la_options.size());
		EXPECT_EQ("deflate", la_options.at("compression"));
		EXPECT_EQ("9", la_options.at("compression-level"));
		EXPECT_EQ("ZipCrypt", la_options.at("encryption"));
		//EXPECT_EQ("UTF-8", la_options.at("hdrcharset"));
		//EXPECT_EQ("", la_options.at("zip64"));
	}
	{
		auto la_options = getLAOptionsFromConfig(fake_args, LF_ARCHIVE_FORMAT::_7Z, LF_WOPT_STANDARD);
		EXPECT_EQ(2, la_options.size());
		EXPECT_EQ("deflate", la_options.at("compression"));
		EXPECT_EQ("9", la_options.at("compression-level"));
	}

	{
		auto la_options = getLAOptionsFromConfig(fake_args, LF_ARCHIVE_FORMAT::TAR, LF_WOPT_STANDARD);
		EXPECT_EQ(1, la_options.size());
		EXPECT_EQ("UTF-8", la_options.at("hdrcharset"));
	}

	{
		auto la_options = getLAOptionsFromConfig(fake_args, LF_ARCHIVE_FORMAT::GZ, LF_WOPT_STANDARD);
		EXPECT_EQ(1, la_options.size());
		EXPECT_EQ("9", la_options.at("compression-level"));
	}

	{
		auto la_options = getLAOptionsFromConfig(fake_args, LF_ARCHIVE_FORMAT::BZ2, LF_WOPT_STANDARD);
		EXPECT_EQ(1, la_options.size());
		EXPECT_EQ("9", la_options.at("compression-level"));
	}

	{
		auto la_options = getLAOptionsFromConfig(fake_args, LF_ARCHIVE_FORMAT::XZ, LF_WOPT_STANDARD);
		EXPECT_EQ(2, la_options.size());
		EXPECT_EQ("9", la_options.at("compression-level"));
		EXPECT_EQ("0", la_options.at("threads"));
	}

	{
		auto la_options = getLAOptionsFromConfig(fake_args, LF_ARCHIVE_FORMAT::LZMA, LF_WOPT_STANDARD);
		EXPECT_EQ(1, la_options.size());
		EXPECT_EQ("9", la_options.at("compression-level"));
	}

	{
		auto la_options = getLAOptionsFromConfig(fake_args, LF_ARCHIVE_FORMAT::ZSTD, LF_WOPT_STANDARD);
		EXPECT_EQ(1, la_options.size());
		EXPECT_EQ("3", la_options.at("compression-level"));
	}
}
#endif

//------

CLFArchiveLA::CLFArchiveLA() {}
CLFArchiveLA::~CLFArchiveLA() {}

void CLFArchiveLA::read_open(const std::filesystem::path& file, std::shared_ptr<ILFPassphrase> passhprase)
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
	std::shared_ptr<ILFPassphrase> passphrase)
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

bool CLFArchiveLA::is_modify_supported()const
{
	if (!_arc_read)return false;
	try {
		auto[la_format, filters, is_encrypted] = LA_FILE_TO_WRITE::mimic_archive_property(*_arc_read);
		if ((la_format & ARCHIVE_FORMAT_BASE_MASK) == ARCHIVE_FORMAT_TAR)return true;
		if ((la_format & ARCHIVE_FORMAT_BASE_MASK) == ARCHIVE_FORMAT_RAW)return false;
		for (const auto &c : g_la_capabilities) {
			if ((c.mapped_libarchive_format & ARCHIVE_FORMAT_BASE_MASK) == la_format) {
				if (!c.contains_multiple_files) {
					return false;
				}
				return true;
			}
		}
		return false;
	} catch(...){
		return false;
	}
}

#ifdef UNIT_TEST

TEST(CLFArchiveLA, is_modify_supported)
{
	const auto dir = LF_PROJECT_DIR();
	auto check=[](const std::filesystem::path &p)->bool {
		CLFArchiveLA a;
		auto pp = std::make_shared<CLFPassphraseNULL>();
		a.read_open(p, pp);
		return a.is_modify_supported();
	};
	EXPECT_FALSE(check(dir / L"ArchiverCode/test/empty.gz"));
	EXPECT_FALSE(check(dir / L"ArchiverCode/test/empty.bz2"));
	EXPECT_FALSE(check(dir / L"ArchiverCode/test/empty.xz"));
	EXPECT_FALSE(check(dir / L"ArchiverCode/test/empty.lzma"));
	EXPECT_FALSE(check(dir / L"ArchiverCode/test/empty.zst"));

	EXPECT_FALSE(check(dir / L"ArchiverCode/test/abcde.gz"));
	EXPECT_FALSE(check(dir / L"ArchiverCode/test/abcde.bz2"));
	EXPECT_FALSE(check(dir / L"ArchiverCode/test/abcde.xz"));
	EXPECT_FALSE(check(dir / L"ArchiverCode/test/abcde.lzma"));
	EXPECT_FALSE(check(dir / L"ArchiverCode/test/abcde.zst"));

	//EXPECT_FALSE(check(__FILEW__));
	//EXPECT_FALSE(check(L"some_non_existing_file"));

	EXPECT_TRUE(check(dir / L"test/test_extract.zip"));
	EXPECT_TRUE(check(dir / L"test/test_extract.zipx"));
	EXPECT_FALSE(check(dir / L"test/test.lzh"));
	EXPECT_TRUE(check(dir / L"test/test.tar.gz"));
}

#endif

LF_ARCHIVE_FORMAT CLFArchiveLA::get_format()
{
	if (_arc_read) {
		//scan for file content; to know archive information
		for (auto entry = read_entry_begin(); entry; entry = read_entry_next()) {
			continue;
		}
		int format = archive_format(*_arc_read);
		switch (format & ARCHIVE_FORMAT_BASE_MASK) {
		case ARCHIVE_FORMAT_ZIP:
			return LF_ARCHIVE_FORMAT::ZIP;
		case ARCHIVE_FORMAT_7ZIP:
			return LF_ARCHIVE_FORMAT::_7Z;
		case ARCHIVE_FORMAT_RAW:
			return LA_FILE_TO_READ::check_format(_arc_read.get()->_rewind.arcpath);
		case ARCHIVE_FORMAT_TAR:
			switch (LA_FILE_TO_READ::check_format(_arc_read.get()->_rewind.arcpath)) {
			case LF_ARCHIVE_FORMAT::GZ:
				return LF_ARCHIVE_FORMAT::TAR_GZ;
			case LF_ARCHIVE_FORMAT::BZ2:
				return LF_ARCHIVE_FORMAT::TAR_BZ2;
			case LF_ARCHIVE_FORMAT::LZMA:
				return LF_ARCHIVE_FORMAT::TAR_LZMA;
			case LF_ARCHIVE_FORMAT::XZ:
				return LF_ARCHIVE_FORMAT::TAR_XZ;
			case LF_ARCHIVE_FORMAT::ZSTD:
				return LF_ARCHIVE_FORMAT::TAR_ZSTD;
			case LF_ARCHIVE_FORMAT::INVALID:
				return LF_ARCHIVE_FORMAT::INVALID;
			default:
				return LF_ARCHIVE_FORMAT::TAR;
			}
			break;
		}
	}else if (_arc_write) {
		int format = archive_format(*_arc_write);
		switch (format & ARCHIVE_FORMAT_BASE_MASK) {
		case ARCHIVE_FORMAT_ZIP:
			return LF_ARCHIVE_FORMAT::ZIP;
		case ARCHIVE_FORMAT_7ZIP:
			return LF_ARCHIVE_FORMAT::_7Z;
		case ARCHIVE_FORMAT_RAW:
			switch (format & ~ARCHIVE_FORMAT_BASE_MASK) {
			case ARCHIVE_FILTER_NONE:
			case ARCHIVE_FILTER_GZIP:
				return LF_ARCHIVE_FORMAT::GZ;
			case ARCHIVE_FILTER_BZIP2:
				return LF_ARCHIVE_FORMAT::BZ2;
			case ARCHIVE_FILTER_LZMA:
				return LF_ARCHIVE_FORMAT::LZMA;
			case ARCHIVE_FILTER_XZ:
				return LF_ARCHIVE_FORMAT::XZ;
			case ARCHIVE_FILTER_ZSTD:
				return LF_ARCHIVE_FORMAT::ZSTD;
			}
			break;
		case ARCHIVE_FORMAT_TAR:
			switch (format & ~ARCHIVE_FORMAT_BASE_MASK) {
			case ARCHIVE_FILTER_GZIP:
				return LF_ARCHIVE_FORMAT::TAR_GZ;
			case ARCHIVE_FILTER_BZIP2:
				return LF_ARCHIVE_FORMAT::TAR_BZ2;
			case ARCHIVE_FILTER_LZMA:
				return LF_ARCHIVE_FORMAT::TAR_LZMA;
			case ARCHIVE_FILTER_XZ:
				return LF_ARCHIVE_FORMAT::TAR_XZ;
			case ARCHIVE_FILTER_ZSTD:
				return LF_ARCHIVE_FORMAT::TAR_ZSTD;
			case ARCHIVE_FILTER_NONE:
			default:
				return LF_ARCHIVE_FORMAT::TAR;
			}
			break;
		}
	}
	//fallback
	return LF_ARCHIVE_FORMAT::READONLY;
}

#ifdef UNIT_TEST
TEST(CLFArchiveLA, get_format)
{
	auto pp = std::make_shared<CLFPassphraseNULL>();
	CLFArchiveLA a;

	a.read_open(LF_PROJECT_DIR() / L"test/test_extract.zip", pp);
	EXPECT_EQ(LF_ARCHIVE_FORMAT::ZIP, a.get_format());

	a.read_open(LF_PROJECT_DIR() / L"test/test.lzh", pp);
	EXPECT_EQ(LF_ARCHIVE_FORMAT::READONLY, a.get_format());

	a.read_open(LF_PROJECT_DIR() / L"test/test_gzip.gz", pp);
	EXPECT_EQ(LF_ARCHIVE_FORMAT::GZ, a.get_format());

	a.read_open(LF_PROJECT_DIR() / L"test/test.tar.gz", pp);
	EXPECT_EQ(LF_ARCHIVE_FORMAT::TAR_GZ, a.get_format());

	a.read_open(LF_PROJECT_DIR() / L"ArchiverCode/test/multistream.txt.bz2", pp);
	EXPECT_EQ(LF_ARCHIVE_FORMAT::BZ2, a.get_format());

	a.read_open(LF_PROJECT_DIR() / L"ArchiverCode/test/abcde.lzma", pp);
	EXPECT_EQ(LF_ARCHIVE_FORMAT::LZMA, a.get_format());

	a.read_open(LF_PROJECT_DIR() / L"ArchiverCode/test/abcde.xz", pp);
	EXPECT_EQ(LF_ARCHIVE_FORMAT::XZ, a.get_format());

	a.read_open(LF_PROJECT_DIR() / L"ArchiverCode/test/abcde.zst", pp);
	EXPECT_EQ(LF_ARCHIVE_FORMAT::ZSTD, a.get_format());

	a.read_open(LF_PROJECT_DIR() / L"ArchiverCode/test/test_2099.tar.zst", pp);
	EXPECT_EQ(LF_ARCHIVE_FORMAT::TAR_ZSTD, a.get_format());
}

#endif

//archive property
std::wstring CLFArchiveLA::get_format_name()
{
	if (_arc_read) {
		//scan for file content; to know archive information
		for (auto entry = read_entry_begin(); entry; entry = read_entry_next()) {
			continue;
		}
		auto p = archive_format_name(*_arc_read);
		if (p)return UtilUTF8toUNICODE(p);
	}
	if (_arc_write) {
		auto p = archive_format_name(*_arc_write);
		if (p)return UtilUTF8toUNICODE(p);
	}
	return L"---";
}

#ifdef UNIT_TEST

TEST(CLFArchiveLA, get_format_name)
{
	auto temp = UtilGetTemporaryFileName();
	{
		CLFArchiveLA a;
		auto pp = std::make_shared<CLFPassphraseNULL>();
		a.read_open(LF_PROJECT_DIR() / L"test/test_extract.zip", pp);
		EXPECT_EQ(L"ZIP 1.0 (uncompressed)", a.get_format_name());

		LF_COMPRESS_ARGS args;
		args.load(CConfigFile());
		a.write_open(temp, LF_ARCHIVE_FORMAT::ZIP, LF_WOPT_STANDARD, args, pp);
		EXPECT_EQ(L"ZIP 1.0 (uncompressed)", a.get_format_name());
	}
	UtilDeletePath(temp);
	EXPECT_FALSE(std::filesystem::exists(temp));
}

#endif

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


#ifdef UNIT_TEST

TEST(CLFArchiveLA, read_entry)
{
	_wsetlocale(LC_ALL, L"");	//default locale
	{
		CLFArchiveLA a;
		auto pp = std::make_shared<CLFPassphraseNULL>();
		a.read_open(LF_PROJECT_DIR() / L"test/test_extract.zip", pp);
		auto entry = a.read_entry_begin();
		EXPECT_NE(nullptr, entry);
		EXPECT_EQ(L"dirA/dirB/", entry->path);

		entry = a.read_entry_next();
		EXPECT_NE(nullptr, entry);
		EXPECT_EQ(L"dirA/dirB/dirC/", entry->path);

		entry = a.read_entry_next();
		EXPECT_NE(nullptr, entry);
		EXPECT_EQ(L"dirA/dirB/dirC/file1.txt", entry->path);

		entry = a.read_entry_next();
		EXPECT_NE(nullptr, entry);
		EXPECT_EQ(L"dirA/dirB/file2.txt", entry->path);

		entry = a.read_entry_next();
		EXPECT_NE(nullptr, entry);
		EXPECT_EQ(L"あいうえお.txt", entry->path);

		entry = a.read_entry_next();
		EXPECT_NE(nullptr, entry);
		EXPECT_EQ(L"かきくけこ/file3.txt", entry->path);

		entry = a.read_entry_next();
		EXPECT_EQ(nullptr, entry);
	}

	{
		CLFArchiveLA a;
		auto pp = std::make_shared<CLFPassphraseNULL>();
		a.read_open(LF_PROJECT_DIR() / L"test/test_extract.zipx", pp);
		auto entry = a.read_entry_begin();
		EXPECT_NE(nullptr, entry);
		EXPECT_EQ(L"dirA/", entry->path);

		entry = a.read_entry_next();
		EXPECT_NE(nullptr, entry);
		EXPECT_EQ(L"dirA/dirB/", entry->path);

		entry = a.read_entry_next();
		EXPECT_NE(nullptr, entry);
		EXPECT_EQ(L"dirA/dirB/dirC/", entry->path);

		entry = a.read_entry_next();
		EXPECT_NE(nullptr, entry);
		EXPECT_EQ(L"dirA/dirB/dirC/file1.txt", entry->path);

		entry = a.read_entry_next();
		EXPECT_NE(nullptr, entry);
		EXPECT_EQ(L"dirA/dirB/file2.txt", entry->path);

		entry = a.read_entry_next();
		EXPECT_NE(nullptr, entry);
		EXPECT_EQ(L"あいうえお.txt", entry->path);

		entry = a.read_entry_next();
		EXPECT_NE(nullptr, entry);
		EXPECT_EQ(L"かきくけこ/", entry->path);

		entry = a.read_entry_next();
		EXPECT_NE(nullptr, entry);
		EXPECT_EQ(L"かきくけこ/file3.txt", entry->path);

		entry = a.read_entry_next();
		EXPECT_EQ(nullptr, entry);
	}
}

#endif

//read file entry
void CLFArchiveLA::read_file_entry_block(std::function<void(const void*, size_t/*data size*/, const offset_info*/*offset*/)> data_receiver)
{
	if (_arc_read) {
		_arc_read->read_block(data_receiver);
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


#ifdef UNIT_TEST

TEST(CLFArchiveLA, add_entry)
{
	auto temp = UtilGetTemporaryFileName();
	auto src = UtilGetTemporaryFileName();
	{
		CAutoFile f;
		f.open(src, L"w");
		fputs("abcde12345", f);
	}
	{
		CLFArchiveLA a;
		LF_COMPRESS_ARGS args;
		args.load(CConfigFile());
		auto pp = std::make_shared<CLFPassphraseNULL>();
		a.write_open(temp, LF_ARCHIVE_FORMAT::ZIP, LF_WOPT_STANDARD, args, pp);
		LF_ENTRY_STAT e;
		e.read_stat(LF_PROJECT_DIR(), L"test/");	//LF_PROJECT_DIR() as a directory template
		a.add_directory_entry(e);

		RAW_FILE_READER provider;
		provider.open(src);
		e.read_stat(src, L"test/file.txt");
		a.add_file_entry(e, [&]() {
			auto data = provider();
			return data;
		});
	}
	{
		CLFArchiveLA a;
		auto pp = std::make_shared<CLFPassphraseNULL>();
		a.read_open(temp, pp);
		auto entry = a.read_entry_begin();
		EXPECT_NE(nullptr, entry);
		EXPECT_EQ(L"test/", entry->path);

		entry = a.read_entry_next();
		EXPECT_NE(nullptr, entry);
		EXPECT_EQ(L"test/file.txt", entry->path);
		EXPECT_EQ(10, entry->stat.st_size);
	}
	UtilDeletePath(temp);
	EXPECT_FALSE(std::filesystem::exists(temp));
	UtilDeletePath(src);
	EXPECT_FALSE(std::filesystem::exists(src));
}

#endif

//make a copy, and returns in "write_open" state
std::unique_ptr<ILFArchiveFile> CLFArchiveLA::make_copy_archive(
	const std::filesystem::path& dest_path,
	const LF_COMPRESS_ARGS& args,
	std::function<bool(const LF_ENTRY_STAT&)> false_to_skip)
{
	if (_arc_read) {
		ASSERT(_arc_read->_rewind.passphrase);
		auto[la_format, filters, is_encrypted] = LA_FILE_TO_WRITE::mimic_archive_property(*_arc_read);
		std::unique_ptr<CLFArchiveLA> dest_archive = std::make_unique<CLFArchiveLA>();
		dest_archive->_arc_write = std::make_unique<LA_FILE_TO_WRITE>();

		//- open an output archive in most similar option
		auto options = getLAOptionsFromConfig(la_format, filters, is_encrypted, args);
		dest_archive->_arc_write->write_open_la(dest_path, la_format, filters, options, _arc_read->_rewind.passphrase);

		//- then, copy entries if filter returns true
		//this would need overhead of extract on read and compress on write
		//there seems no way to get raw data
		_arc_read->rewind();
		for (LF_LA_ENTRY* entry = _arc_read->begin(); entry; entry = _arc_read->next()) {
			if (false_to_skip(entry->_lf_stat)) {
				if (entry->_lf_stat.is_directory()) {
					dest_archive->_arc_write->add_directory(*entry);
				} else {
					dest_archive->_arc_write->add_entry(*entry, [&]() {
						while (UtilDoMessageLoop())continue;	//TODO
						//TODO progress handler
						LF_BUFFER_INFO bi;
						_arc_read->read_block([&](const void* buf, int64_t size, const offset_info* offset) {
							bi.buffer = buf;
							bi.size = (size_t)size;
							bi.offset = offset;
						});
						return bi;
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


#ifdef UNIT_TEST

TEST(CLFArchiveLA, make_copy_archive)
{
	_wsetlocale(LC_ALL, L"");	//default locale
	{
		auto temp = UtilGetTemporaryFileName();
		{
			CLFArchiveLA a;
			auto pp = std::make_shared<CLFPassphraseNULL>();
			a.read_open(LF_PROJECT_DIR() / L"test/test_extract.zip", pp);
			{
				LF_COMPRESS_ARGS args;
				args.load(CConfigFile());
				auto out = a.make_copy_archive(temp, args, [](const LF_ENTRY_STAT&) {return true; });
			}
			a.read_open(temp, pp);

			auto entry = a.read_entry_begin();
			EXPECT_NE(nullptr, entry);
			EXPECT_EQ(L"dirA/dirB/", entry->path);

			entry = a.read_entry_next();
			EXPECT_NE(nullptr, entry);
			EXPECT_EQ(L"dirA/dirB/dirC/", entry->path);

			entry = a.read_entry_next();
			EXPECT_NE(nullptr, entry);
			EXPECT_EQ(L"dirA/dirB/dirC/file1.txt", entry->path);

			entry = a.read_entry_next();
			EXPECT_NE(nullptr, entry);
			EXPECT_EQ(L"dirA/dirB/file2.txt", entry->path);

			entry = a.read_entry_next();
			EXPECT_NE(nullptr, entry);
			EXPECT_EQ(L"あいうえお.txt", entry->path);

			entry = a.read_entry_next();
			EXPECT_NE(nullptr, entry);
			EXPECT_EQ(L"かきくけこ/file3.txt", entry->path);

			entry = a.read_entry_next();
			EXPECT_EQ(nullptr, entry);
		}
		UtilDeletePath(temp);
		EXPECT_FALSE(std::filesystem::exists(temp));
	}
	{
		auto temp = UtilGetTemporaryFileName();
		{
			CLFArchiveLA a;
			auto passphrase = std::make_shared<CLFPassphraseConst>(L"abcde");
			a.read_open(LF_PROJECT_DIR() / L"test/test_password_abcde.zip", passphrase);
			{
				LF_COMPRESS_ARGS args;
				args.load(CConfigFile());
				auto out = a.make_copy_archive(temp, args, [](const LF_ENTRY_STAT&) {return true; });
			}
			a.read_open(temp, passphrase);

			auto entry = a.read_entry_begin();
			EXPECT_NE(nullptr, entry);
			EXPECT_EQ(L"test.txt", entry->path);

			entry = a.read_entry_next();
			EXPECT_EQ(nullptr, entry);
		}
		UtilDeletePath(temp);
		EXPECT_FALSE(std::filesystem::exists(temp));
	}
}

#endif

#include "CommonUtil.h"

bool CLFArchiveLA::is_known_format(const std::filesystem::path &arcname)
{
	return LA_FILE_TO_READ::check_format(arcname) != LF_ARCHIVE_FORMAT::INVALID;
}


#ifdef UNIT_TEST

TEST(CLFArchiveLA, is_known_format)
{
	const auto dir = LF_PROJECT_DIR() / L"ArchiverCode/test";
	EXPECT_TRUE(CLFArchiveLA::is_known_format(dir / L"empty.gz"));
	EXPECT_TRUE(CLFArchiveLA::is_known_format(dir / L"empty.bz2"));
	EXPECT_TRUE(CLFArchiveLA::is_known_format(dir / L"empty.xz"));
	EXPECT_TRUE(CLFArchiveLA::is_known_format(dir / L"empty.lzma"));
	EXPECT_TRUE(CLFArchiveLA::is_known_format(dir / L"empty.zst"));

	EXPECT_TRUE(CLFArchiveLA::is_known_format(dir / L"abcde.gz"));
	EXPECT_TRUE(CLFArchiveLA::is_known_format(dir / L"abcde.bz2"));
	EXPECT_TRUE(CLFArchiveLA::is_known_format(dir / L"abcde.xz"));
	EXPECT_TRUE(CLFArchiveLA::is_known_format(dir / L"abcde.lzma"));
	EXPECT_TRUE(CLFArchiveLA::is_known_format(dir / L"abcde.zst"));

	EXPECT_FALSE(CLFArchiveLA::is_known_format(__FILEW__));
	EXPECT_FALSE(CLFArchiveLA::is_known_format(L"some_non_existing_file"));
}

TEST(CLFArchiveLA, read_enum_2099_zstd)
{
	_wsetlocale(LC_ALL, L"");	//default locale
	const auto file = std::filesystem::path(__FILEW__).parent_path() / L"test/test_2099.tar.zst";

	CLFArchiveLA a;
	auto pp = std::make_shared<CLFPassphraseNULL>();
	a.read_open(file, pp);
	EXPECT_TRUE(a.is_modify_supported());
	ASSERT_EQ(L"POSIX ustar format", a.get_format_name());


	int count = 0;
	int numDir = 0;
	for (auto entry = a.read_entry_begin(); entry; entry = a.read_entry_next()) {
		count++;
		if (entry->is_directory()) {
			numDir++;
		} else {
			if (entry->path.wstring().find(L"ccd.txt") != -1) {
				EXPECT_EQ(entry->stat.st_size, 44);
				EXPECT_EQ(entry->method_name, L"---");
				EXPECT_EQ(entry->compressed_size, -1);
				std::vector<char> data;
				for (;;) {
					bool bEOF = false;
					a.read_file_entry_block([&](const void* buf, size_t data_size, const offset_info* offset) {
						//EXPECT_EQ(nullptr, offset);
						if (buf) {
							data.insert(data.end(), (const char*)buf, ((const char*)buf) + data_size);
						} else {
							bEOF = true;
						}
					});
					if (bEOF) {
						break;
					}
				}
				EXPECT_EQ(data.size(), entry->stat.st_size);
				EXPECT_EQ(std::string(data.begin(), data.end()), ";kljd;lfj;lsdahg;has:hn:h :ahsd:fh:asdhg:ioh");
			} else {
				EXPECT_EQ(entry->stat.st_size, 48);
				EXPECT_EQ(entry->method_name, L"---");
				EXPECT_EQ(entry->compressed_size, -1);
			}
		}
		EXPECT_FALSE(entry->is_encrypted);
	}
	EXPECT_EQ(count, 2099 + 1);
	EXPECT_EQ(numDir, 1);
}

TEST(CLFArchiveLA, name_in_zstd)
{
	_wsetlocale(LC_ALL, L"");	//default locale
	const auto dir = LF_PROJECT_DIR() / L"ArchiverCode/test";
	auto file = dir / L"abcde.zst";
	EXPECT_TRUE(CLFArchiveLA::is_known_format(file));

	CLFArchiveLA a;
	auto pp = std::make_shared<CLFPassphraseNULL>();
	a.read_open(file, pp);
	int count = 0;
	for (auto entry = a.read_entry_begin(); entry; entry = a.read_entry_next()) {
		count++;
		EXPECT_FALSE(entry->is_directory());
		EXPECT_EQ(L"abcde", entry->path.wstring());	//this will be "data" in libarchive
		EXPECT_EQ(entry->stat.st_size, 0);	//cannot get file size
		EXPECT_EQ(entry->method_name, L"---");
		EXPECT_EQ(entry->compressed_size, -1);
		EXPECT_FALSE(entry->is_encrypted);
	}
	EXPECT_EQ(1, count);
}

TEST(CLFArchiveLA, name_in_gzip)
{
	_wsetlocale(LC_ALL, L"");	//default locale
	const auto dir = LF_PROJECT_DIR() / L"ArchiverCode/test";
	auto file = dir / L"abcde.gz";
	EXPECT_TRUE(CLFArchiveLA::is_known_format(file));

	CLFArchiveLA a;
	auto pp = std::make_shared<CLFPassphraseNULL>();
	a.read_open(file, pp);
	int count = 0;
	for (auto entry = a.read_entry_begin(); entry; entry = a.read_entry_next()) {
		count++;
		EXPECT_FALSE(entry->is_directory());
		EXPECT_EQ(L"abcde", entry->path.wstring());	//this will be "data" in libarchive
		EXPECT_EQ(entry->stat.st_size, 0);	//cannot get file size
		EXPECT_EQ(entry->method_name, L"---");
		EXPECT_EQ(entry->compressed_size, -1);
		EXPECT_FALSE(entry->is_encrypted);
	}
	EXPECT_EQ(1, count);
}

TEST(CLFArchiveLA, name_in_xz)
{
	_wsetlocale(LC_ALL, L"");	//default locale
	const auto dir = LF_PROJECT_DIR() / L"ArchiverCode/test";
	auto file = dir / L"abcde.xz";
	EXPECT_TRUE(CLFArchiveLA::is_known_format(file));

	CLFArchiveLA a;
	auto pp = std::make_shared<CLFPassphraseNULL>();
	a.read_open(file, pp);
	int count = 0;
	for (auto entry = a.read_entry_begin(); entry; entry = a.read_entry_next()) {
		count++;
		EXPECT_FALSE(entry->is_directory());
		EXPECT_EQ(L"abcde", entry->path.wstring());	//this will be "data" in libarchive
		EXPECT_EQ(entry->stat.st_size, 0);	//cannot get file size
		EXPECT_EQ(entry->method_name, L"---");
		EXPECT_EQ(entry->compressed_size, -1);
		EXPECT_FALSE(entry->is_encrypted);
	}
	EXPECT_EQ(1, count);
}

TEST(CLFArchiveLA, name_in_lzma)
{
	_wsetlocale(LC_ALL, L"");	//default locale
	const auto dir = LF_PROJECT_DIR() / L"ArchiverCode/test";
	auto file = dir / L"abcde.lzma";
	EXPECT_TRUE(CLFArchiveLA::is_known_format(file));

	CLFArchiveLA a;
	auto pp = std::make_shared<CLFPassphraseNULL>();
	a.read_open(file, pp);
	int count = 0;
	for (auto entry = a.read_entry_begin(); entry; entry = a.read_entry_next()) {
		count++;
		EXPECT_FALSE(entry->is_directory());
		EXPECT_EQ(L"abcde", entry->path.wstring());	//this will be "data" in libarchive
		EXPECT_EQ(entry->stat.st_size, 0);	//cannot get file size
		EXPECT_EQ(entry->method_name, L"---");
		EXPECT_EQ(entry->compressed_size, -1);
		EXPECT_FALSE(entry->is_encrypted);
	}
	EXPECT_EQ(1, count);
}

TEST(CLFArchiveLA, name_in_bz2)
{
	_wsetlocale(LC_ALL, L"");	//default locale
	const auto dir = LF_PROJECT_DIR() / L"ArchiverCode/test";
	auto file = dir / L"abcde.bz2";
	EXPECT_TRUE(CLFArchiveLA::is_known_format(file));

	CLFArchiveLA a;
	auto pp = std::make_shared<CLFPassphraseNULL>();
	a.read_open(file, pp);
	int count = 0;
	for (auto entry = a.read_entry_begin(); entry; entry = a.read_entry_next()) {
		count++;
		EXPECT_FALSE(entry->is_directory());
		EXPECT_EQ(L"abcde", entry->path.wstring());	//this will be "data" in libarchive
		EXPECT_EQ(entry->stat.st_size, 0);	//cannot get file size
		EXPECT_EQ(entry->method_name, L"---");
		EXPECT_EQ(entry->compressed_size, -1);
		EXPECT_FALSE(entry->is_encrypted);
	}
	EXPECT_EQ(1, count);
}

#endif
