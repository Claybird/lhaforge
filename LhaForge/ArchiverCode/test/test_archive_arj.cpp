#include "stdafx.h"
#include "../archive_arj.h"
#include "Utilities/utility.h"

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

TEST(CLFArchiveARJ, is_known_format)
{
	{
		const auto dir = LF_PROJECT_DIR() / L"ArchiverCode/test";
		EXPECT_FALSE(CLFArchiveARJ::is_known_format(dir / L"empty.gz"));
		EXPECT_FALSE(CLFArchiveARJ::is_known_format(dir / L"empty.bz2"));
		EXPECT_FALSE(CLFArchiveARJ::is_known_format(dir / L"empty.xz"));
		EXPECT_FALSE(CLFArchiveARJ::is_known_format(dir / L"empty.lzma"));
		EXPECT_FALSE(CLFArchiveARJ::is_known_format(dir / L"empty.zst"));

		EXPECT_FALSE(CLFArchiveARJ::is_known_format(dir / L"abcde.gz"));
		EXPECT_FALSE(CLFArchiveARJ::is_known_format(dir / L"abcde.bz2"));
		EXPECT_FALSE(CLFArchiveARJ::is_known_format(dir / L"abcde.xz"));
		EXPECT_FALSE(CLFArchiveARJ::is_known_format(dir / L"abcde.lzma"));
		EXPECT_FALSE(CLFArchiveARJ::is_known_format(dir / L"abcde.zst"));

		EXPECT_FALSE(CLFArchiveARJ::is_known_format(__FILEW__));
		EXPECT_FALSE(CLFArchiveARJ::is_known_format(L"some_non_existing_file"));
		EXPECT_FALSE(CLFArchiveARJ::is_known_format(dir / L"smile.png"));
		EXPECT_FALSE(CLFArchiveARJ::is_known_format(dir / L"smile.gif"));
		EXPECT_FALSE(CLFArchiveARJ::is_known_format(dir / L"smile.jpg"));
	}
	{
		const auto dir = LF_PROJECT_DIR() / L"test";
		EXPECT_FALSE(CLFArchiveARJ::is_known_format(dir / L"test_broken_file.zip"));
		EXPECT_FALSE(CLFArchiveARJ::is_known_format(dir / L"test_broken_crc.zip"));
		EXPECT_FALSE(CLFArchiveARJ::is_known_format(dir / L"test_extract.zip"));
		EXPECT_FALSE(CLFArchiveARJ::is_known_format(dir / L"test_extract.zipx"));
		EXPECT_FALSE(CLFArchiveARJ::is_known_format(dir / L"test_password_abcde.zip"));
		EXPECT_FALSE(CLFArchiveARJ::is_known_format(dir / L"test_unicode_control.zip"));
		EXPECT_FALSE(CLFArchiveARJ::is_known_format(dir / L"test_zip_sfx.dat"));
		EXPECT_FALSE(CLFArchiveARJ::is_known_format(dir / L"smile.zip.001"));

		EXPECT_TRUE(CLFArchiveARJ::is_known_format(dir / L"image_method0.arj"));
		EXPECT_FALSE(CLFArchiveARJ::is_known_format(dir / L"test.bza"));
		EXPECT_FALSE(CLFArchiveARJ::is_known_format(dir / L"test.gza"));
	}
}
