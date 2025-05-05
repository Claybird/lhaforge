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
#include "../StringUtil.h"
#include "../Utility.h"

#include "resource.h"


TEST(StringUtil, UtilTrimString) {
	EXPECT_EQ(L"abc", UtilTrimString(L"abc \t  ", L" \t"));
	EXPECT_EQ(L"123abc", UtilTrimString(L"123abc456", L"0123456789"));
	EXPECT_EQ(L"123abc", UtilTrimString(L"123abcあいうえお", L"123あいうえお"));
}


TEST(StringUtil, UtilMakeFilterString) {
	const wchar_t raw[] = L"abc\0def\0\0";
	auto out = UtilMakeFilterString(L"abc|def");
	for (auto i = 0; i < sizeof(raw) / sizeof(raw[0]); i++) {
		EXPECT_EQ(raw[i], out[i]);
	}
}

TEST(StringUtil, Encodings) {
	const char cp932[] = "\x82\xa0\x82\xa2\x82\xa4\x63\x70\x39\x33\x32";
	const wchar_t* cp932_ans = L"あいうcp932";

	const char utf16_le[] = "\x42\x30\x44\x30\x46\x30\x75\x00\x74\x00\x66\x00\x31\x00\x36\x00\x5f\x00\x6c\x00\x65\x00\x00";
	const wchar_t* utf16_le_ans = L"あいうutf16_le";

	const char utf16_le_bom[] = "\xff\xfe" "\x42\x30\x44\x30\x46\x30\x75\x0\x74\x0\x66\x0\x31\x0\x36\x0\x5f\x0\x6c\x0\x65\x0\x5f\x0\x62\x0\x6f\x0\x6d\x0\x0";
	const wchar_t* utf16_le_bom_ans = L"あいうutf16_le_bom";

	const char utf16_be_bom[] = "\xfe\xff" "\x30\x42\x30\x44\x30\x46\x0\x75\x0\x74\x0\x66\x0\x31\x0\x36\x0\x5f\x0\x62\x0\x65\x0\x5f\x0\x62\x0\x6f\x0\x6d\x0";
	const wchar_t* utf16_be_bom_ans = L"あいうutf16_be_bom";

	const char utf8[] = "\xe3\x81\x82\xe3\x81\x84\xe3\x81\x86\x75\x74\x66\x38";
	const wchar_t* utf8_ans = L"あいうutf8";

	const char utf8_bom[] = "\xef\xbb\xbf\xe3\x81\x82\xe3\x81\x84\xe3\x81\x86\x75\x74\x66\x38\x5f\x62\x6f\x6d";
	const wchar_t* utf8_bom_ans = L"あいうutf8_bom";

	EXPECT_EQ(std::string(utf8), UtilToUTF8(utf8_ans));

	EXPECT_EQ(utf8_ans, UtilUTF8toUNICODE(utf8, sizeof(utf8)));
	EXPECT_EQ(utf8_ans, UtilUTF8toUNICODE(std::string(utf8)));
	//with BOM
	EXPECT_EQ(utf8_bom_ans, UtilUTF8toUNICODE(utf8_bom, sizeof(utf8_bom)));

	EXPECT_EQ(cp932_ans, UtilCP932toUNICODE(cp932, sizeof(cp932)));

	EXPECT_EQ(utf16_le_ans, UtilUTF16toUNICODE(utf16_le, sizeof(utf16_le)));
	//with BOM
	//UTF16-LE
	EXPECT_EQ(utf16_le_bom_ans, UtilUTF16toUNICODE(utf16_le_bom, sizeof(utf16_le_bom)));
	//UTF16-BE
	EXPECT_EQ(utf16_be_bom_ans, UtilUTF16toUNICODE(utf16_be_bom, sizeof(utf16_be_bom)));

	//decode
	EXPECT_EQ(cp932_ans, UtilToUNICODE(cp932, sizeof(cp932), UTIL_CODEPAGE::CP932));
	EXPECT_EQ(utf16_le_ans, UtilToUNICODE(utf16_le, sizeof(utf16_le), UTIL_CODEPAGE::UTF16));
	EXPECT_EQ(utf16_le_bom_ans, UtilToUNICODE(utf16_le_bom, sizeof(utf16_le_bom), UTIL_CODEPAGE::UTF16));
	EXPECT_EQ(utf16_be_bom_ans, UtilToUNICODE(utf16_be_bom, sizeof(utf16_be_bom), UTIL_CODEPAGE::UTF16));
	EXPECT_EQ(utf8_ans, UtilToUNICODE(utf8, sizeof(utf8), UTIL_CODEPAGE::UTF8));
	EXPECT_EQ(utf8_bom_ans, UtilToUNICODE(utf8_bom, sizeof(utf8_bom), UTIL_CODEPAGE::UTF8));

	//CP932
	EXPECT_TRUE(UtilVerifyGuessedCodepage(cp932, sizeof(cp932), UTIL_CODEPAGE::CP932));
	EXPECT_FALSE(UtilVerifyGuessedCodepage(cp932, sizeof(cp932), UTIL_CODEPAGE::UTF16));
	EXPECT_FALSE(UtilVerifyGuessedCodepage(cp932, sizeof(cp932), UTIL_CODEPAGE::UTF8));

	//UTF16-LE
	EXPECT_FALSE(UtilVerifyGuessedCodepage(utf16_le, sizeof(utf16_le), UTIL_CODEPAGE::CP932));
	EXPECT_TRUE(UtilVerifyGuessedCodepage(utf16_le, sizeof(utf16_le), UTIL_CODEPAGE::UTF16));
	EXPECT_FALSE(UtilVerifyGuessedCodepage(utf16_le, sizeof(utf16_le), UTIL_CODEPAGE::UTF8));

	//UTF16-LE-BOM
	EXPECT_FALSE(UtilVerifyGuessedCodepage(utf16_le_bom, sizeof(utf16_le_bom), UTIL_CODEPAGE::CP932));
	EXPECT_TRUE(UtilVerifyGuessedCodepage(utf16_le_bom, sizeof(utf16_le_bom), UTIL_CODEPAGE::UTF16));
	EXPECT_FALSE(UtilVerifyGuessedCodepage(utf16_le_bom, sizeof(utf16_le_bom), UTIL_CODEPAGE::UTF8));

	//UTF16-BE-BOM
	EXPECT_FALSE(UtilVerifyGuessedCodepage(utf16_be_bom, sizeof(utf16_be_bom), UTIL_CODEPAGE::CP932));
	EXPECT_TRUE(UtilVerifyGuessedCodepage(utf16_be_bom, sizeof(utf16_be_bom), UTIL_CODEPAGE::UTF16));
	EXPECT_FALSE(UtilVerifyGuessedCodepage(utf16_be_bom, sizeof(utf16_be_bom), UTIL_CODEPAGE::UTF8));

	//UTF-8
	EXPECT_FALSE(UtilVerifyGuessedCodepage(utf8, sizeof(utf8), UTIL_CODEPAGE::CP932));
	EXPECT_FALSE(UtilVerifyGuessedCodepage(utf8, sizeof(utf8), UTIL_CODEPAGE::UTF16));
	EXPECT_TRUE(UtilVerifyGuessedCodepage(utf8, sizeof(utf8), UTIL_CODEPAGE::UTF8));

	//UTF-8-BOM
	EXPECT_FALSE(UtilVerifyGuessedCodepage(utf8_bom, sizeof(utf8_bom), UTIL_CODEPAGE::CP932));
	EXPECT_FALSE(UtilVerifyGuessedCodepage(utf8_bom, sizeof(utf8_bom), UTIL_CODEPAGE::UTF16));
	EXPECT_TRUE(UtilVerifyGuessedCodepage(utf8_bom, sizeof(utf8_bom), UTIL_CODEPAGE::UTF8));

}

