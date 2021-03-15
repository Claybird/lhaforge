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
enum PROCESS_MODE{
	PROCESS_INVALID,
	PROCESS_CONFIGURE,
	PROCESS_COMPRESS,
	PROCESS_EXTRACT,
	PROCESS_AUTOMATIC,	//extract if possible, compress otherwise
	PROCESS_LIST,
	PROCESS_TEST,
	PROCESS_MANAGED,	//depends on users' keyboard state
};


#include "ArchiverCode/archive.h"
#include "Utilities/OSUtil.h"

// command line arguments
struct CMDLINEINFO{
	enum class ACTION{
		Default=-1,
		False,
		True,
	};
	CMDLINEINFO() :
		CompressType(LF_FMT_INVALID),
		Options(0),
		bSingleCompression(false),
		OutputToOverride(OUTPUT_TO_DEFAULT),
		CreateDirOverride(CREATE_OUTPUT_DIR_DEFAULT),
		IgnoreTopDirOverride(ACTION::Default),
		DeleteAfterProcess(ACTION::Default)
		{}

	std::vector<std::wstring> FileList;
	std::wstring OutputDir;
	std::wstring OutputFileName;
	LF_ARCHIVE_FORMAT CompressType;
	int Options;
	bool bSingleCompression;
	std::wstring ConfigPath;
	OUTPUT_TO OutputToOverride;
	CREATE_OUTPUT_DIR CreateDirOverride;
	ACTION IgnoreTopDirOverride;
	ACTION DeleteAfterProcess;
};


//Parse command line
std::pair<PROCESS_MODE, CMDLINEINFO> ParseCommandLine(
	const std::wstring& cmdline,
	std::function<void(const std::wstring& msg)> errorHandler);

