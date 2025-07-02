#include "stdafx.h"
#include "../archive_zip.h"
#define HAVE_ZLIB 1
#define ZLIB_COMPAT 1
#define HAVE_BZIP2 1
#define HAVE_LZMA 1
#define LZMA_API_STATIC 1
#define HAVE_ZSTD 1
#define HAVE_PKCRYPT 1
#define HAVE_WZAES 1

#include "mz.h"
#include "mz_zip.h"
#include "mz_strm.h"
#include "mz_strm_os.h"
#include "mz_os.h"
#include "compress.h"
#include "extract.h"
#include "CommonUtil.h"

TEST(CLFArchiveZIP, read_enum)
{
	_wsetlocale(LC_ALL, L"");	//default locale
	CLFArchiveZIP a;
	auto pp = std::make_shared<CLFPassphraseNULL>();
	a.read_open(LF_PROJECT_DIR() / L"test/test_extract.zip", pp);
	EXPECT_TRUE(a.is_modify_supported());
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
	auto pp = std::make_shared<CLFPassphraseNULL>();
	a.read_open(LF_PROJECT_DIR() / L"test/test_broken_crc.zip", pp);
	EXPECT_TRUE(a.is_modify_supported());
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
	/*for (;;) {
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
	EXPECT_NE(std::string(data.begin(), data.end()), std::string("12345"));	//broken*/

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
	{
		CLFArchiveZIP a;
		auto pp = std::make_shared<CLFPassphraseNULL>();
		a.read_open(LF_PROJECT_DIR() / L"test/test_broken_file.zip", pp);
		EXPECT_EQ(L"ZIP", a.get_format_name());

		LF_ENTRY_STAT* entry = nullptr;
		EXPECT_THROW(entry = a.read_entry_begin(), LF_EXCEPTION);
	}

	{
		CLFArchiveZIP a;
		auto pp = std::make_shared<CLFPassphraseNULL>();
		a.read_open(LF_PROJECT_DIR() / L"test/test_broken_crc.zip", pp);
		EXPECT_EQ(L"ZIP", a.get_format_name());

		EXPECT_NO_THROW({
			for (auto entry = a.read_entry_begin(); entry; entry = a.read_entry_next()) {
			continue;
			}
			});
	}
	EXPECT_THROW({
		ARCLOG arcLog;
		CLFProgressHandlerNULL progressHandler;
		testOneArchive(LF_PROJECT_DIR() / L"test/test_broken_crc.zip", arcLog, progressHandler, std::make_shared<CLFPassphraseNULL>());
		}, LF_EXCEPTION);

	EXPECT_THROW({
		ARCLOG arcLog;
		CLFProgressHandlerNULL progressHandler;
		testOneArchive(LF_PROJECT_DIR() / L"test/test_broken_file.zip", arcLog, progressHandler, std::make_shared<CLFPassphraseNULL>());
		}, LF_EXCEPTION);
}

TEST(CLFArchiveZIP, read_enum_non_existing)
{
	_wsetlocale(LC_ALL, L"");	//default locale
	CLFArchiveZIP a;
	auto pp = std::make_shared<CLFPassphraseNULL>();
	EXPECT_THROW(
		a.read_open(LF_PROJECT_DIR() / L"test/some_file_that_does_not_exist.zip", pp),
		LF_EXCEPTION);
}

