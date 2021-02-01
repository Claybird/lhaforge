#include "stdafx.h"
#ifdef UNIT_TEST
#include <gtest/gtest.h>
#include "compress.h"
#include "Utilities/FileOperation.h"
#include "extract.h"	//for extract test

TEST(compress, getSourcesBasePath)
{
	std::wstring getSourcesBasePath(const std::vector<std::wstring> &sources);

	std::filesystem::path dir = UtilGetTempPath() + L"lhaforge_test/getSourcesBasePath";
	UtilDeletePath(dir);
	EXPECT_FALSE(std::filesystem::exists(dir));
	std::filesystem::create_directories(dir / L"abc");
	std::filesystem::create_directories(dir / L"ghi");

	{
		EXPECT_EQ(L"", getSourcesBasePath(std::vector<std::wstring>({ })));
		EXPECT_EQ(dir / L"abc", getSourcesBasePath(std::vector<std::wstring>({ dir / L"abc/" })));
		EXPECT_EQ(dir / L"abc", getSourcesBasePath(std::vector<std::wstring>({ dir / L"abc/",dir / L"ABC/ghi/" })));
		EXPECT_EQ(dir, getSourcesBasePath(std::vector<std::wstring>({ dir / L"abc",dir / L"ghi/" })));
		EXPECT_EQ(dir / L"abc", getSourcesBasePath(std::vector<std::wstring>({ dir / L"abc",dir / L"abc/" })));
		EXPECT_EQ(std::filesystem::path(L"c:/windows"), getSourcesBasePath(std::vector<std::wstring>({ L"c:/windows",L"c:/windows/system32" })));
	}

	UtilDeletePath(dir);
	EXPECT_FALSE(std::filesystem::exists(dir));
}


#pragma comment(lib,"Bcrypt.lib")
TEST(compress, RAW_FILE_READER)
{
	auto fileToRead = std::filesystem::path(__FILEW__).parent_path() / L"test_raw_file_reader.txt";
	std::string expected_hash = "dc2545110ea53ef9ce169fd676cf9f24a966e6571be630d221eae8b8bb7717a5";

	RAW_FILE_READER reader;
	reader.open(fileToRead);
	std::vector<char> buf;
	for (;;) {
		auto bi = reader();
		if (bi.is_eof())break;
		buf.insert(buf.end(), (const char*)(bi.buffer), (const char*)(bi.buffer) + bi.size);
	}

	//sha256
	//https://docs.microsoft.com/en-us/windows/win32/seccng/creating-a-hash-with-cng
	std::string hash;
	{
		BCRYPT_ALG_HANDLE hAlg = nullptr;

#define NT_SUCCESS(Status)          (((NTSTATUS)(Status)) >= 0)

		//open an algorithm handle
		if (NT_SUCCESS(BCryptOpenAlgorithmProvider(&hAlg, BCRYPT_SHA256_ALGORITHM, nullptr, 0))) {
			//calculate the size of the buffer to hold the hash object
			DWORD cbData = 0, cbHashObject = 0;
			if (NT_SUCCESS(BCryptGetProperty(hAlg, BCRYPT_OBJECT_LENGTH, (PBYTE)&cbHashObject, sizeof(DWORD), &cbData, 0))) {
				//allocate the hash object on the heap
				std::vector<BYTE> bHashObject;
				bHashObject.resize(cbHashObject);
				//calculate the length of the hash
				DWORD cbHash = 0;
				if (NT_SUCCESS(BCryptGetProperty(hAlg, BCRYPT_HASH_LENGTH, (PBYTE)&cbHash, sizeof(DWORD), &cbData, 0))) {
					//allocate the hash buffer on the heap
					std::vector<BYTE> bHash;
					bHash.resize(cbHash);
					//create a hash
					BCRYPT_HASH_HANDLE hHash = nullptr;
					if (NT_SUCCESS(BCryptCreateHash(hAlg, &hHash, &bHashObject[0], cbHashObject, nullptr, 0, 0))) {
						//hash some data
						if (NT_SUCCESS(BCryptHashData(hHash, (PBYTE)&buf[0], buf.size(), 0))) {
							//close the hash
							if (NT_SUCCESS(BCryptFinishHash(hHash, &bHash[0], cbHash, 0))) {
								for (auto c : bHash) {
									char strbuf[8] = {};
									sprintf_s(strbuf, "%02x", c);
									hash += strbuf;
								}
							}
						}
					}
					if (hHash) {
						BCryptDestroyHash(hHash);
					}
				}
			}
		}
		if (hAlg) {
			BCryptCloseAlgorithmProvider(hAlg, 0);
		}
	}


	EXPECT_EQ(expected_hash, hash);
}

