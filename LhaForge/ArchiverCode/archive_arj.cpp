#include "stdafx.h"
#include "archive_arj.h"
#include "mini_lzh_decoder.h"
#include "Utilities/utility.h"

//Arj decoder
// this code provides extract only
// this code is based on xacrett: http://www.kmonos.net/lib/xacrett.ja.html

static BYTE fget_byte(FILE* f)
{
	return getc(f);//    error(M_CANTREAD, "");
}

static WORD fget_word(FILE* f)
{
	WORD b0 = fget_byte(f);
	WORD b1 = fget_byte(f);
	return (b1 << 8) + b0;
}

static DWORD fget_longword(FILE* f)
{
	DWORD b0 = fget_byte(f);
	DWORD b1 = fget_byte(f);
	DWORD b2 = fget_byte(f);
	DWORD b3 = fget_byte(f);
	return (b3 << 24) + (b2 << 16) + (b1 << 8) + b0;
}


size_t CLFArchiveARJ::Header::find_header(const unsigned char* hdr, unsigned long siz)
{
	long last = (siz - 2 - HEADERSIZE_MAX > 50000 ?
		50000 : siz - 2 - HEADERSIZE_MAX);
	if (siz < HEADERSIZE_MAX + 2) {
		last = siz;
	}

	for (long i = 0; i < last; i++) {
		// Flag
		if (hdr[i] != HEADER_ID_LO)continue;
		if (hdr[i + 1] != HEADER_ID_HI)continue;
		// Header_size
		int hsiz = (hdr[i + 2] + (hdr[i + 3] << 8));
		if (hsiz > HEADERSIZE_MAX)continue;
		// Crc
		CRC crc;
		crc.update(hdr + (i + 4), hsiz);
		DWORD hcrc = (hdr[i + 4 + hsiz + 0]
			+ (hdr[i + 4 + hsiz + 1] << 8)
			+ (hdr[i + 4 + hsiz + 2] << 16)
			+ (hdr[i + 4 + hsiz + 3] << 24));
		if (crc.get_crc() == hcrc) {
			return i;
		}
	}
	return -1;
}

std::pair<bool, CLFArchiveARJ::Header> CLFArchiveARJ::Header::read_header(FILE* fp)
{
	WORD header_id = fget_word(fp);
	if (header_id != HEADER_ID)RAISE_EXCEPTION(L"Invalid header ID");

	Header h;
	h.h_Size = fget_word(fp);
	if (h.h_Size == 0)return std::make_pair(false, Header());//RAISE_EXCEPTION(L"Header is empty");
	if (h.h_Size > HEADERSIZE_MAX)RAISE_EXCEPTION(L"Header size too large");;

	BYTE header[HEADERSIZE_MAX], * ph = header;
	CRC crc;
	crc.fread_crc(header, h.h_Size, fp);
	h.h_CRC = fget_longword(fp);
	if (crc.get_crc() != h.h_CRC)RAISE_EXCEPTION(L"Header CRC Error");

	h.h_1stSize = *(ph++);
	h.h_ArjVer = *(ph++);
	h.h_ArjXVer = *(ph++);
	h.h_OS = *(ph++);
	h.h_Flags = *(ph++);
	h.h_Method = *(ph++);
	h.h_FileType = *(ph++);
	(ph++);
	h.h_TimeStamp = (ph[3] << 24) + (ph[2] << 16) + (ph[1] << 8) + ph[0];	ph += 4;
	h.h_Compsize = (ph[3] << 24) + (ph[2] << 16) + (ph[1] << 8) + ph[0];	ph += 4;
	h.h_Origsize = (ph[3] << 24) + (ph[2] << 16) + (ph[1] << 8) + ph[0];	ph += 4;
	h.h_FileCRC = (ph[3] << 24) + (ph[2] << 16) + (ph[1] << 8) + ph[0];	ph += 4;
	h.h_EntryPos = (ph[1] << 8) + ph[0];	ph += 2;
	h.h_Attrib = (ph[1] << 8) + ph[0];	ph += 2;;
	h.h_HostData = (ph[1] << 8) + ph[0];	ph += 2;;

	strncpy_s(h.h_Filename, (char*)&header[h.h_1stSize], ARJ_FNAME_MAX);
	if (h.h_OS != 0 && h.h_OS < 10) { //if file was not created on MSDOS nor WIN95
		//limit to ASCII
		char* pp = h.h_Filename;
		while (*pp) {
			*(pp++) &= 0x7f;
		}
	}

	// ignore extension header
	WORD extheadersize;
	while (extheadersize = fget_word(fp)) {
		_fseeki64(fp, extheadersize + 4, SEEK_CUR);
	}

	return std::make_pair(true, h);
}

