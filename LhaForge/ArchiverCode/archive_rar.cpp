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

#include "stdafx.h"
#include "archive_rar.h"
#include "Utilities/Utility.h"
#undef WINVER	//to avoid compiler warning
#undef _WIN32_WINNT	//to avoid compiler warning
#define RARDLL
#include <unrar/rar.hpp>
#include <unrar/dll.hpp>

std::wstring rarErrMsg(int code)
{
	switch (code) {
	case ERAR_SUCCESS:
		return L"Success";
	case ERAR_END_ARCHIVE:
		return L"End archive";
	case ERAR_NO_MEMORY:
		return L"No memory";
	case ERAR_BAD_DATA:
		return L"Bad data";
	case ERAR_BAD_ARCHIVE:
		return L"Bad archive";
	case ERAR_UNKNOWN_FORMAT:
		return L"Unknown format";
	case ERAR_EOPEN:
		return L"Error opening file";
	case ERAR_ECREATE:
		return L"Error creating file";
	case ERAR_ECLOSE:
		return L"Error closing file";
	case ERAR_EREAD:
		return L"Error reading file";
	case ERAR_EWRITE:
		return L"Error writing file";
	case ERAR_SMALL_BUF:
		return L"Buffer too small";
	case ERAR_MISSING_PASSWORD:
		return L"Missing password";
	case ERAR_EREFERENCE:
		return L"Error reference";
	case ERAR_BAD_PASSWORD:
		return L"Bad password";
	case ERAR_UNKNOWN:
	default:
		return L"Unknown error";
	}
}

std::wstring rarMethod(int method)
{
	switch (method) {
	case 0x30:
		return L"storing";
	case 0x31:
		return L"fastest";
	case 0x32:
		return L"fast";
	case 0x33:
		return L"normal";
	case 0x34:
		return L"good";
	case 0x35:
		return L"best";
	default:
		return L"unknown";
	}
}

struct CLFArchiveRAR::INTERNAL
{
	std::filesystem::path path;
	std::shared_ptr<ILFPassphrase> passphrase_callback;
	std::function<void(const void*, size_t/*data size*/, const offset_info*)> data_receiver;

	LF_ENTRY_STAT _entry;
	HANDLE arc;

	bool bEntryRead;

	INTERNAL():arc(NULL), bEntryRead(false){}
	virtual ~INTERNAL() {
		close();
	}

	static int RarErrorToDll(RAR_EXIT ErrCode){
		switch (ErrCode) {
		case RARX_FATAL:
		case RARX_READ:
			return ERAR_EREAD;
		case RARX_CRC:
			return ERAR_BAD_DATA;
		case RARX_WRITE:
			return ERAR_EWRITE;
		case RARX_OPEN:
			return ERAR_EOPEN;
		case RARX_CREATE:
			return ERAR_ECREATE;
		case RARX_MEMORY:
			return ERAR_NO_MEMORY;
		case RARX_BADPWD:
			return ERAR_BAD_PASSWORD;
		case RARX_SUCCESS:
			return ERAR_SUCCESS; // 0.
		default:
			return ERAR_UNKNOWN;
		}
	}

