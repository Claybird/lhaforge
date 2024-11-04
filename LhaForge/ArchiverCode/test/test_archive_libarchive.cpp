#include "stdafx.h"
#include "../archive_libarchive.h"
#include "../archive_libarchive_impl.h"

#include "CommonUtil.h"
#include "compress.h"
#include "extract.h"

TEST(CLFArchiveLA, mimic_archive_property)
{
	{
		auto fileToRead = LF_PROJECT_DIR() / L"test/test_extract.zip";
		LA_FILE_TO_READ src;
		auto pp = std::make_shared<CLFPassphraseNULL>();
		src.open(fileToRead, pp);
		src.begin();	//need to scan
		auto[la_format, filters, is_encrypted] = LA_FILE_TO_WRITE::mimic_archive_property(src);
		EXPECT_EQ(la_format, ARCHIVE_FORMAT_ZIP);
		EXPECT_EQ(filters.size(), 1);
		EXPECT_EQ(filters.back(), ARCHIVE_FILTER_NONE);
		EXPECT_FALSE(is_encrypted);
	}
	{
		auto fileToRead = LF_PROJECT_DIR() / L"test/test_gzip.gz";
		LA_FILE_TO_READ src;
		auto pp = std::make_shared<CLFPassphraseNULL>();
		src.open(fileToRead, pp);
		src.begin();	//need to scan
		auto[la_format, filters, is_encrypted] = LA_FILE_TO_WRITE::mimic_archive_property(src);
		EXPECT_EQ(la_format, ARCHIVE_FORMAT_RAW);
		EXPECT_EQ(filters.size(), 2);
		EXPECT_EQ(filters[0], ARCHIVE_FILTER_GZIP);
		EXPECT_EQ(filters[1], ARCHIVE_FILTER_NONE);
		EXPECT_FALSE(is_encrypted);
	}
	{
		auto fileToRead = LF_PROJECT_DIR() / L"test/test.tar.gz";
		LA_FILE_TO_READ src;
		auto pp = std::make_shared<CLFPassphraseNULL>();
		src.open(fileToRead, pp);
		src.begin();	//need to scan
		auto[la_format, filters, is_encrypted] = LA_FILE_TO_WRITE::mimic_archive_property(src);
		EXPECT_TRUE(la_format & ARCHIVE_FORMAT_TAR);
		EXPECT_EQ(filters.size(), 2);
		EXPECT_EQ(filters[0], ARCHIVE_FILTER_GZIP);
		EXPECT_EQ(filters[1], ARCHIVE_FILTER_NONE);
		EXPECT_FALSE(is_encrypted);
	}
}

