#include "stdafx.h"
#ifdef UNIT_TEST
#include <gtest/gtest.h>
#include "compress.h"

TEST(compress, getBasePath) {
	std::wstring getBasePath(const std::vector<std::wstring> &files);
	EXPECT_EQ(L"", getBasePath(std::vector<std::wstring>({ })));
	EXPECT_EQ(L"abc", getBasePath(std::vector<std::wstring>({ L"abc/" })));
	EXPECT_EQ(L"abc", getBasePath(std::vector<std::wstring>({ L"abc/",L"ABC/ghi/" })));
	EXPECT_EQ(L"", getBasePath(std::vector<std::wstring>({ L"abc",L"ghi/" })));
	EXPECT_EQ(L"abc", getBasePath(std::vector<std::wstring>({ L"abc",L"abc/" })));
}
#endif

