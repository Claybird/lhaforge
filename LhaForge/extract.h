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

#include "CmdLineInfo.h"
#include "CommonUtil.h"

#include "ConfigCode/ConfigManager.h"
#include "ConfigCode/ConfigGeneral.h"
#include "ConfigCode/ConfigExtract.h"

struct LF_EXTRACT_ARGS {
	//	std::wstring outputPath;
	//	std::wstring pathToOpen;
	CConfigGeneral general;
	CConfigExtract extract;
	LF_GET_OUTPUT_DIR_DEFAULT_CALLBACK output_dir_callback;
};

enum class overwrite_options {
	overwrite,
	overwrite_all,
	skip,
	skip_all,
	abort,
};

void extractOneArchive(
	const std::wstring& archive_path,
	const std::wstring& output_dir,
	ARCLOG &arcLog,
	std::function<overwrite_options(const std::wstring& fullpath, const LF_ARCHIVE_ENTRY* entry)> preExtractHandler,
	std::function<void(const std::wstring& originalPath, UINT64 currentSize, UINT64 totalSize)> progressHandler
);
bool GUI_extract_multiple_files(
	const std::vector<std::wstring> &archive_files,
	const CMDLINEINFO* lpCmdLineInfo
);
void testOneArchive(
	const std::wstring& archive_path,
	ARCLOG &arcLog,
	std::function<void(const std::wstring& originalPath, UINT64 currentSize, UINT64 totalSize)> progressHandler
);
bool GUI_test_multiple_files(
	const std::vector<std::wstring> &archive_files,
	const CMDLINEINFO* lpCmdLineInfo
);