TEST(compress, isAllowedCombination)
{
	bool isAllowedCombination(LF_ARCHIVE_FORMAT fmt, int option);

	EXPECT_TRUE(isAllowedCombination(LF_FMT_ZIP, LF_WOPT_STANDARD));
	EXPECT_TRUE(isAllowedCombination(LF_FMT_ZIP, LF_WOPT_SFX));
	EXPECT_TRUE(isAllowedCombination(LF_FMT_ZIP, LF_WOPT_DATA_ENCRYPTION));
	EXPECT_TRUE(isAllowedCombination(LF_FMT_ZIP, LF_WOPT_SFX | LF_WOPT_DATA_ENCRYPTION));

	EXPECT_TRUE(isAllowedCombination(LF_FMT_7Z, LF_WOPT_STANDARD));
	EXPECT_TRUE(isAllowedCombination(LF_FMT_7Z, LF_WOPT_SFX));
	//EXPECT_TRUE(isAllowedCombination(LF_FMT_7Z, LF_WOPT_DATA_ENCRYPTION));
	//EXPECT_TRUE(isAllowedCombination(LF_FMT_7Z, LF_WOPT_HEADER_ENCRYPTION));
	//EXPECT_TRUE(isAllowedCombination(LF_FMT_7Z, LF_WOPT_DATA_ENCRYPTION | LF_WOPT_HEADER_ENCRYPTION));
	//EXPECT_TRUE(isAllowedCombination(LF_FMT_7Z, LF_WOPT_SFX | LF_WOPT_DATA_ENCRYPTION));

	EXPECT_TRUE(isAllowedCombination(LF_FMT_GZ, LF_WOPT_STANDARD));
	EXPECT_TRUE(isAllowedCombination(LF_FMT_BZ2, LF_WOPT_STANDARD));
	EXPECT_TRUE(isAllowedCombination(LF_FMT_LZMA, LF_WOPT_STANDARD));
	EXPECT_TRUE(isAllowedCombination(LF_FMT_XZ, LF_WOPT_STANDARD));
	EXPECT_TRUE(isAllowedCombination(LF_FMT_ZSTD, LF_WOPT_STANDARD));
	EXPECT_TRUE(isAllowedCombination(LF_FMT_TAR, LF_WOPT_STANDARD));
	EXPECT_TRUE(isAllowedCombination(LF_FMT_TAR_GZ, LF_WOPT_STANDARD));
	EXPECT_TRUE(isAllowedCombination(LF_FMT_TAR_BZ2, LF_WOPT_STANDARD));
	EXPECT_TRUE(isAllowedCombination(LF_FMT_TAR_LZMA, LF_WOPT_STANDARD));
	EXPECT_TRUE(isAllowedCombination(LF_FMT_TAR_XZ, LF_WOPT_STANDARD));
	EXPECT_TRUE(isAllowedCombination(LF_FMT_TAR_ZSTD, LF_WOPT_STANDARD));
	EXPECT_TRUE(isAllowedCombination(LF_FMT_UUE, LF_WOPT_STANDARD));

	//---following are decompress only
	EXPECT_FALSE(isAllowedCombination(LF_FMT_LZH, LF_WOPT_STANDARD));
	EXPECT_FALSE(isAllowedCombination(LF_FMT_CAB, LF_WOPT_STANDARD));
	EXPECT_FALSE(isAllowedCombination(LF_FMT_RAR, LF_WOPT_STANDARD));
	EXPECT_FALSE(isAllowedCombination(LF_FMT_ISO, LF_WOPT_STANDARD));
	EXPECT_FALSE(isAllowedCombination(LF_FMT_AR, LF_WOPT_STANDARD));
	EXPECT_FALSE(isAllowedCombination(LF_FMT_XAR, LF_WOPT_STANDARD));
	EXPECT_FALSE(isAllowedCombination(LF_FMT_WARC, LF_WOPT_STANDARD));
	EXPECT_FALSE(isAllowedCombination(LF_FMT_ACE, LF_WOPT_STANDARD));
	EXPECT_FALSE(isAllowedCombination(LF_FMT_JAK, LF_WOPT_STANDARD));
	EXPECT_FALSE(isAllowedCombination(LF_FMT_BZA, LF_WOPT_STANDARD));
	EXPECT_FALSE(isAllowedCombination(LF_FMT_GZA, LF_WOPT_STANDARD));
	EXPECT_FALSE(isAllowedCombination(LF_FMT_ISH, LF_WOPT_STANDARD));
	EXPECT_FALSE(isAllowedCombination(LF_FMT_CPIO, LF_WOPT_STANDARD));
}

