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
#include "CommonUtil.h"
#include "ConfigCode/ConfigGeneral.h"
#include "resource.h"
#include "Utilities/FileOperation.h"
#include "Utilities/Utility.h"
#include "Utilities/OSUtil.h"
#include "Utilities/CustomControl.h"

std::filesystem::path LF_GET_OUTPUT_DIR_DEFAULT_CALLBACK::operator()()
{
	if (_skip_user_input) {
		return _default_path;
	} else {
		CLFShellFileOpenDialog dlg(_default_path.c_str(), FOS_FORCEFILESYSTEM | FOS_FILEMUSTEXIST | FOS_PATHMUSTEXIST | FOS_PICKFOLDERS);
		if (IDOK == dlg.DoModal()) {
			if (GetKeyState(VK_SHIFT) < 0) {
				//TODO: is this operation is suitable?
				_skip_user_input = true;
			}
			CString tmp;
			dlg.GetFilePath(tmp);
			_default_path = tmp.operator LPCWSTR();
			return _default_path;
		} else {
			CANCEL_EXCEPTION();
		}
	}
}

//returns output directory that corresponds to outputDirType
std::filesystem::path LF_get_output_dir(
	OUTPUT_TO outputDirType,
	const std::filesystem::path& original_file_path,
	const wchar_t* user_specified_path,
	I_LF_GET_OUTPUT_DIR_CALLBACK &ask_callback)
{
	switch (outputDirType) {
	case OUTPUT_TO::SameDir:
		//directory is same as the original file path
		return original_file_path.parent_path().generic_wstring();
	case OUTPUT_TO::AlwaysAsk:
		return ask_callback();
	case OUTPUT_TO::SpecificDir:	//use provided path
		if (user_specified_path && wcslen(user_specified_path) > 0) {
			return user_specified_path;
		} else {
			//user did not provide a valid path; fall back to desktop
		}
		//FALLTHROUGH
	case OUTPUT_TO::Desktop:
	default:
		return UtilGetDesktopPath();
	}
}

#ifdef UNIT_TEST
TEST(CommonUtil, LF_get_output_dir) {
	struct LF_GET_OUTPUT_DIR_TEST_CALLBACK :I_LF_GET_OUTPUT_DIR_CALLBACK {
		std::filesystem::path _default_path;
		void setArchivePath(const std::filesystem::path archivePath) {
			if (_default_path.empty()) {
				_default_path = archivePath.parent_path();
			}
		}
		std::filesystem::path operator()()override {
			return _default_path;
		}
	};
	LF_GET_OUTPUT_DIR_TEST_CALLBACK output_dir_callback;
	output_dir_callback.setArchivePath(L"C:/path_to/test_archive.ext");
	auto outputDir = LF_get_output_dir(OUTPUT_TO::SameDir, L"C:/path_to/test_archive.ext", L"", output_dir_callback);
	EXPECT_EQ(L"C:/path_to", outputDir);

	outputDir = LF_get_output_dir(OUTPUT_TO::AlwaysAsk, L"C:/path_to/test_archive.ext", L"", output_dir_callback);
	EXPECT_EQ(L"C:/path_to", outputDir);

	outputDir = LF_get_output_dir(OUTPUT_TO::Desktop, L"C:/path_to/test_archive.ext", L"", output_dir_callback);
	EXPECT_EQ(UtilGetDesktopPath(), outputDir);

	outputDir = LF_get_output_dir(OUTPUT_TO::SpecificDir, L"C:/path_to/test_archive.ext", L"Z:/path", output_dir_callback);
	EXPECT_EQ(L"Z:/path", outputDir);

	outputDir = LF_get_output_dir(OUTPUT_TO::SpecificDir, L"C:/path_to/test_archive.ext", L"", output_dir_callback);
	EXPECT_EQ(UtilGetDesktopPath(), outputDir);
}
#endif

