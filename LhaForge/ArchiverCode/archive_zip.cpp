#include "stdafx.h"
#include "archive_zip.h"
#include "zip.h"
#include "mz_zip.h"
#include "mz_strm_os.h"
#include "mz_os.h"
#include "compress.h"
//#include "mz_crypt.h"

static std::wstring mzError2Text(int code)
{
	switch (code){
		case MZ_OK: return L"OK";
		case MZ_STREAM_ERROR:return L"Stream error(zlib)";
		case MZ_DATA_ERROR:return L"Data error(zlib)";
		case MZ_MEM_ERROR:return L"Memory allocation error(zlib)";
		case MZ_BUF_ERROR:return L"Buffer error(zlib)";
		case MZ_VERSION_ERROR:return L"Version error(zlib)";
		case MZ_END_OF_LIST:return L"End of list error";
		case MZ_END_OF_STREAM:return L"End of stream error";
		case MZ_PARAM_ERROR:return L"Invalid parameter error";
		case MZ_FORMAT_ERROR:return L"File format error";
		case MZ_INTERNAL_ERROR:return L"Library internal error";
		case MZ_CRC_ERROR:return L"CRC error";
		case MZ_CRYPT_ERROR:return L"Cryptography error";
		case MZ_EXIST_ERROR:return L"Does not exist";
		case MZ_PASSWORD_ERROR:return L"Invalid password";
		case MZ_SUPPORT_ERROR:return L"Library support error";
		case MZ_HASH_ERROR:return L"Hash error";
		case MZ_OPEN_ERROR:return L"Stream open error";
		case MZ_CLOSE_ERROR:return L"Stream close error";
		case MZ_SEEK_ERROR:return L"Stream seek error";
		case MZ_TELL_ERROR:return L"Stream tell error";
		case MZ_READ_ERROR:return L"Stream read error";
		case MZ_WRITE_ERROR:return L"Stream write error";
		case MZ_SIGN_ERROR:return L"Signing error";
		case MZ_SYMLINK_ERROR:return L"Symbolic link error";
		default:return L"Unknown error";
	}
}

static std::wstring mzMethodName(int method)
{
	switch (method) {
	case MZ_COMPRESS_METHOD_STORE: return L"Store";
	case MZ_COMPRESS_METHOD_DEFLATE: return L"Deflate";
	case MZ_COMPRESS_METHOD_BZIP2: return L"Bzip2";
	case MZ_COMPRESS_METHOD_LZMA: return L"LZMA1";
	case MZ_COMPRESS_METHOD_ZSTD: return L"ZSTD";
	case MZ_COMPRESS_METHOD_XZ: return L"XZ";
	default:return L"Unknown";
	}
}

struct CLFArchiveZIP::INTERNAL {
	INTERNAL(ILFPassphrase& pcb):zip(nullptr), stream(nullptr),passphrase_callback(pcb), flag(0){}
	virtual ~INTERNAL() { close(); }

	void* zip;		//mz_zip
	void* stream;	//mz_stream
	int flag;
	int method;
	int level;
	int aesFlag;
	std::shared_ptr<std::string> passphrase;	//UTF-8
	ILFPassphrase& passphrase_callback;
	void close() {
		if (zip) {
			mz_zip_close(zip);
			mz_zip_delete(&zip);
			zip = nullptr;
		}
		if (stream) {
			mz_stream_os_close(stream);
			mz_stream_os_delete(&stream);
			stream = nullptr;
		}
		flag = 0;
	}
	void open(std::filesystem::path path, int32_t mode, std::map<std::string, std::string> param) {
		close();
		mz_stream_os_create(&stream);
		mz_stream_os_open(stream, path.u8string().c_str(), mode);

		mz_zip_create(&zip);
		auto err = mz_zip_open(zip, stream, mode);
		if (err != MZ_OK) {
			RAISE_EXCEPTION(mzError2Text(err));
		}
		{
			auto methodStr = toLower(param["method"]);
			if (methodStr.empty()) {
				methodStr = "deflate";
			}
			std::map<std::string, int> methodMap = {
				{"store", MZ_COMPRESS_METHOD_STORE},
				{"deflate", MZ_COMPRESS_METHOD_DEFLATE},
				{"bzip2", MZ_COMPRESS_METHOD_BZIP2},
				{"lzma", MZ_COMPRESS_METHOD_LZMA},
				{"zstd", MZ_COMPRESS_METHOD_ZSTD},
				{"xz", MZ_COMPRESS_METHOD_XZ},
			};
			auto iter = methodMap.find(methodStr);
			if (methodMap.end() == iter) {
				RAISE_EXCEPTION(L"Invalid method name: %s", UtilUTF8toUNICODE(methodStr).c_str());
			} else {
				method = (*iter).second;
			}
		}
		{
			if (param["level"].empty()) {
				param["level"] = "6";	//default
			}
			level = atoi(param["level"].c_str());
			if (level < 0 || level>9) {
				RAISE_EXCEPTION(L"Invalid compression level: %s", UtilUTF8toUNICODE(param["level"]).c_str());
			}
		}
		{
			std::map<std::string, int> cryptoMap = {
			{"aes256", MZ_AES_ENCRYPTION_MODE_256},
			{"aes192", MZ_AES_ENCRYPTION_MODE_192},
			{"aes128", MZ_AES_ENCRYPTION_MODE_128},
		};
			auto cryptoStr = toLower(param["crypto"]);
			if (cryptoStr.empty()) {
				cryptoStr = "aes256";
			}
			auto iter = cryptoMap.find(cryptoStr);
			if (cryptoMap.end() == iter) {
				RAISE_EXCEPTION(L"Invalid crypto name: %s", UtilUTF8toUNICODE(cryptoStr).c_str());
			} else {
				aesFlag = (*iter).second;
			}
		}
	}
	bool isOpened()const {
		return zip != nullptr;
	}
	void update_passphrase() {
		const char* p = passphrase_callback();
		if (p) {
			passphrase = std::make_shared<std::string>(p);
		} else {
			passphrase.reset();
		}
	}
};

