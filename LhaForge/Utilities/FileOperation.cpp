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

#include "stdafx.h"
#include "FileOperation.h"
#include "Utility.h"
#include "StringUtil.h"
#include "OSUtil.h"
#include "resource.h"

std::filesystem::path UtilGetDesktopPath()
{
	wchar_t* ptr = nullptr;
	if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_Desktop, 0, nullptr, &ptr))) {
		std::filesystem::path path = ptr;
		CoTaskMemFree(ptr);
		ptr = nullptr;
		return path;
	} else {
		//possibly, no desktops
		RAISE_EXCEPTION(UtilLoadString(IDS_ERROR_UNEXPECTED).c_str(), UtilLoadString(IDS_ERROR_GET_DESKTOP).c_str());
	}
}


std::filesystem::path UtilGetSendToPath()
{
	wchar_t* ptr = nullptr;
	if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_SendTo, 0, nullptr, &ptr))) {
		std::filesystem::path path = ptr;
		CoTaskMemFree(ptr);
		ptr = nullptr;
		return path;
	} else {
		//possibly, no sendto
		RAISE_EXCEPTION(UtilLoadString(IDS_ERROR_UNEXPECTED).c_str(), UtilLoadString(IDS_ERROR_GET_DESKTOP).c_str());
	}
}

//returns a temp dir exclusive use of lhaforge
std::filesystem::path UtilGetTempPath()
{
	auto tempDir = std::filesystem::temp_directory_path() / L"lhaforge";
	std::filesystem::create_directories(tempDir);
	return UtilPathAddLastSeparator(tempDir);
}


std::filesystem::path UtilGetTemporaryFileName()
{
	for (size_t index = 0; ;index++){
		auto path = std::filesystem::path(UtilGetTempPath()) / Format(L"tmp%d.tmp", index);
		if (!std::filesystem::exists(path)) {
			touchFile(path);
			return path.make_preferred();
		}
	}
}

bool UtilDeletePath(const std::filesystem::path& path)
{
	if( std::filesystem::is_directory(path) ) {
		//directory
		if( UtilDeleteDir(path, true) )return true;
	} else if( std::filesystem::exists(path) ) {
		//file
		//reset file attribute
		SetFileAttributesW(path.c_str(), FILE_ATTRIBUTE_NORMAL);
		if( DeleteFileW(path.c_str()) )return true;
	}
	return false;
}


//bDeleteParent=true: delete Path itself
//bDeleteParent=false: delete only children of Path
bool UtilDeleteDir(const std::filesystem::path& path, bool bDeleteParent)
{
	auto FindParam = std::filesystem::path(path) / L"*";

	bool bRet = true;

	CFindFile cFindFile;
	BOOL bContinue = cFindFile.FindFile(FindParam.c_str());
	while (bContinue) {
		if (!cFindFile.IsDots()) {
			if (cFindFile.IsDirectory()) {
				//directory
				if (!UtilDeleteDir((const wchar_t*)cFindFile.GetFilePath(), true)) {
					bRet = false;
				}
			} else {
				//reset file attribute
				SetFileAttributesW(cFindFile.GetFilePath(), FILE_ATTRIBUTE_NORMAL);
				if (!DeleteFileW(cFindFile.GetFilePath()))bRet = false;
			}
		}
		bContinue = cFindFile.FindNextFile();
	}

	if(bDeleteParent){
		if(!RemoveDirectoryW(path.c_str()))bRet=false;
	}

	return bRet;
}


bool UtilMoveFileToRecycleBin(const std::vector<std::filesystem::path>& fileList)
{
	ASSERT(!fileList.empty());
	if(fileList.empty())return false;

	std::wstring param;
	for(const auto& item: fileList){
		param += item;
		param += L'|';
	}
	param += L'|';

	auto filter = UtilMakeFilterString(param);

	SHFILEOPSTRUCTW shfo={0};
	shfo.wFunc = FO_DELETE;
	shfo.pFrom = &filter[0];
	shfo.fFlags =
		FOF_SILENT |	//do not show progress
		FOF_ALLOWUNDO |	//allow undo i.e., to recycle bin
		FOF_NOCONFIRMATION;	//no confirm window
	return 0 == SHFileOperationW(&shfo);
}



//enumerates files, removes directory
std::vector<std::filesystem::path> UtilEnumerateFiles(const std::vector<std::filesystem::path>& input, const std::vector<std::wstring>& denyExts)
{
	std::vector<std::filesystem::path> out;
	for (const auto& item : input) {
		std::vector<std::filesystem::path> children;
		if (std::filesystem::is_directory(item)) {
			children = UtilRecursiveEnumFile(item);
		} else {
			children = { item };
		}
		for (const auto& subItem : children) {
			bool bDenied = false;
			for (const auto& deny : denyExts) {
				if (UtilExtMatchSpec(subItem, deny)) {
					bDenied = true;
					break;
				}
			}
			//finally
			if (!bDenied) {
				out.push_back(subItem);
			}
		}
	}
	return out;
}

