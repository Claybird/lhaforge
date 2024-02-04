#include "stdafx.h"
#include "archive_bga.h"
#include "Utilities/Utility.h"

// this code is based on xacrett: http://www.kmonos.net/lib/xacrett.ja.html

const size_t HEADER_STUB_LIMIT = 65536;

struct CLFArchiveBGA::BgaHeader
{
	DWORD checksum;			// signed, sum of type-fname
	char  type[4];			// "GZIP" or "BZ2"
	DWORD compressed_size;	// compressed data size, except header
	DWORD original_size;	// original file size
	WORD  date;				// file modify date, DOS format
	WORD  time;				// file modify time, DOS format
	BYTE  attrib;			// file attribute
							// ( 1:RO 2:Hid 4:Sys 8:Vol 16:Dir 32:Arc )
	BYTE  header_type;		// header type; always 0, up to now
	WORD  arc_type;			// content compression type
							// 0: auto, 1: compressed, 2: incompressed
							// in auto mode, file with following extensions are incompressed
							//	.ARC, .ARJ, .BZ2, .BZA, .CAB, .GZ, .GZA, .LZH,
							//	.LZS, .PAK, .RAR, .TAZ, .TBZ, .TGZ, .Z, .ZIP, .ZOO
	WORD  dir_name_len;		//
	WORD  file_name_len;	//
	char  name[MAX_PATH];	// dir_name_len + file_name_len ( without '\0' )
};

//find header location
static size_t find_header(const unsigned char* dat, unsigned long siz)
{
	if (siz < 28)return -1;
	size_t limit = siz - 28;

	for (size_t i = 0; i < limit; i++) {
		if (dat[i + 4] != 'G' && dat[i + 4] != 'B') continue;
		if (dat[i + 5] != 'Z') continue;
		if (dat[i + 6] != 'I' && dat[i + 6] != '2') continue;
		if (dat[i + 7] != 'P' && dat[i + 7] != '\0') continue;

		DWORD checksum = (dat[i + 0]) + (dat[i + 1] << 8) + (dat[i + 2] << 16) + (dat[i + 3] << 24);
		size_t fnlen = (dat[i + 24]) + (dat[i + 25] << 8) + (dat[i + 26]) + (dat[i + 27] << 8);

		if (i + 28 + fnlen > siz)continue;

		int sum = 0;
		for (int j = i + 4; j != i + 28 + fnlen; j++) {
			sum += (char)dat[j];
		}
		if (checksum == sum) {
			return i;
		}
	}

	return -1;
}

CLFArchiveBGA::CLFArchiveBGA()
{
	close();
}

CLFArchiveBGA::~CLFArchiveBGA()
{
	close();
}


#include <zlib.h>
struct CLFArchiveBGA::DecoderGZ :public CLFArchiveBGA::Decoder
{
	std::array<unsigned char, 1024 * 1024> outbuf;
	int fd;
	gzFile gz;

	const int64_t total_size;
	int64_t offset;
	DecoderGZ(const std::shared_ptr<BgaHeader> header, FILE* fp) :total_size(header->original_size), offset(0) {
		fd = _dup(_fileno(fp));
		_lseek(fd, ftell(fp), SEEK_SET);
		fseek(fp, header->compressed_size, SEEK_CUR);

		gzFile gz = gzdopen(fd, "rb");
		if (!gz) {
			_close(fd);
			RAISE_EXCEPTION(L"Failed to init zlib decoder");
		}
	}
	virtual ~DecoderGZ() {
		gzclose(gz);
	}
	void decode(std::function<void(const void*, size_t/*data size*/)> data_receiver)override {
		if (total_size <= offset) {
			data_receiver(nullptr, 0);
		} else {
			auto toRead = std::min(total_size - offset, (int64_t)outbuf.size());
			auto read = gzread(gz, &outbuf[0], (unsigned int)toRead);

			if (read == -1) {
				RAISE_EXCEPTION(L"Unexpected error");
			}
			if (read != toRead) {
				RAISE_EXCEPTION(L"Unexpected EOF");
			}

			data_receiver(&outbuf[0], read);
			offset += read;
		}
	}
};

