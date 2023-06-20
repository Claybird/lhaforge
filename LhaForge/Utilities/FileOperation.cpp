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
		RAISE_EXCEPTION(L"Unexpected error: %s", UtilLoadString(IDS_ERROR_GET_DESKTOP).c_str());
	}
}
#ifdef UNIT_TEST
TEST(FileOperation, UtilGetDesktopPath) {
	EXPECT_TRUE(std::filesystem::exists(UtilGetDesktopPath()));
}
#endif

std::filesystem::path UtilGetSendToPath()
{
	wchar_t* ptr = nullptr;
	if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_SendTo, 0, nullptr, &ptr))) {
		std::filesystem::path path = ptr;
		CoTaskMemFree(ptr);
		ptr = nullptr;
		return path;
	} else {
		//possibly, no desktops; //TODO: not a suitable error messasge
		RAISE_EXCEPTION(L"Unexpected error: %s", UtilLoadString(IDS_ERROR_GET_DESKTOP).c_str());
	}
}
#ifdef UNIT_TEST
TEST(FileOperation, UtilGetSendToPath) {
	EXPECT_TRUE(std::filesystem::exists(UtilGetSendToPath()));
}
#endif

//returns a temp dir exclusive use of lhaforge
std::filesystem::path UtilGetTempPath()
{
	auto tempDir = std::filesystem::temp_directory_path() / L"lhaforge";
	std::filesystem::create_directories(tempDir);
	return UtilPathAddLastSeparator(tempDir);
}
#ifdef UNIT_TEST
TEST(FileOperation, UtilGetTempPath) {
	EXPECT_TRUE(std::filesystem::exists(UtilGetTempPath()));
}
#endif

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
#ifdef UNIT_TEST
TEST(FileOperation, UtilGetTemporaryFileName) {
	auto path = UtilGetTemporaryFileName();
	EXPECT_TRUE(std::filesystem::exists(path));
	EXPECT_TRUE(UtilDeletePath(path));
}
#endif

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
#ifdef UNIT_TEST
TEST(FileOperation, UtilDeletePath) {
	//delete file
	auto path = UtilGetTemporaryFileName();
	EXPECT_TRUE(std::filesystem::exists(path));
	EXPECT_TRUE(UtilDeletePath(path));
	EXPECT_FALSE(std::filesystem::exists(path));
	EXPECT_FALSE(UtilDeletePath(path));

	//delete directory
	auto dir = UtilGetTempPath() / L"lhaforge_test/UtilDeletePath";
	UtilDeletePath(dir);
	EXPECT_FALSE(std::filesystem::exists(dir));
	std::filesystem::create_directories(dir);
	for (int i = 0; i < 100; i++) {
		touchFile(dir / Format(L"a%03d.txt", i));
	}
	std::filesystem::create_directories(dir / L"testDir");
	for (int i = 0; i < 100; i++) {
		touchFile(dir / Format(L"testDir/b%03d.txt", i));
	}
	UtilDeletePath(dir);
	EXPECT_FALSE(std::filesystem::exists(dir));
}
#endif

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
#ifdef UNIT_TEST
TEST(FileOperation, UtilDeleteDir) {
	//delete file
	auto path = UtilGetTempPath() / L"test_UtilDeleteDir";
	EXPECT_FALSE(std::filesystem::exists(path));
	std::filesystem::create_directories(path);
	EXPECT_TRUE(std::filesystem::exists(path));

	touchFile(path / L"test.txt");
	EXPECT_TRUE(UtilDeleteDir(path, true));
	EXPECT_FALSE(std::filesystem::exists(path));
}
#endif

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
#ifdef UNIT_TEST
TEST(FileOperation, UtilMoveFileToRecycleBin) {
	std::vector<std::filesystem::path> fileList;
	//delete directory
	auto dir = UtilGetTempPath() / L"lhaforge_test/UtilMoveFileToRecycleBin";
	UtilDeletePath(dir);
	EXPECT_FALSE(std::filesystem::exists(dir));
	std::filesystem::create_directories(dir);
	for (int i = 0; i < 3; i++) {
		fileList.push_back(dir / Format(L"a%03d.txt", i));
		touchFile(dir / Format(L"a%03d.txt", i));
	}
	EXPECT_TRUE(UtilMoveFileToRecycleBin(fileList));
	UtilDeletePath(dir);
	EXPECT_FALSE(std::filesystem::exists(dir));
}
#endif


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
#ifdef UNIT_TEST
TEST(FileOperation, UtilRecursiveEnumXXX_UtilPathExpandWild) {
	//prepare files
	std::vector<std::filesystem::path> fileList, fileAndDir;
	auto dir = std::filesystem::path(UtilGetTempPath()) / L"lhaforge_test/UtilRecursiveEnumXXX_UtilPathExpandWild";
	UtilDeletePath(dir);
	EXPECT_FALSE(std::filesystem::exists(dir));
	std::filesystem::create_directories(dir);
	for (int i = 0; i < 3; i++) {
		auto fname = dir / Format(L"a%03d.txt", i);
		fileList.push_back(fname);
		fileAndDir.push_back(fname);
		touchFile(fname);
	}

	std::filesystem::create_directories(dir / L"b");
	fileAndDir.push_back(dir / L"b");
	for (int i = 0; i < 3; i++) {
		auto fname = dir / Format(L"b/a%03d.txt", i);
		fileList.push_back(fname);
		fileAndDir.push_back(fname);
		touchFile(fname);
	}

	//enumerate
	{
		auto enumerated = UtilRecursiveEnumFile(dir);
		EXPECT_EQ(fileList.size(), enumerated.size());
		if (fileList.size() == enumerated.size()) {
			for (size_t i = 0; i < fileList.size(); i++) {
				EXPECT_EQ(
					std::filesystem::path(fileList[i]).make_preferred().wstring(),
					std::filesystem::path(enumerated[i]).make_preferred().wstring());
			}
		}
	}
	{
		auto enumerated = UtilRecursiveEnumFileAndDirectory(dir);
		EXPECT_EQ(fileAndDir.size(), enumerated.size());
		if (fileAndDir.size() == enumerated.size()) {
			for (size_t i = 0; i < fileAndDir.size(); i++) {
				EXPECT_EQ(
					std::filesystem::path(fileAndDir[i]).make_preferred().wstring(),
					std::filesystem::path(enumerated[i]).make_preferred().wstring());
			}
		}
	}
	{
		auto enumerated = UtilEnumSubFileAndDirectory(dir);
		EXPECT_EQ(4, enumerated.size());
		if (4 == enumerated.size()) {
			for (size_t i = 0; i < enumerated.size(); i++) {
				EXPECT_EQ(
					std::filesystem::path(fileAndDir[i]).make_preferred().wstring(),
					std::filesystem::path(enumerated[i]).make_preferred().wstring());
			}
		}
	}

	//expand wild
	{
		auto enumerated = UtilPathExpandWild(dir);
		EXPECT_EQ(size_t(1), enumerated.size());
		EXPECT_EQ(enumerated[0], dir.make_preferred());

		auto pdir = std::filesystem::path(dir);
		enumerated = UtilPathExpandWild(pdir / L"*.txt");
		EXPECT_EQ(size_t(3), enumerated.size());
		EXPECT_TRUE(isIn(enumerated, pdir / L"a000.txt"));
		EXPECT_TRUE(isIn(enumerated, pdir / L"a001.txt"));
		EXPECT_TRUE(isIn(enumerated, pdir / L"a002.txt"));

		enumerated = UtilPathExpandWild(pdir / L"*.exe");
		EXPECT_TRUE(enumerated.empty());
	}

	//cleanup
	UtilDeletePath(dir);
	EXPECT_FALSE(std::filesystem::exists(dir));
}
#endif

