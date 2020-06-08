#include "stdafx.h"
#ifdef UNIT_TEST
#include <gtest/gtest.h>
#include "compress.h"

TEST(compress, getSourcesBasePath) {
	std::wstring getSourcesBasePath(const std::vector<std::wstring> &sources);
	EXPECT_EQ(L"", getSourcesBasePath(std::vector<std::wstring>({ })));
	EXPECT_EQ(L"abc", getSourcesBasePath(std::vector<std::wstring>({ L"abc/" })));
	EXPECT_EQ(L"abc", getSourcesBasePath(std::vector<std::wstring>({ L"abc/",L"ABC/ghi/" })));
	EXPECT_EQ(L"", getSourcesBasePath(std::vector<std::wstring>({ L"abc",L"ghi/" })));
	EXPECT_EQ(L"abc", getSourcesBasePath(std::vector<std::wstring>({ L"abc",L"abc/" })));
	EXPECT_EQ(L"c:/abc", getSourcesBasePath(std::vector<std::wstring>({ L"c:/abc",L"c:/abc/def" })));
}

TEST(compress, getArchiveFileExtension) {
	std::wstring getArchiveFileExtension(LF_ARCHIVE_FORMAT fmt, LF_WRITE_OPTIONS option, const std::wstring& original_path);
	const wchar_t* path = L"abc.ext";
	EXPECT_EQ(L".zip", getArchiveFileExtension(LF_FMT_ZIP, LF_WOPT_STANDARD, path));
	EXPECT_EQ(L".7z", getArchiveFileExtension(LF_FMT_7Z, LF_WOPT_STANDARD, path));
	EXPECT_EQ(L".ext.gz", getArchiveFileExtension(LF_FMT_GZ, LF_WOPT_STANDARD, path));
	EXPECT_EQ(L".ext.bz2", getArchiveFileExtension(LF_FMT_BZ2, LF_WOPT_STANDARD, path));
	EXPECT_EQ(L".ext.lzma", getArchiveFileExtension(LF_FMT_LZMA, LF_WOPT_STANDARD, path));
	EXPECT_EQ(L".ext.xz", getArchiveFileExtension(LF_FMT_XZ, LF_WOPT_STANDARD, path));
	EXPECT_EQ(L".ext.z", getArchiveFileExtension(LF_FMT_Z, LF_WOPT_STANDARD, path));
	EXPECT_EQ(L".tar", getArchiveFileExtension(LF_FMT_TAR, LF_WOPT_STANDARD, path));
	EXPECT_EQ(L".tar.gz", getArchiveFileExtension(LF_FMT_TAR_GZ, LF_WOPT_STANDARD, path));
	EXPECT_EQ(L".tar.bz2", getArchiveFileExtension(LF_FMT_TAR_BZ2, LF_WOPT_STANDARD, path));
	EXPECT_EQ(L".tar.lzma", getArchiveFileExtension(LF_FMT_TAR_LZMA, LF_WOPT_STANDARD, path));
	EXPECT_EQ(L".tar.xz", getArchiveFileExtension(LF_FMT_TAR_XZ, LF_WOPT_STANDARD, path));
	EXPECT_EQ(L".tar.z", getArchiveFileExtension(LF_FMT_TAR_Z, LF_WOPT_STANDARD, path));
	EXPECT_EQ(L".uue", getArchiveFileExtension(LF_FMT_UUE, LF_WOPT_STANDARD, path));


	EXPECT_EQ(L".exe", getArchiveFileExtension(LF_FMT_ZIP, LF_WOPT_SFX, path));
	EXPECT_EQ(L".exe", getArchiveFileExtension(LF_FMT_7Z, LF_WOPT_SFX, path));
	EXPECT_EQ(L".ext.exe", getArchiveFileExtension(LF_FMT_GZ, LF_WOPT_SFX, path));
	EXPECT_EQ(L".ext.exe", getArchiveFileExtension(LF_FMT_BZ2, LF_WOPT_SFX, path));
	EXPECT_EQ(L".ext.exe", getArchiveFileExtension(LF_FMT_LZMA, LF_WOPT_SFX, path));
	EXPECT_EQ(L".ext.exe", getArchiveFileExtension(LF_FMT_XZ, LF_WOPT_SFX, path));
	EXPECT_EQ(L".ext.exe", getArchiveFileExtension(LF_FMT_Z, LF_WOPT_SFX, path));
	EXPECT_EQ(L".exe", getArchiveFileExtension(LF_FMT_TAR, LF_WOPT_SFX, path));
	EXPECT_EQ(L".exe", getArchiveFileExtension(LF_FMT_TAR_GZ, LF_WOPT_SFX, path));
	EXPECT_EQ(L".exe", getArchiveFileExtension(LF_FMT_TAR_BZ2, LF_WOPT_SFX, path));
	EXPECT_EQ(L".exe", getArchiveFileExtension(LF_FMT_TAR_LZMA, LF_WOPT_SFX, path));
	EXPECT_EQ(L".exe", getArchiveFileExtension(LF_FMT_TAR_XZ, LF_WOPT_SFX, path));
	EXPECT_EQ(L".exe", getArchiveFileExtension(LF_FMT_TAR_Z, LF_WOPT_SFX, path));
	EXPECT_EQ(L".exe", getArchiveFileExtension(LF_FMT_UUE, LF_WOPT_SFX, path));
}


TEST(compress, volumeLabelToDirectoryName) {
	std::wstring volumeLabelToDirectoryName(const std::wstring& volume_label);
	EXPECT_EQ(L"a_b_c_d_e_f_g_h_i_j", volumeLabelToDirectoryName(L"a/b\\c:d*e?f\"g<h>i|j"));
}

#endif

