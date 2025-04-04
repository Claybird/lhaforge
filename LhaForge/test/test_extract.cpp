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
#include "extract.h"
#include "resource.h"
#include "Dialogs/LogListDialog.h"
#include "Utilities/Semaphore.h"
#include "Utilities/StringUtil.h"
#include "Utilities/FileOperation.h"
#include "Utilities/OSUtil.h"
#include "Utilities/CustomControl.h"
#include "Utilities/Utility.h"
#include "CommonUtil.h"
#include "CmdLineInfo.h"


TEST(extract, trimArchiveName) {
	std::filesystem::path trimArchiveName(bool RemoveSymbolAndNumber, const std::filesystem::path & archive_path);

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
	std::filesystem::path determineExtractBaseDir(
		const std::filesystem::path & archive_path,
		LF_EXTRACT_ARGS & args);

	LF_EXTRACT_ARGS fakeArg;
	fakeArg.load(CConfigFile());
	fakeArg.extract.OutputDirType = (int)OUTPUT_TO::SpecificDir;
	fakeArg.extract.OutputDirUserSpecified = std::filesystem::current_path().c_str();
	fakeArg.general.WarnNetwork = FALSE;
	fakeArg.general.WarnRemovable = FALSE;
	fakeArg.general.OnDirNotFound = (int)LOSTDIR::ForceCreate;

	auto out = determineExtractBaseDir(L"path_to_archive/archive.ext", fakeArg);
	EXPECT_EQ(std::filesystem::current_path(), out);
}

TEST(extract, preExtractCheck) {
	std::tuple<PRE_EXTRACT_CHECK, std::filesystem::path /*baseDirName*/>
		preExtractCheck(ILFArchiveFile& arc, ILFScanProgressHandler& progressHandler);

	{
		auto [result, baseDirName] = preExtractCheck(CLFArchiveNULL(), CLFScanProgressHandlerNULL());
		EXPECT_EQ(PRE_EXTRACT_CHECK::unknown, result);
	}
	{
		CLFArchive a;
		auto pp = std::make_shared<CLFPassphraseNULL>();
		a.read_open(LF_PROJECT_DIR() / L"test/test_extract.zip", pp);
		auto [result, baseDirName] = preExtractCheck(a, CLFScanProgressHandlerNULL());
		EXPECT_EQ(PRE_EXTRACT_CHECK::multipleEntries, result);
	}
	{
		CLFArchive a;
		auto pp = std::make_shared<CLFPassphraseNULL>();
		a.read_open(LF_PROJECT_DIR() / L"test/test_extract.zipx", pp);
		auto [result, baseDirName] = preExtractCheck(a, CLFScanProgressHandlerNULL());
		EXPECT_EQ(PRE_EXTRACT_CHECK::multipleEntries, result);
	}
	{
		CLFArchive a;
		auto pp = std::make_shared<CLFPassphraseNULL>();
		a.read_open(LF_PROJECT_DIR() / L"test/test_gzip.gz", pp);
		auto [result, baseDirName] = preExtractCheck(a, CLFScanProgressHandlerNULL());
		EXPECT_EQ(PRE_EXTRACT_CHECK::singleFile, result);
	}
	{
		CLFArchive a;
		auto pp = std::make_shared<CLFPassphraseNULL>();
		a.read_open(LF_PROJECT_DIR() / L"test/test.lzh", pp);
		auto [result, baseDirName] = preExtractCheck(a, CLFScanProgressHandlerNULL());
		EXPECT_EQ(PRE_EXTRACT_CHECK::singleFile, result);
	}
	{
		CLFArchive a;
		auto pp = std::make_shared<CLFPassphraseNULL>();
		a.read_open(LF_PROJECT_DIR() / L"test/test.tar.gz", pp);
		auto [result, baseDirName] = preExtractCheck(a, CLFScanProgressHandlerNULL());
		EXPECT_EQ(PRE_EXTRACT_CHECK::singleDir, result);
		EXPECT_EQ(L"test", baseDirName);
	}
}

