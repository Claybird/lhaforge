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

#pragma once

#include "ArchiverCode/arc_interface.h"

//設定から出力先フォルダを読み込む
//r_bUseForAll:今後も同じフォルダ設定を使うならtrue
[[deprecated("will be removed; use LF_get_output_dir instead")]]
HRESULT GetOutputDirPathFromConfig(OUTPUT_TO,LPCTSTR lpszOrgFile,LPCTSTR lpszSpecific,CPath &r_pathOutputDir,bool &r_bUseForAll,CString &strErr);

struct I_LF_GET_OUTPUT_DIR_CALLBACK {
	virtual std::wstring operator()() = 0;
	virtual ~I_LF_GET_OUTPUT_DIR_CALLBACK() {}
};
struct LF_GET_OUTPUT_DIR_DEFAULT_CALLBACK:I_LF_GET_OUTPUT_DIR_CALLBACK {
	std::wstring _default_path;
	bool _skip_user_input;	//true if use _default_path without asking; will be set true by user
	LF_GET_OUTPUT_DIR_DEFAULT_CALLBACK() : _skip_user_input(false) {}
	void setArchivePath(const std::wstring& archivePath) {
		if (_default_path.empty()) {
			_default_path = std::filesystem::path(archivePath).parent_path();
		}
	}
	std::wstring operator()()override {
		if (_skip_user_input) {
			return _default_path;
		} else {
			CString title(MAKEINTRESOURCE(IDS_INPUT_TARGET_FOLDER_WITH_SHIFT));
			CFolderDialog dlg(NULL, title, BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE);
			dlg.SetInitialFolder(_default_path.c_str());
			if (IDOK == dlg.DoModal()) {
				if (GetKeyState(VK_SHIFT) < 0) {
					//TODO: is this operation is suitable?
					_skip_user_input = true;
				}
				_default_path = dlg.GetFolderPath();
				return _default_path;
			} else {
				CANCEL_EXCEPTION();
			}
		}
	}
};

//returns output directory that corresponds to outputDirType
std::wstring LF_get_output_dir(
	OUTPUT_TO outputDirType,
	const std::wstring& original_file_path,
	const wchar_t* user_specified_path,
	I_LF_GET_OUTPUT_DIR_CALLBACK &ask_callback);


struct CConfigGeneral;

//S_FALSEが返ったときには、何らかの方法で新しい出力先を選択させる(「名前をつけて保存」ダイアログを開く)
[[deprecated("will be removed; use LF_confirm_output_dir_type and LF_ask_and_make_sure_output_dir_exists instead")]]
HRESULT ConfirmOutputDir(const CConfigGeneral &Config,LPCTSTR lpszOutputDir,CString &strErr);

//check and ask for user options in case output dir is not suitable; true if user confirms to go
bool LF_confirm_output_dir_type(const CConfigGeneral &Conf, const std::wstring& outputDirIn);
void LF_ask_and_make_sure_output_dir_exists(const std::wstring& outputDir, LOSTDIR OnDirNotFound);


//prepare envInfo map for UtilExpandTemplateString()
std::map<std::wstring, std::wstring> LF_make_expand_information(const wchar_t* lpOpenDir, const wchar_t* lpOutputFile);

std::wstring LF_sanitize_pathname(const std::wstring &rawPath);
