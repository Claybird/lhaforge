#include "stdafx.h"
#ifdef UNIT_TEST
#include "ArchiverCode/archive.h"
#include "CommonUtil.h"

TEST(CLFArchive, is_known_format)
{
	const auto dir = LF_PROJECT_DIR();

	EXPECT_FALSE(CLFArchive::is_known_format(__FILEW__));
	EXPECT_TRUE(CLFArchive::is_known_format(dir / L"test_extract.zip"));
	EXPECT_TRUE(CLFArchive::is_known_format(dir / L"test_broken_crc.zip"));
	EXPECT_FALSE(CLFArchive::is_known_format(dir / L"test_broken_file.zip"));
	EXPECT_FALSE(CLFArchive::is_known_format(L"some_non_existing_file"));
}

TEST(CLFArchive, get_num_entries)
{
	const auto dir = LF_PROJECT_DIR();

	CLFArchive arc;
	arc.read_open(dir / L"test_extract.zip", CLFPassphraseNULL());
	EXPECT_EQ(6, arc.get_num_entries());
	arc.read_open(dir / L"test_broken_crc.zip", CLFPassphraseNULL());
	EXPECT_EQ(6, arc.get_num_entries());
}

TEST(LF_COMPRESS_CAPABILITY, formatExt)
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


	EXPECT_EQ(L".exe", CLFArchive::get_compression_capability(LF_FMT_ZIP).formatExt(path, LF_WOPT_SFX));
	EXPECT_EQ(L".exe", CLFArchive::get_compression_capability(LF_FMT_7Z).formatExt(path, LF_WOPT_SFX));
}

#endif
