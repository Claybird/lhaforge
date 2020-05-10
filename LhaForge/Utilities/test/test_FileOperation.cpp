﻿#include "stdafx.h"
#ifdef UNIT_TEST
#include "CppUnitTest.h"
#include "Utilities/FileOperation.h"
#include "Utilities/OSUtil.h"
#include "Utilities/Utility.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;


namespace UnitTest
{
	void touchFile(const std::wstring& path){
		FILE* fp = nullptr;
		Assert::AreEqual(0, _wfopen_s(&fp, path.c_str(), L"w"));
		Assert::IsNotNull(fp);
		fclose(fp);
	}
	TEST_CLASS(file_operation)
	{
	public:
		TEST_METHOD(test_UtilGetTempPath) {
			Assert::IsTrue(std::filesystem::exists(UtilGetTempPath()));
		}
		TEST_METHOD(test_UtilGetTemporaryFileName) {
			auto path = UtilGetTemporaryFileName();
			Assert::IsTrue(std::filesystem::exists(path));
			Assert::IsTrue(UtilDeletePath(path));
		}
		TEST_METHOD(test_UtilDeletePath) {
			//delete file
			auto path = UtilGetTemporaryFileName();
			Assert::IsTrue(std::filesystem::exists(path));
			Assert::IsTrue(UtilDeletePath(path));
			Assert::IsFalse(std::filesystem::exists(path));
			Assert::IsFalse(UtilDeletePath(path));

			//delete directory
			auto dir = UtilGetTempPath() + L"lhaforge_test";
			UtilDeletePath(dir);
			Assert::IsFalse(std::filesystem::exists(dir));
			std::filesystem::create_directories(dir);
			for (int i = 0; i < 100; i++) {
				touchFile(dir + Format(L"/a%03d.txt", i));
			}
			std::filesystem::create_directories(dir+L"/testDir");
			for (int i = 0; i < 100; i++) {
				touchFile(dir + Format(L"/testDir/b%03d.txt", i));
			}
			UtilDeletePath(dir);
			Assert::IsFalse(std::filesystem::exists(dir));
		}
		TEST_METHOD(test_UtilDeleteDir) {
			//delete file
			auto path = UtilGetTempPath() + L"test_UtilDeleteDir";
			Assert::IsFalse(std::filesystem::exists(path));
			std::filesystem::create_directories(path);
			Assert::IsTrue(std::filesystem::exists(path));

			touchFile(path + L"/test.txt");
			Assert::IsTrue(UtilDeleteDir(path, true));
			Assert::IsFalse(std::filesystem::exists(path));
		}
		TEST_METHOD(test_CTemporaryDirectoryManager) {
			std::wstring path;
			{
				CTemporaryDirectoryManager tmpMngr;
				path = tmpMngr.path();
				Assert::IsTrue(std::filesystem::exists(path));
			}
			Assert::IsFalse(std::filesystem::exists(path));
		}
		TEST_METHOD(test_UtilMoveFileToRecycleBin) {
			std::vector<std::wstring> fileList;
			//delete directory
			auto dir = UtilGetTempPath() + L"lhaforge_test";
			UtilDeletePath(dir);
			Assert::IsFalse(std::filesystem::exists(dir));
			std::filesystem::create_directories(dir);
			for (int i = 0; i < 3; i++) {
				fileList.push_back(dir + Format(L"/a%03d.txt", i));
				touchFile(dir + Format(L"/a%03d.txt", i));
			}
			Assert::IsTrue(UtilMoveFileToRecycleBin(fileList));
			UtilDeletePath(dir);
			Assert::IsFalse(std::filesystem::exists(dir));
		}
		TEST_METHOD(test_UtilRecursiveEnumFile_UtilPathExpandWild) {
			//prepare files
			std::vector<std::wstring> fileList;
			auto dir = UtilGetTempPath() + L"lhaforge_test";
			UtilDeletePath(dir);
			Assert::IsFalse(std::filesystem::exists(dir));
			std::filesystem::create_directories(dir);
			for (int i = 0; i < 3; i++) {
				fileList.push_back(dir + Format(L"/a%03d.txt", i));
				touchFile(dir + Format(L"/a%03d.txt", i));
			}
			std::filesystem::create_directories(dir + L"/b");
			for (int i = 0; i < 3; i++) {
				fileList.push_back(dir + Format(L"/b/a%03d.txt", i));
				touchFile(dir + Format(L"/b/a%03d.txt", i));
			}

			//enumerate
			{
				auto enumerated = UtilRecursiveEnumFile(dir);
				Assert::AreEqual(fileList.size(), enumerated.size());
				for (size_t i = 0; i < fileList.size(); i++) {
					Assert::AreEqual(
						std::filesystem::path(fileList[i]).make_preferred().wstring(),
						std::filesystem::path(enumerated[i]).make_preferred().wstring());
				}
			}

			//expand wild
			{
				auto enumerated = UtilPathExpandWild(dir);
				Assert::AreEqual(size_t(1), enumerated.size());
				Assert::AreEqual(enumerated[0], dir);

				auto pdir = std::filesystem::path(dir);
				enumerated = UtilPathExpandWild(pdir / L"*.txt");
				Assert::AreEqual(size_t(3), enumerated.size());
				Assert::IsTrue(isIn(enumerated, pdir / L"a000.txt"));
				Assert::IsTrue(isIn(enumerated, pdir / L"a001.txt"));
				Assert::IsTrue(isIn(enumerated, pdir / L"a002.txt"));

				enumerated = UtilPathExpandWild(pdir / L"*.exe");
				Assert::IsTrue(enumerated.empty());
			}

			//cleanup
			UtilDeletePath(dir);
			Assert::IsFalse(std::filesystem::exists(dir));
		}
		TEST_METHOD(test_UtilPathIsRoot) {
			Assert::IsTrue(UtilPathIsRoot(L"c:/"));
			Assert::IsTrue(UtilPathIsRoot(L"c:\\"));
			Assert::IsTrue(UtilPathIsRoot(L"c:"));
			Assert::IsFalse(UtilPathIsRoot(L"c:/windows/"));
			Assert::IsFalse(UtilPathIsRoot(L"c:\\windows\\"));
		}
		TEST_METHOD(test_UtilPathAddLastSeparator) {
			std::wstring sep;
			sep = std::filesystem::path::preferred_separator;
			Assert::AreEqual(std::wstring(sep), UtilPathAddLastSeparator(L""));
			Assert::AreEqual(L"C:" + sep, UtilPathAddLastSeparator(L"C:"));
			Assert::AreEqual(std::wstring(L"C:\\"), UtilPathAddLastSeparator(L"C:\\"));
			Assert::AreEqual(std::wstring(L"C:/"), UtilPathAddLastSeparator(L"C:/"));
			Assert::AreEqual(L"/tmp" + sep, UtilPathAddLastSeparator(L"/tmp"));
			Assert::AreEqual(std::wstring(L"/tmp\\"), UtilPathAddLastSeparator(L"/tmp\\"));
			Assert::AreEqual(std::wstring(L"/tmp/"), UtilPathAddLastSeparator(L"/tmp/"));
		}
		TEST_METHOD(test_UtilGetCompletePathName) {
			Assert::ExpectException<LF_EXCEPTION>([]() {
				UtilGetCompletePathName(L"");
			});
			Assert::IsTrue(UtilPathIsRoot(L"C:\\"));
			Assert::IsTrue(UtilPathIsRoot(UtilGetCompletePathName(L"C:")));
			Assert::IsTrue(UtilPathIsRoot(UtilGetCompletePathName(L"C:\\")));
			Assert::IsFalse(UtilPathIsRoot(UtilGetCompletePathName(L"C:\\Windows")));
			auto tempDir = std::filesystem::temp_directory_path();
			Assert::IsFalse(UtilPathIsRoot(UtilGetCompletePathName(tempDir)));
			{
				CCurrentDirManager mngr(tempDir);
				auto dest = L"C:\\Windows";
				auto relpath = std::filesystem::relative(dest);
				auto expected = toLower(std::filesystem::path(dest).make_preferred());
				auto actual = toLower(std::filesystem::path(UtilGetCompletePathName(relpath)));
				Assert::AreEqual(expected, actual);
			}
		}
		TEST_METHOD(test_UtilGetModulePath_UtilGetModuleDirectoryPath) {
			//TODO: is there any better test?
			Assert::IsFalse(UtilGetModulePath().empty());
			Assert::IsTrue(std::filesystem::exists(UtilGetModulePath()));
			Assert::IsTrue(std::filesystem::exists(UtilGetModuleDirectoryPath()));
		}
		TEST_METHOD(test_UtilReadFile) {
			//prepare
			auto fname = UtilGetTempPath() + L"lhaforge_test_file.tmp";
			{
				CAutoFile fp;
				fp.open(fname, L"w");
				fprintf(fp, "test file content");
			}
			{
				auto read = UtilReadFile(fname);
				Assert::AreEqual(size_t(17), read.size());
				read.push_back('\0');
				Assert::AreEqual(std::string("test file content"), std::string((const char*)&read[0]));
			}
		}
	};
};

#endif
