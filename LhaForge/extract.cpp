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
			CLFShellFileSaveDialog dlg(outputDir.c_str(), FOS_FORCEFILESYSTEM | FOS_FILEMUSTEXIST | FOS_PATHMUSTEXIST | FOS_PICKFOLDERS);
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

std::tuple<PRE_EXTRACT_CHECK, std::filesystem::path /*baseDirName*/>
preExtractCheck(ILFArchiveFile &arc, ILFScanProgressHandler& progressHandler)
{
	progressHandler.setArchive(arc.get_archive_path());

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
		//notifier
		progressHandler.onNextEntry(entry->path);
	}
	return { result,baseDirName };
}

std::filesystem::path determineExtractDir(
	ILFArchiveFile& arc,
	ILFScanProgressHandler& progress,
	const std::filesystem::path& archive_path,
	const std::filesystem::path& output_base_dir,
	const LF_EXTRACT_ARGS& args)
{
	bool needToCreateDir = false;
	switch ((EXTRACT_CREATE_DIR)args.extract.CreateDir) {
	case EXTRACT_CREATE_DIR::Never:
		needToCreateDir = false;
		break;
	case EXTRACT_CREATE_DIR::SkipIfSingleFileOrDir:
	{
		auto [result, baseDirName] = preExtractCheck(arc, progress);
		if (PRE_EXTRACT_CHECK::multipleEntries != result) {
			needToCreateDir = false;
		} else {
			needToCreateDir = true;
		}
		break;
	}
	case EXTRACT_CREATE_DIR::SkipIfSingleDirectory:
	{
		auto [result, baseDirName] = preExtractCheck(arc, progress);
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

//load configuration from file, then overwrites with command line arguments.
void parseExtractOption(LF_EXTRACT_ARGS& args, CConfigFile &mngr, const CMDLINEINFO* lpCmdLineInfo)
{
	args.load(mngr);

	//overwrite with command line arguments
	if (lpCmdLineInfo) {
		if (OUTPUT_TO::NoOverride != lpCmdLineInfo->OutputToOverride) {
			args.extract.OutputDirType = (int)lpCmdLineInfo->OutputToOverride;
			args.extract.OutputDirUserSpecified = lpCmdLineInfo->OutputDir;
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

std::filesystem::path extractCurrentEntry(
	ILFArchiveFile &arc,
	const LF_ENTRY_STAT *entry,
	const std::filesystem::path& output_dir,
	bool RestoreFileTime,
	ARCLOG &arcLog,
	ILFOverwriteConfirm& preExtractHandler,
	ILFProgressHandler& progressHandler
) {
	std::filesystem::path outputPath = output_dir / LF_sanitize_pathname(entry->path);

	//original file size (before compression)
	progressHandler.onNextEntry(outputPath, entry->stat.st_size);
	bool created = false;
	try {
		if (entry->is_directory() || !entry->path.has_filename()) {
			try {
				std::filesystem::create_directories(outputPath);
				arcLog(outputPath, UtilLoadString(IDS_ARCLOG_MKDIR));
			} catch (std::filesystem::filesystem_error&) {
				arcLog(outputPath, UtilLoadString(IDS_ARCLOG_MKDIR_FAIL));
				RAISE_EXCEPTION(Format(UtilLoadString(IDS_ERROR_MKDIR), outputPath.c_str()));
			}
		} else {
			//overwrite?
			auto decision = preExtractHandler(outputPath, entry);
			switch (decision) {
			case overwrite_options::overwrite:
				//do nothing, keep going
				break;
			case overwrite_options::skip:
				arcLog(outputPath, UtilLoadString(IDS_ARCLOG_SKIP));
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
					arcLog(parent, UtilLoadString(IDS_ARCLOG_MKDIR));
				}
			}

			//go
			CAutoFile fp;
			fp.open(outputPath, L"wb");
			if (!fp.is_opened()) {
				arcLog(outputPath, UtilLoadString(IDS_ARCLOG_ERROR_WRITE));
				RAISE_EXCEPTION(UtilLoadString(IDS_ERROR_OPEN_FILE), outputPath.c_str());
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
						auto written = fwrite(buf, 1, (size_t)data_size, fp);
						if (written != data_size) {
							RAISE_EXCEPTION(UtilLoadString(IDS_ERROR_WRITE_FILE), outputPath.c_str());
						}
						progressHandler.onEntryIO(_ftelli64(fp));
					}
				});
			}
			arcLog(outputPath, UtilLoadString(IDS_ARCLOG_OK));
			fp.close();
		}

		if (RestoreFileTime) {
			entry->write_stat(outputPath);
		}
		return outputPath;
	} catch (const LF_USER_CANCEL_EXCEPTION& e) {
		arcLog(outputPath, e.what());
		if (created) {
			UtilDeletePath(outputPath);
		}
		throw;
	} catch (LF_EXCEPTION &e) {
		arcLog(outputPath, e.what());
		if (created) {
			UtilDeletePath(outputPath);
		}
		throw;
	}
}

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
			arc.read_open(archive_path, std::make_shared<CLFPassphraseGUI>());

			//output destination directory [could be same as the output base directory]
			{
				CLFScanProgressHandlerGUI progress(NULL);
				output_dir = determineExtractDir(arc, progress, archive_path, output_base_dir, args);
				progressHandler.setNumEntries(arc.get_num_entries());
			}

			if (std::filesystem::exists(output_dir) && std::filesystem::is_regular_file(output_dir)) {
				CLFShellFileSaveDialog dlg(output_dir.c_str(), FOS_FORCEFILESYSTEM | FOS_FILEMUSTEXIST | FOS_PATHMUSTEXIST | FOS_PICKFOLDERS);
				if (IDOK == dlg.DoModal()) {
					CString tmp;
					dlg.GetFilePath(tmp);
					std::filesystem::path pathOutputDir = tmp.operator LPCWSTR();
					output_dir = pathOutputDir;
				} else {
					CANCEL_EXCEPTION();
				}
			}
			//make sure output directory exists
			try {
				std::filesystem::create_directories(output_dir);
			} catch (std::filesystem::filesystem_error&) {
				RAISE_EXCEPTION(UtilLoadString(IDS_ERROR_MKDIR).c_str(), output_dir.c_str());
			}


			logs.resize(logs.size() + 1);
			ARCLOG& arcLog = logs.back();
			// record archive filename
			arcLog.setArchivePath(archive_path);
			progressHandler.setArchive(archive_path);

			// limit concurrent extractions
			CSemaphoreLocker SemaphoreLock;
			if (args.extract.LimitExtractFileCount) {
				const wchar_t* LHAFORGE_EXTRACT_SEMAPHORE_NAME = L"LhaForgeExtractLimitSemaphore";
				SemaphoreLock.Create(LHAFORGE_EXTRACT_SEMAPHORE_NAME, args.extract.MaxExtractFileCount);
				SemaphoreLock.Lock(INFINITE);
				//Wait for semaphore lock
				//progress dialog shows waiting message
				progressHandler.setSpecialMessage(UtilLoadString(IDS_WAITING_FOR_SEMAPHORE));
				for (; !SemaphoreLock.Lock(20);) {
					while (UtilDoMessageLoop())continue;
					progressHandler.poll();	//needed to detect cancel
					Sleep(20);
				}
			}

			CLFOverwriteConfirmGUI preExtractHandler;
			// loop for each entry
			for (auto entry = arc.read_entry_begin(); entry; entry = arc.read_entry_next()) {
				extractCurrentEntry(arc, entry, output_dir, args.extract.RestoreFileTime, arcLog, preExtractHandler, progressHandler);
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
	std::shared_ptr<ILFPassphrase> passphrase_callback
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
				arcLog(originalPath, UtilLoadString(IDS_ARCLOG_ENTRY_IS_DIR));
			} else {
				//go
				int64_t global_offset = 0;
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
				arcLog(originalPath, UtilLoadString(IDS_ARCLOG_OK));
			}
		} catch (const LF_USER_CANCEL_EXCEPTION& e) {
			arcLog(originalPath, e.what());
			throw e;
		} catch (const LF_EXCEPTION &e) {
			arcLog(originalPath, e.what());
			throw e;
		}
	}
	//end
	arc.close();
}


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
	std::vector<ARCLOG> logs;
	for (const auto &archive_path : archive_files) {
		try {
			logs.resize(logs.size() + 1);
			ARCLOG& arcLog = logs.back();
			// record archive filename
			arcLog.setArchivePath(archive_path);
			progressHandler.setArchive(archive_path);

			const wchar_t* LHAFORGE_EXTRACT_SEMAPHORE_NAME = L"LhaForgeExtractLimitSemaphore";
			// limit concurrent extractions
			CSemaphoreLocker SemaphoreLock;
			if (args.extract.LimitExtractFileCount) {
				SemaphoreLock.Create(LHAFORGE_EXTRACT_SEMAPHORE_NAME, args.extract.MaxExtractFileCount);
				SemaphoreLock.Lock(INFINITE);
				//Wait for semaphore lock
				//progress dialog shows waiting message
				progressHandler.setSpecialMessage(UtilLoadString(IDS_WAITING_FOR_SEMAPHORE));
				for (; !SemaphoreLock.Lock(20);) {
					while (UtilDoMessageLoop())continue;
					progressHandler.poll();	//needed to detect cancel
					Sleep(20);
				}
			}
			testOneArchive(archive_path, arcLog, progressHandler, std::make_shared<CLFPassphraseGUI>());
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


