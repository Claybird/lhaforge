#include "stdafx.h"
#include "archive_zip.h"
#include "zip.h"
#include "mz_strm.h"
#include "mz_strm_split.h"
#include "mz_strm_os.h"
#include "mz_zip.h"
#include "mz_os.h"
//#include "mz_crypt.h"
#include "mz_zip_rw.h"
#include "compress.h"
#include "Utilities/ContinuousFile.h"

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
		case MZ_SUPPORT_ERROR:return L"Not supported error";
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

int32_t mz_stream_LF_is_open(void* stream);
int32_t mz_stream_LF_open(void* stream, const char* path, int32_t mode);
int32_t mz_stream_LF_read(void* stream, void* buf, int32_t size);
int64_t mz_stream_LF_tell(void* stream);
int32_t mz_stream_LF_seek(void* stream, int64_t offset, int32_t origin);
int32_t mz_stream_LF_close(void* stream);
int32_t mz_stream_LF_error(void* stream);
void* mz_stream_LF_create(void);
void mz_stream_LF_delete(void** stream);

typedef struct mz_stream_LF_s {
	mz_stream       stream;
	CContinuousFile handle;
} mz_stream_LF;

static mz_stream_vtbl mz_stream_LF_vtbl = {
	mz_stream_LF_open,
	mz_stream_LF_is_open,
	mz_stream_LF_read,
	NULL,
	mz_stream_LF_tell,
	mz_stream_LF_seek,
	mz_stream_LF_close,
	mz_stream_LF_error,
	mz_stream_LF_create,
	mz_stream_LF_delete,
	NULL,
	NULL
};

int32_t mz_stream_LF_is_open(void* stream) {
	mz_stream_LF* lff = (mz_stream_LF*)stream;
	if (!lff->handle.is_opened())
		return MZ_OPEN_ERROR;
	return MZ_OK;
}

int32_t mz_stream_LF_open(void* stream, const char* path_utf8, int32_t mode) {
	mz_stream_LF* lff = (mz_stream_LF*)stream;

	if (!path_utf8)return MZ_PARAM_ERROR;

	if ((mode & MZ_OPEN_MODE_READWRITE) != MZ_OPEN_MODE_READ) {
		return MZ_PARAM_ERROR;
	}

	std::filesystem::path path;
	{
		wchar_t* path_wide = mz_os_unicode_string_create(path_utf8, MZ_ENCODING_UTF8);
		if (!path_wide)return MZ_PARAM_ERROR;
		path = path_wide;
		mz_os_unicode_string_delete(&path_wide);
	}

	std::vector<std::filesystem::path> files;
	std::wregex re_splittedA(L"\\.[zZ]\\d\\d");
	std::wregex re_splittedB(L"\\.\\d\\d\\d");
	if (std::regex_search(path.extension().wstring(), re_splittedA)) {
		//.zXX
		for (int i = 0; i < 99; i++) {
			auto p = path;
			p.replace_extension(Format(L".z%02d", i));
			if (std::filesystem::exists(p)) {
				files.push_back(p);
			} else {
				if (i != 0)break;
			}
		}
	} else if (std::regex_search(path.extension().wstring(), re_splittedB)) {
		//.XXX
		for (int i = 0; i < 99; i++) {
			auto p = path;
			p.replace_extension(Format(L".%03d", i));
			if (std::filesystem::exists(p)) {
				files.push_back(p);
			} else {
				if (i != 0)break;
			}
		}
	} else {
		files.push_back(path);
	}

	lff->handle.openFiles(files);

	if (mz_stream_LF_is_open(stream) != MZ_OK) {
		return MZ_OPEN_ERROR;
	}

	return MZ_OK;
}

int32_t mz_stream_LF_read(void* stream, void* buf, int32_t size) {
	mz_stream_LF* lff = (mz_stream_LF*)stream;

	if (mz_stream_LF_is_open(stream) != MZ_OK)return MZ_OPEN_ERROR;

	try {
		size_t read = lff->handle.read(buf, size);
		return (int32_t)read;
	} catch (const LF_EXCEPTION&) {
		return MZ_READ_ERROR;
	}
}

int64_t mz_stream_LF_tell(void* stream) {
	mz_stream_LF* lff = (mz_stream_LF*)stream;

	if (mz_stream_LF_is_open(stream) != MZ_OK)return MZ_OPEN_ERROR;
	return lff->handle.tell();
}

