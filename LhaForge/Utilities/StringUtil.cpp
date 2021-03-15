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
#include "StringUtil.h"
#include "Utility.h"
#ifdef UNIT_TEST
#include "resource.h"
#endif

std::wstring UtilTrimString(const std::wstring &target, const std::wstring &trimTargets)
{
	auto idx = target.find_last_not_of(trimTargets);
	if (idx == std::wstring::npos)return L"";
	else return target.substr(0, idx + 1);
}

#ifdef UNIT_TEST
TEST(StringUtil, UtilTrimString) {
	EXPECT_EQ(L"abc", UtilTrimString(L"abc \t  ", L" \t"));
	EXPECT_EQ(L"123abc", UtilTrimString(L"123abc456", L"0123456789"));
	EXPECT_EQ(L"123abc", UtilTrimString(L"123abcあいうえお", L"123あいうえお"));
}
#endif

//builds filter string for CFileDialog from MFC style string, i.e., "*.txt|*.doc||"
std::wstring UtilMakeFilterString(const std::wstring& filterIn)
{
	std::wstring out = filterIn;
	out += L"||";

	for(auto& c:out){
		if(c==L'|'){
			c = L'\0';
		}
	}
	return out;
}

#ifdef UNIT_TEST
TEST(StringUtil, UtilMakeFilterString) {
	const wchar_t raw[] = L"abc\0def\0\0";
	auto out = UtilMakeFilterString(L"abc|def");
	for (auto i = 0; i < sizeof(raw) / sizeof(raw[0]); i++) {
		EXPECT_EQ(raw[i], out[i]);
	}
}
#endif

std::wstring UtilToUNICODE(const char* lpSrc, size_t length, UTIL_CODEPAGE uSrcCodePage)
{
	const unsigned char* pByte = (const unsigned char*)lpSrc;
	switch (uSrcCodePage) {
	case UTIL_CODEPAGE::UTF8:
		if (length >= 3 && pByte[0] == 0xEF && pByte[1] == 0xBB && pByte[2] == 0xBF) {	//BOM check
			lpSrc += 3;
			length -= 3;
		}
		break;
	case UTIL_CODEPAGE::UTF16:
		if (length >= 2 && pByte[0] == 0xFF && pByte[1] == 0xFE) {
			//UTF-16-LE
			lpSrc += 2;
			length -= 2;
			return (const wchar_t*)lpSrc;
		} else if (length >= 2 && pByte[0] == 0xFE && pByte[1] == 0xFF) {
			//UTF-16-BE
			lpSrc += 2;
			length -= 2;
			std::wstring buf = (const wchar_t*)lpSrc;
			char* p = (char*)&buf[0];
			for (size_t i = 0; i < buf.length(); i++) {
				std::swap(p[i * 2], p[i * 2 + 1]);
			}
			return buf;
		} else {
			return (const wchar_t*)lpSrc;
		}
	}
	int bufSize = ::MultiByteToWideChar((int)uSrcCodePage, 0, lpSrc, length, NULL, 0);

	std::wstring wstr;
	wstr.resize(bufSize);

	::MultiByteToWideChar((int)uSrcCodePage, 0, lpSrc, length, &wstr[0], bufSize);
	return wstr.c_str();
}

std::string UtilToUTF8(const std::wstring& unicode_string)
{
	int bufSize = ::WideCharToMultiByte(CP_UTF8, 0,
		unicode_string.c_str(), -1,
		nullptr, 0, nullptr, nullptr);

	std::string utf8;
	utf8.resize(bufSize);

	::WideCharToMultiByte(CP_UTF8, 0, unicode_string.c_str(), -1, &utf8[0], bufSize, nullptr, nullptr);
	return utf8.c_str();
}

UTIL_CODEPAGE UtilGuessCodepage(const char* lpSrc, size_t length)
{
	const unsigned char* pByte = (const unsigned char*)lpSrc;
	if (length >= 2 && (
		(pByte[0] == 0xFE && pByte[1] == 0xFF) ||
		(pByte[0] == 0xFF && pByte[1] == 0xFE))) {
		return UTIL_CODEPAGE::UTF16;
	} else if (length >= 3 && pByte[0] == 0xEF && pByte[1] == 0xBB && pByte[2] == 0xBF) {	//BOM check
		return UTIL_CODEPAGE::UTF8;
	} else {
		if (UtilVerityGuessedCodepage(lpSrc, length, UTIL_CODEPAGE::UTF8)) {
			return UTIL_CODEPAGE::UTF8;
		} else if (UtilVerityGuessedCodepage(lpSrc, length, UTIL_CODEPAGE::UTF16)) {
			return UTIL_CODEPAGE::UTF16;
		}else{
			return UTIL_CODEPAGE::CP932;
		}
	}
}

