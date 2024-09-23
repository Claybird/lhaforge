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

//LhaForge mode
enum class PROCESS_MODE : int {
	INVALID,
	CONFIGURE,
	COMPRESS,
	EXTRACT,
	AUTOMATIC,	//extract if possible, compress otherwise
	LIST,
	TEST,
	MANAGED,	//depends on users' keyboard state
};


#include "ArchiverCode/archive.h"
#include "Utilities/OSUtil.h"
#include "extract.h"

// command line arguments
struct CMDLINEINFO{
	enum class ACTION{
		Default=-1,
		False,
		True,
	};
	CMDLINEINFO() :
		CompressType(LF_ARCHIVE_FORMAT::INVALID),
		Options(0),
		bSingleCompression(false),
		OutputToOverride(OUTPUT_TO::NoOverride),
		CreateDirOverride(EXTRACT_CREATE_DIR::NoOverride),
		IgnoreTopDirOverride(ACTION::Default),
		DeleteAfterProcess(ACTION::Default)
		{}

	std::vector<std::filesystem::path> FileList;
	std::filesystem::path OutputDir;
	std::filesystem::path OutputFileName;
	LF_ARCHIVE_FORMAT CompressType;
	int Options;
	bool bSingleCompression;
	std::filesystem::path ConfigPath;
	OUTPUT_TO OutputToOverride;
	EXTRACT_CREATE_DIR CreateDirOverride;
	ACTION IgnoreTopDirOverride;
	ACTION DeleteAfterProcess;
};


//Parse command line
std::pair<PROCESS_MODE, CMDLINEINFO> ParseCommandLine(
	const std::wstring& cmdline,
	std::function<void(const std::wstring& msg)> errorHandler);

