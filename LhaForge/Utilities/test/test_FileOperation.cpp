#include "stdafx.h"
#ifdef UNIT_TEST
#include "CppUnitTest.h"
#include "Utilities/FileOperation.h"

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
			Assert::IsTrue(std::filesystem::exists(UtilGetTemporaryFileName()));
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
	};
};

#endif
