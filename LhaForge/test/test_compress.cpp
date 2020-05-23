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
	EXPECT_EQ(L"c:/abc", getBasePath(std::vector<std::wstring>({ L"c:/abc",L"c:/abc/def" })));
}

TEST(compress, volumeLabelToDirectoryName) {
	std::wstring volumeLabelToDirectoryName(const std::wstring& volume_label);
	EXPECT_EQ(L"a_b_c_d_e_f_g_h_i_j", volumeLabelToDirectoryName(L"a/b\\c:d*e?f\"g<h>i|j"));
}

#endif

