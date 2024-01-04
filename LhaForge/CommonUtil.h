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

#include "ArchiverCode/archive.h"

struct I_LF_GET_OUTPUT_DIR_CALLBACK {
	virtual std::filesystem::path operator()() = 0;
	virtual ~I_LF_GET_OUTPUT_DIR_CALLBACK() {}
};
struct LF_GET_OUTPUT_DIR_DEFAULT_CALLBACK:I_LF_GET_OUTPUT_DIR_CALLBACK {
	std::filesystem::path _default_path;
	bool _skip_user_input;	//true if use _default_path without asking; will be set true by user
	LF_GET_OUTPUT_DIR_DEFAULT_CALLBACK() : _skip_user_input(false) {}
	void setArchivePath(const std::filesystem::path& archivePath) {
		if (_default_path.empty()) {
			_default_path = std::filesystem::path(archivePath).parent_path();
		}
	}
	std::filesystem::path operator()()override;
};

//returns output directory that corresponds to outputDirType
std::filesystem::path LF_get_output_dir(
	OUTPUT_TO outputDirType,
	const std::filesystem::path& original_file_path,
	const wchar_t* user_specified_path,
	I_LF_GET_OUTPUT_DIR_CALLBACK &ask_callback);


struct CConfigGeneral;

//check and ask for user options in case output dir is not suitable; true if user confirms to go
bool LF_confirm_output_dir_type(const CConfigGeneral &Conf, const std::filesystem::path& outputDirIn);
void LF_ask_and_make_sure_output_dir_exists(const std::filesystem::path& outputDir, LOSTDIR OnDirNotFound);


//prepare envInfo map for UtilExpandTemplateString()
std::map<std::wstring, std::wstring> LF_make_expand_information(const wchar_t* lpOpenDir, const wchar_t* lpOutputFile);


//replace filenames that could be harmful
std::filesystem::path LF_sanitize_pathname(const std::filesystem::path &rawPath);

void LF_deleteOriginalArchives(bool moveToRecycleBin, bool noConfirm, const std::vector<std::filesystem::path>& original_files);

void LF_setProcessTempPath(const std::filesystem::path&);


struct CLFPassphraseGUI:public ILFPassphrase {
	virtual ~CLFPassphraseGUI() {}
	const char* operator()()override;
};

struct CLFPassphraseNULL:public ILFPassphrase {
	virtual ~CLFPassphraseNULL() {}
	const char* operator()()override { return nullptr; }
};

struct CLFPassphraseConst :public ILFPassphrase {
	CLFPassphraseConst(const std::wstring& pwd) { set_passphrase(pwd); }
	virtual ~CLFPassphraseConst() {}
	const char* operator()()override { return utf8.c_str(); }
};


struct CLFProgressHandlerNULL :public ILFProgressHandler {
	CLFProgressHandlerNULL() {}
	virtual ~CLFProgressHandlerNULL() {}
	void end()override {}
	void onNextEntry(const std::filesystem::path& entry_path, int64_t entry_size)override {}
	void onEntryIO(int64_t current_size)override {}
};

class CProgressDialog;
struct CLFProgressHandlerGUI :public ILFProgressHandler {
	DISALLOW_COPY_AND_ASSIGN(CLFProgressHandlerGUI);

	int64_t idxEntry;
	std::unique_ptr<CProgressDialog> dlg;

	CLFProgressHandlerGUI(HWND hParentWnd);
	virtual ~CLFProgressHandlerGUI();
	void reset()override {
		__super::reset();
		idxEntry = 0;
	}
	void end()override;
	void onNextEntry(const std::filesystem::path& entry_path, int64_t entry_size)override;
	void onEntryIO(int64_t current_size)override;
};

struct CLFScanProgressHandlerNULL : public ILFScanProgressHandler
{
	virtual ~CLFScanProgressHandlerNULL() {}
	void end()override {}
	void setArchive(const std::filesystem::path& path)override {}
	void onNextEntry(const std::filesystem::path& entry_path)override {}
};

class CWaitDialog;
struct CLFScanProgressHandlerGUI :public ILFScanProgressHandler
{
	DISALLOW_COPY_AND_ASSIGN(CLFScanProgressHandlerGUI);

	std::unique_ptr<CWaitDialog> dlg;

	CLFScanProgressHandlerGUI(HWND hWndParent);
	virtual ~CLFScanProgressHandlerGUI();
	void end()override;
	void setArchive(const std::filesystem::path& path)override;
	void onNextEntry(const std::filesystem::path& entry_path)override;
};



enum class overwrite_options {
	not_defined,
	overwrite,
	skip,
	abort,
};

struct ILFOverwriteConfirm {
	virtual ~ILFOverwriteConfirm() {}
	virtual overwrite_options operator()(const std::filesystem::path& pathToWrite, const LF_ENTRY_STAT* entry) = 0;
};

//confirm overwrite files on disk
struct CLFOverwriteConfirmGUI : public ILFOverwriteConfirm {
	overwrite_options defaultDecision;
	CLFOverwriteConfirmGUI() :defaultDecision(overwrite_options::not_defined) {}
	virtual ~CLFOverwriteConfirmGUI() {}
	overwrite_options operator()(const std::filesystem::path& pathToWrite, const LF_ENTRY_STAT* entry)override;
};

struct CLFOverwriteConfirmFORCED : public ILFOverwriteConfirm {
	overwrite_options decision;
	CLFOverwriteConfirmFORCED(overwrite_options forceDecision) :decision(forceDecision) {}
	virtual ~CLFOverwriteConfirmFORCED() {}
	overwrite_options operator()(const std::filesystem::path& pathToWrite, const LF_ENTRY_STAT* entry)override {
		if (std::filesystem::exists(pathToWrite)
			&& std::filesystem::is_regular_file(pathToWrite)) {
			return decision;
		} else {
			return overwrite_options::overwrite;
		}
	}
};

struct ILFOverwriteInArchiveConfirm {
	virtual ~ILFOverwriteInArchiveConfirm() {}
	virtual overwrite_options operator()(const std::filesystem::path& new_entry_path, const LF_ENTRY_STAT& existing_entry) = 0;
};

//confirm overwrite files in archive
struct CLFOverwriteInArchiveConfirmGUI : public ILFOverwriteInArchiveConfirm {
	overwrite_options defaultDecision;
	CLFOverwriteInArchiveConfirmGUI() :defaultDecision(overwrite_options::not_defined) {}
	virtual ~CLFOverwriteInArchiveConfirmGUI() {}
	overwrite_options operator()(const std::filesystem::path& new_entry_path, const LF_ENTRY_STAT& existing_entry)override;
};

struct CLFOverwriteInArchiveConfirmFORCED : public ILFOverwriteInArchiveConfirm {
	overwrite_options decision;
	CLFOverwriteInArchiveConfirmFORCED(overwrite_options forceDecision) :decision(forceDecision) {}
	virtual ~CLFOverwriteInArchiveConfirmFORCED() {}
	overwrite_options operator()(const std::filesystem::path& new_entry_path, const LF_ENTRY_STAT& existing_entry)override {
		return decision;
	}
};