int32_t mz_stream_LF_seek(void* stream, int64_t offset, int32_t origin) {
	mz_stream_LF* lff = (mz_stream_LF*)stream;

	if (mz_stream_LF_is_open(stream) != MZ_OK)return MZ_OPEN_ERROR;

	switch (origin) {
	case MZ_SEEK_CUR:
		if (lff->handle.seek(offset, SEEK_CUR))return MZ_OK;
		else return MZ_SEEK_ERROR;
	case MZ_SEEK_END:
		if (lff->handle.seek(offset, SEEK_END))return MZ_OK;
		else return MZ_SEEK_ERROR;
	case MZ_SEEK_SET:
		if (lff->handle.seek(offset, SEEK_SET))return MZ_OK;
		else return MZ_SEEK_ERROR;
	default:
		return MZ_SEEK_ERROR;
	}
}

int32_t mz_stream_LF_close(void* stream) {
	mz_stream_LF* lff = (mz_stream_LF*)stream;
	lff->handle.close();
	return MZ_OK;
}

int32_t mz_stream_LF_error(void* stream) {
	return MZ_OK;
}

void* mz_stream_LF_create() {
	mz_stream_LF* lff = new mz_stream_LF;
	lff->stream.vtbl = &mz_stream_LF_vtbl;

	return lff;
}

void mz_stream_LF_delete(void** stream) {
	if (stream) {
		mz_stream_LF* lff = (mz_stream_LF*)*stream;
		if (lff)delete lff;
		*stream = NULL;
	}
}

static bool isMultiPartZip(const std::filesystem::path& path)
{
	std::vector<std::filesystem::path> files;
	std::wregex re_splittedA(L"\\.[zZ]\\d\\d");
	std::wregex re_splittedB(L"\\.\\d\\d\\d");
	if (std::regex_search(path.extension().wstring(), re_splittedA)) {
		return true;
	} else if (std::regex_search(path.extension().wstring(), re_splittedB)) {
		//.XXX
		return true;
	} else {
		return false;
	}
}


struct LF_zip_file :mz_zip_file {
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
		if (aesFlag == 0) {
			file_info.aes_version = 0;/* winzip aes extension if not 0 */
			file_info.aes_strength = aesFlag;
		} else {
			file_info.aes_version = MZ_AES_VERSION;/* winzip aes extension if not 0 */
			file_info.aes_strength = aesFlag;
		}
	}
	//file_info.pk_verify                 /* pkware encryption verifier */
}

struct MINIZIP_PASSPHRASE_BASE {
	std::shared_ptr<std::string> passphrase;	//UTF-8
	std::shared_ptr<ILFPassphrase> passphrase_callback;

	MINIZIP_PASSPHRASE_BASE(std::shared_ptr<ILFPassphrase> pcb) :passphrase_callback(pcb){}
	void update_passphrase() {
		auto callback = passphrase_callback.get();
		if (callback) {
			const char* p = (*callback)();
			if (p) {
				passphrase = std::make_shared<std::string>(p);
			} else {
				passphrase.reset();
			}
		}
	}
	static int32_t password_cb(void* handle, void* userdata, mz_zip_file* file_info, char* password, int32_t max_password) {
		MINIZIP_PASSPHRASE_BASE* base = (MINIZIP_PASSPHRASE_BASE*)userdata;
		if (!base->passphrase.get()) {
			base->update_passphrase();
		}
		//need passphrase
		if (!base->passphrase.get()) {
			//cancelled
			CANCEL_EXCEPTION();
		}
		strncpy_s(password, max_password, base->passphrase.get()->c_str(), base->passphrase.get()->length());
		return 0;
	}
};

struct MINIZIP_READER {
	void* reader;
	void* stream;
	std::filesystem::path _path;
	std::shared_ptr<MINIZIP_PASSPHRASE_BASE> _password_cb;

