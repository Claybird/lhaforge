/*
* MIT License

* Copyright (c) 2005- Claybird

* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:

* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.

* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/

#include "stdafx.h"
#include "compress.h"
#include "ArchiverCode/archive.h"
#include "Dialogs/LogListDialog.h"

#include "resource.h"

#include "Utilities/Semaphore.h"
#include "Utilities/StringUtil.h"
#include "Utilities/FileOperation.h"
#include "Utilities/OSUtil.h"
#include "Utilities/CustomControl.h"
#include "ConfigCode/ConfigFile.h"
#include "ConfigCode/ConfigCompress.h"
#include "ConfigCode/ConfigCompressFormat.h"
#include "ConfigCode/ConfigGeneral.h"
#include "CommonUtil.h"
#include "CmdLineInfo.h"

#include "extract.h"	//for extract test

std::wstring getSourcesBasePath(const std::vector<std::filesystem::path>& sources);

TEST(compress, getSourcesBasePath)
{
	auto dir = UtilGetTempPath() / L"lhaforge_test/getSourcesBasePath";
	UtilDeletePath(dir);
	EXPECT_FALSE(std::filesystem::exists(dir));
	std::filesystem::create_directories(dir / L"abc");
	std::filesystem::create_directories(dir / L"ghi");

	{
		EXPECT_EQ(L"", getSourcesBasePath({ }));
		EXPECT_EQ(dir / L"abc", getSourcesBasePath({ dir / L"abc/" }));
		EXPECT_EQ(dir / L"abc", getSourcesBasePath({ dir / L"abc/",dir / L"ABC/ghi/" }));
		EXPECT_EQ(dir, getSourcesBasePath({ dir / L"abc",dir / L"ghi/" }));
		EXPECT_EQ(dir / L"abc", getSourcesBasePath({ dir / L"abc",dir / L"abc/" }));
		EXPECT_EQ(std::filesystem::path(L"c:/windows"), getSourcesBasePath({ L"c:/windows",L"c:/windows/system32" }));
	}

	UtilDeletePath(dir);
	EXPECT_FALSE(std::filesystem::exists(dir));
}

TEST(compress, volumeLabelToDirectoryName)
{
	std::wstring volumeLabelToDirectoryName(const std::wstring & volume_label);

	EXPECT_EQ(L"a_b_c_d_e_f_g_h_i_j", volumeLabelToDirectoryName(L"a/b\\c:d*e?f\"g<h>i|j"));
}

TEST(compress, determineDefaultArchiveTitle)
{
	std::wstring determineDefaultArchiveTitle(
		LF_ARCHIVE_FORMAT format,
		LF_WRITE_OPTIONS option,
		const std::wstring & input_path
	);

	EXPECT_EQ(L"source.zip", determineDefaultArchiveTitle(LF_ARCHIVE_FORMAT::ZIP, LF_WOPT_STANDARD, L"/path/to/source.txt"));
	EXPECT_EQ(L"source.txt.gz", determineDefaultArchiveTitle(LF_ARCHIVE_FORMAT::GZ, LF_WOPT_STANDARD, L"/path/to/source.txt"));
}


TEST(compress, getRelativePathList)
{
	std::vector<COMPRESS_SOURCES::PATH_PAIR> getRelativePathList(
		const std::filesystem::path & basePath,
		const std::vector<std::filesystem::path>&sourcePathList);

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
	std::vector<std::filesystem::path> getAllSourceFiles(const std::vector<std::filesystem::path> &sourcePathList);

	//delete directory
	std::filesystem::path dir = UtilGetTempPath() / L"lhaforge_test/getAllSourceFiles";
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
		const LF_COMPRESS_ARGS & args,
		const std::vector<std::filesystem::path> &givenFiles
	);
	std::filesystem::path confirmOutputFile(
		const std::filesystem::path & default_archive_path,
		const COMPRESS_SOURCES & original_source_list,
		const std::wstring & ext,	//with '.'
		bool bInputFilenameFirst);	//Compress.SpecifyOutputFilename;

	//delete directory
	std::filesystem::path dir = UtilGetTempPath() / L"lhaforge_test/compressSources";
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
		std::vector<std::filesystem::path> givenFiles;
		givenFiles.push_back(dir / L"a");
		givenFiles.push_back(dir / L"b");

		LF_COMPRESS_ARGS fake_args;
		fake_args.load(CConfigFile());
		fake_args.compress.IgnoreTopDirectory = (int)COMPRESS_IGNORE_TOP_DIR::None;
		auto sources = buildCompressSources(fake_args, givenFiles);

		EXPECT_EQ(dir, sources.basePath);
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
		auto output_path = confirmOutputFile(dir / L"test.archive", sources, L".archive", false);
		EXPECT_EQ(dir / L"test.archive", output_path);
	}
	{
		std::vector<std::filesystem::path> givenFiles;
		givenFiles.push_back(dir / L"a");
		givenFiles.push_back(dir / L"b");

		LF_COMPRESS_ARGS fake_args;
		fake_args.load(CConfigFile());
		fake_args.compress.IgnoreTopDirectory = (int)COMPRESS_IGNORE_TOP_DIR::Recursive;
		auto sources = buildCompressSources(fake_args, givenFiles);

		EXPECT_EQ(dir, sources.basePath);
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
		std::vector<std::filesystem::path> givenFiles;
		givenFiles.push_back(dir / L"a");

		LF_COMPRESS_ARGS fake_args;
		fake_args.load(CConfigFile());
		fake_args.compress.IgnoreTopDirectory = (int)COMPRESS_IGNORE_TOP_DIR::None;
		auto sources = buildCompressSources(fake_args, givenFiles);

		EXPECT_EQ(dir, sources.basePath);
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
		std::vector<std::filesystem::path> givenFiles;
		givenFiles.push_back(dir / L"b");

		LF_COMPRESS_ARGS fake_args;
		fake_args.load(CConfigFile());
		fake_args.compress.IgnoreTopDirectory = (int)COMPRESS_IGNORE_TOP_DIR::FirstTop;
		auto sources = buildCompressSources(fake_args, givenFiles);

		EXPECT_EQ(dir / L"b", sources.basePath);
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
		std::vector<std::filesystem::path> givenFiles;
		givenFiles.push_back(dir / L"b");

		LF_COMPRESS_ARGS fake_args;
		fake_args.load(CConfigFile());
		fake_args.compress.IgnoreTopDirectory = (int)COMPRESS_IGNORE_TOP_DIR::Recursive;
		auto sources = buildCompressSources(fake_args, givenFiles);

		EXPECT_EQ(dir / L"b/c", sources.basePath);
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
		const std::filesystem::path & original_file_path,
		const wchar_t* user_specified_dirpath
	);
	auto temp = std::filesystem::path(UtilGetTempPath());
	EXPECT_EQ(UtilGetDesktopPath(), determineDefaultArchiveDir(OUTPUT_TO::Desktop, temp, L"C:/path_to_dir"));
	EXPECT_EQ(temp.parent_path(), determineDefaultArchiveDir(OUTPUT_TO::SameDir, temp, L"C:/path_to_dir"));
	EXPECT_EQ(L"C:/path_to_dir", determineDefaultArchiveDir(OUTPUT_TO::SpecificDir, temp, L"C:/path_to_dir"));
	EXPECT_EQ(temp.parent_path(), determineDefaultArchiveDir(OUTPUT_TO::AlwaysAsk, temp, L"C:/path_to_dir"));
}


TEST(compress, compressOneArchive)
{
	COMPRESS_SOURCES buildCompressSources(
		const LF_COMPRESS_ARGS & args,
		const std::vector<std::filesystem::path> &givenFiles
	);
	void compressOneArchive(
		LF_ARCHIVE_FORMAT format,
		LF_WRITE_OPTIONS options,
		const LF_COMPRESS_ARGS & args,
		const std::filesystem::path & output_archive,
		const COMPRESS_SOURCES & source_files,
		ARCLOG & arcLog,
		ILFProgressHandler & progressHandler,
		std::shared_ptr<ILFPassphrase> passphrase_callback
	);
	_wsetlocale(LC_ALL, L"");	//default locale

	//delete directory
	std::filesystem::path source_dir = UtilGetTempPath() / L"lhaforge_test/compress";
	UtilDeletePath(source_dir);
	EXPECT_FALSE(std::filesystem::exists(source_dir));
	std::filesystem::create_directories(source_dir);
	std::filesystem::create_directories(source_dir / L"a");
	std::filesystem::create_directories(source_dir / L"b/c");

	std::vector<std::filesystem::path> givenFiles;
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
	fake_args.load(CConfigFile());
	fake_args.compress.IgnoreTopDirectory = (int)COMPRESS_IGNORE_TOP_DIR::None;

	auto sources = buildCompressSources(fake_args, givenFiles);
	auto single_source = buildCompressSources(fake_args, { source_dir / L"a/test.txt" });

	auto test_helper = [&](std::wstring archive_name,
		LF_ARCHIVE_FORMAT format,
		LF_WRITE_OPTIONS options
		) {
		std::filesystem::path archive = UtilGetTempPath() / L"lhaforge_test" / archive_name;
		ARCLOG arcLog;

		const auto& cap = CLFArchive::get_compression_capability(format);

		if (options & LF_WOPT_DATA_ENCRYPTION) {
			//expect user cancel
			auto pp = std::make_shared<CLFPassphraseNULL>();
			EXPECT_THROW(compressOneArchive(format, options, fake_args, archive,
				(cap.contains_multiple_files ? sources : single_source), arcLog,
				CLFProgressHandlerNULL(),
				pp),
				LF_USER_CANCEL_EXCEPTION);

			UtilDeletePath(archive);
		}

		//expect successful compression
		compressOneArchive(format, options, fake_args, archive,
			(cap.contains_multiple_files ? sources : single_source), arcLog,
			CLFProgressHandlerNULL(),
			std::make_shared<CLFPassphraseConst>(L"password"));

		//expect readable archive
		{
			ASSERT_TRUE(std::filesystem::exists(archive));
			testOneArchive(archive, arcLog,
				CLFProgressHandlerNULL(),
				std::make_shared<CLFPassphraseConst>(L"password"));
		}

		UtilDeletePath(archive);
	};

	EXPECT_NO_THROW(test_helper(L"output.zip", LF_ARCHIVE_FORMAT::ZIP, LF_WOPT_STANDARD));	//zip
	EXPECT_NO_THROW(test_helper(L"enc.zip",	LF_ARCHIVE_FORMAT::ZIP, LF_WOPT_DATA_ENCRYPTION));	//zip, encrypted
	EXPECT_NO_THROW(test_helper(L"output.7z",	LF_ARCHIVE_FORMAT::_7Z,	LF_WOPT_STANDARD));
	EXPECT_NO_THROW(test_helper(L"output.tar",	LF_ARCHIVE_FORMAT::TAR, LF_WOPT_STANDARD));
	EXPECT_NO_THROW(test_helper(L"output.tar.gz",	LF_ARCHIVE_FORMAT::TAR_GZ, LF_WOPT_STANDARD));
	EXPECT_NO_THROW(test_helper(L"output.tar.bz2",	LF_ARCHIVE_FORMAT::TAR_BZ2, LF_WOPT_STANDARD));
	EXPECT_NO_THROW(test_helper(L"output.tar.lzma",	LF_ARCHIVE_FORMAT::TAR_LZMA, LF_WOPT_STANDARD));
	EXPECT_NO_THROW(test_helper(L"output.tar.xz",	LF_ARCHIVE_FORMAT::TAR_XZ, LF_WOPT_STANDARD));
	EXPECT_NO_THROW(test_helper(L"output.tar.zst", LF_ARCHIVE_FORMAT::TAR_ZSTD, LF_WOPT_STANDARD));
	EXPECT_NO_THROW(test_helper(L"output.tar.lz4",	LF_ARCHIVE_FORMAT::TAR_LZ4, LF_WOPT_STANDARD));

	//not an archive, single file only
	EXPECT_NO_THROW(test_helper(L"output.gz",	LF_ARCHIVE_FORMAT::GZ,	LF_WOPT_STANDARD));
	EXPECT_NO_THROW(test_helper(L"output.bz2",	LF_ARCHIVE_FORMAT::BZ2,	LF_WOPT_STANDARD));
	EXPECT_NO_THROW(test_helper(L"output.lzma",	LF_ARCHIVE_FORMAT::LZMA,	LF_WOPT_STANDARD));
	EXPECT_NO_THROW(test_helper(L"output.xz",	LF_ARCHIVE_FORMAT::XZ,	LF_WOPT_STANDARD));
	EXPECT_NO_THROW(test_helper(L"output.zst", LF_ARCHIVE_FORMAT::ZSTD, LF_WOPT_STANDARD));
	EXPECT_NO_THROW(test_helper(L"output.lz4",	LF_ARCHIVE_FORMAT::LZ4, LF_WOPT_STANDARD));

	UtilDeletePath(source_dir);
}

/*TEST(compress, compress_helper)
{
}*/

