#include "stdafx.h"
#include "archive.h"
#include "archive_libarchive.h"
#include "archive_bga.h"
#include "archive_arj.h"
#include "archive_zip.h"

#ifdef UNIT_TEST
TEST(archive, exceptions)
{
	try {
		RAISE_EXCEPTION(L"Unknown format");
		EXPECT_STREQ(L"This code", L"should never be called");
	} catch (const LF_EXCEPTION& e) {
		EXPECT_STREQ(L"Unknown format", e.what());
	}
	try {
		throw ARCHIVE_EXCEPTION(L"Unknown format");
	} catch (const LF_EXCEPTION& e) {
		EXPECT_STREQ(L"Unknown format", e.what());
	}

	try {
		throw ARCHIVE_EXCEPTION(EINVAL);
	} catch (const LF_EXCEPTION& e) {
		EXPECT_STREQ(L"Invalid argument", e.what());
	}
}
#endif

std::unique_ptr<ILFArchiveFile> guessSuitableArchiver(const std::filesystem::path& path)
{
	if (CLFArchiveZIP::is_known_format(path))return std::make_unique<CLFArchiveZIP>();
	if (CLFArchiveBGA::is_known_format(path))return std::make_unique<CLFArchiveBGA>();
	if (CLFArchiveARJ::is_known_format(path))return std::make_unique<CLFArchiveARJ>();

	//check for libarchive is weak, in the current implementation
	if (CLFArchiveLA::is_known_format(path))return std::make_unique<CLFArchiveLA>();
	RAISE_EXCEPTION(L"Unknown format");
}

#ifdef UNIT_TEST
#include "CommonUtil.h"
TEST(archive, guessSuitableArchiver)
{
	const auto dir = LF_PROJECT_DIR() / L"test";
	EXPECT_NO_THROW(guessSuitableArchiver(dir / L"test_extract.zip"));
	EXPECT_NO_THROW(guessSuitableArchiver(dir / L"test_extract.zipx"));
	EXPECT_NO_THROW(guessSuitableArchiver(dir / L"test_broken_crc.zip"));

	EXPECT_NO_THROW(guessSuitableArchiver(dir / L"smile.zip.001"));

	EXPECT_NO_THROW(guessSuitableArchiver(dir / L"test.gza"));
	{
		auto a = guessSuitableArchiver(dir / L"test.gza");
		auto pp = std::make_shared<CLFPassphraseNULL>();
		a->read_open(dir / L"test.gza", pp);
		a->read_entry_begin();
		EXPECT_EQ(L"BZA/GZA", a->get_format_name());
	}
	EXPECT_NO_THROW(guessSuitableArchiver(dir / L"test.bza"));
	{
		auto a = guessSuitableArchiver(dir / L"test.bza");
		auto pp = std::make_shared<CLFPassphraseNULL>();
		a->read_open(dir / L"test.bza", pp);
		a->read_entry_begin();
		EXPECT_EQ(L"BZA/GZA", a->get_format_name());
	}
	EXPECT_NO_THROW(guessSuitableArchiver(dir / L"test.arj"));
	{
		auto a = guessSuitableArchiver(dir / L"test.arj");
		auto pp = std::make_shared<CLFPassphraseNULL>();
		a->read_open(dir / L"test.arj", pp);
		a->read_entry_begin();
		EXPECT_EQ(L"ARJ", a->get_format_name());
	}
}

#endif

