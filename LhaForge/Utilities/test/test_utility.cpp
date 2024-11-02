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
#include "resource.h"
#include "Utilities/Utility.h"
#include "Utilities/StringUtil.h"
#include "Utilities/FileOperation.h"
#include "Dialogs/TextInputDlg.h"

TEST(Utility, UtilGetLastErrorMessage) {
	EXPECT_EQ(L"The system cannot find the path specified.\r\n",
		UtilGetLastErrorMessage(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), ERROR_PATH_NOT_FOUND));
}


TEST(Utility, UtilReadFromResponseFile) {
	auto file = std::filesystem::path(__FILEW__).parent_path() / L"test_utility_response1.txt";
	auto files = UtilReadFromResponseFile(file, UTIL_CODEPAGE::UTF8);
	EXPECT_EQ(size_t(4), files.size());
	EXPECT_EQ(L"ファイル1.txt", files[0]);
	EXPECT_EQ(L"C:\\program files\\b.txt", files[1]);
	EXPECT_EQ(L"ファイル3.doc", files[2]);
	EXPECT_EQ(L"#d.exe", files[3]);

	file = std::filesystem::path(__FILEW__).parent_path() / L"path_that_does_not_exist.txt";
	EXPECT_THROW(UtilReadFromResponseFile(file, UTIL_CODEPAGE::UTF8), LF_EXCEPTION);
}


TEST(Utility, UtilExtMatchSpec) {
	//---single
	EXPECT_TRUE(UtilExtMatchSpec(L"test.abc", L"*.*"));
	EXPECT_TRUE(UtilExtMatchSpec(L"test.abc", L".*"));
	EXPECT_TRUE(UtilExtMatchSpec(L"test.abc", L"*"));
	EXPECT_FALSE(UtilExtMatchSpec(L"", L""));
	EXPECT_FALSE(UtilExtMatchSpec(L"", L"*.abc"));
	EXPECT_TRUE(UtilExtMatchSpec(L"test.abc", L"*.abc"));
	EXPECT_TRUE(UtilExtMatchSpec(L"test.abc", L"abc"));
	EXPECT_TRUE(UtilExtMatchSpec(L"test.abc", L".abc"));
	EXPECT_TRUE(UtilExtMatchSpec(L"test.ABC", L"abc"));
	EXPECT_FALSE(UtilExtMatchSpec(L"test.abc", L"ab"));
	EXPECT_FALSE(UtilExtMatchSpec(L"test.abc", L".ab"));
	EXPECT_FALSE(UtilExtMatchSpec(L"test.ABC", L"ab"));
	EXPECT_FALSE(UtilExtMatchSpec(L"test.abc", L"test.abc"));
	EXPECT_FALSE(UtilExtMatchSpec(L"test.abc", L"*.test"));
	EXPECT_FALSE(UtilExtMatchSpec(L"test.abc", L"test"));
	EXPECT_TRUE(UtilExtMatchSpec(L"test.abc", L"ab*"));
	EXPECT_TRUE(UtilExtMatchSpec(L"test.abc", L"abc*"));
	EXPECT_TRUE(UtilExtMatchSpec(L"test.abc", L"??c"));
	EXPECT_FALSE(UtilExtMatchSpec(L"test.abc", L"?c"));
	EXPECT_FALSE(UtilExtMatchSpec(L"test.abc", L"??d"));
	EXPECT_TRUE(UtilExtMatchSpec(L"test.tar.gz", L"tar.gz"));
	EXPECT_FALSE(UtilExtMatchSpec(L"test.tar.gz", L""));

	//---possible regex
	EXPECT_FALSE(UtilExtMatchSpec(L"test.txt", L"(.*)"));
	EXPECT_FALSE(UtilExtMatchSpec(L"test.txt", L"[a-Z]*"));
	EXPECT_FALSE(UtilExtMatchSpec(L"test.txt", L"\\"));
	EXPECT_FALSE(UtilExtMatchSpec(L"test.txt", L"$"));
	EXPECT_FALSE(UtilExtMatchSpec(L"test.txt", L"^"));
	EXPECT_FALSE(UtilExtMatchSpec(L"test.txt", L"txt|abc"));

	//---no name part or no exts
	EXPECT_TRUE(UtilExtMatchSpec(L".gitignore", L".gitignore"));
	EXPECT_TRUE(UtilExtMatchSpec(L"abc.gitignore", L".gitignore"));
	EXPECT_FALSE(UtilExtMatchSpec(L"test", L"test"));
}