TEST(compress, getArchiveFileExtension)
{
	std::wstring getArchiveFileExtension(LF_ARCHIVE_FORMAT fmt, LF_WRITE_OPTIONS option, const std::wstring& original_path);
	const wchar_t* path = L"abc.ext";
	EXPECT_EQ(L".zip", getArchiveFileExtension(LF_FMT_ZIP, LF_WOPT_STANDARD, path));
	EXPECT_EQ(L".7z", getArchiveFileExtension(LF_FMT_7Z, LF_WOPT_STANDARD, path));
	EXPECT_EQ(L".ext.gz", getArchiveFileExtension(LF_FMT_GZ, LF_WOPT_STANDARD, path));
	EXPECT_EQ(L".ext.bz2", getArchiveFileExtension(LF_FMT_BZ2, LF_WOPT_STANDARD, path));
	EXPECT_EQ(L".ext.lzma", getArchiveFileExtension(LF_FMT_LZMA, LF_WOPT_STANDARD, path));
	EXPECT_EQ(L".ext.xz", getArchiveFileExtension(LF_FMT_XZ, LF_WOPT_STANDARD, path));
	EXPECT_EQ(L".ext.zst", getArchiveFileExtension(LF_FMT_ZSTD, LF_WOPT_STANDARD, path));
	EXPECT_EQ(L".tar", getArchiveFileExtension(LF_FMT_TAR, LF_WOPT_STANDARD, path));
	EXPECT_EQ(L".tar.gz", getArchiveFileExtension(LF_FMT_TAR_GZ, LF_WOPT_STANDARD, path));
	EXPECT_EQ(L".tar.bz2", getArchiveFileExtension(LF_FMT_TAR_BZ2, LF_WOPT_STANDARD, path));
	EXPECT_EQ(L".tar.lzma", getArchiveFileExtension(LF_FMT_TAR_LZMA, LF_WOPT_STANDARD, path));
	EXPECT_EQ(L".tar.xz", getArchiveFileExtension(LF_FMT_TAR_XZ, LF_WOPT_STANDARD, path));
	EXPECT_EQ(L".tar.zst", getArchiveFileExtension(LF_FMT_TAR_ZSTD, LF_WOPT_STANDARD, path));
	EXPECT_EQ(L".uue", getArchiveFileExtension(LF_FMT_UUE, LF_WOPT_STANDARD, path));


	EXPECT_EQ(L".exe", getArchiveFileExtension(LF_FMT_ZIP, LF_WOPT_SFX, path));
	EXPECT_EQ(L".exe", getArchiveFileExtension(LF_FMT_7Z, LF_WOPT_SFX, path));
	/*EXPECT_EQ(L".ext.exe", getArchiveFileExtension(LF_FMT_GZ, LF_WOPT_SFX, path));
	EXPECT_EQ(L".ext.exe", getArchiveFileExtension(LF_FMT_BZ2, LF_WOPT_SFX, path));
	EXPECT_EQ(L".ext.exe", getArchiveFileExtension(LF_FMT_LZMA, LF_WOPT_SFX, path));
	EXPECT_EQ(L".ext.exe", getArchiveFileExtension(LF_FMT_XZ, LF_WOPT_SFX, path));
	EXPECT_EQ(L".ext.exe", getArchiveFileExtension(LF_FMT_Z, LF_WOPT_SFX, path));
	EXPECT_EQ(L".exe", getArchiveFileExtension(LF_FMT_TAR, LF_WOPT_SFX, path));
	EXPECT_EQ(L".exe", getArchiveFileExtension(LF_FMT_TAR_GZ, LF_WOPT_SFX, path));
	EXPECT_EQ(L".exe", getArchiveFileExtension(LF_FMT_TAR_BZ2, LF_WOPT_SFX, path));
	EXPECT_EQ(L".exe", getArchiveFileExtension(LF_FMT_TAR_LZMA, LF_WOPT_SFX, path));
	EXPECT_EQ(L".exe", getArchiveFileExtension(LF_FMT_TAR_XZ, LF_WOPT_SFX, path));
	EXPECT_EQ(L".exe", getArchiveFileExtension(LF_FMT_TAR_Z, LF_WOPT_SFX, path));
	EXPECT_EQ(L".exe", getArchiveFileExtension(LF_FMT_UUE, LF_WOPT_SFX, path));*/
}

TEST(compress, volumeLabelToDirectoryName)
{
	std::wstring volumeLabelToDirectoryName(const std::wstring& volume_label);
	EXPECT_EQ(L"a_b_c_d_e_f_g_h_i_j", volumeLabelToDirectoryName(L"a/b\\c:d*e?f\"g<h>i|j"));
}

TEST(compress, determineDefaultArchiveTitle)
{
	std::wstring determineDefaultArchiveTitle(
		LF_ARCHIVE_FORMAT format,
		LF_WRITE_OPTIONS option,
		const std::wstring &input_path
	);
	EXPECT_EQ(L"source.zip", determineDefaultArchiveTitle(LF_FMT_ZIP, LF_WOPT_STANDARD, L"/path/to/source.txt"));
	EXPECT_EQ(L"source.txt.gz", determineDefaultArchiveTitle(LF_FMT_GZ, LF_WOPT_STANDARD, L"/path/to/source.txt"));
}

TEST(compress, getRelativePathList)
{
	std::vector<COMPRESS_SOURCES::PATH_PAIR> getRelativePathList(
		const std::wstring& basePath,
		const std::vector<std::wstring>& sourcePathList);
	auto result = getRelativePathList(L"/path/to/base/",
		{ L"/path/to/base/", L"/path/to/base/file1.txt", L"/path/to/base/dir1/file2.txt", });
	EXPECT_EQ(2, result.size());
	if (result.size() >= 2) {
		EXPECT_EQ(L"file1.txt", result[0].entryPath);
		EXPECT_EQ(L"/path/to/base/file1.txt", result[0].originalFullPath);
		EXPECT_EQ(std::filesystem::path(L"dir1/file2.txt").make_preferred(), result[1].entryPath);
		EXPECT_EQ(L"/path/to/base/dir1/file2.txt", result[1].originalFullPath);
	}
}

