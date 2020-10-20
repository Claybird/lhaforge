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

std::wstring UtilGetDesktopPath()
{
	wchar_t* ptr = nullptr;
	if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_Desktop, 0, nullptr, &ptr))) {
		std::wstring path = ptr;
		CoTaskMemFree(ptr);
		ptr = nullptr;
		return path;
	} else {
		//possibly, no desktops
		RAISE_EXCEPTION(L"Unexpected error: %s", UtilLoadString(IDS_ERROR_GET_DESKTOP).c_str());
	}
}

std::wstring UtilGetSendToPath()
{
	wchar_t* ptr = nullptr;
	if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_SendTo, 0, nullptr, &ptr))) {
		std::wstring path = ptr;
		CoTaskMemFree(ptr);
		ptr = nullptr;
		return path;
	} else {
		//possibly, no desktops; //TODO: not a suitable error messasge
		RAISE_EXCEPTION(L"Unexpected error: %s", UtilLoadString(IDS_ERROR_GET_DESKTOP).c_str());
	}
}

//returns a temp dir exclusive use of lhaforge
std::wstring UtilGetTempPath()
{
	auto tempDir = std::filesystem::temp_directory_path() / L"lhaforge";
	std::filesystem::create_directories(tempDir);
	return UtilPathAddLastSeparator(tempDir);
}

std::wstring UtilGetTemporaryFileName()
{
	for (size_t index = 0; ;index++){
		auto path = std::filesystem::path(UtilGetTempPath()) / Format(L"tmp%d.tmp", index);
		if (!std::filesystem::exists(path)) {
			touchFile(path);
			return path.make_preferred();
		}
	}
}

bool UtilDeletePath(const std::wstring& path)
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
bool UtilDeleteDir(const std::wstring& path, bool bDeleteParent)
{
	std::wstring FindParam = std::filesystem::path(path) / L"*";

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

//recursively enumerates files (no directories) in specified directory
std::vector<std::wstring> UtilRecursiveEnumFile(const std::wstring& root)
{
	CFindFile cFindFile;

	std::vector<std::wstring> files;
	BOOL bContinue = cFindFile.FindFile((std::filesystem::path(root) / L"*").c_str());
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
std::vector<std::wstring> UtilRecursiveEnumFileAndDirectory(const std::wstring& root)
{
	CFindFile cFindFile;

	std::vector<std::wstring> files;
	BOOL bContinue = cFindFile.FindFile((std::filesystem::path(root) / L"*").c_str());
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
std::vector<std::wstring> UtilEnumSubFileAndDirectory(const std::wstring& root)
{
	CFindFile cFindFile;

	std::vector<std::wstring> files;
	BOOL bContinue = cFindFile.FindFile((std::filesystem::path(root) / L"*").c_str());
	while (bContinue) {
		if (!cFindFile.IsDots()) {
			files.push_back((const wchar_t*)cFindFile.GetFilePath());
		}
		bContinue = cFindFile.FindNextFile();
	}

	return files;
}

bool UtilPathIsRoot(const std::wstring& path)
{
	auto p = std::filesystem::path(path);
	p.make_preferred();
	if (p == p.root_path() || p == p.root_name())return true;
	return false;
}

std::wstring UtilPathAddLastSeparator(const std::wstring& path)
{
	std::wstring p = path;
	if (p.empty() || (p.back() != L'/' && p.back() != L'\\')) {
		p += std::filesystem::path::preferred_separator;
	}
	return p;
}

std::wstring UtilPathRemoveLastSeparator(const std::wstring& path)
{
	std::wstring p = path;
	if (!p.empty() && (p.back() == L'/' || p.back() == L'\\')) {
		p.back() = L'\0';
	}
	return p.c_str();
}

//get full & absolute path
std::wstring UtilGetCompletePathName(const std::wstring& filePath)
{
	if(filePath.length()==0){
		RAISE_EXCEPTION(L"empty pathname");
	}

	std::filesystem::path abs_path = filePath;
	if(!UtilPathIsRoot(abs_path)){
		//when only drive letter is given, _wfullpath returns current directory on that drive
		wchar_t* buf = _wfullpath(nullptr, filePath.c_str(), 0);
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
std::vector<std::wstring> UtilPathExpandWild(const std::wstring& pattern)
{
	std::vector<std::wstring> out;
	//expand wild
	CFindFile cFindFile;
	BOOL bContinue=cFindFile.FindFile(pattern.c_str());
	while(bContinue){
		if(!cFindFile.IsDots()){
			out.push_back((const wchar_t*)cFindFile.GetFilePath());
		}
		bContinue=cFindFile.FindNextFile();
	}
	return out;
}

//executable name
std::wstring UtilGetModulePath()
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

std::wstring UtilGetModuleDirectoryPath()
{
	return std::filesystem::path(UtilGetModulePath()).parent_path();
}

//read whole file
std::vector<BYTE> UtilReadFile(const std::wstring& filePath)
{
	if (filePath.length()==0)RAISE_EXCEPTION(L"Invalid Argument");
	if(!std::filesystem::exists(filePath))RAISE_EXCEPTION(L"File not found");

	struct _stat64 stat = {};
	if (0 != _wstat64(filePath.c_str(), &stat))RAISE_EXCEPTION(L"Failed to get filesize");
	auto file_size = (size_t)stat.st_size;

	CAutoFile fp;
	fp.open(filePath, L"rb");
	if (!fp.is_opened())RAISE_EXCEPTION(L"Failed to open for read");

	std::vector<BYTE> cReadBuffer;
	cReadBuffer.resize(file_size);
	auto ret = fread(&cReadBuffer[0], 1, cReadBuffer.size(), fp);
	if (ret != cReadBuffer.size()) {
		RAISE_EXCEPTION(L"Failed to read from file");
	}

	return cReadBuffer;
}

void touchFile(const std::wstring& path)
{
	CAutoFile fp;
	fp.open(path, L"w");
}