#include <bzlib.h>
struct CLFArchiveBGA::DecoderBZ2 :public CLFArchiveBGA::Decoder
{
	std::array<char, 4 * 1024 * 1024> outbuf;

	BZFILE* bz;
	const int64_t total_size;
	int64_t offset;
	DecoderBZ2(const std::shared_ptr<BgaHeader> header, FILE* fp) :total_size(header->original_size), offset(0) {
		fseek(fp, header->compressed_size, SEEK_CUR);
		int status;
		BZFILE* bz = BZ2_bzReadOpen(&status, fp, 0, 0, NULL, 0);
		if (status != BZ_OK || bz == NULL) {
			RAISE_EXCEPTION(L"Failed to init bzip2 decoder");
		}
	}
	virtual ~DecoderBZ2() {
		int status;
		BZ2_bzReadClose(&status, bz);
	}
	void decode(std::function<void(const void*, size_t/*data size*/)> data_receiver)override {
		if (total_size <= offset) {
			data_receiver(nullptr, 0);
		} else {
			auto toRead = std::min(total_size - offset, (int64_t)outbuf.size());
			int status;
			auto read = BZ2_bzRead(&status, bz, &outbuf[0], (int)toRead);

			if (status != BZ_OK) {
				RAISE_EXCEPTION(L"Unexpected error");
			}
			if (read != toRead) {
				RAISE_EXCEPTION(L"Unexpected EOF");
			}

			data_receiver(&outbuf[0], read);
			offset += read;
		}
	}
};

struct CLFArchiveBGA::DecoderRaw :public CLFArchiveBGA::Decoder
{
	std::array<char, 4 * 1024 * 1024> outbuf;

	const int64_t total_size;
	int64_t offset;

	FILE* _fp;
	DecoderRaw(const std::shared_ptr<BgaHeader> header, FILE* fp) : _fp(fp), total_size(header->original_size), offset(0) {}
	virtual ~DecoderRaw() {}
	void decode(std::function<void(const void*, size_t/*data size*/) > data_receiver)override {
		if (total_size <= offset) {
			data_receiver(nullptr, 0);
		} else {
			auto toRead = std::min(total_size - offset, (int64_t)outbuf.size());
			auto read = fread(&outbuf[0], 1, (unsigned int)toRead, _fp);

			if (read != toRead) {
				RAISE_EXCEPTION(L"Unexpected EOF");
			}

			data_receiver(&outbuf[0], read);
			offset += read;
		}
	}
};

void CLFArchiveBGA::read_file_entry_block(std::function<void(const void*, size_t/*data size*/, const offset_info* offset)> data_receiver)
{
	if (_decoder) {
		_decoder->decode([&](const void* buffer, size_t bufsize)->void {
			data_receiver(buffer, bufsize, nullptr);
			if (!buffer || bufsize == 0) {
				_decoder.reset();
			}
		});
	}
}


void CLFArchiveBGA::read_open(const std::filesystem::path& file, std::shared_ptr<ILFPassphrase> )
{
	close();
	_fp.open(file);
	if (!_fp.is_opened()) {
		RAISE_EXCEPTION(L"Failed to open file");
	}
}

void CLFArchiveBGA::close()
{
	_fp.close();
	_entry_stat = LF_ENTRY_STAT();

	read_entry_end();
}

LF_ENTRY_STAT* CLFArchiveBGA::read_entry_begin()
{
	read_entry_end();
	if (!_fp.is_opened()) {
		RAISE_EXCEPTION(L"File is not opened");
	}
	_fseeki64(_fp, 0, SEEK_SET);

	std::vector<BYTE> cReadBuffer;
	cReadBuffer.resize(HEADER_STUB_LIMIT);
	auto read = fread(&cReadBuffer[0], 1, HEADER_STUB_LIMIT, _fp);
	if (read == 0) {
		RAISE_EXCEPTION(L"Unexpected EOF");
	}
	auto offset = find_header(&cReadBuffer[0], read);
	if (-1 == offset) {
		RAISE_EXCEPTION(L"Broken header");
	}
	_current_entry_offset = offset;
	_fseeki64(_fp, offset, SEEK_SET);
	_header.reset();

	return read_entry_next();
}