/************************************************************************/

void CLFArchiveARJ::decode_f::initDecoder(FILE* fp, Header* hdr)
{
	_fp = fp;
	_header = hdr;

	init_getbits();
	getlen = getbuf = 0;
	count = 0;
}

void CLFArchiveARJ::decode_f::init_getbits()
{
	bitbuf = 0;
	subbitbuf = 0;
	bitcount = 0;
	fillbuf(16);
}

void CLFArchiveARJ::decode_f::fillbuf(int n)
{
	bitbuf = (bitbuf << n) & 0xFFFF;
	while (n > bitcount) {
		bitbuf |= subbitbuf << (n -= bitcount);
		if (_header->h_Compsize != 0) {
			_header->h_Compsize--;
			subbitbuf = (BYTE)getc(_fp);
		} else {
			subbitbuf = 0;
		}
		bitcount = 8;
	}
	bitbuf |= subbitbuf >> (bitcount -= n);
}

#define CODE_BIT 16
#define THRESHOLD    3
#define STRTP          9
#define STOPP         13
#define STRTL          0
#define STOPL          7

#define BFIL {getbuf|=bitbuf>>getlen;fillbuf(CODE_BIT-getlen);getlen=CODE_BIT;}
#define GETBIT(c) {if(getlen<=0)BFIL c=(getbuf&0x8000)!=0;getbuf<<=1;getlen--;}
#define BPUL(l) {getbuf<<=l;getlen-=l;}
#define GETBITS(c,l) {if(getlen<l)BFIL c=(WORD)getbuf>>(CODE_BIT-l);BPUL(l)}

const static size_t HEADER_SIZE_LIMIT = 1024 * 1024;

short CLFArchiveARJ::decode_f::decode_ptr()
{
	short c, width, plus, pwr;

	plus = 0;
	pwr = 1 << (STRTP);
	for (width = (STRTP); width < (STOPP); width++) {
		GETBIT(c);
		if (c == 0)
			break;
		plus += pwr;
		pwr <<= 1;
	}
	if (width != 0)
		GETBITS(c, width);
	c += plus;
	return c;
}

short CLFArchiveARJ::decode_f::decode_len()
{
	short c, width, plus, pwr;

	plus = 0;
	pwr = 1 << (STRTL);
	for (width = (STRTL); width < (STOPL); width++) {
		GETBIT(c);
		if (c == 0)break;
		plus += pwr;
		pwr <<= 1;
	}
	if (width != 0)GETBITS(c, width);
	c += plus;
	return c;
}

void CLFArchiveARJ::decode_f::decode(std::function<void(const void*, int64_t/*data size*/)> data_receiver)
{
	std::vector<BYTE> buf;
	buf.resize(ARJ_DICSIZE);
	short r = 0, j = 0;
	short i;
	while (count < _header->h_Origsize) {
		auto c = decode_len();
		if (c == 0) {
			GETBITS(c, 8);
			buf[r] = (BYTE)c;
			count++;
			if (++r >= ARJ_DICSIZE) {
				data_receiver(&buf[0], ARJ_DICSIZE);
				r = 0;
			}
		} else {
			j = c - 1 + THRESHOLD;
			auto pos = decode_ptr();
			if ((i = r - pos - 1) < 0)i += ARJ_DICSIZE;
			while (j-- > 0) {
				count++;
				buf[r] = buf[i];
				if (++i >= ARJ_DICSIZE) {
					i = 0;
				}
				if (++r >= ARJ_DICSIZE) {
					data_receiver(&buf[0], ARJ_DICSIZE);
					r = 0;
				}
			}
		}
	}
	if (r != 0) {
		data_receiver(&buf[0], r);
	}

	data_receiver(nullptr, 0);
}