TEST(extract, determineExtractDir) {
	std::filesystem::path determineExtractDir(
		ILFArchiveFile & arc,
		ILFScanProgressHandler & progress,
		const std::filesystem::path & archive_path,
		const std::filesystem::path & output_base_dir,
		const LF_EXTRACT_ARGS & args);

	LF_EXTRACT_ARGS fakeArg;
	fakeArg.load(CConfigFile());
	{
		fakeArg.extract.CreateDir = (int)EXTRACT_CREATE_DIR::Never;
		fakeArg.extract.RemoveSymbolAndNumber = false;
		CLFArchiveNULL arc;
		auto pp = std::make_shared<CLFPassphraseNULL>();
		arc.read_open(L"path_to_archive/archive.ext", pp);
		EXPECT_EQ(L"path_to_output",
			determineExtractDir(arc, CLFScanProgressHandlerNULL(), L"path_to_archive/archive.ext", L"path_to_output", fakeArg));

		arc.read_open(L"path_to_archive/archive  .ext", pp);
		EXPECT_EQ(L"path_to_output",
			determineExtractDir(arc, CLFScanProgressHandlerNULL(), L"path_to_archive/archive   .ext", L"path_to_output", fakeArg));
	}

	{
		CLFArchiveNULL arc;
		fakeArg.extract.CreateDir = (int)EXTRACT_CREATE_DIR::Always;
		auto pp = std::make_shared<CLFPassphraseNULL>();
		arc.read_open(L"path_to_archive/archive.ext", pp);
		EXPECT_EQ(L"path_to_output/archive",
			determineExtractDir(arc, CLFScanProgressHandlerNULL(), L"path_to_archive/archive.ext", L"path_to_output", fakeArg));
		arc.read_open(L"path_to_archive/archive  .ext", pp);
		EXPECT_EQ(L"path_to_output/archive",
			determineExtractDir(arc, CLFScanProgressHandlerNULL(), L"path_to_archive/archive  .ext", L"path_to_output", fakeArg));
	}

	//---
	{
		CLFArchive arc;
		fakeArg.extract.CreateDir = (int)EXTRACT_CREATE_DIR::SkipIfSingleFileOrDir;
		auto pp = std::make_shared<CLFPassphraseNULL>();
		arc.read_open(LF_PROJECT_DIR() / L"test/test.tar.gz", pp);
		EXPECT_EQ(L"path_to_output",
			determineExtractDir(arc, CLFScanProgressHandlerNULL(), LF_PROJECT_DIR() / L"test/test.tar.gz", L"path_to_output", fakeArg));
	}
	{
		CLFArchive arc;
		fakeArg.extract.CreateDir = (int)EXTRACT_CREATE_DIR::SkipIfSingleDirectory;
		auto pp = std::make_shared<CLFPassphraseNULL>();
		arc.read_open(LF_PROJECT_DIR() / L"test/test.tar.gz", pp);
		EXPECT_EQ(L"path_to_output",
			determineExtractDir(arc, CLFScanProgressHandlerNULL(), LF_PROJECT_DIR() / L"test/test.tar.gz", L"path_to_output", fakeArg));
	}
	//---
	{
		CLFArchive arc;
		fakeArg.extract.CreateDir = (int)EXTRACT_CREATE_DIR::SkipIfSingleFileOrDir;
		auto pp = std::make_shared<CLFPassphraseNULL>();
		arc.read_open(LF_PROJECT_DIR() / L"test/test.lzh", pp);
		EXPECT_EQ(L"path_to_output",
			determineExtractDir(arc, CLFScanProgressHandlerNULL(), LF_PROJECT_DIR() / L"test/test.lzh", L"path_to_output", fakeArg));
	}
	{
		CLFArchive arc;
		fakeArg.extract.CreateDir = (int)EXTRACT_CREATE_DIR::SkipIfSingleDirectory;
		auto pp = std::make_shared<CLFPassphraseNULL>();
		arc.read_open(LF_PROJECT_DIR() / L"test/test.lzh", pp);
		EXPECT_EQ(L"path_to_output/test",
			determineExtractDir(arc, CLFScanProgressHandlerNULL(), LF_PROJECT_DIR() / L"test/test.lzh", L"path_to_output", fakeArg));
	}
	//---
	{
		CLFArchive arc;
		fakeArg.extract.CreateDir = (int)EXTRACT_CREATE_DIR::SkipIfSingleFileOrDir;
		auto pp = std::make_shared<CLFPassphraseNULL>();
		arc.read_open(LF_PROJECT_DIR() / L"test/test_extract.zip", pp);
		EXPECT_EQ(L"path_to_output/test_extract",
			determineExtractDir(arc, CLFScanProgressHandlerNULL(), LF_PROJECT_DIR() / L"test/test_extract.zip", L"path_to_output", fakeArg));
	}
	{
		CLFArchive arc;
		fakeArg.extract.CreateDir = (int)EXTRACT_CREATE_DIR::SkipIfSingleDirectory;
		auto pp = std::make_shared<CLFPassphraseNULL>();
		arc.read_open(LF_PROJECT_DIR() / L"test/test_extract.zip", pp);
		EXPECT_EQ(L"path_to_output/test_extract",
			determineExtractDir(arc, CLFScanProgressHandlerNULL(), LF_PROJECT_DIR() / L"test/test_extract.zip", L"path_to_output", fakeArg));
	}
}

