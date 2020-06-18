#include "stdafx.h"
#ifdef UNIT_TEST
#include <gtest/gtest.h>
#include "compress.h"
#include "Utilities/FileOperation.h"

TEST(compress, getSourcesBasePath)
{
	std::wstring getSourcesBasePath(const std::vector<std::wstring> &sources);

	std::filesystem::path dir = UtilGetTempPath() + L"lhaforge_test/getSourcesBasePath";
	UtilDeletePath(dir);
	EXPECT_FALSE(std::filesystem::exists(dir));
	std::filesystem::create_directories(dir / L"abc");
	std::filesystem::create_directories(dir / L"ghi");

	{
		CCurrentDirManager mngr(dir);

		EXPECT_EQ(L"", getSourcesBasePath(std::vector<std::wstring>({ })));
		EXPECT_EQ(L"abc", getSourcesBasePath(std::vector<std::wstring>({ L"abc/" })));
		EXPECT_EQ(L"abc", getSourcesBasePath(std::vector<std::wstring>({ L"abc/",L"ABC/ghi/" })));
		EXPECT_EQ(L"", getSourcesBasePath(std::vector<std::wstring>({ L"abc",L"ghi/" })));
		EXPECT_EQ(L"abc", getSourcesBasePath(std::vector<std::wstring>({ L"abc",L"abc/" })));
		EXPECT_EQ(L"c:/windows", getSourcesBasePath(std::vector<std::wstring>({ L"c:/windows",L"c:/windows/systen32" })));
	}

	UtilDeletePath(dir);
	EXPECT_FALSE(std::filesystem::exists(dir));
}