CLFArchiveZIP::CLFArchiveZIP():_internal(nullptr)
{
}

CLFArchiveZIP::~CLFArchiveZIP()
{
	close();
}

void CLFArchiveZIP::read_open(const std::filesystem::path& file, ILFPassphrase& passphrase)
{
	close();
	_internal = new INTERNAL(passphrase);
	LF_COMPRESS_ARGS fake_args;
	fake_args.load(CConfigFile());
	_internal->open(file, MZ_OPEN_MODE_READ | MZ_OPEN_MODE_EXISTING, fake_args.formats.zip.params);
}

void CLFArchiveZIP::write_open(const std::filesystem::path& file, LF_ARCHIVE_FORMAT format, LF_WRITE_OPTIONS options, const LF_COMPRESS_ARGS& args, ILFPassphrase& passphrase)
{
	close();
	_internal = new INTERNAL(passphrase);
	_internal->open(file, MZ_OPEN_MODE_CREATE | MZ_OPEN_MODE_WRITE, args.formats.zip.params);
	if (options & LF_WOPT_DATA_ENCRYPTION) {
		_internal->flag |= MZ_ZIP_FLAG_ENCRYPTED;
	}
}

void CLFArchiveZIP::close()
{
	if (_internal) {
		_internal->close();
		delete _internal;
		_internal = nullptr;
	}
}

bool CLFArchiveZIP::is_modify_supported()const
{
	//TODO
	return true;
}

//make a copy, and returns in "write_open" state
std::unique_ptr<ILFArchiveFile> CLFArchiveZIP::make_copy_archive(
	const std::filesystem::path& dest_path,
	const LF_COMPRESS_ARGS& args,
	std::function<bool(const LF_ENTRY_STAT&)> false_if_skip)
{
	//TODO
	throw ARCHIVE_EXCEPTION(ENOSYS);
}

std::vector<LF_COMPRESS_CAPABILITY> CLFArchiveZIP::get_compression_capability()const
{
	return { {
		LF_ARCHIVE_FORMAT::ZIP,
		L".zip",
		true,
		{
			LF_WOPT_STANDARD,
			LF_WOPT_DATA_ENCRYPTION
		}
	} };
}

LF_ENTRY_STAT* CLFArchiveZIP::read_entry_attrib()
{
	mz_zip_file* mzEntry = nullptr;
	auto result = mz_zip_entry_get_info(_internal->zip, &mzEntry);
	if (result == MZ_OK) {
		if (mzEntry) {
			_entry = {};
			_entry.stat.st_atime = mzEntry->accessed_date;
			_entry.stat.st_ctime = mzEntry->creation_date;
			_entry.stat.st_mtime = mzEntry->modified_date;
			_entry.stat.st_size = mzEntry->uncompressed_size;
			uint32_t mode;
			mz_zip_attrib_convert(
				MZ_HOST_SYSTEM(mzEntry->version_madeby),
				mzEntry->external_fa,
				MZ_HOST_SYSTEM_UNIX,
				&mode);
			_entry.stat.st_mode = mode;


			_entry.compressed_size = mzEntry->compressed_size;

			if (mzEntry->flag & MZ_ZIP_FLAG_UTF8) {
				_entry.path = UtilUTF8toUNICODE(mzEntry->filename, mzEntry->filename_size);	//stored-as
			} else {
				//[Documentation bug] mz_zip_file::filename is NOT utf-8!
				//_entry.path = UtilCP932toUNICODE(mzEntry->filename, mzEntry->filename_size);	//stored-as
				auto cp = UtilGuessCodepage(mzEntry->filename, mzEntry->filename_size);
				_entry.path = UtilToUNICODE(mzEntry->filename, mzEntry->filename_size, cp);
			}
			_entry.method_name = mzMethodName(mzEntry->compression_method);
			_entry.is_encrypted = mzEntry->flag & MZ_ZIP_FLAG_ENCRYPTED;

			return &_entry;
		} else {
			return nullptr;
		}
	} else {
		RAISE_EXCEPTION(mzError2Text(result));
	}
}

LF_ENTRY_STAT* CLFArchiveZIP::read_entry_internal(std::function<int32_t(void*)> seeker)
{
	if (!_internal || !_internal->isOpened()) {
		RAISE_EXCEPTION(L"File is not opened");
	}
	if (MZ_OK == mz_zip_entry_is_open(_internal->zip)) {
		mz_zip_entry_close(_internal->zip);
	}
	auto result = seeker(_internal->zip);
	if (result == MZ_OK) {
		auto attrib = read_entry_attrib();
		return attrib;
	} else if (MZ_END_OF_LIST == result) {
		//EOF
		return nullptr;
	} else {
		RAISE_EXCEPTION(mzError2Text(result));
	}
}

//entry seek; returns null if it reached EOF
LF_ENTRY_STAT* CLFArchiveZIP::read_entry_begin()
{
	return read_entry_internal(mz_zip_goto_first_entry);
}

