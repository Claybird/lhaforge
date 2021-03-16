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
#include "CmdLineInfo.h"
#include "CommonUtil.h"
#include "Utilities/FileOperation.h"
#include "Utilities/OSUtil.h"
#include "Utilities/StringUtil.h"
#include "compress.h"
#include "resource.h"

//-----------

struct RAW_CMDLINE{
	std::vector<std::pair<std::wstring, std::wstring>> switches;
	std::vector<std::wstring> files;
};

RAW_CMDLINE getCommandLineArgs(const std::wstring& cmdline)
{
	std::wregex re_switches(L"^(/.+?)(\\:(.*))?$");
	RAW_CMDLINE result;

	int nArgc = 0;
	LPWSTR *lplpArgs = CommandLineToArgvW(cmdline.c_str(), &nArgc);
	std::vector<std::pair<std::wstring, std::wstring> > args;
	for (int i = 1; i < nArgc; i++) {
		//lplpArgs[0] is executable name
		std::wcmatch results;
		if (std::regex_search(lplpArgs[i], results, re_switches)) {
			std::wstring key, value;
			if (results.size() > 1) {
				key = results[1];
			}
			if (results.size() > 3) {
				value = results[3];
			}
			result.switches.push_back(std::make_pair<>(key, value));
		} else {
			result.files.push_back(lplpArgs[i]);
		}
	}
	LocalFree(lplpArgs);
	return result;
}

#ifdef UNIT_TEST
TEST(commandLineInfo, getCommandLineArgs)
{
	auto ret = getCommandLineArgs(L"LhaForge.exe /c:zip /d:c:\\tmp\\data /e /f: c:\\data");
	EXPECT_EQ(4, ret.switches.size());

	if (ret.switches.size() == 4) {
		EXPECT_EQ(L"/c", ret.switches[0].first);
		EXPECT_EQ(L"zip", ret.switches[0].second);

		EXPECT_EQ(L"/d", ret.switches[1].first);
		EXPECT_EQ(L"c:\\tmp\\data", ret.switches[1].second);

		EXPECT_EQ(L"/e", ret.switches[2].first);
		EXPECT_EQ(L"", ret.switches[2].second);

		EXPECT_EQ(L"/f", ret.switches[3].first);
		EXPECT_EQ(L"", ret.switches[3].second);
	}

	EXPECT_EQ(1, ret.files.size());
	EXPECT_EQ(L"c:\\data", ret.files[0]);
}
#endif

struct LF_INVALID_PARAMETER : LF_EXCEPTION {
	std::wstring _parameter;
	LF_INVALID_PARAMETER(const std::wstring& key, const std::wstring& value) :LF_EXCEPTION(L"Invalid") {
		if (value.empty()) {
			_parameter = key;
		} else {
			_parameter = key + L":" + value;
		}
	}
	virtual ~LF_INVALID_PARAMETER() {}
};

