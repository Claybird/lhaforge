#include "stdafx.h"
#ifdef UNIT_TEST
#include "CppUnitTest.h"
#include "Utilities/Utility.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTest
{
	TEST_CLASS(Utility)
	{
	public:
		TEST_METHOD(test_UtilGetLastErrorMessage) {
			Assert::AreEqual(std::wstring(L"The system cannot find the path specified.\r\n"), 
				UtilGetLastErrorMessage(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), ERROR_PATH_NOT_FOUND));
		}
		TEST_METHOD(test_BOOL2bool) {
			Assert::IsTrue(BOOL2bool(TRUE));
			Assert::IsFalse(BOOL2bool(FALSE));
		}
		TEST_METHOD(test_UtilReadFromResponseFile) {
			auto file = std::filesystem::path(__FILEW__).parent_path() / L"test_utility_response1.txt";
			auto files = UtilReadFromResponseFile(file, UTIL_CODEPAGE::UTF8);
			Assert::AreEqual(size_t(4), files.size());
			Assert::AreEqual(std::wstring(L"ファイル1.txt"), files[0]);
			Assert::AreEqual(std::wstring(L"C:\\program files\\b.txt"), files[1]);
			Assert::AreEqual(std::wstring(L"ファイル3.doc"), files[2]);
			Assert::AreEqual(std::wstring(L"#d.exe"), files[3]);

			file = std::filesystem::path(__FILEW__).parent_path() / L"path_that_does_not_exist.txt";
			Assert::ExpectException<LF_EXCEPTION>([&]{
				UtilReadFromResponseFile(file, UTIL_CODEPAGE::UTF8);
			});
		}
		TEST_METHOD(test_UtilExtMatchSpec) {
			//---single
			Assert::IsTrue(UtilExtMatchSpec(L"test.abc", L"*.*"));
			Assert::IsTrue(UtilExtMatchSpec(L"test.abc", L".*"));
			Assert::IsTrue(UtilExtMatchSpec(L"test.abc", L"*"));
			Assert::IsFalse(UtilExtMatchSpec(L"", L""));
			Assert::IsFalse(UtilExtMatchSpec(L"", L"*.abc"));
			Assert::IsTrue(UtilExtMatchSpec(L"test.abc", L"*.abc"));
			Assert::IsTrue(UtilExtMatchSpec(L"test.abc", L"abc"));
			Assert::IsTrue(UtilExtMatchSpec(L"test.abc", L".abc"));
			Assert::IsTrue(UtilExtMatchSpec(L"test.ABC", L"abc"));
			Assert::IsFalse(UtilExtMatchSpec(L"test.abc", L"test.abc"));
			Assert::IsFalse(UtilExtMatchSpec(L"test.abc", L"*.test"));
			Assert::IsFalse(UtilExtMatchSpec(L"test.abc", L"test"));
			Assert::IsTrue(UtilExtMatchSpec(L"test.abc", L"ab*"));
			Assert::IsTrue(UtilExtMatchSpec(L"test.abc", L"abc*"));
			Assert::IsTrue(UtilExtMatchSpec(L"test.abc", L"??c"));
			Assert::IsFalse(UtilExtMatchSpec(L"test.abc", L"??d"));
			Assert::IsTrue(UtilExtMatchSpec(L"test.tar.gz", L"tar.gz"));
			Assert::IsFalse(UtilExtMatchSpec(L"test.tar.gz", L""));

			//---multiple
			Assert::IsTrue(UtilExtMatchSpec(L"あいう.えお", L"*.abc;えお"));
			Assert::IsTrue(UtilExtMatchSpec(L"test.abc", L"abc;def;.ghi"));
			Assert::IsTrue(UtilExtMatchSpec(L"test.doc", L"abc;doc*"));
			Assert::IsTrue(UtilExtMatchSpec(L"test.docx", L"abc;doc*"));
			Assert::IsFalse(UtilExtMatchSpec(L"test.txt", L"abc;doc*;*.exe"));
			Assert::IsFalse(UtilExtMatchSpec(L"test.txt", L";;"));

			//---possible regex
			Assert::IsFalse(UtilExtMatchSpec(L"test.txt", L"(.*)"));
			Assert::IsFalse(UtilExtMatchSpec(L"test.txt", L"[a-Z]*"));
			Assert::IsFalse(UtilExtMatchSpec(L"test.txt", L"\\"));
			Assert::IsFalse(UtilExtMatchSpec(L"test.txt", L"$"));
			Assert::IsFalse(UtilExtMatchSpec(L"test.txt", L"^"));
			Assert::IsFalse(UtilExtMatchSpec(L"test.txt", L"txt|abc"));

			//---no name part or no exts
			Assert::IsTrue(UtilExtMatchSpec(L".gitignore", L".gitignore"));
			Assert::IsTrue(UtilExtMatchSpec(L"abc.gitignore", L".gitignore"));
			Assert::IsFalse(UtilExtMatchSpec(L"test", L"test"));
		}
		TEST_METHOD(test_has_key) {
			std::map<std::wstring, std::wstring> m;
			m[L"abc"] = L"abc";
			m[L"あいう"] = L"あいう";
			Assert::IsTrue(has_key(m, L"abc"));
			Assert::IsTrue(has_key(m, L"あいう"));
			Assert::IsFalse(has_key(m, L"cde"));
		}
		TEST_METHOD(test_index_of) {
			std::vector<int> a = { 2,4,6,8,10 };
			Assert::AreEqual(1, index_of(a, 4));
			Assert::AreEqual(4, index_of(a, 10));
			Assert::AreEqual(-1, index_of(a, 1));
			Assert::AreEqual(-1, index_of(a, 11));

			Assert::AreEqual(1, index_of(&a[0], a.size(), 4));
			Assert::AreEqual(4, index_of(&a[0], a.size(), 10));
			Assert::AreEqual(-1, index_of(&a[0], a.size(), 1));
			Assert::AreEqual(-1, index_of(&a[0], a.size(), 11));
		}
		TEST_METHOD(test_remove_item) {
			std::vector<int> a = { 2,4,6,6,6,10 };
			Assert::AreNotEqual(-1, index_of(a, 6));
			remove_item(a, 6);
			Assert::AreEqual(-1, index_of(a, 6));
		}
		TEST_METHOD(test_remove_item_if) {
			std::vector<int> a = { 2,4,6,6,6,10 };
			Assert::AreNotEqual(-1, index_of(a, 6));
			remove_item_if(a, [](int value) {return value / 2 == 3; });
			Assert::AreEqual(-1, index_of(a, 6));
		}
		TEST_METHOD(test_isIn) {
			std::vector<int> a = { 2,4,6,8,10 };
			Assert::IsTrue(isIn(a, 4));
			Assert::IsFalse(isIn(a, 3));
		}
	};
};

#endif