LF_ENTRY_STAT* CLFArchiveZIP::read_entry_next()
{
	return read_entry_internal(mz_zip_goto_next_entry);
}

void CLFArchiveZIP::read_entry_end()
{
	mz_zip_entry_close(_internal->zip);
}

//read entry
void CLFArchiveZIP::read_file_entry_block(std::function<void(const void*, size_t, const offset_info*)> data_receiver)
{
	if (!_internal || !_internal->isOpened()) {
		RAISE_EXCEPTION(L"File is not opened");
	}
	if (MZ_OK != mz_zip_entry_is_open(_internal->zip)) {
		auto attrib = read_entry_attrib();
		if (attrib->is_encrypted) {
			if (!_internal->passphrase.get()) {
				_internal->update_passphrase();
			}
			while (true) {
				//need passphrase
				if (!_internal->passphrase.get()) {
					//cancelled
					CANCEL_EXCEPTION();
				}
				auto result = mz_zip_entry_read_open(_internal->zip, 0, _internal->passphrase.get()->c_str());
				if (result == MZ_OK)break;
				_internal->update_passphrase();
			}
		} else {
			auto result = mz_zip_entry_read_open(_internal->zip, 0, nullptr);
			if (result != MZ_OK) {
				RAISE_EXCEPTION(mzError2Text(result));
			}
		}
	}

	std::vector<unsigned char> buffer;
	buffer.resize(1024 * 1024);
	int32_t bytes_read = mz_zip_entry_read(_internal->zip, &buffer[0], buffer.size());
	if (bytes_read < 0) {
		//error
		RAISE_EXCEPTION(mzError2Text(bytes_read));
	} else {
		if (bytes_read == 0) {
			//end of entry
			data_receiver(nullptr, 0, nullptr);
		} else {
			data_receiver(&buffer[0], bytes_read, nullptr);
		}
	}
}

void CLFArchiveZIP::read_file_entry_bypass(std::function<void(const void*, size_t, const offset_info*)> data_receiver)
{
	//TODO: remove
}

struct callback_data_struct {
	callback_data_struct() {}
	std::function<LF_BUFFER_INFO()> dataProvider;
	std::vector<BYTE> buffer;
};

static int32_t read_file_callback(void* callback_data_ptr, void* dest, int32_t destSize)
{
	callback_data_struct* callback_data = (callback_data_struct*)callback_data_ptr;
	if (callback_data->buffer.empty()) {
		auto readData = callback_data->dataProvider();
		if (readData.size) {
			callback_data->buffer.assign((const BYTE*)readData.buffer, ((const BYTE*)readData.buffer) + readData.size);
		}
	}
	if (!callback_data->buffer.empty()) {
		auto data_size = std::min((size_t)destSize, callback_data->buffer.size());
		memcpy(dest, &callback_data->buffer[0], data_size);
		if (data_size < callback_data->buffer.size()) {
			callback_data->buffer.clear();
		} else {
			//purge copied data
			std::vector<BYTE> tmp;
			tmp.assign(&callback_data->buffer[data_size], &callback_data->buffer[0] + callback_data->buffer.size());
			std::swap(callback_data->buffer, tmp);
		}
		return data_size;
	}
	return 0;
}

struct LF_zip_file:mz_zip_file {
	std::string path_utf8;
};

static void build_file_info(LF_zip_file& file_info, const LF_ENTRY_STAT& stat, int method, int optionalFlag, int aesFlag)
{
	file_info = {};
	file_info.path_utf8 = stat.path.generic_u8string();
	file_info.version_madeby = MZ_VERSION_MADEBY;
	file_info.flag = MZ_ZIP_FLAG_UTF8 | optionalFlag;

	file_info.compression_method = method;
	file_info.modified_date = stat.stat.st_mtime;
	file_info.accessed_date = stat.stat.st_atime;
	file_info.creation_date = stat.stat.st_ctime;
	//file_info.compressed_size
	file_info.uncompressed_size = stat.stat.st_size;
	file_info.filename_size = (uint16_t)file_info.path_utf8.length();
	//file_info.internal_fa
	auto err = mz_zip_attrib_convert(MZ_HOST_SYSTEM_UNIX, stat.stat.st_mode, MZ_VERSION_MADEBY_HOST_SYSTEM, &file_info.external_fa);
	if (err != MZ_OK) {
		RAISE_EXCEPTION(L"Failed to convert file attribute of %s: %s", stat.path.c_str(), mzError2Text(err).c_str());
	}
	file_info.filename = file_info.path_utf8.c_str();

	//---these are left as default
	//file_info.extrafield
	//file_info.comment
	//file_info.linkname;           /* sym-link filename utf8 null-terminated string */
	//file_info.zip64                     /* zip64 extension mode */
	if (file_info.flag & MZ_ZIP_FLAG_ENCRYPTED) {
		file_info.aes_version = MZ_AES_VERSION;/* winzip aes extension if not 0 */
		file_info.aes_encryption_mode = aesFlag;
	}
	//file_info.pk_verify                 /* pkware encryption verifier */
}