std::wstring CLFArchiveBGA::get_method_name()const
{
	if (_header && isEntryCompressed()) {
		if (0 == memcmp(_header->type, "GZIP", 4)) {
			return L"GZIP";
		} else if (0 == memcmp(_header->type, "BZ2", 4)) {
			return L"BZ2";
		}
	} else {
		return L"Raw";
	}
	return L"---";
}

LF_ENTRY_STAT* CLFArchiveBGA::read_entry_next()
{
	if (!_fp.is_opened()) {
		RAISE_EXCEPTION(L"File is not opened");
	}
	_fseeki64(_fp, _current_entry_offset, SEEK_SET);
	if (!readHeader() || !_header) {
		if (feof(_fp)) {
			return nullptr;
		} else {
			RAISE_EXCEPTION(L"Broken header");
		}
	}
	_current_entry_offset = _ftelli64(_fp) + _header->compressed_size;

	_entry_stat.compressed_size = _header->compressed_size;
	_entry_stat.stat.st_size = _header->original_size;	// original file size

	FILETIME filetime;
	DosDateTimeToFileTime(_header->date, _header->time, &filetime);	//No time-zone information; possible tz is JST
	_entry_stat.stat.st_mtime = UtilFileTimeToUnixTime(filetime);
	_entry_stat.stat.st_atime = _entry_stat.stat.st_mtime;
	_entry_stat.stat.st_ctime = _entry_stat.stat.st_mtime;

	int st_mode = 0;
	//1:RO 2:Hid 4:Sys 8:Vol 16:Dir 32:Arc
	//if (_header->attrib & 1)st_mode |= 0;
	//if (_header->attrib & 2)st_mode |= 0;
	//if (_header->attrib & 4)st_mode |= 0;
	//if (_header->attrib & 8)st_mode |= 0;
	if (_header->attrib & 16) {
		st_mode |= _S_IFDIR;
	} else {
		st_mode |= _S_IFREG;
	}
	//if (_header->attrib & 32)st_mode |= 0;
	_entry_stat.stat.st_mode = st_mode;
	_entry_stat.method_name = get_method_name();

	int end = _header->dir_name_len + _header->file_name_len;
	end = std::min(MAX_PATH - 1, end);
	_entry_stat.path = UtilCP932toUNICODE(_header->name, end);

	//prepare decoder
	if (!isEntryCompressed()) {
		_decoder = std::make_shared<DecoderRaw>(_header, _fp);
	} else if (0 == memcmp(_header->type, "GZIP", 4)) {
		_decoder = std::make_shared<DecoderGZ>(_header, _fp);
	} else if (0 == memcmp(_header->type, "BZ2", 4)) {
		_decoder = std::make_shared<DecoderBZ2>(_header, _fp);
	} else {
		RAISE_EXCEPTION(L"Unknown method");
	}

	return &_entry_stat;
}

void CLFArchiveBGA::read_entry_end()
{
	_header.reset();
	_decoder.reset();
	if (_fp.is_opened()) {
		_fseeki64(_fp, 0, SEEK_SET);
	}
	_current_entry_offset = 0;
}

bool CLFArchiveBGA::is_known_format(const std::filesystem::path& arcname)
{
	try {
		auto buf = UtilReadFile(arcname, HEADER_STUB_LIMIT);
		if (buf.empty())return false;
		return (-1 != find_header(&buf[0], buf.size()));
	} catch (const LF_EXCEPTION&) {
		return false;
	}
}