TEST(extract, parseExtractOption) {
	void parseExtractOption(LF_EXTRACT_ARGS & args, CConfigFile & mngr, const CMDLINEINFO * lpCmdLineInfo);

	{
		LF_EXTRACT_ARGS args;
		parseExtractOption(args, CConfigFile(), nullptr);
		EXPECT_EQ((int)OUTPUT_TO::Desktop, args.extract.OutputDirType);
		EXPECT_EQ(L"", args.extract.OutputDirUserSpecified);
	}
	{
		LF_EXTRACT_ARGS args;
		CMDLINEINFO cmdline;
		cmdline.OutputToOverride = OUTPUT_TO::NoOverride;
		cmdline.OutputDir = L"some_dir";
		parseExtractOption(args, CConfigFile(), &cmdline);
		EXPECT_EQ((int)OUTPUT_TO::Desktop, args.extract.OutputDirType);
		EXPECT_EQ(L"", args.extract.OutputDirUserSpecified);
	}
	{
		LF_EXTRACT_ARGS args;
		CMDLINEINFO cmdline;
		cmdline.OutputToOverride = OUTPUT_TO::SameDir;
		cmdline.OutputDir = L"some_dir";
		parseExtractOption(args, CConfigFile(), &cmdline);
		EXPECT_EQ((int)OUTPUT_TO::SameDir, args.extract.OutputDirType);
		EXPECT_EQ(L"some_dir", args.extract.OutputDirUserSpecified);
	}
	{
		LF_EXTRACT_ARGS args;
		CMDLINEINFO cmdline;
		cmdline.OutputToOverride = OUTPUT_TO::SpecificDir;
		cmdline.OutputDir = L"some_dir";
		parseExtractOption(args, CConfigFile(), &cmdline);
		EXPECT_EQ((int)OUTPUT_TO::SpecificDir, args.extract.OutputDirType);
		EXPECT_EQ(L"some_dir", args.extract.OutputDirUserSpecified);
	}
}

TEST(extract, extractCurrentEntry) {
	_wsetlocale(LC_ALL, L"");	//default locale

	auto tempDir = std::filesystem::path(UtilGetTempPath() / L"test_extractCurrentEntry");
	UtilDeleteDir(tempDir, true);
	EXPECT_FALSE(std::filesystem::exists(tempDir));
	std::filesystem::create_directories(tempDir);
	auto archiveFile = LF_PROJECT_DIR() / L"test/test_extract.zip";
	ASSERT_TRUE(std::filesystem::exists(archiveFile));

	ARCLOG arcLog;
	CLFArchive arc;
	CLFOverwriteConfirmFORCED preExtractHandler(overwrite_options::overwrite);
	auto pp = std::make_shared<CLFPassphraseNULL>();
	EXPECT_NO_THROW(arc.read_open(archiveFile, pp));
	EXPECT_NO_THROW(
		for (auto entry = arc.read_entry_begin(); entry; entry = arc.read_entry_next()) {
			extractCurrentEntry(arc, entry, tempDir, true, arcLog, preExtractHandler,
				CLFProgressHandlerNULL());
		}
	);

	EXPECT_TRUE(std::filesystem::exists(tempDir / L"dirA"));
	EXPECT_TRUE(std::filesystem::exists(tempDir / L"dirA/dirB"));
	EXPECT_TRUE(std::filesystem::exists(tempDir / L"dirA/dirB/file2.txt"));
	EXPECT_TRUE(std::filesystem::exists(tempDir / L"dirA/dirB/dirC"));
	EXPECT_TRUE(std::filesystem::exists(tempDir / L"dirA/dirB/dirC/file1.txt"));
	EXPECT_TRUE(std::filesystem::exists(tempDir / L"かきくけこ"));
	EXPECT_TRUE(std::filesystem::exists(tempDir / L"かきくけこ/file3.txt"));
	EXPECT_TRUE(std::filesystem::exists(tempDir / L"あいうえお.txt"));

	struct _stat64 stat;
	int e = _wstat64((tempDir / L"あいうえお.txt").c_str(), &stat);
	ASSERT_EQ(e, 0);
	EXPECT_EQ(stat.st_mtime, 1589718912ul);

	UtilDeleteDir(tempDir, true);
	EXPECT_FALSE(std::filesystem::exists(tempDir));
}