TEST(compress, getAllSourceFiles)
{
	std::vector<std::wstring> getAllSourceFiles(const std::vector<std::wstring> &sourcePathList);

	//delete directory
	std::filesystem::path dir = UtilGetTempPath() + L"lhaforge_test/getAllSourceFiles";
	UtilDeletePath(dir);
	EXPECT_FALSE(std::filesystem::exists(dir));
	std::filesystem::create_directories(dir);
	std::filesystem::create_directories(dir / L"a");
	std::filesystem::create_directories(dir / L"b/c");
	for (int i = 0; i < 3; i++) {
		touchFile(dir / Format(L"a/a%03d.txt", i));
		touchFile(dir / Format(L"b/c/b%03d.txt", i));
	}

	auto files = getAllSourceFiles({ dir });
	EXPECT_EQ(9, files.size());		//a,a[000-002].txt,b,b/c,b/c/b[000-002].txt

	files = getAllSourceFiles({ dir / L"a" });
	EXPECT_EQ(4, files.size());		//a,a[000-002].txt
	files = getAllSourceFiles({ dir / L"b" });
	EXPECT_EQ(5, files.size());		//b,b/c,b/c/b[000-002].txt
	files = getAllSourceFiles({ dir / L"c" });
	EXPECT_EQ(1, files.size());		//dir nor file does not exist, but listed
	files = getAllSourceFiles({ dir / L"b/c" });
	EXPECT_EQ(4, files.size());		//b/c,b/c/b[000-002].txt

	UtilDeletePath(dir);
	EXPECT_FALSE(std::filesystem::exists(dir));
}

