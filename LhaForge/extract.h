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

#include "CommonUtil.h"

#include "ConfigCode/ConfigFile.h"
#include "ConfigCode/ConfigGeneral.h"
#include "ConfigCode/ConfigExtract.h"

enum class EXTRACT_CREATE_DIR :int {
	NoOverride = -1,	//do not override configured behavior
	Always,	//always create archive-named-directory
	Never,	//never create archive-named-directory, just its contents
	SkipIfSingleFileOrDir,	//skip archive-named-directory, if its root contains single *directory or file*
	SkipIfSingleDirectory,	//skip archive-named-directory, if its root contains single *directory*

	ItemCount,
	LastItem = ItemCount-1,
};

struct LF_EXTRACT_ARGS {
	//	std::wstring outputPath;
	//	std::wstring pathToOpen;
	CConfigGeneral general;
	CConfigExtract extract;
	LF_GET_OUTPUT_DIR_DEFAULT_CALLBACK output_dir_callback;
	void load(const CConfigFile& mngr) {
		general.load(mngr);
		extract.load(mngr);
	}
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

struct CLFOverwriteConfirmGUI: public ILFOverwriteConfirm {
	overwrite_options defaultDecision;
	CLFOverwriteConfirmGUI():defaultDecision(overwrite_options::not_defined){}
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

struct CMDLINEINFO;
std::filesystem::path extractCurrentEntry(
	ILFArchiveFile &arc,
	const LF_ENTRY_STAT *entry,
	const std::filesystem::path& output_dir,
	ARCLOG &arcLog,
	ILFOverwriteConfirm& preExtractHandler,
	ILFProgressHandler& progressHandler
);

bool GUI_extract_multiple_files(
	const std::vector<std::filesystem::path> &archive_files,
	ILFProgressHandler &progressHandler,
	const CMDLINEINFO* lpCmdLineInfo
);
void testOneArchive(
	const std::filesystem::path& archive_path,
	ARCLOG &arcLog,
	ILFProgressHandler &progressHandler,
	ILFPassphrase &passphrase_callback
);
bool GUI_test_multiple_files(
	const std::vector<std::filesystem::path> &archive_files,
	ILFProgressHandler &progressHandler,
	const CMDLINEINFO* lpCmdLineInfo
);