TEST(Utility, UtilPathMatchSpec) {
	//---single
	EXPECT_TRUE(UtilPathMatchSpec(L"test", L"*.*"));
	EXPECT_TRUE(UtilPathMatchSpec(L"test.abc", L"*.*"));
	EXPECT_TRUE(UtilPathMatchSpec(L"test.abc", L".*"));
	EXPECT_TRUE(UtilPathMatchSpec(L"test.abc", L"*"));
	EXPECT_FALSE(UtilPathMatchSpec(L"", L""));
	EXPECT_FALSE(UtilPathMatchSpec(L"", L"*.abc"));
	EXPECT_TRUE(UtilPathMatchSpec(L"test.abc", L"*.abc"));
	EXPECT_TRUE(UtilPathMatchSpec(L"test.abc", L"abc"));
	EXPECT_TRUE(UtilPathMatchSpec(L"test.abc", L".abc"));
	EXPECT_TRUE(UtilPathMatchSpec(L"test.ABC", L"abc"));
	EXPECT_TRUE(UtilPathMatchSpec(L"test.abc", L"ab"));
	EXPECT_TRUE(UtilPathMatchSpec(L"test.abc", L".ab"));
	EXPECT_TRUE(UtilPathMatchSpec(L"test.ABC", L"ab"));
	EXPECT_TRUE(UtilPathMatchSpec(L"test.abc", L"test.abc"));
	EXPECT_FALSE(UtilPathMatchSpec(L"test.abc", L"*.test"));
	EXPECT_TRUE(UtilPathMatchSpec(L"test.abc", L"test"));
	EXPECT_TRUE(UtilPathMatchSpec(L"test.abc", L"test*"));
	EXPECT_TRUE(UtilPathMatchSpec(L"test.abc", L"*test"));
	EXPECT_TRUE(UtilPathMatchSpec(L"test.abc", L"*test*"));
	EXPECT_TRUE(UtilPathMatchSpec(L"test.abc", L"ab*"));
	EXPECT_TRUE(UtilPathMatchSpec(L"test.abc", L"abc*"));
	EXPECT_TRUE(UtilPathMatchSpec(L"test.abc", L"??c"));
	EXPECT_TRUE(UtilPathMatchSpec(L"test.abc", L"?c"));
	EXPECT_FALSE(UtilPathMatchSpec(L"test.abc", L"??d"));
	EXPECT_TRUE(UtilPathMatchSpec(L"test.tar.gz", L"tar.gz"));
	EXPECT_FALSE(UtilPathMatchSpec(L"test.tar.gz", L""));
	EXPECT_TRUE(UtilPathMatchSpec(L"test.tar.gz", L"tar"));

	EXPECT_TRUE(UtilPathMatchSpec(L"abc/def", L"abc/*"));
	EXPECT_TRUE(UtilPathMatchSpec(L"abc/def", L"*/def"));
	EXPECT_TRUE(UtilPathMatchSpec(L"abc/def", L"abc\\*"));
	EXPECT_TRUE(UtilPathMatchSpec(L"abc/def", L"*\\def"));
	EXPECT_TRUE(UtilPathMatchSpec(L"abc\\def", L"abc/*"));
	EXPECT_TRUE(UtilPathMatchSpec(L"abc\\def", L"*/def"));
	EXPECT_TRUE(UtilPathMatchSpec(L"abc\\def", L"abc\\*"));
	EXPECT_TRUE(UtilPathMatchSpec(L"abc\\def", L"*\\def"));
	EXPECT_FALSE(UtilPathMatchSpec(L"abc.def", L"abc/*"));
	EXPECT_FALSE(UtilPathMatchSpec(L"abc.def", L"*/def"));
	EXPECT_FALSE(UtilPathMatchSpec(L"abc.def", L"abc\\*"));
	EXPECT_FALSE(UtilPathMatchSpec(L"abc.def", L"*\\def"));

	//---possible regex
	EXPECT_FALSE(UtilPathMatchSpec(L"test.txt", L"(.*)"));
	EXPECT_FALSE(UtilPathMatchSpec(L"test.txt", L"[a-Z]*"));
	EXPECT_FALSE(UtilPathMatchSpec(L"test.txt", L"\\"));
	EXPECT_FALSE(UtilPathMatchSpec(L"test.txt", L"$"));
	EXPECT_FALSE(UtilPathMatchSpec(L"test.txt", L"^"));
	EXPECT_FALSE(UtilPathMatchSpec(L"test.txt", L"txt|abc"));

	//---no name part or no exts
	EXPECT_TRUE(UtilPathMatchSpec(L".gitignore", L".gitignore"));
	EXPECT_TRUE(UtilPathMatchSpec(L"abc.gitignore", L".gitignore"));
	EXPECT_TRUE(UtilPathMatchSpec(L"test", L"test"));

	//---non ascii strings
	EXPECT_TRUE(UtilPathMatchSpec(L"あいうえお/かきくけこ.txt", L"あいうえお/*.txt"));
	EXPECT_TRUE(UtilPathMatchSpec(L"あいうえお/かきくけこ.txt", L"あいうえお\\*.txt"));
	EXPECT_TRUE(UtilPathMatchSpec(L"あいうえお/かきくけこ.txt", L"*/かきくけこ.txt"));
	EXPECT_TRUE(UtilPathMatchSpec(L"あいうえお/かきくけこ.txt", L"*\\かきくけこ.txt"));
	EXPECT_TRUE(UtilPathMatchSpec(L"あいうえお/かきくけこ.txt", L"*/*.txt"));
	EXPECT_TRUE(UtilPathMatchSpec(L"あいうえお/かきくけこ.txt", L"*\\*.txt"));
	EXPECT_TRUE(UtilPathMatchSpec(L"あいうえお/かきくけこ.txt", L"*/*.*"));
	EXPECT_TRUE(UtilPathMatchSpec(L"あいうえお/かきくけこ.txt", L"*\\*.*"));
	EXPECT_TRUE(UtilPathMatchSpec(L"あいうえお/かきくけこ.txt", L"*.*"));
	EXPECT_TRUE(UtilPathMatchSpec(L"あいうえお/かきくけこ.txt", L"*"));
}

