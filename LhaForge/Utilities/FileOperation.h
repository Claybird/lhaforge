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

std::filesystem::path UtilGetDesktopPath();
std::filesystem::path UtilGetSendToPath();

//returns a temp dir exclusive use of lhaforge
std::filesystem::path UtilGetTempPath();
std::filesystem::path UtilGetTemporaryFileName();
bool UtilDeletePath(const std::filesystem::path& path);

//bDeleteParent=true: delete Path itself
//bDeleteParent=false: delete only children of Path
bool UtilDeleteDir(const std::filesystem::path& path, bool bDeleteParent);


//delete temporary directory automatically
class CTemporaryDirectoryManager
{
	enum { NUM_DIR_LIMIT = 10000 };
protected:
	std::filesystem::path m_path;
public:
	CTemporaryDirectoryManager(){
		//%TEMP%/tmp%05d/filename...
		std::filesystem::path base = UtilGetTempPath();
		for (int count = 0; count < NUM_DIR_LIMIT; count++) {
			auto name = Format(L"tmp%05d", count);
			if(!std::filesystem::exists(base / name)){
				try {
					std::filesystem::create_directories(base / name);
					m_path = base / name;
					return;
				} catch (std::filesystem::filesystem_error) {
					RAISE_EXCEPTION(L"Failed to create directory");
				}
			}
		}
		RAISE_EXCEPTION(L"Failed to create directory");
	}
	virtual ~CTemporaryDirectoryManager() {
		UtilDeleteDir(m_path, true);
	}

	std::filesystem::path path()const {
		return m_path;
	}
};


bool UtilMoveFileToRecycleBin(const std::vector<std::filesystem::path>& fileList);

//recursively enumerates files (no directories) in specified directory
std::vector<std::filesystem::path> UtilRecursiveEnumFile(const std::filesystem::path& root);

//recursively enumerates files and directories in specified directory
std::vector<std::filesystem::path> UtilRecursiveEnumFileAndDirectory(const std::filesystem::path& root);

std::vector<std::filesystem::path> UtilEnumSubFileAndDirectory(const std::filesystem::path& root);


bool UtilPathIsRoot(const std::filesystem::path& path);
std::filesystem::path UtilPathAddLastSeparator(const std::filesystem::path& path);
std::filesystem::path UtilPathRemoveLastSeparator(const std::filesystem::path& path);

//get full & absolute path
std::filesystem::path UtilGetCompletePathName(const std::filesystem::path& filePath);

//returns filenames that matches to the given pattern
std::vector<std::filesystem::path> UtilPathExpandWild(const std::filesystem::path& pattern);

//executable name
std::filesystem::path UtilGetModulePath();
std::filesystem::path UtilGetModuleDirectoryPath();

//read whole file
std::vector<BYTE> UtilReadFile(const std::filesystem::path& filePath);


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
	void open(const std::filesystem::path& fname, const std::wstring& mode = L"r") {
		close();
		auto err = _wfopen_s(&_fp, fname.c_str(), mode.c_str());
		if (err==0 && _fp) {
			//set buffer size
			setvbuf(_fp, NULL, _IOFBF, 1024 * 1024);
		}
	}
};


void touchFile(const std::filesystem::path& path);
