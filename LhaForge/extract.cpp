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
#include "ConfigCode/ConfigManager.h"
#include "ConfigCode/ConfigGeneral.h"
#include "ConfigCode/ConfigExtract.h"
#include "Utilities/Semaphore.h"
#include "Dialogs/LogDialog.h"
#include "Dialogs/LogListDialog.h"
#include "Dialogs/ProgressDlg.h"
#include "Dialogs/ConfirmOverwriteDlg.h"
#include "Utilities/StringUtil.h"
#include "Utilities/FileOperation.h"
#include "Utilities/OSUtil.h"
#include "CommonUtil.h"
#include "CmdLineInfo.h"

std::wstring LF_sanitize_pathname(const std::wstring& rawPath)
{
	const std::pair<std::wregex, wchar_t*> pattern[] = {
		//potential directory traversals
		{std::wregex(L"/{2,}"),L"/"},	//not harmful if properly treated
		{std::wregex(L"(^|/).(/|$)"),L"/"},		//not harmful if properly treated
		{std::wregex(L"(^|/)(\\.){2,}(/|$)"),L"$0_@@@_$2"},
		{std::wregex(L"\\\\"),L"_@@@_"},
		{std::wregex(L"^/"),L""},

		//unicode control characters
		{std::wregex(L"("
			L"\u001e|\u001f|\u00ad|\uFEFF|\uFFF9|\uFFFA|\uFFFB|\uFFFE|"
			L"[\u200b-\u200f]|"
			L"[\u202a-\u202e]|"
			L"[\u2060-\u2063]|"
			L"[\u206a-\u206f]|"
			L")"),
			L"_(UNICODE_CTRL)_"},
	};

	auto buf = rawPath;
	for (;;) {
		bool modified = false;
		for (const auto &p : pattern) {
			auto updated = std::regex_replace(buf, p.first, p.second);
			if ((!modified) && (updated != buf)) {
				modified = true;
			}
			buf = updated;
		}
		if (!modified)break;
	}
	return buf;
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

			auto path_components = split_string(fname, L"/");
			//to remove trailing '/'
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


struct LF_EXTRACT_ARGS {
//	std::wstring outputPath;
//	std::wstring pathToOpen;
	CConfigGeneral general;
	CConfigExtract extract;
};

std::wstring trimArchiveName(bool RemoveSymbolAndNumber, const wchar_t* archive_path)
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

	dirname += L"/";
	return dirname;
}

//GUICallback(default directory)->output directory
std::wstring determineExtractBaseDir(
	const wchar_t* archive_path,
	LF_EXTRACT_ARGS& args)
{
	CPath outputDir;
	CString strErr;
	bool keepConfig;

	auto hr = GetOutputDirPathFromConfig(
		args.extract.OutputDirType,
		archive_path,
		args.extract.OutputDirUserSpecified,
		outputDir,
		keepConfig,
		strErr);

	if (E_ABORT == hr) {
		CANCEL_EXCEPTION();
	} else if (FAILED(hr)) {
		RAISE_EXCEPTION(L"%s", (LPCTSTR)strErr);
	}

	// Warn if output is on network or on a removable disk
	// Confirm to make extract dir if it does not exist
	hr = ConfirmOutputDir(args.general, outputDir, strErr);
	//TODO
	if (E_ABORT==hr) {
		CANCEL_EXCEPTION();
	} else if (FAILED(hr)) {
		RAISE_EXCEPTION(L"%s", (LPCTSTR)strErr);
	}

/*	// pathname too long?
	//TODO: _MAX_PATH should not be a limit any more.
	if (S_OK == hStatus) {
		if (_tcslen(pathOutputDir) >= _MAX_PATH) {
			ErrorMessage(CString(MAKEINTRESOURCE(IDS_ERROR_MAX_PATH)));
			hStatus = S_FALSE;
		}
	}*/

	if (S_FALSE == hr) {
		// Need to change path
		CString title(MAKEINTRESOURCE(IDS_INPUT_TARGET_FOLDER_WITH_SHIFT));
		CFolderDialog dlg(NULL, title, BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE);
		if (IDOK == dlg.DoModal()) {
			keepConfig = (GetKeyState(VK_SHIFT) < 0);	//TODO
			std::filesystem::path pathOutputDir = dlg.GetFolderPath();
			pathOutputDir /= L"/";
			outputDir = pathOutputDir.c_str();
			if (keepConfig) {
				args.extract.OutputDirType = OUTPUT_TO_SPECIFIC_DIR;
				args.extract.OutputDirUserSpecified = pathOutputDir.c_str();
			}
		} else {
			CANCEL_EXCEPTION();
		}
	}

	if (keepConfig) {
		args.extract.OutputDirType = OUTPUT_TO_SPECIFIC_DIR;
		args.extract.OutputDirUserSpecified = (LPCTSTR)outputDir;
	}
	return (LPCTSTR)outputDir;
}

