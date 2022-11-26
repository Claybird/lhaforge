#include "stdafx.h"
#include "archive_zip.h"
#include "zip.h"
#include "mz_zip.h"
#include "mz_strm_os.h"
#include "mz_os.h"

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
	INTERNAL(ILFPassphrase& pcb):zip(nullptr), stream(nullptr),passphrase_callback(pcb) {}
	virtual ~INTERNAL() { close(); }

	void* zip;		//mz_zip
	void* stream;	//mz_stream
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
	}
	void open(std::filesystem::path path, int32_t mode) {
		close();
		mz_stream_os_create(&stream);
		mz_stream_os_open(stream, path.u8string().c_str(), mode);

		mz_zip_create(&zip);
		auto err = mz_zip_open(zip, stream, mode);
		if (err != MZ_OK) {
			RAISE_EXCEPTION(mzError2Text(err));
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
	_internal->open(file, MZ_OPEN_MODE_READ | MZ_OPEN_MODE_EXISTING);
}

void CLFArchiveZIP::write_open(const std::filesystem::path& file, LF_ARCHIVE_FORMAT format, LF_WRITE_OPTIONS options, const LF_COMPRESS_ARGS& args, ILFPassphrase& passphrase)
{
	close();
	_internal = new INTERNAL(passphrase);
	_internal->open(file, MZ_OPEN_MODE_CREATE);
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

//entry seek; returns null if it reached EOF
LF_ENTRY_STAT* CLFArchiveZIP::read_entry_begin()
{
	if (!_internal->isOpened()) {
		RAISE_EXCEPTION(L"File is not opened");
	}
	auto result = mz_zip_goto_first_entry(_internal->zip);
	if (result == MZ_OK) {
		auto attrib = read_entry_attrib();
		if (result == MZ_OK) {
			if (attrib->is_encrypted) {
				if (!_internal->passphrase.get()) {
					_internal->update_passphrase();
				}
				while (true) {
					//need passphrase
					result = mz_zip_entry_read_open(_internal->zip, 0, _internal->passphrase.get()->c_str());
					if (result == MZ_OK)break;
					_internal->update_passphrase();
					if (!_internal->passphrase.get()) {
						//cancelled
						CANCEL_EXCEPTION();
					}
				}
			} else {
				result = mz_zip_entry_read_open(_internal->zip, 0, nullptr);
			}
			if (result == MZ_OK) {
				return attrib;
			} else {
				RAISE_EXCEPTION(mzError2Text(result));
			}
		} else {
			RAISE_EXCEPTION(mzError2Text(result));
		}
	} else {
		RAISE_EXCEPTION(mzError2Text(result));
	}
}

LF_ENTRY_STAT* CLFArchiveZIP::read_entry_next()
{
	if (!_internal->isOpened()) {
		RAISE_EXCEPTION(L"File is not opened");
	}
	mz_zip_entry_close(_internal->zip);
	auto result = mz_zip_goto_next_entry(_internal->zip);
	if (result == MZ_OK) {
		auto attrib = read_entry_attrib();
		if (result == MZ_OK) {
			if (attrib->is_encrypted) {
				if (!_internal->passphrase.get()) {
					_internal->update_passphrase();
				}
				while (true) {
					//need passphrase
					result = mz_zip_entry_read_open(_internal->zip, 0, _internal->passphrase.get()->c_str());
					if (result == MZ_OK)break;
					_internal->update_passphrase();
					if (!_internal->passphrase.get()) {
						//cancelled
						CANCEL_EXCEPTION();
					}
				}
			} else {
				result = mz_zip_entry_read_open(_internal->zip, 0, nullptr);
			}
			if (result == MZ_OK) {
				return attrib;
			} else {
				RAISE_EXCEPTION(mzError2Text(result));
			}
		} else {
			RAISE_EXCEPTION(mzError2Text(result));
		}
	}else if(MZ_END_OF_LIST==result){
		//EOF
		return nullptr;
	} else {
		RAISE_EXCEPTION(mzError2Text(result));
	}
}

void CLFArchiveZIP::read_entry_end()
{
	mz_zip_entry_close(_internal->zip);
}

//read entry
void CLFArchiveZIP::read_file_entry_block(std::function<void(const void*, size_t, const offset_info*)> data_receiver)
{
	if (!_internal->isOpened()) {
		RAISE_EXCEPTION(L"File is not opened");
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

static LF_zip_file build_file_info(const LF_ENTRY_STAT& stat)
{
	LF_zip_file file_info = { 0 };
	file_info.path_utf8 = stat.path.generic_u8string();
	file_info.version_madeby = MZ_VERSION_MADEBY;
	file_info.flag = MZ_ZIP_FLAG_UTF8;	//TODO: MZ_ZIP_FLAG_ENCRYPTED if encrypted
	//TODO: MZ_ZIP_FLAG_DEFLATE_MAX/MZ_ZIP_FLAG_DEFLATE_NORMAL/MZ_ZIP_FLAG_DEFLATE_FAST/MZ_ZIP_FLAG_DEFLATE_SUPER_FAST

	file_info.compression_method = MZ_COMPRESS_METHOD_STORE;	//TODO
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
		file_info.aes_encryption_mode = MZ_AES_ENCRYPTION_MODE_256;	/* winzip aes encryption mode */
		//TODO:MZ_AES_ENCRYPTION_MODE_128 or MZ_AES_ENCRYPTION_MODE_192
	}
	//file_info.pk_verify                 /* pkware encryption verifier */

	return file_info;
}

//write entry
void CLFArchiveZIP::add_file_entry(const LF_ENTRY_STAT& stat, std::function<LF_BUFFER_INFO()> dataProvider)
{
	auto file_info = build_file_info(stat);

	auto passphrase = nullptr;	//TODO
	auto err = mz_zip_entry_write_open(_internal->zip, &file_info, 9/*TODO*/, false, passphrase);
	if (err == MZ_OK) {
		for (;;) {
			auto data = dataProvider();
			if (data.buffer) {
				err = mz_zip_entry_write(_internal->zip, data.buffer, data.size);
				if (err != MZ_OK) {
					RAISE_EXCEPTION(L"Failed to add entry %s: %s", stat.path.c_str(), mzError2Text(err).c_str());
				}
			} else {
				break;
			}
		}
	} else {
		RAISE_EXCEPTION(L"Failed to open a new entry %s: %s", stat.path.c_str(), mzError2Text(err).c_str());
	}
	err = mz_zip_entry_close(_internal->zip);
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
		zip.read_open(arcname, CLFPassphraseNULL());
		zip.read_entry_begin();
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
	a.read_open(LF_PROJECT_DIR() / L"test/test_extract.zip", CLFPassphraseNULL());
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
	a.read_open(LF_PROJECT_DIR() / L"test/test_broken_crc.zip", CLFPassphraseNULL());
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
	a.read_open(LF_PROJECT_DIR() / L"test/test_broken_file.zip", CLFPassphraseNULL());
	EXPECT_EQ(L"ZIP", a.get_format_name());

	LF_ENTRY_STAT* entry = nullptr;
	EXPECT_THROW(entry = a.read_entry_begin(), LF_EXCEPTION);
}


TEST(CLFArchiveZIP, read_enum_unicode)
{
	_wsetlocale(LC_ALL, L"");	//default locale
	CLFArchiveZIP a;
	a.read_open(LF_PROJECT_DIR() / L"test/test_unicode_control.zip", CLFPassphraseNULL());
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
	a.read_open(LF_PROJECT_DIR() / L"test/test_zip_sfx.dat", CLFPassphraseNULL());
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
/*
TEST(CLFArchiveZIP, read_password)
{
	_wsetlocale(LC_ALL, L"");	//default locale
	CLFArchiveZIP a;
	EXPECT_THROW(
		a.read_open(LF_PROJECT_DIR() / L"test/test_password_abcde.zip", CLFPassphraseNULL()),
		LF_EXCEPTION);
	a.read_open(LF_PROJECT_DIR() / L"test/test_password_abcde.zip", CLFPassphraseConst(L"abcde"));
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
}*/

/*zipx ?
not exist
is known format// unsupported format detection
create
create with password*/

#endif
