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
#include "ConfigCode/ConfigManager.h"
#include "ConfigCode/ConfigGeneral.h"
#include "ArchiverCode/archive.h"
#include "resource.h"
#include "Utilities/FileOperation.h"
#include "Utilities/OSUtil.h"
#include "Utilities/Utility.h"

//設定から出力先フォルダを読み込む
//r_bUseForAll:今後も同じフォルダ設定を使うならtrue
HRESULT GetOutputDirPathFromConfig(OUTPUT_TO outputDirType,LPCTSTR lpszOrgFile,LPCTSTR lpszSpecific,CPath &r_pathOutputDir,bool &r_bUseForAll,CString &strErr)
{
	switch(outputDirType){
	case OUTPUT_TO_SPECIFIC_DIR:	//Specific Directory
		//TRACE(_T("Specific Dir:%s\n"),Config.Common.Extract.OutputDir);
		r_pathOutputDir=lpszSpecific;
		if(_tcslen(r_pathOutputDir)>0){
			return S_OK;
		}else{
			//出力先がかかれていなければ、デスクトップに出力する
		}
		//FALLTHROUGH
	case OUTPUT_TO_DESKTOP:	//Desktop
		try{
			r_pathOutputDir = UtilGetDesktopPath().c_str();
		}catch(const LF_EXCEPTION&){	//デスクトップがない？
			strErr=CString(MAKEINTRESOURCE(IDS_ERROR_GET_DESKTOP));
			return E_FAIL;
		}
		return S_OK;
	case OUTPUT_TO_SAME_DIR:	//Same Directory
		r_pathOutputDir = std::filesystem::path(lpszOrgFile).parent_path().c_str();
		return S_OK;
	case OUTPUT_TO_ALWAYS_ASK_WHERE:	//出力先を毎回聞く
		TRACE(_T("Always ask\n"));
		{
			//元のファイルと同じ場所にする;2回目以降は前回出力場所を使用する
			static CString s_strLastOutput;
			CPath pathTmp;
			if(s_strLastOutput.IsEmpty()){
				pathTmp=lpszOrgFile;
				pathTmp.RemoveFileSpec();
			}else{
				pathTmp=(LPCTSTR)s_strLastOutput;
			}

			LFShellFileOpenDialog dlg(pathTmp, FOS_FORCEFILESYSTEM | FOS_FILEMUSTEXIST | FOS_PATHMUSTEXIST | FOS_PICKFOLDERS);
			if(IDOK==dlg.DoModal()){
				r_bUseForAll=(GetKeyState(VK_SHIFT)<0);	//TODO
				dlg.GetFilePath(r_pathOutputDir);
				s_strLastOutput=(LPCTSTR)r_pathOutputDir;
				return S_OK;
			}else{
				return E_ABORT;	//キャンセルされた
			}
		}
		break;
	default:
		ASSERT(!"This code cannot be run");
		return E_NOTIMPL;
	}
}

//returns output directory that corresponds to outputDirType
std::wstring LF_get_output_dir(
	OUTPUT_TO outputDirType,
	const std::wstring& original_file_path,
	const wchar_t* user_specified_path,
	I_LF_GET_OUTPUT_DIR_CALLBACK &ask_callback)
{
	switch (outputDirType) {
	case OUTPUT_TO_SAME_DIR:
		//directory is same as the original file path
		return std::filesystem::path(original_file_path).parent_path().generic_wstring();
	case OUTPUT_TO_ALWAYS_ASK_WHERE:
		return ask_callback();
	case OUTPUT_TO_SPECIFIC_DIR:	//use provided path
		if (user_specified_path && wcslen(user_specified_path) > 0) {
			return user_specified_path;
		} else {
			//user did not provide a valid path; fall back to desktop
		}
		//FALLTHROUGH
	case OUTPUT_TO_DESKTOP:
	default:
		return UtilGetDesktopPath();
	}
}


