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

int ErrorMessage(const std::wstring& message)
{
	TRACE(L"ErrorMessage:%s\n", message.c_str());
	return UtilMessageBox(NULL, message, MB_OK | MB_ICONSTOP);
}

int UtilMessageBox(HWND hWnd, const std::wstring& message, UINT uType)
{
	const CString strCaption(MAKEINTRESOURCE(IDS_MESSAGE_CAPTION));
	return MessageBoxW(
		hWnd,
		message.c_str(),
		strCaption,
		uType);
}

std::wstring UtilGetLastErrorMessage(DWORD langID, DWORD errorCode)
{
	LPVOID lpMsgBuf;
	FormatMessageW(
		FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, errorCode,
		langID,
		(wchar_t*)&lpMsgBuf, 0, NULL);

	std::wstring out=(const wchar_t*)lpMsgBuf;
	LocalFree(lpMsgBuf);
	return out;
}

std::vector<std::wstring> UtilReadFromResponseFile(const std::wstring& respFile, UTIL_CODEPAGE uSrcCodePage)
{
	std::vector<BYTE> cReadBuffer = UtilReadFile(respFile);

	//adding \0 to end
	cReadBuffer.push_back('\0');
	cReadBuffer.push_back('\0');

	//encoding
	auto content = UtilToUNICODE((const char*)&cReadBuffer[0], cReadBuffer.size(), uSrcCodePage);

	std::wstring line;
	std::vector<std::wstring> files;
	for (const auto& c: content) {
		if (c == L'\n' || c == L'\r' || c == L'\0') {
			if (!line.empty()) {
				if (line.front() == L'"' && line[line.length() - 1] == L'"') {
					//unquote
					line = std::wstring(&line[1], &line[line.length() - 1]);
				}
				files.push_back(line);
			}
			line.clear();
		} else {
			line += c;
		}
	}
	return files;
}

//checks if path extension matches specific patterns
//pattern_string may contain only one pattern, such as "*.txt" and/or "*.do?"
bool UtilExtMatchSpec(const std::wstring& path, const std::wstring& pattern_string)
{
	if (pattern_string.empty())return false;
	//characters to be escaped
	const std::wstring escapeSubjects = L".(){}[]\\+^$|";

	//compatibility
	auto pattern = pattern_string;
	if (pattern.find(L"*.") == 0) {
		pattern = pattern.substr(1);
	}

	std::wstring regex_str;
	if (pattern[0] != L'.') {
		regex_str += L"\\.";
	}
	for (const auto& p : pattern) {
		if (isIn(escapeSubjects, p)) {
			regex_str += L"\\";
			regex_str += p;
		} else if (p == L'*') {
			regex_str += L".*?";
		} else if (p == L'?') {
			regex_str += L".";
		} else {
			regex_str += p;
		}
	}

	if (regex_str.empty())return false;
	regex_str += L"$";

	std::wregex re(toLower(regex_str));
	return std::regex_search(toLower(path), re);
}

//checks if path matches specific patterns
//pattern_string may contain only one pattern, such as "*.txt" and/or "*.do?"
bool UtilPathMatchSpec(const std::wstring& path, const std::wstring& pattern_string)
{
	if (pattern_string.empty())return false;
	//characters to be escaped
	const std::wstring escapeSubjects = L".(){}[]\\+^$|";

	auto pattern = replace(pattern_string, L"*.*", L"*");	//compatibility
	std::wstring regex_str;
	for (const auto& p : pattern) {
		if (isIn(escapeSubjects, p)) {
			regex_str += L"\\";
			regex_str += p;
		} else if (p == L'*') {
			regex_str += L".*?";
		} else if (p == L'?') {
			regex_str += L".";
		} else {
			regex_str += p;
		}
	}

	if (regex_str.empty())return false;

	std::wregex re(toLower(regex_str));
	return std::regex_search(toLower(path), re);
}

bool UtilDoMessageLoop()
{
	MSG msg;
	if (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE)) {
		if (!GetMessage(&msg, NULL, 0, 0)) {
			return false;
		}

		TranslateMessage(&msg);
		DispatchMessage(&msg);
		return true;
	}
	return false;
}






