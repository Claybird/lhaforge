#include "stdafx.h"
#ifdef UNIT_TEST
#include <gtest/gtest.h>
#include "Utilities/Utility.h"

TEST(Utility, UtilGetLastErrorMessage) {
	EXPECT_EQ(L"The system cannot find the path specified.\r\n", 
		UtilGetLastErrorMessage(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), ERROR_PATH_NOT_FOUND));
}
TEST(Utility, BOOL2bool) {
	EXPECT_TRUE(BOOL2bool(TRUE));
	EXPECT_FALSE(BOOL2bool(FALSE));
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

#endif
