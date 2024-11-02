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
#include "../OSUtil.h"
#include "../Utility.h"
#include "../FileOperation.h"
#include "resource.h"

TEST(OSUtil, UtilCreateShortcut_UtilGetShortcutInfo) {
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
		//size_t s = 0;
		//_wgetenv_s(&s, buf, item.first.c_str());
		GetEnvironmentVariableW(item.first.c_str(), buf, COUNTOF(buf));
		std::wstring env = buf;
		EXPECT_EQ(std::wstring(env), item.second);
	}
}


TEST(OSUtil, UtilSetTextOnClipboard)
{
	const auto string = L"abcdeあいうえお";

	UtilSetTextOnClipboard(string);
	ASSERT_TRUE(IsClipboardFormatAvailable(CF_UNICODETEXT));
	ASSERT_TRUE(OpenClipboard(nullptr));
	HGLOBAL hg = nullptr;
	hg = GetClipboardData(CF_UNICODETEXT);
	ASSERT_NE(nullptr, hg);
	std::wstring p = (const wchar_t*)GlobalLock(hg);

	GlobalUnlock(hg);
	CloseClipboard();

	EXPECT_EQ(string, p);
}


TEST(OSUtil, UtilPathParseIconLocation)
{
	{
		auto path_and_index = UtilPathParseIconLocation(L"c:/te,st/icon.dll,5");
		EXPECT_EQ(path_and_index.first, L"c:/te,st/icon.dll");
		EXPECT_EQ(path_and_index.second, 5);
	}

	{
		auto path_and_index = UtilPathParseIconLocation(L"c:/test/icon.dll,-1");
		EXPECT_EQ(path_and_index.first, L"c:/test/icon.dll");
		EXPECT_EQ(path_and_index.second, -1);
	}

	{
		auto path_and_index = UtilPathParseIconLocation(L"c:/test/icon.dll");
		EXPECT_EQ(path_and_index.first, L"c:/test/icon.dll");
		EXPECT_EQ(path_and_index.second, 0);
	}

	{
		auto path_and_index = UtilPathParseIconLocation(L"c:/test/icon.dll,");
		EXPECT_EQ(path_and_index.first, L"c:/test/icon.dll,");
		EXPECT_EQ(path_and_index.second, 0);
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
	auto currentPath = std::filesystem::current_path();
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