std::wstring determineExtractDir(
	const wchar_t* archive_path,
	const wchar_t* output_base_dir,
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

enum class overwrite_options {
	overwrite,
	overwrite_all,
	skip,
	skip_all,
	abort,
};

#include "Dialogs/ConfirmOverwriteDlg.h"
overwrite_options confirmOverwrite(
	const wchar_t* extracting_file_path,
	UINT64 extracting_file_size,
	__time64_t extracting_file_mtime,
	const wchar_t* existing_file_path,
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
	const wchar_t* archive_path,
	const wchar_t* output_dir,
	LF_EXTRACT_ARGS& args,
	ARCLOG &arcLog,
	std::function<overwrite_options(const wchar_t* /*fullpath*/, const LF_ARCHIVE_ENTRY* /*entry*/)> preExtractHandler,
	std::function<void(const wchar_t* /*originalPath*/, UINT64/*currentSize*/, UINT64/*totalSize*/)> progressHandler
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

		auto fullPath = std::filesystem::path(output_dir) / originalPath;
		fullPath = LF_sanitize_pathname(fullPath);
		fullPath.make_preferred();

		auto outputPath = fullPath.generic_wstring();
		progressHandler(outputPath.c_str(), 0, llOriginalSize);
		if (entry->is_dir()) {
			try {
				std::filesystem::create_directories(outputPath);
				arcLog(originalPath.c_str(), L"directory created");
			} catch (std::filesystem::filesystem_error&) {
				arcLog(originalPath.c_str(), L"failed to create directory");
				CString err;	//TODO
				err.Format(IDS_ERROR_CANNOT_MAKE_DIR, outputPath.c_str());
				RAISE_EXCEPTION((LPCWSTR)err);
			}
		} else {
			auto decision = overwrite_options::overwrite;
			if (std::filesystem::exists(outputPath.c_str())
				&& std::filesystem::is_regular_file(outputPath.c_str())) {
				if (overwrite_options::skip_all == defaultDecision) {
					decision = overwrite_options::skip;
				} else if (overwrite_options::overwrite_all == defaultDecision) {
					decision = overwrite_options::overwrite;
				} else {
					decision = preExtractHandler(outputPath.c_str(), entry);
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
				arcLog(originalPath.c_str(), L"skipped");
				continue;
				break;
			case overwrite_options::abort:
				//abort
				arcLog(originalPath.c_str(), L"cancelled by user");
				CANCEL_EXCEPTION();
				break;
			}

			//go
			CAutoFile fp;
			fp.open(outputPath.c_str(), L"wb");
			if (fp.is_opened()) {
				arcLog(originalPath.c_str(), L"failed to open for write");
				RAISE_EXCEPTION(L"Failed to open file %s", outputPath.c_str());
			}
			for (;;) {
				auto buffer = arc.read_block();
				if (buffer.is_eof()) {
					progressHandler(originalPath.c_str(), llOriginalSize, llOriginalSize);
					break;
				} else {
					progressHandler(originalPath.c_str(), 0, llOriginalSize);
					auto written = fwrite(buffer.buffer, 1, buffer.size, fp);
					if (written != buffer.size) {
						arcLog(originalPath.c_str(), L"write failed");
						RAISE_EXCEPTION(L"Failed to write file %s", outputPath.c_str());
					}
				}
			}
			arcLog(originalPath.c_str(), L"OK");
			fp.close();
		}
		struct __utimbuf64 ut;
		ut.modtime = entry->get_mtime();
		ut.actime = entry->get_mtime();
		_wutime64(outputPath.c_str(), &ut);
	}
	//end
	arc.close();
}


bool DeleteOriginalArchives(const CConfigExtract &ConfExtract,LPCTSTR lpszArcFile)
{
	std::list<CString> fileList;	//削除対象のファイル一覧

	//---マルチボリュームならまとめて削除
	CString strFindParam;
	bool bMultiVolume=false;
	if(ConfExtract.DeleteMultiVolume){	//マルチボリュームでの削除が有効か？
		bMultiVolume=UtilIsMultiVolume(lpszArcFile,strFindParam);
	}

	CString strFiles;	//ファイル一覧

	if(bMultiVolume){
		UtilPathExpandWild(fileList,strFindParam);
		for(std::list<CString>::iterator ite=fileList.begin();ite!=fileList.end();++ite){
			strFiles+=_T("\n");
			strFiles+=*ite;
		}
	}else{
		fileList.push_back(lpszArcFile);
	}

	//削除する
	if(ConfExtract.MoveToRecycleBin){
		//--------------
		// ごみ箱に移動
		//--------------
		if(!ConfExtract.DeleteNoConfirm){	//削除確認する場合
			CString Message;
			if(bMultiVolume){
				//マルチボリューム
				Message.Format(IDS_ASK_MOVE_ARCHIVE_TO_RECYCLE_BIN_MANY);
				Message+=strFiles;
			}else{
				//単一ファイル
				Message.Format(IDS_ASK_MOVE_ARCHIVE_TO_RECYCLE_BIN,lpszArcFile);
			}

			//確認後ゴミ箱に移動
			if(IDYES!=MessageBox(NULL,Message,UtilGetMessageCaption(),MB_YESNO|MB_ICONQUESTION|MB_DEFBUTTON2)){
				return false;
			}
		}

		//削除実行
		UtilMoveFileToRecycleBin(fileList);
		return true;
	}else{
		//------
		// 削除
		//------
		if(!ConfExtract.DeleteNoConfirm){	//確認する場合
			CString Message;
			if(bMultiVolume){
				//マルチボリューム
				Message.Format(IDS_ASK_DELETE_ARCHIVE_MANY);
				Message+=strFiles;
			}else{
				//単一ファイル
				Message.Format(IDS_ASK_DELETE_ARCHIVE,lpszArcFile);
			}
			if(IDYES!=MessageBox(NULL,Message,UtilGetMessageCaption(),MB_YESNO|MB_ICONQUESTION|MB_DEFBUTTON2)){
				return false;
			}
		}
		//削除実行
		for(std::list<CString>::iterator ite=fileList.begin();ite!=fileList.end();++ite){
			DeleteFile(*ite);
		}
		return true;
	}
}

void GUI_extract_multiple_files(
	const CMDLINEINFO* lpCmdLineInfo,
	const std::vector<std::wstring> &archive_files
)
{
	//プログレスバー
	CProgressDialog dlg;
	dlg.Create(NULL);
	dlg.ShowWindow(SW_SHOW);

	LF_EXTRACT_ARGS args;
	CConfigManager mngr;
	try {
		CString strErr;	//TODO: remove this
		if (!mngr.LoadConfig(strErr)) {
			RAISE_EXCEPTION(strErr);
		}
		// load configuration, then override them with command line args
		parseExtractOption(args, mngr, lpCmdLineInfo);
	} catch (const LF_EXCEPTION& e) {
		MessageBox(NULL, e.what(), UtilGetMessageCaption(), MB_OK | MB_ICONERROR);
		return;
	}

	UINT64 totalFiles = archive_files.size();
	std::vector<ARCLOG> logs;
	for (const auto &archive_path : archive_files) {
		std::wstring output_dir;
		try {
			//determine output base directory
			auto output_base_dir = determineExtractBaseDir(archive_path.c_str(), args);

			//output destination directory [could be same as the output base directory]
			output_dir = determineExtractDir(archive_path.c_str(), output_base_dir.c_str(), args);

			//make sure output directory exists
			try {
				std::filesystem::create_directories(output_dir);
			} catch (std::filesystem::filesystem_error&) {
				CString err;
				err.Format(IDS_ERROR_CANNOT_MAKE_DIR, output_dir.c_str());
				RAISE_EXCEPTION((LPCWSTR)err);
			}

			const wchar_t* LHAFORGE_EXTRACT_SEMAPHORE_NAME = L"LhaForgeExtractLimitSemaphore";
			// limit concurrent extractions
			CSemaphoreLocker SemaphoreLock;
			if (args.extract.LimitExtractFileCount) {
				SemaphoreLock.Lock(LHAFORGE_EXTRACT_SEMAPHORE_NAME, args.extract.MaxExtractFileCount);
			}

			while (UtilDoMessageLoop())continue;
			std::function<overwrite_options(const wchar_t*, const LF_ARCHIVE_ENTRY*)> preExtractHandler =
				[&](const wchar_t* fullpath, const LF_ARCHIVE_ENTRY* entry)->overwrite_options {
				if (args.extract.ForceOverwrite
					|| !std::filesystem::exists(fullpath)
					|| !std::filesystem::is_regular_file(fullpath)) {
					return overwrite_options::overwrite;
				} else {
					struct _stat64 st;
					_wstat64(fullpath, &st);
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
			std::function<void(const wchar_t*, UINT64, UINT64)> progressHandler =
				[&](const wchar_t* originalPath, UINT64 currentSize, UINT64 totalSize)->void {
				dlg.SetProgress(
					archive_path.c_str(),
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
			arcLog.setArchivePath(archive_path.c_str());
			extractOneArchive(archive_path.c_str(), output_dir.c_str(), args, arcLog, preExtractHandler, progressHandler);
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

		// open output directory
		if (args.extract.OpenDir) {
			if (args.general.Filer.UseFiler) {
				// expand environment
				std::map<stdString, CString> envInfo;
				MakeExpandInformationEx(envInfo, output_dir.c_str(), NULL);

				// expand command parameter
				CString strCmd, strParam;
				UtilExpandTemplateString(strCmd, args.general.Filer.FilerPath, envInfo);
				UtilExpandTemplateString(strParam, args.general.Filer.Param, envInfo);
				ShellExecuteW(NULL, L"open", strCmd, strParam, NULL, SW_SHOWNORMAL);
			} else {
				//open with explorer
				UtilNavigateDirectory(output_dir.c_str());
			}
		}

		// delete archive or move it to recycle bin
		if (args.extract.DeleteArchiveAfterExtract) {
			DeleteOriginalArchives(args.extract, archive_path.c_str());
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

	if (displayLog) {
		CLogListDialog LogDlg(CString(MAKEINTRESOURCE(IDS_LOGINFO_OPERATION_EXTRACT)));
		LogDlg.SetLogArray(logs);
		LogDlg.DoModal(::GetDesktopWindow());
	}

	// close progress bar
	if (dlg.IsWindow())dlg.DestroyWindow();
}
