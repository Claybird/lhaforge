#include "stdafx.h"
#ifdef UNIT_TEST
#include <gtest/gtest.h>
#include "extract.h"

TEST(extract, trimArchiveName) {
	std::wstring trimArchiveName(bool RemoveSymbolAndNumber, const wchar_t* archive_path);
	EXPECT_EQ(L"", trimArchiveName(true, L""));

	EXPECT_EQ(L"123", trimArchiveName(true, L"123"));	//restore original
	EXPECT_EQ(L"123", trimArchiveName(false, L"123"));

	EXPECT_EQ(L"123abc", trimArchiveName(true, L"123abc456"));
	EXPECT_EQ(L"123abc456", trimArchiveName(false, L"123abc456"));

	EXPECT_EQ(L"123abc", trimArchiveName(true, L"123abc456[1]"));
	EXPECT_EQ(L"123abc456[1]", trimArchiveName(false, L"123abc456[1]"));

	EXPECT_EQ(L"123abc", trimArchiveName(true, L"123abc456."));
	EXPECT_EQ(L"123abc456", trimArchiveName(false, L"123abc456."));

	EXPECT_EQ(L"", trimArchiveName(true, L"123abc456\\"));
	EXPECT_EQ(L"", trimArchiveName(false, L"123abc456\\"));

	EXPECT_EQ(L"123abc", trimArchiveName(true, L"123abc456 "));
	EXPECT_EQ(L"123abc456", trimArchiveName(false, L"123abc456 "));

	//full-width space
	EXPECT_EQ(L"123abc", trimArchiveName(true, L"123abc456　"));
	EXPECT_EQ(L"123abc456", trimArchiveName(false, L"123abc456　"));
}

TEST(extract, determineExtractBaseDir) {
	std::wstring determineExtractBaseDir(const wchar_t* archive_path, LF_EXTRACT_ARGS& args);
	LF_EXTRACT_ARGS fakeArg;
	fakeArg.extract.OutputDirType = OUTPUT_TO::OUTPUT_TO_SPECIFIC_DIR;
	fakeArg.extract.OutputDirUserSpecified = std::filesystem::current_path().c_str();
	fakeArg.general.WarnNetwork = FALSE;
	fakeArg.general.WarnRemovable = FALSE;
	fakeArg.general.OnDirNotFound = LOSTDIR_FORCE_CREATE;

	auto out = determineExtractBaseDir(L"path_to_archive/archive.ext", fakeArg);
	EXPECT_EQ(std::wstring(std::filesystem::current_path().c_str()), out);
}

TEST(extract, determineExtractDir) {
	std::wstring determineExtractDir(const wchar_t* archive_path, const wchar_t* output_base_dir, const LF_EXTRACT_ARGS& args);
	LF_EXTRACT_ARGS fakeArg;
	fakeArg.extract.CreateDir = CREATE_OUTPUT_DIR_NEVER;
	fakeArg.extract.RemoveSymbolAndNumber = false;
	EXPECT_EQ(L"path_to_output", 
		replace(determineExtractDir(L"path_to_archive/archive.ext", L"path_to_output", fakeArg), L"\\", L"/"));
	EXPECT_EQ(L"path_to_output",
		replace(determineExtractDir(L"path_to_archive/archive  .ext", L"path_to_output", fakeArg), L"\\", L"/"));

	fakeArg.extract.CreateDir = CREATE_OUTPUT_DIR_ALWAYS;
	EXPECT_EQ(L"path_to_output/archive",
		replace(determineExtractDir(L"path_to_archive/archive.ext", L"path_to_output", fakeArg), L"\\", L"/"));
	EXPECT_EQ(L"path_to_output/archive",
		replace(determineExtractDir(L"path_to_archive/archive  .ext", L"path_to_output", fakeArg), L"\\", L"/"));
}

