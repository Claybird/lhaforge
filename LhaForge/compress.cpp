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
#include "ArchiverCode/arc_interface.h"
#include "Dialogs/LogListDialog.h"
#include "Dialogs/ProgressDlg.h"

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


void parseCompressOption(LF_COMPRESS_ARGS& args, const CMDLINEINFO* lpCmdLineInfo)
{
	args.general.load(args.mngr);
	args.compress.load(args.mngr);

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
std::wstring getSourcesBasePath(const std::vector<std::wstring> &sources)
{
	std::unordered_set<std::wstring> directories;
	for (const auto &item : sources) {
		if (std::filesystem::is_directory(item)) {
			directories.insert(item);
		} else {
			directories.insert(std::filesystem::path(item).parent_path());
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

//TODO: is this needed?
bool isAllowedCombination(LF_ARCHIVE_FORMAT fmt, int option)
{
	const auto &cap = get_archive_capability(fmt);
	for (auto allowed : cap.allowed_combinations) {
		if (allowed == option) {
			return true;
		}
	}
	return false;
}

//returns extension with first "."
std::wstring getArchiveFileExtension(LF_ARCHIVE_FORMAT fmt, LF_WRITE_OPTIONS option, const std::wstring& original_path)
{
	const auto &cap = get_archive_capability(fmt);
	for (auto allowed : cap.allowed_combinations) {
		if (allowed == option) {
			std::wstring original_ext;
			if (cap.need_original_ext) {
				original_ext = std::filesystem::path(original_path).extension();
			}
			if (cap.multi_file_archive) {
				if (option & LF_WOPT_SFX) {
					return original_ext + L".exe";
				} else {
					return original_ext + cap.extension;
				}
			} else {
				if (option & LF_WOPT_SFX) {
					return original_ext + L".exe";
				} else {
					return original_ext + cap.extension;
				}
			}
		}
	}
	RAISE_EXCEPTION(L"unexpected format");
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
	auto ext = getArchiveFileExtension(format, option, input_path);

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
	const std::wstring& basePath,
	const std::vector<std::wstring>& sourcePathList)
{
	auto base = std::filesystem::path(basePath);
	std::vector<COMPRESS_SOURCES::PATH_PAIR> out;
	for (const auto& path : sourcePathList) {
		if (base != path) {
			COMPRESS_SOURCES::PATH_PAIR rp;
			rp.originalFullPath = path;
			rp.entryPath = std::filesystem::relative(path, basePath);
			rp.entryPath = replace(rp.entryPath, L"\\", L"/");
			out.push_back(rp);
		}
	}
	return out;
}

std::vector<std::wstring> getAllSourceFiles(const std::vector<std::wstring> &sourcePathList)
{
	std::vector<std::wstring> out;
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
	const std::vector<std::wstring> &givenFiles
)
{
	COMPRESS_SOURCES targets;
	std::vector<std::wstring> sourcePathList = getAllSourceFiles(givenFiles);
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

		//original size
		targets.total_filesize = 0;
		for (const auto& path : sourcePathList) {
			if (std::filesystem::is_regular_file(path)) {
				targets.total_filesize += std::filesystem::file_size(path);
			}
		}
	} catch (const std::filesystem::filesystem_error& e) {
		auto msg = UtilCP932toUNICODE(e.what(), strlen(e.what()));
		throw LF_EXCEPTION(msg);
	}

	return targets;
}


std::wstring confirmOutputFile(
	const std::wstring &default_archive_path,
	const COMPRESS_SOURCES &original_source_list,
	const std::wstring& ext,	//with '.'
	bool bInputFilenameFirst)	//Compress.SpecifyOutputFilename;
{
	std::unordered_set<std::wstring> sourceFiles;
	for (const auto& src : original_source_list.pathPair) {
		sourceFiles.insert(toLower(src.originalFullPath));
	}

	auto archive_path = std::filesystem::path(default_archive_path).make_preferred();
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

std::wstring determineDefaultArchiveDir(
	OUTPUT_TO outputDirType,
	const std::wstring& original_file_path,
	const wchar_t* user_specified_dirpath
	)
{
	struct FILE_CALLBACK :I_LF_GET_OUTPUT_DIR_CALLBACK {
		std::wstring defaultPath;
		virtual std::wstring operator()() { return defaultPath; }	//called in case OUTPUT_TO_ALWAYS_ASK_WHERE
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
	const std::map<std::string, std::string> &archive_options,
	const std::wstring& output_archive,
	const COMPRESS_SOURCES &source_files,
	ARCLOG arcLog,
	std::function<void(const std::wstring& archivePath,
		const std::wstring& path_on_disk,
		UINT64 currentSize,
		UINT64 totalSize)> progressHandler,
	std::function<const char*(struct archive*, LF_PASSPHRASE&)> passphrase_callback
) {
	ARCHIVE_FILE_TO_WRITE archive;
	archive.write_open(output_archive, format, archive_options, passphrase_callback);
	std::uintmax_t processed = 0;
	for (const auto &source : source_files.pathPair) {
		try {
			LF_ARCHIVE_ENTRY entry;
			entry.copy_file_stat(source.originalFullPath, source.entryPath);
			progressHandler(output_archive, source.originalFullPath, processed, source_files.total_filesize);

			if (std::filesystem::is_regular_file(source.originalFullPath)) {
				RAW_FILE_READER provider;
				provider.open(source.originalFullPath);
				archive.add_entry(entry, [&]() {
					auto data = provider();
					if (!data.is_eof()) {
						progressHandler(
							output_archive,
							source.originalFullPath,
							processed+data.offset,
							source_files.total_filesize);
					}
					while (UtilDoMessageLoop())continue;
					return data;
				});

				processed += std::filesystem::file_size(source.originalFullPath);
				progressHandler(
					output_archive,
					source.originalFullPath,
					processed,
					source_files.total_filesize);
			} else {
				//directory
				archive.add_directory(entry);
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

std::map<std::string, std::string> getLAOptionsFromConfig(
	int la_format,
	const std::vector<int> &la_filters,
	bool encrypt,
	CConfigManager &mngr)
{
	std::map<std::string, std::string> params;
	//formats
	switch (la_format & ARCHIVE_FORMAT_BASE_MASK) {
	case ARCHIVE_FORMAT_ZIP:
	{
		CConfigCompressFormatZIP conf;
		conf.load(mngr);
		params.merge(conf.params);
		if (!encrypt) {
			params.erase("encryption");
		}
	}
	break;
	case ARCHIVE_FORMAT_7ZIP:
	{
		CConfigCompressFormat7Z conf;
		conf.load(mngr);
		params.merge(conf.params);
	}
	break;
	case ARCHIVE_FORMAT_TAR:
	{
		CConfigCompressFormatTAR conf;
		conf.load(mngr);
		params = conf.params;
	}
	break;
	case ARCHIVE_FORMAT_RAW:
	{
		//nothing to do
	}
	break;
	}

	//filters
	for (auto la_filter : la_filters) {
		switch (la_filter & ~ARCHIVE_FORMAT_BASE_MASK) {
		case ARCHIVE_FILTER_GZIP:
		{
			CConfigCompressFormatGZ conf;
			conf.load(mngr);
			params.merge(conf.params);
		}
		break;
		case ARCHIVE_FILTER_BZIP2:
		{
			CConfigCompressFormatBZ2 conf;
			conf.load(mngr);
			params.merge(conf.params);
		}
		break;
		case ARCHIVE_FILTER_LZMA:
		{
			CConfigCompressFormatLZMA conf;
			conf.load(mngr);
			params.merge(conf.params);
		}
		break;
		case ARCHIVE_FILTER_XZ:
		{
			CConfigCompressFormatXZ conf;
			conf.load(mngr);
			params.merge(conf.params);
		}
		break;
		case ARCHIVE_FILTER_ZSTD:
		{
			CConfigCompressFormatZSTD conf;
			conf.load(mngr);
			params.merge(conf.params);
		}
		break;
		}
	}
	return params;
}

std::map<std::string, std::string> getLAOptionsFromConfig(
	LF_COMPRESS_ARGS &args,
	LF_ARCHIVE_FORMAT format,
	LF_WRITE_OPTIONS options)
{
	const auto& cap = get_archive_capability(format);
	int la_format = cap.mapped_libarchive_format & ARCHIVE_FORMAT_BASE_MASK;
	std::vector<int> la_filters = { cap.mapped_libarchive_format & ~ARCHIVE_FORMAT_BASE_MASK };

	bool encrypt = (options & LF_WOPT_DATA_ENCRYPTION) != 0;
	return getLAOptionsFromConfig(la_format, la_filters, encrypt, args.mngr);
}


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
		UINT64 totalSize)> &progressHandler,
	std::function<const char*(struct archive*, LF_PASSPHRASE&)> passphraseHandler
	)
{
	// get common path for base path
	COMPRESS_SOURCES sources = buildCompressSources(args, givenFiles);

	//check if the format can contain only one file
	const auto& cap = get_archive_capability(format);
	if (!cap.multi_file_archive) {
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
		getArchiveFileExtension(format, options, givenFiles.front()),
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
		getLAOptionsFromConfig(args, format, options),
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


bool GUI_compress_multiple_files(
	const std::vector<std::wstring> &givenFiles,
	LF_ARCHIVE_FORMAT format,
	LF_WRITE_OPTIONS options,
	CMDLINEINFO& CmdLineInfo)
{
	LF_COMPRESS_ARGS args;
	parseCompressOption(args, &CmdLineInfo);
	//progress bar
	CProgressDialog dlg;
	dlg.Create(nullptr);
	dlg.ShowWindow(SW_SHOW);

	//do compression
	if (0 != CmdLineInfo.Options) {
		options = (LF_WRITE_OPTIONS)CmdLineInfo.Options;
	}

	size_t idxFile = 0, totalFiles = 1;
	std::function<void(const std::wstring&, const std::wstring&, UINT64, UINT64)> progressHandler =
		[&](const std::wstring& archivePath,
			const std::wstring& path_on_disk,
			UINT64 currentSize,
			UINT64 totalSize)->void {
		dlg.SetProgress(
			archivePath,
			idxFile,
			totalFiles,
			path_on_disk,
			currentSize,
			totalSize);
		while (UtilDoMessageLoop())continue;
		if (dlg.isAborted()) {
			CANCEL_EXCEPTION();
		}
	};

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
					LF_passphrase_input
				);
				if (dlg.isAborted()) {
					CANCEL_EXCEPTION();
				}
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
				LF_passphrase_input
			);
		} catch (const LF_EXCEPTION &e) {
			ARCLOG &arcLog = logs.back();
			arcLog.logException(e);
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

	// close progress bar
	if (dlg.IsWindow())dlg.DestroyWindow();

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
	{L"tgz",		LF_FMT_TAR_GZ,	LF_WOPT_STANDARD			,IDS_FORMAT_NAME_TGZ},
	{L"tar+gz",	LF_FMT_TAR_GZ,	LF_WOPT_STANDARD			,IDS_FORMAT_NAME_TGZ},
	{L"tbz",		LF_FMT_TAR_BZ2,	LF_WOPT_STANDARD			,IDS_FORMAT_NAME_TBZ},
	{L"tar+bz2",	LF_FMT_TAR_BZ2,	LF_WOPT_STANDARD			,IDS_FORMAT_NAME_TBZ},
	{L"tlz",		LF_FMT_TAR_LZMA,LF_WOPT_STANDARD			,IDS_FORMAT_NAME_TAR_LZMA},
	{L"tar+lzma",LF_FMT_TAR_LZMA,LF_WOPT_STANDARD			,IDS_FORMAT_NAME_TAR_LZMA},
	{L"txz",		LF_FMT_TAR_XZ,	LF_WOPT_STANDARD			,IDS_FORMAT_NAME_TAR_XZ},
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

//get most similar option
std::tuple<int/*la_format*/, std::vector<int>/*filters*/>
mimic_archive_property(ARCHIVE_FILE_TO_READ& src_archive) {
	src_archive.begin();	//need to scan
	int la_format = archive_format(src_archive);

	std::vector<int> filters;
	auto filter_count = archive_filter_count(src_archive);
	for (int i = 0; i < filter_count; i++) {
		auto code = archive_filter_code(src_archive, i);
		filters.push_back(code);
	}

	return { la_format, filters };
}

//-----
//read from source, write to new
//this would need overhead of extract on read and compress on write
//there seems no way to get raw data
void copyArchive(
	CConfigManager& mngr,
	const std::wstring& dest_filename,
	ARCHIVE_FILE_TO_WRITE& dest,
	const std::wstring& src_filename,
	std::function<bool(LF_ARCHIVE_ENTRY*)> false_if_skip)
{
	//- open the source archive
	bool is_src_encrypted = false;
	std::string passphrase;

	//- scan for archive status
	ARCHIVE_FILE_TO_READ src;
	src.read_open(src_filename, [&](struct archive *a, LF_PASSPHRASE& pf) ->const char* {
		is_src_encrypted = true;
		auto p = LF_passphrase_input(a, pf);
		if (p) {
			passphrase = p;
		} else {
			CANCEL_EXCEPTION();
		}
		return p;
	});
	for (LF_ARCHIVE_ENTRY* entry = src.begin(); entry; entry = src.next()) {
		if (entry->is_encrypted() || is_src_encrypted) {
			break;
		}
	}

	auto [la_format, filters] = mimic_archive_property(src);

	//- open an output archive in most similar option
	LF_COMPRESS_ARGS args;
	parseCompressOption(args, nullptr);	//TODO:command line args are ignored
	auto options = getLAOptionsFromConfig(la_format, filters, is_src_encrypted, mngr);
	dest.write_open_la(dest_filename, la_format, filters, options, [&](struct archive*,LF_PASSPHRASE&) {return passphrase.c_str(); });

	//- reopen for reading with new callback
	src.close();
	src.read_open(src_filename, [&](struct archive *a, LF_PASSPHRASE& pf) ->const char* {
		return passphrase.c_str();
	});

	//- then, copy entries if filter returns true
	for (LF_ARCHIVE_ENTRY* entry = src.begin(); entry; entry = src.next()) {
		if (false_if_skip(entry)) {
			if (entry->is_dir()) {
				dest.add_directory(*entry);
			} else {
				dest.add_entry(*entry, [&]() {
					while (UtilDoMessageLoop())continue;	//TODO
					//TODO progress handler
					return src.read_block();
				});
			}
		}
	}

	//- copy finished. now the caller can add extra files
}
