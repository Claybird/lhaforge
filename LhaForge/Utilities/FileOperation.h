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

std::wstring UtilGetTempPath();
std::wstring UtilGetTemporaryFileName();

bool UtilDeletePath(const wchar_t* PathName);
bool UtilDeleteDir(const wchar_t* Path, bool bDeleteParent);

bool UtilMoveFileToRecycleBin(const std::vector<std::wstring>& fileList);

//recursively enumerates files (no directories) in specified directory
std::vector<std::wstring> UtilRecursiveEnumFile(const wchar_t* lpszRoot);

bool UtilPathIsRoot(const wchar_t* path);
std::wstring UtilPathAddLastSeparator(const wchar_t* path);

//get full & absolute path
std::wstring UtilGetCompletePathName(const wchar_t* lpszFileName);

//returns filenames that matches to the given pattern
std::vector<std::wstring> UtilPathExpandWild(const wchar_t* pattern);

//executable name
std::wstring UtilGetModulePath();
std::wstring UtilGetModuleDirectoryPath();

//read whole file
std::vector<BYTE> UtilReadFile(const wchar_t* lpFile);


class CAutoFile {
protected:
	FILE *_fp;
	CAutoFile(const CAutoFile&) = delete;
	const CAutoFile& operator=(const CAutoFile&) = delete;
public:
	CAutoFile() :_fp(NULL){}
	virtual ~CAutoFile() {
		close();
	}
	operator FILE*() { return _fp; }
	bool is_opened() const { return _fp != NULL; }
	void close() {
		if (_fp) {
			fclose(_fp);
			_fp = NULL;
		}
	}
	void open(const wchar_t* fname, const wchar_t* mode = L"r") {
		close();
		_wfopen_s(&_fp, fname, mode);
		if (_fp) {
			//set buffer size
			setvbuf(_fp, NULL, _IOFBF, 1024 * 1024);
		}
	}
};