TEST(archive_libarchive, getLAOptionsFromConfig)
{
	std::map<std::string, std::string> getLAOptionsFromConfig(
		const LF_COMPRESS_ARGS & args,
		LF_ARCHIVE_FORMAT format,
		LF_WRITE_OPTIONS options);

	LF_COMPRESS_ARGS fake_args;
	CConfigFile mngr;
	fake_args.load(mngr);
	{
		auto la_options = getLAOptionsFromConfig(fake_args, LF_ARCHIVE_FORMAT::ZIP, LF_WOPT_STANDARD);
		EXPECT_EQ(2, la_options.size());
		EXPECT_EQ("deflate", la_options.at("compression"));
		EXPECT_EQ("9", la_options.at("compression-level"));
		//EXPECT_EQ("ZipCrypt", la_options.at("encryption"));
		//EXPECT_EQ("UTF-8", la_options.at("hdrcharset"));
		//EXPECT_EQ("", la_options.at("zip64"));
	}
	{
		auto la_options = getLAOptionsFromConfig(fake_args, LF_ARCHIVE_FORMAT::ZIP, LF_WOPT_DATA_ENCRYPTION);
		EXPECT_EQ(3, la_options.size());
		EXPECT_EQ("deflate", la_options.at("compression"));
		EXPECT_EQ("9", la_options.at("compression-level"));
		EXPECT_EQ("ZipCrypt", la_options.at("encryption"));
		//EXPECT_EQ("UTF-8", la_options.at("hdrcharset"));
		//EXPECT_EQ("", la_options.at("zip64"));
	}
	{
		auto la_options = getLAOptionsFromConfig(fake_args, LF_ARCHIVE_FORMAT::_7Z, LF_WOPT_STANDARD);
		EXPECT_EQ(2, la_options.size());
		EXPECT_EQ("deflate", la_options.at("compression"));
		EXPECT_EQ("9", la_options.at("compression-level"));
	}

	{
		auto la_options = getLAOptionsFromConfig(fake_args, LF_ARCHIVE_FORMAT::TAR, LF_WOPT_STANDARD);
		EXPECT_EQ(1, la_options.size());
		EXPECT_EQ("UTF-8", la_options.at("hdrcharset"));
	}

	{
		auto la_options = getLAOptionsFromConfig(fake_args, LF_ARCHIVE_FORMAT::GZ, LF_WOPT_STANDARD);
		EXPECT_EQ(1, la_options.size());
		EXPECT_EQ("9", la_options.at("compression-level"));
	}

	{
		auto la_options = getLAOptionsFromConfig(fake_args, LF_ARCHIVE_FORMAT::BZ2, LF_WOPT_STANDARD);
		EXPECT_EQ(1, la_options.size());
		EXPECT_EQ("9", la_options.at("compression-level"));
	}

	{
		auto la_options = getLAOptionsFromConfig(fake_args, LF_ARCHIVE_FORMAT::XZ, LF_WOPT_STANDARD);
		EXPECT_EQ(2, la_options.size());
		EXPECT_EQ("9", la_options.at("compression-level"));
		EXPECT_EQ("0", la_options.at("threads"));
	}

	{
		auto la_options = getLAOptionsFromConfig(fake_args, LF_ARCHIVE_FORMAT::LZMA, LF_WOPT_STANDARD);
		EXPECT_EQ(1, la_options.size());
		EXPECT_EQ("9", la_options.at("compression-level"));
	}

	{
		auto la_options = getLAOptionsFromConfig(fake_args, LF_ARCHIVE_FORMAT::ZSTD, LF_WOPT_STANDARD);
		EXPECT_EQ(1, la_options.size());
		EXPECT_EQ("3", la_options.at("compression-level"));
	}

	{
		auto la_options = getLAOptionsFromConfig(fake_args, LF_ARCHIVE_FORMAT::LZ4, LF_WOPT_STANDARD);
		EXPECT_EQ(1, la_options.size());
		EXPECT_EQ("1", la_options.at("compression-level"));
	}
}


TEST(CLFArchiveLA, is_modify_supported)
{
	const auto dir = LF_PROJECT_DIR();
	auto check=[](const std::filesystem::path &p)->bool {
		CLFArchiveLA a;
		auto pp = std::make_shared<CLFPassphraseNULL>();
		a.read_open(p, pp);
		return a.is_modify_supported();
	};
	EXPECT_FALSE(check(dir / L"ArchiverCode/test/empty.gz"));
	EXPECT_FALSE(check(dir / L"ArchiverCode/test/empty.bz2"));
	EXPECT_FALSE(check(dir / L"ArchiverCode/test/empty.xz"));
	EXPECT_FALSE(check(dir / L"ArchiverCode/test/empty.lzma"));
	EXPECT_FALSE(check(dir / L"ArchiverCode/test/empty.zst"));

	EXPECT_FALSE(check(dir / L"ArchiverCode/test/abcde.gz"));
	EXPECT_FALSE(check(dir / L"ArchiverCode/test/abcde.bz2"));
	EXPECT_FALSE(check(dir / L"ArchiverCode/test/abcde.xz"));
	EXPECT_FALSE(check(dir / L"ArchiverCode/test/abcde.lzma"));
	EXPECT_FALSE(check(dir / L"ArchiverCode/test/abcde.zst"));
	EXPECT_FALSE(check(dir / L"ArchiverCode/test/abcde.lz4"));

	//EXPECT_FALSE(check(__FILEW__));
	//EXPECT_FALSE(check(L"some_non_existing_file"));

	EXPECT_TRUE(check(dir / L"test/test_extract.zip"));
	EXPECT_TRUE(check(dir / L"test/test_extract.zipx"));
	EXPECT_FALSE(check(dir / L"test/test.lzh"));
	EXPECT_TRUE(check(dir / L"test/test.tar.gz"));
}


