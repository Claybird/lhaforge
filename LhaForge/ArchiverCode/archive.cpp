#include "stdafx.h"
#include "archive.h"
#include "archive_libarchive.h"

std::unique_ptr<ILFArchiveFile> guessSuitableArchiver(const std::filesystem::path& path)
{
	if (CLFArchiveLA::is_known_format(path))return std::make_unique<CLFArchiveLA>();
	RAISE_EXCEPTION(L"Unknown format");
}

#ifdef UNIT_TEST
TEST(archive, guessSuitableArchiver)
{
	const auto dir = LF_PROJECT_DIR() / L"test";
	EXPECT_NO_THROW(guessSuitableArchiver(dir / L"test_extract.zip"));
	EXPECT_NO_THROW(guessSuitableArchiver(dir / L"test_broken_crc.zip"));
}

#endif

std::unique_ptr<ILFArchiveFile> guessSuitableArchiver(LF_ARCHIVE_FORMAT format)
{
	switch (format) {
	case LF_FMT_ZIP:
	case LF_FMT_7Z:
	case LF_FMT_GZ:
	case LF_FMT_BZ2:
	case LF_FMT_LZMA:
	case LF_FMT_XZ:
	case LF_FMT_ZSTD:
	case LF_FMT_TAR:
	case LF_FMT_TAR_GZ:
	case LF_FMT_TAR_BZ2:
	case LF_FMT_TAR_LZMA:
	case LF_FMT_TAR_XZ:
	case LF_FMT_TAR_ZSTD:
	case LF_FMT_UUE:
		return std::make_unique<CLFArchiveLA>();
	default:
		RAISE_EXCEPTION(L"Unknown format");
	}
}
#ifdef UNIT_TEST
TEST(archive, guessSuitableArchiver2)
{
	EXPECT_NO_THROW(guessSuitableArchiver(LF_FMT_ZIP));
	EXPECT_NO_THROW(guessSuitableArchiver(LF_FMT_7Z));
	EXPECT_NO_THROW(guessSuitableArchiver(LF_FMT_GZ));
	EXPECT_NO_THROW(guessSuitableArchiver(LF_FMT_BZ2));
	EXPECT_NO_THROW(guessSuitableArchiver(LF_FMT_LZMA));
	EXPECT_NO_THROW(guessSuitableArchiver(LF_FMT_XZ));
	EXPECT_NO_THROW(guessSuitableArchiver(LF_FMT_ZSTD));
	EXPECT_NO_THROW(guessSuitableArchiver(LF_FMT_TAR));
	EXPECT_NO_THROW(guessSuitableArchiver(LF_FMT_TAR_GZ));
	EXPECT_NO_THROW(guessSuitableArchiver(LF_FMT_TAR_BZ2));
	EXPECT_NO_THROW(guessSuitableArchiver(LF_FMT_TAR_LZMA));
	EXPECT_NO_THROW(guessSuitableArchiver(LF_FMT_TAR_XZ));
	EXPECT_NO_THROW(guessSuitableArchiver(LF_FMT_TAR_ZSTD));
	EXPECT_NO_THROW(guessSuitableArchiver(LF_FMT_UUE));
}
#endif

void CLFArchive::read_open(const std::filesystem::path& file, ILFPassphrase& passphrase)
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
	EXPECT_NO_THROW(a.read_open(dir / L"test_extract.zip", CLFPassphraseNULL()));
	EXPECT_NO_THROW(a.read_open(dir / L"test_broken_crc.zip", CLFPassphraseNULL()));
	EXPECT_NO_THROW(a.read_open(dir / L"test.lzh", CLFPassphraseNULL()));
}
#endif

void CLFArchive::write_open(
	const std::filesystem::path& file,
	LF_ARCHIVE_FORMAT format,
	LF_WRITE_OPTIONS options,
	const LF_COMPRESS_ARGS &args,
	ILFPassphrase& passphrase)
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
		EXPECT_NO_THROW(a.write_open(temp / L"test_write_open/test_write.zip", LF_FMT_ZIP, LF_WRITE_OPTIONS::LF_WOPT_STANDARD, arg, CLFPassphraseNULL()));
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
	EXPECT_EQ(L".zip", CLFArchive::get_compression_capability(LF_FMT_ZIP).formatExt(path, LF_WOPT_STANDARD));
	EXPECT_EQ(L".7z", CLFArchive::get_compression_capability(LF_FMT_7Z).formatExt(path, LF_WOPT_STANDARD));
	EXPECT_EQ(L".ext.gz", CLFArchive::get_compression_capability(LF_FMT_GZ).formatExt(path, LF_WOPT_STANDARD));
	EXPECT_EQ(L".ext.bz2", CLFArchive::get_compression_capability(LF_FMT_BZ2).formatExt(path, LF_WOPT_STANDARD));
	EXPECT_EQ(L".ext.lzma", CLFArchive::get_compression_capability(LF_FMT_LZMA).formatExt(path, LF_WOPT_STANDARD));
	EXPECT_EQ(L".ext.xz", CLFArchive::get_compression_capability(LF_FMT_XZ).formatExt(path, LF_WOPT_STANDARD));
	EXPECT_EQ(L".ext.zst", CLFArchive::get_compression_capability(LF_FMT_ZSTD).formatExt(path, LF_WOPT_STANDARD));
	EXPECT_EQ(L".tar", CLFArchive::get_compression_capability(LF_FMT_TAR).formatExt(path, LF_WOPT_STANDARD));
	EXPECT_EQ(L".tar.gz", CLFArchive::get_compression_capability(LF_FMT_TAR_GZ).formatExt(path, LF_WOPT_STANDARD));
	EXPECT_EQ(L".tar.bz2", CLFArchive::get_compression_capability(LF_FMT_TAR_BZ2).formatExt(path, LF_WOPT_STANDARD));
	EXPECT_EQ(L".tar.lzma", CLFArchive::get_compression_capability(LF_FMT_TAR_LZMA).formatExt(path, LF_WOPT_STANDARD));
	EXPECT_EQ(L".tar.xz", CLFArchive::get_compression_capability(LF_FMT_TAR_XZ).formatExt(path, LF_WOPT_STANDARD));
	EXPECT_EQ(L".tar.zst", CLFArchive::get_compression_capability(LF_FMT_TAR_ZSTD).formatExt(path, LF_WOPT_STANDARD));
	EXPECT_EQ(L".uue", CLFArchive::get_compression_capability(LF_FMT_UUE).formatExt(path, LF_WOPT_STANDARD));
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
	arc.read_open(dir / L"test_extract.zip", CLFPassphraseNULL());
	EXPECT_EQ(6, arc.get_num_entries());
	arc.read_open(dir / L"test_broken_crc.zip", CLFPassphraseNULL());
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
}
#endif

