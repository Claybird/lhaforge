#include "stdafx.h"
#ifdef UNIT_TEST
#include <gtest/gtest.h>
#include "ArchiverCode/archive.h"

TEST(CLFArchive, is_known_format)
{
	const auto dir = std::filesystem::path(__FILEW__).parent_path();

	EXPECT_FALSE(CLFArchive::is_known_format(__FILEW__));
	EXPECT_TRUE(CLFArchive::is_known_format(dir / L"test_extract.zip"));
	EXPECT_TRUE(CLFArchive::is_known_format(dir / L"test_broken_crc.zip"));
	EXPECT_FALSE(CLFArchive::is_known_format(dir / L"test_broken_file.zip"));
	EXPECT_FALSE(CLFArchive::is_known_format(L"some_non_existing_file"));
}

#endif