std::unique_ptr<ILFArchiveFile> guessSuitableArchiver(LF_ARCHIVE_FORMAT format)
{
	switch (format) {
	case LF_ARCHIVE_FORMAT::ZIP:
		return std::make_unique<CLFArchiveZIP>();
	case LF_ARCHIVE_FORMAT::_7Z:
	case LF_ARCHIVE_FORMAT::GZ:
	case LF_ARCHIVE_FORMAT::BZ2:
	case LF_ARCHIVE_FORMAT::LZMA:
	case LF_ARCHIVE_FORMAT::XZ:
	case LF_ARCHIVE_FORMAT::ZSTD:
	case LF_ARCHIVE_FORMAT::TAR:
	case LF_ARCHIVE_FORMAT::TAR_GZ:
	case LF_ARCHIVE_FORMAT::TAR_BZ2:
	case LF_ARCHIVE_FORMAT::TAR_LZMA:
	case LF_ARCHIVE_FORMAT::TAR_XZ:
	case LF_ARCHIVE_FORMAT::TAR_ZSTD:
		return std::make_unique<CLFArchiveLA>();
	default:
		RAISE_EXCEPTION(L"Unknown format");
	}
}
#ifdef UNIT_TEST
TEST(archive, guessSuitableArchiver2)
{
	EXPECT_NO_THROW(guessSuitableArchiver(LF_ARCHIVE_FORMAT::ZIP));
	EXPECT_NO_THROW(guessSuitableArchiver(LF_ARCHIVE_FORMAT::_7Z));
	EXPECT_NO_THROW(guessSuitableArchiver(LF_ARCHIVE_FORMAT::GZ));
	EXPECT_NO_THROW(guessSuitableArchiver(LF_ARCHIVE_FORMAT::BZ2));
	EXPECT_NO_THROW(guessSuitableArchiver(LF_ARCHIVE_FORMAT::LZMA));
	EXPECT_NO_THROW(guessSuitableArchiver(LF_ARCHIVE_FORMAT::XZ));
	EXPECT_NO_THROW(guessSuitableArchiver(LF_ARCHIVE_FORMAT::ZSTD));
	EXPECT_NO_THROW(guessSuitableArchiver(LF_ARCHIVE_FORMAT::TAR));
	EXPECT_NO_THROW(guessSuitableArchiver(LF_ARCHIVE_FORMAT::TAR_GZ));
	EXPECT_NO_THROW(guessSuitableArchiver(LF_ARCHIVE_FORMAT::TAR_BZ2));
	EXPECT_NO_THROW(guessSuitableArchiver(LF_ARCHIVE_FORMAT::TAR_LZMA));
	EXPECT_NO_THROW(guessSuitableArchiver(LF_ARCHIVE_FORMAT::TAR_XZ));
	EXPECT_NO_THROW(guessSuitableArchiver(LF_ARCHIVE_FORMAT::TAR_ZSTD));
}
#endif

void CLFArchive::read_open(const std::filesystem::path& file, std::shared_ptr<ILFPassphrase> passphrase)
{
	close();
	m_ptr = guessSuitableArchiver(file);
	m_ptr->read_open(file, passphrase);
}

#ifdef UNIT_TEST
#include "CommonUtil.h"
TEST(CLFArchive, read_open)
{
	CLFArchive a;
	const auto dir = LF_PROJECT_DIR() / L"test";
	auto pp = std::make_shared<CLFPassphraseNULL>();
	EXPECT_NO_THROW(a.read_open(dir / L"test_extract.zip", pp));
	EXPECT_NO_THROW(a.read_open(dir / L"test_extract.zipx", pp));
	EXPECT_NO_THROW(a.read_open(dir / L"test_broken_crc.zip", pp));
	EXPECT_NO_THROW(a.read_open(dir / L"test.lzh", pp));
}
#endif

void CLFArchive::write_open(
	const std::filesystem::path& file,
	LF_ARCHIVE_FORMAT format,
	LF_WRITE_OPTIONS options,
	const LF_COMPRESS_ARGS &args,
	std::shared_ptr<ILFPassphrase> passphrase)
{
	close();
	m_ptr = guessSuitableArchiver(format);
	m_ptr->write_open(file, format, options, args, passphrase);
}