	MINIZIP_READER(std::shared_ptr<MINIZIP_PASSPHRASE_BASE> pcb):reader(nullptr),stream(nullptr),_password_cb(pcb) {}
	virtual ~MINIZIP_READER() {
		close();
	}
	void close() {
		if (reader) {
			mz_zip_reader_close(reader);
			mz_zip_reader_delete(&reader);
			reader = nullptr;
		}
		if (stream) {
			mz_stream_LF_close(stream);
			mz_stream_LF_delete(&stream);
			stream = nullptr;
		}
		_path.clear();
	}
	bool is_open()const { return reader != nullptr; }
	void open(const std::filesystem::path& path) {
		close();
		_path = path;
		reader = mz_zip_reader_create();
		if (!reader)RAISE_EXCEPTION(L"Failed to create zip reader");
		mz_zip_reader_set_password_cb(reader, _password_cb.get(), MINIZIP_PASSPHRASE_BASE::password_cb);

		//auto err = mz_zip_reader_open_file(reader, path.u8string().c_str());
		stream = mz_stream_LF_create();
		mz_stream_LF_open(stream, path.u8string().c_str(), MZ_OPEN_MODE_READ);
		auto err = mz_zip_reader_open(reader, stream);
		if (err != MZ_OK) {
			RAISE_EXCEPTION(L"Failed to open file %s: %s", path.c_str(), mzError2Text(err).c_str());
		}
	}
	mz_zip_file* rewind() {
		auto err = mz_zip_reader_goto_first_entry(reader);
		if (err == MZ_END_OF_LIST) {
			return nullptr;
		}
		if (err != MZ_OK) {
			RAISE_EXCEPTION(L"Failed to seek first entry of file %s: %s", _path.c_str(), mzError2Text(err).c_str());
		}
		return get_entry_info();
	}
	mz_zip_file* next() {
		auto err = mz_zip_reader_goto_next_entry(reader);
		if (err == MZ_END_OF_LIST) {
			return nullptr;
		}
		if (err != MZ_OK) {
			RAISE_EXCEPTION(L"Failed to seek entry in file %s: %s", _path.c_str(), mzError2Text(err).c_str());
		}
		return get_entry_info();
	}
	mz_zip_file* get_entry_info() {
		mz_zip_file* fileinfo = nullptr;
		auto err = mz_zip_reader_entry_get_info(reader, &fileinfo);
		if (err != MZ_OK) {
			RAISE_EXCEPTION(L"Failed to get entry info in file %s: %s", _path.c_str(), mzError2Text(err).c_str());
		}
		return fileinfo;
	}
	bool is_any_encrypted() {
		for (auto entry = rewind(); entry; entry = next()) {
			if (entry->flag & MZ_ZIP_FLAG_ENCRYPTED)return true;
		}
		return false;
	}
	void raw_copy_into_file(std::filesystem::path dest, std::function<bool(mz_zip_file*)> keep_check) {
		void* writer = mz_zip_writer_create();
		auto err = mz_zip_writer_open_file(writer, dest.u8string().c_str(), 0, 0);
		if (err != MZ_OK) {
			mz_zip_writer_delete(&writer);
			RAISE_EXCEPTION(
				L"Failed to open copy dest file %s: %s",
				dest.c_str(),
				mzError2Text(err).c_str()
			);
		}

		for (auto entry = rewind(); entry; entry = next()) {
			if (keep_check(entry)) {
				err = mz_zip_writer_copy_from_reader(writer, reader);
				if (err != MZ_OK) {
					mz_zip_writer_delete(&writer);
					RAISE_EXCEPTION(
						L"Failed to copy zip entry %s from %s: %s",
						UtilUTF8toUNICODE(entry->filename).c_str(),
						_path.c_str(),
						mzError2Text(err).c_str()
					);
				}
			}
		}

		uint8_t zip_cd = 0;
		mz_zip_reader_get_zip_cd(reader, &zip_cd);
		mz_zip_writer_set_zip_cd(writer, zip_cd);
		mz_zip_writer_close(writer);
		mz_zip_writer_delete(&writer);
	}

	struct auto_entry {
		void* _reader;
		auto_entry(void* reader) :_reader(reader) {
			auto err = mz_zip_reader_entry_open(_reader);
			if (MZ_OK != err) {
				RAISE_EXCEPTION(mzError2Text(err));
			}
		}
		int32_t close() {
			if (_reader) {
				auto err = mz_zip_reader_entry_close(_reader);
				_reader = nullptr;
				return err;
			} else {
				return MZ_OK;
			}
		}
		virtual ~auto_entry() {
			close();
		}
	};
	void read_entry(std::function<void(const void*, size_t, const offset_info*)> data_receiver) {
		auto_entry ae(reader);
		std::vector<unsigned char> buffer;
		buffer.resize(1024 * 1024);
		for (;;) {
			auto read = mz_zip_reader_entry_read(reader, &buffer[0], (int32_t)buffer.size());
			if (read == 0) {
				auto err = ae.close();
				if (MZ_OK != err) {
					RAISE_EXCEPTION(
						L"Error while reading zip entry from %s: %s",
						_path.c_str(),
						mzError2Text(err).c_str()
					);
				}
				//end of entry
				data_receiver(nullptr, 0, nullptr);
				break;
			} else if (read < 0) {
				RAISE_EXCEPTION(mzError2Text(read));
			} else {
				data_receiver(&buffer[0], read, nullptr);
			}
		}
	}
};