TEST(StringUtil, UtilIsSafeUnicode) {
	EXPECT_TRUE(UtilIsSafeUnicode(L"abcdef"));
	EXPECT_FALSE(UtilIsSafeUnicode(L"abc\u202Edef"));
}


TEST(StringUtil, UtilExpandTemplateString) {
	std::map<std::wstring, std::wstring> envVars;
	envVars[L"abc"] = L"123";
	envVars[L"def"] = L"{abc}";
	EXPECT_EQ(
		L"123;123;{abc};{ghi};%jkl%;%F;%mnopq{};123",
		UtilExpandTemplateString(L"%abc%;{abc};{def};{ghi};%jkl%;%F;%mnopq{};%ABC%", envVars));
}


TEST(StringUtil, UtilSplitString) {
	auto out = UtilSplitString(L"12,345,abc,あいう,,def,", L",");
	EXPECT_EQ(size_t(7), out.size());
	EXPECT_EQ(L"12", out[0]);
	EXPECT_EQ(L"345", out[1]);
	EXPECT_EQ(L"abc", out[2]);
	EXPECT_EQ(L"あいう", out[3]);
	EXPECT_EQ(L"", out[4]);
	EXPECT_EQ(L"def", out[5]);
	EXPECT_EQ(L"", out[6]);

	out = UtilSplitString(L"", L",");
	EXPECT_EQ(size_t(1), out.size());
}