bool CLFArchiveBGA::readHeader()
{
	_header = std::make_shared<BgaHeader>();
	unsigned char buf[sizeof(BgaHeader)] = {};
	//--------------------------------------------------------------------//

	if (28 != fread(buf, 1, 28, _fp)) {
		return false;
	}

	// make Byte-Order-Independent
	_header->checksum = (buf[0]) + (buf[1] << 8) +
		(buf[2] << 16) + (buf[3] << 24);
	memcpy(_header->type, buf + 4, 4);
	_header->compressed_size = (buf[8]) + (buf[9] << 8) +
		(buf[10] << 16) + (buf[11] << 24);
	_header->original_size = (buf[12]) + (buf[13] << 8) +
		(buf[14] << 16) + (buf[15] << 24);
	_header->date = (buf[16]) + (buf[17] << 8);
	_header->time = (buf[18]) + (buf[19] << 8);
	_header->attrib = (buf[20]);
	_header->header_type = (buf[21]);
	_header->arc_type = (buf[22]) + (buf[23] << 8);
	_header->dir_name_len = (buf[24]) + (buf[25] << 8);
	_header->file_name_len = (buf[26]) + (buf[27] << 8);

	// truncate filename that exceeds MAX_PATH
	DWORD fnlen = _header->dir_name_len + _header->file_name_len;
	if (fnlen != (unsigned)fread(buf + 28, 1, fnlen, _fp)) {
		return false;
	}
	int len = fnlen > MAX_PATH - 1 ? MAX_PATH - 1 : fnlen;
	memcpy(_header->name, buf + 28, len);
	_header->name[len] = '\0';

	// checksum
	int sum = 0;
	for (unsigned i = 4; i != 28 + fnlen; i++) {
		sum += (char)buf[i];
	}
	return (_header->checksum == (unsigned)sum);
}

bool CLFArchiveBGA::isEntryCompressed()const
{
	if (!_header)RAISE_EXCEPTION(L"File not opened");
	if (_header->arc_type == 2) {	//incompressed
		return false;
	}else if (_header->arc_type == 0) {	//auto
		if (_header->compressed_size == _header->original_size) {
			auto fname = UtilCP932toUNICODE(_header->name, MAX_PATH);
			auto ext = toLower(std::filesystem::path(fname).extension());

			const wchar_t* incompressed_exts[] = {
				L".arc",
				L".arj",
				L".bz2",
				L".bza",
				L".cab",
				L".gz" ,
				L".gza",
				L".lzh",
				L".lzs",
				L".pak",
				L".rar",
				L".taz",
				L".tbz",
				L".tgz",
				L".z"  ,
				L".zip",
				L".zoo",
			};
			for (const auto ie : incompressed_exts) {
				if (ext == ie)return false;
			}
		}
	}

	return true;
}

#ifdef UNIT_TEST
#include "CommonUtil.h"
TEST(CLFArchiveBGA, scan_bza)
{
	_wsetlocale(LC_ALL, L"");	//default locale
	{
		CLFArchiveBGA a;
		auto pp = std::make_shared<CLFPassphraseNULL>();
		a.read_open(LF_PROJECT_DIR() / L"test/test.bza", pp);
		auto entry = a.read_entry_begin();
		EXPECT_NE(nullptr, entry);
		EXPECT_EQ(L"dir\\empty\\", entry->path.wstring());
		EXPECT_TRUE(entry->stat.st_mode & _S_IFDIR);

		entry = a.read_entry_next();
		EXPECT_NE(nullptr, entry);
		EXPECT_EQ(L"dir\\test.txt", entry->path.wstring());
		EXPECT_EQ(17, entry->compressed_size);
		EXPECT_EQ(17, entry->stat.st_size);
		EXPECT_EQ(L"Raw", entry->method_name);

		entry = a.read_entry_next();
		EXPECT_NE(nullptr, entry);
		EXPECT_EQ(L"dir\\テスト.txt", entry->path.wstring());
		EXPECT_EQ(17, entry->compressed_size);
		EXPECT_EQ(17, entry->stat.st_size);
		EXPECT_EQ(L"Raw", entry->method_name);
	}
}