//write entry
void CLFArchiveZIP::add_file_entry(const LF_ENTRY_STAT& stat, std::function<LF_BUFFER_INFO()> dataProvider)
{
	LF_zip_file file_info;
	build_file_info(file_info, stat, _internal->method, _internal->flag, _internal->aesFlag);

	const char* passphrase = nullptr;
	if (_internal->flag & MZ_ZIP_FLAG_ENCRYPTED) {
		if (!_internal->passphrase.get()) {
			_internal->update_passphrase();
		}
		//need passphrase
		if (!_internal->passphrase.get()) {
			//cancelled
			CANCEL_EXCEPTION();
		}
		passphrase = _internal->passphrase.get()->c_str();
	}
	auto err = mz_zip_entry_write_open(_internal->zip, &file_info, _internal->level, false, passphrase);
	if (err == MZ_OK) {
		for (;;) {
			auto data = dataProvider();
			if (data.buffer) {
				int totalWritten = 0;
				int toWrite = data.size;
				for (; toWrite > 0;) {
					auto written = mz_zip_entry_write(_internal->zip, (const char*)data.buffer + totalWritten, toWrite);
					if (written < 0) {
						RAISE_EXCEPTION(L"Failed to add entry %s: %s", stat.path.c_str(), mzError2Text(err).c_str());
					} else {
						totalWritten += written;
						toWrite -= written;
					}
				}
			} else {
				break;
			}
		}
	} else {
		RAISE_EXCEPTION(L"Failed to open a new entry %s: %s", stat.path.c_str(), mzError2Text(err).c_str());
	}
	if (stat.is_directory()) {
		err = mz_zip_entry_write_close(_internal->zip, 0, 0, 0);
		//err = mz_zip_entry_close(_internal->zip);
	} else {
		err = mz_zip_entry_write_close(_internal->zip, -1, -1, stat.stat.st_size);
	}
	//err = mz_zip_entry_close(_internal->zip);
	if (err != MZ_OK) {
		RAISE_EXCEPTION(L"Failed to close a new entry %s: %s", stat.path.c_str(), mzError2Text(err).c_str());
	}
}

void CLFArchiveZIP::add_directory_entry(const LF_ENTRY_STAT& stat)
{
	add_file_entry(stat, []() {LF_BUFFER_INFO bi = { 0 }; return bi; });
}

void CLFArchiveZIP::add_file_entry_bypass(const LF_ENTRY_STAT&, std::function<LF_BUFFER_INFO()> dataProvider)
{
	//TODO
}

#include "CommonUtil.h"
bool CLFArchiveZIP::is_known_format(const std::filesystem::path& arcname)
{
	try {
		CLFArchiveZIP zip;
		CLFPassphraseNULL pp;
		zip.read_open(arcname, pp);
		//zip.read_entry_begin();
		return true;
	} catch (...) {
		return false;
	}
}

#ifdef UNIT_TEST

