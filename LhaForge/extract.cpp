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
#include "Dialogs/ProgressDlg.h"
#include "Dialogs/ConfirmOverwriteDlg.h"
#include "Utilities/Semaphore.h"
#include "Utilities/StringUtil.h"
#include "Utilities/FileOperation.h"
#include "Utilities/OSUtil.h"
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
		args.extract.OutputDirUserSpecified.operator LPCWSTR(),
		args.output_dir_callback);

	// Warn if output is on network or on a removable disk
	for (;;) {
		if (LF_confirm_output_dir_type(args.general, outputDir)) {
			break;
		} else {
			// Need to change path
			CString title(MAKEINTRESOURCE(IDS_INPUT_TARGET_FOLDER_WITH_SHIFT));
			CFolderDialog dlg(NULL, title, BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE);
			if (IDOK == dlg.DoModal()) {
				std::filesystem::path pathOutputDir = dlg.GetFolderPath();
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

	PRE_EXTRACT_CHECK() :allInOneDir(false) {}
	void check(ARCHIVE_FILE_TO_READ &arc) {
		bool bFirst = true;

		for (auto entry = arc.begin(); entry != NULL; entry = arc.next()) {
			std::wstring fname = entry->get_pathname();
			fname = LF_sanitize_pathname(fname);
			TRACE(L"%s\n", fname.c_str());

			auto path_components = UtilSplitString(fname, L"/");
			//to remove trailing '/'
			//TODO: use UtilPathRemoveLastSeparator
			remove_item_if(path_components, [](const std::wstring &s) {return s.empty(); });

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

std::wstring determineExtractDir(
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
			ARCHIVE_FILE_TO_READ a;
			a.read_open(archive_path);	//TODO: exception
			PRE_EXTRACT_CHECK result;
			result.check(a);
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
		if (-1 != lpCmdLineInfo->OutputToOverride) {
			args.extract.OutputDirType = lpCmdLineInfo->OutputToOverride;
		}
		if (-1 != lpCmdLineInfo->CreateDirOverride) {
			args.extract.CreateDir = lpCmdLineInfo->CreateDirOverride;
		}
		if (-1 != lpCmdLineInfo->DeleteAfterProcess) {
			args.extract.DeleteArchiveAfterExtract = lpCmdLineInfo->DeleteAfterProcess;
		}
	}

}

#include "Dialogs/ConfirmOverwriteDlg.h"
overwrite_options confirmOverwrite(
	const std::wstring& extracting_file_path,
	UINT64 extracting_file_size,
	__time64_t extracting_file_mtime,
	const std::wstring& existing_file_path,
	UINT64 existing_file_size,
	__time64_t existing_file_mtime
	)
{
	CConfirmOverwriteDialog dlg;
	dlg.SetFileInfo(
		extracting_file_path,
		extracting_file_size,
		extracting_file_mtime,
		existing_file_path,
		existing_file_size,
		existing_file_mtime
	);
	auto ret = dlg.DoModal();
	switch (ret) {
	case IDC_BUTTON_EXTRACT_OVERWRITE:
		return overwrite_options::overwrite;
	case IDC_BUTTON_EXTRACT_OVERWRITE_ALL:
		return overwrite_options::overwrite_all;
	case IDC_BUTTON_EXTRACT_SKIP:
		return overwrite_options::skip;
	case IDC_BUTTON_EXTRACT_SKIP_ALL:
		return overwrite_options::skip_all;
	case IDC_BUTTON_EXTRACT_ABORT:
	default:
		return overwrite_options::abort;
	}
}

void extractOneArchive(
	const std::wstring& archive_path,
	const std::wstring& output_dir,
	ARCLOG &arcLog,
	std::function<overwrite_options(const std::wstring& fullpath, const LF_ARCHIVE_ENTRY* entry)> preExtractHandler,
	std::function<void(const std::wstring& originalPath, UINT64 currentSize, UINT64 totalSize)> progressHandler
){
	auto defaultDecision = overwrite_options::abort;
	ARCHIVE_FILE_TO_READ arc;
	arc.read_open(archive_path);
	// loop for each entry
	for (LF_ARCHIVE_ENTRY* entry = arc.begin(); entry; entry = arc.next()) {
		//original file name
		std::wstring originalPath = entry->get_pathname();
		//original attributes
		int nAttribute = entry->get_file_mode();
		//original file size (before compression)
		auto llOriginalSize = entry->get_original_filesize();
		//filetime
		auto cFileTime = entry->get_mtime();

		auto outputPath = std::filesystem::path(output_dir) / LF_sanitize_pathname(originalPath);

		progressHandler(outputPath, 0, llOriginalSize);
		try {
			if (entry->is_dir()) {
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
				auto decision = overwrite_options::overwrite;
				if (std::filesystem::exists(outputPath)
					&& std::filesystem::is_regular_file(outputPath)) {
					if (overwrite_options::skip_all == defaultDecision) {
						decision = overwrite_options::skip;
					} else if (overwrite_options::overwrite_all == defaultDecision) {
						decision = overwrite_options::overwrite;
					} else {
						decision = preExtractHandler(outputPath, entry);
					}
				}
				switch (decision) {
				case overwrite_options::overwrite_all:	//FALLTHROUGH
					defaultDecision = decision;
				case overwrite_options::overwrite:
					//do nothing, keep going
					break;
				case overwrite_options::skip_all:	//FALLTHROUGH
					defaultDecision = decision;
				case overwrite_options::skip:
					arcLog(originalPath, L"skipped");
					continue;
					break;
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
					auto buffer = arc.read_block();
					if (buffer.is_eof()) {
						progressHandler(originalPath, llOriginalSize, llOriginalSize);
						break;
					} else {
						progressHandler(originalPath, buffer.offset, llOriginalSize);
						auto written = fwrite(buffer.buffer, 1, buffer.size, fp);
						if (written != buffer.size) {
							RAISE_EXCEPTION(L"Failed to write file %s", outputPath.c_str());
						}
					}
				}
				arcLog(originalPath, L"OK");
				fp.close();
			}
			struct __utimbuf64 ut;
			ut.modtime = entry->get_mtime();
			ut.actime = entry->get_mtime();
			_wutime64(outputPath.c_str(), &ut);
		} catch (LF_EXCEPTION &e) {
			arcLog(originalPath, e.what());
			throw e;
		}
	}
	//end
	arc.close();
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
	const CMDLINEINFO* lpCmdLineInfo
)
{
	//progress bar
	CProgressDialog dlg;
	dlg.Create(NULL);
	dlg.ShowWindow(SW_SHOW);

	LF_EXTRACT_ARGS args;
	CConfigManager mngr;
	try {
		CString strErr;	//TODO: remove this
		if (!mngr.LoadConfig(strErr)) {
			RAISE_EXCEPTION((const wchar_t*)strErr);
		}
		// load configuration, then override them with command line args
		parseExtractOption(args, mngr, lpCmdLineInfo);
	} catch (const LF_EXCEPTION& e) {
		UtilMessageBox(NULL, e.what(), MB_OK | MB_ICONERROR);
		return false;
	}

	UINT64 totalFiles = archive_files.size();
	std::vector<ARCLOG> logs;
	for (const auto &archive_path : archive_files) {
		std::wstring output_dir;
		try {
			//determine output base directory
			auto output_base_dir = determineExtractBaseDir(archive_path, args);

			//output destination directory [could be same as the output base directory]
			output_dir = determineExtractDir(archive_path, output_base_dir, args);

			//make sure output directory exists
			try {
				std::filesystem::create_directories(output_dir);
			} catch (std::filesystem::filesystem_error&) {
				CString err;
				err.Format(IDS_ERROR_CANNOT_MAKE_DIR, output_dir.c_str());
				RAISE_EXCEPTION((LPCWSTR)err);
			}

			// limit concurrent extractions
			CSemaphoreLocker SemaphoreLock;
			if (args.extract.LimitExtractFileCount) {
				const wchar_t* LHAFORGE_EXTRACT_SEMAPHORE_NAME = L"LhaForgeExtractLimitSemaphore";
				SemaphoreLock.Lock(LHAFORGE_EXTRACT_SEMAPHORE_NAME, args.extract.MaxExtractFileCount);
			}

			while (UtilDoMessageLoop())continue;
			std::function<overwrite_options(const std::wstring&, const LF_ARCHIVE_ENTRY*)> preExtractHandler =
				[&](const std::wstring& fullpath, const LF_ARCHIVE_ENTRY* entry)->overwrite_options {
				if (args.extract.ForceOverwrite
					|| !std::filesystem::exists(fullpath)
					|| !std::filesystem::is_regular_file(fullpath)) {
					return overwrite_options::overwrite;
				} else {
					struct _stat64 st;
					_wstat64(fullpath.c_str(), &st);
					return confirmOverwrite(
						std::filesystem::path(entry->get_pathname()).filename().generic_wstring().c_str(),
						entry->get_original_filesize(),
						entry->get_mtime(),
						fullpath,
						st.st_size,
						st.st_mtime
					);
				}
			};
			std::function<void(const std::wstring&, UINT64, UINT64)> progressHandler =
				[&](const std::wstring& originalPath, UINT64 currentSize, UINT64 totalSize)->void {
				dlg.SetProgress(
					archive_path,
					logs.size(),
					totalFiles,
					originalPath,
					currentSize,
					totalSize);
				while (UtilDoMessageLoop())continue;
			};

			logs.resize(logs.size() + 1);
			ARCLOG &arcLog = logs.back();
			// record archive filename
			arcLog.setArchivePath(archive_path);
			extractOneArchive(archive_path, output_dir, arcLog, preExtractHandler, progressHandler);
			arcLog.overallResult = LF_RESULT::OK;
		} catch (const LF_USER_CANCEL_EXCEPTION&) {
			ARCLOG &arcLog = logs.back();
			arcLog.overallResult = LF_RESULT::CANCELED;
			break;
		} catch (const ARCHIVE_EXCEPTION&) {
			ARCLOG &arcLog = logs.back();
			arcLog.overallResult = LF_RESULT::NOTARCHIVE;
			continue;
		} catch (const LF_EXCEPTION&) {
			ARCLOG &arcLog = logs.back();
			arcLog.overallResult = LF_RESULT::NG;
			continue;
		}

		// open output directory
		if (args.extract.OpenDir) {
			if (args.general.Filer.UseFiler) {
				// expand environment
				auto envInfo = LF_make_expand_information(output_dir.c_str(), nullptr);

				// expand command parameter
				auto strCmd = UtilExpandTemplateString((const wchar_t*)args.general.Filer.FilerPath, envInfo);
				auto strParam = UtilExpandTemplateString((const wchar_t*)args.general.Filer.Param, envInfo);
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
		bAllOK = bAllOK && (log.overallResult == LF_RESULT::OK);
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

	// close progress bar
	if (dlg.IsWindow())dlg.DestroyWindow();

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
	std::function<void(const std::wstring& originalPath, UINT64 currentSize, UINT64 totalSize)> progressHandler
) {
	ARCHIVE_FILE_TO_READ arc;
	arc.read_open(archive_path);
	// loop for each entry
	for (LF_ARCHIVE_ENTRY* entry = arc.begin(); entry; entry = arc.next()) {
		//original file name
		std::wstring originalPath = entry->get_pathname();
		//original attributes
		int nAttribute = entry->get_file_mode();
		//original file size (before compression)
		auto llOriginalSize = entry->get_original_filesize();

		progressHandler(originalPath, 0, llOriginalSize);
		try {
			if (entry->is_dir()) {
				arcLog(originalPath, L"directory");
			} else {
				//go
				for (;;) {
					auto buffer = arc.read_block();
					if (buffer.is_eof()) {
						progressHandler(originalPath, llOriginalSize, llOriginalSize);
						break;
					} else {
						progressHandler(originalPath, buffer.offset, llOriginalSize);
					}
				}
				arcLog(originalPath, L"OK");
			}
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
	const CMDLINEINFO* lpCmdLineInfo
)
{
	//progress bar
	CProgressDialog dlg;
	dlg.Create(NULL);
	dlg.ShowWindow(SW_SHOW);

	LF_EXTRACT_ARGS args;
	CConfigManager mngr;
	try {
		CString strErr;	//TODO: remove this
		if (!mngr.LoadConfig(strErr)) {
			RAISE_EXCEPTION((const wchar_t*)strErr);
		}
		// load configuration, then override them with command line args
		parseExtractOption(args, mngr, lpCmdLineInfo);
	} catch (const LF_EXCEPTION& e) {
		UtilMessageBox(NULL, e.what(), MB_OK | MB_ICONERROR);
		return false;
	}

	UINT64 totalFiles = archive_files.size();
	std::vector<ARCLOG> logs;
	for (const auto &archive_path : archive_files) {
		try {
			const wchar_t* LHAFORGE_EXTRACT_SEMAPHORE_NAME = L"LhaForgeExtractLimitSemaphore";
			// limit concurrent extractions
			CSemaphoreLocker SemaphoreLock;
			if (args.extract.LimitExtractFileCount) {
				SemaphoreLock.Lock(LHAFORGE_EXTRACT_SEMAPHORE_NAME, args.extract.MaxExtractFileCount);
			}
			while (UtilDoMessageLoop())continue;
			std::function<void(const std::wstring&, UINT64, UINT64)> progressHandler =
				[&](const std::wstring& originalPath, UINT64 currentSize, UINT64 totalSize)->void {
				dlg.SetProgress(
					archive_path,
					logs.size(),
					totalFiles,
					originalPath,
					currentSize,
					totalSize);
				while (UtilDoMessageLoop())continue;
			};

			logs.resize(logs.size() + 1);
			ARCLOG &arcLog = logs.back();
			// record archive filename
			arcLog.setArchivePath(archive_path);
			testOneArchive(archive_path, arcLog, progressHandler);
			arcLog.overallResult = LF_RESULT::OK;
		} catch (const LF_USER_CANCEL_EXCEPTION &e) {
			ARCLOG &arcLog = logs.back();
			arcLog.overallResult = LF_RESULT::CANCELED;
			break;
		} catch (const ARCHIVE_EXCEPTION& e) {
			ARCLOG &arcLog = logs.back();
			arcLog.overallResult = LF_RESULT::NOTARCHIVE;
			continue;
		} catch (const LF_EXCEPTION &e) {
			ARCLOG &arcLog = logs.back();
			arcLog.overallResult = LF_RESULT::NG;
			continue;
		}
	}

	bool bAllOK = true;
	for (const auto& log : logs) {
		bAllOK = bAllOK && (log.overallResult == LF_RESULT::OK);
	}
	// close progress bar
	if (dlg.IsWindow())dlg.DestroyWindow();

	//---display logs
	CLogListDialog LogDlg(CString(MAKEINTRESOURCE(IDS_LOGINFO_OPERATION_TESTARCHIVE)));
	LogDlg.SetLogArray(logs);
	LogDlg.DoModal(::GetDesktopWindow());

	return bAllOK;
}