TEST(CLFArchiveLA, get_format)
{
	auto pp = std::make_shared<CLFPassphraseNULL>();
	CLFArchiveLA a;

	a.read_open(LF_PROJECT_DIR() / L"test/test_extract.zip", pp);
	EXPECT_EQ(LF_ARCHIVE_FORMAT::ZIP, a.get_format());

	a.read_open(LF_PROJECT_DIR() / L"test/test.lzh", pp);
	EXPECT_EQ(LF_ARCHIVE_FORMAT::READONLY, a.get_format());

	a.read_open(LF_PROJECT_DIR() / L"test/test_gzip.gz", pp);
	EXPECT_EQ(LF_ARCHIVE_FORMAT::GZ, a.get_format());

	a.read_open(LF_PROJECT_DIR() / L"test/test.tar.gz", pp);
	EXPECT_EQ(LF_ARCHIVE_FORMAT::TAR_GZ, a.get_format());

	a.read_open(LF_PROJECT_DIR() / L"ArchiverCode/test/multistream.txt.bz2", pp);
	EXPECT_EQ(LF_ARCHIVE_FORMAT::BZ2, a.get_format());

	a.read_open(LF_PROJECT_DIR() / L"ArchiverCode/test/abcde.lzma", pp);
	EXPECT_EQ(LF_ARCHIVE_FORMAT::LZMA, a.get_format());

	a.read_open(LF_PROJECT_DIR() / L"ArchiverCode/test/abcde.xz", pp);
	EXPECT_EQ(LF_ARCHIVE_FORMAT::XZ, a.get_format());

	a.read_open(LF_PROJECT_DIR() / L"ArchiverCode/test/abcde.zst", pp);
	EXPECT_EQ(LF_ARCHIVE_FORMAT::ZSTD, a.get_format());

	a.read_open(LF_PROJECT_DIR() / L"ArchiverCode/test/abcde.lz4", pp);
	EXPECT_EQ(LF_ARCHIVE_FORMAT::LZ4, a.get_format());

	a.read_open(LF_PROJECT_DIR() / L"ArchiverCode/test/test_2099.tar.zst", pp);
	EXPECT_EQ(LF_ARCHIVE_FORMAT::TAR_ZSTD, a.get_format());

	a.read_open(LF_PROJECT_DIR() / L"ArchiverCode/test/test_2099.tar.lz4", pp);
	EXPECT_EQ(LF_ARCHIVE_FORMAT::TAR_LZ4, a.get_format());
}


TEST(CLFArchiveLA, get_format_name)
{
	auto temp = UtilGetTemporaryFileName();
	{
		CLFArchiveLA a;
		auto pp = std::make_shared<CLFPassphraseNULL>();
		a.read_open(LF_PROJECT_DIR() / L"test/test_extract.zip", pp);
		EXPECT_EQ(L"ZIP 1.0 (uncompressed)", a.get_format_name());

		LF_COMPRESS_ARGS args;
		args.load(CConfigFile());
		a.write_open(temp, LF_ARCHIVE_FORMAT::ZIP, LF_WOPT_STANDARD, args, pp);
		EXPECT_EQ(L"ZIP 1.0 (uncompressed)", a.get_format_name());
	}
	UtilDeletePath(temp);
	EXPECT_FALSE(std::filesystem::exists(temp));
}