//----

CLFArchiveARJ::CLFArchiveARJ()
{
	_header = std::make_shared<Header>();
	_lzh = std::make_shared<CLzhDecoder2>();
	close();
}

CLFArchiveARJ::~CLFArchiveARJ()
{
	close();
}

void CLFArchiveARJ::read_open(const std::filesystem::path& file, std::shared_ptr<ILFPassphrase>)
{
	close();

	_fp.open(file);
	if (!_fp.is_opened()) {
		RAISE_EXCEPTION(L"Failed to open file");
	}
}

void CLFArchiveARJ::close()
{
	_fp.close();
	_entry_stat = LF_ENTRY_STAT();
	read_entry_end();
}

LF_ENTRY_STAT* CLFArchiveARJ::read_entry_begin()
{
	read_entry_end();
	if (!_fp.is_opened()) {
		RAISE_EXCEPTION(L"File is not opened");
	}
	_fseeki64(_fp, 0, SEEK_SET);

	std::vector<BYTE> cReadBuffer;
	cReadBuffer.resize(HEADER_SIZE_LIMIT);
	auto read = fread(&cReadBuffer[0], 1, HEADER_SIZE_LIMIT, _fp);
	if (read == 0) {
		RAISE_EXCEPTION(L"Unexpected EOF");
	}
	auto offset = Header::find_header(&cReadBuffer[0], (unsigned long)read);
	if (-1 == offset) {
		RAISE_EXCEPTION(L"Broken header");
	}
	if (0 != _fseeki64(_fp, offset, SEEK_SET)) {
		RAISE_EXCEPTION(L"Incomplete file");
	}

	//skip dummy header entry
	//EOF check
	{
		char c = fgetc(_fp);
		if (feof(_fp)) {
			return nullptr;
		}
		ungetc(c, _fp);
	}
	try {
		Header::read_header(_fp);
		_nextEntryPos = _ftelli64(_fp);
	} catch (const LF_EXCEPTION e) {
		throw e;
	}

	return read_entry_next();
}

LF_ENTRY_STAT* CLFArchiveARJ::read_entry_next()
{
	if (_header) {
		if (0 != _fseeki64(_fp, _nextEntryPos, SEEK_SET)) {
			RAISE_EXCEPTION(L"Incomplete file");
		}
	}
	//EOF check
	{
		char c = fgetc(_fp);
		if (feof(_fp)) {
			return nullptr;
		}
		ungetc(c, _fp);
	}
	try {
		auto [isOK, hdr] = Header::read_header(_fp);
		if (!isOK)return nullptr;	//EOF
		*_header = hdr;
	} catch (const LF_EXCEPTION e) {
		throw e;
	}
	if (_header->validate_flags()) {
		_nextEntryPos = _ftelli64(_fp) + _header->h_Compsize;
	} else {
		//_fseeki64(_fp, _nextEntryPos, SEEK_SET);
		RAISE_EXCEPTION(L"Broken header");
	}

	_entry_stat.compressed_size = _header->h_Compsize;
	_entry_stat.stat.st_size = _header->h_Origsize;	// original file size

	FILETIME ft, lc;
	DosDateTimeToFileTime((WORD)(_header->h_TimeStamp >> 16), (WORD)_header->h_TimeStamp, &lc);
	LocalFileTimeToFileTime(&lc, &ft);
	_entry_stat.stat.st_mtime = UtilFileTimeToUnixTime(lc);
	_entry_stat.stat.st_atime = _entry_stat.stat.st_mtime;
	_entry_stat.stat.st_ctime = _entry_stat.stat.st_mtime;
	_entry_stat.compressed_size = _header->h_Compsize;

	_entry_stat.stat.st_mode = _S_IFREG;

	_entry_stat.method_name = Format(L"Method %d", _header->h_Method);

	size_t end = strlen(_header->h_Filename);
	end = std::min((size_t)MAX_PATH - 1, end);
	_entry_stat.path = UtilCP932toUNICODE(_header->h_Filename, end);

	_crc.reset();
	_lzh = std::make_shared<CLzhDecoder2>();
	switch (_header->h_Method) {
	case 0:
		_lzh->initDecoder(LH0, _fp, _header->h_Compsize, _header->h_Origsize);
		break;
	case 1:
	case 2:
	case 3:
		_lzh->initDecoder(ARJ, _fp, _header->h_Compsize, _header->h_Origsize);
		break;
	case 4:
	default:
		_decode_f.initDecoder(_fp, _header.get());
		break;
	}

	return &_entry_stat;
}