	void close() {
		if (arc) {
			RARCloseArchive(arc);
		}
		arc = NULL;
	}
	void rewind() {
		close();
		RAROpenArchiveDataEx d = {};
		d.Callback = rar_event_handler;
		d.UserData = (LPARAM)this;

		d.ArcNameW = (wchar_t*)path.c_str();
		d.OpenMode = RAR_OM_EXTRACT;
		arc = RAROpenArchiveEx(&d);
		if (!arc) {
			throw LF_EXCEPTION(rarErrMsg(ERAR_EOPEN));
		}
		bEntryRead = false;
	}
	bool isOpened()const {
		return arc != NULL;
	}
	void open(const std::filesystem::path& file, std::shared_ptr<ILFPassphrase> passphrase) {
		passphrase_callback = passphrase;
		path = file;
		rewind();
	}
	LF_ENTRY_STAT* scanNext() {
		if (!bEntryRead) {
			int ret = skipEntryData();
			if (ret != ERAR_SUCCESS) {
				throw LF_EXCEPTION(rarErrMsg(ret));
			}
		}
		bEntryRead = false;

		RARHeaderDataEx data = {};
		int ret = RARReadHeaderEx(arc, &data);
		if (ERAR_END_ARCHIVE == ret) {
			return nullptr;
		}else if (ret != ERAR_SUCCESS) {
			throw LF_EXCEPTION(rarErrMsg(ret));
		}
		_entry.compressed_size = data.PackSize + (((unsigned __int64)data.PackSizeHigh) << 32);
		_entry.stat.st_size = data.UnpSize + (((unsigned __int64)data.UnpSizeHigh) << 32);

		_entry.path = data.FileNameW;
		_entry.method_name = rarMethod(data.Method);

		_entry.stat.st_mtime = UtilFileTimeToUnixTime({ data.MtimeLow, data.MtimeHigh });
		_entry.stat.st_atime = UtilFileTimeToUnixTime({ data.AtimeLow, data.AtimeHigh });
		_entry.stat.st_ctime = UtilFileTimeToUnixTime({ data.CtimeLow, data.CtimeHigh });

		_entry.stat.st_mode = S_IFREG;
		if (data.Flags & RHDF_DIRECTORY) _entry.stat.st_mode = S_IFDIR;

		_entry.is_encrypted = data.Flags & RHDF_ENCRYPTED;

		return &_entry;
	}
	int skipEntryData() {
		if (bEntryRead) {
			RAISE_EXCEPTION(L"Entry is already read. Need to Proceed to next entry.");
		}
		bEntryRead = true;
		return RARProcessFileW(arc, RAR_SKIP, nullptr, nullptr);
	}
	int readEntryContent(std::function<void(const void*, size_t/*data size*/, const offset_info*)> receiver) {
		if (bEntryRead) {
			RAISE_EXCEPTION(L"Entry is already read. Need to Proceed to next entry.");
		}
		data_receiver = receiver;
		bEntryRead = true;
		return RARProcessFileW(arc, RAR_EXTRACT, nullptr, nullptr);
	}

	static int CALLBACK rar_event_handler(UINT msg, LPARAM UserData, LPARAM P1, LPARAM P2) {
		INTERNAL* p = (INTERNAL*)UserData;
		switch (msg) {
		case UCM_CHANGEVOLUMEW:
			if (P2 == RAR_VOL_ASK) {
				//Need to ask user for new file path
				//TODO: not implemented
				return -1;
			}
			return 0;
		case UCM_NEEDPASSWORDW:
		if(p->passphrase_callback){
			auto cb = p->passphrase_callback.get();
			auto pwdA = cb->operator()();
			if (pwdA) {
				//got some password input
				wcsncpy_s((wchar_t*)P1, P2, cb->raw.c_str(), P2);
				return 0;
			} else {
				return -1;
			}
		} else {
			return -1;
		}
		case UCM_PROCESSDATA:
			p->data_receiver((const void*)P1, (size_t)P2, nullptr);
			return 0;

		//---implement wide version only
		case UCM_NEEDPASSWORD:
		case UCM_CHANGEVOLUME:
			return 0;
		}
		return 0;
	}
};


CLFArchiveRAR::CLFArchiveRAR()
{
	_internal = new INTERNAL;
}

CLFArchiveRAR::~CLFArchiveRAR()
{
	delete _internal;
}

void CLFArchiveRAR::read_open(const std::filesystem::path& file, std::shared_ptr<ILFPassphrase> passphrase)
{
	_internal->open(file, passphrase);
}

void CLFArchiveRAR::close()
{
	_internal->close();
}

//archive property
std::wstring CLFArchiveRAR::get_format_name()
{
	return L"RAR";
}

//entry seek; returns null if it reached EOF
LF_ENTRY_STAT* CLFArchiveRAR::read_entry_begin()
{
	_internal->rewind();
	return read_entry_next();
}

LF_ENTRY_STAT* CLFArchiveRAR::read_entry_next()
{
	return _internal->scanNext();
}

void CLFArchiveRAR::read_entry_end()
{
	//do nothing
}