TEST(compress, get_archive_format_args)
{
	EXPECT_EQ(L"zip", get_archive_format_args(LF_ARCHIVE_FORMAT::ZIP, LF_WOPT_STANDARD).name);
	EXPECT_EQ(L"zippass", get_archive_format_args(LF_ARCHIVE_FORMAT::ZIP, LF_WOPT_DATA_ENCRYPTION).name);
	EXPECT_EQ(L"7z", get_archive_format_args(LF_ARCHIVE_FORMAT::_7Z, LF_WOPT_STANDARD).name);
	EXPECT_EQ(L"gz", get_archive_format_args(LF_ARCHIVE_FORMAT::GZ, LF_WOPT_STANDARD).name);
	EXPECT_EQ(L"bz2", get_archive_format_args(LF_ARCHIVE_FORMAT::BZ2, LF_WOPT_STANDARD).name);
	EXPECT_EQ(L"lzma", get_archive_format_args(LF_ARCHIVE_FORMAT::LZMA, LF_WOPT_STANDARD).name);
	EXPECT_EQ(L"xz", get_archive_format_args(LF_ARCHIVE_FORMAT::XZ, LF_WOPT_STANDARD).name);
	EXPECT_EQ(L"zstd", get_archive_format_args(LF_ARCHIVE_FORMAT::ZSTD, LF_WOPT_STANDARD).name);
	EXPECT_EQ(L"lz4", get_archive_format_args(LF_ARCHIVE_FORMAT::LZ4, LF_WOPT_STANDARD).name);
	EXPECT_EQ(L"tar", get_archive_format_args(LF_ARCHIVE_FORMAT::TAR, LF_WOPT_STANDARD).name);
	EXPECT_EQ(L"tgz", get_archive_format_args(LF_ARCHIVE_FORMAT::TAR_GZ, LF_WOPT_STANDARD).name);
	//EXPECT_EQ(L"tar+gz", get_archive_format_args(LF_ARCHIVE_FORMAT::TAR_GZ, LF_WOPT_STANDARD).name);
	EXPECT_EQ(L"tbz", get_archive_format_args(LF_ARCHIVE_FORMAT::TAR_BZ2, LF_WOPT_STANDARD).name);
	//EXPECT_EQ(L"tar+bz2", get_archive_format_args(LF_ARCHIVE_FORMAT::TAR_BZ2, LF_WOPT_STANDARD).name);
	EXPECT_EQ(L"tlz", get_archive_format_args(LF_ARCHIVE_FORMAT::TAR_LZMA, LF_WOPT_STANDARD).name);
	//EXPECT_EQ(L"tar+lzma", get_archive_format_args(LF_ARCHIVE_FORMAT::TAR_LZMA, LF_WOPT_STANDARD).name);
	EXPECT_EQ(L"txz", get_archive_format_args(LF_ARCHIVE_FORMAT::TAR_XZ, LF_WOPT_STANDARD).name);
	//EXPECT_EQ(L"tar+xz", get_archive_format_args(LF_ARCHIVE_FORMAT::TAR_XZ, LF_WOPT_STANDARD).name);
	EXPECT_EQ(L"tar+zstd", get_archive_format_args(LF_ARCHIVE_FORMAT::TAR_ZSTD, LF_WOPT_STANDARD).name);
	EXPECT_EQ(L"tar+lz4", get_archive_format_args(LF_ARCHIVE_FORMAT::TAR_LZ4, LF_WOPT_STANDARD).name);
}