struct RAW_FILE_READER {
	CAutoFile fp;
	LF_BUFFER_INFO ibi;
	std::vector<unsigned char> buffer;
	RAW_FILE_READER() {
		ibi.make_eof();
		buffer.resize(1024 * 1024 * 32);	//32MB cache
	}
	virtual ~RAW_FILE_READER() {}
	const LF_BUFFER_INFO& operator()() {
		if (!fp || feof(fp)) {
			ibi.make_eof();
		} else {
			ibi.size = fread(&buffer[0], 1, buffer.size(), fp);
			ibi.buffer = &buffer[0];
			ibi.offset = _ftelli64(fp);
		}
		return ibi;
	}
	void open(const std::wstring& path) {
		close();
		fp.open(path, L"rb");
	}
	void close() {
		fp.close();
	}
};


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
	EXPECT_TRUE(isAllowedCombination(LF_FMT_ZIP, LF_WOPT_HEADER_ENCRYPTION));
	EXPECT_TRUE(isAllowedCombination(LF_FMT_ZIP, LF_WOPT_DATA_ENCRYPTION | LF_WOPT_HEADER_ENCRYPTION));
	EXPECT_TRUE(isAllowedCombination(LF_FMT_ZIP, LF_WOPT_SFX | LF_WOPT_DATA_ENCRYPTION));

	EXPECT_TRUE(isAllowedCombination(LF_FMT_7Z, LF_WOPT_STANDARD));
	EXPECT_TRUE(isAllowedCombination(LF_FMT_7Z, LF_WOPT_SFX));
	EXPECT_TRUE(isAllowedCombination(LF_FMT_7Z, LF_WOPT_DATA_ENCRYPTION));
	EXPECT_TRUE(isAllowedCombination(LF_FMT_7Z, LF_WOPT_HEADER_ENCRYPTION));
	EXPECT_TRUE(isAllowedCombination(LF_FMT_7Z, LF_WOPT_DATA_ENCRYPTION | LF_WOPT_HEADER_ENCRYPTION));
	EXPECT_TRUE(isAllowedCombination(LF_FMT_7Z, LF_WOPT_SFX | LF_WOPT_DATA_ENCRYPTION));

	EXPECT_TRUE(isAllowedCombination(LF_FMT_GZ, LF_WOPT_STANDARD));
	EXPECT_TRUE(isAllowedCombination(LF_FMT_BZ2, LF_WOPT_STANDARD));
	EXPECT_TRUE(isAllowedCombination(LF_FMT_LZMA, LF_WOPT_STANDARD));
	EXPECT_TRUE(isAllowedCombination(LF_FMT_XZ, LF_WOPT_STANDARD));
	EXPECT_TRUE(isAllowedCombination(LF_FMT_Z, LF_WOPT_STANDARD));
	EXPECT_TRUE(isAllowedCombination(LF_FMT_TAR, LF_WOPT_STANDARD));
	EXPECT_TRUE(isAllowedCombination(LF_FMT_TAR_GZ, LF_WOPT_STANDARD));
	EXPECT_TRUE(isAllowedCombination(LF_FMT_TAR_BZ2, LF_WOPT_STANDARD));
	EXPECT_TRUE(isAllowedCombination(LF_FMT_TAR_LZMA, LF_WOPT_STANDARD));
	EXPECT_TRUE(isAllowedCombination(LF_FMT_TAR_XZ, LF_WOPT_STANDARD));
	EXPECT_TRUE(isAllowedCombination(LF_FMT_TAR_Z, LF_WOPT_STANDARD));
	EXPECT_TRUE(isAllowedCombination(LF_FMT_CPIO, LF_WOPT_STANDARD));
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
	EXPECT_EQ(L".ext.z", getArchiveFileExtension(LF_FMT_Z, LF_WOPT_STANDARD, path));
	EXPECT_EQ(L".tar", getArchiveFileExtension(LF_FMT_TAR, LF_WOPT_STANDARD, path));
	EXPECT_EQ(L".tar.gz", getArchiveFileExtension(LF_FMT_TAR_GZ, LF_WOPT_STANDARD, path));
	EXPECT_EQ(L".tar.bz2", getArchiveFileExtension(LF_FMT_TAR_BZ2, LF_WOPT_STANDARD, path));
	EXPECT_EQ(L".tar.lzma", getArchiveFileExtension(LF_FMT_TAR_LZMA, LF_WOPT_STANDARD, path));
	EXPECT_EQ(L".tar.xz", getArchiveFileExtension(LF_FMT_TAR_XZ, LF_WOPT_STANDARD, path));
	EXPECT_EQ(L".tar.z", getArchiveFileExtension(LF_FMT_TAR_Z, LF_WOPT_STANDARD, path));
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
	EXPECT_EQ(3, result.size());
	EXPECT_EQ(L".", result[0].entryPath);
	EXPECT_EQ(L"/path/to/base/", result[0].originalFullPath);
	EXPECT_EQ(L"file1.txt", result[1].entryPath);
	EXPECT_EQ(L"/path/to/base/file1.txt", result[1].originalFullPath);
	EXPECT_EQ(std::filesystem::path(L"dir1/file2.txt").make_preferred(), result[2].entryPath);
	EXPECT_EQ(L"/path/to/base/dir1/file2.txt", result[2].originalFullPath);
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
/*
TEST(compress, buildCompressSources)
{
	COMPRESS_SOURCES buildCompressSources(
		const LF_COMPRESS_ARGS &args,
		const std::vector<std::wstring> &givenFiles
	);

	//delete directory
	std::filesystem::path dir = UtilGetTempPath() + L"lhaforge_test";
	UtilDeletePath(dir);
	EXPECT_FALSE(std::filesystem::exists(dir));
	std::filesystem::create_directories(dir);
	std::filesystem::create_directories(dir / L"a");
	std::filesystem::create_directories(dir / L"b/c");

	std::vector<std::wstring> givenFiles;
	givenFiles.push_back(dir / L"a");
	givenFiles.push_back(dir / L"b");
	for (int i = 0; i < 3; i++) {
		touchFile(dir / Format(L"a/a%03d.txt", i));
		touchFile(dir / Format(L"b/c/b%03d.txt", i));
	}
	{
		CAutoFile fp;
		fp.open(dir / L"/a/test.txt", L"w");
		EXPECT_TRUE(fp.is_opened());
		fprintf(fp, "abcde");
	}

	LF_COMPRESS_ARGS fake_args;
	fake_args.compress.IgnoreTopDirectory = false;
	fake_args.compress.IgnoreTopDirectoryRecursively = false;
	auto out = buildCompressSources(fake_args, givenFiles);

	EXPECT_EQ(dir, out.basePath);
	EXPECT_EQ(5, out.total_filesize);
	EXPECT_EQ(10, out.pathPair.size());
	EXPECT_EQ(dir / L"a", out.pathPair[0].originalFullPath);
	EXPECT_EQ(L"a", out.pathPair[0].entryPath);
	EXPECT_EQ(dir / L"a/a000.txt", out.pathPair[1].originalFullPath);
	EXPECT_EQ(L"a/a000.txt", out.pathPair[1].entryPath);

	UtilDeletePath(dir);
	EXPECT_FALSE(std::filesystem::exists(dir));
}
*/
/*TEST(compress, confirmOutputFile)
{
	std::wstring confirmOutputFile(
		const std::wstring &original_archive_path,
		const COMPRESS_SOURCES &original_source_list,
		const std::wstring& ext,	//with '.'
		bool bInputFilenameFirst);	//Compress.SpecifyOutputFilename;

	TODO;
}

TEST(compress, determineDefaultArchiveDir)
{
	std::wstring determineDefaultArchiveDir(
		OUTPUT_TO outputDirType,
		const std::wstring& original_file_path,
		const wchar_t* user_specified_dirpath
	);
	TODO;
}

TEST(compress, compressOneArchive)
{
	void compressOneArchive(
		LF_ARCHIVE_FORMAT format,
		LF_WRITE_OPTIONS options,
		const std::wstring& output_archive,
		const COMPRESS_SOURCES &source_files,
		ARCLOG arcLog,
		std::function<void(const std::wstring& archivePath,
			const std::wstring& path_on_disk,
			UINT64 currentSize,
			UINT64 totalSize)> progressHandler
	);
}

TEST(compress, compress_helper)
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

TEST(compress, GUI_compress_multiple_archives)
{
	bool GUI_compress_multiple_archives(
		const std::vector<std::wstring> &givenFiles,
		LF_ARCHIVE_FORMAT format,
		LF_WRITE_OPTIONS options,
		CMDLINEINFO& CmdLineInfo);
	TODO;
}
*/
#endif