TEST(CLFArchiveBGA, read_bza)
{
	_wsetlocale(LC_ALL, L"");	//default locale
	{
		CLFArchiveBGA a;
		auto pp = std::make_shared<CLFPassphraseNULL>();
		a.read_open(LF_PROJECT_DIR() / L"test/test.bza", pp);
		auto entry = a.read_entry_begin();
		EXPECT_NE(nullptr, entry);
		EXPECT_EQ(L"dir\\empty\\", entry->path.wstring());
		EXPECT_TRUE(entry->stat.st_mode & _S_IFDIR);

		entry = a.read_entry_next();
		EXPECT_NE(nullptr, entry);
		EXPECT_EQ(L"dir\\test.txt", entry->path.wstring());
		EXPECT_EQ(17, entry->compressed_size);
		EXPECT_EQ(17, entry->stat.st_size);
		EXPECT_EQ(L"Raw", entry->method_name);

		{
			std::vector<BYTE> tmp;
			for (;;) {
				bool bEOF = false;
				a.read_file_entry_block([&](const void* buf, size_t data_size, const offset_info* offset) {
					EXPECT_EQ(nullptr, offset);
					if (buf) {
						tmp.insert(tmp.end(), (const BYTE*)buf, ((const BYTE*)buf) + data_size);
					} else {
						bEOF = true;
					}
				});
				if (bEOF) {
					break;
				}
			}
			std::vector<BYTE> tmp2 = { 0xE3, 0x81, 0x82, 0xE3, 0x81, 0x84, 0xE3, 0x81, 0x86, 0xE3, 0x81, 0x88, 0xE3, 0x81, 0x8A, 0x0D, 0x0A };
			EXPECT_EQ(tmp2, tmp);
		}

		entry = a.read_entry_next();
		EXPECT_NE(nullptr, entry);
		EXPECT_EQ(L"dir\\テスト.txt", entry->path.wstring());
		EXPECT_EQ(17, entry->compressed_size);
		EXPECT_EQ(17, entry->stat.st_size);
		EXPECT_EQ(L"Raw", entry->method_name);
		{
			std::vector<BYTE> tmp;
			for (;;) {
				bool bEOF = false;
				a.read_file_entry_block([&](const void* buf, int64_t data_size, const offset_info* offset) {
					EXPECT_EQ(nullptr, offset);
					if (buf) {
						tmp.insert(tmp.end(), (const BYTE*)buf, ((const BYTE*)buf) + data_size);
					} else {
						bEOF = true;
					}
				});
				if (bEOF) {
					break;
				}
			}
			std::vector<BYTE> tmp2 = { 0xE3, 0x81, 0x8B, 0xE3, 0x81, 0x8D, 0xE3, 0x81, 0x8F, 0xE3, 0x81, 0x91, 0xE3, 0x81, 0x93, 0x0D, 0x0A };
			EXPECT_EQ(tmp2, tmp);
		}
	}
}

TEST(CLFArchiveBGA, read_gza)
{
	_wsetlocale(LC_ALL, L"");	//default locale
	{
		CLFArchiveBGA a;
		auto pp = std::make_shared<CLFPassphraseNULL>();
		a.read_open(LF_PROJECT_DIR() / L"test/test.gza", pp);
		auto entry = a.read_entry_begin();
		EXPECT_NE(nullptr, entry);
		EXPECT_EQ(L"dir\\empty\\", entry->path.wstring());
		EXPECT_TRUE(entry->stat.st_mode & _S_IFDIR);

		entry = a.read_entry_next();
		EXPECT_NE(nullptr, entry);
		EXPECT_EQ(L"dir\\test.txt", entry->path.wstring());
		EXPECT_EQ(17, entry->compressed_size);
		EXPECT_EQ(17, entry->stat.st_size);
		EXPECT_EQ(L"Raw", entry->method_name);
		{
			std::vector<BYTE> tmp;
			for (;;) {
				bool bEOF = false;
				a.read_file_entry_block([&](const void* buf, int64_t data_size, const offset_info* offset) {
					EXPECT_EQ(nullptr, offset);
					if (buf) {
						tmp.insert(tmp.end(), (const BYTE*)buf, ((const BYTE*)buf) + data_size);
					} else {
						bEOF = true;
					}
				});
				if (bEOF) {
					break;
				}
			}
			std::vector<BYTE> tmp2 = { 0xE3, 0x81, 0x82, 0xE3, 0x81, 0x84, 0xE3, 0x81, 0x86, 0xE3, 0x81, 0x88, 0xE3, 0x81, 0x8A, 0x0D, 0x0A };
			EXPECT_EQ(tmp2, tmp);
		}

		entry = a.read_entry_next();
		EXPECT_NE(nullptr, entry);
		EXPECT_EQ(L"dir\\テスト.txt", entry->path.wstring());
		EXPECT_EQ(17, entry->compressed_size);
		EXPECT_EQ(17, entry->stat.st_size);
		EXPECT_EQ(L"Raw", entry->method_name);
		{
			std::vector<BYTE> tmp;
			for (;;) {
				bool bEOF = false;
				a.read_file_entry_block([&](const void* buf, int64_t data_size, const offset_info* offset) {
					EXPECT_EQ(nullptr, offset);
					if (buf) {
						tmp.insert(tmp.end(), (const BYTE*)buf, ((const BYTE*)buf) + data_size);
					} else {
						bEOF = true;
					}
				});
				if (bEOF) {
					break;
				}
			}
			std::vector<BYTE> tmp2 = { 0xE3, 0x81, 0x8B, 0xE3, 0x81, 0x8D, 0xE3, 0x81, 0x8F, 0xE3, 0x81, 0x91, 0xE3, 0x81, 0x93, 0x0D, 0x0A };
			EXPECT_EQ(tmp2, tmp);
		}
	}
}