TEST(compress, buildCompressSources_confirmOutputFile)
{
	COMPRESS_SOURCES buildCompressSources(
		const LF_COMPRESS_ARGS &args,
		const std::vector<std::wstring> &givenFiles
	);

	//delete directory
	std::filesystem::path dir = UtilGetTempPath() + L"lhaforge_test/compressSources";
	UtilDeletePath(dir);
	//subject files
	EXPECT_FALSE(std::filesystem::exists(dir));
	std::filesystem::create_directories(dir);
	std::filesystem::create_directories(dir / L"a");
	std::filesystem::create_directories(dir / L"b/c");
	for (int i = 0; i < 3; i++) {
		touchFile(dir / Format(L"a/a%03d.txt", i));
		touchFile(dir / Format(L"b/c/b%03d.txt", i));
	}
	{
		CAutoFile fp;
		fp.open(dir / L"a/test.txt", L"w");
		EXPECT_TRUE(fp.is_opened());
		fprintf(fp, "abcde");
	}

	{
		std::vector<std::wstring> givenFiles;
		givenFiles.push_back(dir / L"a");
		givenFiles.push_back(dir / L"b");

		LF_COMPRESS_ARGS fake_args;
		fake_args.compress.IgnoreTopDirectory = false;
		fake_args.compress.IgnoreTopDirectoryRecursively = false;
		auto sources = buildCompressSources(fake_args, givenFiles);

		EXPECT_EQ(dir, sources.basePath);
		EXPECT_EQ(5, sources.total_filesize);
		EXPECT_EQ(10, sources.pathPair.size());

		std::map<std::wstring, std::wstring> expected = {
			{(dir / L"a").make_preferred(),L"a"},
			{(dir / L"a/a000.txt").make_preferred(),L"a/a000.txt"},
			{(dir / L"a/a001.txt").make_preferred(),L"a/a001.txt"},
			{(dir / L"a/a002.txt").make_preferred(),L"a/a002.txt"},
			{(dir / L"a/test.txt").make_preferred(),L"a/test.txt"},
			{(dir / L"b").make_preferred(),L"b"},
			{(dir / L"b/c").make_preferred(),L"b/c"},
			{(dir / L"b/c/b000.txt").make_preferred(),L"b/c/b000.txt"},
			{(dir / L"b/c/b001.txt").make_preferred(),L"b/c/b001.txt"},
			{(dir / L"b/c/b002.txt").make_preferred(),L"b/c/b002.txt"},
		};

		for (auto pair : sources.pathPair) {
			auto orgPath = std::filesystem::path(pair.originalFullPath).make_preferred();
			EXPECT_TRUE(has_key(expected, orgPath));
			EXPECT_EQ(expected[orgPath], pair.entryPath);
		}


		//---
		std::wstring confirmOutputFile(
			const std::wstring &default_archive_path,
			const COMPRESS_SOURCES &original_source_list,
			const std::wstring& ext,	//with '.'
			bool bInputFilenameFirst);	//Compress.SpecifyOutputFilename;
		auto output_path = confirmOutputFile(dir / L"test.archive", sources, L".archive", false);
		EXPECT_EQ(dir / L"test.archive", output_path);
	}
	{
		std::vector<std::wstring> givenFiles;
		givenFiles.push_back(dir / L"a");
		givenFiles.push_back(dir / L"b");

		LF_COMPRESS_ARGS fake_args;
		fake_args.compress.IgnoreTopDirectory = true;
		fake_args.compress.IgnoreTopDirectoryRecursively = true;
		auto sources = buildCompressSources(fake_args, givenFiles);

		EXPECT_EQ(dir, sources.basePath);
		EXPECT_EQ(5, sources.total_filesize);
		EXPECT_EQ(10, sources.pathPair.size());

		std::map<std::wstring, std::wstring> expected = {
			{(dir / L"a").make_preferred(),L"a"},
			{(dir / L"a/a000.txt").make_preferred(),L"a/a000.txt"},
			{(dir / L"a/a001.txt").make_preferred(),L"a/a001.txt"},
			{(dir / L"a/a002.txt").make_preferred(),L"a/a002.txt"},
			{(dir / L"a/test.txt").make_preferred(),L"a/test.txt"},
			{(dir / L"b").make_preferred(),L"b"},
			{(dir / L"b/c").make_preferred(),L"b/c"},
			{(dir / L"b/c/b000.txt").make_preferred(),L"b/c/b000.txt"},
			{(dir / L"b/c/b001.txt").make_preferred(),L"b/c/b001.txt"},
			{(dir / L"b/c/b002.txt").make_preferred(),L"b/c/b002.txt"},
		};

		for (auto pair : sources.pathPair) {
			auto orgPath = std::filesystem::path(pair.originalFullPath).make_preferred();
			EXPECT_TRUE(has_key(expected, orgPath));
			EXPECT_EQ(expected[orgPath], pair.entryPath);
		}
	}
	{
		std::vector<std::wstring> givenFiles;
		givenFiles.push_back(dir / L"a");

		LF_COMPRESS_ARGS fake_args;
		fake_args.compress.IgnoreTopDirectory = false;
		fake_args.compress.IgnoreTopDirectoryRecursively = false;
		auto sources = buildCompressSources(fake_args, givenFiles);

		EXPECT_EQ(dir, sources.basePath);
		EXPECT_EQ(5, sources.total_filesize);
		EXPECT_EQ(5, sources.pathPair.size());

		std::map<std::wstring, std::wstring> expected = {
			{(dir / L"a").make_preferred(),L"a"},
			{(dir / L"a/a000.txt").make_preferred(),L"a/a000.txt"},
			{(dir / L"a/a001.txt").make_preferred(),L"a/a001.txt"},
			{(dir / L"a/a002.txt").make_preferred(),L"a/a002.txt"},
			{(dir / L"a/test.txt").make_preferred(),L"a/test.txt"},
		};

		for (auto pair : sources.pathPair) {
			auto orgPath = std::filesystem::path(pair.originalFullPath).make_preferred();
			EXPECT_TRUE(has_key(expected, orgPath));
			EXPECT_EQ(expected[orgPath], pair.entryPath);
		}
	}
	{
		std::vector<std::wstring> givenFiles;
		givenFiles.push_back(dir / L"b");

		LF_COMPRESS_ARGS fake_args;
		fake_args.compress.IgnoreTopDirectory = true;
		fake_args.compress.IgnoreTopDirectoryRecursively = false;
		auto sources = buildCompressSources(fake_args, givenFiles);

		EXPECT_EQ(dir / L"b", sources.basePath);
		EXPECT_EQ(0, sources.total_filesize);
		EXPECT_EQ(4, sources.pathPair.size());

		std::map<std::wstring, std::wstring> expected = {
			{(dir / L"b/c").make_preferred(),L"c"},
			{(dir / L"b/c/b000.txt").make_preferred(),L"c/b000.txt"},
			{(dir / L"b/c/b001.txt").make_preferred(),L"c/b001.txt"},
			{(dir / L"b/c/b002.txt").make_preferred(),L"c/b002.txt"},
		};

		for (auto pair : sources.pathPair) {
			auto orgPath = std::filesystem::path(pair.originalFullPath).make_preferred();
			EXPECT_TRUE(has_key(expected, orgPath));
			EXPECT_EQ(expected[orgPath], pair.entryPath);
		}
	}
	//---
	{
		std::vector<std::wstring> givenFiles;
		givenFiles.push_back(dir / L"b");

		LF_COMPRESS_ARGS fake_args;
		fake_args.compress.IgnoreTopDirectory = true;
		fake_args.compress.IgnoreTopDirectoryRecursively = true;
		auto sources = buildCompressSources(fake_args, givenFiles);

		EXPECT_EQ(dir / L"b/c", sources.basePath);
		EXPECT_EQ(0, sources.total_filesize);
		EXPECT_EQ(3, sources.pathPair.size());

		std::map<std::wstring, std::wstring> expected = {
			{(dir / L"b/c/b000.txt").make_preferred(),L"b000.txt"},
			{(dir / L"b/c/b001.txt").make_preferred(),L"b001.txt"},
			{(dir / L"b/c/b002.txt").make_preferred(),L"b002.txt"},
		};

		for (auto pair : sources.pathPair) {
			auto orgPath = std::filesystem::path(pair.originalFullPath).make_preferred();
			EXPECT_TRUE(has_key(expected, orgPath));
			EXPECT_EQ(expected[orgPath], pair.entryPath);
		}
	}

	UtilDeletePath(dir);
	EXPECT_FALSE(std::filesystem::exists(dir));
}

