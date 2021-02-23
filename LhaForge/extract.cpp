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
#include "Utilities/Utility.h"
#include "CommonUtil.h"


std::wstring trimArchiveName(bool RemoveSymbolAndNumber, const std::wstring& archive_path)
{
	//Symbols to be deleted
	//last two characters are "half-width space" and "full-width space"
	const wchar_t* symbols = L"0123456789./*-+{}[]@`:;!\"#$%&\'()_><=~^|,\\ 　";

	std::filesystem::path an = archive_path;
	std::wstring dirname = an.stem();	//pure filename; no directory path, no extensions

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

//GUICallback(default directory)->output directory
std::wstring determineExtractBaseDir(
	const std::wstring& archive_path,
	LF_EXTRACT_ARGS& args)
{
	args.output_dir_callback.setArchivePath(archive_path);
	auto outputDir = LF_get_output_dir(
		args.extract.OutputDirType,
		archive_path,
		args.extract.OutputDirUserSpecified.c_str(),
		args.output_dir_callback);

	// Warn if output is on network or on a removable disk
	for (;;) {
		if (LF_confirm_output_dir_type(args.general, outputDir)) {
			break;
		} else {
			// Need to change path
			LFShellFileOpenDialog dlg(outputDir.c_str(), FOS_FORCEFILESYSTEM | FOS_FILEMUSTEXIST | FOS_PATHMUSTEXIST | FOS_PICKFOLDERS);
			if (IDOK == dlg.DoModal()) {
				CString tmp;
				dlg.GetFilePath(tmp);
				std::filesystem::path pathOutputDir = tmp.operator LPCWSTR();
				outputDir = pathOutputDir;
				bool keepConfig = (GetKeyState(VK_SHIFT) < 0);	//TODO
				if (keepConfig) {
					args.extract.OutputDirType = OUTPUT_TO_SPECIFIC_DIR;
					args.extract.OutputDirUserSpecified = pathOutputDir.c_str();
				}
			} else {
				CANCEL_EXCEPTION();
			}
		}
	}
	// Confirm to make extract dir if it does not exist
	LF_ask_and_make_sure_output_dir_exists(outputDir, args.general.OnDirNotFound);

	return outputDir;
}

struct PRE_EXTRACT_CHECK {
	bool allInOneDir;	//true if all the contents are under one root directory
	std::wstring baseDirName;	//valid if allInOneDir is true

	PRE_EXTRACT_CHECK() :allInOneDir(false){}
	void check(ILFArchiveFile &arc) {
		bool bFirst = true;

		for (auto entry = arc.read_entry_begin(); entry; entry = arc.read_entry_next()) {
			auto path = LF_sanitize_pathname(entry->path);
			//to remove trailing '/'
			path = UtilPathRemoveLastSeparator(path);
			auto path_components = UtilSplitString(path, L"/");
			if (path_components.empty())continue;

			if (bFirst) {
				this->baseDirName = path_components.front();
				this->allInOneDir = true;
				bFirst = false;
			} else {
				if (0 != _wcsicmp(this->baseDirName.c_str(), path_components.front().c_str())) {
					//another root entry found
					this->allInOneDir = false;
					return;
				}
			}
		}
	}
};

#ifdef UNIT_TEST
#include <gtest/gtest.h>
TEST(extract, PRE_EXTRACT_CHECK) {
	//TODO
}

#endif

std::wstring determineExtractDir(
	ILFArchiveFile& arc,
	const std::wstring& archive_path,
	const std::wstring& output_base_dir,
	const LF_EXTRACT_ARGS& args)
{
	bool needToCreateDir;
	switch (args.extract.CreateDir) {
	case CREATE_OUTPUT_DIR_ALWAYS:
		needToCreateDir = true;
		break;
	case CREATE_OUTPUT_DIR_SINGLE:
		if (args.extract.CreateNoFolderIfSingleFileOnly) {
			PRE_EXTRACT_CHECK result;
			result.check(arc);
			if (result.allInOneDir) {
				needToCreateDir = false;
			} else {
				needToCreateDir = true;
			}
		} else {
			needToCreateDir = true;
		}
		break;
	case CREATE_OUTPUT_DIR_NEVER:
		needToCreateDir = false;
		break;
	}

	if (needToCreateDir) {
		auto subdir = trimArchiveName(args.extract.RemoveSymbolAndNumber, archive_path);
		return std::filesystem::path(output_base_dir) / subdir;
	} else {
		return output_base_dir;
	}
}


//load configuration from file, then overwrites with command line arguments.
void parseExtractOption(LF_EXTRACT_ARGS& args, CConfigManager &mngr, const CMDLINEINFO* lpCmdLineInfo)
{
	args.general.load(mngr);
	args.extract.load(mngr);

	//overwrite with command line arguments
	if (lpCmdLineInfo) {
		if (OUTPUT_TO_DEFAULT != lpCmdLineInfo->OutputToOverride) {
			args.extract.OutputDirType = lpCmdLineInfo->OutputToOverride;
		}
		if (CREATE_OUTPUT_DIR_DEFAULT != lpCmdLineInfo->CreateDirOverride) {
			args.extract.CreateDir = lpCmdLineInfo->CreateDirOverride;
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
			existing.read_file_stat(pathToWrite, pathToWrite);
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

void extractCurrentEntry(
	ILFArchiveFile &arc,
	const LF_ENTRY_STAT *entry,
	const std::wstring& output_dir,
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

	auto outputPath = std::filesystem::path(output_dir) / LF_sanitize_pathname(originalPath);

	//original file size (before compression)
	progressHandler.onNextEntry(outputPath, entry->stat.st_size);
	try {
		if (entry->is_directory()) {
			try {
				std::filesystem::create_directories(outputPath);
				arcLog(originalPath, L"directory created");
			} catch (std::filesystem::filesystem_error&) {
				arcLog(originalPath, L"failed to create directory");
				CString err;	//TODO
				err.Format(IDS_ERROR_CANNOT_MAKE_DIR, outputPath.c_str());
				RAISE_EXCEPTION((LPCWSTR)err);
			}
		} else {
			//overwrite?
			auto decision = preExtractHandler(outputPath, entry);
			switch (decision) {
			case overwrite_options::overwrite:
				//do nothing, keep going
				break;
			case overwrite_options::skip:
				arcLog(originalPath, L"skipped");
				return;
			case overwrite_options::abort:
				//abort
				arcLog(originalPath, L"cancelled by user");
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
			for (;;) {
				auto buffer = arc.read_file_entry_block();
				if (buffer.is_eof()) {
					progressHandler.onEntryIO(entry->stat.st_size);
					break;
				} else {
					auto written = fwrite(buffer.buffer, 1, (size_t)buffer.size, fp);
					if (written != buffer.size) {
						RAISE_EXCEPTION(L"Failed to write file %s", outputPath.c_str());
					}
					progressHandler.onEntryIO(buffer.offset);
				}
			}
			arcLog(originalPath, L"OK");
			fp.close();
		}

		entry->write_file_stat(outputPath);
	} catch (const LF_USER_CANCEL_EXCEPTION& e) {
		arcLog(originalPath, e.what());
		throw e;
	} catch (LF_EXCEPTION &e) {
		arcLog(originalPath, e.what());
		throw e;
	}
}

//enumerate archives to delete
std::vector<std::wstring> enumerateOriginalArchives(const std::wstring& original_archive)
{
	ASSERT(!std::filesystem::is_directory(original_archive));
	if (std::filesystem::is_directory(original_archive))return std::vector<std::wstring>();

	//currently, only rar is supported
	auto rar_pattern = std::wregex(LR"(\.part\d+.*\.rar$)", std::regex_constants::icase);
	if (std::regex_search(original_archive, rar_pattern)) {
		//---RAR
		auto path = std::filesystem::path(original_archive);
		path.make_preferred();
		auto stem = path.stem().stem();

		std::vector<std::wstring> files;
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


bool GUI_extract_multiple_files(
	const std::vector<std::wstring> &archive_files,
	ILFProgressHandler &progressHandler,
	const CMDLINEINFO* lpCmdLineInfo
)
{
	LF_EXTRACT_ARGS args;
	CConfigManager mngr;
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
		std::wstring output_dir;
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
		if (args.general.NotifyShellAfterProcess) {
			//notify shell that output is completed
			::SHChangeNotify(SHCNE_UPDATEDIR, SHCNF_PATH, output_dir.c_str(), NULL);
		}
	}

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
	const std::wstring& archive_path,
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
				for (;;) {
					auto buffer = arc.read_file_entry_block();
					if (buffer.is_eof()) {
						progressHandler.onEntryIO(entry->stat.st_size);
						break;
					} else {
						progressHandler.onEntryIO(buffer.offset);
					}
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


bool GUI_test_multiple_files(
	const std::vector<std::wstring> &archive_files,
	ILFProgressHandler &progressHandler,
	const CMDLINEINFO* lpCmdLineInfo
)
{
	LF_EXTRACT_ARGS args;
	CConfigManager mngr;
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