#ifdef UNIT_TEST
#include "compress.h"
#include "ConfigCode/ConfigFile.h"
TEST(CLFArchive, write_open)
{
	auto temp = std::filesystem::path(UtilGetTempPath());
	LF_COMPRESS_ARGS arg;
	arg.load(CConfigFile());
	std::filesystem::create_directories(temp / L"test_write_open");
	EXPECT_TRUE(std::filesystem::exists(temp / L"test_write_open"));
	{
		CLFArchive a;
		auto pp = std::make_shared<CLFPassphraseNULL>();
		EXPECT_NO_THROW(a.write_open(temp / L"test_write_open/test_write.zip", LF_ARCHIVE_FORMAT::ZIP, LF_WRITE_OPTIONS::LF_WOPT_STANDARD, arg, pp));
	}
	UtilDeleteDir(temp / L"test_write_open", true);
	EXPECT_FALSE(std::filesystem::exists(temp / L"test_write_open"));
}
#endif

std::vector<LF_COMPRESS_CAPABILITY> CLFArchive::get_compression_capability()const
{
	std::vector<LF_COMPRESS_CAPABILITY> caps;
	auto capsLA = CLFArchiveLA().get_compression_capability();
	caps.insert(caps.end(), capsLA.begin(), capsLA.end());
	return caps;
}

LF_COMPRESS_CAPABILITY CLFArchive::get_compression_capability(LF_ARCHIVE_FORMAT format)
{
	auto arc = guessSuitableArchiver(format);
	std::vector<LF_COMPRESS_CAPABILITY> caps = arc->get_compression_capability();
	for (const auto cap : caps) {
		if (cap.format == format) {
			return cap;
		}
	}
	RAISE_EXCEPTION(L"Unknown format");
}

#ifdef UNIT_TEST
TEST(CLFArchive, get_compression_capability_formatExt)
{
	const wchar_t* path = L"abc.ext";
	EXPECT_EQ(L".zip", CLFArchive::get_compression_capability(LF_ARCHIVE_FORMAT::ZIP).formatExt(path, LF_WOPT_STANDARD));
	EXPECT_EQ(L".7z", CLFArchive::get_compression_capability(LF_ARCHIVE_FORMAT::_7Z).formatExt(path, LF_WOPT_STANDARD));
	EXPECT_EQ(L".ext.gz", CLFArchive::get_compression_capability(LF_ARCHIVE_FORMAT::GZ).formatExt(path, LF_WOPT_STANDARD));
	EXPECT_EQ(L".ext.bz2", CLFArchive::get_compression_capability(LF_ARCHIVE_FORMAT::BZ2).formatExt(path, LF_WOPT_STANDARD));
	EXPECT_EQ(L".ext.lzma", CLFArchive::get_compression_capability(LF_ARCHIVE_FORMAT::LZMA).formatExt(path, LF_WOPT_STANDARD));
	EXPECT_EQ(L".ext.xz", CLFArchive::get_compression_capability(LF_ARCHIVE_FORMAT::XZ).formatExt(path, LF_WOPT_STANDARD));
	EXPECT_EQ(L".ext.zst", CLFArchive::get_compression_capability(LF_ARCHIVE_FORMAT::ZSTD).formatExt(path, LF_WOPT_STANDARD));
	EXPECT_EQ(L".tar", CLFArchive::get_compression_capability(LF_ARCHIVE_FORMAT::TAR).formatExt(path, LF_WOPT_STANDARD));
	EXPECT_EQ(L".tar.gz", CLFArchive::get_compression_capability(LF_ARCHIVE_FORMAT::TAR_GZ).formatExt(path, LF_WOPT_STANDARD));
	EXPECT_EQ(L".tar.bz2", CLFArchive::get_compression_capability(LF_ARCHIVE_FORMAT::TAR_BZ2).formatExt(path, LF_WOPT_STANDARD));
	EXPECT_EQ(L".tar.lzma", CLFArchive::get_compression_capability(LF_ARCHIVE_FORMAT::TAR_LZMA).formatExt(path, LF_WOPT_STANDARD));
	EXPECT_EQ(L".tar.xz", CLFArchive::get_compression_capability(LF_ARCHIVE_FORMAT::TAR_XZ).formatExt(path, LF_WOPT_STANDARD));
	EXPECT_EQ(L".tar.zst", CLFArchive::get_compression_capability(LF_ARCHIVE_FORMAT::TAR_ZSTD).formatExt(path, LF_WOPT_STANDARD));
}
#endif

