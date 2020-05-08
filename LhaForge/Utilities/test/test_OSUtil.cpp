#include "stdafx.h"
#ifdef UNIT_TEST
#include "CppUnitTest.h"
#include "Utilities/OSUtil.h"
#include "Utilities/Utility.h"
#include "Utilities/FileOperation.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTest
{
	TEST_CLASS(osutil)
	{
	public:
		TEST_METHOD(test_shortcuts) {
			auto temp_dir = std::filesystem::temp_directory_path();
			auto link_file = temp_dir / "test.lnk";
			const wchar_t* target = LR"(C:\Windows\notepad.exe)";
			const wchar_t* args = L"";
			const wchar_t* icon_file = LR"(C:\Windows\System32\SHELL32.dll)";
			const int icon_index = 5;
			const wchar_t* desc = L"test link";

			Assert::IsFalse(std::filesystem::exists(link_file));
			Assert::AreEqual(S_OK, UtilCreateShortcut(
				link_file.c_str(),
				target,
				args,
				icon_file,
				icon_index,
				desc));
			Assert::IsTrue(std::filesystem::exists(link_file));

			UTIL_SHORTCUTINFO info;
			Assert::AreEqual(S_OK, UtilGetShortcutInfo(link_file.c_str(), info));
			Assert::AreEqual(toLower(target), toLower(info.cmd));
			Assert::AreEqual(std::wstring(args), info.param);
			Assert::AreEqual(std::wstring(L""), info.workingDir);
			std::filesystem::remove(link_file);
		}
		TEST_METHOD(test_UtilGetEnvInfo) {
			auto envInfo = UtilGetEnvInfo();
			Assert::IsTrue(has_key(envInfo, L"PATH"));
			for (const auto& item : envInfo) {
				wchar_t buf[_MAX_ENV] = {};
				size_t s = 0;
				_wgetenv_s(&s, buf, item.first.c_str());
				std::wstring env = buf;
				Assert::AreEqual(std::wstring(env), item.second);
			}
		}
		TEST_METHOD(test_CurrentDirManager) {
			auto prevPath = std::filesystem::current_path();
			{
				CCurrentDirManager cdm(std::filesystem::temp_directory_path().c_str());
				auto currentPath = UtilPathAddLastSeparator(std::filesystem::current_path().c_str());
				Assert::AreEqual(UtilPathAddLastSeparator(std::filesystem::temp_directory_path().c_str()),
					currentPath);
			}
			auto currentPath= std::filesystem::current_path();
			Assert::AreEqual(prevPath.wstring(), currentPath.wstring());

			auto path = std::filesystem::temp_directory_path() / L"lf_path_test";
			{
				std::filesystem::create_directories(path);
				CCurrentDirManager cdm(path.c_str());
				//what if previous directory does not exist?
				Assert::ExpectException<LF_EXCEPTION>([&](){
					CCurrentDirManager cdm2(prevPath.c_str());
					std::filesystem::remove(path);
					Assert::IsFalse(std::filesystem::exists(path));
				});
			}
		}
	};
};

#endif
