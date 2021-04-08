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

#ifdef UNIT_TEST
TEST(Utility, UtilGetLastErrorMessage) {
	EXPECT_EQ(L"The system cannot find the path specified.\r\n",
		UtilGetLastErrorMessage(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), ERROR_PATH_NOT_FOUND));
}
#endif

std::vector<std::wstring> UtilReadFromResponseFile(const std::filesystem::path& respFile, UTIL_CODEPAGE uSrcCodePage)
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

#ifdef UNIT_TEST

TEST(Utility, UtilReadFromResponseFile) {
	auto file = std::filesystem::path(__FILEW__).parent_path() / L"test/test_utility_response1.txt";
	auto files = UtilReadFromResponseFile(file, UTIL_CODEPAGE::UTF8);
	EXPECT_EQ(size_t(4), files.size());
	EXPECT_EQ(L"ファイル1.txt", files[0]);
	EXPECT_EQ(L"C:\\program files\\b.txt", files[1]);
	EXPECT_EQ(L"ファイル3.doc", files[2]);
	EXPECT_EQ(L"#d.exe", files[3]);

	file = std::filesystem::path(__FILEW__).parent_path() / L"test/path_that_does_not_exist.txt";
	EXPECT_THROW(UtilReadFromResponseFile(file, UTIL_CODEPAGE::UTF8), LF_EXCEPTION);
}
#endif

//checks if path extension matches specific patterns
//pattern_string may contain only one pattern, such as "*.txt" and/or "*.do?"
bool UtilExtMatchSpec(const std::filesystem::path& path, const std::wstring& pattern_string)
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

	std::wregex re(regex_str, std::regex_constants::icase);
	return std::regex_search(path.wstring(), re);
}

#ifdef UNIT_TEST
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
#endif

//checks if path matches specific patterns
//pattern_string may contain only one pattern, such as "*.txt" and/or "*.do?"
bool UtilPathMatchSpec(const std::filesystem::path& path, const std::wstring& pattern_string)
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

#ifdef UNIT_TEST
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
#endif

bool UtilDoMessageLoop()
{
	if (_Module.m_pMsgLoopMap) {
		CCustomMessageLoop* pLoop = (CCustomMessageLoop*)_Module.GetMessageLoop();
		return FALSE != pLoop->OneRun();
	} else {
		return false;
	}
}


#ifdef UNIT_TEST

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


#endif