//check and ask for user options in case output dir is not suitable; true if user confirms to go
bool LF_confirm_output_dir_type(const CConfigGeneral &Conf, const std::filesystem::path& outputDirIn)
{
	auto outputDir = UtilPathAddLastSeparator(outputDirIn);

	for (;;) {
		auto status = std::filesystem::status(outputDir);
		if (status.type() != std::filesystem::file_type::not_found &&
			status.type() != std::filesystem::file_type::directory) {
			//file with same name as the output directory already exists
			return false;	//no need to confirm
		}

		switch (GetDriveType(outputDir.c_str())) {
		case DRIVE_REMOVABLE://removable
		case DRIVE_CDROM://CD-ROM
			if (Conf.WarnRemovable) {
				if (IDNO == UtilMessageBox(NULL, UtilLoadString(IDS_ASK_ISOK_REMOVABLE), MB_YESNO | MB_ICONQUESTION)) {
					return false;
				} else {
					return true;
				}
			}
			break;
		case DRIVE_REMOTE://remote
		case DRIVE_NO_ROOT_DIR:
			if (Conf.WarnNetwork) {
				if (IDNO == UtilMessageBox(NULL, UtilLoadString(IDS_ASK_ISOK_NETWORK), MB_YESNO | MB_ICONQUESTION)) {
					return false;
				} else {
					return true;
				}
			}
			break;
		}

		if (outputDir.has_parent_path()) {
			auto parent = outputDir.parent_path();
			if (outputDir == parent) {
				return true;
			} else {
				outputDir = parent;
			}
		} else {
			return true;
		}
	}
}
#ifdef UNIT_TEST
TEST(CommonUtil, LF_confirm_output_dir_type) {
	CConfigGeneral conf;
	conf.WarnRemovable = false;
	conf.WarnNetwork = false;
	EXPECT_TRUE(LF_confirm_output_dir_type(conf, L"C:/"));
	EXPECT_TRUE(LF_confirm_output_dir_type(conf, L"C:/temp"));
}
#endif

void LF_ask_and_make_sure_output_dir_exists(const std::filesystem::path& outputDir, LOSTDIR OnDirNotFound)
{
	auto status = std::filesystem::status(outputDir);
	if (status.type() == std::filesystem::file_type::not_found) {
		//destination does not exist
		switch (OnDirNotFound) {
		case LOSTDIR::AskToCreate:
		{
			auto strMsg = Format(UtilLoadString(IDS_ASK_CREATE_DIR), outputDir.c_str());
			if (IDNO == UtilMessageBox(NULL, strMsg, MB_YESNO | MB_ICONQUESTION)) {
				CANCEL_EXCEPTION();
			}
		}
			//FALLTHROUGH
		case LOSTDIR::ForceCreate:
			try {
				std::filesystem::create_directories(outputDir);
			} catch (const std::filesystem::filesystem_error) {
				RAISE_EXCEPTION(UtilLoadString(IDS_ERROR_MKDIR), outputDir.c_str());
			}
			break;
		default://treat as error
			RAISE_EXCEPTION(UtilLoadString(IDS_ERROR_DIR_NOTFOUND), outputDir.c_str());
		}
	}
}

#ifdef UNIT_TEST
TEST(CommonUtil, LF_ask_and_make_sure_output_dir_exists) {
	auto target = UtilGetTempPath() / L"make_sure_test";
	EXPECT_FALSE(std::filesystem::exists(target));
	EXPECT_THROW(LF_ask_and_make_sure_output_dir_exists(target.c_str(), LOSTDIR::Error), LF_EXCEPTION);
	EXPECT_FALSE(std::filesystem::exists(target));
	LF_ask_and_make_sure_output_dir_exists(target.c_str(), LOSTDIR::ForceCreate);
	EXPECT_TRUE(std::filesystem::exists(target));
	UtilDeleteDir(target, true);
}
#endif

//prepare envInfo map for UtilExpandTemplateString()
std::map<std::wstring, std::wstring> LF_make_expand_information(const wchar_t* lpOpenDir, const wchar_t* lpOutputFile)
{
	std::map<std::wstring, std::wstring> templateParams;

	//environment variables
	auto envs = UtilGetEnvInfo();
	for (auto item : envs) {
		//%ENVIRONMENT%=value
		templateParams[toLower(item.first)] = item.second;
	}

	//---about myself
	templateParams[toLower(L"ProgramPath")] = UtilGetModulePath();
	templateParams[toLower(L"ProgramFileName")] = std::filesystem::path(UtilGetModulePath()).filename();
	templateParams[toLower(L"ProgramDir")] = UtilGetModuleDirectoryPath();
	templateParams[toLower(L"ProgramDrive")] = std::filesystem::path(UtilGetModuleDirectoryPath()).root_name();

	if (lpOpenDir) {
		templateParams[toLower(L"dir")] = lpOpenDir;
		templateParams[toLower(L"OutputDir")] = lpOpenDir;
		templateParams[toLower(L"OutputDrive")] = std::filesystem::path(lpOpenDir).root_name();
	}

	if (lpOutputFile) {
		templateParams[toLower(L"OutputFile")] = lpOutputFile;
		templateParams[toLower(L"OutputFileName")] = std::filesystem::path(lpOutputFile).filename();
	}
	return templateParams;
}