TEST(StringUtil, UtilStringToIntArray) {
	auto out = UtilStringToIntArray(L"1,23,,45.6");
	EXPECT_EQ(size_t(4), out.size());
	EXPECT_EQ(1, out[0]);
	EXPECT_EQ(23, out[1]);
	EXPECT_EQ(0, out[2]);
	EXPECT_EQ(45, out[3]);

	out = UtilStringToIntArray(L"a,b,c");
	EXPECT_EQ(size_t(3), out.size());
	EXPECT_EQ(0, out[0]);
	EXPECT_EQ(0, out[1]);
	EXPECT_EQ(0, out[2]);
}

TEST(StringUtil, UtilFormatSize) {
	EXPECT_EQ(L"---", UtilFormatSize(-1));
	EXPECT_EQ(L"1 Bytes", UtilFormatSize(1));
	EXPECT_EQ(L"1000 Bytes", UtilFormatSize(1000));
	EXPECT_EQ(L"1.00 KB", UtilFormatSize(1024));
	EXPECT_EQ(L"1.05 KB", UtilFormatSize(1075));
	EXPECT_EQ(L"1.00 MB", UtilFormatSize(1024 * 1024));
	EXPECT_EQ(L"1.00 GB", UtilFormatSize(1024 * 1024 * 1024));
	EXPECT_EQ(L"1.00 TB", UtilFormatSize(1024 * 1024 * 1024 * 1024ull));
	EXPECT_EQ(L"1.00 PB", UtilFormatSize(1024 * 1024 * 1024 * 1024ull * 1024ull));
	EXPECT_EQ(L"10.00 PB", UtilFormatSize(1024 * 1024 * 1024 * 1024ull * 1024ull * 10));
}


TEST(StringUtil, UtilFormatSizeStrict) {
	EXPECT_EQ(L"---", UtilFormatSizeStrict(-1));
	EXPECT_EQ(L"1B", UtilFormatSizeStrict(1));
	EXPECT_EQ(L"1000B", UtilFormatSizeStrict(1000));
	EXPECT_EQ(L"1KB", UtilFormatSizeStrict(1024));
	EXPECT_EQ(L"1025B", UtilFormatSizeStrict(1025));
	EXPECT_EQ(L"1MB", UtilFormatSizeStrict(1024 * 1024));
	EXPECT_EQ(L"1048577B", UtilFormatSizeStrict(1024 * 1024 + 1));
	EXPECT_EQ(L"1025KB", UtilFormatSizeStrict(1024 * 1025));
	EXPECT_EQ(L"1GB", UtilFormatSizeStrict(1024 * 1024 * 1024));
	EXPECT_EQ(L"1TB", UtilFormatSizeStrict(1024 * 1024 * 1024 * 1024ull));
	EXPECT_EQ(L"1PB", UtilFormatSizeStrict(1024 * 1024 * 1024 * 1024ull * 1024ull));
	EXPECT_EQ(L"10PB", UtilFormatSizeStrict(1024 * 1024 * 1024 * 1024ull * 1024ull * 10));
}


