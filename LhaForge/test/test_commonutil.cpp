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

#endif
