#include "stdafx.h"
#include "../archive_bga.h"
#include "Utilities/Utility.h"


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
