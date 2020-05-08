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

std::wstring UtilGetTempPath()
{
	auto tempDir = std::filesystem::temp_directory_path();
	return UtilPathAddLastSeparator(tempDir.c_str());
}

std::wstring UtilGetTemporaryFileName()
{
	wchar_t buf[MAX_PATH] = {};
	GetTempFileNameW(UtilGetTempPath().c_str(), L"lhf", 0, buf);
	return buf;
}

bool UtilDeletePath(const wchar_t* PathName)
{
	if( PathIsDirectoryW(PathName) ) {
		//directory
		if( UtilDeleteDir(PathName, true) )return true;
	} else if( PathFileExistsW(PathName) ) {
		//file
		//reset file attribute
		SetFileAttributesW(PathName, FILE_ATTRIBUTE_NORMAL);
		if( DeleteFileW(PathName) )return true;
	}
	return false;
}

//bDeleteParent=true: delete Path itself
//bDeleteParent=false: delete only children of Path
bool UtilDeleteDir(const wchar_t* Path,bool bDeleteParent)
{
	std::wstring FindParam = std::filesystem::path(Path) / L"*";

	bool bRet = true;

	CFindFile cFindFile;
	BOOL bContinue = cFindFile.FindFile(FindParam.c_str());
	while (bContinue) {
		if (!cFindFile.IsDots()) {
			if (cFindFile.IsDirectory()) {
				//directory
				if (!UtilDeleteDir(cFindFile.GetFilePath(), true)) {
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
		if(!RemoveDirectoryW(Path))bRet=false;
	}

	return bRet;
}

bool UtilMoveFileToRecycleBin(const std::vector<std::wstring>& fileList)
{
	ASSERT(!fileList.empty());
	if(fileList.empty())return false;

	std::wstring param;
	for(const auto& item: fileList){
		param += item;
		param += L'|';
	}
	param += L'|';

	auto filter = UtilMakeFilterString(param.c_str());

	SHFILEOPSTRUCTW shfo={0};
	shfo.wFunc = FO_DELETE;
	shfo.pFrom = &filter[0];
	shfo.fFlags =
		FOF_SILENT |	//do not show progress
		FOF_ALLOWUNDO |	//allow undo i.e., to recycle bin
		FOF_NOCONFIRMATION;	//no confirm window
	return 0 == SHFileOperationW(&shfo);
}

//recursively enumerates files (no directories) in specified directory
std::vector<std::wstring> UtilRecursiveEnumFile(const wchar_t* lpszRoot)
{
	CFindFile cFindFile;

	std::vector<std::wstring> files;
	BOOL bContinue = cFindFile.FindFile((std::filesystem::path(lpszRoot) / L"*").c_str());
	while(bContinue){
		if(!cFindFile.IsDots()){
			if(cFindFile.IsDirectory()){
				auto subFiles = UtilRecursiveEnumFile(cFindFile.GetFilePath());
				files.insert(files.end(), subFiles.begin(), subFiles.end());
			}else{
				files.push_back((const wchar_t*)cFindFile.GetFilePath());
			}
		}
		bContinue=cFindFile.FindNextFile();
	}

	return files;
}

bool UtilPathIsRoot(const wchar_t* path)
{
	auto p = std::filesystem::path(path);
	p.make_preferred();
	if (p == p.root_path() || p == p.root_name())return true;
	return false;
}

std::wstring UtilPathAddLastSeparator(const wchar_t* path)
{
	std::wstring p = path;
	if (p.empty() || (p.back() != L'/' && p.back() != L'\\')) {
		p += std::filesystem::path::preferred_separator;
	}
	return p;
}

//get full & absolute path
std::wstring UtilGetCompletePathName(const wchar_t* lpszFileName)
{
	if(!lpszFileName||wcslen(lpszFileName)<=0){
		RAISE_EXCEPTION(L"empty pathname");
	}

	std::filesystem::path abs_path = lpszFileName;
	if(!UtilPathIsRoot(abs_path.c_str())){
		//when only drive letter is given, _wfullpath returns current directory on that drive
		wchar_t* buf = _wfullpath(nullptr, lpszFileName, 0);
		if (!buf) {
			RAISE_EXCEPTION(L"failed to get fullpath");
		}
		abs_path = buf;
		free(buf);
	}

	if (std::filesystem::exists(abs_path)) {
		DWORD bufSize = GetLongPathNameW(abs_path.c_str(), nullptr, 0);
		std::wstring buf;
		buf.resize(bufSize);
		if (!GetLongPathNameW(abs_path.c_str(), &buf[0], bufSize)) {
			RAISE_EXCEPTION(L"failed to get long filename");
		}
		abs_path = buf.c_str();
	}
	return abs_path.make_preferred().wstring();
}

//returns filenames that matches to the given pattern
std::vector<std::wstring> UtilPathExpandWild(const wchar_t* pattern)
{
	std::vector<std::wstring> out;
	std::wstring p = pattern;
	if(std::wstring::npos == p.find_first_of(L"*?")){	//no characters to expand
		out.push_back(pattern);
	}else{
		//expand wild
		CFindFile cFindFile;
		BOOL bContinue=cFindFile.FindFile(pattern);
		while(bContinue){
			if(!cFindFile.IsDots()){
				out.push_back((const wchar_t*)cFindFile.GetFilePath());
			}
			bContinue=cFindFile.FindNextFile();
		}
	}
	return out;
}

//executable name
std::wstring UtilGetModulePath()
{
	std::wstring name;
	name.resize(_MAX_PATH);
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

std::wstring UtilGetModuleDirectoryPath()
{
	return std::filesystem::path(UtilGetModulePath()).parent_path();
}

//read whole file
std::vector<BYTE> UtilReadFile(const wchar_t* lpFile)
{
	if (!lpFile)RAISE_EXCEPTION(L"Invalid Argument");
	if(!std::filesystem::exists(lpFile))RAISE_EXCEPTION(L"File not found");

	struct _stat64 stat = {};
	if (0 != _wstat64(lpFile, &stat))RAISE_EXCEPTION(L"Failed to get filesize");
	auto file_size = (size_t)stat.st_size;

	CAutoFile fp;
	fp.open(lpFile, L"rb");
	if (!fp.is_opened())RAISE_EXCEPTION(L"Failed to open for read");

	std::vector<BYTE> cReadBuffer;
	cReadBuffer.resize(file_size);
	auto ret = fread(&cReadBuffer[0], 1, cReadBuffer.size(), fp);
	if (ret != cReadBuffer.size()) {
		RAISE_EXCEPTION(L"Failed to read from file");
	}

	return cReadBuffer;
}

