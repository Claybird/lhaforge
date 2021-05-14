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
#include "ConfigFile.h"
#include "ArchiverCode/archive.h"

struct CConfigCompress:public IConfigIO{
public:

	int/*OUTPUT_TO*/ OutputDirType;
	std::wstring OutputDirUserSpecified;
	bool OpenDir;
	bool SpecifyOutputFilename;
	bool LimitCompressFileCount;
	int MaxCompressFileCount;
	bool UseDefaultParameter;
	LF_ARCHIVE_FORMAT DefaultType;
	int DefaultOptions;

	bool DeleteAfterCompress;
	bool MoveToRecycleBin;
	bool DeleteNoConfirm;

	int/*IGNORE_TOP_DIR*/ IgnoreTopDirectory;
public:
	virtual ~CConfigCompress(){}
	virtual void load(const CConfigFile&);
	virtual void store(CConfigFile&)const;
};