TEST(CLFArchiveBGA, read_bza_sfx)
{
	_wsetlocale(LC_ALL, L"");	//default locale
	{
		CLFArchiveBGA a;
		auto pp = std::make_shared<CLFPassphraseNULL>();
		a.read_open(LF_PROJECT_DIR() / L"test/test_bza_exe.dat", pp);
		auto entry = a.read_entry_begin();
		EXPECT_NE(nullptr, entry);
		EXPECT_EQ(L"dir\\empty\\", entry->path.wstring());
		EXPECT_TRUE(entry->stat.st_mode & _S_IFDIR);

		entry = a.read_entry_next();
		EXPECT_NE(nullptr, entry);
		EXPECT_EQ(L"dir\\test.txt", entry->path.wstring());
		EXPECT_EQ(17, entry->compressed_size);
		EXPECT_EQ(17, entry->stat.st_size);
		EXPECT_EQ(L"Raw", entry->method_name);
		{
			std::vector<BYTE> tmp;
			for (bool bEOF = false; !bEOF;) {
				a.read_file_entry_block([&](const void* buf, int64_t data_size, const offset_info* offset) {
					EXPECT_EQ(nullptr, offset);
					if (buf) {
						tmp.insert(tmp.end(), (const BYTE*)buf, ((const BYTE*)buf) + data_size);
					} else {
						bEOF = true;
					}
				});
			}
			std::vector<BYTE> tmp2 = { 0xE3, 0x81, 0x82, 0xE3, 0x81, 0x84, 0xE3, 0x81, 0x86, 0xE3, 0x81, 0x88, 0xE3, 0x81, 0x8A, 0x0D, 0x0A };
			EXPECT_EQ(tmp2, tmp);
		}

		entry = a.read_entry_next();
		EXPECT_NE(nullptr, entry);
		EXPECT_EQ(L"dir\\テスト.txt", entry->path.wstring());
		EXPECT_EQ(17, entry->compressed_size);
		EXPECT_EQ(17, entry->stat.st_size);
		EXPECT_EQ(L"Raw", entry->method_name);
		{
			std::vector<BYTE> tmp;
			for (bool bEOF = false; !bEOF;) {
				a.read_file_entry_block([&](const void* buf, int64_t data_size, const offset_info* offset) {
					EXPECT_EQ(nullptr, offset);
					if (buf) {
						tmp.insert(tmp.end(), (const BYTE*)buf, ((const BYTE*)buf) + data_size);
					} else {
						bEOF = true;
					}
				});
			}
			std::vector<BYTE> tmp2 = { 0xE3, 0x81, 0x8B, 0xE3, 0x81, 0x8D, 0xE3, 0x81, 0x8F, 0xE3, 0x81, 0x91, 0xE3, 0x81, 0x93, 0x0D, 0x0A };
			EXPECT_EQ(tmp2, tmp);
		}
	}
}
#endif
