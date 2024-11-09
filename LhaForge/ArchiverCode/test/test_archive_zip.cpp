#include "stdafx.h"
#include "../archive_zip.h"
#include "zip.h"
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

		EXPECT_THROW({
			for (auto entry = a.read_entry_begin(); entry; entry = a.read_entry_next()) {
				for (bool bEOF = false; !bEOF;) {
					a.read_file_entry_block([&](const void* buf, int64_t data_size, const offset_info* offset) {
						if (!buf || data_size == 0) {
							bEOF = true;
						}
					});
				}
			}
			}, LF_EXCEPTION);
	}

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
		//---content listing does not require passphrase
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
		EXPECT_TRUE(CLFArchiveZIP::is_known_format(dir / L"test_extract.zipx"));
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
		std::map<std::string, std::wstring> methods = {
			{"store", L"Store"},
			{"deflate", L"Deflate"},
			{"bzip2", L"Bzip2"},
			{"lzma", L"LZMA1"},
			{"zstd", L"ZSTD"},
			{"xz", L"XZ"},
			//{"lz4", L"LZ4"},
		};
		for (const auto& method : methods) {
			{
				CLFArchiveZIP a;
				LF_COMPRESS_ARGS args;
				args.load(CConfigFile());
				args.formats.zip.params["compression"] = method.first;
				args.formats.zip.params["level"] = UtilToUTF8(Format(L"%d", level));
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
	std::vector<std::string> codes = {
		"aes256", "aes192", "aes128", "zipcrypto"
	};
	for (const auto& code : codes) {
		auto pp = std::make_shared<CLFPassphraseConst>(L"password");
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

TEST(CLFArchiveZIP, add_file_to_existing_zip)
{
	const auto dir = LF_PROJECT_DIR() / L"test";
	std::vector<std::filesystem::path> zip_files = {
		dir / L"test_extract.zip",
		dir / L"test_extract.zipx",
		dir / L"test_password_abcde.zip",
		dir / L"test_unicode_control.zip",
		dir / L"test_zip_sfx.dat",
	};

	auto src = UtilGetTemporaryFileName();
	{
		CAutoFile f;
		f.open(src, L"w");
		for (int i = 0; i < 100; i++) {
			fputs("abcde12345", f);
		}
	}
	for (const auto& zip_file : zip_files) {
		auto temp = UtilGetTemporaryFileName();
		{
			CLFArchiveZIP r;
			LF_COMPRESS_ARGS args;
			args.load(CConfigFile());
			auto pp = std::make_shared<CLFPassphraseConst>(L"password");
			r.read_open(zip_file, pp);
			auto a = r.make_copy_archive(temp, args, [](const LF_ENTRY_STAT&) {return true; });

			LF_ENTRY_STAT e;
			RAW_FILE_READER provider;
			provider.open(src);
			e.read_stat(src, L"test/added_file.txt");
			a->add_file_entry(e, [&]() {
				auto data = provider();
				return data;
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
	UtilDeletePath(src);
	EXPECT_FALSE(std::filesystem::exists(src));
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