TEST(Utility, has_key) {
	std::map<std::wstring, std::wstring> m;
	m[L"abc"] = L"abc";
	m[L"あいう"] = L"あいう";
	EXPECT_TRUE(has_key(m, L"abc"));
	EXPECT_TRUE(has_key(m, L"あいう"));
	EXPECT_FALSE(has_key(m, L"cde"));
}
TEST(Utility, index_of) {
	std::vector<int> a = { 2,4,6,8,10 };
	EXPECT_EQ(1, index_of(a, 4));
	EXPECT_EQ(4, index_of(a, 10));
	EXPECT_EQ(-1, index_of(a, 1));
	EXPECT_EQ(-1, index_of(a, 11));

	EXPECT_EQ(1, index_of(&a[0], a.size(), 4));
	EXPECT_EQ(4, index_of(&a[0], a.size(), 10));
	EXPECT_EQ(-1, index_of(&a[0], a.size(), 1));
	EXPECT_EQ(-1, index_of(&a[0], a.size(), 11));
}
TEST(Utility, remove_item) {
	std::vector<int> a = { 2,4,6,6,6,10 };
	EXPECT_NE(-1, index_of(a, 6));
	remove_item(a, 6);
	EXPECT_EQ(-1, index_of(a, 6));
}
TEST(Utility, remove_item_if) {
	std::vector<int> a = { 2,4,6,6,6,10 };
	EXPECT_NE(-1, index_of(a, 6));
	remove_item_if(a, [](int value) {return value / 2 == 3; });
	EXPECT_EQ(-1, index_of(a, 6));
}
TEST(Utility, isIn) {
	std::vector<int> a = { 2,4,6,8,10 };
	EXPECT_TRUE(isIn(a, 4));
	EXPECT_FALSE(isIn(a, 3));
}
TEST(Utility, merge_map) {
	std::map<std::string, std::string> a = {
		{"a","a"},
		{"b","b"},
		{"c","c"},
	}, b = {
		{"a","A"},
		{"b","B"},
		{"d","D"},
	};
	merge_map(a, b);
	EXPECT_EQ(a["a"], "A");
	EXPECT_EQ(a["b"], "B");
	EXPECT_EQ(a["c"], "c");
	EXPECT_EQ(a["d"], "D");
}



TEST(Utility, UtilUnixTimeToFileTimeime)
{
	auto ft = UtilUnixTimeToFileTime(946730096);	//2000-01-01T12:34:56
	SYSTEMTIME systime;
	FileTimeToSystemTime(&ft, &systime);

	EXPECT_EQ(2000, systime.wYear);
	EXPECT_EQ(1, systime.wMonth);
	EXPECT_EQ(1, systime.wDay);
	EXPECT_EQ(12, systime.wHour);
	EXPECT_EQ(34, systime.wMinute);
	EXPECT_EQ(56, systime.wSecond);
}

TEST(Utility, UtilFileTimeToUnixTime)
{
	FILETIME ft = UtilUnixTimeToFileTime(946730096);

	EXPECT_EQ(946730096, UtilFileTimeToUnixTime(ft));
}