//read entry
void CLFArchiveRAR::read_file_entry_block(std::function<void(const void*, size_t/*data size*/, const offset_info*)> data_receiver)
{
	int ret = _internal->readEntryContent(data_receiver);
	if (ret != ERAR_SUCCESS) {
		throw LF_EXCEPTION(rarErrMsg(ret));
	}
	data_receiver(nullptr, 0, nullptr);	//tell end of archive
}

#include "CommonUtil.h"
bool CLFArchiveRAR::is_known_format(const std::filesystem::path& arcname)
{
	//passphrase callback shoud be called if RAR header is encrypted.
	struct CLFPassphraseRunCheck :public ILFPassphrase {
		bool called;
		CLFPassphraseRunCheck() :called(false) {}
		virtual ~CLFPassphraseRunCheck() {}
		const char* operator()()override { called = true; return nullptr; }
	};

	auto passphrase = std::make_shared<CLFPassphraseRunCheck>();
	try {
		CLFArchiveRAR a;

		a.read_open(arcname, passphrase);
		return true;
	} catch (...) {
		return passphrase.get()->called;
	}
}



#ifdef UNIT_TEST
TEST(CLFArchiveRAR, is_known_format)
{
	{
		const auto dir = LF_PROJECT_DIR() / L"ArchiverCode/test";
		EXPECT_FALSE(CLFArchiveRAR::is_known_format(dir / L"empty.gz"));
		EXPECT_FALSE(CLFArchiveRAR::is_known_format(dir / L"empty.bz2"));
		EXPECT_FALSE(CLFArchiveRAR::is_known_format(dir / L"empty.xz"));
		EXPECT_FALSE(CLFArchiveRAR::is_known_format(dir / L"empty.lzma"));
		EXPECT_FALSE(CLFArchiveRAR::is_known_format(dir / L"empty.zst"));

		EXPECT_FALSE(CLFArchiveRAR::is_known_format(dir / L"abcde.gz"));
		EXPECT_FALSE(CLFArchiveRAR::is_known_format(dir / L"abcde.bz2"));
		EXPECT_FALSE(CLFArchiveRAR::is_known_format(dir / L"abcde.xz"));
		EXPECT_FALSE(CLFArchiveRAR::is_known_format(dir / L"abcde.lzma"));
		EXPECT_FALSE(CLFArchiveRAR::is_known_format(dir / L"abcde.zst"));

		EXPECT_FALSE(CLFArchiveRAR::is_known_format(__FILEW__));
		EXPECT_FALSE(CLFArchiveRAR::is_known_format(L"some_non_existing_file"));
	}
	{
		const auto dir = LF_PROJECT_DIR() / L"test";
		EXPECT_FALSE(CLFArchiveRAR::is_known_format(dir / L"test_broken_file.zip"));
		EXPECT_FALSE(CLFArchiveRAR::is_known_format(dir / L"test_broken_crc.zip"));
		EXPECT_FALSE(CLFArchiveRAR::is_known_format(dir / L"test_extract.zip"));
		EXPECT_FALSE(CLFArchiveRAR::is_known_format(dir / L"test_extract.zipx"));
		EXPECT_FALSE(CLFArchiveRAR::is_known_format(dir / L"test_password_abcde.zip"));
		EXPECT_FALSE(CLFArchiveRAR::is_known_format(dir / L"test_unicode_control.zip"));
		EXPECT_FALSE(CLFArchiveRAR::is_known_format(dir / L"test_zip_sfx.dat"));

		EXPECT_TRUE(CLFArchiveRAR::is_known_format(dir / L"smile_encrypted.rar"));
		EXPECT_TRUE(CLFArchiveRAR::is_known_format(dir / L"smile_header_encrypted.rar"));
		EXPECT_TRUE(CLFArchiveRAR::is_known_format(dir / L"smile_locked.rar"));
		EXPECT_TRUE(CLFArchiveRAR::is_known_format(dir / L"smile_solid.rar"));
		EXPECT_TRUE(CLFArchiveRAR::is_known_format(dir / L"smile.part0001.rar"));
		EXPECT_TRUE(CLFArchiveRAR::is_known_format(dir / L"smile.part0002.rar"));
	}
}