bool UtilPathIsRoot(const std::filesystem::path& path)
{
	auto p = std::filesystem::path(path);
	p.make_preferred();
	if (p == p.root_path() || p == p.root_name())return true;
	return false;
}
#ifdef UNIT_TEST
TEST(FileOperation, UtilPathIsRoot) {
	EXPECT_TRUE(UtilPathIsRoot(L"c:/"));
	EXPECT_TRUE(UtilPathIsRoot(L"c:\\"));
	EXPECT_TRUE(UtilPathIsRoot(L"c:"));
	EXPECT_FALSE(UtilPathIsRoot(L"c:/windows/"));
	EXPECT_FALSE(UtilPathIsRoot(L"c:\\windows\\"));
}
#endif

std::filesystem::path UtilPathAddLastSeparator(const std::filesystem::path& path)
{
	std::wstring p = path.wstring();
	if (p.empty() || (p.back() != L'/' && p.back() != L'\\')) {
		p += std::filesystem::path::preferred_separator;
	}
	return p;
}
#ifdef UNIT_TEST
TEST(FileOperation, UtilPathAddLastSeparator) {
	std::wstring sep;
	sep = std::filesystem::path::preferred_separator;
	EXPECT_EQ(sep, UtilPathAddLastSeparator(L""));
	EXPECT_EQ(L"C:" + sep, UtilPathAddLastSeparator(L"C:"));
	EXPECT_EQ(L"C:\\", UtilPathAddLastSeparator(L"C:\\"));
	EXPECT_EQ(L"C:/", UtilPathAddLastSeparator(L"C:/"));
	EXPECT_EQ(std::wstring(L"/tmp") + sep, UtilPathAddLastSeparator(L"/tmp"));
	EXPECT_EQ(L"/tmp\\", UtilPathAddLastSeparator(L"/tmp\\"));
	EXPECT_EQ(L"/tmp/", UtilPathAddLastSeparator(L"/tmp/"));
}
#endif