//NOTE: this function should work for relatively long string, but not reliable for short string
//checks if the code page is correct for the given string
bool UtilVerityGuessedCodepage(const char* lpSrc, size_t length, UTIL_CODEPAGE uSrcCodePage)
{
	const unsigned char* pByte = (const unsigned char*)lpSrc;
	if (length>=3 && pByte[0] == 0xEF && pByte[1] == 0xBB && pByte[2] == 0xBF) {	//BOM check
		if (UTIL_CODEPAGE::UTF8 == uSrcCodePage) {
			return true;
		} else {
			return false;
		}
	}
	if (length >= 2 && (
		(pByte[0] == 0xFE && pByte[1] == 0xFF) ||
		(pByte[0] == 0xFF && pByte[1] == 0xFE))) {
		if (UTIL_CODEPAGE::UTF16 == uSrcCodePage) {
			return true;
		}
	}

	auto decoded = UtilToUNICODE(lpSrc, length, uSrcCodePage);

	std::string encoded;
	BOOL fallBack = FALSE;
	if (uSrcCodePage == UTIL_CODEPAGE::UTF16) {
		encoded.assign((const char*)&decoded[0], (const char*)(&decoded[0] + decoded.size() + 1));
	} else {
		int bufSize = ::WideCharToMultiByte((int)uSrcCodePage, 0, decoded.c_str(), -1, NULL, 0, "?", NULL);
		encoded.resize(bufSize);

		::WideCharToMultiByte((int)uSrcCodePage, 0, decoded.c_str(), -1, &encoded[0], bufSize, "?", &fallBack);
	}
	return !fallBack && std::string(lpSrc, lpSrc + length) == encoded;
}

#ifdef UNIT_TEST

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
	EXPECT_TRUE(UtilVerityGuessedCodepage(cp932, sizeof(cp932), UTIL_CODEPAGE::CP932));
	EXPECT_FALSE(UtilVerityGuessedCodepage(cp932, sizeof(cp932), UTIL_CODEPAGE::UTF16));
	EXPECT_FALSE(UtilVerityGuessedCodepage(cp932, sizeof(cp932), UTIL_CODEPAGE::UTF8));

	//UTF16-LE
	EXPECT_FALSE(UtilVerityGuessedCodepage(utf16_le, sizeof(utf16_le), UTIL_CODEPAGE::CP932));
	EXPECT_TRUE(UtilVerityGuessedCodepage(utf16_le, sizeof(utf16_le), UTIL_CODEPAGE::UTF16));
	EXPECT_FALSE(UtilVerityGuessedCodepage(utf16_le, sizeof(utf16_le), UTIL_CODEPAGE::UTF8));

	//UTF16-LE-BOM
	EXPECT_FALSE(UtilVerityGuessedCodepage(utf16_le_bom, sizeof(utf16_le_bom), UTIL_CODEPAGE::CP932));
	EXPECT_TRUE(UtilVerityGuessedCodepage(utf16_le_bom, sizeof(utf16_le_bom), UTIL_CODEPAGE::UTF16));
	EXPECT_FALSE(UtilVerityGuessedCodepage(utf16_le_bom, sizeof(utf16_le_bom), UTIL_CODEPAGE::UTF8));

	//UTF16-BE-BOM
	EXPECT_FALSE(UtilVerityGuessedCodepage(utf16_be_bom, sizeof(utf16_be_bom), UTIL_CODEPAGE::CP932));
	EXPECT_TRUE(UtilVerityGuessedCodepage(utf16_be_bom, sizeof(utf16_be_bom), UTIL_CODEPAGE::UTF16));
	EXPECT_FALSE(UtilVerityGuessedCodepage(utf16_be_bom, sizeof(utf16_be_bom), UTIL_CODEPAGE::UTF8));

	//UTF-8
	EXPECT_FALSE(UtilVerityGuessedCodepage(utf8, sizeof(utf8), UTIL_CODEPAGE::CP932));
	EXPECT_FALSE(UtilVerityGuessedCodepage(utf8, sizeof(utf8), UTIL_CODEPAGE::UTF16));
	EXPECT_TRUE(UtilVerityGuessedCodepage(utf8, sizeof(utf8), UTIL_CODEPAGE::UTF8));

	//UTF-8-BOM
	EXPECT_FALSE(UtilVerityGuessedCodepage(utf8_bom, sizeof(utf8_bom), UTIL_CODEPAGE::CP932));
	EXPECT_FALSE(UtilVerityGuessedCodepage(utf8_bom, sizeof(utf8_bom), UTIL_CODEPAGE::UTF16));
	EXPECT_TRUE(UtilVerityGuessedCodepage(utf8_bom, sizeof(utf8_bom), UTIL_CODEPAGE::UTF8));

}
#endif