void CLFArchiveARJ::read_entry_end()
{
	if (_fp.is_opened()) {
		_fseeki64(_fp, 0, SEEK_SET);
	}
	_nextEntryPos = 0;
	_lzh.reset();
}

void CLFArchiveARJ::read_file_entry_block(std::function<void(const void*, size_t/*data size*/, const offset_info*)> data_receiver)
{
	auto wrapper = [&](const void* buffer, size_t/*data size*/ size) {
		if (buffer) {
			_crc.update_crc((const BYTE*)buffer, (int)size);
		} else {
			if (_crc.get_crc() != _header->h_FileCRC)RAISE_EXCEPTION(L"File CRC error");
		}
		data_receiver(buffer, size, nullptr);
	};

	switch (_header->h_Method) {
	case 0:
	case 1:
	case 2:
	case 3:
		_lzh->decode(wrapper);
		break;
	case 4:
	default:
		_decode_f.decode(wrapper);
		break;
	}
}

bool CLFArchiveARJ::is_known_format(const std::filesystem::path& arcname)
{
	try {
		auto buf = UtilReadFile(arcname, HEADER_SIZE_LIMIT);
		if (buf.empty())return false;
		return (-1 != Header::find_header(&buf[0], (unsigned long)buf.size()));
	} catch (const LF_EXCEPTION&) {
		return false;
	}
}

#ifdef UNIT_TEST
#include "CommonUtil.h"
TEST(CLFArchiveARJ, read_open_many)
{
	_wsetlocale(LC_ALL, L"");	//default locale
	{
		CLFArchiveARJ a;
		auto pp = std::make_shared<CLFPassphraseNULL>();
		a.read_open(LF_PROJECT_DIR() / L"test/test.arj", pp);
		EXPECT_FALSE(a.is_modify_supported());
		EXPECT_EQ(L"ARJ", a.get_format_name());
		auto entry = a.read_entry_begin();

		EXPECT_NE(nullptr, entry);
		EXPECT_EQ(L"test_2099/ccd.txt", entry->path.wstring());
		EXPECT_EQ(_S_IFREG, entry->stat.st_mode);
		EXPECT_FALSE(entry->is_directory());
		EXPECT_EQ(40, entry->compressed_size);
		EXPECT_EQ(44, entry->stat.st_size);
		//EXPECT_EQ(, entry->stat.st_mtime);
		EXPECT_EQ(L"Method 1", entry->method_name);

		{
			std::vector<char> tmp;
			for (bool bEOF = false; !bEOF;) {
				a.read_file_entry_block([&](const void* buf, size_t size, const offset_info* offset) {
					EXPECT_EQ(nullptr, offset);
					if (buf) {
						tmp.insert(tmp.end(), (const char*)buf, ((const char*)buf) + size);
					} else {
						bEOF = true;
					}
					});
			}
			const char* content = ";kljd;lfj;lsdahg;has:hn:h :ahsd:fh:asdhg:ioh";
			std::vector<char> tmp2(content, content + strlen(content));
			EXPECT_EQ(tmp2, tmp);
		}

		a.read_entry_end();
	}
}

TEST(CLFArchiveARJ, read_open_non_existing)
{
	_wsetlocale(LC_ALL, L"");	//default locale
	CLFArchiveARJ a;
	auto pp = std::make_shared<CLFPassphraseNULL>();
	EXPECT_THROW(a.read_open(LF_PROJECT_DIR() / L"test/non_existing.arj", pp), LF_EXCEPTION);
}