TEST(CLFArchiveZIP, read_enum_unicode)
{
	_wsetlocale(LC_ALL, L"");	//default locale
	CLFArchiveZIP a;
	auto pp = std::make_shared<CLFPassphraseNULL>();
	a.read_open(LF_PROJECT_DIR() / L"test/test_unicode_control.zip", pp);
	EXPECT_TRUE(a.is_modify_supported());
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
	auto pp = std::make_shared<CLFPassphraseNULL>();
	a.read_open(LF_PROJECT_DIR() / L"test/test_zip_sfx.dat", pp);
	EXPECT_TRUE(a.is_modify_supported());
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

TEST(CLFArchiveZIP, read_enum_2099_zip)
{
	_wsetlocale(LC_ALL, L"");	//default locale
	const auto file = std::filesystem::path(__FILEW__).parent_path() / L"test_2099.zip";

	CLFArchiveZIP a;
	auto pp = std::make_shared<CLFPassphraseNULL>();
	a.read_open(file, pp);
	EXPECT_TRUE(a.is_modify_supported());
	ASSERT_EQ(L"ZIP", a.get_format_name());

	int count = 0;
	int numDir = 0;
	for (auto entry = a.read_entry_begin(); entry; entry = a.read_entry_next()) {
		count++;
		if (entry->is_directory()) {
			numDir++;
		} else {
			if (entry->path.wstring().find(L"ccd.txt") != -1) {
				ASSERT_EQ(entry->stat.st_size, 44);
				//EXPECT_EQ(entry->method_name, L"---");
				ASSERT_EQ(entry->compressed_size, 44);
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
				//EXPECT_EQ(entry->method_name, L"---");
				ASSERT_EQ(entry->compressed_size, 46);
			}
		}
		EXPECT_FALSE(entry->is_encrypted);
	}
	EXPECT_EQ(count, 2099);
	EXPECT_EQ(numDir, 0);
}

#if 0
zipx support is disabled
TEST(CLFArchiveZIP, read_enum_2099_zipx)
{
	_wsetlocale(LC_ALL, L"");	//default locale
	const auto file = std::filesystem::path(__FILEW__).parent_path() / L"test_2099.zipx";

	CLFArchiveZIP a;
	auto pp = std::make_shared<CLFPassphraseNULL>();
	a.read_open(file, pp);
	EXPECT_TRUE(a.is_modify_supported());
	ASSERT_EQ(L"ZIP", a.get_format_name());

	int count = 0;
	int numDir = 0;
	for (auto entry = a.read_entry_begin(); entry; entry = a.read_entry_next()) {
		count++;
		if (entry->is_directory()) {
			numDir++;
		} else {
			if (entry->path.wstring().find(L"ccd.txt") != -1) {
				ASSERT_EQ(entry->stat.st_size, 44);
				//EXPECT_EQ(entry->method_name, L"---");
				EXPECT_EQ(entry->compressed_size, 40);
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
				ASSERT_EQ(entry->stat.st_size, 48);
				//EXPECT_EQ(entry->method_name, L"---");
				ASSERT_EQ(entry->compressed_size, 41);
			}
		}
		EXPECT_FALSE(entry->is_encrypted);
	}
	EXPECT_EQ(count, 2099 + 1);
	EXPECT_EQ(numDir, 1);
}
#endif

TEST(CLFArchiveZIP, read_enum_multipart)
{
	CLFArchiveZIP a;
	EXPECT_TRUE(a.is_known_format(LF_PROJECT_DIR() / L"test" / L"smile.zip.001"));

	auto pp = std::make_shared<CLFPassphraseNULL>();
	a.read_open(LF_PROJECT_DIR() / L"test" / L"smile.zip.001", pp);
	EXPECT_FALSE(a.is_modify_supported());
	EXPECT_EQ(L"ZIP", a.get_format_name());
	auto entry = a.read_entry_begin();
	EXPECT_NE(nullptr, entry);
	EXPECT_EQ(entry->path.wstring(), L"smile.bmp");
	EXPECT_FALSE(entry->is_directory());
	EXPECT_EQ(L"Deflate", entry->method_name);
	EXPECT_EQ(6110262, entry->stat.st_size);
	EXPECT_EQ(14563, entry->compressed_size);
	entry = a.read_entry_next();
	EXPECT_EQ(nullptr, entry);
}

TEST(CLFArchiveZIP, read_passphrase)
{
	_wsetlocale(LC_ALL, L"");	//default locale
	CLFArchiveZIP a;
	{
		//---content listing does not require passphrase
		auto pp = std::make_shared<CLFPassphraseNULL>();
		a.read_open(LF_PROJECT_DIR() / L"test/test_password_abcde.zip", pp);
		for (auto item = a.read_entry_begin(); item; item = a.read_entry_next()) {
			//do nothing
			EXPECT_TRUE(item->is_encrypted);
		}
	}
	{
		//---content listing does not require passphrase
		auto pp = std::make_shared<CLFPassphraseConst>(L"abcde");
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
		auto pp = std::make_shared<CLFPassphraseConst>(L"abcde");
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

	//what if wrong password?
	{
		auto pp = std::make_shared<CLFPassphraseArray>();	//"abc" for first time, abort on second time
		pp->passwords = { "abc" };
		a.read_open(LF_PROJECT_DIR() / L"test/test_password_abcde.zip", pp);
		std::vector<char> data;
		data.clear();
		auto entry = a.read_entry_begin();
		EXPECT_THROW(
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
		}, LF_EXCEPTION);
	}
}

TEST(CLFArchiveZIP, zipx)
{
	CLFArchiveZIP a;
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
		EXPECT_FALSE(CLFArchiveZIP::is_known_format(dir / L"smile.png"));
		EXPECT_FALSE(CLFArchiveZIP::is_known_format(dir / L"smile.gif"));
		EXPECT_FALSE(CLFArchiveZIP::is_known_format(dir / L"smile.jpg"));
	}
	{
		const auto dir = LF_PROJECT_DIR() / L"test";
		EXPECT_TRUE(CLFArchiveZIP::is_known_format(dir / L"test_broken_file.zip"));
		EXPECT_TRUE(CLFArchiveZIP::is_known_format(dir / L"test_broken_crc.zip"));
		EXPECT_TRUE(CLFArchiveZIP::is_known_format(dir / L"test_extract.zip"));
		EXPECT_FALSE(CLFArchiveZIP::is_known_format(dir / L"test_extract.zipx"));	//rejected by extension
		EXPECT_TRUE(CLFArchiveZIP::is_known_format(dir / L"test_password_abcde.zip"));
		EXPECT_TRUE(CLFArchiveZIP::is_known_format(dir / L"test_unicode_control.zip"));

		EXPECT_TRUE(CLFArchiveZIP::is_known_format(dir / L"smile.zip.001"));

		EXPECT_FALSE(CLFArchiveZIP::is_known_format(dir / L"image_method0.arj"));
		EXPECT_FALSE(CLFArchiveZIP::is_known_format(dir / L"test.bza"));
		EXPECT_FALSE(CLFArchiveZIP::is_known_format(dir / L"test.gza"));
		EXPECT_TRUE(CLFArchiveZIP::is_known_format(dir / L"test_zip_sfx.dat"));	//true if checked inside content
	}
}

