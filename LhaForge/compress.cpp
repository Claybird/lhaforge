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
#include "ConfigCode/ConfigManager.h"
#include "ConfigCode/ConfigCompress.h"
#include "ConfigCode/ConfigCompressFormat.h"
#include "ConfigCode/ConfigGeneral.h"
#include "CommonUtil.h"
#include "CmdLineInfo.h"

#ifdef UNIT_TEST
#include "extract.h"	//for extract test
#endif

void parseCompressOption(const CConfigManager& config, LF_COMPRESS_ARGS& args, const CMDLINEINFO* lpCmdLineInfo)
{
	args.load(config);

	//overwrite with command line arguments
	if (lpCmdLineInfo) {
		if (OUTPUT_TO_DEFAULT != lpCmdLineInfo->OutputToOverride) {
			args.compress.OutputDirType = lpCmdLineInfo->OutputToOverride;
		}
		if (CMDLINEINFO::ACTION::Default != lpCmdLineInfo->DeleteAfterProcess) {
			if (CMDLINEINFO::ACTION::False == lpCmdLineInfo->DeleteAfterProcess) {
				args.compress.DeleteAfterCompress = false;
			} else {
				args.compress.DeleteAfterCompress = true;
			}
		}
		if (CMDLINEINFO::ACTION::Default != lpCmdLineInfo->IgnoreTopDirOverride) {
			if (CMDLINEINFO::ACTION::False == lpCmdLineInfo->IgnoreTopDirOverride) {
				args.compress.IgnoreTopDirectory = false;
			} else {
				args.compress.IgnoreTopDirectory = true;
			}
		}
	}
}

//retrieves common path name of containing directories
std::wstring getSourcesBasePath(const std::vector<std::filesystem::path> &sources)
{
	std::unordered_set<std::wstring> directories;
	for (const auto &item : sources) {
		if (std::filesystem::is_directory(item)) {
			directories.insert(item);
		} else {
			directories.insert(item.parent_path());
		}
	}

	if (directories.empty())return L"";
	std::vector<std::wstring> commonParts;

	bool first = true;
	for(const auto &directory: directories){
		auto parts = UtilSplitString(
			UtilPathRemoveLastSeparator(
				replace(directory, L"\\", L"/")
			), L"/");

		if (first) {
			commonParts = parts;
			first = false;
		} else {
			size_t count = std::min(commonParts.size(), parts.size());
			for (size_t i = 0; i < count; i++) {
				//compare path; Win32 ignores path cases
				if (0 != _wcsicmp(parts[i].c_str(), commonParts[i].c_str())) {
					commonParts.resize(i);
					break;
				}
			}
		}
		if (commonParts.empty())break;
	}

	return join(L"/", commonParts);
}

#ifdef UNIT_TEST
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

#endif

//replace filenames that are not suitable for pathname
std::wstring volumeLabelToDirectoryName(const std::wstring& volume_label)
{
	const std::vector<wchar_t> toFilter = {
		L'/', L'\\', L':', L'*', L'?', L'"', L'<', L'>', L'|',
	};
	auto p = volume_label;
	for (const auto &f : toFilter) {
		p = replace(p, f, L"_");
	}
	return p;
}

#ifdef UNIT_TEST
TEST(compress, volumeLabelToDirectoryName)
{
	EXPECT_EQ(L"a_b_c_d_e_f_g_h_i_j", volumeLabelToDirectoryName(L"a/b\\c:d*e?f\"g<h>i|j"));
}
#endif

std::wstring determineDefaultArchiveTitle(
	LF_ARCHIVE_FORMAT format,
	LF_WRITE_OPTIONS option,
	const std::wstring &input_path
	)
{
	auto ext = CLFArchive::get_compression_capability(format).formatExt(input_path, option);

	//calculate archive name from first selection
	if (UtilPathIsRoot(input_path)) {
		//drives: get volume label
		//MSDN: The maximum buffer size is MAX_PATH+1
		wchar_t szVolume[MAX_PATH + 2] = {};
		GetVolumeInformationW(
			UtilPathAddLastSeparator(input_path).c_str(),
			szVolume, MAX_PATH + 1,
			nullptr, nullptr, nullptr, nullptr, 0);
		return volumeLabelToDirectoryName(szVolume) + ext;
	} else if (std::filesystem::is_directory(input_path)) {
		return std::filesystem::path(UtilPathRemoveLastSeparator(input_path)).filename().wstring() + ext;
	} else {
		//regular file: remove extension
		return std::filesystem::path(input_path).stem().wstring() + ext;
	}
}