//https://stackoverflow.com/questions/22617209/regex-replace-with-callback-in-c11
template<class BidirIt, class Traits, class CharT, class UnaryFunction>
std::basic_string<CharT> util_regex_replace(BidirIt first, BidirIt last,
	const std::basic_regex<CharT, Traits>& re, UnaryFunction f)
{
	std::basic_string<CharT> s;

	typename std::match_results<BidirIt>::difference_type
		positionOfLastMatch = 0;
	auto endOfLastMatch = first;

	auto callback = [&](const std::match_results<BidirIt>& match)
	{
		auto positionOfThisMatch = match.position(0);
		auto diff = positionOfThisMatch - positionOfLastMatch;

		auto startOfThisMatch = endOfLastMatch;
		std::advance(startOfThisMatch, diff);

		s.append(endOfLastMatch, startOfThisMatch);
		s.append(f(match));

		auto lengthOfMatch = match.length(0);

		positionOfLastMatch = positionOfThisMatch + lengthOfMatch;

		endOfLastMatch = startOfThisMatch;
		std::advance(endOfLastMatch, lengthOfMatch);
	};

	std::regex_iterator<BidirIt> begin(first, last, re), end;
	std::for_each(begin, end, callback);

	s.append(endOfLastMatch, last);

	return s;
}

//expand variables, "{foo}" for process specific variables, and "%bar%" for OS environment variables
std::wstring UtilExpandTemplateString(const std::wstring& format, const std::map<std::wstring, std::wstring> &envVars)
{
	auto fmt = format;
	return util_regex_replace(fmt.cbegin(), fmt.cend(), std::wregex(L"\\{([^;]*?)\\}|%([^;]*?)%"),
		[&](const std::wsmatch& m) {
		auto key = m.str(0);
		auto found = envVars.find(toLower(key.substr(1, key.length() - 2)));
		if (found==envVars.end()) {
			return key;
		} else {
			return (*found).second;
		}
	});
}
#ifdef UNIT_TEST
TEST(StringUtil, UtilExpandTemplateString) {
	std::map<std::wstring, std::wstring> envVars;
	envVars[L"abc"] = L"123";
	envVars[L"def"] = L"{abc}";
	EXPECT_EQ(
		L"123;123;{abc};{ghi};%jkl%;%F;%mnopq{};123",
		UtilExpandTemplateString(L"%abc%;{abc};{def};{ghi};%jkl%;%F;%mnopq{};%ABC%", envVars));
}
#endif

std::vector<std::wstring> UtilSplitString(const std::wstring& target, const std::wstring& separator)
{
	std::vector<std::wstring> splitted;
	if (separator.empty()) {
		splitted.push_back(target);
	} else {
		std::wstring::size_type first = 0;
		while (true) {
			auto pos = target.find(separator, first);
			if (std::wstring::npos == pos) {
				splitted.push_back(target.substr(first));
				break;
			} else {
				splitted.push_back(target.substr(first, pos - first));
				first = pos + separator.length();
			}
		}
	}
	return splitted;
}
#ifdef UNIT_TEST
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
#endif

//split string into number array
std::vector<int> UtilStringToIntArray(const std::wstring& str)
{
	std::vector<int> numArr;
	for(const auto& sub: UtilSplitString(str, L",")){
		numArr.push_back(_wtoi(sub.c_str()));
	}
	return numArr;
}
#ifdef UNIT_TEST
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
#endif