std::filesystem::path UtilPathRemoveLastSeparator(const std::filesystem::path& path)
{
	std::wstring p = path;
	if (!p.empty() && (p.back() == L'/' || p.back() == L'\\')) {
		p.back() = L'\0';
	}
	return p.c_str();
}
#ifdef UNIT_TEST
TEST(FileOperation, UtilPathRemoveLastSeparator) {
	EXPECT_EQ(L"", UtilPathRemoveLastSeparator(L""));
	EXPECT_EQ(L"", UtilPathRemoveLastSeparator(L"/"));
	EXPECT_EQ(L"", UtilPathRemoveLastSeparator(L"\\"));
	EXPECT_EQ(L"C:", UtilPathRemoveLastSeparator(L"C:/"));
	EXPECT_EQ(L"C:", UtilPathRemoveLastSeparator(L"C:\\"));
	EXPECT_EQ(L"C:", UtilPathRemoveLastSeparator(L"C:"));
	EXPECT_EQ(L"/tmp", UtilPathRemoveLastSeparator(L"/tmp\\"));
	EXPECT_EQ(L"/tmp", UtilPathRemoveLastSeparator(L"/tmp/"));
	EXPECT_EQ(L"/tmp", UtilPathRemoveLastSeparator(L"/tmp"));
}
#endif