std::pair<PROCESS_MODE, CMDLINEINFO> ParseCommandLine(
	const std::wstring& cmdline,
	std::function<void(const std::wstring& msg)> errorHandler)
{
	PROCESS_MODE ProcessMode = PROCESS_AUTOMATIC;

	UTIL_CODEPAGE uCodePage = UTIL_CODEPAGE::UTF8;	//default encoding for response file

	CMDLINEINFO cli;
	auto args = getCommandLineArgs(cmdline);
	cli.FileList = args.files;

	try{
		for (const auto& arg : args.switches) {
			//---------------
			// parse options
			//---------------
			std::wstring key = toLower(arg.first);
			std::wstring value = arg.second;
			if (L"/cfg" == key) {	//configuration file
				if (value.empty()) {
					cli.ConfigPath.clear();
				} else {
					auto envInfo = LF_make_expand_information(nullptr, nullptr);
					cli.ConfigPath = UtilExpandTemplateString(value, envInfo);
				}
			} else if (L"/cp" == key) {//code page for response file
				value = toLower(value);
				if (value.empty()) {
					uCodePage = UTIL_CODEPAGE::UTF8;	//default
				} else {
					auto cp = toLower(value);
					if (cp == L"utf8" || cp == L"utf-8") {
						uCodePage = UTIL_CODEPAGE::UTF8;
					} else if (cp == L"utf16" || cp == L"utf-16" || cp == L"unicode") {
						uCodePage = UTIL_CODEPAGE::UTF16;
					} else if (cp == L"sjis" || cp == L"shiftjis" || cp == L"s-jis" || cp == L"s_jis" || cp == L"cp932") {
						uCodePage = UTIL_CODEPAGE::CP932;
					} else {
						throw LF_INVALID_PARAMETER(arg.first, arg.second);
					}
				}
			} else if (L"/c" == key) {//compress
				value = toLower(value);
				ProcessMode = PROCESS_COMPRESS;
				cli.CompressType = LF_FMT_INVALID;	//format not specified
				if (!value.empty()) {
					//lookup table
					for (const auto &p : g_CompressionCmdParams) {
						if (value == p.name) {
							cli.CompressType = p.Type;
							cli.Options = p.Options;
							break;
						}
					}
					if (-1 == cli.CompressType) {
						throw LF_INVALID_PARAMETER(arg.first, arg.second);
					}
				}
			} else if (L"/e" == key) {//extract
				ProcessMode = PROCESS_EXTRACT;
			} else if (L"/l" == key) {//list mode
				ProcessMode = PROCESS_LIST;
			} else if (L"/t" == key) {//test mode
				ProcessMode = PROCESS_TEST;
			} else if (L"/m" == key) {//select mode
				ProcessMode = PROCESS_MANAGED;
			} else if (L"/s" == key) {//single compression
				cli.bSingleCompression = true;
			} else if (L"/o" == key) {//output directory
				if (value.empty()) {
					cli.OutputToOverride = OUTPUT_TO_DEFAULT;
					cli.OutputDir.clear();
				}else{
					cli.OutputToOverride = OUTPUT_TO_SPECIFIC_DIR;
					cli.OutputDir = value;
				}
			}else if (L"/od" == key) {	//to desktop
				cli.OutputDir.clear();
				cli.OutputToOverride = OUTPUT_TO_DESKTOP;
			} else if (L"/os" == key) {	//same directory as input files
				cli.OutputDir.clear();
				cli.OutputToOverride = OUTPUT_TO_SAME_DIR;
			} else if (L"/oa" == key) {	//ask everytime
				cli.OutputDir.clear();
				cli.OutputToOverride = OUTPUT_TO_ALWAYS_ASK_WHERE;
			} else if (L"/@" == key || L"/$" == key) {//file listed in file
				if (value.empty()) {
					throw LF_INVALID_PARAMETER(arg.first, L"(empty)");
				} else {
					try {
						auto strFile = UtilGetCompletePathName(value);
						auto files = UtilReadFromResponseFile(strFile.c_str(), uCodePage);
						cli.FileList.insert(cli.FileList.end(), files.begin(), files.end());
						if (L"/$" == key) {	//file listed in file; delete after read
							DeleteFileW(strFile.c_str());
						}
					} catch (LF_EXCEPTION) {
						errorHandler(UtilLoadString(IDS_ERROR_READ_RESPONSEFILE));
						return std::make_pair(PROCESS_INVALID, cli);
					}
				}
			} else if (L"/f" == key) {//output file name
				cli.OutputFileName = value;
			} else if (L"/mkdir" == key) {//output directory control
				value = toLower(value);
				if (L"no" == value) {
					cli.CreateDirOverride = CREATE_OUTPUT_DIR_NEVER;
				} else if (L"single" == value) {
					cli.CreateDirOverride = CREATE_OUTPUT_DIR_SINGLE;
				} else if (L"always" == value || value.empty()) {
					cli.CreateDirOverride = CREATE_OUTPUT_DIR_ALWAYS;
				} else {
					throw LF_INVALID_PARAMETER(arg.first, arg.second);
				}
			} else if (L"/popdir" == key) {//ignore top directory on extract
				value = toLower(value);
				if (L"no" == value) {
					cli.IgnoreTopDirOverride = CMDLINEINFO::ACTION::False;
				} else if (L"yes" == value || value.empty()) {
					cli.IgnoreTopDirOverride = CMDLINEINFO::ACTION::True;
				} else {
					throw LF_INVALID_PARAMETER(arg.first, arg.second);
				}
			} else if (L"/delete" == key) {//delete source file after process
				value = toLower(value);
				if (L"no" == value) {
					cli.DeleteAfterProcess = CMDLINEINFO::ACTION::False;
				} else if (L"yes" == value || value.empty()) {
					cli.DeleteAfterProcess = CMDLINEINFO::ACTION::True;
				} else {
					throw LF_INVALID_PARAMETER(arg.first, arg.second);
				}
			} else {	//unknown parameter
				throw LF_INVALID_PARAMETER(arg.first, arg.second);
			}
		}
		//get absolute path of output directory
		if (!cli.OutputDir.empty()) {
			try {
				cli.OutputDir = UtilGetCompletePathName(cli.OutputDir);
			} catch (LF_EXCEPTION) {
				errorHandler(UtilLoadString(IDS_ERROR_FAIL_GET_ABSPATH));
				return std::make_pair(PROCESS_INVALID, cli);
			}
		}
	} catch (const LF_INVALID_PARAMETER &e) {
		auto msg = Format(UtilLoadString(IDS_ERROR_INVALID_PARAMETER), e._parameter.c_str());
		errorHandler(msg);
		return std::make_pair(PROCESS_INVALID, cli);
	}

	{
		//expand filename pattern
		std::vector<std::wstring> tmp;
		for (const auto& item : cli.FileList) {
			auto out = UtilPathExpandWild(item.c_str());
			tmp.insert(tmp.end(), out.begin(), out.end());
		}
		//---get absolute path
		for (auto &item : tmp) {
			std::wstring path;
			try {
				path = UtilGetCompletePathName(item);
			} catch (LF_EXCEPTION) {
				//failed to get absolute path
				errorHandler(UtilLoadString(IDS_ERROR_FAIL_GET_ABSPATH));
				return std::make_pair(PROCESS_INVALID, cli);
			}

			//remove last separator
			UtilPathRemoveLastSeparator(path);

			//update
			item = path;
		}
		cli.FileList = tmp;
	}
	if (cli.FileList.empty()) {
		//no files, then go to configuration
		return std::make_pair(PROCESS_CONFIGURE, cli);
	}

	return std::make_pair(ProcessMode, cli);
}