//S_FALSEが返ったときには、「名前をつけて保存」ダイアログを開く
HRESULT ConfirmOutputDir(const CConfigGeneral &Conf,LPCTSTR lpszOutputDir,CString &strErr)
{
	//---
	// 出力先がネットワークドライブ/リムーバブルディスクであるなら、出力先を選択させる
	if(Conf.WarnNetwork || Conf.WarnRemovable){
		//ルートドライブ名取得
		CPath pathDrive=lpszOutputDir;
		pathDrive.StripToRoot();

		switch(GetDriveType(pathDrive)){
		case DRIVE_REMOVABLE://ドライブからディスクを抜くことができます。
		case DRIVE_CDROM://CD-ROM
			if(Conf.WarnRemovable){
				if(IDNO== UtilMessageBox(NULL, (const wchar_t*)CString(MAKEINTRESOURCE(IDS_ASK_ISOK_REMOVABLE)),MB_YESNO|MB_ICONQUESTION)){
					return S_FALSE;
				}
			}
			break;
		case DRIVE_REMOTE://リモート (ネットワーク) ドライブです。
		case DRIVE_NO_ROOT_DIR:
			if(Conf.WarnNetwork){
				if(IDNO== UtilMessageBox(NULL, (const wchar_t*)CString(MAKEINTRESOURCE(IDS_ASK_ISOK_NETWORK)),MB_YESNO|MB_ICONQUESTION)){
					return S_FALSE;
				}
			}
			break;
		}
	}

	//---
	//出力先のチェック
	if(!PathIsDirectory(lpszOutputDir)){
		//パスが存在しない場合
		CString strMsg;
		switch(Conf.OnDirNotFound){
		case LOSTDIR_ASK_TO_CREATE:	//作成するかどうか聞く
			strMsg.Format(IDS_ASK_CREATE_DIR,lpszOutputDir);
			if(IDNO== UtilMessageBox(NULL, (const wchar_t*)strMsg,MB_YESNO|MB_ICONQUESTION)){
				return E_ABORT;
			}
			//FALLTHROUGH
		case LOSTDIR_FORCE_CREATE:	//ディレクトリ作成
			try {
				std::filesystem::create_directories(lpszOutputDir);
			} catch (std::filesystem::filesystem_error) {
				strErr.Format(IDS_ERROR_CANNOT_MAKE_DIR,lpszOutputDir);
				//ErrorMessage(strMsg);
				return E_FAIL;
			}
			break;
		default://エラーと見なす
			strErr.Format(IDS_ERROR_DIR_NOTFOUND,lpszOutputDir);
			return E_FAIL;
		}
	}

	return S_OK;
}

//check and ask for user options in case output dir is not suitable; true if user confirms to go
bool LF_confirm_output_dir_type(const CConfigGeneral &Conf, const std::wstring& outputDirIn)
{
	auto outputDir = std::filesystem::path(outputDirIn) / L"/";

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
				if (IDNO == UtilMessageBox(NULL, (const wchar_t*)CString(MAKEINTRESOURCE(IDS_ASK_ISOK_REMOVABLE)), MB_YESNO | MB_ICONQUESTION)) {
					return false;
				} else {
					return true;
				}
			}
			break;
		case DRIVE_REMOTE://remote
		case DRIVE_NO_ROOT_DIR:
			if (Conf.WarnNetwork) {
				if (IDNO == UtilMessageBox(NULL, (const wchar_t*)CString(MAKEINTRESOURCE(IDS_ASK_ISOK_NETWORK)), MB_YESNO | MB_ICONQUESTION)) {
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

void LF_ask_and_make_sure_output_dir_exists(const std::wstring& outputDir, LOSTDIR OnDirNotFound)
{
	auto status = std::filesystem::status(outputDir);
	if (status.type() == std::filesystem::file_type::not_found) {
		//destination does not exist
		switch (OnDirNotFound) {
		case LOSTDIR_ASK_TO_CREATE:
		{
			CString strMsg;
			strMsg.Format(IDS_ASK_CREATE_DIR, outputDir.c_str());
			if (IDNO == UtilMessageBox(NULL, (const wchar_t*)strMsg, MB_YESNO | MB_ICONQUESTION)) {
				CANCEL_EXCEPTION();
			}
		}
			//FALLTHROUGH
		case LOSTDIR_FORCE_CREATE:
			try {
				std::filesystem::create_directories(outputDir);
			} catch (const std::filesystem::filesystem_error) {
				CString strErr;
				strErr.Format(IDS_ERROR_CANNOT_MAKE_DIR, outputDir.c_str());
				RAISE_EXCEPTION((const wchar_t*)strErr);
			}
			break;
		default://treat as error
		{
			CString strErr;
			strErr.Format(IDS_ERROR_DIR_NOTFOUND, outputDir.c_str());
			RAISE_EXCEPTION((const wchar_t*)strErr);
		}
		}
	}
}

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

//replace filenames that could be harmful
std::wstring LF_sanitize_pathname(const std::wstring &rawPath)
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

void LF_deleteOriginalArchives(bool moveToRecycleBin, bool noConfirm, const std::vector<std::wstring>& original_files)
{
	const size_t max_limit = 10;
	std::wstring files;
	if (!noConfirm) {
		files = join(L"\n", original_files, max_limit);
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

#include "Dialogs/TextInputDlg.h"

const char* CLFPassphraseGUI::operator()()
{
	//TODO: use more sophisticated dialog
	if (raw.empty()) {
		CTextInputDialog dlg(L"Enter passphrase");
		dlg.SetInputText(raw.c_str());
		if (IDOK == dlg.DoModal()) {
			set_passphrase(raw);
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

void CLFProgressHandlerGUI::onNextEntry(const std::filesystem::path& entry_path, int64_t entry_size)
{
	idxEntry++;
	if (dlg) {
		dlg->SetEntry(
			archivePath,
			idxEntry,
			numEntries,
			entry_path,
			entry_size);
		while (UtilDoMessageLoop())continue;
		if (dlg->isAborted()) {
			CANCEL_EXCEPTION();
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
		if (dlg->isAborted()) {
			CANCEL_EXCEPTION();
		}
	}
}
