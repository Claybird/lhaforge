#include "stdafx.h"
#ifdef UNIT_TEST
#include "CppUnitTest.h"
#include "Utilities/StringUtil.h"



using namespace Microsoft::VisualStudio::CppUnitTestFramework;


namespace UnitTest
{
	TEST_CLASS(StringUtil)
	{
	public:
		TEST_METHOD(test_Format) {
			Assert::AreEqual(std::wstring(L""), Format(L""));
			Assert::AreEqual(std::wstring(L"1234567"), Format(L"%d45%d", 123, 67));
			Assert::AreEqual(std::wstring(L"abcde"), Format(L"%sde", L"abc"));
			Assert::AreEqual(std::wstring(L"あいうえお"), Format(L"%sえお", L"あいう"));
		}

		TEST_METHOD(test_UtilTrimString) {
			Assert::AreEqual(std::wstring(L"abc"), UtilTrimString(L"abc \t  ",L" \t"));
			Assert::AreEqual(std::wstring(L"123abc"), UtilTrimString(L"123abc456", L"0123456789"));
			Assert::AreEqual(std::wstring(L"123abc"), UtilTrimString(L"123abcあいうえお", L"123あいうえお"));
		}

		TEST_METHOD(test_UtilMakeFilterString) {
			const wchar_t raw[] = L"abc\0def\0\0";
			auto out = UtilMakeFilterString(L"abc|def");
			for (auto i = 0; i < sizeof(raw) / sizeof(raw[0]); i++) {
				Assert::AreEqual(raw[i], out[i]);
			}
		}

		TEST_METHOD(test_Encodings) {
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

			Assert::AreEqual(std::wstring(utf8_ans), UtilUTF8toUNICODE(utf8, sizeof(utf8)));
			//with BOM
			Assert::AreEqual(std::wstring(utf8_bom_ans), UtilUTF8toUNICODE(utf8_bom, sizeof(utf8_bom)));

			Assert::AreEqual(std::wstring(cp932_ans), UtilCP932toUNICODE(cp932, sizeof(cp932)));

			Assert::AreEqual(std::wstring(utf16_le_ans), UtilUTF16toUNICODE(utf16_le, sizeof(utf16_le)));
			//with BOM
			//UTF16-LE
			Assert::AreEqual(std::wstring(utf16_le_bom_ans), UtilUTF16toUNICODE(utf16_le_bom, sizeof(utf16_le_bom)));
			//UTF16-BE
			Assert::AreEqual(std::wstring(utf16_be_bom_ans), UtilUTF16toUNICODE(utf16_be_bom, sizeof(utf16_be_bom)));

			//decode
			Assert::AreEqual(std::wstring(cp932_ans),
				UtilToUNICODE(cp932, sizeof(cp932), UTIL_CODEPAGE::CP932));
			Assert::AreEqual(std::wstring(utf16_le_ans),
				UtilToUNICODE(utf16_le, sizeof(utf16_le), UTIL_CODEPAGE::UTF16));
			Assert::AreEqual(std::wstring(utf16_le_bom_ans),
				UtilToUNICODE(utf16_le_bom, sizeof(utf16_le_bom), UTIL_CODEPAGE::UTF16));
			Assert::AreEqual(std::wstring(utf16_be_bom_ans),
				UtilToUNICODE(utf16_be_bom, sizeof(utf16_be_bom), UTIL_CODEPAGE::UTF16));
			Assert::AreEqual(std::wstring(utf8_ans),
				UtilToUNICODE(utf8, sizeof(utf8), UTIL_CODEPAGE::UTF8));
			Assert::AreEqual(std::wstring(utf8_bom_ans),
				UtilToUNICODE(utf8_bom, sizeof(utf8_bom), UTIL_CODEPAGE::UTF8));

			//CP932
			Assert::IsTrue(UtilVerityGuessedCodepage(cp932, sizeof(cp932), UTIL_CODEPAGE::CP932));
			Assert::IsFalse(UtilVerityGuessedCodepage(cp932, sizeof(cp932), UTIL_CODEPAGE::UTF16));
			Assert::IsFalse(UtilVerityGuessedCodepage(cp932, sizeof(cp932), UTIL_CODEPAGE::UTF8));

			//UTF16-LE
			Assert::IsFalse(UtilVerityGuessedCodepage(utf16_le, sizeof(utf16_le), UTIL_CODEPAGE::CP932));
			Assert::IsTrue(UtilVerityGuessedCodepage(utf16_le, sizeof(utf16_le), UTIL_CODEPAGE::UTF16));
			Assert::IsFalse(UtilVerityGuessedCodepage(utf16_le, sizeof(utf16_le), UTIL_CODEPAGE::UTF8));

			//UTF16-LE-BOM
			Assert::IsFalse(UtilVerityGuessedCodepage(utf16_le_bom, sizeof(utf16_le_bom), UTIL_CODEPAGE::CP932));
			Assert::IsTrue(UtilVerityGuessedCodepage(utf16_le_bom, sizeof(utf16_le_bom), UTIL_CODEPAGE::UTF16));
			Assert::IsFalse(UtilVerityGuessedCodepage(utf16_le_bom, sizeof(utf16_le_bom), UTIL_CODEPAGE::UTF8));

			//UTF16-BE-BOM
			Assert::IsFalse(UtilVerityGuessedCodepage(utf16_be_bom, sizeof(utf16_be_bom), UTIL_CODEPAGE::CP932));
			Assert::IsTrue(UtilVerityGuessedCodepage(utf16_be_bom, sizeof(utf16_be_bom), UTIL_CODEPAGE::UTF16));
			Assert::IsFalse(UtilVerityGuessedCodepage(utf16_be_bom, sizeof(utf16_be_bom), UTIL_CODEPAGE::UTF8));

			//UTF-8
			Assert::IsFalse(UtilVerityGuessedCodepage(utf8, sizeof(utf8), UTIL_CODEPAGE::CP932));
			Assert::IsFalse(UtilVerityGuessedCodepage(utf8, sizeof(utf8), UTIL_CODEPAGE::UTF16));
			Assert::IsTrue(UtilVerityGuessedCodepage(utf8, sizeof(utf8), UTIL_CODEPAGE::UTF8));

			//UTF-8-BOM
			Assert::IsFalse(UtilVerityGuessedCodepage(utf8_bom, sizeof(utf8_bom), UTIL_CODEPAGE::CP932));
			Assert::IsFalse(UtilVerityGuessedCodepage(utf8_bom, sizeof(utf8_bom), UTIL_CODEPAGE::UTF16));
			Assert::IsTrue(UtilVerityGuessedCodepage(utf8_bom, sizeof(utf8_bom), UTIL_CODEPAGE::UTF8));

		}

		TEST_METHOD(test_UtilExpandTemplateString) {
			std::map<std::wstring, std::wstring> envVars;
			envVars[L"abc"] = L"123";
			envVars[L"def"] = L"{abc}";
			Assert::AreEqual(
				std::wstring(L"123;123;{abc};{ghi};%jkl%;%F;%mnopq{}"),
				UtilExpandTemplateString(L"%abc%;{abc};{def};{ghi};%jkl%;%F;%mnopq{}", envVars));
		}


		TEST_METHOD(test_UtilFormatSize) {
			Assert::AreEqual(std::wstring(L"---"), UtilFormatSize(-1));
			Assert::AreEqual(std::wstring(L"1 Bytes"), UtilFormatSize(1));
			Assert::AreEqual(std::wstring(L"1000 Bytes"), UtilFormatSize(1000));
			Assert::AreEqual(std::wstring(L"1 KB"), UtilFormatSize(1024));
			Assert::AreEqual(std::wstring(L"1 MB"), UtilFormatSize(1024 * 1024));
			Assert::AreEqual(std::wstring(L"1 GB"), UtilFormatSize(1024 * 1024 * 1024));
			Assert::AreEqual(std::wstring(L"1 TB"), UtilFormatSize(1024 * 1024 * 1024 * 1024ull));
			Assert::AreEqual(std::wstring(L"1 PB"), UtilFormatSize(1024 * 1024 * 1024 * 1024ull * 1024ull));
			Assert::AreEqual(std::wstring(L"10 PB"), UtilFormatSize(1024 * 1024 * 1024 * 1024ull * 1024ull * 10));
		}

	};
};
#endif
