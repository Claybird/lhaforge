#include "stdafx.h"
#ifdef UNIT_TEST
#include <gtest/gtest.h>
#include "Utilities/FileOperation.h"
#include "Utilities/OSUtil.h"
#include "Utilities/Utility.h"

TEST(FileOperation, UtilGetTempPath) {
	EXPECT_TRUE(std::filesystem::exists(UtilGetTempPath()));
}
TEST(FileOperation, UtilGetTemporaryFileName) {
	auto path = UtilGetTemporaryFileName();
	EXPECT_TRUE(std::filesystem::exists(path));
	EXPECT_TRUE(UtilDeletePath(path));
}
TEST(FileOperation, UtilDeletePath) {
	//delete file
	auto path = UtilGetTemporaryFileName();
	EXPECT_TRUE(std::filesystem::exists(path));
	EXPECT_TRUE(UtilDeletePath(path));
	EXPECT_FALSE(std::filesystem::exists(path));
	EXPECT_FALSE(UtilDeletePath(path));

	//delete directory
	auto dir = UtilGetTempPath() + L"lhaforge_test";
	UtilDeletePath(dir);
	EXPECT_FALSE(std::filesystem::exists(dir));
	std::filesystem::create_directories(dir);
	for (int i = 0; i < 100; i++) {
		touchFile(dir + Format(L"/a%03d.txt", i));
	}
	std::filesystem::create_directories(dir + L"/testDir");
	for (int i = 0; i < 100; i++) {
		touchFile(dir + Format(L"/testDir/b%03d.txt", i));
	}
	UtilDeletePath(dir);
	EXPECT_FALSE(std::filesystem::exists(dir));
}
TEST(FileOperation, UtilDeleteDir) {
	//delete file
	auto path = UtilGetTempPath() + L"test_UtilDeleteDir";
	EXPECT_FALSE(std::filesystem::exists(path));
	std::filesystem::create_directories(path);
	EXPECT_TRUE(std::filesystem::exists(path));

	touchFile(path + L"/test.txt");
	EXPECT_TRUE(UtilDeleteDir(path, true));
	EXPECT_FALSE(std::filesystem::exists(path));
}
TEST(FileOperation, CTemporaryDirectoryManager) {
	std::wstring path;
	{
		CTemporaryDirectoryManager tmpMngr;
		path = tmpMngr.path();
		EXPECT_TRUE(std::filesystem::exists(path));
	}
	EXPECT_FALSE(std::filesystem::exists(path));
}
TEST(FileOperation, UtilMoveFileToRecycleBin) {
	std::vector<std::wstring> fileList;
	//delete directory
	auto dir = UtilGetTempPath() + L"lhaforge_test";
	UtilDeletePath(dir);
	EXPECT_FALSE(std::filesystem::exists(dir));
	std::filesystem::create_directories(dir);
	for (int i = 0; i < 3; i++) {
		fileList.push_back(dir + Format(L"/a%03d.txt", i));
		touchFile(dir + Format(L"/a%03d.txt", i));
	}
	EXPECT_TRUE(UtilMoveFileToRecycleBin(fileList));
	UtilDeletePath(dir);
	EXPECT_FALSE(std::filesystem::exists(dir));
}
TEST(FileOperation, UtilRecursiveEnumFile_UtilPathExpandWild) {
	//prepare files
	std::vector<std::wstring> fileList;
	auto dir = UtilGetTempPath() + L"lhaforge_test";
	UtilDeletePath(dir);
	EXPECT_FALSE(std::filesystem::exists(dir));
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
		EXPECT_EQ(fileList.size(), enumerated.size());
		for (size_t i = 0; i < fileList.size(); i++) {
			EXPECT_EQ(
				std::filesystem::path(fileList[i]).make_preferred().wstring(),
				std::filesystem::path(enumerated[i]).make_preferred().wstring());
		}
	}

	//expand wild
	{
		auto enumerated = UtilPathExpandWild(dir);
		EXPECT_EQ(size_t(1), enumerated.size());
		EXPECT_EQ(enumerated[0], dir);

		auto pdir = std::filesystem::path(dir);
		enumerated = UtilPathExpandWild(pdir / L"*.txt");
		EXPECT_EQ(size_t(3), enumerated.size());
		EXPECT_TRUE(isIn(enumerated, pdir / L"a000.txt"));
		EXPECT_TRUE(isIn(enumerated, pdir / L"a001.txt"));
		EXPECT_TRUE(isIn(enumerated, pdir / L"a002.txt"));

		enumerated = UtilPathExpandWild(pdir / L"*.exe");
		EXPECT_TRUE(enumerated.empty());
	}

	//cleanup
	UtilDeletePath(dir);
	EXPECT_FALSE(std::filesystem::exists(dir));
}
TEST(FileOperation, UtilPathIsRoot) {
	EXPECT_TRUE(UtilPathIsRoot(L"c:/"));
	EXPECT_TRUE(UtilPathIsRoot(L"c:\\"));
	EXPECT_TRUE(UtilPathIsRoot(L"c:"));
	EXPECT_FALSE(UtilPathIsRoot(L"c:/windows/"));
	EXPECT_FALSE(UtilPathIsRoot(L"c:\\windows\\"));
}
TEST(FileOperation, UtilPathAddLastSeparator) {
	std::wstring sep;
	sep = std::filesystem::path::preferred_separator;
	EXPECT_EQ(sep, UtilPathAddLastSeparator(L""));
	EXPECT_EQ(L"C:" + sep, UtilPathAddLastSeparator(L"C:"));
	EXPECT_EQ(L"C:\\", UtilPathAddLastSeparator(L"C:\\"));
	EXPECT_EQ(L"C:/", UtilPathAddLastSeparator(L"C:/"));
	EXPECT_EQ(std::wstring(L"/tmp") + sep, UtilPathAddLastSeparator(L"/tmp"));
	EXPECT_EQ(L"/tmp\\", UtilPathAddLastSeparator(L"/tmp\\"));
	EXPECT_EQ(L"/tmp/", UtilPathAddLastSeparator(L"/tmp/"));
}
TEST(FileOperation, UtilPathRemoveLastSeparator) {
	EXPECT_EQ(L"", UtilPathRemoveLastSeparator(L""));
	EXPECT_EQ(L"", UtilPathRemoveLastSeparator(L"/"));
	EXPECT_EQ(L"", UtilPathRemoveLastSeparator(L"\\"));
	EXPECT_EQ(L"C:", UtilPathRemoveLastSeparator(L"C:/"));
	EXPECT_EQ(L"C:", UtilPathRemoveLastSeparator(L"C:\\"));
	EXPECT_EQ(L"C:", UtilPathRemoveLastSeparator(L"C:"));
	EXPECT_EQ(L"/tmp", UtilPathRemoveLastSeparator(L"/tmp\\"));
	EXPECT_EQ(L"/tmp", UtilPathRemoveLastSeparator(L"/tmp/"));
	EXPECT_EQ(L"/tmp", UtilPathRemoveLastSeparator(L"/tmp"));
}
TEST(FileOperation, UtilGetCompletePathName) {
	EXPECT_THROW(UtilGetCompletePathName(L""), LF_EXCEPTION);
	EXPECT_TRUE(UtilPathIsRoot(L"C:\\"));
	EXPECT_TRUE(UtilPathIsRoot(UtilGetCompletePathName(L"C:")));
	EXPECT_TRUE(UtilPathIsRoot(UtilGetCompletePathName(L"C:\\")));
	EXPECT_FALSE(UtilPathIsRoot(UtilGetCompletePathName(L"C:\\Windows")));
	auto tempDir = std::filesystem::temp_directory_path();
	EXPECT_FALSE(UtilPathIsRoot(UtilGetCompletePathName(tempDir)));
	{
		CCurrentDirManager mngr(tempDir);
		auto dest = L"C:\\Windows";
		auto relpath = std::filesystem::relative(dest);
		auto expected = toLower(std::filesystem::path(dest).make_preferred());
		auto actual = toLower(std::filesystem::path(UtilGetCompletePathName(relpath)));
		EXPECT_EQ(expected, actual);
	}
}
TEST(FileOperation, UtilGetModulePath_UtilGetModuleDirectoryPath) {
	//TODO: is there any better test?
	EXPECT_FALSE(UtilGetModulePath().empty());
	EXPECT_TRUE(std::filesystem::exists(UtilGetModulePath()));
	EXPECT_TRUE(std::filesystem::exists(UtilGetModuleDirectoryPath()));
}
TEST(FileOperation, UtilReadFile) {
	//prepare
	auto fname = UtilGetTempPath() + L"lhaforge_test_file.tmp";
	{
		CAutoFile fp;
		fp.open(fname, L"w");
		fprintf(fp, "test file content");
	}
	{
		auto read = UtilReadFile(fname);
		EXPECT_EQ(size_t(17), read.size());
		read.push_back('\0');
		EXPECT_EQ("test file content", std::string((const char*)&read[0]));
	}
}

#endif