#ifdef UNIT_TEST
TEST(compress, determineDefaultArchiveTitle)
{
	EXPECT_EQ(L"source.zip", determineDefaultArchiveTitle(LF_FMT_ZIP, LF_WOPT_STANDARD, L"/path/to/source.txt"));
	EXPECT_EQ(L"source.txt.gz", determineDefaultArchiveTitle(LF_FMT_GZ, LF_WOPT_STANDARD, L"/path/to/source.txt"));
}
#endif



//---
//get relative path of source files, from basePath
std::vector<COMPRESS_SOURCES::PATH_PAIR> getRelativePathList(
	const std::filesystem::path& basePath,
	const std::vector<std::filesystem::path>& sourcePathList)
{
	std::vector<COMPRESS_SOURCES::PATH_PAIR> out;
	for (const auto& path : sourcePathList) {
		if (basePath != path) {
			COMPRESS_SOURCES::PATH_PAIR rp;
			rp.originalFullPath = path;
			rp.entryPath = std::filesystem::relative(path, basePath);
			rp.entryPath = replace(rp.entryPath, L"\\", L"/");
			out.push_back(rp);
		}
	}
	return out;
}

#ifdef UNIT_TEST
TEST(compress, getRelativePathList)
{
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
#endif

std::vector<std::filesystem::path> getAllSourceFiles(const std::vector<std::filesystem::path> &sourcePathList)
{
	std::vector<std::filesystem::path> out;
	for (const auto& path : sourcePathList) {
		out.push_back(path);
		if (std::filesystem::is_directory(path)) {
			auto tmp = UtilRecursiveEnumFileAndDirectory(path);
			out.insert(out.end(), tmp.begin(), tmp.end());
		}
	}
	std::sort(out.begin(), out.end());
	out.erase(std::unique(out.begin(), out.end()), out.end());
	return out;
}

#ifdef UNIT_TEST
TEST(compress, getAllSourceFiles)
{
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
#endif

COMPRESS_SOURCES buildCompressSources(
	const LF_COMPRESS_ARGS &args,
	const std::vector<std::filesystem::path> &givenFiles
)
{
	COMPRESS_SOURCES targets;
	std::vector<std::filesystem::path> sourcePathList = getAllSourceFiles(givenFiles);
	targets.basePath = getSourcesBasePath(givenFiles);
	try {
		if (givenFiles.size() == 1 && std::filesystem::is_directory(givenFiles[0])) {
			if (args.compress.IgnoreTopDirectory) {
				if (args.compress.IgnoreTopDirectoryRecursively) {
					auto parent = givenFiles[0];
					auto files = UtilEnumSubFileAndDirectory(parent);
					for (;;) {
						if (files.size() == 1 && std::filesystem::is_directory(files[0])) {
							parent = files[0];
							auto files_sub = UtilEnumSubFileAndDirectory(parent);
							if (!files_sub.empty()) {
								files = files_sub;
								continue;
							}
						}
						sourcePathList = getAllSourceFiles(files);
						targets.basePath = getSourcesBasePath(sourcePathList);
						break;
					}
				} else {
					auto di = std::filesystem::directory_iterator(givenFiles.front());
					if (std::filesystem::begin(di) != std::filesystem::end(di)) {	//not an empty dir
						targets.basePath = givenFiles.front();
					}
				}
			} else {
				targets.basePath = std::filesystem::path(targets.basePath).parent_path();
			}
		}

		targets.pathPair = getRelativePathList(targets.basePath, sourcePathList);
	} catch (const std::filesystem::filesystem_error& e) {
		auto msg = UtilCP932toUNICODE(e.what(), strlen(e.what()));
		throw LF_EXCEPTION(msg);
	}

	return targets;
}


std::filesystem::path confirmOutputFile(
	const std::filesystem::path &default_archive_path,
	const COMPRESS_SOURCES &original_source_list,
	const std::wstring& ext,	//with '.'
	bool bInputFilenameFirst)	//Compress.SpecifyOutputFilename;
{
	std::set<std::filesystem::path> sourceFiles;
	for (const auto& src : original_source_list.pathPair) {
		sourceFiles.insert(toLower(src.originalFullPath));
	}

	auto archive_path = default_archive_path;
	archive_path.make_preferred();
	bool bForceOverwrite = false;
	
	//if file exists
	bInputFilenameFirst = bInputFilenameFirst || std::filesystem::exists(archive_path);

	for (;;) {
		if (!bInputFilenameFirst) {
			if (isIn(sourceFiles, toLower(archive_path))) {
				// source file and output archive are the same
				auto msg = Format(UtilLoadString(IDS_ERROR_SAME_INPUT_AND_OUTPUT), archive_path.c_str());
				ErrorMessage(msg);
			} else {
				return archive_path;
			}
		}

		// "save as" dialog
		LFShellFileSaveDialog dlg(archive_path.c_str(), FOS_FORCEFILESYSTEM | FOS_PATHMUSTEXIST | FOS_OVERWRITEPROMPT, ext.c_str());
		if (IDCANCEL == dlg.DoModal()) {
			CANCEL_EXCEPTION();
		}

		{
			CString path;
			dlg.GetFilePath(path);
			archive_path = path.operator LPCWSTR();
		}

		bInputFilenameFirst = false;
	}
}

#ifdef UNIT_TEST
TEST(compress, buildCompressSources_confirmOutputFile)
{
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
		fake_args.load(CConfigManager());
		fake_args.compress.IgnoreTopDirectory = false;
		fake_args.compress.IgnoreTopDirectoryRecursively = false;
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
		fake_args.load(CConfigManager());
		fake_args.compress.IgnoreTopDirectory = true;
		fake_args.compress.IgnoreTopDirectoryRecursively = true;
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
		fake_args.load(CConfigManager());
		fake_args.compress.IgnoreTopDirectory = false;
		fake_args.compress.IgnoreTopDirectoryRecursively = false;
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
		fake_args.load(CConfigManager());
		fake_args.compress.IgnoreTopDirectory = true;
		fake_args.compress.IgnoreTopDirectoryRecursively = false;
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
		fake_args.load(CConfigManager());
		fake_args.compress.IgnoreTopDirectory = true;
		fake_args.compress.IgnoreTopDirectoryRecursively = true;
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


#endif

std::wstring determineDefaultArchiveDir(
	OUTPUT_TO outputDirType,
	const std::filesystem::path& original_file_path,
	const wchar_t* user_specified_dirpath
	)
{
	struct FILE_CALLBACK :I_LF_GET_OUTPUT_DIR_CALLBACK {
		std::filesystem::path defaultPath;
		virtual std::filesystem::path operator()() { return defaultPath; }	//called in case OUTPUT_TO_ALWAYS_ASK_WHERE
		virtual ~FILE_CALLBACK() {}
	};
	FILE_CALLBACK fileCallback;
	fileCallback.defaultPath= std::filesystem::path(original_file_path).parent_path().make_preferred();

	return LF_get_output_dir(
		outputDirType,
		original_file_path,
		user_specified_dirpath,
		fileCallback);
}

#ifdef UNIT_TEST

TEST(compress, determineDefaultArchiveDir)
{
	auto temp = std::filesystem::path(UtilGetTempPath());
	EXPECT_EQ(UtilGetDesktopPath(), determineDefaultArchiveDir(OUTPUT_TO_DESKTOP, temp, L"C:/path_to_dir"));
	EXPECT_EQ(temp.parent_path(), determineDefaultArchiveDir(OUTPUT_TO_SAME_DIR, temp, L"C:/path_to_dir"));
	EXPECT_EQ(L"C:/path_to_dir", determineDefaultArchiveDir(OUTPUT_TO_SPECIFIC_DIR, temp, L"C:/path_to_dir"));
	EXPECT_EQ(temp.parent_path(), determineDefaultArchiveDir(OUTPUT_TO_ALWAYS_ASK_WHERE, temp, L"C:/path_to_dir"));
	EXPECT_EQ(temp.parent_path(), determineDefaultArchiveDir(OUTPUT_TO_ALWAYS_ASK_WHERE, temp, L"C:/path_to_dir"));
}

#endif

void compressOneArchive(
	LF_ARCHIVE_FORMAT format,
	LF_WRITE_OPTIONS options,
	const LF_COMPRESS_ARGS& args,
	const std::filesystem::path& output_archive,
	const COMPRESS_SOURCES &source_files,
	ARCLOG arcLog,
	ILFProgressHandler &progressHandler,
	ILFPassphrase& passphrase_callback
) {
	CLFArchive archive;
	archive.write_open(output_archive, format, options, args, passphrase_callback);
	progressHandler.setArchive(output_archive);
	progressHandler.setNumEntries(source_files.pathPair.size());
	for (const auto &source : source_files.pathPair) {
		try {
			LF_ENTRY_STAT entry;
			entry.read_file_stat(source.originalFullPath, source.entryPath);
			progressHandler.onNextEntry(source.originalFullPath, entry.stat.st_size);

			if (std::filesystem::is_regular_file(source.originalFullPath)) {
				RAW_FILE_READER provider;
				provider.open(source.originalFullPath);
				archive.add_file_entry(entry, [&]() {
					auto data = provider();
					if (!data.is_eof()) {
						progressHandler.onEntryIO(data.offset);
					}
					while (UtilDoMessageLoop())continue;
					return data;
				});

				progressHandler.onEntryIO(entry.stat.st_size);
			} else {
				//directory
				progressHandler.onNextEntry(source.originalFullPath, 0);
				archive.add_directory_entry(entry);
			}
			arcLog(output_archive, L"OK");
		} catch (const LF_USER_CANCEL_EXCEPTION& e) {	//need this to know that user cancel
			arcLog(output_archive, e.what());
			throw e;
		} catch (const LF_EXCEPTION& e) {
			arcLog(output_archive, e.what());
			throw e;
		} catch (const std::filesystem::filesystem_error& e) {
			auto msg = UtilUTF8toUNICODE(e.what(), strlen(e.what()));
			arcLog(output_archive, msg);
			throw LF_EXCEPTION(msg);
		}
	}
}

#ifdef UNIT_TEST
TEST(compress, compressOneArchive)
{
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
	fake_args.load(CConfigManager());
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
	for (const auto &p : patterns) {
		std::filesystem::path archive = UtilGetTempPath() / L"lhaforge_test" / p.archive_name;
		ARCLOG arcLog;

		const auto& cap = CLFArchive::get_compression_capability(p.format);

		if (p.options & LF_WOPT_DATA_ENCRYPTION) {
			//expect user cancel
			EXPECT_THROW(compressOneArchive(p.format, p.options, fake_args, archive,
				(cap.contains_multiple_files ? sources : single_source), arcLog,
				CLFProgressHandlerNULL(),
				CLFPassphraseNULL()),
				LF_USER_CANCEL_EXCEPTION);

			UtilDeletePath(archive);
		}

		//expect successful compression
		EXPECT_NO_THROW(compressOneArchive(p.format, p.options, fake_args, archive,
			(cap.contains_multiple_files ? sources : single_source), arcLog,
			CLFProgressHandlerNULL(),
			CLFPassphraseConst(L"password")));

		//expect readable archive
		{
			ASSERT_TRUE(std::filesystem::exists(archive));
			EXPECT_NO_THROW(
				testOneArchive(archive, arcLog,
					CLFProgressHandlerNULL(),
					CLFPassphraseConst(L"password")));
		}

		UtilDeletePath(archive);
	}

	UtilDeletePath(source_dir);
}

#endif

void compress_helper(
	const std::vector<std::filesystem::path> &givenFiles,
	LF_ARCHIVE_FORMAT format,
	LF_WRITE_OPTIONS options,
	CMDLINEINFO& CmdLineInfo,
	ARCLOG &arcLog,
	LF_COMPRESS_ARGS &args,
	ILFProgressHandler &progressHandler,
	ILFPassphrase& passphraseHandler
	)
{
	// get common path for base path
	COMPRESS_SOURCES sources = buildCompressSources(args, givenFiles);

	//check if the format can contain only one file
	const auto& cap = CLFArchive::get_compression_capability(format);
	if (!cap.contains_multiple_files) {
		size_t fileCount = 0;
		for (const auto& src : sources.pathPair) {
			if (std::filesystem::is_regular_file(src.entryPath)) {
				fileCount++;
			}
			if (fileCount >= 2) {
				//warn that archiving is not supported for this file
				ErrorMessage(UtilLoadString(IDS_ERROR_SINGLE_FILE_ONLY));
				RAISE_EXCEPTION(UtilLoadString(IDS_ERROR_SINGLE_FILE_ONLY));
			}
		}
		if (0 == fileCount) {
			ErrorMessage(UtilLoadString(IDS_ERROR_FILE_NOT_SPECIFIED));
			RAISE_EXCEPTION(UtilLoadString(IDS_ERROR_FILE_NOT_SPECIFIED));
		}
	}

	//output directory
	std::filesystem::path pathOutputDir;
	if (!CmdLineInfo.OutputDir.empty()) {
		pathOutputDir = CmdLineInfo.OutputDir;
	} else {
		auto p = std::filesystem::path(CmdLineInfo.OutputFileName).filename();
		if (!CmdLineInfo.OutputFileName.empty() && p.has_parent_path()) {
			pathOutputDir = p.parent_path();
		} else {
			pathOutputDir = determineDefaultArchiveDir(
				args.compress.OutputDirType,
				givenFiles.front(),
				args.compress.OutputDirUserSpecified.c_str());
		}
	}

	// Warn if output is on network or on a removable disk
	for (;;) {
		if (LF_confirm_output_dir_type(args.general, pathOutputDir)) {
			break;
		} else {
			// Need to change path
			LFShellFileOpenDialog dlg(pathOutputDir.c_str(), FOS_FORCEFILESYSTEM | FOS_FILEMUSTEXIST | FOS_PATHMUSTEXIST | FOS_PICKFOLDERS);
			if (IDOK == dlg.DoModal()) {
				CString tmp;
				dlg.GetFilePath(tmp);
				pathOutputDir = tmp.operator LPCWSTR();
				bool keepConfig = (GetKeyState(VK_SHIFT) < 0);	//TODO
				if (keepConfig) {
					args.compress.OutputDirType = OUTPUT_TO_SPECIFIC_DIR;
					args.compress.OutputDirUserSpecified = pathOutputDir.c_str();
				}
			} else {
				CANCEL_EXCEPTION();
			}
		}
	}
	// Confirm to make extract dir if it does not exist
	LF_ask_and_make_sure_output_dir_exists(pathOutputDir, args.general.OnDirNotFound);

	//archive file title
	std::wstring defaultArchiveTitle;
	if (CmdLineInfo.OutputFileName.empty()) {
		defaultArchiveTitle = determineDefaultArchiveTitle(format, options, givenFiles.front());
	} else {
		//only filenames
		defaultArchiveTitle = std::filesystem::path(CmdLineInfo.OutputFileName).filename();
	}

	//confirm
	arcLog.setArchivePath(pathOutputDir / defaultArchiveTitle);
	auto archivePath = confirmOutputFile(
		pathOutputDir / defaultArchiveTitle,
		sources,
		cap.formatExt(givenFiles.front(), options),
		args.compress.SpecifyOutputFilename || args.compress.OutputDirType == OUTPUT_TO_ALWAYS_ASK_WHERE);

	//delete archive
	if (std::filesystem::exists(archivePath)) {
		if (!UtilDeletePath(archivePath)) {
			auto strLastError = UtilGetLastErrorMessage();
			auto msg = Format(UtilLoadString(IDS_ERROR_FILE_REPLACE), strLastError.c_str());

			ErrorMessage(msg.c_str());
			RAISE_EXCEPTION(msg);
		}
	}

	//limit concurrent compressions
	CSemaphoreLocker SemaphoreLock;
	if (args.compress.LimitCompressFileCount) {
		const wchar_t* LHAFORGE_COMPRESS_SEMAPHORE_NAME = L"LhaForgeCompressLimitSemaphore";
		SemaphoreLock.Lock(LHAFORGE_COMPRESS_SEMAPHORE_NAME, args.compress.MaxCompressFileCount);
	}

	//do compression
	arcLog.setArchivePath(archivePath);
	compressOneArchive(
		format,
		options,
		args,
		archivePath,
		sources,
		arcLog,
		progressHandler,
		passphraseHandler);

	if (args.general.NotifyShellAfterProcess) {
		//notify shell
		::SHChangeNotify(SHCNE_CREATE, SHCNF_PATH, archivePath.c_str(), nullptr);
	}

	//open output directory
	if (args.compress.OpenDir) {
		auto pathOpenDir = std::filesystem::path(archivePath).parent_path();
		if (args.general.Filer.UseFiler) {
			auto envInfo = LF_make_expand_information(pathOpenDir.c_str(), archivePath.c_str());

			auto strCmd = UtilExpandTemplateString(args.general.Filer.FilerPath, envInfo);
			auto strParam = UtilExpandTemplateString(args.general.Filer.Param, envInfo);
			ShellExecuteW(nullptr, L"open", strCmd.c_str(), strParam.c_str(), nullptr, SW_SHOWNORMAL);
		} else {
			UtilNavigateDirectory(pathOpenDir);
		}
	}

	//delete original asrchive
	if (args.compress.DeleteAfterCompress) {
		//set current directory to that of myself
		::SetCurrentDirectoryW(UtilGetModuleDirectoryPath().c_str());
		//delete
		LF_deleteOriginalArchives(args.compress.MoveToRecycleBin, args.compress.DeleteNoConfirm, givenFiles);
	}
}

#ifdef UNIT_TEST
/*TEST(compress, compress_helper)
{
}*/
#endif

bool GUI_compress_multiple_files(
	const std::vector<std::filesystem::path> &givenFiles,
	LF_ARCHIVE_FORMAT format,
	LF_WRITE_OPTIONS options,
	ILFProgressHandler &progressHandler,
	const CConfigManager& config,
	CMDLINEINFO& CmdLineInfo)
{
	LF_COMPRESS_ARGS args;
	parseCompressOption(config, args, &CmdLineInfo);

	//do compression
	if (0 != CmdLineInfo.Options) {
		options = (LF_WRITE_OPTIONS)CmdLineInfo.Options;
	}

	size_t idxFile = 0, totalFiles = 1;

	std::vector<ARCLOG> logs;
	if (CmdLineInfo.bSingleCompression) {
		totalFiles = givenFiles.size();
		for (const auto &file : givenFiles) {
			idxFile++;
			try {
				logs.resize(logs.size() + 1);
				compress_helper(
					{ file },
					format,
					options,
					CmdLineInfo,
					logs.back(),
					args,
					progressHandler,
					CLFPassphraseGUI()
				);
			} catch (const LF_USER_CANCEL_EXCEPTION& e) {
				ARCLOG &arcLog = logs.back();
				arcLog.logException(e);
				break;
			} catch (const LF_EXCEPTION& e) {
				ARCLOG &arcLog = logs.back();
				arcLog.logException(e);
				continue;
			}
		}
	} else {
		idxFile++;
		try {
			logs.resize(logs.size() + 1);
			compress_helper(
				givenFiles,
				format,
				options,
				CmdLineInfo,
				logs.back(),
				args,
				progressHandler,
				CLFPassphraseGUI()
			);
		} catch (const LF_EXCEPTION &e) {
			ARCLOG &arcLog = logs.back();
			arcLog.logException(e);
		}
	}
	progressHandler.end();

	bool bAllOK = true;
	for (const auto& log : logs) {
		bAllOK = bAllOK && (log._overallResult == LF_RESULT::OK);
	}
	//---display logs
	bool displayLog = false;
	switch (args.general.LogViewEvent) {
	case LOGVIEW_ON_ERROR:
		if (!bAllOK) {
			displayLog = true;
		}
		break;
	case LOGVIEW_ALWAYS:
		displayLog = true;
		break;
	}

	if (displayLog) {
		CLogListDialog LogDlg(UtilLoadString(IDS_LOGINFO_OPERATION_EXTRACT).c_str());
		LogDlg.SetLogArray(logs);
		LogDlg.DoModal(::GetDesktopWindow());
	}

	return bAllOK;
}


const std::vector<COMPRESS_COMMANDLINE_PARAMETER> g_CompressionCmdParams = {
	{L"zip",		LF_FMT_ZIP,		LF_WOPT_STANDARD					,IDS_FORMAT_NAME_ZIP},
	{L"zippass",	LF_FMT_ZIP,		LF_WOPT_DATA_ENCRYPTION	,IDS_FORMAT_NAME_ZIP_PASS},
//	{L"zipsfx",	LF_FMT_ZIP,		LF_WOPT_SFX		,IDS_FORMAT_NAME_ZIP_SFX},
//	{L"zippasssfx",LF_FMT_ZIP,		LF_WOPT_DATA_ENCRYPTION | COMPRESS_SFX,IDS_FORMAT_NAME_ZIP_PASS_SFX},
//	{L"zipsplit",	LF_FMT_ZIP,		COMPRESS_SPLIT		,IDS_FORMAT_NAME_ZIP_SPLIT},
//	{L"zippasssplit",LF_FMT_ZIP,		LF_WOPT_DATA_ENCRYPTION | COMPRESS_SPLIT,IDS_FORMAT_NAME_ZIP_PASS_SPLIT},
	{L"7z",		LF_FMT_7Z,		LF_WOPT_STANDARD					,IDS_FORMAT_NAME_7Z},
//	{L"7zpass",	LF_FMT_7Z,		LF_WOPT_DATA_ENCRYPTION	,IDS_FORMAT_NAME_7Z_PASS},
//	{L"7zsfx",	LF_FMT_7Z,		LF_WOPT_SFX		,IDS_FORMAT_NAME_7Z_SFX},
//	{L"7zsplit",	LF_FMT_7Z,		COMPRESS_SPLIT		,IDS_FORMAT_NAME_7Z_SPLIT},
//	{L"7zpasssplit",LF_FMT_7Z,		LF_WOPT_DATA_ENCRYPTION | COMPRESS_SPLIT,IDS_FORMAT_NAME_7Z_PASS_SPLIT},
	{L"gz",		LF_FMT_GZ,		LF_WOPT_STANDARD			,IDS_FORMAT_NAME_GZ},
	{L"bz2",		LF_FMT_BZ2,		LF_WOPT_STANDARD			,IDS_FORMAT_NAME_BZ2},
	{L"lzma",	LF_FMT_LZMA,	LF_WOPT_STANDARD			,IDS_FORMAT_NAME_LZMA},
	{L"xz",		LF_FMT_XZ,		LF_WOPT_STANDARD			,IDS_FORMAT_NAME_XZ},
	{L"zstd",	LF_FMT_ZSTD,	LF_WOPT_STANDARD			,IDS_FORMAT_NAME_ZSTD},
	{L"tar",		LF_FMT_TAR,		LF_WOPT_STANDARD			,IDS_FORMAT_NAME_TAR},
	{L"tgz",		LF_FMT_TAR_GZ,	LF_WOPT_STANDARD			,IDS_FORMAT_NAME_TGZ},		//compatibility
	{L"tar+gz",	LF_FMT_TAR_GZ,	LF_WOPT_STANDARD			,IDS_FORMAT_NAME_TGZ},
	{L"tbz",		LF_FMT_TAR_BZ2,	LF_WOPT_STANDARD			,IDS_FORMAT_NAME_TBZ},			//compatibility
	{L"tar+bz2",	LF_FMT_TAR_BZ2,	LF_WOPT_STANDARD			,IDS_FORMAT_NAME_TBZ},
	{L"tlz",		LF_FMT_TAR_LZMA,LF_WOPT_STANDARD			,IDS_FORMAT_NAME_TAR_LZMA},		//compatibility
	{L"tar+lzma",LF_FMT_TAR_LZMA,LF_WOPT_STANDARD			,IDS_FORMAT_NAME_TAR_LZMA},
	{L"txz",		LF_FMT_TAR_XZ,	LF_WOPT_STANDARD			,IDS_FORMAT_NAME_TAR_XZ},		//compatibility
	{L"tar+xz",	LF_FMT_TAR_XZ,	LF_WOPT_STANDARD			,IDS_FORMAT_NAME_TAR_XZ},
	{L"tar+zstd",LF_FMT_TAR_ZSTD,LF_WOPT_STANDARD			,IDS_FORMAT_NAME_TAR_ZSTD},
	{L"uue",		LF_FMT_UUE,		LF_WOPT_STANDARD			,IDS_FORMAT_NAME_UUE},
};


const COMPRESS_COMMANDLINE_PARAMETER& get_archive_format_args(LF_ARCHIVE_FORMAT fmt, int opts)
{
	for (const auto &item : g_CompressionCmdParams) {
		if (item.Type == fmt && item.Options == opts) {
			return item;
		}
	}
	throw ARCHIVE_EXCEPTION(EINVAL);
}

#ifdef UNIT_TEST
TEST(compress, get_archive_format_args)
{
	EXPECT_EQ(L"zip", get_archive_format_args(LF_FMT_ZIP, LF_WOPT_STANDARD).name);
	EXPECT_EQ(L"zippass", get_archive_format_args(LF_FMT_ZIP, LF_WOPT_DATA_ENCRYPTION).name);
	EXPECT_EQ(L"7z", get_archive_format_args(LF_FMT_7Z, LF_WOPT_STANDARD).name);
	EXPECT_EQ(L"gz", get_archive_format_args(LF_FMT_GZ, LF_WOPT_STANDARD).name);
	EXPECT_EQ(L"bz2", get_archive_format_args(LF_FMT_BZ2, LF_WOPT_STANDARD).name);
	EXPECT_EQ(L"lzma", get_archive_format_args(LF_FMT_LZMA, LF_WOPT_STANDARD).name);
	EXPECT_EQ(L"xz", get_archive_format_args(LF_FMT_XZ, LF_WOPT_STANDARD).name);
	EXPECT_EQ(L"zstd", get_archive_format_args(LF_FMT_ZSTD, LF_WOPT_STANDARD).name);
	EXPECT_EQ(L"tar", get_archive_format_args(LF_FMT_TAR, LF_WOPT_STANDARD).name);
	EXPECT_EQ(L"tgz", get_archive_format_args(LF_FMT_TAR_GZ, LF_WOPT_STANDARD).name);
	//EXPECT_EQ(L"tar+gz", get_archive_format_args(LF_FMT_TAR_GZ, LF_WOPT_STANDARD).name);
	EXPECT_EQ(L"tbz", get_archive_format_args(LF_FMT_TAR_BZ2, LF_WOPT_STANDARD).name);
	//EXPECT_EQ(L"tar+bz2", get_archive_format_args(LF_FMT_TAR_BZ2, LF_WOPT_STANDARD).name);
	EXPECT_EQ(L"tlz", get_archive_format_args(LF_FMT_TAR_LZMA, LF_WOPT_STANDARD).name);
	//EXPECT_EQ(L"tar+lzma", get_archive_format_args(LF_FMT_TAR_LZMA, LF_WOPT_STANDARD).name);
	EXPECT_EQ(L"txz", get_archive_format_args(LF_FMT_TAR_XZ, LF_WOPT_STANDARD).name);
	//EXPECT_EQ(L"tar+xz", get_archive_format_args(LF_FMT_TAR_XZ, LF_WOPT_STANDARD).name);
	EXPECT_EQ(L"tar+zstd", get_archive_format_args(LF_FMT_TAR_ZSTD, LF_WOPT_STANDARD).name);
	EXPECT_EQ(L"uue", get_archive_format_args(LF_FMT_UUE, LF_WOPT_STANDARD).name);
}
#endif


//----

#ifdef UNIT_TEST


#pragma comment(lib,"Bcrypt.lib")
TEST(compress, RAW_FILE_READER)
{
	auto fileToRead = std::filesystem::path(__FILEW__).parent_path() / L"test/test_raw_file_reader.txt";
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
	auto src_filename = std::filesystem::path(__FILEW__).parent_path() / L"test/test_extract.zip";
	auto tempFile = UtilGetTemporaryFileName();
	CLFArchive src;
	LF_COMPRESS_ARGS fake_args;
	fake_args.load(CConfigManager());
	src.read_open(src_filename, CLFPassphraseNULL());
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

	auto tempDir = UtilGetTempPath() / L"test_copyArchive";
	UtilDeleteDir(tempDir, true);
	EXPECT_FALSE(std::filesystem::exists(tempDir));
	std::filesystem::create_directories(tempDir);

	ARCLOG arcLog;
	CLFArchive arc;
	CLFOverwriteConfirmFORCED preExtractHandler(overwrite_options::abort);
	EXPECT_NO_THROW(arc.read_open(tempFile, CLFPassphraseNULL()));
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
#endif
