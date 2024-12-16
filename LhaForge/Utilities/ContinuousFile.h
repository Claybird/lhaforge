#pragma once
#include "FileOperation.h"

//read only
class CContinuousFile
{
protected:
	std::vector<std::filesystem::path> _files;
	CAutoFile _fp;
	size_t _currentFile;
	int64_t _curPos;
	bool _error;
protected:
	bool nextFile() {
		//_fp.close();
		//reached end of file list
		if (_currentFile +1 >= _files.size()) return false;
		_currentFile++;

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
			if (offset + capacity >= 0) {
				_curPos += offset;
				_fseeki64(_fp, offset, SEEK_CUR);
				return true;
			} else {
				offset += capacity;
				_curPos -= capacity;

				//reached begin of file list
				if (_currentFile <= 0) return false;
				_currentFile--;
				_fp.close();

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
		_error = false;
	}
	bool is_opened()const { return _fp.is_opened(); }
	bool is_error()const { return _error; }
	size_t getNumFiles()const { return _files.size(); }
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
			if (offset < 0) {
				return false;
			} else if (offset >= _curPos) {
				return seek_forward(offset - _curPos);
			} else {
				return seek_backward(offset - _curPos);
			}
			break;
		default:
			return false;
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
	size_t read(void* buffer, size_t toRead) {
		if (_currentFile >= _files.size()) {
			return 0;
			//if (_files.empty()) {
			//	RAISE_EXCEPTION(UtilLoadString(IDS_ERROR_UNEXPECTED_EOF));
			//} else {
			//	RAISE_EXCEPTION(UtilLoadString(IDS_ERROR_UNEXPECTED_EOF) + L": " + _files.back().c_str());
			//}
		}
		if (!_fp.is_opened()) {
			_error = true;
			return 0;
			//RAISE_EXCEPTION(UtilLoadString(IDS_ERROR_OPEN_FILE), _files[_currentFile].c_str());
		}
		size_t actualRead = 0;
		for (;;) {
			actualRead += fread(((unsigned char*)buffer) + actualRead, 1, toRead - actualRead, _fp);

			if (actualRead >= toRead) {
				_curPos += actualRead;
				return actualRead;
			} else if (feof(_fp)) {
				if (!nextFile()) {
					if (_currentFile + 1 >= _files.size()) {
						//reached end of file list
						_curPos += actualRead;
						return actualRead;
					} else if (!_fp.is_opened()) {
						_error = true;
						return 0;
						//RAISE_EXCEPTION(UtilLoadString(IDS_ERROR_OPEN_FILE), _files[_currentFile].c_str());
					}
				}
				continue;
			} else if (ferror(_fp)) {
				_error = true;
				return 0;
				//RAISE_EXCEPTION(UtilLoadString(IDS_ERROR_READ_FILE), _files[_currentFile].c_str());
			}
		}
	}
};