TEST(CLFArchiveZIP, check_if_encrypted)
{
	const auto dir = LF_PROJECT_DIR() / L"test";
	auto pp = std::make_shared<CLFPassphraseNULL>();
	{
		CLFArchiveZIP a;
		a.read_open(dir / L"test_extract.zip", pp);
		EXPECT_FALSE(a.contains_encryted_entry());
	}
	{
		CLFArchiveZIP a;
		a.read_open(dir / L"test_password_abcde.zip", pp);
		EXPECT_TRUE(a.contains_encryted_entry());
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
		auto pp = std::make_shared<CLFPassphraseNULL>();
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
		auto pp = std::make_shared<CLFPassphraseNULL>();
		a.read_open(temp, pp);
		auto entry = a.read_entry_begin();
		EXPECT_NE(nullptr, entry);
		EXPECT_EQ(L"test/file.txt", entry->path.wstring());
		EXPECT_EQ(1000, entry->stat.st_size);
	}
	//test file consistency
	EXPECT_NO_THROW({
		ARCLOG arcLog;
		CLFProgressHandlerNULL progressHandler;
		testOneArchive(temp, arcLog, progressHandler, std::make_shared<CLFPassphraseNULL>());
		});
	UtilDeletePath(temp);
	EXPECT_FALSE(std::filesystem::exists(temp));
	UtilDeletePath(src);
	EXPECT_FALSE(std::filesystem::exists(src));
}

