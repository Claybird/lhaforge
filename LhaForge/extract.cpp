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
#include "Dialogs/ConfirmOverwriteDlg.h"
#include "Utilities/Semaphore.h"
#include "Utilities/StringUtil.h"
#include "Utilities/FileOperation.h"
#include "Utilities/OSUtil.h"
#include "Utilities/CustomControl.h"
#include "Utilities/Utility.h"
#include "CommonUtil.h"
#include "CmdLineInfo.h"


std::filesystem::path trimArchiveName(bool RemoveSymbolAndNumber, const std::filesystem::path& archive_path)
{
	//Symbols to be deleted
	//last two characters are "half-width space" and "full-width space"
	const wchar_t* symbols = L"0123456789./*-+{}[]@`:;!\"#$%&\'()_><=~^|,\\ 　";

	std::filesystem::path an = archive_path;
	std::filesystem::path dirname = an.stem();	//pure filename; no directory path, no extensions

	// trims trailing symbols
	if (RemoveSymbolAndNumber) {
		dirname = UtilTrimString(dirname, symbols);
	} else {
		dirname = UtilTrimString(dirname, L".\\ 　");
	}
	//if dirname become empty, restore original
	if (dirname.empty()) {
		dirname = an.stem();
	}

	return dirname;
}
#ifdef UNIT_TEST

