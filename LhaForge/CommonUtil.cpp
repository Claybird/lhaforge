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
#include "ArchiverCode/arc_interface.h"
#include "resource.h"
#include "Utilities/FileOperation.h"
#include "Utilities/OSUtil.h"

//設定から出力先フォルダを読み込む
//r_bUseForAll:今後も同じフォルダ設定を使うならtrue
HRESULT GetOutputDirPathFromConfig(OUTPUT_TO outputDirType,LPCTSTR lpszOrgFile,LPCTSTR lpszSpecific,CPath &r_pathOutputDir,bool &r_bUseForAll,CString &strErr)
{
	TCHAR szBuffer[_MAX_PATH+1];
	FILL_ZERO(szBuffer);

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
		if(SHGetSpecialFolderPath(NULL,szBuffer,CSIDL_DESKTOPDIRECTORY,FALSE)){
			r_pathOutputDir=szBuffer;
		}else{	//デスクトップがない？
			strErr=CString(MAKEINTRESOURCE(IDS_ERROR_GET_DESKTOP));
			return E_FAIL;
		}
		return S_OK;
	case OUTPUT_TO_SAME_DIR:	//Same Directory
		_tcsncpy_s(szBuffer,lpszOrgFile,_MAX_PATH);
		PathRemoveFileSpec(szBuffer);
		r_pathOutputDir=szBuffer;
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

			CString title(MAKEINTRESOURCE(IDS_INPUT_TARGET_FOLDER_WITH_SHIFT));
			CFolderDialog dlg(NULL,title,BIF_RETURNONLYFSDIRS|BIF_NEWDIALOGSTYLE);
			dlg.SetInitialFolder(pathTmp);
			if(IDOK==dlg.DoModal()){
				r_bUseForAll=(GetKeyState(VK_SHIFT)<0);	//TODO
				r_pathOutputDir=dlg.GetFolderPath();
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
	const wchar_t* original_file_path,
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
	{
		std::array<wchar_t, MAX_PATH + 1> buf = {};
		if (SHGetSpecialFolderPathW(NULL, &buf[0], CSIDL_DESKTOPDIRECTORY, FALSE)) {
			return std::wstring(&buf[0]);
		} else {	//unexpected case; desktop does not exist?
			RAISE_EXCEPTION(L"Unexpected error: %s", (const wchar_t*)CString(MAKEINTRESOURCE(IDS_ERROR_GET_DESKTOP)));
		}
	}
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
				if(IDNO== UtilMessageBox(NULL,CString(MAKEINTRESOURCE(IDS_ASK_ISOK_REMOVABLE)),MB_YESNO|MB_ICONQUESTION)){
					return S_FALSE;
				}
			}
			break;
		case DRIVE_REMOTE://リモート (ネットワーク) ドライブです。
		case DRIVE_NO_ROOT_DIR:
			if(Conf.WarnNetwork){
				if(IDNO== UtilMessageBox(NULL,CString(MAKEINTRESOURCE(IDS_ASK_ISOK_NETWORK)),MB_YESNO|MB_ICONQUESTION)){
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
			if(IDNO== UtilMessageBox(NULL,strMsg,MB_YESNO|MB_ICONQUESTION)){
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
bool LF_confirm_output_dir_type(const CConfigGeneral &Conf, const wchar_t* outputDirIn)
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
				if (IDNO == UtilMessageBox(NULL, CString(MAKEINTRESOURCE(IDS_ASK_ISOK_REMOVABLE)), MB_YESNO | MB_ICONQUESTION)) {
					return false;
				} else {
					return true;
				}
			}
			break;
		case DRIVE_REMOTE://remote
		case DRIVE_NO_ROOT_DIR:
			if (Conf.WarnNetwork) {
				if (IDNO == UtilMessageBox(NULL, CString(MAKEINTRESOURCE(IDS_ASK_ISOK_NETWORK)), MB_YESNO | MB_ICONQUESTION)) {
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

void LF_ask_and_make_sure_output_dir_exists(const wchar_t* outputDir, LOSTDIR OnDirNotFound)
{
	auto status = std::filesystem::status(outputDir);
	if (status.type() == std::filesystem::file_type::not_found) {
		//destination does not exist
		switch (OnDirNotFound) {
		case LOSTDIR_ASK_TO_CREATE:
		{
			CString strMsg;
			strMsg.Format(IDS_ASK_CREATE_DIR, outputDir);
			if (IDNO == UtilMessageBox(NULL, strMsg, MB_YESNO | MB_ICONQUESTION)) {
				CANCEL_EXCEPTION();
			}
		}
			//FALLTHROUGH
		case LOSTDIR_FORCE_CREATE:
			try {
				std::filesystem::create_directories(outputDir);
			} catch (const std::filesystem::filesystem_error) {
				CString strErr;
				strErr.Format(IDS_ERROR_CANNOT_MAKE_DIR, outputDir);
				RAISE_EXCEPTION((const wchar_t*)strErr);
			}
			break;
		default://treat as error
		{
			CString strErr;
			strErr.Format(IDS_ERROR_DIR_NOTFOUND, outputDir);
			RAISE_EXCEPTION((const wchar_t*)strErr);
		}
		}
	}
}

//prepare envInfo map for UtilExpandTemplateString()
std::map<std::wstring, std::wstring> LF_make_expand_information(LPCTSTR lpOpenDir, LPCTSTR lpOutputFile)
{
	std::map<std::wstring, std::wstring> templateParams;

	//environment variables
	auto envs = UtilGetEnvInfo();
	for (auto item : envs) {
		//%ENVIRONMENT%=value
		templateParams[L'%' + item.first + L'%'] = item.second;
	}

	//---about myself
	templateParams[L"ProgramPath"] = UtilGetModulePath();
	templateParams[L"ProgramFileName"] = std::filesystem::path(UtilGetModulePath()).filename();
	templateParams[L"ProgramDir"] = UtilGetModuleDirectoryPath();
	templateParams[L"ProgramDrive"] = std::filesystem::path(UtilGetModuleDirectoryPath()).root_name();

	if (lpOpenDir) {
		templateParams[L"dir"] = lpOpenDir;
		templateParams[L"OutputDir"] = lpOpenDir;
		templateParams[L"OutputDrive"] = std::filesystem::path(lpOpenDir).root_name();
	}

	if (lpOutputFile) {
		templateParams[L"OutputFile"] = lpOutputFile;
		templateParams[L"OutputFileName"] = std::filesystem::path(lpOutputFile).filename();
	}
	return templateParams;
}