TEST(extract, extractOneArchive) {
	_wsetlocale(LC_ALL, L"");	//default locale

	auto tempDir = std::filesystem::path(UtilGetTempPath() + L"test_extractOneArchive");
	UtilDeleteDir(tempDir, true);
	EXPECT_FALSE(std::filesystem::exists(tempDir));
	std::filesystem::create_directories(tempDir);
	auto archiveFile = std::filesystem::path(__FILEW__).parent_path() / L"test_extract.zip";
	ASSERT_TRUE(std::filesystem::exists(archiveFile));

	ARCLOG arcLog;
	EXPECT_NO_THROW(
		extractOneArchive(archiveFile, tempDir, arcLog,
		[&](const std::wstring& fullpath, const LF_ARCHIVE_ENTRY* entry) {return overwrite_options::abort; },
		[&](const std::wstring& originalPath, UINT64 currentSize, UINT64 totalSize) {}
	));

	EXPECT_TRUE(std::filesystem::exists(tempDir / L"dirA"));
	EXPECT_TRUE(std::filesystem::exists(tempDir / L"dirA/dirB"));
	EXPECT_TRUE(std::filesystem::exists(tempDir / L"dirA/dirB/file2.txt"));
	EXPECT_TRUE(std::filesystem::exists(tempDir / L"dirA/dirB/dirC"));
	EXPECT_TRUE(std::filesystem::exists(tempDir / L"dirA/dirB/dirC/file1.txt"));
	EXPECT_TRUE(std::filesystem::exists(tempDir / L"かきくけこ"));
	EXPECT_TRUE(std::filesystem::exists(tempDir / L"かきくけこ/file3.txt"));
	EXPECT_TRUE(std::filesystem::exists(tempDir / L"あいうえお.txt"));

	UtilDeleteDir(tempDir, true);
	EXPECT_FALSE(std::filesystem::exists(tempDir));
}

TEST(extract, extractOneArchive_broken_files) {
	_wsetlocale(LC_ALL, L"");	//default locale

	const std::vector<std::wstring> files = { L"test_broken_file.zip" , L"test_broken_crc.zip" };

	for (const auto& file : files) {
		auto tempDir = std::filesystem::path(UtilGetTempPath() + L"test_extractOneArchive");
		UtilDeleteDir(tempDir, true);
		EXPECT_FALSE(std::filesystem::exists(tempDir));
		std::filesystem::create_directories(tempDir);
		auto archiveFile = std::filesystem::path(__FILEW__).parent_path() / file;
		ASSERT_TRUE(std::filesystem::exists(archiveFile));

		ARCLOG arcLog;
		EXPECT_THROW(
			extractOneArchive(archiveFile, tempDir, arcLog,
				[&](const std::wstring& fullpath, const LF_ARCHIVE_ENTRY* entry) {return overwrite_options::abort; },
				[&](const std::wstring& originalPath, UINT64 currentSize, UINT64 totalSize) {}
		), LF_EXCEPTION);

		UtilDeleteDir(tempDir, true);
		EXPECT_FALSE(std::filesystem::exists(tempDir));
	}
}

TEST(extract, testOneArchive) {
	_wsetlocale(LC_ALL, L"");	//default locale

	auto archiveFile = std::filesystem::path(__FILEW__).parent_path() / L"test_extract.zip";
	ASSERT_TRUE(std::filesystem::exists(archiveFile));

	ARCLOG arcLog;
	EXPECT_NO_THROW(
	testOneArchive(archiveFile, arcLog,
		[&](const std::wstring& originalPath, UINT64 currentSize, UINT64 totalSize) {}
	));
}

TEST(extract, testOneArchive_broken_files) {
	_wsetlocale(LC_ALL, L"");	//default locale

	const std::vector<std::wstring> files = { L"test_broken_file.zip" , L"test_broken_crc.zip" };

	for (const auto& file : files) {
		auto archiveFile = std::filesystem::path(__FILEW__).parent_path() / file;
		ASSERT_TRUE(std::filesystem::exists(archiveFile));

		ARCLOG arcLog;
		EXPECT_THROW(
			testOneArchive(archiveFile, arcLog,
				[&](const std::wstring& originalPath, UINT64 currentSize, UINT64 totalSize) {}
		), LF_EXCEPTION);
	}
}

#endif

