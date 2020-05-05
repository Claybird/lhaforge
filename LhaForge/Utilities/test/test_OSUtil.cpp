#include "stdafx.h"
#ifdef UNIT_TEST
#include "CppUnitTest.h"
#include "Utilities/OSUtil.h"
#include "Utilities/Utility.h"

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
			wchar_t prevPath[_MAX_PATH + 1];
			_wgetcwd(prevPath, _MAX_PATH);
			{
				CCurrentDirManager cdm(std::filesystem::temp_directory_path().c_str());
				wchar_t currentPath[_MAX_PATH + 1];
				_wgetcwd(currentPath, _MAX_PATH);
				Assert::AreEqual(std::filesystem::temp_directory_path().wstring(),
					(std::filesystem::path(currentPath) / L"").wstring());
			}
			wchar_t currentPath[_MAX_PATH + 1];
			_wgetcwd(currentPath, _MAX_PATH);
			Assert::AreEqual(std::wstring(prevPath), std::wstring(currentPath));
		}
	};
};

#endif
