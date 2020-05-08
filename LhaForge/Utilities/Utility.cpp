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

int ErrorMessage(const wchar_t* message)
{
	TRACE(L"ErrorMessage:%s\n", message);
	return UtilMessageBox(NULL, message, MB_OK | MB_ICONSTOP);
}

int UtilMessageBox(HWND hWnd, const wchar_t* lpText, UINT uType)
{
	const CString strCaption(MAKEINTRESOURCE(IDS_MESSAGE_CAPTION));
	return MessageBoxW(
		hWnd,
		lpText,
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

std::vector<std::wstring> UtilReadFromResponseFile(const wchar_t* lpszRespFile, UTIL_CODEPAGE uSrcCodePage)
{
	std::vector<BYTE> cReadBuffer = UtilReadFile(lpszRespFile);

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
//pattern_string may contain multiple patterns separated with ';', such as "*.txt;*.do?"
bool UtilExtMatchSpec(const wchar_t* path, const wchar_t* pattern_string)
{
	//characters to be escaped
	const std::wstring escapeSubjects = L".(){}[]\\+^$|";
	auto patterns = UtilSplitString(pattern_string, L";");

	std::wstring regex_string;
	for (auto& pattern : patterns) {
		if (pattern.empty())continue;
		//compatibility
		if (pattern.find(L"*.") == 0) {
			pattern = pattern.substr(1);
		}

		std::wstring pstr;
		if (pattern[0] != L'.') {
			pstr += L"\\.";
		}
		for (const auto& p : pattern) {
			if (isIn(escapeSubjects, p)) {
				pstr += L"\\";
				pstr += p;
			} else if (p == L'*') {
				pstr += L".*?";
			} else if (p == L'?') {
				pstr += L".";
			} else {
				pstr += p;
			}
		}

		if (!regex_string.empty()) {
			regex_string += L'|';
		}
		regex_string += L"(" + pstr + L"$)";
	}

	if (regex_string.empty())return false;

	std::wregex re(toLower(regex_string));
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






