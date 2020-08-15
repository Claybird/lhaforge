#include "stdafx.h"
#ifdef UNIT_TEST
#include <gtest/gtest.h>
#include "Utilities/OSUtil.h"
#include "Utilities/Utility.h"
#include "Utilities/FileOperation.h"

TEST(OSUtil, shortcuts) {
	auto temp_dir = std::filesystem::temp_directory_path();
	auto link_file = temp_dir / "test.lnk";
	const wchar_t* target = LR"(C:\Windows\notepad.exe)";
	const wchar_t* args = L"";
	const wchar_t* icon_file = LR"(C:\Windows\System32\SHELL32.dll)";
	const int icon_index = 5;
	const wchar_t* desc = L"test link";

	EXPECT_FALSE(std::filesystem::exists(link_file));
	EXPECT_EQ(S_OK, UtilCreateShortcut(
		link_file.c_str(),
		target,
		args,
		icon_file,
		icon_index,
		desc));
	EXPECT_TRUE(std::filesystem::exists(link_file));

	UTIL_SHORTCUTINFO info;
	EXPECT_EQ(S_OK, UtilGetShortcutInfo(link_file, info));
	EXPECT_EQ(toLower(target), toLower(info.cmd));
	EXPECT_EQ(args, info.param);
	EXPECT_EQ(L"", info.workingDir);
	std::filesystem::remove(link_file);
}

TEST(OSUtil, UtilGetEnvInfo) {
	auto envInfo = UtilGetEnvInfo();
	EXPECT_TRUE(has_key(envInfo, L"PATH"));
	for (const auto& item : envInfo) {
		wchar_t buf[_MAX_ENV] = {};
		size_t s = 0;
		_wgetenv_s(&s, buf, item.first.c_str());
		std::wstring env = buf;
		EXPECT_EQ(std::wstring(env), item.second);
	}
}

TEST(OSUtil, CurrentDirManager) {
	auto prevPath = std::filesystem::current_path();
	{
		CCurrentDirManager cdm(std::filesystem::temp_directory_path().c_str());
		auto currentPath = UtilPathAddLastSeparator(std::filesystem::current_path());
		EXPECT_EQ(UtilPathAddLastSeparator(std::filesystem::temp_directory_path()),
			currentPath);
	}
	auto currentPath= std::filesystem::current_path();
	EXPECT_EQ(prevPath.wstring(), currentPath.wstring());

	auto path = std::filesystem::temp_directory_path() / L"lf_path_test";
	{
		std::filesystem::create_directories(path);
		CCurrentDirManager cdm(path.c_str());
		//what if previous directory does not exist?
		EXPECT_THROW({
			CCurrentDirManager cdm2(prevPath.c_str());
			std::filesystem::remove(path);
			EXPECT_FALSE(std::filesystem::exists(path));
			}, LF_EXCEPTION);
	}
}

#endif
