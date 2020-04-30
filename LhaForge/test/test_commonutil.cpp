#include "stdafx.h"
#ifdef UNIT_TEST
#include "CppUnitTest.h"
#include "CommonUtil.h"
#include "ConfigCode/ConfigGeneral.h"

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
	};
};

#endif
