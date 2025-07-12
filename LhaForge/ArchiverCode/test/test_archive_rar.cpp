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
#include "../archive_rar.h"
#include "Utilities/Utility.h"
#include "CommonUtil.h"
#undef WINVER	//to avoid compiler warning
#undef _WIN32_WINNT	//to avoid compiler warning
#define RARDLL
#include <unrar/rar.hpp>
#include <unrar/dll.hpp>

std::filesystem::path rar_first_file(const std::filesystem::path& file);
TEST(CLFArchiveRAR, rar_first_file)
{
	rar_first_file(L"test.part0001.rar");
	EXPECT_EQ(std::filesystem::path(L"test.part0001.rar"), rar_first_file(L"test.part0001.rar"));
	EXPECT_EQ(std::filesystem::path(L"test.part0001.rar"), rar_first_file(L"test.part0002.rar"));
	EXPECT_EQ(std::filesystem::path(L"test.part0001.rar"), rar_first_file(L"test.part0101.rar"));
	EXPECT_EQ(std::filesystem::path(L"test.part0001.rar"), rar_first_file(L"test.part0101.RaR"));
	EXPECT_EQ(std::filesystem::path(L"test.part0001.rar"), rar_first_file(L"test.part0001.RaR"));
	EXPECT_EQ(std::filesystem::path(L"test.part001.rar"), rar_first_file(L"test.part005.rar"));
	EXPECT_EQ(std::filesystem::path(L"test.part01.rar"), rar_first_file(L"test.part05.rar"));
	EXPECT_EQ(std::filesystem::path(L"test.part1.rar"), rar_first_file(L"test.part5.rar"));
	EXPECT_EQ(std::filesystem::path(L"test.part.rar"), rar_first_file(L"test.part.rar"));

	EXPECT_EQ(std::filesystem::path(L"test.part000a.rar"), rar_first_file(L"test.part000a.rar"));
	EXPECT_EQ(std::filesystem::path(L"test.rar"), rar_first_file(L"test.rar"));
	EXPECT_EQ(std::filesystem::path(L"test.part0005.zip"), rar_first_file(L"test.part0005.zip"));

	EXPECT_EQ(std::filesystem::path(L"test.part0005.rar.zip"), rar_first_file(L"test.part0005.rar.zip"));
}

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
		EXPECT_FALSE(CLFArchiveRAR::is_known_format(dir / L"smile.png"));
		EXPECT_FALSE(CLFArchiveRAR::is_known_format(dir / L"smile.gif"));
		EXPECT_FALSE(CLFArchiveRAR::is_known_format(dir / L"smile.jpg"));
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
	const auto file = std::filesystem::path(__FILEW__).parent_path() / L"test_2099.rar";

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

TEST(CLFArchiveRAR, read_enum_2099_encrypted)
{
	_wsetlocale(LC_ALL, L"");	//default locale
	const auto file = std::filesystem::path(__FILEW__).parent_path() / L"test_2099_password.rar";

	CLFArchiveRAR a;
	auto pp = std::make_shared<CLFPassphraseArray>();
	//for (int i = 0; i < 2100; i++) {
		pp->passwords.push_back(L"password");
	//}
	a.read_open(file, pp);
	EXPECT_FALSE(a.is_modify_supported());
	EXPECT_EQ(L"RAR", a.get_format_name());

	int count = 0;
	int numDir = 0;
	for (auto entry = a.read_entry_begin(); entry; entry = a.read_entry_next()) {
		count++;
		if (entry->is_directory()) {
			numDir++;
			EXPECT_FALSE(entry->is_encrypted);
		} else {
			if (entry->path.wstring().find(L"ccd.txt") != -1) {
				EXPECT_EQ(entry->stat.st_size, 44);
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
				EXPECT_EQ(std::string(data.begin(), data.end()), ";kljd;lfj;lsdahg;has:hn:h :ahsd:fh:asdhg:ioh");
			} else {
				EXPECT_EQ(entry->stat.st_size, 48);
			}
			EXPECT_TRUE(entry->is_encrypted);
		}
	}
	EXPECT_EQ(count, 2099 + 1);
	EXPECT_EQ(numDir, 1);
}