TEST(CLFArchiveARJ, enum_archive)
{
	_wsetlocale(LC_ALL, L"");	//default locale
	{
		CLFArchiveARJ a;
		auto pp = std::make_shared<CLFPassphraseNULL>();
		a.read_open(LF_PROJECT_DIR() / L"test/test.arj", pp);

		auto entry = a.read_entry_begin();
		int count = 0;
		for (; entry; entry = a.read_entry_next()) {
			EXPECT_TRUE(entry->path.wstring().find(L"test_2099/") == 0);
			count++;
		}
		EXPECT_EQ(2099, count);
	}
}


TEST(CLFArchiveARJ, read_open_method0)
{
	_wsetlocale(LC_ALL, L"");	//default locale
	CLFArchiveARJ a;
	auto pp = std::make_shared<CLFPassphraseNULL>();
	a.read_open(LF_PROJECT_DIR() / L"test/image_method0.arj", pp);
	auto entry = a.read_entry_begin();

	EXPECT_NE(nullptr, entry);
	EXPECT_EQ(L"image.png", entry->path.wstring());
	EXPECT_FALSE(entry->is_directory());
	EXPECT_EQ(31846, entry->compressed_size);
	EXPECT_EQ(31846, entry->stat.st_size);

	EXPECT_NO_THROW({
		std::vector<char> tmp;
		for (bool bEOF = false; !bEOF;) {
			a.read_file_entry_block([&](const void* buf, size_t size, const offset_info* offset) {
				EXPECT_EQ(nullptr, offset);
				if (buf) {
					tmp.insert(tmp.end(), (const char*)buf, ((const char*)buf) + size);
				} else {
					bEOF = true;
				}
			});
		}
		EXPECT_EQ(tmp.size(), entry->stat.st_size);
		});
}

TEST(CLFArchiveARJ, read_open_method1)
{
	_wsetlocale(LC_ALL, L"");	//default locale
	CLFArchiveARJ a;
	auto pp = std::make_shared<CLFPassphraseNULL>();
	a.read_open(LF_PROJECT_DIR() / L"test/image_method1.arj", pp);
	auto entry = a.read_entry_begin();

	EXPECT_NE(nullptr, entry);
	EXPECT_EQ(L"image.png", entry->path.wstring());
	EXPECT_FALSE(entry->is_directory());
	EXPECT_EQ(30174, entry->compressed_size);
	EXPECT_EQ(31846, entry->stat.st_size);

	EXPECT_NO_THROW({
		std::vector<char> tmp;
		for (bool bEOF = false; !bEOF;) {
			a.read_file_entry_block([&](const void* buf, int64_t size, const offset_info* offset) {
				EXPECT_EQ(nullptr, offset);
				if (buf) {
					tmp.insert(tmp.end(), (const char*)buf, ((const char*)buf) + size);
				} else {
					bEOF = true;
				}
			});
		}
		EXPECT_EQ(tmp.size(), entry->stat.st_size);
		});
}

TEST(CLFArchiveARJ, read_open_method2)
{
	_wsetlocale(LC_ALL, L"");	//default locale
	CLFArchiveARJ a;
	auto pp = std::make_shared<CLFPassphraseNULL>();
	a.read_open(LF_PROJECT_DIR() / L"test/image_method2.arj", pp);
	auto entry = a.read_entry_begin();

	EXPECT_NE(nullptr, entry);
	EXPECT_EQ(L"image.png", entry->path.wstring());
	EXPECT_FALSE(entry->is_directory());
	EXPECT_EQ(30181, entry->compressed_size);
	EXPECT_EQ(31846, entry->stat.st_size);

	EXPECT_NO_THROW({
		std::vector<char> tmp;
		for (bool bEOF = false; !bEOF;) {
			a.read_file_entry_block([&](const void* buf, int64_t size, const offset_info* offset) {
				EXPECT_EQ(nullptr, offset);
				if (buf) {
					tmp.insert(tmp.end(), (const char*)buf, ((const char*)buf) + size);
				} else {
					bEOF = true;
				}
			});
		}
		EXPECT_EQ(tmp.size(), entry->stat.st_size);
		});
}