//recursively enumerates files (no directories) in specified directory
std::vector<std::filesystem::path> UtilRecursiveEnumFile(const std::filesystem::path& root)
{
	CFindFile cFindFile;

	std::vector<std::filesystem::path> files;
	BOOL bContinue = cFindFile.FindFile((root / L"*").c_str());
	while(bContinue){
		if(!cFindFile.IsDots()){
			if(cFindFile.IsDirectory()){
				auto subFiles = UtilRecursiveEnumFile((const wchar_t*)cFindFile.GetFilePath());
				files.insert(files.end(), subFiles.begin(), subFiles.end());
			}else{
				files.push_back((const wchar_t*)cFindFile.GetFilePath());
			}
		}
		bContinue=cFindFile.FindNextFile();
	}

	return files;
}

//recursively enumerates files and directories in specified directory
std::vector<std::filesystem::path> UtilRecursiveEnumFileAndDirectory(const std::filesystem::path& root)
{
	CFindFile cFindFile;

	std::vector<std::filesystem::path> files;
	BOOL bContinue = cFindFile.FindFile((root / L"*").c_str());
	while (bContinue) {
		if (!cFindFile.IsDots()) {
			files.push_back((const wchar_t*)cFindFile.GetFilePath());
			if (cFindFile.IsDirectory()) {
				auto subFiles = UtilRecursiveEnumFile((const wchar_t*)cFindFile.GetFilePath());
				files.insert(files.end(), subFiles.begin(), subFiles.end());
			}
		}
		bContinue = cFindFile.FindNextFile();
	}

	return files;
}

//enumerates files and directories in specified directory
std::vector<std::filesystem::path> UtilEnumSubFileAndDirectory(const std::filesystem::path& root)
{
	CFindFile cFindFile;

	std::vector<std::filesystem::path> files;
	BOOL bContinue = cFindFile.FindFile((std::filesystem::path(root) / L"*").c_str());
	while (bContinue) {
		if (!cFindFile.IsDots()) {
			files.push_back((const wchar_t*)cFindFile.GetFilePath());
		}
		bContinue = cFindFile.FindNextFile();
	}

	return files;
}

//returns filenames that matches to the given pattern
std::vector<std::filesystem::path> UtilPathExpandWild(const std::filesystem::path& pattern)
{
	std::vector<std::filesystem::path> out;
	//expand wild
	CFindFile cFindFile;
	BOOL bContinue = cFindFile.FindFile(pattern.c_str());
	while (bContinue) {
		if (!cFindFile.IsDots()) {
			out.push_back((const wchar_t*)cFindFile.GetFilePath());
		}
		bContinue = cFindFile.FindNextFile();
	}
	return out;
}



bool UtilPathIsRoot(const std::filesystem::path& path)
{
	auto p = std::filesystem::path(path);
	p.make_preferred();
	if (p == p.root_path() || p == p.root_name())return true;
	return false;
}


std::filesystem::path UtilPathAddLastSeparator(const std::filesystem::path& path)
{
	std::wstring p = path.wstring();
	if (p.empty() || (p.back() != L'/' && p.back() != L'\\')) {
		p += std::filesystem::path::preferred_separator;
	}
	return p;
}



std::filesystem::path UtilPathRemoveLastSeparator(const std::filesystem::path& path)
{
	std::wstring p = path;
	if (!p.empty() && (p.back() == L'/' || p.back() == L'\\')) {
		p.back() = L'\0';
	}
	return p.c_str();
}



//get full & absolute path
std::filesystem::path UtilGetCompletePathName(const std::filesystem::path& filePath)
{
	if(filePath.empty()){
		RAISE_EXCEPTION(UtilLoadString(IDS_ERROR_PATH_EMPTY));
	}

	std::filesystem::path abs_path = filePath;
	if(!UtilPathIsRoot(abs_path)){
		//when only drive letter is given, _wfullpath returns current directory on that drive
		wchar_t* buf = _wfullpath(nullptr, filePath.c_str(), 0);
		if (!buf) {
			RAISE_EXCEPTION(UtilLoadString(IDS_ERROR_GET_FULLPATH));
		}
		abs_path = buf;
		free(buf);
	}

	if (std::filesystem::exists(abs_path)) {
		DWORD bufSize = GetLongPathNameW(abs_path.c_str(), nullptr, 0);
		std::wstring buf;
		buf.resize(bufSize);
		if (!GetLongPathNameW(abs_path.c_str(), &buf[0], bufSize)) {
			//RAISE_EXCEPTION(L"failed to get long filename");
			RAISE_EXCEPTION(UtilLoadString(IDS_ERROR_GET_FULLPATH));
		}
		abs_path = buf.c_str();
	}
	return abs_path.make_preferred().wstring();
}