TEST(extract, trimArchiveName) {
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
#endif

//GUICallback(default directory)->output directory
std::filesystem::path determineExtractBaseDir(
	const std::filesystem::path& archive_path,
	LF_EXTRACT_ARGS& args)
{
	args.output_dir_callback.setArchivePath(archive_path);
	auto outputDir = LF_get_output_dir(
		(OUTPUT_TO)args.extract.OutputDirType,
		archive_path,
		args.extract.OutputDirUserSpecified.c_str(),
		args.output_dir_callback);

	// Warn if output is on network or on a removable disk
	for (;;) {
		if (LF_confirm_output_dir_type(args.general, outputDir)) {
			break;
		} else {
			// Need to change path
			CLFShellFileOpenDialog dlg(outputDir.c_str(), FOS_FORCEFILESYSTEM | FOS_FILEMUSTEXIST | FOS_PATHMUSTEXIST | FOS_PICKFOLDERS);
			if (IDOK == dlg.DoModal()) {
				CString tmp;
				dlg.GetFilePath(tmp);
				std::filesystem::path pathOutputDir = tmp.operator LPCWSTR();
				outputDir = pathOutputDir;
				bool keepConfig = (GetKeyState(VK_SHIFT) < 0);	//TODO
				if (keepConfig) {
					args.extract.OutputDirType = (int)OUTPUT_TO::SpecificDir;
					args.extract.OutputDirUserSpecified = pathOutputDir.c_str();
				}
			} else {
				CANCEL_EXCEPTION();
			}
		}
	}
	// Confirm to make extract dir if it does not exist
	LF_ask_and_make_sure_output_dir_exists(outputDir, (LOSTDIR)args.general.OnDirNotFound);

	return outputDir;
}

#ifdef UNIT_TEST

TEST(extract, determineExtractBaseDir) {
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
#endif

enum class PRE_EXTRACT_CHECK :int {
	unknown,
	singleDir,	//all the contents are under one root directory
	singleFile,
	multipleEntries,
};

std::tuple<PRE_EXTRACT_CHECK, std::filesystem::path /*baseDirName*/>
preExtractCheck(ILFArchiveFile &arc)
{
	std::filesystem::path baseDirName;
	auto result = PRE_EXTRACT_CHECK::unknown;
	bool bFirst = true;

	for (auto entry = arc.read_entry_begin(); entry; entry = arc.read_entry_next()) {
		auto path = LF_sanitize_pathname(entry->path);
		//to remove trailing '/'
		path = UtilPathRemoveLastSeparator(path);
		auto path_components = UtilSplitString(path, L"/");
		if (path_components.empty())continue;

		if (bFirst) {
			if (entry->is_directory() || path_components.size() > 1) {
				baseDirName = path_components.front();
				result = PRE_EXTRACT_CHECK::singleDir;
			} else {
				result = PRE_EXTRACT_CHECK::singleFile;
			}
			bFirst = false;
		} else {
			if (PRE_EXTRACT_CHECK::singleFile == result) {
				result = PRE_EXTRACT_CHECK::multipleEntries;
				baseDirName.clear();
				break;
			} else if (PRE_EXTRACT_CHECK::singleDir == result) {
				if (0 != _wcsicmp(baseDirName.c_str(), path_components.front().c_str())) {
					//another root entry found
					result = PRE_EXTRACT_CHECK::multipleEntries;
					baseDirName.clear();
					break;
				}
			}
		}
	}
	return { result,baseDirName };
}

#ifdef UNIT_TEST
TEST(extract, preExtractCheck) {
	{
		auto [result, baseDirName] = preExtractCheck(CLFArchiveNULL());
		EXPECT_EQ(PRE_EXTRACT_CHECK::unknown, result);
	}
	{
		CLFArchive a;
		CLFPassphraseNULL pp;
		a.read_open(LF_PROJECT_DIR() / L"test/test_extract.zip", pp);
		auto [result, baseDirName] = preExtractCheck(a);
		EXPECT_EQ(PRE_EXTRACT_CHECK::multipleEntries, result);
	}
	{
		CLFArchive a;
		CLFPassphraseNULL pp;
		a.read_open(LF_PROJECT_DIR() / L"test/test_extract.zipx", pp);
		auto [result, baseDirName] = preExtractCheck(a);
		EXPECT_EQ(PRE_EXTRACT_CHECK::multipleEntries, result);
	}
	{
		CLFArchive a;
		CLFPassphraseNULL pp;
		a.read_open(LF_PROJECT_DIR() / L"test/test_gzip.gz", pp);
		auto [result, baseDirName] = preExtractCheck(a);
		EXPECT_EQ(PRE_EXTRACT_CHECK::singleFile, result);
	}
	{
		CLFArchive a;
		CLFPassphraseNULL pp;
		a.read_open(LF_PROJECT_DIR() / L"test/test.lzh", pp);
		auto [result, baseDirName] = preExtractCheck(a);
		EXPECT_EQ(PRE_EXTRACT_CHECK::singleFile, result);
	}
	{
		CLFArchive a;
		CLFPassphraseNULL pp;
		a.read_open(LF_PROJECT_DIR() / L"test/test.tar.gz", pp);
		auto [result, baseDirName] = preExtractCheck(a);
		EXPECT_EQ(PRE_EXTRACT_CHECK::singleDir, result);
		EXPECT_EQ(L"test", baseDirName);
	}
}

#endif

std::filesystem::path determineExtractDir(
	ILFArchiveFile& arc,
	const std::filesystem::path& archive_path,
	const std::filesystem::path& output_base_dir,
	const LF_EXTRACT_ARGS& args)
{
	bool needToCreateDir;
	switch ((EXTRACT_CREATE_DIR)args.extract.CreateDir) {
	case EXTRACT_CREATE_DIR::Never:
		needToCreateDir = false;
		break;
	case EXTRACT_CREATE_DIR::SkipIfSingleFileOrDir:
	{
		auto [result, baseDirName] = preExtractCheck(arc);
		if (PRE_EXTRACT_CHECK::multipleEntries != result) {
			needToCreateDir = false;
		} else {
			needToCreateDir = true;
		}
		break;
	}
	case EXTRACT_CREATE_DIR::SkipIfSingleDirectory:
	{
		auto [result, baseDirName] = preExtractCheck(arc);
		if (PRE_EXTRACT_CHECK::singleDir == result) {
			needToCreateDir = false;
		} else {
			needToCreateDir = true;
		}
		break;
	}
	case EXTRACT_CREATE_DIR::Always:
	default:
		needToCreateDir = true;
		break;
	}

	if (needToCreateDir) {
		auto subdir = trimArchiveName(args.extract.RemoveSymbolAndNumber, archive_path);
		return output_base_dir / subdir;
	} else {
		return output_base_dir;
	}
}

#ifdef UNIT_TEST
TEST(extract, determineExtractDir) {
	LF_EXTRACT_ARGS fakeArg;
	fakeArg.load(CConfigFile());
	{
		fakeArg.extract.CreateDir = (int)EXTRACT_CREATE_DIR::Never;
		fakeArg.extract.RemoveSymbolAndNumber = false;
		CLFArchiveNULL arc;
		CLFPassphraseNULL pp;
		arc.read_open(L"path_to_archive/archive.ext", pp);
		EXPECT_EQ(L"path_to_output",
			determineExtractDir(arc, L"path_to_archive/archive.ext", L"path_to_output", fakeArg));

		arc.read_open(L"path_to_archive/archive  .ext", pp);
		EXPECT_EQ(L"path_to_output",
			determineExtractDir(arc, L"path_to_archive/archive   .ext", L"path_to_output", fakeArg));
	}

	{
		CLFArchiveNULL arc;
		fakeArg.extract.CreateDir = (int)EXTRACT_CREATE_DIR::Always;
		CLFPassphraseNULL pp;
		arc.read_open(L"path_to_archive/archive.ext", pp);
		EXPECT_EQ(L"path_to_output/archive",
			determineExtractDir(arc, L"path_to_archive/archive.ext", L"path_to_output", fakeArg));
		arc.read_open(L"path_to_archive/archive  .ext", pp);
		EXPECT_EQ(L"path_to_output/archive",
			determineExtractDir(arc, L"path_to_archive/archive  .ext", L"path_to_output", fakeArg));
	}

	//---
	{
		CLFArchive arc;
		fakeArg.extract.CreateDir = (int)EXTRACT_CREATE_DIR::SkipIfSingleFileOrDir;
		CLFPassphraseNULL pp;
		arc.read_open(LF_PROJECT_DIR() / L"test/test.tar.gz", pp);
		EXPECT_EQ(L"path_to_output",
			determineExtractDir(arc, LF_PROJECT_DIR() / L"test/test.tar.gz", L"path_to_output", fakeArg));
	}
	{
		CLFArchive arc;
		fakeArg.extract.CreateDir = (int)EXTRACT_CREATE_DIR::SkipIfSingleDirectory;
		CLFPassphraseNULL pp;
		arc.read_open(LF_PROJECT_DIR() / L"test/test.tar.gz", pp);
		EXPECT_EQ(L"path_to_output",
			determineExtractDir(arc, LF_PROJECT_DIR() / L"test/test.tar.gz", L"path_to_output", fakeArg));
	}
	//---
	{
		CLFArchive arc;
		fakeArg.extract.CreateDir = (int)EXTRACT_CREATE_DIR::SkipIfSingleFileOrDir;
		CLFPassphraseNULL pp;
		arc.read_open(LF_PROJECT_DIR() / L"test/test.lzh", pp);
		EXPECT_EQ(L"path_to_output",
			determineExtractDir(arc, LF_PROJECT_DIR() / L"test/test.lzh", L"path_to_output", fakeArg));
	}
	{
		CLFArchive arc;
		fakeArg.extract.CreateDir = (int)EXTRACT_CREATE_DIR::SkipIfSingleDirectory;
		CLFPassphraseNULL pp;
		arc.read_open(LF_PROJECT_DIR() / L"test/test.lzh", pp);
		EXPECT_EQ(L"path_to_output/test",
			determineExtractDir(arc, LF_PROJECT_DIR() / L"test/test.lzh", L"path_to_output", fakeArg));
	}
	//---
	{
		CLFArchive arc;
		fakeArg.extract.CreateDir = (int)EXTRACT_CREATE_DIR::SkipIfSingleFileOrDir;
		CLFPassphraseNULL pp;
		arc.read_open(LF_PROJECT_DIR() / L"test/test_extract.zip", pp);
		EXPECT_EQ(L"path_to_output/test_extract",
			determineExtractDir(arc, LF_PROJECT_DIR() / L"test/test_extract.zip", L"path_to_output", fakeArg));
	}
	{
		CLFArchive arc;
		fakeArg.extract.CreateDir = (int)EXTRACT_CREATE_DIR::SkipIfSingleDirectory;
		CLFPassphraseNULL pp;
		arc.read_open(LF_PROJECT_DIR() / L"test/test_extract.zip", pp);
		EXPECT_EQ(L"path_to_output/test_extract",
			determineExtractDir(arc, LF_PROJECT_DIR() / L"test/test_extract.zip", L"path_to_output", fakeArg));
	}
}
#endif

//load configuration from file, then overwrites with command line arguments.
void parseExtractOption(LF_EXTRACT_ARGS& args, CConfigFile &mngr, const CMDLINEINFO* lpCmdLineInfo)
{
	args.load(mngr);

	//overwrite with command line arguments
	if (lpCmdLineInfo) {
		if (OUTPUT_TO::NoOverride != lpCmdLineInfo->OutputToOverride) {
			args.extract.OutputDirType = (int)lpCmdLineInfo->OutputToOverride;
		}
		if (EXTRACT_CREATE_DIR::NoOverride != lpCmdLineInfo->CreateDirOverride) {
			args.extract.CreateDir = (int)lpCmdLineInfo->CreateDirOverride;
		}
		if (CMDLINEINFO::ACTION::Default != lpCmdLineInfo->DeleteAfterProcess) {
			if (CMDLINEINFO::ACTION::False == lpCmdLineInfo->DeleteAfterProcess) {
				args.extract.DeleteArchiveAfterExtract = false;
			} else {
				args.extract.DeleteArchiveAfterExtract = true;
			}
		}
	}
}

#include "Dialogs/ConfirmOverwriteDlg.h"
overwrite_options CLFOverwriteConfirmGUI::operator()(const std::filesystem::path& pathToWrite, const LF_ENTRY_STAT* entry)
{
	if (std::filesystem::exists(pathToWrite)
		&& std::filesystem::is_regular_file(pathToWrite)) {
		if (defaultDecision == overwrite_options::not_defined) {
			//file exists. overwrite?

			//existing file
			LF_ENTRY_STAT existing;
			existing.read_stat(pathToWrite, pathToWrite);
			CConfirmOverwriteDialog dlg;
			dlg.SetFileInfo(
				entry->path,entry->stat.st_size,entry->stat.st_mtime,
				existing.path,existing.stat.st_size,existing.stat.st_mtime
			);
			auto ret = dlg.DoModal();
			switch (ret) {
			case IDC_BUTTON_EXTRACT_OVERWRITE:
				return overwrite_options::overwrite;
			case IDC_BUTTON_EXTRACT_OVERWRITE_ALL:
				defaultDecision = overwrite_options::overwrite;
				return overwrite_options::overwrite;
			case IDC_BUTTON_EXTRACT_SKIP:
				return overwrite_options::skip;
			case IDC_BUTTON_EXTRACT_SKIP_ALL:
				defaultDecision = overwrite_options::skip;
				return overwrite_options::skip;
			case IDC_BUTTON_EXTRACT_ABORT:
			default:
				return overwrite_options::abort;
			}
		} else {
			return defaultDecision;
		}
	} else {
		return overwrite_options::overwrite;
	}
}

std::filesystem::path extractCurrentEntry(
	ILFArchiveFile &arc,
	const LF_ENTRY_STAT *entry,
	const std::filesystem::path& output_dir,
	ARCLOG &arcLog,
	ILFOverwriteConfirm& preExtractHandler,
	ILFProgressHandler& progressHandler
) {
	//original file name
	auto originalPath = entry->path;
	//original attributes
	int nAttribute = entry->stat.st_mode;
	//filetime
	auto cFileTime = entry->stat.st_mtime;

	auto outputPath = output_dir / LF_sanitize_pathname(originalPath);

	//original file size (before compression)
	progressHandler.onNextEntry(outputPath, entry->stat.st_size);
	bool created = false;
	try {
		if (entry->is_directory()) {
			try {
				std::filesystem::create_directories(outputPath);
				arcLog(outputPath, L"directory created");
			} catch (std::filesystem::filesystem_error&) {
				arcLog(outputPath, L"failed to create directory");
				RAISE_EXCEPTION(Format(UtilLoadString(IDS_ERROR_CANNOT_MAKE_DIR), outputPath.c_str()));
			}
		} else {
			//overwrite?
			auto decision = preExtractHandler(outputPath, entry);
			switch (decision) {
			case overwrite_options::overwrite:
				//do nothing, keep going
				break;
			case overwrite_options::skip:
				arcLog(outputPath, L"skipped");
				return {};
			case overwrite_options::abort:
				//abort
				CANCEL_EXCEPTION();
				break;
			}

			{
				auto parent = std::filesystem::path(outputPath).parent_path();
				if (!std::filesystem::exists(parent)) {
					//in case directory entry is not in archive
					std::filesystem::create_directories(parent);
					arcLog(parent, L"directory created");
				}
			}

			//go
			CAutoFile fp;
			fp.open(outputPath, L"wb");
			if (!fp.is_opened()) {
				arcLog(originalPath, L"failed to open for write");
				RAISE_EXCEPTION(L"Failed to open file %s", outputPath.c_str());
			}
			created = true;
			for (bool bEOF = false;!bEOF;) {
				arc.read_file_entry_block([&](const void* buf, int64_t data_size, const offset_info* offset) {
					if (!buf || data_size == 0) {
						progressHandler.onEntryIO(entry->stat.st_size);
						bEOF = true;
					} else {
						if (offset && _ftelli64(fp) != offset->offset) {
							_fseeki64(fp, offset->offset, SEEK_SET);
						}
						auto written = fwrite(buf, 1, data_size, fp);
						if (written != data_size) {
							RAISE_EXCEPTION(L"Failed to write file %s", outputPath.c_str());
						}
						progressHandler.onEntryIO(_ftelli64(fp));
					}
				});
			}
			arcLog(outputPath, L"OK");
			fp.close();
		}

		entry->write_stat(outputPath);
		return outputPath;
	} catch (const LF_USER_CANCEL_EXCEPTION& e) {
		arcLog(outputPath, e.what());
		if (created) {
			UtilDeletePath(originalPath);
		}
		throw;
	} catch (LF_EXCEPTION &e) {
		arcLog(outputPath, e.what());
		if (created) {
			UtilDeletePath(originalPath);
		}
		throw;
	}
}

#ifdef UNIT_TEST
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
	CLFPassphraseNULL pp;
	EXPECT_NO_THROW(arc.read_open(archiveFile, pp));
	EXPECT_NO_THROW(
		for (auto entry = arc.read_entry_begin(); entry; entry = arc.read_entry_next()) {
			extractCurrentEntry(arc, entry, tempDir, arcLog, preExtractHandler,
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
		CLFPassphraseNULL pp;
		EXPECT_NO_THROW(arc.read_open(archiveFile, pp));
		EXPECT_THROW(
			for (auto entry = arc.read_entry_begin(); entry; entry = arc.read_entry_next()) {
				extractCurrentEntry(arc, entry, tempDir, arcLog, preExtractHandler,
					CLFProgressHandlerNULL());
			}
		, LF_EXCEPTION);

		UtilDeleteDir(tempDir, true);
		EXPECT_FALSE(std::filesystem::exists(tempDir));
	}
}
#endif

//enumerate archives to delete
std::vector<std::filesystem::path> enumerateOriginalArchives(const std::filesystem::path& original_archive)
{
	ASSERT(!std::filesystem::is_directory(original_archive));
	if (std::filesystem::is_directory(original_archive))return {};

	//currently, only rar is supported
	auto rar_pattern = std::wregex(LR"(\.part\d+.*\.rar$)", std::regex_constants::icase);
	if (std::regex_search(original_archive.wstring(), rar_pattern)) {
		//---RAR
		auto path = std::filesystem::path(original_archive);
		path.make_preferred();
		auto stem = path.stem().stem();

		std::vector<std::filesystem::path> files;
		for (const auto& entry : std::filesystem::directory_iterator(path.parent_path())) {
			auto p = entry.path();
			p.make_preferred();
			if (std::regex_search(p.wstring(), rar_pattern)) {
				if (0==_wcsicmp(p.stem().stem().c_str(),stem.c_str())) {
					files.push_back(p);
				}
			}
		}
		return files;
	} else {
		return { original_archive };
	}
}

#ifdef UNIT_TEST
TEST(extract, enumerateOriginalArchives)
{
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
#endif

bool GUI_extract_multiple_files(
	const std::vector<std::filesystem::path> &archive_files,
	ILFProgressHandler &progressHandler,
	const CMDLINEINFO* lpCmdLineInfo
)
{
	LF_EXTRACT_ARGS args;
	CConfigFile mngr;
	try {
		mngr.load();
		// load configuration, then override them with command line args
		parseExtractOption(args, mngr, lpCmdLineInfo);
	} catch (const LF_EXCEPTION& e) {
		UtilMessageBox(NULL, e.what(), MB_OK | MB_ICONERROR);
		return false;
	}

	UINT64 totalFiles = archive_files.size();
	//TODO: queueDialog
	std::vector<ARCLOG> logs;
	for (const auto &archive_path : archive_files) {
		progressHandler.reset();
		progressHandler.setArchive(archive_path);
		std::filesystem::path output_dir;
		try {
			//determine output base directory
			auto output_base_dir = determineExtractBaseDir(archive_path, args);

			CLFArchive arc;
			arc.read_open(archive_path, CLFPassphraseGUI());
			progressHandler.setNumEntries(arc.get_num_entries());

			//output destination directory [could be same as the output base directory]
			output_dir = determineExtractDir(arc, archive_path, output_base_dir, args);

			//make sure output directory exists
			try {
				std::filesystem::create_directories(output_dir);
			} catch (std::filesystem::filesystem_error&) {
				RAISE_EXCEPTION(UtilLoadString(IDS_ERROR_CANNOT_MAKE_DIR).c_str(), output_dir.c_str());
			}

			// limit concurrent extractions
			CSemaphoreLocker SemaphoreLock;
			if (args.extract.LimitExtractFileCount) {
				const wchar_t* LHAFORGE_EXTRACT_SEMAPHORE_NAME = L"LhaForgeExtractLimitSemaphore";
				SemaphoreLock.Lock(LHAFORGE_EXTRACT_SEMAPHORE_NAME, args.extract.MaxExtractFileCount);
			}

			logs.resize(logs.size() + 1);
			ARCLOG &arcLog = logs.back();
			// record archive filename
			arcLog.setArchivePath(archive_path);

			CLFOverwriteConfirmGUI preExtractHandler;
			// loop for each entry
			for (auto entry = arc.read_entry_begin(); entry; entry = arc.read_entry_next()) {
				extractCurrentEntry(arc, entry, output_dir, arcLog, preExtractHandler, progressHandler);
			}
			//end
			arc.close();
		} catch (const LF_USER_CANCEL_EXCEPTION& e) {
			ARCLOG &arcLog = logs.back();
			arcLog.logException(e);
			break;
		} catch (const ARCHIVE_EXCEPTION& e) {
			ARCLOG &arcLog = logs.back();
			arcLog.logException(e);
			continue;
		} catch (const LF_EXCEPTION& e) {
			ARCLOG &arcLog = logs.back();
			arcLog.logException(e);
			continue;
		}

		// open output directory
		if (args.extract.OpenDir) {
			if (args.general.Filer.UseFiler) {
				// expand environment
				auto envInfo = LF_make_expand_information(output_dir.c_str(), nullptr);

				// expand command parameter
				auto strCmd = UtilExpandTemplateString(args.general.Filer.FilerPath, envInfo);
				auto strParam = UtilExpandTemplateString(args.general.Filer.Param, envInfo);
				ShellExecuteW(NULL, L"open", strCmd.c_str(), strParam.c_str(), NULL, SW_SHOWNORMAL);
			} else {
				//open with explorer
				UtilNavigateDirectory(output_dir);
			}
		}

		// delete archive or move it to recycle bin
		if (args.extract.DeleteArchiveAfterExtract) {
			auto original_files = enumerateOriginalArchives(archive_path);
			LF_deleteOriginalArchives(args.extract.MoveToRecycleBin, args.extract.DeleteNoConfirm, original_files);
		}
		//notify shell that output is completed
		::SHChangeNotify(SHCNE_UPDATEDIR, SHCNF_PATH, output_dir.c_str(), NULL);
	}

	bool bAllOK = true;
	for (const auto& log : logs) {
		bAllOK = bAllOK && (log._overallResult == LF_RESULT::OK);
	}
	//---display logs
	bool displayLog = false;
	switch ((LOGVIEW)args.general.LogViewEvent) {
	case LOGVIEW::OnError:
		if (!bAllOK) {
			displayLog = true;
		}
		break;
	case LOGVIEW::Always:
		displayLog = true;
		break;
	}

	progressHandler.end();

	if (displayLog) {
		CLogListDialog LogDlg(CString(MAKEINTRESOURCE(IDS_LOGINFO_OPERATION_EXTRACT)));
		LogDlg.SetLogArray(logs);
		LogDlg.DoModal(::GetDesktopWindow());
	}

	return bAllOK;
}

//------

//test an archive by reading whole archive
void testOneArchive(
	const std::filesystem::path& archive_path,
	ARCLOG &arcLog,
	ILFProgressHandler &progressHandler,
	ILFPassphrase &passphrase_callback
) {
	CLFArchive arc;
	progressHandler.reset();
	progressHandler.setArchive(archive_path);
	arc.read_open(archive_path, passphrase_callback);
	progressHandler.setNumEntries(arc.get_num_entries());
	// loop for each entry
	for (auto* entry = arc.read_entry_begin(); entry; entry = arc.read_entry_next()) {
		//original file name
		auto originalPath = entry->path;
		//original attributes
		int nAttribute = entry->stat.st_mode;
		//original file size (before compression)
		progressHandler.onNextEntry(originalPath, entry->stat.st_size);

		try {
			if (entry->is_directory()) {
				arcLog(originalPath, L"directory");
			} else {
				//go
				size_t global_offset = 0;
				for (bool bEOF = false; !bEOF;) {
					arc.read_file_entry_block([&](const void* buf, int64_t data_size, const offset_info* offset) {
						if (!buf || data_size == 0) {
							progressHandler.onEntryIO(entry->stat.st_size);
							bEOF = true;
						} else {
							global_offset += data_size;
							if (offset && offset->offset != global_offset) {
								global_offset = offset->offset;
							}
							progressHandler.onEntryIO(global_offset);
						}
					});
				}
				arcLog(originalPath, L"OK");
			}
		} catch (const LF_USER_CANCEL_EXCEPTION& e) {
			arcLog(originalPath, e.what());
			throw e;
		} catch (LF_EXCEPTION &e) {
			arcLog(originalPath, e.what());
			throw e;
		}
	}
	//end
	arc.close();
}

#ifdef UNIT_TEST
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
				CLFPassphraseNULL()
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
				CLFPassphraseNULL()
			), LF_EXCEPTION);
	}
}