TEST(CLFArchiveRAR, read_enum_2099_header_encrypted)
{
	_wsetlocale(LC_ALL, L"");	//default locale
	const auto file = std::filesystem::path(__FILEW__).parent_path() / L"test_2099_password_header_encrypted.rar";

	CLFArchiveRAR a;
	auto pp = std::make_shared<CLFPassphraseArray>();
	pp->passwords.push_back(L"password");	//header
	pp->passwords.push_back(L"password");	//content
	a.read_open(file, pp);
	EXPECT_FALSE(a.is_modify_supported());
	EXPECT_EQ(L"RAR", a.get_format_name());

	int count = 0;
	int numDir = 0;
	for (auto entry = a.read_entry_begin(); entry; entry = a.read_entry_next()) {
		count++;
		if (entry->is_directory()) {
			numDir++;
			EXPECT_FALSE(entry->is_encrypted);
		} else {
			if (entry->path.wstring().find(L"ccd.txt") != -1) {
				EXPECT_EQ(entry->stat.st_size, 44);
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
				EXPECT_EQ(std::string(data.begin(), data.end()), ";kljd;lfj;lsdahg;has:hn:h :ahsd:fh:asdhg:ioh");
			} else {
				EXPECT_EQ(entry->stat.st_size, 48);
			}
			EXPECT_TRUE(entry->is_encrypted);
		}
	}
	EXPECT_EQ(count, 2099 + 1);
	EXPECT_EQ(numDir, 1);
}

void sub_rar_test(std::filesystem::path file)
{
	CLFArchiveRAR a;
	EXPECT_TRUE(a.is_known_format(file));

	//auto pp = std::make_shared<CLFPassphraseConst>(L"password");
	auto pp = std::make_shared<CLFPassphraseArray>();
	//when header is encrypted, first password is used for header, second is for file content
	pp->passwords = { L"password", L"password" };
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

TEST(CLFArchiveRAR, rar_multipart_not_from_0001)
{
	sub_rar_test(LF_PROJECT_DIR() / L"test/smile.part0002.rar");
	sub_rar_test(LF_PROJECT_DIR() / L"test/smile.part0003.rar");
}

TEST(CLFArchiveRAR, rar_multi_password)
{
	auto file = LF_PROJECT_DIR() / L"test/smile_multi_encrypted.rar";
	CLFArchiveRAR a;
	EXPECT_TRUE(a.is_known_format(file));

	//auto pp = std::make_shared<CLFPassphraseConst>(L"password");
	auto pp = std::make_shared<CLFPassphraseArray>();
	pp->passwords = { L"password",L"abcde" };
	a.read_open(file, pp);
	EXPECT_FALSE(a.is_modify_supported());
	EXPECT_EQ(L"RAR", a.get_format_name());

	auto entry = a.read_entry_begin();
	ASSERT_NE(nullptr, entry);
	ASSERT_EQ(entry->path.wstring(), L"smile.bmp");
	EXPECT_FALSE(entry->is_directory());
	EXPECT_EQ(L"normal", entry->method_name);
	EXPECT_EQ(6110262, entry->stat.st_size);
	{
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

	entry = a.read_entry_next();
	ASSERT_NE(nullptr, entry);
	ASSERT_EQ(entry->path.wstring(), L"added_file.txt");
	EXPECT_FALSE(entry->is_directory());
	for (;;) {
		bool bEOF = false;
		a.read_file_entry_block([&](const void* buf, size_t data_size, const offset_info* offset) {
			if (!buf){
				bEOF = true;
			}
		});
		if (bEOF) {
			break;
		}
	}
	EXPECT_EQ(1000, entry->stat.st_size);
}
