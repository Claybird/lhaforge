#include "stdafx.h"
#ifdef UNIT_TEST
#include "CppUnitTest.h"
#include "CommonUtil.h"
#include "ConfigCode/ConfigGeneral.h"
#include "Utilities/FileOperation.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTest
{
	TEST_CLASS(CommonUtil)
	{
	public:
		TEST_METHOD(test_LF_get_output_dir) {
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
			Assert::AreEqual(outputDir, std::wstring(L"C:/path_to"));
		}
		TEST_METHOD(test_LF_confirm_output_dir_type) {
			CConfigGeneral conf;
			conf.WarnRemovable = false;
			conf.WarnNetwork = false;
			Assert::AreEqual(true, LF_confirm_output_dir_type(conf, L"C:/"));
			Assert::AreEqual(true, LF_confirm_output_dir_type(conf, L"C:/temp"));
		}
		TEST_METHOD(test_LF_ask_and_make_sure_output_dir_exists) {
			auto target = UtilGetTempPath() + L"make_sure_test";
			Assert::IsFalse(std::filesystem::exists(target));
			Assert::ExpectException<LF_EXCEPTION>([&]() {LF_ask_and_make_sure_output_dir_exists(target.c_str(), LOSTDIR::LOSTDIR_ERROR); });
			Assert::IsFalse(std::filesystem::exists(target));
			LF_ask_and_make_sure_output_dir_exists(target.c_str(), LOSTDIR::LOSTDIR_FORCE_CREATE);
			Assert::IsTrue(std::filesystem::exists(target));
			UtilDeleteDir(target, true);
		}
		TEST_METHOD(test_LF_make_expand_information) {
			const auto open_dir = LR"(C:\test\)";
			const auto output_path = LR"(D:\test\output.ext)";
			auto envInfo = LF_make_expand_information(open_dir, output_path);
			Assert::IsTrue(has_key(envInfo, L"%PATH%"));
			Assert::AreEqual(UtilGetModulePath(), envInfo[L"ProgramPath"]);
			Assert::AreEqual(std::filesystem::path(UtilGetModulePath()).parent_path().wstring(), envInfo[L"ProgramDir"]);

			Assert::AreEqual(std::wstring(open_dir), envInfo[L"dir"]);
			Assert::AreEqual(std::wstring(open_dir), envInfo[L"OutputDir"]);
			Assert::AreEqual(std::wstring(L"C:"), envInfo[L"OutputDrive"]);

			Assert::AreEqual(std::wstring(output_path), envInfo[L"OutputFile"]);
			Assert::AreEqual(std::wstring(L"output.ext"), envInfo[L"OutputFileName"]);
		}
	};
};

#endif
