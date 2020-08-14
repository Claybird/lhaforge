#include "stdafx.h"
#ifdef UNIT_TEST
#include <gtest/gtest.h>
#include "ArchiverCode//arc_interface.h"

TEST(ARCHIVE_FILE_TO_READ, isKnownFormat)
{
	const auto dir = std::filesystem::path(__FILEW__).parent_path();
	EXPECT_TRUE(ARCHIVE_FILE_TO_READ::isKnownFormat(dir / L"empty.gz"));
	EXPECT_TRUE(ARCHIVE_FILE_TO_READ::isKnownFormat(dir / L"empty.bz2"));
	EXPECT_TRUE(ARCHIVE_FILE_TO_READ::isKnownFormat(dir / L"empty.xz"));
	EXPECT_TRUE(ARCHIVE_FILE_TO_READ::isKnownFormat(dir / L"empty.lzma"));
	EXPECT_TRUE(ARCHIVE_FILE_TO_READ::isKnownFormat(dir / L"empty.zst"));

	EXPECT_TRUE(ARCHIVE_FILE_TO_READ::isKnownFormat(dir / L"abcde.gz"));
	EXPECT_TRUE(ARCHIVE_FILE_TO_READ::isKnownFormat(dir / L"abcde.bz2"));
	EXPECT_TRUE(ARCHIVE_FILE_TO_READ::isKnownFormat(dir / L"abcde.xz"));
	EXPECT_TRUE(ARCHIVE_FILE_TO_READ::isKnownFormat(dir / L"abcde.lzma"));
	EXPECT_TRUE(ARCHIVE_FILE_TO_READ::isKnownFormat(dir / L"abcde.zst"));

	EXPECT_FALSE(ARCHIVE_FILE_TO_READ::isKnownFormat(__FILEW__));
}

#endif
