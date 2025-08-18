/*
* MIT License

* Copyright (c) 2005- Claybird

* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:

* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.

* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/

#include "stdafx.h"
#include "CommonUtil.h"
#include "ConfigCode/ConfigGeneral.h"
#include "resource.h"
#include "Utilities/FileOperation.h"
#include "Utilities/Utility.h"
#include "Utilities/OSUtil.h"
#include "Utilities/CustomControl.h"

TEST(CommonUtil, LF_get_output_dir) {
	struct LF_GET_OUTPUT_DIR_TEST_CALLBACK :I_LF_GET_OUTPUT_DIR_CALLBACK {
		std::filesystem::path _default_path;
		void setArchivePath(const std::filesystem::path archivePath) {
			if (_default_path.empty()) {
				_default_path = archivePath.parent_path();
			}
		}
		std::filesystem::path operator()()override {
			return _default_path;
		}
	};
	LF_GET_OUTPUT_DIR_TEST_CALLBACK output_dir_callback;
	output_dir_callback.setArchivePath(L"C:/path_to/test_archive.ext");
	auto outputDir = LF_get_output_dir(OUTPUT_TO::SameDir, L"C:/path_to/test_archive.ext", L"", output_dir_callback);
	EXPECT_EQ(L"C:/path_to", outputDir);

	outputDir = LF_get_output_dir(OUTPUT_TO::AlwaysAsk, L"C:/path_to/test_archive.ext", L"", output_dir_callback);
	EXPECT_EQ(L"C:/path_to", outputDir);

	outputDir = LF_get_output_dir(OUTPUT_TO::Desktop, L"C:/path_to/test_archive.ext", L"", output_dir_callback);
	EXPECT_EQ(UtilGetDesktopPath(), outputDir);

	outputDir = LF_get_output_dir(OUTPUT_TO::SpecificDir, L"C:/path_to/test_archive.ext", L"Z:/path", output_dir_callback);
	EXPECT_EQ(L"Z:/path", outputDir);

	outputDir = LF_get_output_dir(OUTPUT_TO::SpecificDir, L"C:/path_to/test_archive.ext", L"", output_dir_callback);
	EXPECT_EQ(UtilGetDesktopPath(), outputDir);
}

TEST(CommonUtil, LF_confirm_output_dir_type) {
	CConfigGeneral conf;
	conf.WarnRemovable = false;
	conf.WarnNetwork = false;
	EXPECT_TRUE(LF_confirm_output_dir_type(conf, L"C:/"));
	EXPECT_TRUE(LF_confirm_output_dir_type(conf, L"C:/temp"));
	EXPECT_TRUE(LF_confirm_output_dir_type(conf, L"C:/some_non_exisintg_dir"));
}

TEST(CommonUtil, LF_ask_and_make_sure_output_dir_exists) {
	auto target = UtilGetTempPath() / L"make_sure_test";
	EXPECT_FALSE(std::filesystem::exists(target));
	EXPECT_THROW(LF_ask_and_make_sure_output_dir_exists(target.c_str(), LOSTDIR::Error), LF_EXCEPTION);
	EXPECT_FALSE(std::filesystem::exists(target));
	LF_ask_and_make_sure_output_dir_exists(target.c_str(), LOSTDIR::ForceCreate);
	EXPECT_TRUE(std::filesystem::exists(target));
	UtilDeleteDir(target, true);
}

TEST(CommonUtil, LF_make_expand_information) {
	const auto open_dir = LR"(C:\test\)";
	const auto output_path = LR"(D:\test\output.ext)";
	auto envInfo = LF_make_expand_information(open_dir, output_path);
	EXPECT_TRUE(has_key(envInfo, toLower(L"PATH")));
	EXPECT_TRUE(has_key(envInfo, toLower(L"tmp")));
	EXPECT_EQ(UtilGetModulePath(), envInfo[toLower(L"ProgramPath")]);
	EXPECT_EQ(std::filesystem::path(UtilGetModulePath()).parent_path().wstring(), envInfo[toLower(L"ProgramDir")]);

	EXPECT_EQ(open_dir, envInfo[toLower(L"dir")]);
	EXPECT_EQ(open_dir, envInfo[toLower(L"OutputDir")]);
	EXPECT_EQ(L"C:", envInfo[toLower(L"OutputDrive")]);

	EXPECT_EQ(output_path, envInfo[toLower(L"OutputFile")]);
	EXPECT_EQ(L"output.ext", envInfo[toLower(L"OutputFileName")]);
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
	EXPECT_EQ(L"c/_@@@_/_@@@_/d", LF_sanitize_pathname(L"c/../../d"));
	EXPECT_EQ(L"c/_@@@_/_@@@_/d", LF_sanitize_pathname(L"c/..///\\\\../d"));
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

/*TEST(CommonUtil, LF_sanitize_pathname) {
	LF_deleteOriginalArchives();
}*/

TEST(CommonUtil, LF_setProcessTempPath)
{
	auto original = LF_make_expand_information(nullptr, nullptr);
	EXPECT_TRUE(has_key(original, L"temp"));
	EXPECT_FALSE(original[L"temp"].empty());
	EXPECT_TRUE(has_key(original, L"tmp"));
	EXPECT_FALSE(original[L"tmp"].empty());

	EXPECT_FALSE(has_key(original, L"lf_system_original_temp"));
	EXPECT_FALSE(has_key(original, L"lf_system_original_tmp"));

	{
		LF_setProcessTempPath(L"");
		auto modified = LF_make_expand_information(nullptr, nullptr);
		EXPECT_TRUE(has_key(modified, L"temp"));
		EXPECT_FALSE(modified[L"temp"].empty());
		EXPECT_TRUE(has_key(modified, L"tmp"));
		EXPECT_FALSE(modified[L"tmp"].empty());

		EXPECT_TRUE(has_key(modified, L"temp"));
		EXPECT_EQ(original[L"temp"], modified[L"lf_system_original_temp"]);
		EXPECT_EQ(original[L"temp"], modified[L"temp"]);
		EXPECT_TRUE(has_key(modified, L"tmp"));
		EXPECT_EQ(original[L"tmp"], modified[L"lf_system_original_tmp"]);
		EXPECT_EQ(original[L"tmp"], modified[L"tmp"]);
	}

	{
		LF_setProcessTempPath(L"abc");
		auto modified = LF_make_expand_information(nullptr, nullptr);
		EXPECT_TRUE(has_key(modified, L"temp"));
		EXPECT_EQ(UtilGetModuleDirectoryPath() / L"abc", modified[L"temp"]);
		EXPECT_TRUE(has_key(modified, L"tmp"));
		EXPECT_EQ(UtilGetModuleDirectoryPath() / L"abc", modified[L"tmp"]);

		EXPECT_EQ(original[L"temp"], modified[L"lf_system_original_temp"]);
		EXPECT_EQ(original[L"tmp"], modified[L"lf_system_original_tmp"]);
	}

	{
		LF_setProcessTempPath(L"");
		auto modified = LF_make_expand_information(nullptr, nullptr);
		EXPECT_TRUE(has_key(modified, L"temp"));
		EXPECT_EQ(original[L"temp"], modified[L"lf_system_original_temp"]);
		EXPECT_EQ(original[L"temp"], modified[L"temp"]);
		EXPECT_TRUE(has_key(modified, L"tmp"));
		EXPECT_EQ(original[L"tmp"], modified[L"lf_system_original_tmp"]);
		EXPECT_EQ(original[L"tmp"], modified[L"tmp"]);
	}
}