//executable name
std::filesystem::path UtilGetModulePath()
{
	std::wstring name;
	name.resize(256);
	for (;;) {
		DWORD bufsize = (DWORD)name.size();
		auto nCopied = GetModuleFileNameW(GetModuleHandleW(nullptr), &name[0], bufsize);
		if (nCopied < bufsize) {
			break;
		} else {
			name.resize(name.size() * 2);
		}
	}
	return name.c_str();
}

std::filesystem::path UtilGetModuleDirectoryPath()
{
	return std::filesystem::path(UtilGetModulePath()).parent_path();
}


//read whole file
std::vector<BYTE> UtilReadFile(const std::filesystem::path& filePath, size_t maxSize)
{
	if (filePath.empty())RAISE_EXCEPTION(UtilLoadString(IDS_ERROR_PATH_EMPTY));
	if(!std::filesystem::exists(filePath))RAISE_EXCEPTION(UtilLoadString(IDS_ERROR_OPEN_FILE), filePath.c_str());

	struct _stat64 stat = {};
	if (0 != _wstat64(filePath.c_str(), &stat))RAISE_EXCEPTION(UtilLoadString(IDS_ERROR_GET_STAT));
	auto file_size = (size_t)stat.st_size;
	if (maxSize != 0) {
		file_size = std::min(file_size, maxSize);
	}

	CAutoFile fp;
	fp.open(filePath, L"rb");
	if (!fp.is_opened())RAISE_EXCEPTION(UtilLoadString(IDS_ERROR_OPEN_FILE), filePath.c_str());

	std::vector<BYTE> cReadBuffer;
	cReadBuffer.resize(file_size);
	auto ret = fread(&cReadBuffer[0], 1, cReadBuffer.size(), fp);
	if (ret != cReadBuffer.size()) {
		RAISE_EXCEPTION(UtilLoadString(IDS_ERROR_READ_FILE), filePath.c_str());
	}

	return cReadBuffer;
}


bool UtilPathIsInSubDirectory(const std::filesystem::path& subject, const std::filesystem::path& directory)
{
	return startsWith(
		std::filesystem::path(subject).make_preferred(),
		std::filesystem::path(UtilPathAddLastSeparator(directory)).make_preferred()
	);
}


void touchFile(const std::filesystem::path& path)
{
	CAutoFile fp;
	fp.open(path, L"w");
}


CTemporaryDirectoryManager::CTemporaryDirectoryManager()
{
	//%TEMP%/tmp%05d/filename...
	std::filesystem::path base = UtilGetTempPath();
	for (int count = 0; count < NUM_DIR_LIMIT; count++) {
		auto name = Format(L"tmp%05d", count);
		if (!std::filesystem::exists(base / name)) {
			try {
				m_path = base / name;
				std::filesystem::create_directories(base / name);
				return;
			} catch (std::filesystem::filesystem_error) {
				RAISE_EXCEPTION(UtilLoadString(IDS_ERROR_MKDIR), m_path.c_str());
			}
		}
	}
	RAISE_EXCEPTION(UtilLoadString(IDS_ERROR_MKDIR), m_path.c_str());
}



size_t CContinuousFile::read(void* buffer, size_t toRead)
{
	if (_currentFile >= _files.size()) {
		if (_files.empty()) {
			RAISE_EXCEPTION(UtilLoadString(IDS_ERROR_UNEXPECTED_EOF));
		} else {
			RAISE_EXCEPTION(UtilLoadString(IDS_ERROR_UNEXPECTED_EOF) + L": " + _files.back().c_str());
		}
	}
	if (!_fp.is_opened()) {
		RAISE_EXCEPTION(UtilLoadString(IDS_ERROR_OPEN_FILE), _files[_currentFile].c_str());
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
				} else if (!_fp.is_opened()) {
					RAISE_EXCEPTION(UtilLoadString(IDS_ERROR_OPEN_FILE), _files[_currentFile].c_str());
				}
			}
			continue;
		} else if (ferror(_fp)) {
			RAISE_EXCEPTION(UtilLoadString(IDS_ERROR_READ_FILE), _files[_currentFile].c_str());
		}
	}
}