//-1 if no information is given
int64_t CLFArchive::get_num_entries()
{
	if (m_numEntries == -1) {
		//no cache available
		try {
			m_numEntries = 0;
			for (auto entry = read_entry_begin(); entry; entry = read_entry_next()) {
				m_numEntries++;
			}
		} catch (const LF_EXCEPTION&) {
			m_numEntries = -1;
		}
	}
	return m_numEntries;
}

#ifdef UNIT_TEST
#include "CommonUtil.h"
TEST(CLFArchive, get_num_entries)
{
	const auto dir = LF_PROJECT_DIR() / L"test";

	CLFArchive arc;
	auto pp = std::make_shared<CLFPassphraseNULL>();
	arc.read_open(dir / L"test_extract.zip", pp);
	EXPECT_EQ(6, arc.get_num_entries());
	arc.read_open(dir / L"test_broken_crc.zip", pp);
	EXPECT_EQ(6, arc.get_num_entries());
}
#endif

bool CLFArchive::is_known_format(const std::filesystem::path& path)
{
	try {
		guessSuitableArchiver(path);
		return true;
	} catch(...) {
		return false;
	}
	return false;
}

#ifdef UNIT_TEST
TEST(CLFArchive, is_known_format)
{
	const auto dir = LF_PROJECT_DIR() / L"test";

	EXPECT_FALSE(CLFArchive::is_known_format(__FILEW__));
	EXPECT_TRUE(CLFArchive::is_known_format(dir / L"test_extract.zip"));
	EXPECT_TRUE(CLFArchive::is_known_format(dir / L"test_broken_crc.zip"));
	EXPECT_TRUE(CLFArchive::is_known_format(dir / L"test_broken_file.zip"));
	EXPECT_FALSE(CLFArchive::is_known_format(L"some_non_existing_file"));

	EXPECT_TRUE(CLFArchive::is_known_format(dir / L"smile.zip.001"));
}

TEST(CLFArchive, extract_multipart)
{
	CLFArchive a;
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
}

void sub_rar_test(std::filesystem::path file)
{
	CLFArchive a;
	EXPECT_TRUE(a.is_known_format(file));

	auto pp = std::make_shared<CLFPassphraseConst>(L"password");
	a.read_open(file, pp);
	EXPECT_FALSE(a.is_modify_supported());
	EXPECT_EQ(L"RAR5", a.get_format_name());
	auto entry = a.read_entry_begin();
	EXPECT_NE(nullptr, entry);
	EXPECT_EQ(entry->path.wstring(), L"smile.bmp");
	EXPECT_FALSE(entry->is_directory());
	EXPECT_EQ(L"---", entry->method_name);
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
TEST(CLFArchive, extract_rar_solid)
{
	sub_rar_test(LF_PROJECT_DIR() / L"test/smile_solid.rar");
}

TEST(CLFArchive, extract_rar_locked)
{
	sub_rar_test(LF_PROJECT_DIR() / L"test/smile_locked.rar");
}

TEST(CLFArchive, extract_rar_encrypted)
{
	sub_rar_test(LF_PROJECT_DIR() / L"test/smile_encrypted.rar");
}

TEST(CLFArchive, extract_rar_header_encrypted)
{
	sub_rar_test(LF_PROJECT_DIR() / L"test/smile_header_encrypted.rar");
}

TEST(CLFArchive, extract_rar_multipart)
{
	sub_rar_test(LF_PROJECT_DIR() / L"test/smile.part0001.rar");
}


#endif

