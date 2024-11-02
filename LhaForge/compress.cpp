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

void parseCompressOption(const CConfigFile& config, LF_COMPRESS_ARGS& args, const CMDLINEINFO* lpCmdLineInfo)
{
	args.load(config);

	//overwrite with command line arguments
	if (lpCmdLineInfo) {
		if (OUTPUT_TO::NoOverride != lpCmdLineInfo->OutputToOverride) {
			args.compress.OutputDirType = (int)lpCmdLineInfo->OutputToOverride;
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
			switch ((COMPRESS_IGNORE_TOP_DIR)args.compress.IgnoreTopDirectory) {
			case COMPRESS_IGNORE_TOP_DIR::Recursive:
			{
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
			}break;
			case COMPRESS_IGNORE_TOP_DIR::FirstTop:
			{
				auto di = std::filesystem::directory_iterator(givenFiles.front());
				if (std::filesystem::begin(di) != std::filesystem::end(di)) {	//not an empty dir
					targets.basePath = givenFiles.front();
				}
			}
				break;
			case COMPRESS_IGNORE_TOP_DIR::None:
			default:
				targets.basePath = std::filesystem::path(targets.basePath).parent_path();
				break;
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
		CLFShellFileSaveDialog dlg(archive_path.c_str(), FOS_FORCEFILESYSTEM | FOS_PATHMUSTEXIST | FOS_OVERWRITEPROMPT, ext.c_str());
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


void compressOneArchive(
	LF_ARCHIVE_FORMAT format,
	LF_WRITE_OPTIONS options,
	const LF_COMPRESS_ARGS& args,
	const std::filesystem::path& output_archive,
	const COMPRESS_SOURCES &source_files,
	ARCLOG &arcLog,
	ILFProgressHandler &progressHandler,
	std::shared_ptr<ILFPassphrase> passphrase_callback
) {
	CLFArchive archive;
	archive.write_open(output_archive, format, options, args, passphrase_callback);
	progressHandler.setArchive(output_archive);
	progressHandler.setNumEntries(source_files.pathPair.size());
	for (const auto &source : source_files.pathPair) {
		try {
			LF_ENTRY_STAT entry;
			entry.read_stat(source.originalFullPath, source.entryPath);

			if (std::filesystem::is_regular_file(source.originalFullPath)) {
				progressHandler.onNextEntry(source.originalFullPath, entry.stat.st_size);
				RAW_FILE_READER provider;
				provider.open(source.originalFullPath);
				uint64_t size = 0;
				archive.add_file_entry(entry, [&]() {
					auto data = provider();
					if (data.offset) {
						size = data.offset->offset + data.size;
					} else {
						size += data.size;
					}
					progressHandler.onEntryIO(size);
					while (UtilDoMessageLoop())continue;
					return data;
				});

				progressHandler.onEntryIO(entry.stat.st_size);
				arcLog(source.originalFullPath, UtilLoadString(IDS_ARCLOG_OK));
			} else {
				//directory
				progressHandler.onNextEntry(source.originalFullPath, 0);
				archive.add_directory_entry(entry);
				arcLog(source.originalFullPath, UtilLoadString(IDS_ARCLOG_OK));
			}
		} catch (const LF_USER_CANCEL_EXCEPTION& e) {	//need this to know that user cancel
			arcLog.logException(e);
			archive.close();
			UtilDeletePath(output_archive);
			throw;
		} catch (const LF_EXCEPTION& e) {
			arcLog.logException(e);
			archive.close();
			UtilDeletePath(output_archive);
			throw;
		} catch (const std::filesystem::filesystem_error& e) {
			auto msg = UtilUTF8toUNICODE(e.what(), strlen(e.what()));
			arcLog.logException(LF_EXCEPTION(msg));
			archive.close();
			UtilDeletePath(output_archive);
			throw LF_EXCEPTION(msg);
		}
	}
	arcLog(output_archive, UtilLoadString(IDS_ARCLOG_OK));
}


void compress_helper(
	const std::vector<std::filesystem::path> &givenFiles,
	LF_ARCHIVE_FORMAT format,
	LF_WRITE_OPTIONS options,
	const CMDLINEINFO* lpCmdLineInfo,
	ARCLOG &arcLog,
	LF_COMPRESS_ARGS &args,
	ILFProgressHandler &progressHandler,
	std::shared_ptr<ILFPassphrase> passphraseHandler
	)
{
	// get common path for base path
	COMPRESS_SOURCES sources = buildCompressSources(args, givenFiles);

	//check if the format can contain only one file
	const auto& cap = CLFArchive::get_compression_capability(format);
	if (!cap.contains_multiple_files) {
		size_t fileCount = 0;
		for (const auto& src : sources.pathPair) {
			if (std::filesystem::is_regular_file(src.originalFullPath)) {
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
	if (lpCmdLineInfo && !lpCmdLineInfo->OutputDir.empty()) {
		pathOutputDir = lpCmdLineInfo->OutputDir;
	} else {
		auto OutputFileName = lpCmdLineInfo ? lpCmdLineInfo->OutputFileName : std::filesystem::path();
		if (!OutputFileName.empty() && OutputFileName.filename().has_parent_path()) {
			pathOutputDir = OutputFileName.filename().parent_path();
		} else {
			pathOutputDir = determineDefaultArchiveDir(
				(OUTPUT_TO)args.compress.OutputDirType,
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
			CLFShellFileOpenDialog dlg(pathOutputDir.c_str(), FOS_FORCEFILESYSTEM | FOS_FILEMUSTEXIST | FOS_PATHMUSTEXIST | FOS_PICKFOLDERS);
			if (IDOK == dlg.DoModal()) {
				CString tmp;
				dlg.GetFilePath(tmp);
				pathOutputDir = tmp.operator LPCWSTR();
				bool keepConfig = (GetKeyState(VK_SHIFT) < 0);	//TODO
				if (keepConfig) {
					args.compress.OutputDirType = (int)OUTPUT_TO::SpecificDir;
					args.compress.OutputDirUserSpecified = pathOutputDir.c_str();
				}
			} else {
				CANCEL_EXCEPTION();
			}
		}
	}
	// Confirm to make extract dir if it does not exist
	LF_ask_and_make_sure_output_dir_exists(pathOutputDir, (LOSTDIR)args.general.OnDirNotFound);

	//archive file title
	std::wstring defaultArchiveTitle;
	if (!lpCmdLineInfo || lpCmdLineInfo->OutputFileName.empty()) {
		defaultArchiveTitle = determineDefaultArchiveTitle(format, options, givenFiles.front());
	} else {
		//only filenames
		defaultArchiveTitle = lpCmdLineInfo->OutputFileName.filename();
	}

	//confirm
	arcLog.setArchivePath(pathOutputDir / defaultArchiveTitle);
	auto archivePath = confirmOutputFile(
		pathOutputDir / defaultArchiveTitle,
		sources,
		cap.formatExt(givenFiles.front(), options),
		args.compress.SpecifyOutputFilename || args.compress.OutputDirType == (int)OUTPUT_TO::AlwaysAsk);

	//delete archive
	if (std::filesystem::exists(archivePath)) {
		if (!UtilDeletePath(archivePath)) {
			auto strLastError = UtilGetLastErrorMessage();
			auto msg = Format(UtilLoadString(IDS_ERROR_FILE_REPLACE), strLastError.c_str());

			ErrorMessage(msg.c_str());
			RAISE_EXCEPTION(msg);
		}
	}

	progressHandler.setArchive(archivePath);
	arcLog.setArchivePath(archivePath);

	//limit concurrent compressions
	CSemaphoreLocker SemaphoreLock;
	if (args.compress.LimitCompressFileCount) {
		const wchar_t* LHAFORGE_COMPRESS_SEMAPHORE_NAME = L"LhaForgeCompressLimitSemaphore";
		SemaphoreLock.Create(LHAFORGE_COMPRESS_SEMAPHORE_NAME, args.compress.MaxCompressFileCount);
		//Wait for semaphore lock
		//progress dialog shows waiting message
		progressHandler.setSpecialMessage(UtilLoadString(IDS_WAITING_FOR_SEMAPHORE));
		for (; !SemaphoreLock.Lock(20);) {
			while (UtilDoMessageLoop())continue;
			progressHandler.poll();	//needed to detect cancel
			Sleep(20);
		}
	}

	//do compression
	compressOneArchive(
		format,
		options,
		args,
		archivePath,
		sources,
		arcLog,
		progressHandler,
		passphraseHandler);

	//notify shell
	::SHChangeNotify(SHCNE_CREATE, SHCNF_PATH, archivePath.c_str(), nullptr);

	//open output directory
	if (args.compress.OpenDir) {
		if (args.general.Filer.UseFiler) {
			auto pathOpenDir = std::filesystem::path(archivePath).parent_path();
			auto envInfo = LF_make_expand_information(pathOpenDir.c_str(), archivePath.c_str());

			auto strCmd = UtilExpandTemplateString(args.general.Filer.FilerPath, envInfo);
			auto strParam = UtilExpandTemplateString(args.general.Filer.Param, envInfo);
			ShellExecuteW(nullptr, L"open", strCmd.c_str(), strParam.c_str(), nullptr, SW_SHOWNORMAL);
		} else {
			UtilNavigateDirectory(archivePath);
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

bool GUI_compress_multiple_files(
	const std::vector<std::filesystem::path> &givenFiles,
	LF_ARCHIVE_FORMAT format,
	LF_WRITE_OPTIONS options,
	ILFProgressHandler &progressHandler,
	const CConfigFile& config,
	const CMDLINEINFO* lpCmdLineInfo)
{
	LF_COMPRESS_ARGS args;
	parseCompressOption(config, args, lpCmdLineInfo);

	//do compression
	if (lpCmdLineInfo && 0 != lpCmdLineInfo->Options) {
		options = (LF_WRITE_OPTIONS)lpCmdLineInfo->Options;
	}

	size_t idxFile = 0, totalFiles = 1;

	std::vector<ARCLOG> logs;
	if (lpCmdLineInfo && lpCmdLineInfo->bSingleCompression) {
		totalFiles = givenFiles.size();
		for (const auto &file : givenFiles) {
			idxFile++;
			try {
				logs.resize(logs.size() + 1);
				compress_helper(
					{ file },
					format,
					options,
					lpCmdLineInfo,
					logs.back(),
					args,
					progressHandler,
					std::make_shared<CLFPassphraseGUI>()
				);
			} catch (const LF_USER_CANCEL_EXCEPTION&) {
				break;
			} catch (const LF_EXCEPTION&) {
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
				lpCmdLineInfo,
				logs.back(),
				args,
				progressHandler,
				std::make_shared<CLFPassphraseGUI>()
			);
		} catch (const LF_EXCEPTION &) {
		}
	}
	progressHandler.end();

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

	if (displayLog) {
		CLogListDialog LogDlg(UtilLoadString(IDS_LOGINFO_OPERATION_COMPRESS).c_str());
		LogDlg.SetLogArray(logs);
		LogDlg.DoModal(::GetDesktopWindow());
	}

	return bAllOK;
}


const std::vector<COMPRESS_COMMANDLINE_PARAMETER> g_CompressionCmdParams = {
	{L"zip",		LF_ARCHIVE_FORMAT::ZIP,		LF_WOPT_STANDARD					,IDS_FORMAT_NAME_ZIP},
	{L"zippass",	LF_ARCHIVE_FORMAT::ZIP,		LF_WOPT_DATA_ENCRYPTION	,IDS_FORMAT_NAME_ZIP_PASS},
//	{L"zipsfx",	LF_ARCHIVE_FORMAT::ZIP,		LF_WOPT_SFX		,IDS_FORMAT_NAME_ZIP_SFX},
//	{L"zippasssfx",LF_ARCHIVE_FORMAT::ZIP,		LF_WOPT_DATA_ENCRYPTION | COMPRESS_SFX,IDS_FORMAT_NAME_ZIP_PASS_SFX},
//	{L"zipsplit",	LF_ARCHIVE_FORMAT::ZIP,		COMPRESS_SPLIT		,IDS_FORMAT_NAME_ZIP_SPLIT},
//	{L"zippasssplit",LF_ARCHIVE_FORMAT::ZIP,		LF_WOPT_DATA_ENCRYPTION | COMPRESS_SPLIT,IDS_FORMAT_NAME_ZIP_PASS_SPLIT},
	{L"7z",		LF_ARCHIVE_FORMAT::_7Z,		LF_WOPT_STANDARD					,IDS_FORMAT_NAME_7Z},
//	{L"7zpass",	LF_ARCHIVE_FORMAT::7Z,		LF_WOPT_DATA_ENCRYPTION	,IDS_FORMAT_NAME_7Z_PASS},
//	{L"7zsfx",	LF_ARCHIVE_FORMAT::7Z,		LF_WOPT_SFX		,IDS_FORMAT_NAME_7Z_SFX},
//	{L"7zsplit",	LF_ARCHIVE_FORMAT::7Z,		COMPRESS_SPLIT		,IDS_FORMAT_NAME_7Z_SPLIT},
//	{L"7zpasssplit",LF_ARCHIVE_FORMAT::7Z,		LF_WOPT_DATA_ENCRYPTION | COMPRESS_SPLIT,IDS_FORMAT_NAME_7Z_PASS_SPLIT},
	{L"gz",		LF_ARCHIVE_FORMAT::GZ,		LF_WOPT_STANDARD			,IDS_FORMAT_NAME_GZ},
	{L"bz2",		LF_ARCHIVE_FORMAT::BZ2,		LF_WOPT_STANDARD			,IDS_FORMAT_NAME_BZ2},
	{L"lzma",	LF_ARCHIVE_FORMAT::LZMA,	LF_WOPT_STANDARD			,IDS_FORMAT_NAME_LZMA},
	{L"xz",		LF_ARCHIVE_FORMAT::XZ,		LF_WOPT_STANDARD			,IDS_FORMAT_NAME_XZ},
	{L"zstd",	LF_ARCHIVE_FORMAT::ZSTD,	LF_WOPT_STANDARD			,IDS_FORMAT_NAME_ZSTD},
	{L"lz4",		LF_ARCHIVE_FORMAT::LZ4,		LF_WOPT_STANDARD			,IDS_FORMAT_NAME_LZ4},
	{L"tar",		LF_ARCHIVE_FORMAT::TAR,		LF_WOPT_STANDARD			,IDS_FORMAT_NAME_TAR},
	{L"tgz",		LF_ARCHIVE_FORMAT::TAR_GZ,	LF_WOPT_STANDARD			,IDS_FORMAT_NAME_TGZ},		//compatibility
	{L"tar+gz",	LF_ARCHIVE_FORMAT::TAR_GZ,	LF_WOPT_STANDARD			,IDS_FORMAT_NAME_TGZ},
	{L"tbz",		LF_ARCHIVE_FORMAT::TAR_BZ2,	LF_WOPT_STANDARD			,IDS_FORMAT_NAME_TBZ},			//compatibility
	{L"tar+bz2",	LF_ARCHIVE_FORMAT::TAR_BZ2,	LF_WOPT_STANDARD			,IDS_FORMAT_NAME_TBZ},
	{L"tlz",		LF_ARCHIVE_FORMAT::TAR_LZMA,LF_WOPT_STANDARD			,IDS_FORMAT_NAME_TAR_LZMA},		//compatibility
	{L"tar+lzma",LF_ARCHIVE_FORMAT::TAR_LZMA,LF_WOPT_STANDARD			,IDS_FORMAT_NAME_TAR_LZMA},
	{L"txz",		LF_ARCHIVE_FORMAT::TAR_XZ,	LF_WOPT_STANDARD			,IDS_FORMAT_NAME_TAR_XZ},		//compatibility
	{L"tar+xz",	LF_ARCHIVE_FORMAT::TAR_XZ,	LF_WOPT_STANDARD			,IDS_FORMAT_NAME_TAR_XZ},
	{L"tar+zstd",LF_ARCHIVE_FORMAT::TAR_ZSTD,LF_WOPT_STANDARD			,IDS_FORMAT_NAME_TAR_ZSTD},
	{L"tar+lz4",LF_ARCHIVE_FORMAT::TAR_LZ4,LF_WOPT_STANDARD			,IDS_FORMAT_NAME_TAR_LZ4},
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

//----

void RAW_FILE_READER::open(const std::filesystem::path& path)
{
	close();
	fp.open(path, L"rb");
	if (!fp.is_opened()) {
		RAISE_EXCEPTION(UtilLoadString(IDS_ERROR_OPEN_FILE), path.wstring().c_str());
	}
}


#pragma comment(lib,"Bcrypt.lib")