TEST(compress, determineDefaultArchiveDir)
{
	std::wstring determineDefaultArchiveDir(
		OUTPUT_TO outputDirType,
		const std::wstring& original_file_path,
		const wchar_t* user_specified_dirpath
	);

	auto temp = std::filesystem::path(UtilGetTempPath());
	EXPECT_EQ(UtilGetDesktopPath(), determineDefaultArchiveDir(OUTPUT_TO_DESKTOP, temp, L"C:/path_to_dir"));
	EXPECT_EQ(temp.parent_path(), determineDefaultArchiveDir(OUTPUT_TO_SAME_DIR, temp, L"C:/path_to_dir"));
	EXPECT_EQ(L"C:/path_to_dir", determineDefaultArchiveDir(OUTPUT_TO_SPECIFIC_DIR, temp, L"C:/path_to_dir"));
	EXPECT_EQ(temp.parent_path(), determineDefaultArchiveDir(OUTPUT_TO_ALWAYS_ASK_WHERE, temp, L"C:/path_to_dir"));
	EXPECT_EQ(temp.parent_path(), determineDefaultArchiveDir(OUTPUT_TO_ALWAYS_ASK_WHERE, temp, L"C:/path_to_dir"));
}

std::map<std::string, std::string> getLAOptionsFromConfig(
	LF_COMPRESS_ARGS &args,
	LF_ARCHIVE_FORMAT format,
	LF_WRITE_OPTIONS options);

TEST(compress, getLAOptionsFromConfig)
{
	LF_COMPRESS_ARGS fake_args;
	{
		auto la_options = getLAOptionsFromConfig(fake_args, LF_FMT_ZIP, LF_WOPT_STANDARD);
		EXPECT_EQ(4, la_options.size());
		EXPECT_EQ("deflate", la_options.at("compression"));
		EXPECT_EQ("9", la_options.at("compression-level"));
		//EXPECT_EQ("ZipCrypt", la_options.at("encryption"));
		EXPECT_EQ("UTF-8", la_options.at("hdrcharset"));
		EXPECT_EQ("enabled", la_options.at("zip64"));
	}
	{
		auto la_options = getLAOptionsFromConfig(fake_args, LF_FMT_ZIP, LF_WOPT_DATA_ENCRYPTION);
		EXPECT_EQ(5, la_options.size());
		EXPECT_EQ("deflate", la_options.at("compression"));
		EXPECT_EQ("9", la_options.at("compression-level"));
		EXPECT_EQ("ZipCrypt", la_options.at("encryption"));
		EXPECT_EQ("UTF-8", la_options.at("hdrcharset"));
		EXPECT_EQ("enabled", la_options.at("zip64"));
	}
	{
		auto la_options = getLAOptionsFromConfig(fake_args, LF_FMT_7Z, LF_WOPT_STANDARD);
		EXPECT_EQ(2, la_options.size());
		EXPECT_EQ("deflate", la_options.at("compression"));
		EXPECT_EQ("9", la_options.at("compression-level"));
	}

	{
		auto la_options = getLAOptionsFromConfig(fake_args, LF_FMT_TAR, LF_WOPT_STANDARD);
		EXPECT_EQ(1, la_options.size());
		EXPECT_EQ("UTF-8", la_options.at("hdrcharset"));
	}

	{
		auto la_options = getLAOptionsFromConfig(fake_args, LF_FMT_GZ, LF_WOPT_STANDARD);
		EXPECT_EQ(1, la_options.size());
		EXPECT_EQ("9", la_options.at("compression-level"));
	}

	{
		auto la_options = getLAOptionsFromConfig(fake_args, LF_FMT_BZ2, LF_WOPT_STANDARD);
		EXPECT_EQ(1, la_options.size());
		EXPECT_EQ("9", la_options.at("compression-level"));
	}

	{
		auto la_options = getLAOptionsFromConfig(fake_args, LF_FMT_XZ, LF_WOPT_STANDARD);
		EXPECT_EQ(2, la_options.size());
		EXPECT_EQ("9", la_options.at("compression-level"));
		EXPECT_EQ("0", la_options.at("threads"));
	}

	{
		auto la_options = getLAOptionsFromConfig(fake_args, LF_FMT_LZMA, LF_WOPT_STANDARD);
		EXPECT_EQ(1, la_options.size());
		EXPECT_EQ("9", la_options.at("compression-level"));
	}

	{
		auto la_options = getLAOptionsFromConfig(fake_args, LF_FMT_ZSTD, LF_WOPT_STANDARD);
		EXPECT_EQ(1, la_options.size());
		EXPECT_EQ("3", la_options.at("compression-level"));
	}
}