TEST(CLFArchiveZIP, add_file_entry_non_ascii_archive)
{
	auto temp = UtilGetTempPath() / L"テスト_ソ_表.zip";
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
		auto pp = std::make_shared<CLFPassphraseNULL>();
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
		auto pp = std::make_shared<CLFPassphraseNULL>();
		a.read_open(temp, pp);
		auto entry = a.read_entry_begin();
		EXPECT_NE(nullptr, entry);
		EXPECT_EQ(L"test/file.txt", entry->path.wstring());
		EXPECT_EQ(1000, entry->stat.st_size);
	}
	//test file consistency
	EXPECT_NO_THROW({
		ARCLOG arcLog;
		CLFProgressHandlerNULL progressHandler;
		testOneArchive(temp, arcLog, progressHandler, std::make_shared<CLFPassphraseNULL>());
		});
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
		auto pp = std::make_shared<CLFPassphraseNULL>();
		a.write_open(temp, LF_ARCHIVE_FORMAT::ZIP, LF_WOPT_STANDARD, args, pp);
		LF_ENTRY_STAT e;
		e.read_stat(LF_PROJECT_DIR(), L"test/");	//LF_PROJECT_DIR() as a directory template
		a.add_directory_entry(e);
		a.close();
	}
	{
		CLFArchiveZIP a;
		auto pp = std::make_shared<CLFPassphraseNULL>();
		a.read_open(temp, pp);
		auto entry = a.read_entry_begin();
		EXPECT_NE(nullptr, entry);
		EXPECT_EQ(L"test/", entry->path.wstring());
	}
	//test file consistency
	EXPECT_NO_THROW({
		ARCLOG arcLog;
		CLFProgressHandlerNULL progressHandler;
		testOneArchive(temp, arcLog, progressHandler, std::make_shared<CLFPassphraseNULL>());
		});
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
		auto pp = std::make_shared<CLFPassphraseConst>(L"password");
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
		auto pp = std::make_shared<CLFPassphraseNULL>();
		CLFArchiveZIP a;
		a.read_open(temp, pp);
		for (auto item = a.read_entry_begin(); item; item = a.read_entry_next()) {
			//do nothing
			EXPECT_TRUE(item->is_encrypted);
		}
	}
	{
		CLFArchiveZIP a;
		auto pp = std::make_shared<CLFPassphraseConst>(L"password");
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
	//test file consistency
	EXPECT_NO_THROW({
		ARCLOG arcLog;
		CLFProgressHandlerNULL progressHandler;
		testOneArchive(temp, arcLog, progressHandler, std::make_shared<CLFPassphraseConst>(L"password"));
		});
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
		std::map<std::wstring, std::wstring> methods = {
			{L"store", L"Store"},
			{L"deflate", L"Deflate"},
			{L"bzip2", L"Bzip2"},
			{L"lzma", L"LZMA1"},
			{L"zstd", L"ZSTD"},
			{L"xz", L"XZ"},
			//{"lz4", L"LZ4"},
		};
		for (const auto& method : methods) {
			{
				CLFArchiveZIP a;
				LF_COMPRESS_ARGS args;
				args.load(CConfigFile());
				args.formats.zip.compression.value = method.first;
				args.formats.zip.compression_level.value = Format(L"%d", level);
				auto pp = std::make_shared<CLFPassphraseNULL>();
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
				auto pp = std::make_shared<CLFPassphraseNULL>();
				a.read_open(temp, pp);
				auto entry = a.read_entry_begin();
				EXPECT_NE(nullptr, entry);
				EXPECT_EQ(L"test/file.txt", entry->path.wstring());
				EXPECT_EQ(1000, entry->stat.st_size);
				EXPECT_LE(entry->compressed_size, 1000);
				EXPECT_EQ(method.second, entry->method_name);
				//EXPECT_EQ(level, ); no way to get compression level; checking creation errors only
			}
			//test file consistency
			EXPECT_NO_THROW({
				ARCLOG arcLog;
				CLFProgressHandlerNULL progressHandler;
				testOneArchive(temp, arcLog, progressHandler, std::make_shared<CLFPassphraseNULL>());
				});
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
	struct CRYPTO_CODE{
		const wchar_t* method;
		int aes_version;
		int aes_strength;
	};
	std::vector<CRYPTO_CODE> codes = {
		{L"aes256", MZ_AES_VERSION, MZ_AES_STRENGTH_256},
		{L"aes192", MZ_AES_VERSION, MZ_AES_STRENGTH_192},
		{L"aes128", MZ_AES_VERSION, MZ_AES_STRENGTH_128},
		{L"zipcrypto", 0, 0},
	};
	for (const auto [code, aes_version, aes_strength] : codes) {
		auto pp = std::make_shared<CLFPassphraseConst>(L"password");
		{
			CLFArchiveZIP a;
			LF_COMPRESS_ARGS args;
			args.load(CConfigFile());
			args.formats.zip.encryption.value = code;
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
			auto [entry_aes_version, entry_aes_strength] = a.test_sub_get_encryption();
			EXPECT_EQ(entry_aes_version, aes_version);
			EXPECT_EQ(entry_aes_strength, aes_strength);
		}
		//test file consistency
		EXPECT_NO_THROW({
			ARCLOG arcLog;
			CLFProgressHandlerNULL progressHandler;
			testOneArchive(temp, arcLog, progressHandler, pp);
			});
		UtilDeletePath(temp);
		EXPECT_FALSE(std::filesystem::exists(temp));
	}
	UtilDeletePath(src);
	EXPECT_FALSE(std::filesystem::exists(src));
}

#include "../archive_libarchive.h"

void sub_test_archive_LA(const std::filesystem::path& archive_path, std::shared_ptr<ILFPassphrase> passphrase_callback)
{
	CLFArchiveLA arc;
	arc.read_open(archive_path, passphrase_callback);
	// loop for each entry
	for (auto* entry = arc.read_entry_begin(); entry; entry = arc.read_entry_next()) {
		//original file name
		auto originalPath = entry->path;
		//original attributes
		int nAttribute = entry->stat.st_mode;

		try {
			if (!entry->is_directory()) {
				//go
				int64_t global_offset = 0;
				for (bool bEOF = false; !bEOF;) {
					arc.read_file_entry_block([&](const void* buf, int64_t data_size, const offset_info* offset) {
						if (!buf || data_size == 0) {
							bEOF = true;
						} else {
							global_offset += data_size;
							if (offset && offset->offset != global_offset) {
								global_offset = offset->offset;
							}
						}
					});
				}
				//arcLog(originalPath, UtilLoadString(IDS_ARCLOG_OK));
			}
		} catch (const LF_USER_CANCEL_EXCEPTION& e) {
			throw e;
		} catch (const LF_EXCEPTION& e) {
			throw e;
		}
	}
	//end
	arc.close();
}

void sub_add_file_to_existing_zip(const std::filesystem::path& zip_file, std::shared_ptr<ILFPassphrase> pp = std::make_shared<CLFPassphraseNULL>())
{
	ASSERT_TRUE(std::filesystem::exists(zip_file));
	ASSERT_NO_THROW({
		ARCLOG arcLog;
		CLFProgressHandlerNULL progressHandler;
		testOneArchive(zip_file, arcLog, progressHandler, pp);
	});

	auto temp = UtilGetTemporaryFileName();
	{
		CLFArchiveZIP r;
		LF_COMPRESS_ARGS args;
		args.load(CConfigFile());
		r.read_open(zip_file, pp);
		auto a = r.make_copy_archive(temp, args, [](const LF_ENTRY_STAT&) {return true; });

		{
			auto src = UtilGetTemporaryFileName();
			{
				CAutoFile f;
				f.open(src, L"w");
				for (int i = 0; i < 100; i++) {
					fputs("abcde12345", f);
				}
			}
			RAW_FILE_READER provider;
			provider.open(src);
			LF_ENTRY_STAT e;
			e.read_stat(src, L"test/added_file.txt");
			size_t sum = 0;
			a->add_file_entry(e, [&]() {
				auto data = provider();
				sum += data.size;
				return data;
			});
			EXPECT_EQ(sum, 10 * 100);
			provider.close();
			UtilDeletePath(src);
			EXPECT_FALSE(std::filesystem::exists(src));
		}
		a->close();
	}

	EXPECT_NO_THROW(sub_test_archive_LA(temp, pp));
	//test file consistency
	EXPECT_NO_THROW({
		ARCLOG arcLog;
		CLFProgressHandlerNULL progressHandler;
		testOneArchive(temp, arcLog, progressHandler, pp);
	});
	{
		CLFArchiveZIP modified;
		CLFArchiveZIP original;
		auto pp = std::make_shared<CLFPassphraseNULL>();
		modified.read_open(temp, pp);
		original.read_open(zip_file, pp);

		EXPECT_EQ(modified.contains_encryted_entry(), original.contains_encryted_entry());

		auto entry_mod = modified.read_entry_begin();
		auto entry_org = original.read_entry_begin();
		for (; entry_org;) {
			ASSERT_NE(nullptr, entry_org);
			ASSERT_NE(nullptr, entry_mod);
			EXPECT_EQ(entry_org->path.wstring(), entry_mod->path.wstring());
			EXPECT_EQ(entry_org->stat.st_size, entry_mod->stat.st_size);
			EXPECT_EQ(entry_org->stat.st_mtime, entry_mod->stat.st_mtime);

			entry_org = original.read_entry_next();
			entry_mod = modified.read_entry_next();
		}
		EXPECT_NE(nullptr, entry_mod);
		EXPECT_EQ(L"test/added_file.txt", entry_mod->path.wstring());
		EXPECT_EQ(1000, entry_mod->stat.st_size);

		//entry will be encrypted if zip is encrypted with my implementation
		EXPECT_EQ(entry_mod->is_encrypted, original.contains_encryted_entry());
	}
	UtilDeletePath(temp);
	EXPECT_FALSE(std::filesystem::exists(temp));
}
TEST(CLFArchiveZIP, add_file_to_existing_zip)
{
	sub_add_file_to_existing_zip(LF_PROJECT_DIR() / L"test/test_extract.zip");
}

TEST(CLFArchiveZIP, add_file_to_existing_zip_2099)
{
	sub_add_file_to_existing_zip(LF_PROJECT_DIR() / L"ArchiverCode/test/test_2099.zip");
}

TEST(CLFArchiveZIP, add_file_to_existing_zipx)
{
	sub_add_file_to_existing_zip(LF_PROJECT_DIR() / L"test/test_extract.zipx");
}

TEST(CLFArchiveZIP, add_file_to_existing_encrypted_2099)
{
	auto pp = std::make_shared<CLFPassphraseConst>(L"abcde");
	const auto src = std::filesystem::path(__FILEW__).parent_path() / L"test_2099_password_abcde.zip";
	sub_add_file_to_existing_zip(src, pp);
}

TEST(CLFArchiveZIP, add_file_to_existing_encrypted_zip)
{
	auto pp = std::make_shared<CLFPassphraseConst>(L"abcde");
	sub_add_file_to_existing_zip(LF_PROJECT_DIR() / L"test/test_password_abcde.zip", pp);
}

TEST(CLFArchiveZIP, add_file_to_existing_another_zip)
{
	sub_add_file_to_existing_zip(LF_PROJECT_DIR() / L"test/test_unicode_control.zip");
}

TEST(CLFArchiveZIP, add_file_to_existing_self_extracting_zip)
{
	sub_add_file_to_existing_zip(LF_PROJECT_DIR() / L"test/test_zip_sfx.dat");
}

TEST(CLFArchiveZIP, remove_file_from_existing_zip)
{
	const auto src = LF_PROJECT_DIR() / L"test" / L"test_extract.zip";

	auto temp = UtilGetTemporaryFileName();
	{
		CLFArchiveZIP r;
		LF_COMPRESS_ARGS args;
		args.load(CConfigFile());
		auto pp = std::make_shared<CLFPassphraseNULL>();
		r.read_open(src, pp);
		auto a = r.make_copy_archive(temp, args, [](const LF_ENTRY_STAT& entry) {
			if (entry.path.filename() == L"file3.txt")return false;
			return true;
		});
		a->close();
	}
	//test file consistency
	EXPECT_NO_THROW({
		ARCLOG arcLog;
		CLFProgressHandlerNULL progressHandler;
		testOneArchive(temp, arcLog, progressHandler, std::make_shared<CLFPassphraseNULL>());
		});
	{
		CLFArchiveZIP modified;
		CLFArchiveZIP original;
		auto pp = std::make_shared<CLFPassphraseNULL>();
		modified.read_open(temp, pp);
		original.read_open(src, pp);

		EXPECT_EQ(modified.contains_encryted_entry(), original.contains_encryted_entry());

		auto entry_mod = modified.read_entry_begin();
		auto entry_org = original.read_entry_begin();
		for (; entry_mod;) {
			ASSERT_NE(nullptr, entry_org);
			ASSERT_NE(nullptr, entry_mod);
			EXPECT_EQ(entry_org->path.wstring(), entry_mod->path.wstring());
			EXPECT_EQ(entry_org->stat.st_size, entry_mod->stat.st_size);
			EXPECT_EQ(entry_org->stat.st_mtime, entry_mod->stat.st_mtime);

			entry_org = original.read_entry_next();
			entry_mod = modified.read_entry_next();
		}
		EXPECT_NE(nullptr, entry_org);
		EXPECT_EQ(L"かきくけこ/file3.txt", entry_org->path.wstring());
		EXPECT_EQ(5, entry_org->stat.st_size);
	}
	UtilDeletePath(temp);
	EXPECT_FALSE(std::filesystem::exists(temp));
}

TEST(CLFArchiveZIP, remove_directory_from_existing_zip)
{
	const auto src = LF_PROJECT_DIR() / L"test" / L"test_extract.zip";

	auto temp = UtilGetTemporaryFileName();
	{
		CLFArchiveZIP r;
		LF_COMPRESS_ARGS args;
		args.load(CConfigFile());
		auto pp = std::make_shared<CLFPassphraseNULL>();
		r.read_open(src, pp);
		auto a = r.make_copy_archive(temp, args, [](const LF_ENTRY_STAT& entry) {
			if (startsWith(entry.path.wstring(), L"かきくけこ/"))return false;
			return true;
		});
		a->close();
	}
	//test file consistency
	EXPECT_NO_THROW({
		ARCLOG arcLog;
		CLFProgressHandlerNULL progressHandler;
		testOneArchive(temp, arcLog, progressHandler, std::make_shared<CLFPassphraseNULL>());
		});
	{
		CLFArchiveZIP modified;
		auto pp = std::make_shared<CLFPassphraseNULL>();
		modified.read_open(temp, pp);

		std::vector<std::wstring> contents = {
			L"dirA/dirB/",
			L"dirA/dirB/dirC/",
			L"dirA/dirB/dirC/file1.txt",
			L"dirA/dirB/file2.txt",
			L"あいうえお.txt",
			//L"かきくけこ/file3.txt", <- removed item
		};
		auto entry_mod = modified.read_entry_begin();
		size_t i;
		for (i = 0; entry_mod; i++) {
			EXPECT_EQ(contents[i], entry_mod->path.wstring());
			entry_mod = modified.read_entry_next();
		}
		EXPECT_EQ(i, contents.size());
	}
	UtilDeletePath(temp);
	EXPECT_FALSE(std::filesystem::exists(temp));
}

TEST(CLFArchiveZIP, remove_file_from_existing_zip_2099)
{
	const auto src = std::filesystem::path(__FILEW__).parent_path() / L"test_2099.zip";

	auto temp = UtilGetTemporaryFileName();
	{
		CLFArchiveZIP r;
		LF_COMPRESS_ARGS args;
		args.load(CConfigFile());
		auto pp = std::make_shared<CLFPassphraseNULL>();
		r.read_open(src, pp);
		auto a = r.make_copy_archive(temp, args, [](const LF_ENTRY_STAT& entry) {
			if (entry.path.filename().wstring().find(L"ccd.txt")!=-1)return false;
			return true;
		});
		a->close();
	}
	//test file consistency
	EXPECT_NO_THROW({
		ARCLOG arcLog;
		CLFProgressHandlerNULL progressHandler;
		testOneArchive(temp, arcLog, progressHandler, std::make_shared<CLFPassphraseNULL>());
		});
	UtilDeletePath(temp);
	EXPECT_FALSE(std::filesystem::exists(temp));
}

TEST(CLFArchiveZIP, remove_file_from_existing_encrypted_2099)
{
	const auto src = std::filesystem::path(__FILEW__).parent_path() / L"test_2099_password_abcde.zip";
	//test file consistency
	EXPECT_NO_THROW({
		ARCLOG arcLog;
		CLFProgressHandlerNULL progressHandler;
		testOneArchive(src, arcLog, progressHandler, std::make_shared<CLFPassphraseConst>(L"abcde"));
	});

	auto temp = UtilGetTemporaryFileName();
	{
		CLFArchiveZIP r;
		LF_COMPRESS_ARGS args;
		args.load(CConfigFile());
		auto pp = std::make_shared<CLFPassphraseNULL>();
		r.read_open(src, pp);
		auto a = r.make_copy_archive(temp, args, [](const LF_ENTRY_STAT& entry) {
			if (entry.path.filename().wstring().find(L"ccd.txt") != -1)return false;
			return true;
		});
		a->close();
	}
	//test file consistency
	EXPECT_NO_THROW({
		ARCLOG arcLog;
		CLFProgressHandlerNULL progressHandler;
		testOneArchive(temp, arcLog, progressHandler, std::make_shared<CLFPassphraseConst>(L"abcde"));
	});
	UtilDeletePath(temp);
	EXPECT_FALSE(std::filesystem::exists(temp));
}
TEST(CLFArchiveZIP, make_copy_archive_2099)
{
	const auto src = std::filesystem::path(__FILEW__).parent_path() / L"test_2099.zip";
	//test file consistency
	ASSERT_NO_THROW({
		ARCLOG arcLog;
		CLFProgressHandlerNULL progressHandler;
		testOneArchive(src, arcLog, progressHandler, std::make_shared<CLFPassphraseNULL>());
		});

	auto temp = UtilGetTemporaryFileName();
	{
		CLFArchiveZIP r;
		LF_COMPRESS_ARGS args;
		args.load(CConfigFile());
		auto pp = std::make_shared<CLFPassphraseNULL>();
		r.read_open(src, pp);
		auto a = r.make_copy_archive(temp, args, [](const LF_ENTRY_STAT& entry) {
			return true;
		});
		a->close();
	}
	//test file consistency
	EXPECT_NO_THROW({
		ARCLOG arcLog;
		CLFProgressHandlerNULL progressHandler;
		testOneArchive(temp, arcLog, progressHandler, std::make_shared<CLFPassphraseNULL>());
		});
	UtilDeletePath(temp);
	EXPECT_FALSE(std::filesystem::exists(temp));
}

TEST(CLFArchiveZIP, make_copy_archive_encrypted_2099)
{
	const auto src = std::filesystem::path(__FILEW__).parent_path() / L"test_2099_password_abcde.zip";
	//test file consistency
	EXPECT_NO_THROW({
		ARCLOG arcLog;
		CLFProgressHandlerNULL progressHandler;
		testOneArchive(src, arcLog, progressHandler, std::make_shared<CLFPassphraseConst>(L"abcde"));
		});

	auto temp = UtilGetTemporaryFileName();
	{
		CLFArchiveZIP r;
		LF_COMPRESS_ARGS args;
		args.load(CConfigFile());
		auto pp = std::make_shared<CLFPassphraseNULL>();
		r.read_open(src, pp);
		auto a = r.make_copy_archive(temp, args, [](const LF_ENTRY_STAT& entry) {
			return true;
		});
		a->close();
	}
	//test file consistency
	EXPECT_NO_THROW({
		ARCLOG arcLog;
		CLFProgressHandlerNULL progressHandler;
		testOneArchive(temp, arcLog, progressHandler, std::make_shared<CLFPassphraseConst>(L"abcde"));
		});
	UtilDeletePath(temp);
	EXPECT_FALSE(std::filesystem::exists(temp));
}

TEST(CLFArchiveZIP, make_copy_archive_encrypted)
{
	const auto src = LF_PROJECT_DIR() / L"test/test_password_abcde.zip";
	//test file consistency
	EXPECT_NO_THROW({
		ARCLOG arcLog;
		CLFProgressHandlerNULL progressHandler;
		testOneArchive(src, arcLog, progressHandler, std::make_shared<CLFPassphraseConst>(L"abcde"));
		});

	auto temp = UtilGetTemporaryFileName();
	{
		CLFArchiveZIP r;
		LF_COMPRESS_ARGS args;
		args.load(CConfigFile());
		auto pp = std::make_shared<CLFPassphraseNULL>();
		r.read_open(src, pp);
		auto a = r.make_copy_archive(temp, args, [](const LF_ENTRY_STAT& entry) {
			return true;
		});
		a->close();
	}
	//test file consistency
	EXPECT_NO_THROW({
		ARCLOG arcLog;
		CLFProgressHandlerNULL progressHandler;
		testOneArchive(temp, arcLog, progressHandler, std::make_shared<CLFPassphraseConst>(L"abcde"));
		});
	UtilDeletePath(temp);
	EXPECT_FALSE(std::filesystem::exists(temp));
}

