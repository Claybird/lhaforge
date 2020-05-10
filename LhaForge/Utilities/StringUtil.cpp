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

std::wstring UtilTrimString(const std::wstring &target, const std::wstring &trimTargets)
{
	auto idx = target.find_last_not_of(trimTargets);
	if (idx == std::wstring::npos)return L"";
	else return target.substr(0, idx + 1);
}

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

//expand variables, "{foo}" for process specific variables, and "%bar" for OS environment variables
std::wstring UtilExpandTemplateString(const std::wstring& format, const std::map<std::wstring, std::wstring> &envVars)
{
	auto fmt = format;
	return util_regex_replace(fmt.cbegin(), fmt.cend(), std::wregex(L"\\{([^;]*?)\\}|%([^;]*?)%"),
		[&](const std::wsmatch& m) {
		auto key = m.str(0);
		auto found = envVars.find(key.substr(1, key.length() - 2));
		if (found==envVars.end()) {
			return key;
		} else {
			return (*found).second;
		}
	});
}


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

//split string into number array
std::vector<int> UtilStringToIntArray(const std::wstring& str)
{
	std::vector<int> numArr;
	for(const auto& sub: UtilSplitString(str, L",")){
		numArr.push_back(_wtoi(sub.c_str()));
	}
	return numArr;
}


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

//loads string from resource
std::wstring UtilLoadString(UINT uID)
{
	wchar_t buf[256];
	LoadStringW(GetModuleHandleW(nullptr), uID, buf, 256);
	return buf;
/*	const wchar_t* ptr = nullptr;
	int length = LoadStringW(GetModuleHandleW(nullptr), uID, (LPWSTR)&ptr, 0);
	std::wstring buf;
	ASSERT(ptr);
	if (ptr) {
		buf.assign(ptr, ptr + length + 1);
	}
	return buf;*/
}