TEST(compress, compressOneArchive)
{
	_wsetlocale(LC_ALL, L"");	//default locale

	COMPRESS_SOURCES buildCompressSources(
		const LF_COMPRESS_ARGS &args,
		const std::vector<std::wstring> &givenFiles
	);
	void compressOneArchive(
		LF_ARCHIVE_FORMAT format,
		const std::map<std::string, std::string> &archive_options,
		const std::wstring& output_archive,
		const COMPRESS_SOURCES &source_files,
		ARCLOG arcLog,
		std::function<void(const std::wstring& archivePath,
			const std::wstring& path_on_disk,
			UINT64 currentSize,
			UINT64 totalSize)> progressHandler,
		std::function<const char*(struct archive*, LF_PASSPHRASE&)> passphrase_callback
	);
	//delete directory
	std::filesystem::path source_dir = UtilGetTempPath() + L"lhaforge_test/compress";
	UtilDeletePath(source_dir);
	EXPECT_FALSE(std::filesystem::exists(source_dir));
	std::filesystem::create_directories(source_dir);
	std::filesystem::create_directories(source_dir / L"a");
	std::filesystem::create_directories(source_dir / L"b/c");

	std::vector<std::wstring> givenFiles;
	givenFiles.push_back(source_dir / L"a");
	givenFiles.push_back(source_dir / L"b");
	for (int i = 0; i < 3; i++) {
		touchFile(source_dir / Format(L"a/a%03d.txt", i));
		touchFile(source_dir / Format(L"b/c/b%03d.txt", i));
	}
	{
		CAutoFile fp;
		fp.open(source_dir / L"a/test.txt", L"w");
		EXPECT_TRUE(fp.is_opened());
		fprintf(fp, "abcde");
	}

	LF_COMPRESS_ARGS fake_args;
	fake_args.compress.IgnoreTopDirectory = false;
	fake_args.compress.IgnoreTopDirectoryRecursively = false;

	auto sources = buildCompressSources(fake_args, givenFiles);
	auto single_source = buildCompressSources(fake_args, { source_dir / L"a/test.txt" });

	struct PATTERN {
		std::wstring archive_name;
		LF_ARCHIVE_FORMAT format;
		LF_WRITE_OPTIONS options;
	};
	const std::vector<PATTERN> patterns = {
		{L"output.zip",	LF_FMT_ZIP, LF_WOPT_STANDARD},	//zip
		{L"enc.zip",	LF_FMT_ZIP, LF_WOPT_DATA_ENCRYPTION},	//zip, encrypted
		{L"output.7z",	LF_FMT_7Z,	LF_WOPT_STANDARD},
		{L"output.tar",	LF_FMT_TAR, LF_WOPT_STANDARD},
		{L"output.tar.gz",	LF_FMT_TAR_GZ, LF_WOPT_STANDARD},
		{L"output.tar.bz2",	LF_FMT_TAR_BZ2, LF_WOPT_STANDARD},
		{L"output.tar.lzma",	LF_FMT_TAR_LZMA, LF_WOPT_STANDARD},
		{L"output.tar.xz",	LF_FMT_TAR_XZ, LF_WOPT_STANDARD},
		{L"output.tar.zstd",	LF_FMT_TAR_ZSTD, LF_WOPT_STANDARD},

		//not an archive, single file only
		{L"output.gz",	LF_FMT_GZ,	LF_WOPT_STANDARD},
		{L"output.bz2",	LF_FMT_BZ2,	LF_WOPT_STANDARD},
		{L"output.lzma",	LF_FMT_LZMA,	LF_WOPT_STANDARD },
		{L"output.xz",	LF_FMT_XZ,	LF_WOPT_STANDARD},
		{L"output.zst",	LF_FMT_ZSTD, LF_WOPT_STANDARD},
		{L"output.uue",	LF_FMT_UUE, LF_WOPT_STANDARD},
	};
	for(const auto &p: patterns){
		std::filesystem::path archive = UtilGetTempPath() + L"lhaforge_test/" + p.archive_name;
		ARCLOG arcLog;

		const auto& cap = get_archive_capability(p.format);

		auto la_options = getLAOptionsFromConfig(fake_args, p.format, p.options);

		auto null_passphrase_callback = [](struct archive *, LF_PASSPHRASE&) ->const char* {
			return nullptr;
		};
		auto passphrase_callback = [](struct archive *, LF_PASSPHRASE&) ->const char* {
			return "password";
		};

		if (p.options & LF_WOPT_DATA_ENCRYPTION) {
			//expect user cancel
			EXPECT_THROW(compressOneArchive(p.format, la_options, archive, 
				(cap.multi_file_archive ? sources : single_source), arcLog, [](
				const std::wstring&,
				const std::wstring&,
				UINT64,
				UINT64) {},
				null_passphrase_callback),
				LF_USER_CANCEL_EXCEPTION);

			UtilDeletePath(archive);
		}

		//expect successful compression
		EXPECT_NO_THROW(compressOneArchive(p.format, la_options, archive,
			(cap.multi_file_archive ? sources : single_source), arcLog, [&](
			const std::wstring&,
			const std::wstring&,
			UINT64,
			UINT64) {},
			passphrase_callback));

		//expect readable archive
		{
			ASSERT_TRUE(std::filesystem::exists(archive));
			EXPECT_NO_THROW(
				testOneArchive(archive, arcLog,
					[&](const std::wstring& originalPath, UINT64 currentSize, UINT64 totalSize) {},
					passphrase_callback));
		}

		UtilDeletePath(archive);
	}

	UtilDeletePath(source_dir);
}