TEST(CLFArchiveARJ, read_open_method3)
{
	_wsetlocale(LC_ALL, L"");	//default locale
	CLFArchiveARJ a;
	auto pp = std::make_shared<CLFPassphraseNULL>();
	a.read_open(LF_PROJECT_DIR() / L"test/image_method3.arj", pp);
	auto entry = a.read_entry_begin();

	EXPECT_NE(nullptr, entry);
	EXPECT_EQ(L"image.png", entry->path.wstring());
	EXPECT_FALSE(entry->is_directory());
	EXPECT_EQ(30220, entry->compressed_size);
	EXPECT_EQ(31846, entry->stat.st_size);

	EXPECT_NO_THROW({
		std::vector<char> tmp;
		for (bool bEOF = false; !bEOF;) {
			a.read_file_entry_block([&](const void* buf, int64_t size, const offset_info* offset) {
				EXPECT_EQ(nullptr, offset);
				if (buf) {
					tmp.insert(tmp.end(), (const char*)buf, ((const char*)buf) + size);
				} else {
					bEOF = true;
				}
			});
		}
		EXPECT_EQ(tmp.size(), entry->stat.st_size);
		});
}

TEST(CLFArchiveARJ, read_open_method4)
{
	_wsetlocale(LC_ALL, L"");	//default locale
	CLFArchiveARJ a;
	auto pp = std::make_shared<CLFPassphraseNULL>();
	a.read_open(LF_PROJECT_DIR() / L"test/image_method4.arj", pp);
	auto entry = a.read_entry_begin();

	EXPECT_NE(nullptr, entry);
	EXPECT_EQ(L"image.bmp", entry->path.wstring());
	EXPECT_FALSE(entry->is_directory());
	EXPECT_EQ(60415, entry->compressed_size);
	EXPECT_EQ(1638574, entry->stat.st_size);

	EXPECT_NO_THROW({
		std::vector<char> tmp;
		for (bool bEOF = false; !bEOF;) {
			a.read_file_entry_block([&](const void* buf, int64_t size, const offset_info* offset) {
				EXPECT_EQ(nullptr, offset);
				if (buf) {
					tmp.insert(tmp.end(), (const char*)buf, ((const char*)buf) + size);
				} else {
					bEOF = true;
				}
			});
		}
		EXPECT_EQ(tmp.size(), entry->stat.st_size);
		});
}

TEST(CLFArchiveARJ, broken_file)
{
	_wsetlocale(LC_ALL, L"");	//default locale
	{
		CLFArchiveARJ a;
		auto pp = std::make_shared<CLFPassphraseNULL>();
		a.read_open(LF_PROJECT_DIR() / L"test/test_broken_method0.arj", pp);
		auto entry = a.read_entry_begin();
		EXPECT_THROW({
			for (bool bEOF = false; !bEOF;) {
				a.read_file_entry_block([&](const void* buf, int64_t size, const offset_info* offset) {
					EXPECT_EQ(nullptr, offset);
					if (!buf) {
						bEOF = true;
					}
					});
			}
			}, LF_EXCEPTION);
	}
	{
		CLFArchiveARJ a;
		auto pp = std::make_shared<CLFPassphraseNULL>();
		a.read_open(LF_PROJECT_DIR() / L"test/test_broken_method1.arj", pp);
		auto entry = a.read_entry_begin();
		EXPECT_THROW({
			for (bool bEOF = false; !bEOF;) {
				a.read_file_entry_block([&](const void* buf, int64_t size, const offset_info* offset) {
					EXPECT_EQ(nullptr, offset);
					if (!buf) {
						bEOF = true;
					}
					});
			}
			}, LF_EXCEPTION);
	}
	{
		CLFArchiveARJ a;
		auto pp = std::make_shared<CLFPassphraseNULL>();
		a.read_open(LF_PROJECT_DIR() / L"test/test_broken_method4.arj", pp);
		auto entry = a.read_entry_begin();
		EXPECT_THROW({
			for (bool bEOF = false; !bEOF;) {
				a.read_file_entry_block([&](const void* buf, int64_t size, const offset_info* offset) {
					EXPECT_EQ(nullptr, offset);
					if (!buf) {
						bEOF = true;
					}
					});
			}
			}, LF_EXCEPTION);
	}
}

#endif