TEST(StringUtil, UtilFormatTime) {
	std::string currentLocale = setlocale(LC_TIME, nullptr);
	auto ret = setlocale(LC_TIME, "ja_JP");
	EXPECT_NE(nullptr, ret);
	EXPECT_EQ(L"2020/05/01 21:50:42", UtilFormatTime(1588337442ul));

	setlocale(LC_TIME, currentLocale.c_str());
}


TEST(StringUtil, UtilLoadString) {
	EXPECT_EQ(L"LhaForge", UtilLoadString(IDS_MESSAGE_CAPTION));
}

TEST(StringUtil, Format) {
	EXPECT_EQ(L"", Format(L""));
	EXPECT_EQ(L"1234567", Format(L"%d45%d", 123, 67));
	EXPECT_EQ(L"abcde", Format(L"%sde", L"abc"));
	EXPECT_EQ(L"12.35", Format(L"%.2f", 12.3456));
	EXPECT_EQ(L"あいうえお", Format(L"%sえお", L"あいう"));
	EXPECT_EQ(L"あいうえお", Format(L"%sえお", std::wstring(L"あいう")));
	EXPECT_EQ(L"あいうえお", Format(L"%sえお", std::filesystem::path(L"あいう")));
}

TEST(StringUtil, replace) {
	EXPECT_EQ(L"", replace(L"", L"", L""));
	EXPECT_EQ(L"", replace(L"", L"", L"aa"));
	EXPECT_EQ(L"abcdeab", replace(L"12cde12", L"12", L"ab"));
	EXPECT_EQ(L"abcいde12", replace(L"12cいde12", L"12", L"ab", true));

	EXPECT_EQ(L"あいうcdeあいう", replace(L"12cde12", L"12", L"あいう"));
}

TEST(StringUtil, toLower) {
	_wsetlocale(LC_ALL, L"");	//default locale
	EXPECT_EQ(L"", toLower(L""));
	EXPECT_EQ(L"abcde", toLower(L"abcde"));
	EXPECT_EQ(L"abcde", toLower(L"aBcDe"));
	EXPECT_EQ(L"あいうcde", toLower(L"あいうCdE"));
	EXPECT_EQ(L"👪", toLower(L"👪"));	//emoji

	EXPECT_EQ("", toLower(""));
	EXPECT_EQ("abcde", toLower("abcde"));
	EXPECT_EQ("abcde", toLower("aBcDe"));
	EXPECT_EQ("あいうcde", toLower("あいうCdE"));
}

TEST(StringUtil, toUpper) {
	_wsetlocale(LC_ALL, L"");	//default locale
	EXPECT_EQ(L"", toUpper(L""));
	EXPECT_EQ(L"ABCDE", toUpper(L"abcde"));
	EXPECT_EQ(L"ABCDE", toUpper(L"aBcDe"));
	EXPECT_EQ(L"あいうCDE", toUpper(L"あいうCdE"));
	EXPECT_EQ(L"👪", toUpper(L"👪"));	//emoji

	EXPECT_EQ("", toUpper(""));
	EXPECT_EQ("ABCDE", toUpper("abcde"));
	EXPECT_EQ("ABCDE", toUpper("aBcDe"));
	EXPECT_EQ("あいうCDE", toUpper("あいうCdE"));
}

TEST(StringUtil, join) {
	EXPECT_EQ(L"abc", join(L", ", std::vector<std::wstring>({ L"abc" })));
	EXPECT_EQ(L"abc, def", join(L", ", std::vector<std::wstring>({ L"abc", L"def" })));
	EXPECT_EQ(L"abc, def", join(L", ", std::vector<std::wstring>({ L"abc", L"def",L"ghi" }), 2));
}

TEST(StringUtil, startsWith) {
	EXPECT_TRUE(startsWith(L"abc", L"ab"));
	EXPECT_TRUE(startsWith(L"abc", L"abc"));
	EXPECT_FALSE(startsWith(L"abc", L"Ab"));
	EXPECT_FALSE(startsWith(L"abc", L"abcd"));
	EXPECT_TRUE(startsWith(L"abc", L""));
	EXPECT_FALSE(startsWith(L"abc", L"cde"));
}

