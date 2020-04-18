#include "stdafx.h"
#include "CppUnitTest.h"
#include "extract.h"

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

			Assert::AreEqual(std::wstring(L"abc_(UNICODE_CTRL)_def"), LF_sanitize_pathname(L"abc\u202Edef"));
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
			Assert::AreEqual(std::wstring(L"123abc"), trimArchiveName(true, L"123abc456　"));
			Assert::AreEqual(std::wstring(L"123abc456"), trimArchiveName(false, L"123abc456　"));
		}

		TEST_METHOD(test_determineExtractBaseDir) {
			std::wstring determineExtractBaseDir(const wchar_t* archive_path, LF_EXTRACT_ARGS& args);
			LF_EXTRACT_ARGS fakeArg;
			fakeArg.extract.OutputDirType = OUTPUT_TO::OUTPUT_TO_SPECIFIC_DIR;
			fakeArg.extract.OutputDirUserSpecified = std::filesystem::current_path().c_str();
			fakeArg.general.WarnNetwork = FALSE;
			fakeArg.general.WarnRemovable = FALSE;
			fakeArg.general.OnDirNotFound = LOSTDIR_FORCE_CREATE;

			auto out = determineExtractBaseDir(L"path_to_archive/archive.ext", fakeArg);
			Assert::AreEqual(std::wstring(std::filesystem::current_path().c_str()), out);
		}
		TEST_METHOD(test_determineExtractDir) {
			std::wstring determineExtractDir(const wchar_t* archive_path, const wchar_t* output_base_dir, const LF_EXTRACT_ARGS& args);
			LF_EXTRACT_ARGS fakeArg;
			fakeArg.extract.CreateDir = CREATE_OUTPUT_DIR_NEVER;
			fakeArg.extract.RemoveSymbolAndNumber = false;
			Assert::AreEqual(std::wstring(L"path_to_output"), 
				replace(determineExtractDir(L"path_to_archive/archive.ext", L"path_to_output", fakeArg), L"\\", L"/"));
			Assert::AreEqual(std::wstring(L"path_to_output"),
				replace(determineExtractDir(L"path_to_archive/archive  .ext", L"path_to_output", fakeArg), L"\\", L"/"));

			fakeArg.extract.CreateDir = CREATE_OUTPUT_DIR_ALWAYS;
			Assert::AreEqual(std::wstring(L"path_to_output/archive"),
				replace(determineExtractDir(L"path_to_archive/archive.ext", L"path_to_output", fakeArg), L"\\", L"/"));
			Assert::AreEqual(std::wstring(L"path_to_output/archive"),
				replace(determineExtractDir(L"path_to_archive/archive  .ext", L"path_to_output", fakeArg), L"\\", L"/"));
		}
	};
}