TEST(extract, extractCurrentEntry_no_restore_filetime) {
	_wsetlocale(LC_ALL, L"");	//default locale

	auto tempDir = std::filesystem::path(UtilGetTempPath() / L"test_extractCurrentEntry");
	UtilDeleteDir(tempDir, true);
	EXPECT_FALSE(std::filesystem::exists(tempDir));
	std::filesystem::create_directories(tempDir);
	auto archiveFile = LF_PROJECT_DIR() / L"test/test_extract.zip";
	ASSERT_TRUE(std::filesystem::exists(archiveFile));

	ARCLOG arcLog;
	CLFArchive arc;
	CLFOverwriteConfirmFORCED preExtractHandler(overwrite_options::overwrite);
	auto pp = std::make_shared<CLFPassphraseNULL>();
	EXPECT_NO_THROW(arc.read_open(archiveFile, pp));
	EXPECT_NO_THROW(
		for (auto entry = arc.read_entry_begin(); entry; entry = arc.read_entry_next()) {
			extractCurrentEntry(arc, entry, tempDir, false/* here! */, arcLog, preExtractHandler,
				CLFProgressHandlerNULL());
		}
	);

	EXPECT_TRUE(std::filesystem::exists(tempDir / L"dirA"));
	EXPECT_TRUE(std::filesystem::exists(tempDir / L"dirA/dirB"));
	EXPECT_TRUE(std::filesystem::exists(tempDir / L"dirA/dirB/file2.txt"));
	EXPECT_TRUE(std::filesystem::exists(tempDir / L"dirA/dirB/dirC"));
	EXPECT_TRUE(std::filesystem::exists(tempDir / L"dirA/dirB/dirC/file1.txt"));
	EXPECT_TRUE(std::filesystem::exists(tempDir / L"かきくけこ"));
	EXPECT_TRUE(std::filesystem::exists(tempDir / L"かきくけこ/file3.txt"));
	EXPECT_TRUE(std::filesystem::exists(tempDir / L"あいうえお.txt"));

	struct _stat64 stat;
	int e = _wstat64((tempDir / L"あいうえお.txt").c_str(), &stat);
	ASSERT_EQ(e, 0);
	EXPECT_GT(stat.st_mtime, 1589718912ul);

	UtilDeleteDir(tempDir, true);
	EXPECT_FALSE(std::filesystem::exists(tempDir));
}


TEST(extract, extractCurrentEntry_broken_files) {
	_wsetlocale(LC_ALL, L"");	//default locale

	const std::vector<std::filesystem::path> files = { L"test_broken_file.zip" , L"test_broken_crc.zip" };

	for (const auto& file : files) {
		auto tempDir = std::filesystem::path(UtilGetTempPath()) / L"test_extractCurrentEntry";
		UtilDeleteDir(tempDir, true);
		EXPECT_FALSE(std::filesystem::exists(tempDir));
		std::filesystem::create_directories(tempDir);
		auto archiveFile = LF_PROJECT_DIR() / L"test" / file;
		ASSERT_TRUE(std::filesystem::exists(archiveFile));

		ARCLOG arcLog;
		CLFArchive arc;
		CLFOverwriteConfirmFORCED preExtractHandler(overwrite_options::overwrite);
		auto pp = std::make_shared<CLFPassphraseNULL>();
		EXPECT_NO_THROW(arc.read_open(archiveFile, pp));
		EXPECT_THROW(
			for (auto entry = arc.read_entry_begin(); entry; entry = arc.read_entry_next()) {
				extractCurrentEntry(arc, entry, tempDir, true, arcLog, preExtractHandler,
					CLFProgressHandlerNULL());
			}
		, LF_EXCEPTION);

		UtilDeleteDir(tempDir, true);
		EXPECT_FALSE(std::filesystem::exists(tempDir));
	}
}