struct MINIZIP_WRITER {
	void* writer;
	std::filesystem::path _path;
	std::shared_ptr<MINIZIP_PASSPHRASE_BASE> _password_cb;
	int _aes_encryption;
	int _method;
	int _flag;

	MINIZIP_WRITER(std::shared_ptr<MINIZIP_PASSPHRASE_BASE> pcb) :
		writer(nullptr),
		_password_cb(pcb),
		_aes_encryption(0) {}
	virtual ~MINIZIP_WRITER() {
		close();
	}
	void close() {
		if (writer) {
			mz_zip_writer_close(writer);
			mz_zip_writer_delete(&writer);
			writer = nullptr;
		}
		_path.clear();
	}
	bool is_open()const { return writer != nullptr; }
	void open(const std::filesystem::path& path,
		bool append,	//true if adding to existing file
		int method,
		bool use_encryption,
		int aes_enc)
	{
		close();
		_path = path;
		_method = method;
		_flag = 0;

		writer = mz_zip_writer_create();
		if (!writer)RAISE_EXCEPTION(L"Failed to create zip writer");
		auto err = mz_zip_writer_open_file(writer, path.u8string().c_str(), 0, append);
		if (err != MZ_OK) {
			RAISE_EXCEPTION(L"Failed to open file %s: %s", path.c_str(), mzError2Text(err).c_str());
		}

		if (use_encryption) {
			_flag |= MZ_ZIP_FLAG_ENCRYPTED;
			_aes_encryption = aes_enc;
			mz_zip_writer_set_aes(writer, aes_enc);
			mz_zip_writer_set_password_cb(writer, _password_cb.get(), MINIZIP_PASSPHRASE_BASE::password_cb);
		}
	}

	struct DATA_BRIDGE{
		std::function<LF_BUFFER_INFO()> dataProvider;
		LF_BUFFER_INFO info;
		size_t written;
	};

	static int32_t read_cb(void* stream, void* buf, int32_t size) {
		auto bridge = (DATA_BRIDGE*)stream;
		while (true) {
			if (bridge->info.buffer && bridge->written < bridge->info.size) {
				auto toWrite = std::min(size, int32_t(bridge->info.size - bridge->written));
				memcpy(buf, (unsigned char*)bridge->info.buffer + bridge->written, toWrite);
				bridge->written += toWrite;
				return toWrite;
			} else {
				bridge->info = bridge->dataProvider();
				if (!bridge->info.buffer)return 0;
			}
		}
	}

	void add(const LF_ENTRY_STAT& stat, std::function<LF_BUFFER_INFO()> dataProvider) {
		LF_zip_file file_info;
		build_file_info(file_info, stat, _method, _flag, _aes_encryption);

		if (stat.is_directory()) {
			mz_zip_writer_add_info(writer, nullptr, nullptr, &file_info);
		} else {
			DATA_BRIDGE bridge = {};
			bridge.dataProvider = dataProvider;
			mz_zip_writer_add_info(writer, &bridge, read_cb, &file_info);
		}
	}
};

struct CLFArchiveZIP::INTERNAL {
	MINIZIP_READER _reader;
	MINIZIP_WRITER _writer;
	INTERNAL(std::shared_ptr<MINIZIP_PASSPHRASE_BASE> pcb):_reader(pcb), _writer(pcb) {  }
	virtual ~INTERNAL() { close(); }
	void close() {
		_reader.close();
		_writer.close();
	}

