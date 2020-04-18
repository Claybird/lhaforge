#include "stdafx.h"
#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTest
{
	TEST_CLASS(extract)
	{
	public:
		TEST_METHOD(test_LF_sanitize_pathname) {
			std::wstring LF_sanitize_pathname(const std::wstring rawPath);
			Assert::AreEqual(std::wstring(L""), LF_sanitize_pathname(L""));
			Assert::AreEqual(std::wstring(L""), LF_sanitize_pathname(L"//"));
			Assert::AreEqual(std::wstring(L"a"), LF_sanitize_pathname(L"/a"));
			Assert::AreEqual(std::wstring(L"a"), LF_sanitize_pathname(L"//a"));
			Assert::AreEqual(std::wstring(L"a/"), LF_sanitize_pathname(L"//a////"));
			Assert::AreEqual(std::wstring(L"a_@@@_b"), LF_sanitize_pathname(L"a\\b"));
			Assert::AreEqual(std::wstring(L"a/b"), LF_sanitize_pathname(L"a/./././b"));
			Assert::AreEqual(std::wstring(L"c/_@@@_/d"), LF_sanitize_pathname(L"c/../d"));
			Assert::AreEqual(std::wstring(L"e/_@@@_/f"), LF_sanitize_pathname(L"e/....../f"));
		}

		TEST_METHOD(test_trimArchiveName) {
			std::wstring trimArchiveName(bool RemoveSymbolAndNumber, const wchar_t* archive_path);
			Assert::AreEqual(std::wstring(L""), trimArchiveName(true, L""));

			Assert::AreEqual(std::wstring(L"123"), trimArchiveName(true, L"123"));	//restore original
			Assert::AreEqual(std::wstring(L"123"), trimArchiveName(false, L"123"));

			Assert::AreEqual(std::wstring(L"123abc"), trimArchiveName(true, L"123abc456"));
			Assert::AreEqual(std::wstring(L"123abc456"), trimArchiveName(false, L"123abc456"));

			Assert::AreEqual(std::wstring(L"123abc"), trimArchiveName(true, L"123abc456[1]"));
			Assert::AreEqual(std::wstring(L"123abc456[1]"), trimArchiveName(false, L"123abc456[1]"));

			Assert::AreEqual(std::wstring(L"123abc"), trimArchiveName(true, L"123abc456."));
			Assert::AreEqual(std::wstring(L"123abc456"), trimArchiveName(false, L"123abc456."));

			Assert::AreEqual(std::wstring(L""), trimArchiveName(true, L"123abc456\\"));
			Assert::AreEqual(std::wstring(L""), trimArchiveName(false, L"123abc456\\"));

			Assert::AreEqual(std::wstring(L"123abc"), trimArchiveName(true, L"123abc456 "));
			Assert::AreEqual(std::wstring(L"123abc456"), trimArchiveName(false, L"123abc456 "));

			//full-width space
			Assert::AreEqual(std::wstring(L"123abc"), trimArchiveName(true, L"123abc456Å@"));
			Assert::AreEqual(std::wstring(L"123abc456"), trimArchiveName(false, L"123abc456Å@"));
		}
	};
}