#ifdef UNIT_TEST
TEST(commandLineInfo, ParseCommandLine)
{
	std::filesystem::path dir = UtilGetTempPath() + L"lhaforge_test/ParseCommandLine";
	UtilDeletePath(dir);
	EXPECT_FALSE(std::filesystem::exists(dir));
	std::filesystem::create_directories(dir);
	EXPECT_TRUE(std::filesystem::exists(dir));

	auto errorHandler = [](const std::wstring& msg) {};

	//specific config file
	{
		auto[mode, cli] = ParseCommandLine(L"LhaForge.exe /cfg:%temp%\\lhaforge.ini", errorHandler);
		EXPECT_EQ(PROCESS_CONFIGURE, mode);
		EXPECT_EQ(std::filesystem::temp_directory_path() / L"lhaforge.ini", std::filesystem::path(cli.ConfigPath).lexically_normal());
	}

	//compression
	{
		for (const auto &p : g_CompressionCmdParams) {
			auto[mode, cli] = ParseCommandLine(Format(L"LhaForge.exe /c:%s %s", p.name.c_str(), dir.c_str()), errorHandler);
			EXPECT_EQ(PROCESS_COMPRESS, mode);
			EXPECT_EQ(1, cli.FileList.size());
			EXPECT_EQ(p.Type, cli.CompressType);
			EXPECT_FALSE(cli.bSingleCompression);
			EXPECT_TRUE(cli.OutputFileName.empty());
		}
		{
			auto[mode, cli] = ParseCommandLine(L"LhaForge.exe /c /s /f:C:\\temp\\test.zip " + dir.wstring(), errorHandler);
			EXPECT_EQ(PROCESS_COMPRESS, mode);
			EXPECT_TRUE(cli.bSingleCompression);
			EXPECT_EQ(L"C:\\temp\\test.zip", cli.OutputFileName);
		}
	}
	//extract
	{
		auto[mode, cli] = ParseCommandLine(L"LhaForge.exe /e " + dir.wstring(), errorHandler);
		EXPECT_EQ(PROCESS_EXTRACT, mode);
	}
	{
		auto[mode, cli] = ParseCommandLine(L"LhaForge.exe /l " + dir.wstring(), errorHandler);
		EXPECT_EQ(PROCESS_LIST, mode);
	}
	{
		auto[mode, cli] = ParseCommandLine(L"LhaForge.exe /t " + dir.wstring(), errorHandler);
		EXPECT_EQ(PROCESS_TEST, mode);
	}
	{
		auto[mode, cli] = ParseCommandLine(L"LhaForge.exe /m " + dir.wstring(), errorHandler);
		EXPECT_EQ(PROCESS_MANAGED, mode);
	}

	//unknown option
	{
		auto[mode, cli] = ParseCommandLine(L"LhaForge.exe /unknown_parameter", errorHandler);
		EXPECT_EQ(PROCESS_INVALID, mode);
	}
	//---file list & code page
	{
		std::filesystem::create_directories(dir / L"work");
		std::vector<std::wstring> files = { L"work/あいうえお.txt", L"work/b.txt", L"work/c.txt", L"work/d.txt" };
		std::wstring responseFile = dir / L"filelist.txt";
		{
			CAutoFile fp;
			fp.open(responseFile, L"w");
			for (const auto &n : files) {
				fprintf(fp, "%s\n", UtilToUTF8(n).c_str());
			}
			fp.close();
		}
		files.push_back(L"work/e.txt");
		for (const auto &n : files) {
			touchFile(dir / n);
		}
		for (auto &n : files) {
			n = (dir / n).make_preferred();
		}

		CCurrentDirManager mngr(dir);
		{
			auto[mode, cli] = ParseCommandLine(Format(L"LhaForge.exe /c:zip /cp:utf-8 /@:%s work/e.txt", responseFile.c_str()), errorHandler);
			EXPECT_EQ(PROCESS_COMPRESS, mode);
			ASSERT_EQ(files.size(), cli.FileList.size());
			EXPECT_EQ(files[0], cli.FileList[1]);
			EXPECT_EQ(files[1], cli.FileList[2]);
			EXPECT_EQ(files[2], cli.FileList[3]);
			EXPECT_EQ(files[3], cli.FileList[4]);
			EXPECT_EQ(files[4], cli.FileList[0]);
			EXPECT_TRUE(std::filesystem::exists(responseFile));
		}
		{
			auto[mode, cli] = ParseCommandLine(Format(L"LhaForge.exe /c:zip /cp:utf-8 /$:%s work/e.txt", responseFile.c_str()), errorHandler);
			EXPECT_EQ(PROCESS_COMPRESS, mode);
			ASSERT_EQ(files.size(), cli.FileList.size());
			EXPECT_EQ(files[0], cli.FileList[1]);
			EXPECT_EQ(files[1], cli.FileList[2]);
			EXPECT_EQ(files[2], cli.FileList[3]);
			EXPECT_EQ(files[3], cli.FileList[4]);
			EXPECT_EQ(files[4], cli.FileList[0]);
			EXPECT_FALSE(std::filesystem::exists(responseFile));
		}
		{
			auto[mode, cli] = ParseCommandLine(Format(L"LhaForge.exe /c:zip work/*.txt", responseFile.c_str()), errorHandler);
			EXPECT_EQ(PROCESS_COMPRESS, mode);
			ASSERT_EQ(files.size(), cli.FileList.size());
			EXPECT_EQ(std::set<std::wstring>(files.begin(), files.end()), std::set<std::wstring>(cli.FileList.begin(), cli.FileList.end()));
		}
	}

	//output directory
	{
		auto[mode, cli] = ParseCommandLine(L"LhaForge.exe /e /o " + dir.wstring(), errorHandler);
		EXPECT_EQ(PROCESS_EXTRACT, mode);
		EXPECT_EQ(OUTPUT_TO_DEFAULT, cli.OutputToOverride);
		EXPECT_TRUE(cli.OutputDir.empty());
		//default values
		EXPECT_EQ(CMDLINEINFO::ACTION::Default, cli.IgnoreTopDirOverride);
		EXPECT_EQ(CMDLINEINFO::ACTION::Default, cli.DeleteAfterProcess);
	}
	{
		auto[mode, cli] = ParseCommandLine(Format(L"LhaForge.exe /e /o:%s %s", UtilGetTempPath().c_str(), dir.c_str()), errorHandler);
		EXPECT_EQ(PROCESS_EXTRACT, mode);
		EXPECT_EQ(OUTPUT_TO_SPECIFIC_DIR, cli.OutputToOverride);
		EXPECT_EQ(UtilGetTempPath(), cli.OutputDir);
	}
	{
		auto[mode, cli] = ParseCommandLine(L"LhaForge.exe /e /od " + dir.wstring(), errorHandler);
		EXPECT_EQ(PROCESS_EXTRACT, mode);
		EXPECT_EQ(OUTPUT_TO_DESKTOP, cli.OutputToOverride);
		EXPECT_TRUE(cli.OutputDir.empty());
	}
	{
		auto[mode, cli] = ParseCommandLine(L"LhaForge.exe /e /os " + dir.wstring(), errorHandler);
		EXPECT_EQ(PROCESS_EXTRACT, mode);
		EXPECT_EQ(OUTPUT_TO_SAME_DIR, cli.OutputToOverride);
		EXPECT_TRUE(cli.OutputDir.empty());
	}
	{
		auto[mode, cli] = ParseCommandLine(L"LhaForge.exe /e /oa " + dir.wstring(), errorHandler);
		EXPECT_EQ(PROCESS_EXTRACT, mode);
		EXPECT_EQ(OUTPUT_TO_ALWAYS_ASK_WHERE, cli.OutputToOverride);
		EXPECT_TRUE(cli.OutputDir.empty());
	}

	//directory control
	{
		auto[mode, cli] = ParseCommandLine(L"LhaForge.exe /e /mkdir " + dir.wstring(), errorHandler);
		EXPECT_EQ(PROCESS_EXTRACT, mode);
		EXPECT_EQ(CREATE_OUTPUT_DIR_ALWAYS, cli.CreateDirOverride);
	}
	{
		auto[mode, cli] = ParseCommandLine(L"LhaForge.exe /e /mkdir:always " + dir.wstring(), errorHandler);
		EXPECT_EQ(PROCESS_EXTRACT, mode);
		EXPECT_EQ(CREATE_OUTPUT_DIR_ALWAYS, cli.CreateDirOverride);
	}
	{
		auto[mode, cli] = ParseCommandLine(L"LhaForge.exe /e /mkdir:no " + dir.wstring(), errorHandler);
		EXPECT_EQ(PROCESS_EXTRACT, mode);
		EXPECT_EQ(CREATE_OUTPUT_DIR_NEVER, cli.CreateDirOverride);
	}
	{
		auto[mode, cli] = ParseCommandLine(L"LhaForge.exe /e /mkdir:single " + dir.wstring(), errorHandler);
		EXPECT_EQ(PROCESS_EXTRACT, mode);
		EXPECT_EQ(CREATE_OUTPUT_DIR_SINGLE, cli.CreateDirOverride);
	}

	{
		auto[mode, cli] = ParseCommandLine(L"LhaForge.exe /e /popdir:no " + dir.wstring(), errorHandler);
		EXPECT_EQ(PROCESS_EXTRACT, mode);
		EXPECT_EQ(CMDLINEINFO::ACTION::False, cli.IgnoreTopDirOverride);
	}
	{
		auto[mode, cli] = ParseCommandLine(L"LhaForge.exe /e /popdir:yes " + dir.wstring(), errorHandler);
		EXPECT_EQ(PROCESS_EXTRACT, mode);
		EXPECT_EQ(CMDLINEINFO::ACTION::True, cli.IgnoreTopDirOverride);
	}
	{
		auto[mode, cli] = ParseCommandLine(L"LhaForge.exe /e /popdir " + dir.wstring(), errorHandler);
		EXPECT_EQ(PROCESS_EXTRACT, mode);
		EXPECT_EQ(CMDLINEINFO::ACTION::True, cli.IgnoreTopDirOverride);
	}

	//delete source file after process
	{
		auto[mode, cli] = ParseCommandLine(L"LhaForge.exe /e /delete " + dir.wstring(), errorHandler);
		EXPECT_EQ(PROCESS_EXTRACT, mode);
		EXPECT_EQ(CMDLINEINFO::ACTION::True, cli.DeleteAfterProcess);
	}
	{
		auto[mode, cli] = ParseCommandLine(L"LhaForge.exe /e /delete:yes " + dir.wstring(), errorHandler);
		EXPECT_EQ(PROCESS_EXTRACT, mode);
		EXPECT_EQ(CMDLINEINFO::ACTION::True, cli.DeleteAfterProcess);
	}
	{
		auto[mode, cli] = ParseCommandLine(L"LhaForge.exe /e /delete:no " + dir.wstring(), errorHandler);
		EXPECT_EQ(PROCESS_EXTRACT, mode);
		EXPECT_EQ(CMDLINEINFO::ACTION::False, cli.DeleteAfterProcess);
	}

	UtilDeletePath(UtilGetTempPath() + L"lhaforge_test/");
	EXPECT_FALSE(std::filesystem::exists(dir));
}
#endif