	void read_open(const std::filesystem::path& path) {
		close();
		_reader.open(path);
	}
	void write_open(
		const std::filesystem::path& path,
		bool append, LF_WRITE_OPTIONS options,
		const LF_COMPRESS_ARGS &args
	) {
		close();

		auto param = args.formats.zip.params;
		int method;
		{
			auto methodStr = toLower(param["compression"]);
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
			int level = atoi(param["level"].c_str());
			if (level < 0 || level>9) {
				RAISE_EXCEPTION(L"Invalid compression level: %s", UtilUTF8toUNICODE(param["level"]).c_str());
			}
		}
		int aes_enc = 0;
		{
			std::map<std::string, int> cryptoMap = {
				{"aes256", MZ_AES_STRENGTH_256},
				{"aes192", MZ_AES_STRENGTH_192},
				{"aes128", MZ_AES_STRENGTH_128},
				{"zipcrypto", 0},
			};
			auto cryptoStr = toLower(param["encryption"]);
			if (cryptoStr.empty()) {
				cryptoStr = "zipcrypto";
			}
			auto iter = cryptoMap.find(cryptoStr);
			if (cryptoMap.end() == iter) {
				RAISE_EXCEPTION(L"Invalid crypto name: %s", UtilUTF8toUNICODE(cryptoStr).c_str());
			} else {
				aes_enc = (*iter).second;
			}
		}

		bool use_encryption = ((options & LF_WOPT_DATA_ENCRYPTION) != 0);
		_writer.open(path, append, method, use_encryption, aes_enc);
	}
	bool is_read_mode()const {
		return _reader.is_open();
	}
};

CLFArchiveZIP::CLFArchiveZIP():_internal(nullptr)
{
}

CLFArchiveZIP::~CLFArchiveZIP()
{
	close();
}

void CLFArchiveZIP::read_open(const std::filesystem::path& file, std::shared_ptr<ILFPassphrase> passphrase)
{
	close();
	auto pcb = std::make_shared<MINIZIP_PASSPHRASE_BASE>(passphrase);
	_internal = std::make_shared<INTERNAL>(pcb);
	_internal->read_open(file);
	_path = file;
}

void CLFArchiveZIP::write_open(
	const std::filesystem::path& file,
	LF_ARCHIVE_FORMAT format,
	LF_WRITE_OPTIONS options,
	const LF_COMPRESS_ARGS& args,
	std::shared_ptr<ILFPassphrase> passphrase)
{
	close();
	auto pcb = std::make_shared<MINIZIP_PASSPHRASE_BASE>(passphrase);
	_internal = std::make_shared<INTERNAL>(pcb);
	_internal->write_open(file, false, options, args);
	_path = file;
}

void CLFArchiveZIP::close()
{
	if (_internal) {
		_internal->close();
		_internal.reset();
	}
	_path.clear();
}

bool CLFArchiveZIP::is_modify_supported()const
{
	return !isMultiPartZip(_path);
}

bool CLFArchiveZIP::contains_encryted_entry()
{
	for (auto ite = read_entry_begin(); ite; ite = read_entry_next()) {
		if (ite->is_encrypted) {
			read_entry_end();
			return true;
		}
	}
	return false;
}

static LF_ENTRY_STAT mz_to_LF_ENTRY_STAT(const mz_zip_file* entry)
{
	LF_ENTRY_STAT lfstat = {};

	uint32_t mode;
	mz_zip_attrib_convert(
		MZ_HOST_SYSTEM(entry->version_madeby),
		entry->external_fa,
		MZ_HOST_SYSTEM_UNIX,
		&mode);
	lfstat.stat.st_mode = mode;
	lfstat.stat.st_size = entry->uncompressed_size;
	lfstat.stat.st_atime = entry->accessed_date;
	lfstat.stat.st_mtime = entry->modified_date;
	lfstat.stat.st_ctime = entry->creation_date;

	lfstat.compressed_size = entry->compressed_size;
	if (entry->flag & MZ_ZIP_FLAG_UTF8) {
		lfstat.path = UtilUTF8toUNICODE(entry->filename, entry->filename_size);	//stored-as
	} else {
		//[Documentation bug?] mz_zip_file::filename is NOT utf-8!
		//_entry.path = UtilCP932toUNICODE(mzEntry->filename, mzEntry->filename_size);	//stored-as
		auto cp = UtilGuessCodepage(entry->filename, entry->filename_size);
		lfstat.path = UtilToUNICODE(entry->filename, entry->filename_size, cp);
	}
	lfstat.method_name = mzMethodName(entry->compression_method);
	lfstat.is_encrypted = entry->flag & MZ_ZIP_FLAG_ENCRYPTED;

	return lfstat;
};


LF_ENTRY_STAT* CLFArchiveZIP::read_entry_attrib()
{
	mz_zip_file* entry = _internal->_reader.get_entry_info();
	if (entry) {
		_entry = mz_to_LF_ENTRY_STAT(entry);
		return &_entry;
	} else {
		return nullptr;
	}
}