TEST(extract, enumerateOriginalArchives)
{
	std::vector<std::filesystem::path> enumerateOriginalArchives(const std::filesystem::path & original_archive);


	auto tempDir = std::filesystem::path(UtilGetTempPath() / L"test_enumerateOriginalArchives");
	UtilDeleteDir(tempDir, true);
	EXPECT_FALSE(std::filesystem::exists(tempDir));
	std::filesystem::create_directories(tempDir);

	//fake archives
	for (int i = 0; i < 12; i++) {
		if (i % 2 == 0) {
			touchFile(tempDir / Format(L"TEST.PART%d.RAR", i + 1));
		} else {
			touchFile(tempDir / Format(L"test.part%d.rar", i + 1));
		}
	}
	touchFile(tempDir / Format(L"do_not_detect_this.part1.rar"));
	touchFile(tempDir / Format(L"test.part1.rar.txt"));
	touchFile(tempDir / Format(L"test.part1.rar.rar"));
	touchFile(tempDir / Format(L"test2.rar"));
	touchFile(tempDir / Format(L"test3.zip"));

	auto files = enumerateOriginalArchives(tempDir / L"test.part3.rar");
	ASSERT_EQ(12, files.size());
	EXPECT_TRUE(isIn(files, (tempDir / L"TEST.PART5.RAR").make_preferred()));
	EXPECT_TRUE(isIn(files, (tempDir / L"test.part6.rar").make_preferred()));
	EXPECT_TRUE(isIn(files, (tempDir / L"TEST.PART11.RAR").make_preferred()));
	EXPECT_TRUE(isIn(files, (tempDir / L"test.part12.rar").make_preferred()));
	EXPECT_FALSE(isIn(files, (tempDir / L"do_not_detect_this.part1.rar").make_preferred()));
	EXPECT_FALSE(isIn(files, (tempDir / L"test.part1.rar.rar").make_preferred()));

	files = enumerateOriginalArchives(tempDir / L"test.part1.rar.rar");
	ASSERT_EQ(1, files.size());
	EXPECT_TRUE(isIn(files, (tempDir / L"test.part1.rar.rar").make_preferred()));

	files = enumerateOriginalArchives(tempDir / L"test2.rar");
	ASSERT_EQ(1, files.size());
	EXPECT_TRUE(isIn(files, (tempDir / L"test2.rar").make_preferred()));

	files = enumerateOriginalArchives(tempDir / L"test3.zip");
	ASSERT_EQ(1, files.size());
	EXPECT_TRUE(isIn(files, (tempDir / L"test3.zip").make_preferred()));

	UtilDeleteDir(tempDir, true);
}


TEST(extract, testOneArchive) {
	_wsetlocale(LC_ALL, L"");	//default locale
	auto files = { L"test_extract.zip", L"test_extract.zipx", L"test_gzip.gz" };

	for (const auto &file : files) {
		auto archiveFile = LF_PROJECT_DIR() / L"test" / file;
		ASSERT_TRUE(std::filesystem::exists(archiveFile));

		ARCLOG arcLog;
		EXPECT_NO_THROW(
			testOneArchive(archiveFile, arcLog,
				CLFProgressHandlerNULL(),
				std::make_shared<CLFPassphraseNULL>()
			));
	}
}

TEST(extract, testOneArchive_broken_files) {
	_wsetlocale(LC_ALL, L"");	//default locale

	const std::vector<std::filesystem::path> files = { L"test_broken_file.zip" , L"test_broken_crc.zip",__FILEW__ };

	for (const auto& file : files) {
		auto archiveFile = LF_PROJECT_DIR() / L"test" / file;
		ASSERT_TRUE(std::filesystem::exists(archiveFile));

		ARCLOG arcLog;
		EXPECT_THROW(
			testOneArchive(archiveFile, arcLog,
				CLFProgressHandlerNULL(),
				std::make_shared<CLFPassphraseNULL>()
			), LF_EXCEPTION);
	}
}
