#include "stdafx.h"
#ifdef UNIT_TEST
#include <gtest/gtest.h>
#include "ArchiverCode/archive.h"

TEST(main, LF_isKnownArchive)
{
	const auto dir = std::filesystem::path(__FILEW__).parent_path();

	EXPECT_FALSE(LF_isKnownArchive(__FILEW__));
	EXPECT_TRUE(LF_isKnownArchive(dir / L"test_extract.zip"));
	EXPECT_TRUE(LF_isKnownArchive(dir / L"test_broken_crc.zip"));
	EXPECT_FALSE(LF_isKnownArchive(dir / L"test_broken_file.zip"));
	EXPECT_FALSE(LF_isKnownArchive(L"some_non_existing_file"));
}

#endif