//make a copy, and returns in "write_open" state
std::unique_ptr<ILFArchiveFile> CLFArchiveZIP::make_copy_archive(
	const std::filesystem::path& dest_path,
	const LF_COMPRESS_ARGS& args,
	std::function<bool(const LF_ENTRY_STAT&)> false_to_skip)
{
	if (_internal->is_read_mode()) {
		auto keep_check = [&](mz_zip_file* entry)->bool {
			return false_to_skip(mz_to_LF_ENTRY_STAT(entry));
		};
		_internal->_reader.raw_copy_into_file(dest_path, keep_check);
		bool encrypted = _internal->_reader.is_any_encrypted();

		std::unique_ptr<CLFArchiveZIP> dest = std::make_unique<CLFArchiveZIP>();

		dest->close();
		dest->_internal = std::make_shared<INTERNAL>(_internal->_reader._password_cb);
		dest->_internal->write_open(dest_path, true, encrypted ? LF_WOPT_DATA_ENCRYPTION : LF_WOPT_STANDARD, args);
		dest->_path = dest_path;

		//- copy finished. now the caller can add extra files
		return dest;
	} else {
		throw ARCHIVE_EXCEPTION(EFAULT);
	}
}

LF_ENTRY_STAT* CLFArchiveZIP::read_entry_begin()
{
	if (_internal->_reader.rewind()) {
		return read_entry_attrib();
	} else {
		return nullptr;
	}
}

LF_ENTRY_STAT* CLFArchiveZIP::read_entry_next()
{
	if (_internal->_reader.next()) {
		return read_entry_attrib();
	} else {
		return nullptr;
	}
}

void CLFArchiveZIP::read_entry_end()
{
	_internal->_reader.rewind();
}

//read entry
void CLFArchiveZIP::read_file_entry_block(std::function<void(const void*, size_t, const offset_info*)> data_receiver)
{
	if (!_internal || !_internal->is_read_mode()) {
		RAISE_EXCEPTION(L"File is not opened");
	}

	_internal->_reader.read_entry(data_receiver);
}

//write entry
void CLFArchiveZIP::add_file_entry(const LF_ENTRY_STAT& stat, std::function<LF_BUFFER_INFO()> dataProvider)
{
	_internal->_writer.add(stat, dataProvider);
}

void CLFArchiveZIP::add_directory_entry(const LF_ENTRY_STAT& stat)
{
	add_file_entry(stat, []() {LF_BUFFER_INFO bi = { 0 }; return bi; });
}

#include "CommonUtil.h"
bool CLFArchiveZIP::is_known_format(const std::filesystem::path& arcname)
{
	//some zipx archives use methods that minizip-ng does not support
	if (toLower(arcname.extension()) == L".zipx")return false;

	CAutoFile fp;
	fp.open(arcname);
	if (!fp.is_opened())return false;
	const size_t bufSize = 64 * 1024;
	std::vector<unsigned char> header(bufSize);
	size_t read = fread(&header[0], 1, bufSize, fp);
	if (read < 4) {
		return false;
	}
	header.resize(read);
	//zip, zipx: https://en.wikipedia.org/wiki/ZIP_(file_format)
	//if (header[0] == 'P' && header[1] == 'K' && (
	//	(header[2] == 0x03 && header[3] == 0x04) ||
	//	(header[2] == 0x05 && header[3] == 0x06) ||
	//	(header[2] == 0x07 && header[3] == 0x08)
	//	)) {
	//	return true;
	//}

	//skipping self extracting archive header
	for (auto ite = header.begin(); ite != header.end(); ++ite) {
		ite = std::find(ite, header.end(), 'P');
		if (ite == header.end()) {
			break;
		} else {
			if (std::next(ite, 1) == header.end() || *std::next(ite, 1) != 'K')continue;
			if (std::next(ite, 2) == header.end() || std::next(ite, 3) == header.end())break;
			if (*std::next(ite, 2) == 0x03 && *std::next(ite, 3) == 0x04)return true;
			if (*std::next(ite, 2) == 0x05 && *std::next(ite, 3) == 0x06)return true;
			if (*std::next(ite, 2) == 0x07 && *std::next(ite, 3) == 0x08)return true;
		}
	}

	return false;
}

#ifdef UNIT_TEST
std::pair<int, int> CLFArchiveZIP::test_sub_get_encryption()const
{
	auto info = _internal->_reader.get_entry_info();
	return { info->aes_version,info->aes_strength };
}
#endif
