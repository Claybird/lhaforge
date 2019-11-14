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

#pragma once
//---文字列処理

//文字コード
enum UTIL_CODEPAGE{
	UTILCP_SJIS,
	UTILCP_UTF8,
	UTILCP_UTF16
};


//MFCスタイルでCFileDialogのフィルター文字列を作る
void UtilMakeFilterString(LPCTSTR,LPTSTR,int);

struct CUTF8String {
	CUTF8String() {}
	CUTF8String(const CUTF8String& utf8) {
		_utf8_str = utf8._utf8_str;
	}
	CUTF8String(const char* utf8) {
		_utf8_str = (const char*)utf8;
	}
	CUTF8String(const std::wstring& wstr) {
		this->operator=(wstr);
	}
	virtual ~CUTF8String() {}
	std::string _utf8_str;
	std::wstring toWstring()const {
		int bufSize = ::MultiByteToWideChar(CP_UTF8, 0, _utf8_str.c_str(), -1, NULL, 0);

		std::wstring wstr;
		wstr.resize(bufSize);

		::MultiByteToWideChar(CP_UTF8, 0, _utf8_str.c_str(), -1, &wstr[0], bufSize);
		return wstr;
	}
	const char* utf8() const { return _utf8_str.c_str(); }
	const CUTF8String& operator=(const CUTF8String& c) {
		_utf8_str = c._utf8_str;
		return *this;
	}
	const CUTF8String& operator=(LPCBYTE utf8) {
		_utf8_str = (const char*)utf8;
	}
	const CUTF8String& operator=(const std::wstring& wstr) {
		int bufSize = ::WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, NULL, 0, NULL, NULL);
		_utf8_str.resize(bufSize);
		::WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &_utf8_str[0], bufSize, NULL, NULL);
		return *this;
	}
	size_t length_utf8()const { return _utf8_str.length(); }
};


//適当な文字コード->UNICODE
//dwSizeはUTILCP_UTF16のときのみ必要
bool UtilToUNICODE(CString &strRet,LPCBYTE lpcByte,DWORD dwSize,UTIL_CODEPAGE uSrcCodePage);
//UTF16-BE/UTF16-LE/SJISを自動判定してUNICODEに
void UtilGuessToUNICODE(CString &strRet,LPCBYTE lpcByte,DWORD dwSize);

//UNICODEとして安全ならtrue
bool UtilIsSafeUnicode(LPCTSTR);

//TCHARファイル名がSJISファイル名で表現できるならtrue
bool UtilCheckT2A(LPCTSTR);
bool UtilCheckT2AList(const std::list<CString>&);	//複数ファイルのうち、一つでもUNICODE専用ファイル名があればfalse

//末尾から指定された文字を削る
void UtilTrimString(CStringW&,LPCWSTR lpszSubject);

//指定されたフォーマットで書かれた文字列を展開する
void UtilExpandTemplateString(CString &strOut,LPCTSTR lpszFormat,const std::map<stdString,CString> &env);

void UtilAssignSubString(CString &strOut,LPCTSTR lpStart,LPCTSTR lpEnd);

//文字列を分解し数値配列として取得
void UtilStringToIntArray(LPCTSTR, std::vector<int>&);

template <typename ...Args>
std::wstring Format(const wchar_t* fmt, Args && ...args)
{
	std::wstring work;
	auto size = _snwprintf_s(nullptr, 0, 0, fmt, std::forward<Args>(args)...);
	work.resize(size);
	_snwprintf_s(&work[0], work.size(), work.size(), fmt, std::forward<Args>(args)...);
	return work;
}

std::wstring FormatFileTime(const FILETIME &rFileTime)
{
	if (-1 == rFileTime.dwHighDateTime && -1 == rFileTime.dwLowDateTime) {
		return L"------";
	} else {
		FILETIME LocalFileTime;
		SYSTEMTIME SystemTime;

		FileTimeToLocalFileTime(&rFileTime, &LocalFileTime);
		FileTimeToSystemTime(&LocalFileTime, &SystemTime);

		return Format(L"%04d/%02d/%02d %02d:%02d:%02d",
			SystemTime.wYear, SystemTime.wMonth, SystemTime.wDay,
			SystemTime.wHour, SystemTime.wMinute, SystemTime.wSecond);
	}
}
