#include "stdafx.h"
#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTest
{
	TEST_CLASS(extract)
	{
	public:
		TEST_METHOD(test_LF_sanitize_pathname) {
			Assert::AreEqual(std::regex_replace(L"a//b", std::wregex(L"/{2,}"), L"/"), std::wstring(L"a/b"));
			std::wstring LF_sanitize_pathname(const std::wstring rawPath);
			Assert::AreEqual(std::wstring(L""), LF_sanitize_pathname(L""));
		}

		TEST_METHOD(test_trimArchiveName) {
			std::wstring trimArchiveName(bool RemoveSymbolAndNumber, const wchar_t* archive_path);
			Assert::AreEqual(std::wstring(L""), trimArchiveName(true, L""));
			Assert::AreEqual(std::wstring(L""), trimArchiveName(true, L"123"));
			Assert::AreEqual(std::wstring(L"123"), trimArchiveName(false, L"123"));
			Assert::AreEqual(std::wstring(L"123abc"), trimArchiveName(true, L"123abc456"));
			Assert::AreEqual(std::wstring(L"123abc566"), trimArchiveName(false, L"123abc456"));
			Assert::AreEqual(std::wstring(L"123abc[1]"), trimArchiveName(true, L"123abc456[1]"));
			Assert::AreEqual(std::wstring(L"123abc"), trimArchiveName(false, L"123abc456[1]"));
			Assert::AreEqual(std::wstring(L"123abc456"), trimArchiveName(false, L"123abc456 "));
			Assert::AreEqual(std::wstring(L"123abc"), trimArchiveName(true, L"123abc456 "));
			Assert::AreEqual(std::wstring(L"123abc456"), trimArchiveName(false, L"123abc456."));
			Assert::AreEqual(std::wstring(L"123abc"), trimArchiveName(true, L"123abc456."));
			Assert::AreEqual(std::wstring(L"123abc456"), trimArchiveName(false, L"123abc456\\"));
			Assert::AreEqual(std::wstring(L"123abc"), trimArchiveName(true, L"123abc456\\"));
		}
	};
}