TEST(compress, RAW_FILE_READER)
{
	auto fileToRead = std::filesystem::path(__FILEW__).parent_path() / L"test_raw_file_reader.txt";
	std::string expected_hash = "dc2545110ea53ef9ce169fd676cf9f24a966e6571be630d221eae8b8bb7717a5";

	RAW_FILE_READER reader;
	EXPECT_THROW(reader.open(L"some_non_existing_file"), LF_EXCEPTION);
	reader.open(fileToRead);
	std::vector<char> buf;
	for (;;) {
		auto bi = reader();
		if (!bi.size)break;
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
						if (NT_SUCCESS(BCryptHashData(hHash, (PBYTE)&buf[0], (ULONG)buf.size(), 0))) {
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




/*

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

TEST(compress, copyArchive)	//or maybe test for CLFArchive
{
	auto src_filename = std::filesystem::path(__FILEW__).parent_path() / L"test_extract.zip";
	auto tempFile = UtilGetTemporaryFileName();
	CLFArchive src;
	LF_COMPRESS_ARGS fake_args;
	fake_args.load(CConfigFile());
	auto pp = std::make_shared<CLFPassphraseNULL>();
	src.read_open(src_filename, pp);
	auto dest = src.make_copy_archive(tempFile, fake_args, [](const LF_ENTRY_STAT& entry) {
		if (entry.path.wstring().find(L"dirC") == std::wstring::npos) {
			return true;
		} else {
			return false;
		}
		return true;
	});

	dest->close();
	dest = nullptr;

	EXPECT_TRUE(CLFArchive::is_known_format(tempFile));

	EXPECT_NO_THROW({
		ARCLOG arcLog;
		CLFProgressHandlerNULL progressHandler;
		testOneArchive(tempFile, arcLog, progressHandler, std::make_shared<CLFPassphraseNULL>());
		});

	auto tempDir = UtilGetTempPath() / L"test_copyArchive";
	UtilDeleteDir(tempDir, true);
	EXPECT_FALSE(std::filesystem::exists(tempDir));
	std::filesystem::create_directories(tempDir);

	ARCLOG arcLog;
	CLFArchive arc;
	CLFOverwriteConfirmFORCED preExtractHandler(overwrite_options::abort);
	EXPECT_NO_THROW(arc.read_open(tempFile, pp));
	EXPECT_NO_THROW(
		for (auto entry = arc.read_entry_begin(); entry; entry = arc.read_entry_next()) {
			extractCurrentEntry(arc, entry, tempDir, arcLog, preExtractHandler,
				CLFProgressHandlerNULL());
		}
	);

	EXPECT_TRUE(std::filesystem::exists(tempDir / L"dirA"));
	EXPECT_TRUE(std::filesystem::exists(tempDir / L"dirA/dirB"));
	EXPECT_TRUE(std::filesystem::exists(tempDir / L"dirA/dirB/file2.txt"));
	EXPECT_FALSE(std::filesystem::exists(tempDir / L"dirA/dirB/dirC"));
	EXPECT_FALSE(std::filesystem::exists(tempDir / L"dirA/dirB/dirC/file1.txt"));
	EXPECT_TRUE(std::filesystem::exists(tempDir / L"かきくけこ"));
	EXPECT_TRUE(std::filesystem::exists(tempDir / L"かきくけこ/file3.txt"));
	EXPECT_TRUE(std::filesystem::exists(tempDir / L"あいうえお.txt"));

	UtilDeleteDir(tempDir, true);

	UtilDeletePath(tempFile);
}