//get full & absolute path
std::filesystem::path UtilGetCompletePathName(const std::filesystem::path& filePath)
{
	if(filePath.empty()){
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

#ifdef UNIT_TEST
TEST(FileOperation, UtilGetCompletePathName) {
	EXPECT_THROW(UtilGetCompletePathName(L""), LF_EXCEPTION);
	EXPECT_TRUE(UtilPathIsRoot(L"C:\\"));
	EXPECT_TRUE(UtilPathIsRoot(UtilGetCompletePathName(L"C:")));
	EXPECT_TRUE(UtilPathIsRoot(UtilGetCompletePathName(L"C:\\")));
	EXPECT_FALSE(UtilPathIsRoot(UtilGetCompletePathName(L"C:\\Windows")));
	auto tempDir = std::filesystem::temp_directory_path();
	EXPECT_FALSE(UtilPathIsRoot(UtilGetCompletePathName(tempDir)));
	{
		CCurrentDirManager mngr(tempDir);
		auto dest = L"C:\\Windows";
		auto relpath = std::filesystem::relative(dest);
		auto expected = toLower(std::filesystem::path(dest).make_preferred());
		auto actual = toLower(std::filesystem::path(UtilGetCompletePathName(relpath)));
		EXPECT_EQ(expected, actual);
	}
}
#endif

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


#ifdef UNIT_TEST
TEST(FileOperation, UtilGetModulePath_UtilGetModuleDirectoryPath) {
	//TODO: is there any better test?
	EXPECT_FALSE(UtilGetModulePath().empty());
	EXPECT_TRUE(std::filesystem::exists(UtilGetModulePath()));
	EXPECT_TRUE(std::filesystem::exists(UtilGetModuleDirectoryPath()));
}
#endif

//read whole file
std::vector<BYTE> UtilReadFile(const std::filesystem::path& filePath, size_t maxSize)
{
	if (filePath.empty())RAISE_EXCEPTION(L"Invalid Argument");
	if(!std::filesystem::exists(filePath))RAISE_EXCEPTION(L"File not found");

	struct _stat64 stat = {};
	if (0 != _wstat64(filePath.c_str(), &stat))RAISE_EXCEPTION(L"Failed to get filesize");
	auto file_size = (size_t)stat.st_size;
	if (maxSize != 0) {
		file_size = std::min(file_size, maxSize);
	}

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

#ifdef UNIT_TEST
TEST(FileOperation, UtilReadFile) {
	//prepare
	auto fname = UtilGetTempPath() / L"lhaforge_test_file.tmp";
	{
		CAutoFile fp;
		fp.open(fname, L"w");
		fprintf(fp, "test file content");
	}
	{
		auto read = UtilReadFile(fname);
		EXPECT_EQ(size_t(17), read.size());
		read.push_back('\0');
		EXPECT_EQ("test file content", std::string((const char*)&read[0]));
	}
	{
		auto read = UtilReadFile(fname, 5);
		EXPECT_EQ(size_t(5), read.size());
		read.push_back('\0');
		EXPECT_EQ("test ", std::string((const char*)&read[0]));
	}
	std::filesystem::remove(fname);
}

#endif

void touchFile(const std::filesystem::path& path)
{
	CAutoFile fp;
	fp.open(path, L"w");
}


#ifdef UNIT_TEST
TEST(FileOperation, touchFile_CAutoFile)
{
	std::filesystem::path dir = UtilGetTempPath() / L"lhaforge_test/touchFile";
	UtilDeletePath(dir);
	EXPECT_FALSE(std::filesystem::exists(dir));
	std::filesystem::create_directories(dir);

	{
		CAutoFile fp;
		auto filename = dir / L"a.txt";
		EXPECT_FALSE(std::filesystem::exists(filename));
		fp.open(filename);
		EXPECT_FALSE(fp.is_opened());

		touchFile(filename);

		EXPECT_TRUE(std::filesystem::exists(filename));
		fp.open(filename);
		EXPECT_TRUE(fp.is_opened());
		EXPECT_EQ(filename, fp.get_path());
	}

	UtilDeletePath(dir);
	EXPECT_FALSE(std::filesystem::exists(dir));
}

#endif

#ifdef UNIT_TEST
TEST(FileOperation, CTemporaryDirectoryManager) {
	std::filesystem::path path;
	{
		CTemporaryDirectoryManager tmpMngr;
		path = tmpMngr.path();
		EXPECT_TRUE(std::filesystem::exists(path));
	}
	EXPECT_FALSE(std::filesystem::exists(path));
}

TEST(FileOperation, CContinuousFile) {
	//create test files
	std::filesystem::path dir = UtilGetTempPath() / L"lhaforge_test/continuousFile";
	UtilDeletePath(dir);
	EXPECT_FALSE(std::filesystem::exists(dir));
	std::filesystem::create_directories(dir);

	std::vector<std::filesystem::path> files;
	for (int i = 0; i < 10; i++) {
		CAutoFile fp;
		auto fname = dir / Format(L"file%02d.txt", i);
		files.push_back(fname);
		fp.open(fname, L"w");
		for (int j = 0; j < 5; j++) {
			fputc('a' + i, fp);
		}
	}

	//test subject
	CContinuousFile cfp;
	EXPECT_FALSE(cfp.is_opened());
	cfp.openFiles(files);
	EXPECT_TRUE(cfp.is_opened());
	EXPECT_EQ(0, cfp.tell());

	char buffer[256];	// 256 > 5*10
	memset(buffer, 0, sizeof(buffer));
	EXPECT_EQ(10, cfp.read(buffer, 10));
	EXPECT_STREQ("aaaaabbbbb", buffer);
	EXPECT_EQ(10, cfp.tell());

	memset(buffer, 0, sizeof(buffer));
	EXPECT_EQ(1, cfp.read(buffer, 1));
	EXPECT_STREQ("c", buffer);
	EXPECT_EQ(11, cfp.tell());

	memset(buffer, 0, sizeof(buffer));
	EXPECT_EQ(1, cfp.read(buffer, 1));
	EXPECT_STREQ("c", buffer);
	EXPECT_EQ(12, cfp.tell());

	memset(buffer, 0, sizeof(buffer));
	EXPECT_EQ(50-12, cfp.read(buffer, 50));
	EXPECT_STREQ("cccdddddeeeeefffffggggghhhhhiiiiijjjjj", buffer);
	EXPECT_EQ(50, cfp.tell());

	//---seek
	memset(buffer, 0, sizeof(buffer));
	cfp.seek(0, SEEK_SET);
	EXPECT_EQ(0, cfp.tell());
	EXPECT_EQ(10, cfp.read(buffer, 10));
	EXPECT_STREQ("aaaaabbbbb", buffer);
	EXPECT_EQ(10, cfp.tell());

	memset(buffer, 0, sizeof(buffer));
	cfp.seek(20, SEEK_CUR);
	EXPECT_EQ(30, cfp.tell());
	EXPECT_EQ(10, cfp.read(buffer, 10));
	EXPECT_STREQ("ggggghhhhh", buffer);
	EXPECT_EQ(40, cfp.tell());

	memset(buffer, 0, sizeof(buffer));
	cfp.seek(-20, SEEK_CUR);
	EXPECT_EQ(20, cfp.tell());
	EXPECT_EQ(10, cfp.read(buffer, 10));
	EXPECT_STREQ("eeeeefffff", buffer);
	EXPECT_EQ(30, cfp.tell());

	memset(buffer, 0, sizeof(buffer));
	cfp.seek(-20, SEEK_END);
	EXPECT_EQ(30, cfp.tell());
	EXPECT_EQ(10, cfp.read(buffer, 10));
	EXPECT_STREQ("ggggghhhhh", buffer);
	EXPECT_EQ(40, cfp.tell());

	cfp.close();

	//cleanup
	UtilDeletePath(dir);
	EXPECT_FALSE(std::filesystem::exists(dir));
}

TEST(FileOperation, CContinuousFile_fail) {
	//create test files
	std::filesystem::path fname = UtilGetTempPath() / L"lhaforge_test/some_non_existing_file";
	EXPECT_FALSE(std::filesystem::exists(fname));

	std::vector<std::filesystem::path> files;
	files.push_back(fname);

	//test subject
	CContinuousFile cfp;
	EXPECT_FALSE(cfp.openFiles(files));
	EXPECT_THROW(cfp.read(nullptr, 0), LF_EXCEPTION);
}

#endif

