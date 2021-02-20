#include "stdafx.h"
#ifdef UNIT_TEST
#include <gtest/gtest.h>
#include "Utilities/FileOperation.h"

TEST(main, enumerateFiles)
{
	std::vector<std::wstring> enumerateFiles(const std::vector<std::wstring>& input, const std::vector<std::wstring>& denyExts);

	std::filesystem::path dir = UtilGetTempPath() + L"lhaforge_test/enumerateFiles";
	UtilDeletePath(dir);
	EXPECT_FALSE(std::filesystem::exists(dir));
	std::filesystem::create_directories(dir / L"abc");
	touchFile(dir / L"abc/ghi.txt");
	std::filesystem::create_directories(dir / L"def");
	touchFile(dir / L"def/test.exe");
	touchFile(dir / L"def/test.bat");

	auto out = enumerateFiles({ dir / L"abc", dir / L"def" }, { L".exe", L".bat" });
	EXPECT_EQ(1, out.size());
	if (out.size() > 0) {
		EXPECT_EQ(dir / L"abc/ghi.txt", out[0]);
	}

	UtilDeletePath(dir);
	EXPECT_FALSE(std::filesystem::exists(dir));
}

#endif
