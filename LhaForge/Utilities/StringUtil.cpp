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
std::wstring UtilMakeFilterString(const wchar_t* lpszIn)
{
	std::wstring out = lpszIn;
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


inline bool between(WCHAR a,WCHAR begin,WCHAR end){
	return (begin<=a && a<=end);
}

//指定されたフォーマットで書かれた文字列を展開する
void UtilExpandTemplateString(CString &strOut,LPCTSTR lpszFormat,const std::map<stdString,CString> &env)
{
	strOut=lpszFormat;
	for(std::map<stdString,CString>::const_iterator ite=env.begin();ite!=env.end();++ite){
		//%で始まるブロックは環境変数と見なすので{}による修飾は行わない
		stdString strKey=( (*ite).first[0]==L'%' ? (*ite).first : _T("{")+(*ite).first+_T("}") );
		strOut.Replace(strKey.c_str(),(*ite).second);
	}
}

void UtilAssignSubString(CString &strOut,LPCTSTR lpStart,LPCTSTR lpEnd)
{
	ASSERT(lpStart);
	ASSERT(lpEnd);
	strOut=_T("");
	for(;lpStart!=lpEnd;++lpStart){
		strOut+=*lpStart;
	}
}


//文字列を分解し数値配列として取得
void UtilStringToIntArray(LPCTSTR str, std::vector<int>& numArr)
{
	numArr.clear();

	for(;_T('\0')!=*str;){
		CString Temp;
		for(;;){
			if(_T(',')==*str||_T('\0')==*str){
				str++;
				break;
			}else{
				Temp += *str;
				str++;
			}
		}
		int num = _ttoi(Temp);
		numArr.push_back(num);
	}
}

std::vector<std::wstring> split_string(const std::wstring& target, const std::wstring& separator)
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
		if (0 != ret)return buf;
	}
}