TEST(CLFArchiveRAR, read_enum)
{
	_wsetlocale(LC_ALL, L"");	//default locale
	const auto dir = LF_PROJECT_DIR() / L"test";

	CLFArchiveRAR a;
	auto pp = std::make_shared<CLFPassphraseNULL>();
	a.read_open(dir / L"smile_solid.rar", pp);
	EXPECT_FALSE(a.is_modify_supported());
	EXPECT_EQ(L"RAR", a.get_format_name());

	auto entry = a.read_entry_begin();
	EXPECT_NE(nullptr, entry);
	EXPECT_EQ(entry->path.wstring(), L"smile.bmp");
	EXPECT_FALSE(entry->is_directory());
	EXPECT_EQ(L"normal", entry->method_name);

	entry = a.read_entry_next();
	EXPECT_EQ(nullptr, entry);
}

TEST(CLFArchiveRAR, read_enum_2099)
{
	_wsetlocale(LC_ALL, L"");	//default locale
	const auto file = std::filesystem::path(__FILEW__).parent_path() / L"test/test_2099.rar";

	CLFArchiveRAR a;
	auto pp = std::make_shared<CLFPassphraseNULL>();
	a.read_open(file, pp);
	EXPECT_FALSE(a.is_modify_supported());
	EXPECT_EQ(L"RAR", a.get_format_name());

	
	int count = 0;
	int numDir = 0;
	for (auto entry = a.read_entry_begin(); entry; entry = a.read_entry_next()) {
		count++;
		if (entry->is_directory()) {
			numDir++;
		} else {
			if (entry->path.wstring().find(L"ccd.txt") != -1) {
				EXPECT_EQ(entry->stat.st_size, 44);
				EXPECT_EQ(entry->method_name, L"normal");
				EXPECT_EQ(entry->compressed_size, 44);
				std::vector<char> data;
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
				EXPECT_EQ(std::string(data.begin(),data.end()), ";kljd;lfj;lsdahg;has:hn:h :ahsd:fh:asdhg:ioh");
			} else {
				EXPECT_EQ(entry->stat.st_size, 48);
				EXPECT_EQ(entry->method_name, L"storing");
				EXPECT_EQ(entry->compressed_size, 48);
			}
		}
		EXPECT_FALSE(entry->is_encrypted);
	}
	EXPECT_EQ(count, 2099 + 1);
	EXPECT_EQ(numDir, 1);
}


void sub_rar_test(std::filesystem::path file)
{
	CLFArchiveRAR a;
	EXPECT_TRUE(a.is_known_format(file));

	auto pp = std::make_shared<CLFPassphraseConst>(L"password");
	a.read_open(file, pp);
	EXPECT_FALSE(a.is_modify_supported());
	EXPECT_EQ(L"RAR", a.get_format_name());
	auto entry = a.read_entry_begin();
	EXPECT_NE(nullptr, entry);
	EXPECT_EQ(entry->path.wstring(), L"smile.bmp");
	EXPECT_FALSE(entry->is_directory());
	EXPECT_EQ(L"normal", entry->method_name);
	EXPECT_EQ(6110262, entry->stat.st_size);
	std::vector<char> data;
	for (;;) {
		bool bEOF = false;
		a.read_file_entry_block([&](const void* buf, size_t data_size, const offset_info* offset) {
			if (offset) {
				data.resize(offset->offset);
			}
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
}

TEST(CLFArchiveRAR, extract_rar_solid)
{
	sub_rar_test(LF_PROJECT_DIR() / L"test/smile_solid.rar");
}

TEST(CLFArchiveRAR, extract_rar_locked)
{
	sub_rar_test(LF_PROJECT_DIR() / L"test/smile_locked.rar");
}

TEST(CLFArchiveRAR, extract_rar_encrypted)
{
	sub_rar_test(LF_PROJECT_DIR() / L"test/smile_encrypted.rar");
}

TEST(CLFArchiveRAR, extract_rar_header_encrypted)
{
	sub_rar_test(LF_PROJECT_DIR() / L"test/smile_header_encrypted.rar");
}

TEST(CLFArchiveRAR, extract_rar_multipart)
{
	sub_rar_test(LF_PROJECT_DIR() / L"test/smile.part0001.rar");
}

#endif