#ifdef UNIT_TEST
TEST(CommonUtil, LF_make_expand_information) {
	const auto open_dir = LR"(C:\test\)";
	const auto output_path = LR"(D:\test\output.ext)";
	auto envInfo = LF_make_expand_information(open_dir, output_path);
	EXPECT_TRUE(has_key(envInfo, toLower(L"PATH")));
	EXPECT_TRUE(has_key(envInfo, toLower(L"tmp")));
	EXPECT_EQ(UtilGetModulePath(), envInfo[toLower(L"ProgramPath")]);
	EXPECT_EQ(std::filesystem::path(UtilGetModulePath()).parent_path().wstring(), envInfo[toLower(L"ProgramDir")]);

	EXPECT_EQ(open_dir, envInfo[toLower(L"dir")]);
	EXPECT_EQ(open_dir, envInfo[toLower(L"OutputDir")]);
	EXPECT_EQ(L"C:", envInfo[toLower(L"OutputDrive")]);

	EXPECT_EQ(output_path, envInfo[toLower(L"OutputFile")]);
	EXPECT_EQ(L"output.ext", envInfo[toLower(L"OutputFileName")]);
}
#endif

//replace filenames that could be harmful
std::filesystem::path LF_sanitize_pathname(const std::filesystem::path &rawPath)
{
	const std::pair<std::wregex, wchar_t*> pattern[] = {
		//backslashes "\\" -> "/" ; libarchive will use only "/" for directory separator
		{std::wregex(L"\\\\"),L"/"},
		//---potential directory traversals are not harmful if properly replaced
		//more than two directory separators, e.g., "//" -> "/"
		{std::wregex(L"/{2,}"),L"/"},
		//dot directory name, e.g., "/./" -> "/"
		{std::wregex(L"(^|/)\\.(/|$)"),L"/"},
		//parent directory name, e.g., "/../" -> "/_@@@_/"
		{std::wregex(L"(^|/)(\\.){2,}(/|$)"),L"$1_@@@_$3"},
		//root directory, e.g., "/abc" -> "abc"
		{std::wregex(L"^/"),L""},
		//unavailabre letters
		{std::wregex(LR"((:|\*|\?|"|<|>|\|))"),L"_"},
		//reserved names
		{std::wregex(LR"((^|/)(aux|com\d+|con|lpt\d+|nul|prn)(\.|/|$))", std::regex_constants::icase),L"$1$2_$3"},

		//unicode control characters
		{std::wregex(L"("
			L"[\u0001-\u001f]"	//ISO 6429 C0 control; https://en.wikipedia.org/wiki/List_of_Unicode_characters
			L"|\u007F"	//DEL
			L"|[\u0080-\u009F]"	//ISO 6429 C1 control; https://en.wikipedia.org/wiki/List_of_Unicode_characters
			L"|[\u200b-\u200d]"	//zero-width spaces

			//https://en.wikipedia.org/wiki/Bidirectional_text
			L"|\u200e|\u200f|\u061C"	//LRM,RLM,ALM
			L"|[\u202a-\u202e]"	//RLE,LRO,PDF,RLE,RLO,
			L"|[\u2066-\u2069]"	//LRI,RLI,FSI,PDI
			L")"),
			L"_(UNICODE_CTRL)_"},
	};

	auto buf = rawPath.wstring();
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

#ifdef UNIT_TEST
TEST(CommonUtil, LF_sanitize_pathname) {
	EXPECT_EQ(L"", LF_sanitize_pathname(L""));
	EXPECT_EQ(L"", LF_sanitize_pathname(L"//"));
	EXPECT_EQ(L"", LF_sanitize_pathname(L"\\"));
	EXPECT_EQ(L"a", LF_sanitize_pathname(L"/a"));
	EXPECT_EQ(L"a", LF_sanitize_pathname(L"//a"));
	EXPECT_EQ(L"a", LF_sanitize_pathname(L"\\a"));
	EXPECT_EQ(L"a/", LF_sanitize_pathname(L"//a////"));
	EXPECT_EQ(L"a/b", LF_sanitize_pathname(L"a\\b"));
	EXPECT_EQ(L"a/b", LF_sanitize_pathname(L"a//b"));
	EXPECT_EQ(L"a/b", LF_sanitize_pathname(L"a/./././b"));
	EXPECT_EQ(L"c/_@@@_/d", LF_sanitize_pathname(L"c/../d"));
	EXPECT_EQ(L"e/_@@@_/f", LF_sanitize_pathname(L"e/....../f"));
	EXPECT_EQ(L"a/b/c", LF_sanitize_pathname(L"a/b/c"));
	EXPECT_EQ(L"a/b/c/", LF_sanitize_pathname(L"a/b/c/"));

	EXPECT_EQ(L"abc_(UNICODE_CTRL)_def", LF_sanitize_pathname(L"abc\u202Edef"));

	EXPECT_EQ(L"あいうえお", LF_sanitize_pathname(L"あいうえお"));
	EXPECT_EQ(L"あいう/えお", LF_sanitize_pathname(L"あいう//えお"));

	EXPECT_EQ(L"c_/", LF_sanitize_pathname(L"c:/"));
	EXPECT_EQ(L"c_/AUX_/", LF_sanitize_pathname(L"c:/AUX/"));
	EXPECT_EQ(L"c_/AUX_", LF_sanitize_pathname(L"c:/AUX"));
	EXPECT_EQ(L"AUX_", LF_sanitize_pathname(L"AUX"));

	EXPECT_EQ(L"c_/com1_/", LF_sanitize_pathname(L"c:/com1/"));
	EXPECT_EQ(L"c_/CON_/", LF_sanitize_pathname(L"c:/CON/"));
	EXPECT_EQ(L"c_/lpt1_/", LF_sanitize_pathname(L"c:/lpt1/"));
	EXPECT_EQ(L"c_/nul_/", LF_sanitize_pathname(L"c:/nul/"));
	EXPECT_EQ(L"c_/PRN_/", LF_sanitize_pathname(L"c:/PRN/"));
	EXPECT_EQ(L"c_/COM1_/CON_/PRN_/", LF_sanitize_pathname(L"c:/COM1/CON/PRN/"));

	EXPECT_EQ(L"_______", LF_sanitize_pathname(L":*?\"<>|"));
}
#endif

void LF_deleteOriginalArchives(bool moveToRecycleBin, bool noConfirm, const std::vector<std::filesystem::path>& original_files)
{
	const size_t max_limit = 10;
	std::filesystem::path files;
	if (!noConfirm) {
		std::vector<std::wstring> tmp(original_files.begin(), original_files.end());
		files = join(L"\n", tmp, max_limit);
		if (original_files.size() > max_limit) {
			files += Format(UtilLoadString(IDS_NUM_EXTRA_FILES), original_files.size() - max_limit);
		}

		std::wstring msg;
		if (moveToRecycleBin) {
			msg = UtilLoadString(IDS_ASK_MOVE_PROCESSED_FILES_TO_RECYCLE_BIN);
		} else {
			msg = UtilLoadString(IDS_ASK_DELETE_PROCESSED_FILE);
		}
		msg += files;

		if (IDYES != UtilMessageBox(NULL, msg, MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2)) {
			return;
		}
	}

	//---
	if (moveToRecycleBin) {
		UtilMoveFileToRecycleBin(original_files);
	} else {
		for (const auto &item : original_files) {
			UtilDeletePath(item);
		}
	}
}

#ifdef UNIT_TEST
/*TEST(CommonUtil, LF_sanitize_pathname) {
	LF_deleteOriginalArchives();
}*/
#endif

void LF_setProcessTempPath(const std::filesystem::path& path)
{
	auto envInfo = LF_make_expand_information(nullptr, nullptr);
	std::wstring keyTemp = L"lf_system_original_temp";
	std::wstring keyTmp = L"lf_system_original_tmp";
	//save original
	if (!has_key(envInfo, keyTemp)) {
		SetEnvironmentVariableW(L"LF_SYSTEM_ORIGINAL_TEMP", envInfo[L"temp"].c_str());
	}
	if (!has_key(envInfo, keyTmp)) {
		SetEnvironmentVariableW(L"LF_SYSTEM_ORIGINAL_TMP", envInfo[L"tmp"].c_str());
	}

	//To use custom temporary directory, if necessary
	if (path.empty()) {
		//restore original
		if (has_key(envInfo, keyTemp)) {
			SetEnvironmentVariableW(L"TEMP", envInfo[keyTemp].c_str());
		}
		if (has_key(envInfo, keyTmp)) {
			SetEnvironmentVariableW(L"TMP", envInfo[keyTmp].c_str());
		}
	}else{
		std::filesystem::path pathWork = UtilExpandTemplateString(path, envInfo);

		//get absolute path
		if (pathWork.is_relative()) {
			auto tmp = UtilGetModuleDirectoryPath() / pathWork;
			pathWork = tmp.lexically_normal();
		}
		try {
			pathWork = UtilGetCompletePathName(pathWork);
		} catch (LF_EXCEPTION) {
			//do nothing
		}

		//set environment
		SetEnvironmentVariableW(L"TEMP", pathWork.c_str());
		SetEnvironmentVariableW(L"TMP", pathWork.c_str());
	}
}

#ifdef UNIT_TEST
TEST(CommonUtil, LF_setProcessTempPath)
{
	auto original = LF_make_expand_information(nullptr, nullptr);
	EXPECT_TRUE(has_key(original, L"temp"));
	EXPECT_FALSE(original[L"temp"].empty());
	EXPECT_TRUE(has_key(original, L"tmp"));
	EXPECT_FALSE(original[L"tmp"].empty());

	EXPECT_FALSE(has_key(original, L"lf_system_original_temp"));
	EXPECT_FALSE(has_key(original, L"lf_system_original_tmp"));

	{
		LF_setProcessTempPath(L"");
		auto modified = LF_make_expand_information(nullptr, nullptr);
		EXPECT_TRUE(has_key(modified, L"temp"));
		EXPECT_FALSE(modified[L"temp"].empty());
		EXPECT_TRUE(has_key(modified, L"tmp"));
		EXPECT_FALSE(modified[L"tmp"].empty());

		EXPECT_TRUE(has_key(modified, L"temp"));
		EXPECT_EQ(original[L"temp"], modified[L"lf_system_original_temp"]);
		EXPECT_EQ(original[L"temp"], modified[L"temp"]);
		EXPECT_TRUE(has_key(modified, L"tmp"));
		EXPECT_EQ(original[L"tmp"], modified[L"lf_system_original_tmp"]);
		EXPECT_EQ(original[L"tmp"], modified[L"tmp"]);
	}

	{
		LF_setProcessTempPath(L"abc");
		auto modified = LF_make_expand_information(nullptr, nullptr);
		EXPECT_TRUE(has_key(modified, L"temp"));
		EXPECT_EQ(UtilGetModuleDirectoryPath() / L"abc", modified[L"temp"]);
		EXPECT_TRUE(has_key(modified, L"tmp"));
		EXPECT_EQ(UtilGetModuleDirectoryPath() / L"abc", modified[L"tmp"]);

		EXPECT_EQ(original[L"temp"], modified[L"lf_system_original_temp"]);
		EXPECT_EQ(original[L"tmp"], modified[L"lf_system_original_tmp"]);
	}

	{
		LF_setProcessTempPath(L"");
		auto modified = LF_make_expand_information(nullptr, nullptr);
		EXPECT_TRUE(has_key(modified, L"temp"));
		EXPECT_EQ(original[L"temp"], modified[L"lf_system_original_temp"]);
		EXPECT_EQ(original[L"temp"], modified[L"temp"]);
		EXPECT_TRUE(has_key(modified, L"tmp"));
		EXPECT_EQ(original[L"tmp"], modified[L"lf_system_original_tmp"]);
		EXPECT_EQ(original[L"tmp"], modified[L"tmp"]);
	}
}
#endif

#include "Dialogs/TextInputDlg.h"

const char* CLFPassphraseGUI::operator()()
{
	if (raw.empty()) {
		CTextInputDialog dlg(UtilLoadString(IDS_ENTER_PASSPRASE), true);
		dlg.SetInputText(raw);
		if (IDOK == dlg.DoModal()) {
			set_passphrase(dlg.GetInputText());
			return utf8.c_str();
		} else {
			return nullptr;	//give up
		}
	} else {
		return utf8.c_str();
	}
}

#include "Dialogs/ProgressDlg.h"

CLFProgressHandlerGUI::CLFProgressHandlerGUI(HWND hParentWnd):
	idxEntry(0)
{
	//progress bar
	dlg = std::make_unique<CProgressDialog>();
	dlg->Create(hParentWnd);
	dlg->ShowWindow(SW_SHOW);
}

CLFProgressHandlerGUI::~CLFProgressHandlerGUI()
{
	end();
}

void CLFProgressHandlerGUI::end()
{
	if (dlg && dlg->IsWindow()) {
		dlg->DestroyWindow();
	}
}

void CLFProgressHandlerGUI::setArchive(const std::filesystem::path& path)
{
	__super::setArchive(path);
	if (dlg) {
		dlg->SetEntry(
			archivePath,
			0,
			0,
			L"",
			0);
		while (UtilDoMessageLoop())continue;
	}
}

void CLFProgressHandlerGUI::onNextEntry(const std::filesystem::path& entry_path, int64_t entry_size)
{
	idxEntry++;
	if (dlg) {
		dlg->SetEntry(
			archivePath,
			idxEntry,
			numEntries,
			entry_path.lexically_normal(),
			entry_size);
		while (UtilDoMessageLoop())continue;
		if (dlg->isAborted()) {
			CANCEL_EXCEPTION();
		}
		while (dlg->isPaused()) {
			UtilDoMessageLoop();
			Sleep(100);
		}
	}
}

void CLFProgressHandlerGUI::onEntryIO(int64_t current_size)
{
	if (dlg) {
		dlg->SetEntryProgress(current_size);
		while (UtilDoMessageLoop())continue;
		if (dlg->isAborted()) {
			CANCEL_EXCEPTION();
		}
		while (dlg->isPaused()) {
			UtilDoMessageLoop();
			if (dlg->isAborted()) {
				CANCEL_EXCEPTION();
			}
			Sleep(100);
		}
	}
}

void CLFProgressHandlerGUI::setSpecialMessage(const std::wstring& msg)
{
	if (dlg) {
		dlg->SetSpecialMessage(msg);
	}
}

void CLFProgressHandlerGUI::poll()
{
	if (dlg && dlg->isAborted()) {
		CANCEL_EXCEPTION();
	}
}

#include "Dialogs/WaitDialog.h"
CLFScanProgressHandlerGUI::CLFScanProgressHandlerGUI(HWND hWndParent)
{
	dlg = std::make_unique<CWaitDialog>();
	dlg->Prepare(hWndParent, 5000);
}

CLFScanProgressHandlerGUI::~CLFScanProgressHandlerGUI()
{
	end();
}

void CLFScanProgressHandlerGUI::end()
{
	if (dlg && dlg->IsWindow()) {
		dlg->DestroyWindow();
	}
}

void CLFScanProgressHandlerGUI::setArchive(const std::filesystem::path& path)
{
	if (dlg) {
		dlg->setEntry(path);
		if (dlg->isAborted()) {
			CANCEL_EXCEPTION();
		}
	}
}

void CLFScanProgressHandlerGUI::onNextEntry(const std::filesystem::path& entry_path)
{
	if (dlg) {
		dlg->setEntry(entry_path);
		while (UtilDoMessageLoop())continue;
		if (dlg->isAborted()) {
			CANCEL_EXCEPTION();
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
				entry->path, entry->stat.st_size, entry->stat.st_mtime,
				existing.path, existing.stat.st_size, existing.stat.st_mtime
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


#include "Dialogs/ConfirmOverwriteInArchiveDlg.h"
overwrite_options CLFOverwriteInArchiveConfirmGUI::operator()(
	const std::filesystem::path& new_entry_path,
	const LF_ENTRY_STAT& existing_entry)
{
	if (defaultDecision == overwrite_options::not_defined) {
		CConfirmOverwriteInArchiveDialog dlg;

		LF_ENTRY_STAT new_entry;
		new_entry.read_stat(new_entry_path, new_entry_path);

		dlg.SetFileInfo(
			new_entry_path, new_entry.stat.st_size, new_entry.stat.st_mtime,
			existing_entry.path, existing_entry.stat.st_size, existing_entry.stat.st_mtime
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
}