#endif



bool GUI_test_multiple_files(
	const std::vector<std::filesystem::path> &archive_files,
	ILFProgressHandler &progressHandler,
	const CMDLINEINFO* lpCmdLineInfo
)
{
	LF_EXTRACT_ARGS args;
	CConfigFile mngr;
	try {
		mngr.load();
		// load configuration, then override them with command line args
		parseExtractOption(args, mngr, lpCmdLineInfo);
	} catch (const LF_EXCEPTION& e) {
		UtilMessageBox(NULL, e.what(), MB_OK | MB_ICONERROR);
		return false;
	}

	UINT64 totalFiles = archive_files.size();
	//TODO: queue
	std::vector<ARCLOG> logs;
	for (const auto &archive_path : archive_files) {
		try {
			const wchar_t* LHAFORGE_EXTRACT_SEMAPHORE_NAME = L"LhaForgeExtractLimitSemaphore";
			// limit concurrent extractions
			CSemaphoreLocker SemaphoreLock;
			if (args.extract.LimitExtractFileCount) {
				SemaphoreLock.Lock(LHAFORGE_EXTRACT_SEMAPHORE_NAME, args.extract.MaxExtractFileCount);
			}
			logs.resize(logs.size() + 1);
			ARCLOG &arcLog = logs.back();
			// record archive filename
			arcLog.setArchivePath(archive_path);
			testOneArchive(archive_path, arcLog, progressHandler, CLFPassphraseGUI());
		} catch (const LF_USER_CANCEL_EXCEPTION &e) {
			ARCLOG &arcLog = logs.back();
			arcLog.logException(e);
			break;
		} catch (const ARCHIVE_EXCEPTION& e) {
			ARCLOG &arcLog = logs.back();
			arcLog.logException(e);
			continue;
		} catch (const LF_EXCEPTION &e) {
			ARCLOG &arcLog = logs.back();
			arcLog.logException(e);
			continue;
		}
	}

	bool bAllOK = true;
	for (const auto& log : logs) {
		bAllOK = bAllOK && (log._overallResult == LF_RESULT::OK);
	}
	progressHandler.end();

	//---display logs
	CLogListDialog LogDlg(CString(MAKEINTRESOURCE(IDS_LOGINFO_OPERATION_TESTARCHIVE)));
	LogDlg.SetLogArray(logs);
	LogDlg.DoModal(::GetDesktopWindow());

	return bAllOK;
}


