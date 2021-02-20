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

//include this file to use all archive handlers

#pragma once
#include "resource.h"
#include "archive_base.h"
#include "archive_libarchive.h"

enum LOGVIEW{
	LOGVIEW_ON_ERROR,
	LOGVIEW_ALWAYS,
	LOGVIEW_NEVER,

	ENUM_COUNT_AND_LASTITEM(LOGVIEW),
};
enum LOSTDIR{
	LOSTDIR_ASK_TO_CREATE,
	LOSTDIR_FORCE_CREATE,
	LOSTDIR_ERROR,

	ENUM_COUNT_AND_LASTITEM(LOSTDIR),
};
enum OUTPUT_TO{
	OUTPUT_TO_DEFAULT=-1,
	OUTPUT_TO_DESKTOP,
	OUTPUT_TO_SAME_DIR,
	OUTPUT_TO_SPECIFIC_DIR,
	OUTPUT_TO_ALWAYS_ASK_WHERE,

	ENUM_COUNT_AND_LASTITEM(OUTPUT_TO),
};
enum CREATE_OUTPUT_DIR{
	CREATE_OUTPUT_DIR_DEFAULT=-1,
	CREATE_OUTPUT_DIR_ALWAYS,
	CREATE_OUTPUT_DIR_SINGLE,
	CREATE_OUTPUT_DIR_NEVER,

	ENUM_COUNT_AND_LASTITEM(CREATE_OUTPUT_DIR)
};


/*
//---following are extracted other than libarchive
{LF_FMT_ACE, NOT_BY_LIBARCHIVE, false, L".ace", true, {}},
{ LF_FMT_JAK, NOT_BY_LIBARCHIVE, false, L".jak", true, {} },
{ LF_FMT_BZA, NOT_BY_LIBARCHIVE, false, L".bza", true, {} },
{ LF_FMT_GZA, NOT_BY_LIBARCHIVE, false, L".gza", true, {} },
{ LF_FMT_ISH, NOT_BY_LIBARCHIVE, false, L".ish", false, {} },
*/

bool LF_isKnownArchive(const std::filesystem::path& fname);