TEST(CLFArchiveZIP, read_enum)
{
	_wsetlocale(LC_ALL, L"");	//default locale
	CLFArchiveZIP a;
	CLFPassphraseNULL pp;
	a.read_open(LF_PROJECT_DIR() / L"test/test_extract.zip", pp);
	EXPECT_TRUE(a.is_modify_supported());
	EXPECT_TRUE(a.is_bypass_io_supported());
	EXPECT_EQ(L"ZIP", a.get_format_name());
	auto entry = a.read_entry_begin();
	EXPECT_NE(nullptr, entry);
	EXPECT_EQ(entry->path.wstring(),L"dirA/dirB/");
	EXPECT_TRUE(entry->is_directory());
	EXPECT_EQ(L"Store", entry->method_name);

	entry = a.read_entry_next();
	EXPECT_EQ(entry->path.wstring(), L"dirA/dirB/dirC/");
	EXPECT_TRUE(entry->is_directory());
	EXPECT_EQ(L"Store", entry->method_name);

	entry = a.read_entry_next();
	EXPECT_EQ(entry->path.wstring(), L"dirA/dirB/dirC/file1.txt");
	EXPECT_FALSE(entry->is_directory());
	EXPECT_EQ(5, entry->stat.st_size);
	EXPECT_EQ(5, entry->compressed_size);
	EXPECT_EQ(L"Store", entry->method_name);

	std::vector<char> data;
	data.clear();
	for (;;) {
		bool bEOF = false;
		a.read_file_entry_block([&](const void* buf, size_t data_size, const offset_info* offset) {
			EXPECT_EQ(nullptr, offset);
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
	EXPECT_EQ(data.size(), 5);
	EXPECT_EQ(std::string(data.begin(), data.end()), std::string("12345"));

	entry = a.read_entry_next();
	EXPECT_EQ(entry->path.wstring(), L"dirA/dirB/file2.txt");
	EXPECT_FALSE(entry->is_directory());
	EXPECT_EQ(5, entry->stat.st_size);
	EXPECT_EQ(5, entry->compressed_size);
	EXPECT_EQ(L"Store", entry->method_name);
	data.clear();
	for (;;) {
		bool bEOF = false;
		a.read_file_entry_block([&](const void* buf, size_t data_size, const offset_info* offset) {
			EXPECT_EQ(nullptr, offset);
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
	EXPECT_EQ(data.size(), 5);
	EXPECT_EQ(std::string(data.begin(), data.end()), std::string("aaaaa"));

	entry = a.read_entry_next();
	EXPECT_EQ(entry->path.wstring(), L"あいうえお.txt");
	EXPECT_FALSE(entry->is_directory());
	EXPECT_EQ(0, entry->stat.st_size);
	EXPECT_EQ(0, entry->compressed_size);
	EXPECT_EQ(L"Store", entry->method_name);
	data.clear();
	for (;;) {
		bool bEOF = false;
		a.read_file_entry_block([&](const void* buf, size_t data_size, const offset_info* offset) {
			EXPECT_EQ(nullptr, offset);
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
	EXPECT_EQ(data.size(), 0);

	entry = a.read_entry_next();
	EXPECT_EQ(entry->path.wstring(), L"かきくけこ/file3.txt");
	EXPECT_FALSE(entry->is_directory());
	EXPECT_EQ(5, entry->stat.st_size);
	EXPECT_EQ(5, entry->compressed_size);
	EXPECT_EQ(L"Store", entry->method_name);
	data.clear();
	for (;;) {
		bool bEOF = false;
		a.read_file_entry_block([&](const void* buf, size_t data_size, const offset_info* offset) {
			EXPECT_EQ(nullptr, offset);
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
	EXPECT_EQ(data.size(), 5);
	EXPECT_EQ(std::string(data.begin(), data.end()), std::string("bbbbb"));

	entry = a.read_entry_next();
	EXPECT_EQ(nullptr, entry);
}

TEST(CLFArchiveZIP, read_enum_broken1)
{
	_wsetlocale(LC_ALL, L"");	//default locale
	CLFArchiveZIP a;
	CLFPassphraseNULL pp;
	a.read_open(LF_PROJECT_DIR() / L"test/test_broken_crc.zip", pp);
	EXPECT_TRUE(a.is_modify_supported());
	EXPECT_TRUE(a.is_bypass_io_supported());
	EXPECT_EQ(L"ZIP", a.get_format_name());
	auto entry = a.read_entry_begin();
	EXPECT_NE(nullptr, entry);
	EXPECT_EQ(entry->path.wstring(), L"dirA/dirB/");
	EXPECT_TRUE(entry->is_directory());
	EXPECT_EQ(L"Store", entry->method_name);

	entry = a.read_entry_next();
	EXPECT_EQ(entry->path.wstring(), L"dirA/dirB/dirC/");
	EXPECT_TRUE(entry->is_directory());
	EXPECT_EQ(L"Store", entry->method_name);

	entry = a.read_entry_next();
	EXPECT_EQ(entry->path.wstring(), L"dirA/dirB/dirC/file1.txt");
	EXPECT_FALSE(entry->is_directory());
	EXPECT_EQ(5, entry->stat.st_size);
	EXPECT_EQ(5, entry->compressed_size);
	EXPECT_EQ(L"Store", entry->method_name);
	std::vector<char> data;
	data.clear();
	for (;;) {
		bool bEOF = false;
		a.read_file_entry_block([&](const void* buf, size_t data_size, const offset_info* offset) {
			EXPECT_EQ(nullptr, offset);
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
	EXPECT_EQ(data.size(), 5);
	EXPECT_NE(std::string(data.begin(), data.end()), std::string("12345"));	//broken

	entry = a.read_entry_next();
	EXPECT_EQ(entry->path.wstring(), L"dirA/dirB/file2.txt");
	EXPECT_FALSE(entry->is_directory());
	EXPECT_EQ(5, entry->stat.st_size);
	EXPECT_EQ(5, entry->compressed_size);
	EXPECT_EQ(L"Store", entry->method_name);
	data.clear();
	for (;;) {
		bool bEOF = false;
		a.read_file_entry_block([&](const void* buf, size_t data_size, const offset_info* offset) {
			EXPECT_EQ(nullptr, offset);
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
	EXPECT_EQ(data.size(), 5);
	EXPECT_EQ(std::string(data.begin(), data.end()), std::string("aaaaa"));

	entry = a.read_entry_next();
	EXPECT_EQ(entry->path.wstring(), L"あいうえお.txt");
	EXPECT_FALSE(entry->is_directory());
	EXPECT_EQ(0, entry->stat.st_size);
	EXPECT_EQ(0, entry->compressed_size);
	EXPECT_EQ(L"Store", entry->method_name);

	entry = a.read_entry_next();
	EXPECT_EQ(entry->path.wstring(), L"かきくけこ/file3.txt");
	EXPECT_FALSE(entry->is_directory());
	EXPECT_EQ(5, entry->stat.st_size);
	EXPECT_EQ(5, entry->compressed_size);
	EXPECT_EQ(L"Store", entry->method_name);

	entry = a.read_entry_next();
	EXPECT_EQ(nullptr, entry);
}

TEST(CLFArchiveZIP, read_enum_broken2)
{
	_wsetlocale(LC_ALL, L"");	//default locale
	CLFArchiveZIP a;
	CLFPassphraseNULL pp;
	a.read_open(LF_PROJECT_DIR() / L"test/test_broken_file.zip", pp);
	EXPECT_EQ(L"ZIP", a.get_format_name());

	LF_ENTRY_STAT* entry = nullptr;
	EXPECT_THROW(entry = a.read_entry_begin(), LF_EXCEPTION);
}

TEST(CLFArchiveZIP, read_enum_non_existing)
{
	_wsetlocale(LC_ALL, L"");	//default locale
	CLFArchiveZIP a;
	CLFPassphraseNULL pp;
	EXPECT_THROW(
		a.read_open(LF_PROJECT_DIR() / L"test/some_file_that_does_not_exist.zip", pp),
		LF_EXCEPTION);
}

TEST(CLFArchiveZIP, read_enum_unicode)
{
	_wsetlocale(LC_ALL, L"");	//default locale
	CLFArchiveZIP a;
	CLFPassphraseNULL pp;
	a.read_open(LF_PROJECT_DIR() / L"test/test_unicode_control.zip", pp);
	EXPECT_TRUE(a.is_modify_supported());
	EXPECT_TRUE(a.is_bypass_io_supported());
	EXPECT_EQ(L"ZIP", a.get_format_name());
	auto entry = a.read_entry_begin();
	EXPECT_NE(nullptr, entry);
	EXPECT_EQ(entry->path.wstring(), L"test_unicode_control/rlo_test_\u202Eabc.txt");
	EXPECT_FALSE(entry->is_directory());
	EXPECT_EQ(L"Store", entry->method_name);

	entry = a.read_entry_next();
	EXPECT_EQ(entry->path.wstring(), L"test_unicode_control/standard.txt");
	EXPECT_FALSE(entry->is_directory());
	EXPECT_EQ(L"Store", entry->method_name);

	entry = a.read_entry_next();
	EXPECT_EQ(nullptr, entry);
}

TEST(CLFArchiveZIP, read_enum_sfx)
{
	_wsetlocale(LC_ALL, L"");	//default locale
	CLFArchiveZIP a;
	CLFPassphraseNULL pp;
	a.read_open(LF_PROJECT_DIR() / L"test/test_zip_sfx.dat", pp);
	EXPECT_TRUE(a.is_modify_supported());
	EXPECT_TRUE(a.is_bypass_io_supported());
	EXPECT_EQ(L"ZIP", a.get_format_name());
	auto entry = a.read_entry_begin();
	EXPECT_NE(nullptr, entry);
	EXPECT_EQ(entry->path.wstring(), L"dirA/dirB/");
	EXPECT_TRUE(entry->is_directory());
	EXPECT_EQ(L"Store", entry->method_name);

	entry = a.read_entry_next();
	EXPECT_EQ(entry->path.wstring(), L"dirA/dirB/dirC/");
	EXPECT_TRUE(entry->is_directory());
	EXPECT_EQ(L"Store", entry->method_name);

	entry = a.read_entry_next();
	EXPECT_EQ(entry->path.wstring(), L"dirA/dirB/dirC/file1.txt");
	EXPECT_FALSE(entry->is_directory());
	EXPECT_EQ(5, entry->stat.st_size);
	EXPECT_EQ(5, entry->compressed_size);
	EXPECT_EQ(L"Store", entry->method_name);
	std::vector<char> data;
	data.clear();
	for (;;) {
		bool bEOF = false;
		a.read_file_entry_block([&](const void* buf, size_t data_size, const offset_info* offset) {
			EXPECT_EQ(nullptr, offset);
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
	EXPECT_EQ(data.size(), 5);
	EXPECT_EQ(std::string(data.begin(), data.end()), std::string("12345"));

	entry = a.read_entry_next();
	EXPECT_EQ(entry->path.wstring(), L"dirA/dirB/file2.txt");
	EXPECT_FALSE(entry->is_directory());
	EXPECT_EQ(5, entry->stat.st_size);
	EXPECT_EQ(5, entry->compressed_size);
	EXPECT_EQ(L"Store", entry->method_name);

	entry = a.read_entry_next();
	EXPECT_EQ(entry->path.wstring(), L"あいうえお.txt");
	EXPECT_FALSE(entry->is_directory());
	EXPECT_EQ(0, entry->stat.st_size);
	EXPECT_EQ(0, entry->compressed_size);
	EXPECT_EQ(L"Store", entry->method_name);

	entry = a.read_entry_next();
	EXPECT_EQ(entry->path.wstring(), L"かきくけこ/file3.txt");
	EXPECT_FALSE(entry->is_directory());
	EXPECT_EQ(5, entry->stat.st_size);
	EXPECT_EQ(5, entry->compressed_size);
	EXPECT_EQ(L"Store", entry->method_name);

	entry = a.read_entry_next();
	EXPECT_EQ(nullptr, entry);
}

TEST(CLFArchiveZIP, read_passphrase)
{
	_wsetlocale(LC_ALL, L"");	//default locale
	CLFArchiveZIP a;
	{
		//---content listing does not require passphrase
		CLFPassphraseNULL pp;
		a.read_open(LF_PROJECT_DIR() / L"test/test_password_abcde.zip", pp);
		for (auto item = a.read_entry_begin(); item; item = a.read_entry_next()) {
			//do nothing
			EXPECT_TRUE(item->is_encrypted);
		}
	}
	{
		//---content listing does not require passphrase
		CLFPassphraseConst pp(L"abcde");
		a.read_open(LF_PROJECT_DIR() / L"test/test_password_abcde.zip", pp);
		for (auto item = a.read_entry_begin(); item; item = a.read_entry_next()) {
			//do nothing
			EXPECT_TRUE(item->is_encrypted);
		}
		EXPECT_EQ(L"ZIP", a.get_format_name());
		auto entry = a.read_entry_begin();
		EXPECT_NE(nullptr, entry);
		EXPECT_EQ(entry->path.wstring(), L"test.txt");
		EXPECT_FALSE(entry->is_directory());
		EXPECT_EQ(L"Store", entry->method_name);
	}

	{
		//---content listing does not require passphrase
		CLFPassphraseConst pp(L"abcde");
		a.read_open(LF_PROJECT_DIR() / L"test/test_password_abcde.zip", pp);
		std::vector<char> data;
		data.clear();
		auto entry = a.read_entry_begin();
		for (;;) {
			bool bEOF = false;
			a.read_file_entry_block([&](const void* buf, size_t data_size, const offset_info* offset) {
				EXPECT_EQ(nullptr, offset);
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
		EXPECT_EQ(data.size(), 7);
		EXPECT_EQ(std::string(data.begin(), data.end()), std::string("abcde\r\n"));
	}
}

TEST(CLFArchiveZIP, zipx)
{
	CLFArchiveZIP a;
	CLFPassphraseNULL pp;
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

TEST(CLFArchiveZIP, is_known_format)
{
	{
		const auto dir = LF_PROJECT_DIR() / L"ArchiverCode/test";
		EXPECT_FALSE(CLFArchiveZIP::is_known_format(dir / L"empty.gz"));
		EXPECT_FALSE(CLFArchiveZIP::is_known_format(dir / L"empty.bz2"));
		EXPECT_FALSE(CLFArchiveZIP::is_known_format(dir / L"empty.xz"));
		EXPECT_FALSE(CLFArchiveZIP::is_known_format(dir / L"empty.lzma"));
		EXPECT_FALSE(CLFArchiveZIP::is_known_format(dir / L"empty.zst"));

		EXPECT_FALSE(CLFArchiveZIP::is_known_format(dir / L"abcde.gz"));
		EXPECT_FALSE(CLFArchiveZIP::is_known_format(dir / L"abcde.bz2"));
		EXPECT_FALSE(CLFArchiveZIP::is_known_format(dir / L"abcde.xz"));
		EXPECT_FALSE(CLFArchiveZIP::is_known_format(dir / L"abcde.lzma"));
		EXPECT_FALSE(CLFArchiveZIP::is_known_format(dir / L"abcde.zst"));

		EXPECT_FALSE(CLFArchiveZIP::is_known_format(__FILEW__));
		EXPECT_FALSE(CLFArchiveZIP::is_known_format(L"some_non_existing_file"));
	}
	{
		const auto dir = LF_PROJECT_DIR() / L"test";
		EXPECT_TRUE(CLFArchiveZIP::is_known_format(dir / L"test_broken_file.zip"));
		EXPECT_TRUE(CLFArchiveZIP::is_known_format(dir / L"test_broken_crc.zip"));
		EXPECT_TRUE(CLFArchiveZIP::is_known_format(dir / L"test_extract.zip"));
		EXPECT_TRUE(CLFArchiveZIP::is_known_format(dir / L"test_extract.zipx"));
		EXPECT_TRUE(CLFArchiveZIP::is_known_format(dir / L"test_password_abcde.zip"));
		EXPECT_TRUE(CLFArchiveZIP::is_known_format(dir / L"test_unicode_control.zip"));
		EXPECT_TRUE(CLFArchiveZIP::is_known_format(dir / L"test_zip_sfx.dat"));
	}
}

#include "Utilities/FileOperation.h"
TEST(CLFArchiveZIP, add_file_entry)
{
	auto temp = UtilGetTemporaryFileName();
	auto src = UtilGetTemporaryFileName();
	{
		CAutoFile f;
		f.open(src, L"w");
		for (int i = 0; i < 100; i++) {
			fputs("abcde12345", f);
		}
	}
	{
		CLFArchiveZIP a;
		LF_COMPRESS_ARGS args;
		args.load(CConfigFile());
		CLFPassphraseNULL pp;
		a.write_open(temp, LF_ARCHIVE_FORMAT::ZIP, LF_WOPT_STANDARD, args, pp);
		LF_ENTRY_STAT e;

		RAW_FILE_READER provider;
		provider.open(src);
		e.read_stat(src, L"test/file.txt");
		a.add_file_entry(e, [&]() {
			auto data = provider();
			return data;
		});
		a.close();
	}
	{
		CLFArchiveZIP a;
		CLFPassphraseNULL pp;
		a.read_open(temp, pp);
		auto entry = a.read_entry_begin();
		EXPECT_NE(nullptr, entry);
		EXPECT_EQ(L"test/file.txt", entry->path.wstring());
		EXPECT_EQ(1000, entry->stat.st_size);
	}
	UtilDeletePath(temp);
	EXPECT_FALSE(std::filesystem::exists(temp));
	UtilDeletePath(src);
	EXPECT_FALSE(std::filesystem::exists(src));
}

TEST(CLFArchiveZIP, add_directory_entry)
{
	auto temp = UtilGetTemporaryFileName();
	{
		CLFArchiveZIP a;
		LF_COMPRESS_ARGS args;
		args.load(CConfigFile());
		CLFPassphraseNULL pp;
		a.write_open(temp, LF_ARCHIVE_FORMAT::ZIP, LF_WOPT_STANDARD, args, pp);
		LF_ENTRY_STAT e;
		e.read_stat(LF_PROJECT_DIR(), L"test/");	//LF_PROJECT_DIR() as a directory template
		a.add_directory_entry(e);
		a.close();
	}
	{
		CLFArchiveZIP a;
		CLFPassphraseNULL pp;
		a.read_open(temp, pp);
		auto entry = a.read_entry_begin();
		EXPECT_NE(nullptr, entry);
		EXPECT_EQ(L"test/", entry->path.wstring());
	}
	UtilDeletePath(temp);
	EXPECT_FALSE(std::filesystem::exists(temp));
}

TEST(CLFArchiveZIP, add_file_entry_with_password)
{
	auto temp = UtilGetTemporaryFileName();
	auto src = UtilGetTemporaryFileName();
	{
		CAutoFile f;
		f.open(src, L"w");
		for (int i = 0; i < 100; i++) {
			fputs("abcde12345", f);
		}
	}
	{
		CLFArchiveZIP a;
		LF_COMPRESS_ARGS args;
		args.load(CConfigFile());
		CLFPassphraseConst pp(L"password");
		a.write_open(temp, LF_ARCHIVE_FORMAT::ZIP, LF_WOPT_DATA_ENCRYPTION, args, pp);
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
		a.close();
	}
	{
		//---content listing does not require passphrase
		CLFPassphraseNULL pp;
		CLFArchiveZIP a;
		a.read_open(temp, pp);
		for (auto item = a.read_entry_begin(); item; item = a.read_entry_next()) {
			//do nothing
			EXPECT_TRUE(item->is_encrypted);
		}
	}
	{
		CLFArchiveZIP a;
		CLFPassphraseConst pp(L"password");
		a.read_open(temp, pp);
		auto entry = a.read_entry_begin();
		EXPECT_NE(nullptr, entry);
		EXPECT_EQ(L"test/", entry->path.wstring());

		entry = a.read_entry_next();
		EXPECT_NE(nullptr, entry);
		EXPECT_EQ(L"test/file.txt", entry->path.wstring());
		EXPECT_EQ(1000, entry->stat.st_size);

		std::vector<char> data;
		data.clear();
		for (;;) {
			bool bEOF = false;
			a.read_file_entry_block([&](const void* buf, size_t data_size, const offset_info* offset) {
				EXPECT_EQ(nullptr, offset);
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
		for (int i = 0; i < 100; i++) {
			EXPECT_EQ(std::string(&data[10 * i], &data[10 * i] + 10), std::string("abcde12345"));
		}

	}
	UtilDeletePath(temp);
	EXPECT_FALSE(std::filesystem::exists(temp));
	UtilDeletePath(src);
	EXPECT_FALSE(std::filesystem::exists(src));
}

TEST(CLFArchiveZIP, add_file_entry_methods_and_levels)
{
	auto temp = UtilGetTemporaryFileName();
	auto src = UtilGetTemporaryFileName();
	{
		CAutoFile f;
		f.open(src, L"w");
		for (int i = 0; i < 100; i++) {
			fputs("abcde12345", f);
		}
	}
	for (int level = 1; level <= 9; level++) {
		std::map<std::string, std::wstring> methods = {
			{"store", L"Store"},
			{"deflate", L"Deflate"},
			{"bzip2", L"Bzip2"},
			{"lzma", L"LZMA1"},
			{"zstd", L"ZSTD"},
			{"xz", L"XZ"},
		};
		for (const auto& method : methods) {
			{
				CLFArchiveZIP a;
				LF_COMPRESS_ARGS args;
				args.load(CConfigFile());
				args.formats.zip.params["method"] = method.first;
				args.formats.zip.params["level"] = UtilToUTF8(Format(L"%d", level));
				CLFPassphraseNULL pp;
				a.write_open(temp, LF_ARCHIVE_FORMAT::ZIP, LF_WOPT_STANDARD, args, pp);
				LF_ENTRY_STAT e;

				RAW_FILE_READER provider;
				provider.open(src);
				e.read_stat(src, L"test/file.txt");
				a.add_file_entry(e, [&]() {
					auto data = provider();
					return data;
				});
				a.close();
			}
			{
				CLFArchiveZIP a;
				CLFPassphraseNULL pp;
				a.read_open(temp, pp);
				auto entry = a.read_entry_begin();
				EXPECT_NE(nullptr, entry);
				EXPECT_EQ(L"test/file.txt", entry->path.wstring());
				EXPECT_EQ(1000, entry->stat.st_size);
				EXPECT_LE(entry->compressed_size, 1000);
				EXPECT_EQ(method.second, entry->method_name);
				//EXPECT_EQ(level, ); no way to get compression level; checking creation errors only
			}
			UtilDeletePath(temp);
			EXPECT_FALSE(std::filesystem::exists(temp));
		}
	}
	UtilDeletePath(src);
	EXPECT_FALSE(std::filesystem::exists(src));
}

TEST(CLFArchiveZIP, add_file_entry_crypto_level)
{
	auto temp = UtilGetTemporaryFileName();
	auto src = UtilGetTemporaryFileName();
	{
		CAutoFile f;
		f.open(src, L"w");
		for (int i = 0; i < 100; i++) {
			fputs("abcde12345", f);
		}
	}
	std::vector<std::string> codes = {
		"aes256", "aes192", "aes128"
	};
	for (const auto& code : codes) {
		CLFPassphraseConst pp(L"password");
		{
			CLFArchiveZIP a;
			LF_COMPRESS_ARGS args;
			args.load(CConfigFile());
			args.formats.zip.params["crypto"] = code;
			a.write_open(temp, LF_ARCHIVE_FORMAT::ZIP, LF_WOPT_DATA_ENCRYPTION, args, pp);
			LF_ENTRY_STAT e;

			RAW_FILE_READER provider;
			provider.open(src);
			e.read_stat(src, L"test/file.txt");
			a.add_file_entry(e, [&]() {
				auto data = provider();
				return data;
			});
			a.close();
		}
		{
			CLFArchiveZIP a;
			a.read_open(temp, pp);
			auto entry = a.read_entry_begin();
			EXPECT_NE(nullptr, entry);
			EXPECT_EQ(L"test/file.txt", entry->path.wstring());
			EXPECT_TRUE(entry->is_encrypted);
		}
		UtilDeletePath(temp);
		EXPECT_FALSE(std::filesystem::exists(temp));
	}
	UtilDeletePath(src);
	EXPECT_FALSE(std::filesystem::exists(src));
}

/*
add to existing
remove from existing
bypass
remove zip from libarchive
*/

#endif