/*TEST(compress, compress_helper)
{
	void compress_helper(
		const std::vector<std::wstring> &givenFiles,
		LF_ARCHIVE_FORMAT format,
		LF_WRITE_OPTIONS options,
		CMDLINEINFO& CmdLineInfo,
		ARCLOG &arcLog,
		LF_COMPRESS_ARGS &args,
		std::function<void(const std::wstring& archivePath,
			const std::wstring& path_on_disk,
			UINT64 currentSize,
			UINT64 totalSize)> &progressHandler
	);
	TODO;
}

TEST(compress, GUI_compress_multiple_files)
{
	bool GUI_compress_multiple_files(
		const std::vector<std::wstring> &givenFiles,
		LF_ARCHIVE_FORMAT format,
		LF_WRITE_OPTIONS options,
		CMDLINEINFO& CmdLineInfo);
	TODO;
}
*/
#endif


TEST(compress, mimic_archive_property)
{
	std::tuple<int/*la_format*/, std::vector<int>/*filters*/>
		mimic_archive_property(ARCHIVE_FILE_TO_READ& src_archive);

	{
		auto fileToRead = std::filesystem::path(__FILEW__).parent_path() / L"test_extract.zip";
		ARCHIVE_FILE_TO_READ src;
		src.read_open(fileToRead, [&](archive*, LF_PASSPHRASE&) ->const char* {return nullptr; });
		src.begin();	//need to scan
		auto[la_format, filters] = mimic_archive_property(src);
		EXPECT_EQ(la_format, ARCHIVE_FORMAT_ZIP);
		EXPECT_EQ(filters.size(), 1);
		EXPECT_EQ(filters.back(), ARCHIVE_FILTER_NONE);
	}
	{
		auto fileToRead = std::filesystem::path(__FILEW__).parent_path() / L"test_gzip.gz";
		ARCHIVE_FILE_TO_READ src;
		src.read_open(fileToRead, [&](archive*, LF_PASSPHRASE&) ->const char* {return nullptr; });
		src.begin();	//need to scan
		auto[la_format, filters] = mimic_archive_property(src);
		EXPECT_EQ(la_format, ARCHIVE_FORMAT_RAW);
		EXPECT_EQ(filters.size(), 2);
		EXPECT_EQ(filters[0], ARCHIVE_FILTER_GZIP);
		EXPECT_EQ(filters[1], ARCHIVE_FILTER_NONE);
	}
	{
		auto fileToRead = std::filesystem::path(__FILEW__).parent_path() / L"test.tar.gz";
		ARCHIVE_FILE_TO_READ src;
		src.read_open(fileToRead, [&](archive*, LF_PASSPHRASE&) ->const char* {return nullptr; });
		src.begin();	//need to scan
		auto[la_format, filters] = mimic_archive_property(src);
		EXPECT_TRUE(la_format & ARCHIVE_FORMAT_TAR);
		EXPECT_EQ(filters.size(), 2);
		EXPECT_EQ(filters[0], ARCHIVE_FILTER_GZIP);
		EXPECT_EQ(filters[1], ARCHIVE_FILTER_NONE);
	}
}

TEST(compress, copyArchive)
{
	void copyArchive(
		CConfigManager& mngr,
		const std::wstring& dest_filename,
		ARCHIVE_FILE_TO_WRITE& dest,
		const std::wstring& src_filename,
		std::function<bool(LF_ARCHIVE_ENTRY*)> false_if_skip);

	auto src_filename = std::filesystem::path(__FILEW__).parent_path() / L"test_extract.zip";
	auto tempFile = UtilGetTemporaryFileName();
	CConfigManager mngr;
	auto dummyIni = UtilGetTemporaryFileName();
	mngr.setPath(dummyIni);
	ARCHIVE_FILE_TO_WRITE dest;
	copyArchive(mngr, tempFile, dest, src_filename, [](LF_ARCHIVE_ENTRY*) {return true; });
	dest.close();

	EXPECT_TRUE(ARCHIVE_FILE_TO_READ::isKnownFormat(tempFile));

	auto tempDir = std::filesystem::path(UtilGetTempPath() + L"test_copyArchive");
	UtilDeleteDir(tempDir, true);
	EXPECT_FALSE(std::filesystem::exists(tempDir));
	std::filesystem::create_directories(tempDir);

	ARCLOG arcLog;
	EXPECT_NO_THROW(
		extractOneArchive(tempFile, tempDir, arcLog,
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

	UtilDeletePath(tempFile);
	UtilDeletePath(dummyIni);
}