TEST(CLFArchiveLA, read_entry)
{
	_wsetlocale(LC_ALL, L"");	//default locale
	{
		CLFArchiveLA a;
		auto pp = std::make_shared<CLFPassphraseNULL>();
		a.read_open(LF_PROJECT_DIR() / L"test/test_extract.zip", pp);
		auto entry = a.read_entry_begin();
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
		EXPECT_EQ(L"かきくけこ/file3.txt", entry->path);

		entry = a.read_entry_next();
		EXPECT_EQ(nullptr, entry);
	}

	{
		CLFArchiveLA a;
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
}


TEST(CLFArchiveLA, add_entry)
{
	auto temp = UtilGetTemporaryFileName();
	auto src = UtilGetTemporaryFileName();
	{
		CAutoFile f;
		f.open(src, L"w");
		fputs("abcde12345", f);
	}
	{
		CLFArchiveLA a;
		LF_COMPRESS_ARGS args;
		args.load(CConfigFile());
		auto pp = std::make_shared<CLFPassphraseNULL>();
		a.write_open(temp, LF_ARCHIVE_FORMAT::ZIP, LF_WOPT_STANDARD, args, pp);
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
	}
	{
		CLFArchiveLA a;
		auto pp = std::make_shared<CLFPassphraseNULL>();
		a.read_open(temp, pp);
		auto entry = a.read_entry_begin();
		EXPECT_NE(nullptr, entry);
		EXPECT_EQ(L"test/", entry->path);

		entry = a.read_entry_next();
		EXPECT_NE(nullptr, entry);
		EXPECT_EQ(L"test/file.txt", entry->path);
		EXPECT_EQ(10, entry->stat.st_size);
	}
	UtilDeletePath(temp);
	EXPECT_FALSE(std::filesystem::exists(temp));
	UtilDeletePath(src);
	EXPECT_FALSE(std::filesystem::exists(src));
}


TEST(CLFArchiveLA, make_copy_archive)
{
	_wsetlocale(LC_ALL, L"");	//default locale
	{
		auto temp = UtilGetTemporaryFileName();
		{
			CLFArchiveLA a;
			auto pp = std::make_shared<CLFPassphraseNULL>();
			a.read_open(LF_PROJECT_DIR() / L"test/test_extract.zip", pp);
			{
				LF_COMPRESS_ARGS args;
				args.load(CConfigFile());
				auto out = a.make_copy_archive(temp, args, [](const LF_ENTRY_STAT&) {return true; });
			}
			a.read_open(temp, pp);

			auto entry = a.read_entry_begin();
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
			EXPECT_EQ(L"かきくけこ/file3.txt", entry->path);

			entry = a.read_entry_next();
			EXPECT_EQ(nullptr, entry);
		}
		EXPECT_NO_THROW({
			ARCLOG arcLog;
			CLFProgressHandlerNULL progressHandler;
			testOneArchive(temp, arcLog, progressHandler, std::make_shared<CLFPassphraseNULL>());
			});
		UtilDeletePath(temp);
		EXPECT_FALSE(std::filesystem::exists(temp));
	}
	{
		auto temp = UtilGetTemporaryFileName();
		{
			CLFArchiveLA a;
			auto passphrase = std::make_shared<CLFPassphraseConst>(L"abcde");
			a.read_open(LF_PROJECT_DIR() / L"test/test_password_abcde.zip", passphrase);
			{
				LF_COMPRESS_ARGS args;
				args.load(CConfigFile());
				auto out = a.make_copy_archive(temp, args, [](const LF_ENTRY_STAT&) {return true; });
			}
			a.read_open(temp, passphrase);

			auto entry = a.read_entry_begin();
			EXPECT_NE(nullptr, entry);
			EXPECT_EQ(L"test.txt", entry->path);

			entry = a.read_entry_next();
			EXPECT_EQ(nullptr, entry);
		}
		EXPECT_NO_THROW({
			ARCLOG arcLog;
			CLFProgressHandlerNULL progressHandler;
			testOneArchive(temp, arcLog, progressHandler, std::make_shared<CLFPassphraseConst>(L"abcde"));
			});
		UtilDeletePath(temp);
		EXPECT_FALSE(std::filesystem::exists(temp));
	}
}


TEST(CLFArchiveLA, is_known_format)
{
	const auto dir = LF_PROJECT_DIR() / L"ArchiverCode/test";
	EXPECT_TRUE(CLFArchiveLA::is_known_format(dir / L"empty.gz"));
	EXPECT_TRUE(CLFArchiveLA::is_known_format(dir / L"empty.bz2"));
	EXPECT_TRUE(CLFArchiveLA::is_known_format(dir / L"empty.xz"));
	EXPECT_TRUE(CLFArchiveLA::is_known_format(dir / L"empty.lzma"));
	EXPECT_TRUE(CLFArchiveLA::is_known_format(dir / L"empty.zst"));

	EXPECT_TRUE(CLFArchiveLA::is_known_format(dir / L"abcde.gz"));
	EXPECT_TRUE(CLFArchiveLA::is_known_format(dir / L"abcde.bz2"));
	EXPECT_TRUE(CLFArchiveLA::is_known_format(dir / L"abcde.xz"));
	EXPECT_TRUE(CLFArchiveLA::is_known_format(dir / L"abcde.lzma"));
	EXPECT_TRUE(CLFArchiveLA::is_known_format(dir / L"abcde.zst"));
	EXPECT_TRUE(CLFArchiveLA::is_known_format(dir / L"abcde.lz4"));
	EXPECT_TRUE(CLFArchiveLA::is_known_format(dir / L"smile.cab"));
	EXPECT_TRUE(CLFArchiveLA::is_known_format(dir / L"smile2.cab"));
	EXPECT_TRUE(CLFArchiveLA::is_known_format(dir / L"test_2099.lzh"));

	EXPECT_FALSE(CLFArchiveLA::is_known_format(__FILEW__));
	EXPECT_FALSE(CLFArchiveLA::is_known_format(L"some_non_existing_file"));
	EXPECT_FALSE(CLFArchiveLA::is_known_format(dir / L"smile.png"));
	EXPECT_FALSE(CLFArchiveLA::is_known_format(dir / L"smile.gif"));
	EXPECT_FALSE(CLFArchiveLA::is_known_format(dir / L"smile.jpg"));
}

TEST(CLFArchiveLA, read_enum_2099_lzh)
{
	_wsetlocale(LC_ALL, L"");	//default locale
	const auto file = std::filesystem::path(__FILEW__).parent_path() / L"test_2099.lzh";

	CLFArchiveLA a;
	auto pp = std::make_shared<CLFPassphraseNULL>();
	a.read_open(file, pp);
	EXPECT_FALSE(a.is_modify_supported());
	ASSERT_EQ(L"lha -lh5-", a.get_format_name());


	int count = 0;
	int numDir = 0;
	for (auto entry = a.read_entry_begin(); entry; entry = a.read_entry_next()) {
		count++;
		if (entry->is_directory()) {
			numDir++;
		} else {
			if (entry->path.wstring().find(L"ccd.txt") != -1) {
				EXPECT_EQ(entry->stat.st_size, 44);
				EXPECT_EQ(entry->method_name, L"---");
				EXPECT_EQ(entry->compressed_size, -1);
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
				EXPECT_EQ(entry->method_name, L"---");
				EXPECT_EQ(entry->compressed_size, -1);
			}
		}
		EXPECT_FALSE(entry->is_encrypted);
	}
	EXPECT_EQ(count, 2099 + 1);
	EXPECT_EQ(numDir, 1);
}

TEST(CLFArchiveLA, read_enum_2099_zstd)
{
	_wsetlocale(LC_ALL, L"");	//default locale
	const auto file = std::filesystem::path(__FILEW__).parent_path() / L"test_2099.tar.zst";

	CLFArchiveLA a;
	auto pp = std::make_shared<CLFPassphraseNULL>();
	a.read_open(file, pp);
	EXPECT_TRUE(a.is_modify_supported());
	ASSERT_EQ(L"POSIX ustar format", a.get_format_name());


	int count = 0;
	int numDir = 0;
	for (auto entry = a.read_entry_begin(); entry; entry = a.read_entry_next()) {
		count++;
		if (entry->is_directory()) {
			numDir++;
		} else {
			if (entry->path.wstring().find(L"ccd.txt") != -1) {
				EXPECT_EQ(entry->stat.st_size, 44);
				EXPECT_EQ(entry->method_name, L"---");
				EXPECT_EQ(entry->compressed_size, -1);
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
				EXPECT_EQ(entry->method_name, L"---");
				EXPECT_EQ(entry->compressed_size, -1);
			}
		}
		EXPECT_FALSE(entry->is_encrypted);
	}
	EXPECT_EQ(count, 2099 + 1);
	EXPECT_EQ(numDir, 1);
}

TEST(CLFArchiveLA, name_in_zstd)
{
	_wsetlocale(LC_ALL, L"");	//default locale
	const auto dir = LF_PROJECT_DIR() / L"ArchiverCode/test";
	auto file = dir / L"abcde.zst";
	EXPECT_TRUE(CLFArchiveLA::is_known_format(file));

	CLFArchiveLA a;
	auto pp = std::make_shared<CLFPassphraseNULL>();
	a.read_open(file, pp);
	int count = 0;
	for (auto entry = a.read_entry_begin(); entry; entry = a.read_entry_next()) {
		count++;
		EXPECT_FALSE(entry->is_directory());
		EXPECT_EQ(L"abcde", entry->path.wstring());	//this will be "data" in libarchive
		EXPECT_EQ(entry->stat.st_size, 0);	//cannot get file size
		EXPECT_EQ(entry->method_name, L"---");
		EXPECT_EQ(entry->compressed_size, -1);
		EXPECT_FALSE(entry->is_encrypted);
	}
	EXPECT_EQ(1, count);
}

TEST(CLFArchiveLA, name_in_gzip)
{
	_wsetlocale(LC_ALL, L"");	//default locale
	const auto dir = LF_PROJECT_DIR() / L"ArchiverCode/test";
	auto file = dir / L"abcde.gz";
	EXPECT_TRUE(CLFArchiveLA::is_known_format(file));

	CLFArchiveLA a;
	auto pp = std::make_shared<CLFPassphraseNULL>();
	a.read_open(file, pp);
	int count = 0;
	for (auto entry = a.read_entry_begin(); entry; entry = a.read_entry_next()) {
		count++;
		EXPECT_FALSE(entry->is_directory());
		EXPECT_EQ(L"abcde", entry->path.wstring());	//this will be "data" in libarchive
		EXPECT_EQ(entry->stat.st_size, 0);	//cannot get file size
		EXPECT_EQ(entry->method_name, L"---");
		EXPECT_EQ(entry->compressed_size, -1);
		EXPECT_FALSE(entry->is_encrypted);
	}
	EXPECT_EQ(1, count);
}

TEST(CLFArchiveLA, name_in_xz)
{
	_wsetlocale(LC_ALL, L"");	//default locale
	const auto dir = LF_PROJECT_DIR() / L"ArchiverCode/test";
	auto file = dir / L"abcde.xz";
	EXPECT_TRUE(CLFArchiveLA::is_known_format(file));

	CLFArchiveLA a;
	auto pp = std::make_shared<CLFPassphraseNULL>();
	a.read_open(file, pp);
	int count = 0;
	for (auto entry = a.read_entry_begin(); entry; entry = a.read_entry_next()) {
		count++;
		EXPECT_FALSE(entry->is_directory());
		EXPECT_EQ(L"abcde", entry->path.wstring());	//this will be "data" in libarchive
		EXPECT_EQ(entry->stat.st_size, 0);	//cannot get file size
		EXPECT_EQ(entry->method_name, L"---");
		EXPECT_EQ(entry->compressed_size, -1);
		EXPECT_FALSE(entry->is_encrypted);
	}
	EXPECT_EQ(1, count);
}

TEST(CLFArchiveLA, name_in_lzma)
{
	_wsetlocale(LC_ALL, L"");	//default locale
	const auto dir = LF_PROJECT_DIR() / L"ArchiverCode/test";
	auto file = dir / L"abcde.lzma";
	EXPECT_TRUE(CLFArchiveLA::is_known_format(file));

	CLFArchiveLA a;
	auto pp = std::make_shared<CLFPassphraseNULL>();
	a.read_open(file, pp);
	int count = 0;
	for (auto entry = a.read_entry_begin(); entry; entry = a.read_entry_next()) {
		count++;
		EXPECT_FALSE(entry->is_directory());
		EXPECT_EQ(L"abcde", entry->path.wstring());	//this will be "data" in libarchive
		EXPECT_EQ(entry->stat.st_size, 0);	//cannot get file size
		EXPECT_EQ(entry->method_name, L"---");
		EXPECT_EQ(entry->compressed_size, -1);
		EXPECT_FALSE(entry->is_encrypted);
	}
	EXPECT_EQ(1, count);
}

TEST(CLFArchiveLA, name_in_bz2)
{
	_wsetlocale(LC_ALL, L"");	//default locale
	const auto dir = LF_PROJECT_DIR() / L"ArchiverCode/test";
	auto file = dir / L"abcde.bz2";
	EXPECT_TRUE(CLFArchiveLA::is_known_format(file));

	CLFArchiveLA a;
	auto pp = std::make_shared<CLFPassphraseNULL>();
	a.read_open(file, pp);
	int count = 0;
	for (auto entry = a.read_entry_begin(); entry; entry = a.read_entry_next()) {
		count++;
		EXPECT_FALSE(entry->is_directory());
		EXPECT_EQ(L"abcde", entry->path.wstring());
		EXPECT_EQ(entry->stat.st_size, 0);	//cannot get file size
		EXPECT_EQ(entry->method_name, L"---");
		EXPECT_EQ(entry->compressed_size, -1);
		EXPECT_FALSE(entry->is_encrypted);
	}
	EXPECT_EQ(1, count);
}

TEST(CLFArchiveLA, capability_cab)
{
	_wsetlocale(LC_ALL, L"");	//default locale
	const auto dir = LF_PROJECT_DIR() / L"ArchiverCode/test";
	auto file = dir / L"smile.cab";
	EXPECT_TRUE(CLFArchiveLA::is_known_format(file));

	CLFArchiveLA a;
	auto pp = std::make_shared<CLFPassphraseNULL>();
	a.read_open(file, pp);
	int count = 0;
	for (auto entry = a.read_entry_begin(); entry; entry = a.read_entry_next()) {
		count++;
		EXPECT_FALSE(entry->is_directory());
		EXPECT_EQ(L"smile.bmp", entry->path.wstring());
		EXPECT_EQ(entry->stat.st_size, 6110262);
		EXPECT_EQ(entry->method_name, L"---");
		EXPECT_EQ(entry->compressed_size, -1);
		EXPECT_FALSE(entry->is_encrypted);
	}
	EXPECT_EQ(1, count);
}

//this format is not supported now
TEST(CLFArchiveLA, capability_cab2)
{
	_wsetlocale(LC_ALL, L"");	//default locale
	const auto dir = LF_PROJECT_DIR() / L"ArchiverCode/test";
	auto file = dir / L"smile2.cab";
	EXPECT_TRUE(CLFArchiveLA::is_known_format(file));

	EXPECT_ANY_THROW({
		CLFArchiveLA a;
		auto pp = std::make_shared<CLFPassphraseNULL>();
		a.read_open(file, pp);
		int count = 0;
		for (auto entry = a.read_entry_begin(); entry; entry = a.read_entry_next()) {
			count++;
			//EXPECT_FALSE(entry->is_directory());
			//EXPECT_EQ(L"smile.bmp", entry->path.wstring());
			//EXPECT_EQ(entry->stat.st_size, 6110262);
			//EXPECT_EQ(entry->method_name, L"---");
			//EXPECT_EQ(entry->compressed_size, -1);
			//EXPECT_FALSE(entry->is_encrypted);
		}
		//EXPECT_EQ(1, count);
	});
}