std::wstring UtilFormatSize(UINT64 size)
{
	if (-1 == size) {
		return L"---";
	}
	const std::vector<std::pair<UINT64, std::wstring> > units = {
		{1, L"Bytes"},
		{1024, L"KB"},
		{1024 * 1024, L"MB"},
		{1024 * 1024 * 1024, L"GB" },
		{1024 * 1024 * 1024 * 1024ull, L"TB"},
		{1024 * 1024 * 1024 * 1024ull * 1024ull, L"PB"},
	};

	for (const auto &unit : units) {
		if (size / unit.first < 1024) {
			return Format(L"%llu %s", size / unit.first, unit.second.c_str());
		}
	}

	return Format(L"%llu %s", size / units.back().first, units.back().second.c_str());
}
#ifdef UNIT_TEST
TEST(StringUtil, UtilFormatSize) {
	EXPECT_EQ(L"---", UtilFormatSize(-1));
	EXPECT_EQ(L"1 Bytes", UtilFormatSize(1));
	EXPECT_EQ(L"1000 Bytes", UtilFormatSize(1000));
	EXPECT_EQ(L"1 KB", UtilFormatSize(1024));
	EXPECT_EQ(L"1 MB", UtilFormatSize(1024 * 1024));
	EXPECT_EQ(L"1 GB", UtilFormatSize(1024 * 1024 * 1024));
	EXPECT_EQ(L"1 TB", UtilFormatSize(1024 * 1024 * 1024 * 1024ull));
	EXPECT_EQ(L"1 PB", UtilFormatSize(1024 * 1024 * 1024 * 1024ull * 1024ull));
	EXPECT_EQ(L"10 PB", UtilFormatSize(1024 * 1024 * 1024 * 1024ull * 1024ull * 10));
}
#endif

std::wstring UtilFormatTime(__time64_t timer)
{
	std::wstring buf;
	buf.resize(64);
	struct tm localtime;
	_localtime64_s(&localtime, &timer);
	for (;;buf.resize(buf.size()*2)) {
		auto ret = wcsftime(&buf[0], buf.size(), L"%c", &localtime);
		if (0 != ret)return buf.c_str();
	}
}
#ifdef UNIT_TEST
TEST(StringUtil, UtilFormatTime) {
	std::string currentLocale = setlocale(LC_TIME, nullptr);
	auto ret = setlocale(LC_TIME, "ja_JP");
	EXPECT_NE(nullptr, ret);
	EXPECT_EQ(L"2020/05/01 21:50:42", UtilFormatTime(1588337442ul));

	setlocale(LC_TIME, currentLocale.c_str());
}
#endif

//loads string from resource
std::wstring UtilLoadString(UINT uID)
{
	wchar_t buf[256];
	LoadStringW(GetModuleHandleW(nullptr), uID, buf, 256);
	return buf;
}
#ifdef UNIT_TEST
TEST(StringUtil, UtilLoadString) {
	EXPECT_EQ(L"LhaForge", UtilLoadString(IDS_MESSAGE_CAPTION));
}
#endif

#ifdef UNIT_TEST


TEST(StringUtil, Format) {
	EXPECT_EQ(L"", Format(L""));
	EXPECT_EQ(L"1234567", Format(L"%d45%d", 123, 67));
	EXPECT_EQ(L"abcde", Format(L"%sde", L"abc"));
	EXPECT_EQ(L"12.35", Format(L"%.2f", 12.3456));
	EXPECT_EQ(L"あいうえお", Format(L"%sえお", L"あいう"));
}

TEST(StringUtil, replace) {
	EXPECT_EQ(L"", replace(L"", L"", L""));
	EXPECT_EQ(L"", replace(L"", L"", L"aa"));
	EXPECT_EQ(L"abcdeab", replace(L"12cde12", L"12", L"ab"));
	EXPECT_EQ(L"abcいde12", replace(L"12cいde12", L"12", L"ab", true));

	EXPECT_EQ(L"あいうcdeあいう", replace(L"12cde12", L"12", L"あいう"));
}

TEST(StringUtil, toLower) {
	EXPECT_EQ(L"", toLower(L""));
	EXPECT_EQ(L"abcde", toLower(L"abcde"));
	EXPECT_EQ(L"abcde", toLower(L"aBcDe"));
	EXPECT_EQ(L"あいうcde", toLower(L"あいうCdE"));
	EXPECT_EQ(L"👪", toLower(L"👪"));	//emoji
}

TEST(StringUtil, toUpper) {
	EXPECT_EQ(L"", toUpper(L""));
	EXPECT_EQ(L"ABCDE", toUpper(L"abcde"));
	EXPECT_EQ(L"ABCDE", toUpper(L"aBcDe"));
	EXPECT_EQ(L"あいうCDE", toUpper(L"あいうCdE"));
	EXPECT_EQ(L"👪", toUpper(L"👪"));	//emoji
}

TEST(StringUtil, join) {
	EXPECT_EQ(L"abc", join(L", ", std::vector<std::wstring>({ L"abc" })));
	EXPECT_EQ(L"abc, def", join(L", ", std::vector<std::wstring>({ L"abc", L"def" })));
	EXPECT_EQ(L"abc, def", join(L", ", std::vector<std::wstring>({ L"abc", L"def",L"ghi" }), 2));
}
#endif

