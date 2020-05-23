#include "stdafx.h"
#ifdef UNIT_TEST
#include <gtest/gtest.h>
#include "CommonUtil.h"
#include "ConfigCode/ConfigGeneral.h"
#include "Utilities/FileOperation.h"

TEST(CommonUtil, LF_get_output_dir) {
	struct LF_GET_OUTPUT_DIR_TEST_CALLBACK :I_LF_GET_OUTPUT_DIR_CALLBACK {
		std::wstring _default_path;
		void setArchivePath(const wchar_t* archivePath) {
			if (_default_path.empty()) {
				_default_path = std::filesystem::path(archivePath).parent_path();
			}
		}
		std::wstring operator()()override {
			return _default_path;
		}
	};
	LF_GET_OUTPUT_DIR_TEST_CALLBACK output_dir_callback;
	output_dir_callback.setArchivePath(L"C:/path_to/test_archive.ext");
	auto outputDir = LF_get_output_dir(OUTPUT_TO_SAME_DIR, L"C:/path_to/test_archive.ext", L"", output_dir_callback);
	EXPECT_EQ(L"C:/path_to", outputDir);
}
TEST(CommonUtil, LF_confirm_output_dir_type) {
	CConfigGeneral conf;
	conf.WarnRemovable = false;
	conf.WarnNetwork = false;
	EXPECT_TRUE(LF_confirm_output_dir_type(conf, L"C:/"));
	EXPECT_TRUE(LF_confirm_output_dir_type(conf, L"C:/temp"));
}
TEST(CommonUtil, LF_ask_and_make_sure_output_dir_exists) {
	auto target = UtilGetTempPath() + L"make_sure_test";
	EXPECT_FALSE(std::filesystem::exists(target));
	EXPECT_THROW(LF_ask_and_make_sure_output_dir_exists(target.c_str(), LOSTDIR::LOSTDIR_ERROR), LF_EXCEPTION);
	EXPECT_FALSE(std::filesystem::exists(target));
	LF_ask_and_make_sure_output_dir_exists(target.c_str(), LOSTDIR::LOSTDIR_FORCE_CREATE);
	EXPECT_TRUE(std::filesystem::exists(target));
	UtilDeleteDir(target, true);
}
TEST(CommonUtil, LF_make_expand_information) {
	const auto open_dir = LR"(C:\test\)";
	const auto output_path = LR"(D:\test\output.ext)";
	auto envInfo = LF_make_expand_information(open_dir, output_path);
	EXPECT_TRUE(has_key(envInfo, L"%PATH%"));
	EXPECT_EQ(UtilGetModulePath(), envInfo[L"ProgramPath"]);
	EXPECT_EQ(std::filesystem::path(UtilGetModulePath()).parent_path().wstring(), envInfo[L"ProgramDir"]);

	EXPECT_EQ(open_dir, envInfo[L"dir"]);
	EXPECT_EQ(open_dir, envInfo[L"OutputDir"]);
	EXPECT_EQ(L"C:", envInfo[L"OutputDrive"]);

	EXPECT_EQ(output_path, envInfo[L"OutputFile"]);
	EXPECT_EQ(L"output.ext", envInfo[L"OutputFileName"]);
}

TEST(CommonUtil, LF_sanitize_pathname) {
	EXPECT_EQ(L"", LF_sanitize_pathname(L""));
	EXPECT_EQ(L"", LF_sanitize_pathname(L"//"));
	EXPECT_EQ(L"", LF_sanitize_pathname(L"\\"));
	EXPECT_EQ(L"a", LF_sanitize_pathname(L"/a"));
	EXPECT_EQ(L"a", LF_sanitize_pathname(L"//a"));
	EXPECT_EQ(L"a", LF_sanitize_pathname(L"\\a"));
	EXPECT_EQ(L"a/", LF_sanitize_pathname(L"//a////"));
	EXPECT_EQ(L"a/b", LF_sanitize_pathname(L"a\\b"));
	EXPECT_EQ(L"a/b", LF_sanitize_pathname(L"a//b"));
	EXPECT_EQ(L"a/b", LF_sanitize_pathname(L"a/./././b"));
	EXPECT_EQ(L"c/_@@@_/d", LF_sanitize_pathname(L"c/../d"));
	EXPECT_EQ(L"e/_@@@_/f", LF_sanitize_pathname(L"e/....../f"));
	EXPECT_EQ(L"a/b/c", LF_sanitize_pathname(L"a/b/c"));
	EXPECT_EQ(L"a/b/c/", LF_sanitize_pathname(L"a/b/c/"));

	EXPECT_EQ(L"abc_(UNICODE_CTRL)_def", LF_sanitize_pathname(L"abc\u202Edef"));

	EXPECT_EQ(L"あいうえお", LF_sanitize_pathname(L"あいうえお"));
	EXPECT_EQ(L"あいう/えお", LF_sanitize_pathname(L"あいう//えお"));

	EXPECT_EQ(L"c_/", LF_sanitize_pathname(L"c:/"));
	EXPECT_EQ(L"c_/AUX_/", LF_sanitize_pathname(L"c:/AUX/"));
	EXPECT_EQ(L"c_/AUX_", LF_sanitize_pathname(L"c:/AUX"));
	EXPECT_EQ(L"AUX_", LF_sanitize_pathname(L"AUX"));

	EXPECT_EQ(L"c_/com1_/", LF_sanitize_pathname(L"c:/com1/"));
	EXPECT_EQ(L"c_/CON_/", LF_sanitize_pathname(L"c:/CON/"));
	EXPECT_EQ(L"c_/lpt1_/", LF_sanitize_pathname(L"c:/lpt1/"));
	EXPECT_EQ(L"c_/nul_/", LF_sanitize_pathname(L"c:/nul/"));
	EXPECT_EQ(L"c_/PRN_/", LF_sanitize_pathname(L"c:/PRN/"));
	EXPECT_EQ(L"c_/COM1_/CON_/PRN_/", LF_sanitize_pathname(L"c:/COM1/CON/PRN/"));

	EXPECT_EQ(L"_______", LF_sanitize_pathname(L":*?\"<>|"));
}


#endif
