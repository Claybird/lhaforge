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
std::vector<BYTE> UtilReadFile(const std::filesystem::path& filePath, size_t maxSize = 0);

class CAutoFile {
protected:
	FILE *_fp;
	std::filesystem::path _path;
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
			_path.clear();
		}
	}
	void open(const std::filesystem::path& fname, const std::wstring& mode = L"rb") {
		close();
		_path = fname;
		auto err = _wfopen_s(&_fp, fname.c_str(), mode.c_str());
		if (err==0 && _fp) {
			//set buffer size
			setvbuf(_fp, NULL, _IOFBF, 1024 * 1024);
		}
	}
	const std::filesystem::path &get_path()const { return _path; }
};

void touchFile(const std::filesystem::path& path);



//read only
class CContinuousFile
{
protected:
	std::vector<std::filesystem::path> _files;
	CAutoFile _fp;
	size_t _currentFile;
	int64_t _curPos;
protected:
	bool nextFile() {
		//_fp.close();
		_currentFile++;
		//reached end of file list
		if (_currentFile >= _files.size()) return false;

		_fp.open(_files[_currentFile]);
		if (!_fp.is_opened()) return false;
		return true;
	}
	bool seek_forward(int64_t offset) {
		if (_files.empty()) return false;
		if(!_fp.is_opened()) return false;
		for (;;) {
			int64_t remain = std::filesystem::file_size(_files[_currentFile]) - _ftelli64(_fp);
			if (offset < remain) {
				_curPos += offset;
				_fseeki64(_fp, offset, SEEK_CUR);
				return true;
			} else {
				offset -= remain;
				_curPos += remain;
				if (!nextFile()) {
					return false;
				}
			}
		}
	}
	bool seek_backward(int64_t offset/*negative value*/) {
		if (_files.empty())return false;
		if (!_fp.is_opened())return false;
		for (;;) {
			int64_t capacity = _ftelli64(_fp);
			if (offset + capacity > 0) {
				_curPos += offset;
				_fseeki64(_fp, offset, SEEK_CUR);
				return true;
			} else {
				offset += capacity;
				_curPos -= capacity;

				_fp.close();
				_currentFile--;
				//reached end of file list
				if (_currentFile < 0) return false;

				_fp.open(_files[_currentFile]);
				if (!_fp.is_opened()) return false;
				_fseeki64(_fp, 0, SEEK_END);
			}
		}
	}
	bool seek_to_end() {
		if (_files.empty())return false;
		_currentFile = _files.size() - 1;
		_fp.open(_files[_currentFile]);
		_fseeki64(_fp, 0, SEEK_END);
		_curPos = 0;
		for (const auto& fname : _files) {
			_curPos += std::filesystem::file_size(fname);
		}
		return true;
	}
	bool seek_to_begin() {
		if (_files.empty())return false;
		_currentFile = 0;
		_curPos = 0;
		_fp.open(_files[_currentFile]);
		return _fp.is_opened();
	}
public:
	CContinuousFile() :_currentFile(0), _curPos(0){}
	virtual ~CContinuousFile() { close(); }
	void close() {
		_fp.close();
		_files.clear();
		_currentFile = 0;
		_curPos = 0;
	}
	bool is_opened()const { return _fp.is_opened(); }
	int64_t tell()const { return _curPos; }
	bool seek(int64_t offset, int32_t origin) {
		switch (origin) {
		case SEEK_CUR:
			if (offset >= 0)return seek_forward(offset);
			else return seek_backward(offset);
			break;
		case SEEK_END:
			if (offset > 0){
				return false;
			} else {
				if(!seek_to_end())return false;
				return seek_backward(offset);
			}
			break;
		case SEEK_SET:
			if (offset >= 0) {
				if(!seek_to_begin())return false;
				return seek_forward(offset);
			} else {
				return false;
			}
			break;
		default:
			return false;
		}
	}
	size_t read(void* buffer, size_t toRead) {
		if (_currentFile >= _files.size()) {
			RAISE_EXCEPTION(L"Reached EOF");
		}
		if (!_fp.is_opened()) {
			RAISE_EXCEPTION(L"Failed to open file %s", _files[_currentFile].c_str());
		}
		size_t actualRead = 0;
		for (;;) {
			actualRead += fread(((unsigned char*)buffer) + actualRead, 1, toRead - actualRead, _fp);
			if (actualRead >= toRead) {
				_curPos += actualRead;
				return actualRead;
			} else if (feof(_fp)) {
				if (!nextFile()) {
					if (_currentFile >= _files.size()) {
						//reached end of file list
						_curPos += actualRead;
						return actualRead;
					}else if (!_fp.is_opened()) {
						RAISE_EXCEPTION(L"Failed to open file %s", _files[_currentFile].c_str());
					}
				}
				continue;
			} else if (ferror(_fp)) {
				RAISE_EXCEPTION(L"Failed to read file %s", _files[_currentFile].c_str());
			}
		}
	}
	bool openFiles(const std::vector<std::filesystem::path>& files) {
		close();
		_files = files;
		if (files.empty())return false;
		_currentFile = 0;
		_fp.open(files[0]);
		return _fp.is_opened();
	}
};

