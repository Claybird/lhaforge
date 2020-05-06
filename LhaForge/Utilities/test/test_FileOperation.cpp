#include "stdafx.h"
#ifdef UNIT_TEST
#include "CppUnitTest.h"
#include "Utilities/FileOperation.h"
#include "Utilities/OSUtil.h"

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
			Assert::IsTrue(UtilDeletePath(path.c_str()));
		}
		TEST_METHOD(test_UtilDeletePath) {
			//delete file
			auto path = UtilGetTemporaryFileName();
			Assert::IsTrue(std::filesystem::exists(path));
			Assert::IsTrue(UtilDeletePath(path.c_str()));
			Assert::IsFalse(std::filesystem::exists(path));
			Assert::IsFalse(UtilDeletePath(path.c_str()));

			//delete directory
			auto dir = UtilGetTempPath() + L"lhaforge_test";
			UtilDeletePath(dir.c_str());
			Assert::IsFalse(std::filesystem::exists(dir));
			std::filesystem::create_directories(dir.c_str());
			for (int i = 0; i < 100; i++) {
				touchFile(dir + Format(L"/a%03d.txt", i));
			}
			std::filesystem::create_directories((dir+L"/testDir").c_str());
			for (int i = 0; i < 100; i++) {
				touchFile(dir + Format(L"/testDir/b%03d.txt", i));
			}
			UtilDeletePath(dir.c_str());
			Assert::IsFalse(std::filesystem::exists(dir.c_str()));
		}
		TEST_METHOD(test_UtilMoveFileToRecycleBin) {
			std::vector<std::wstring> fileList;
			//delete directory
			auto dir = UtilGetTempPath() + L"lhaforge_test";
			UtilDeletePath(dir.c_str());
			Assert::IsFalse(std::filesystem::exists(dir));
			std::filesystem::create_directories(dir.c_str());
			for (int i = 0; i < 3; i++) {
				fileList.push_back(dir + Format(L"/a%03d.txt", i));
				touchFile(dir + Format(L"/a%03d.txt", i));
			}
			Assert::IsTrue(UtilMoveFileToRecycleBin(fileList));
			UtilDeletePath(dir.c_str());
			Assert::IsFalse(std::filesystem::exists(dir.c_str()));
		}
		TEST_METHOD(test_UtilRecursiveEnumFile) {
			//prepare files
			std::vector<std::wstring> fileList;
			auto dir = UtilGetTempPath() + L"lhaforge_test";
			UtilDeletePath(dir.c_str());
			Assert::IsFalse(std::filesystem::exists(dir));
			std::filesystem::create_directories(dir.c_str());
			for (int i = 0; i < 3; i++) {
				fileList.push_back(dir + Format(L"/a%03d.txt", i));
				touchFile(dir + Format(L"/a%03d.txt", i));
			}
			std::filesystem::create_directories((dir + L"/b").c_str());
			for (int i = 0; i < 3; i++) {
				fileList.push_back(dir + Format(L"/b/a%03d.txt", i));
				touchFile(dir + Format(L"/b/a%03d.txt", i));
			}

			//enumerate
			auto enumerated = UtilRecursiveEnumFile(dir.c_str());
			Assert::AreEqual(fileList.size(), enumerated.size());
			for (size_t i = 0; i < fileList.size(); i++) {
				Assert::AreEqual(
					std::filesystem::path(fileList[i]).make_preferred().wstring(),
					std::filesystem::path(enumerated[i]).make_preferred().wstring());
			}

			//cleanup
			UtilDeletePath(dir.c_str());
			Assert::IsFalse(std::filesystem::exists(dir.c_str()));
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
				UtilGetCompletePathName(nullptr);
			});
			Assert::ExpectException<LF_EXCEPTION>([]() {
				UtilGetCompletePathName(L"");
			});
			Assert::IsTrue(UtilPathIsRoot(L"C:\\"));
			Assert::IsTrue(UtilPathIsRoot(UtilGetCompletePathName(L"C:").c_str()));
			Assert::IsTrue(UtilPathIsRoot(UtilGetCompletePathName(L"C:\\").c_str()));
			Assert::IsFalse(UtilPathIsRoot(UtilGetCompletePathName(L"C:\\Windows").c_str()));
			auto tempDir = std::filesystem::temp_directory_path();
			Assert::IsFalse(UtilPathIsRoot(UtilGetCompletePathName(tempDir.c_str()).c_str()));
			{
				CCurrentDirManager mngr(tempDir.c_str());
				auto dest = L"C:\\Windows";
				auto relpath = std::filesystem::relative(dest);
				auto expected = toLower(std::filesystem::path(dest).make_preferred());
				auto actual = toLower(std::filesystem::path(UtilGetCompletePathName(relpath.c_str())));
				Assert::AreEqual(expected, actual);
			}
		}
	};
};

#endif
