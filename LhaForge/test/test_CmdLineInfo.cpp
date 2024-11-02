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

struct RAW_CMDLINE {
	std::vector<std::pair<std::wstring, std::wstring>> switches;
	std::vector<std::filesystem::path> files;
};

RAW_CMDLINE getCommandLineArgs(const std::wstring& cmdline);

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

TEST(commandLineInfo, ParseCommandLine)
{
	std::filesystem::path dir = UtilGetTempPath() / L"lhaforge_test/ParseCommandLine";
	UtilDeletePath(dir);
	EXPECT_FALSE(std::filesystem::exists(dir));
	std::filesystem::create_directories(dir);
	EXPECT_TRUE(std::filesystem::exists(dir));

	auto errorHandler = [](const std::wstring& msg) {};

	//specific config file
	{
		//%temp%/lhaforge.ini does not exist
		EXPECT_FALSE(std::filesystem::exists(std::filesystem::temp_directory_path() / L"lhaforge.ini"));
		auto[mode, cli] = ParseCommandLine(L"LhaForge.exe /cfg:%temp%\\lhaforge.ini", errorHandler);
		EXPECT_EQ(PROCESS_MODE::CONFIGURE, mode);
		EXPECT_EQ(std::filesystem::temp_directory_path() / L"lhaforge.ini", std::filesystem::path(cli.ConfigPath).lexically_normal());
	}

	//compression
	{
		for (const auto &p : g_CompressionCmdParams) {
			auto[mode, cli] = ParseCommandLine(Format(L"LhaForge.exe /c:%s %s", p.name.c_str(), dir.c_str()), errorHandler);
			EXPECT_EQ(PROCESS_MODE::COMPRESS, mode);
			EXPECT_EQ(1, cli.FileList.size());
			EXPECT_EQ(p.Type, cli.CompressType);
			EXPECT_FALSE(cli.bSingleCompression);
			EXPECT_TRUE(cli.OutputFileName.empty());
		}
		{
			auto[mode, cli] = ParseCommandLine(L"LhaForge.exe /c /s /f:C:\\temp\\test.zip " + dir.wstring(), errorHandler);
			EXPECT_EQ(PROCESS_MODE::COMPRESS, mode);
			EXPECT_TRUE(cli.bSingleCompression);
			EXPECT_EQ(L"C:\\temp\\test.zip", cli.OutputFileName);
		}
	}
	//extract
	{
		auto[mode, cli] = ParseCommandLine(L"LhaForge.exe /e " + dir.wstring(), errorHandler);
		EXPECT_EQ(PROCESS_MODE::EXTRACT, mode);
	}
	{
		auto[mode, cli] = ParseCommandLine(L"LhaForge.exe /l " + dir.wstring(), errorHandler);
		EXPECT_EQ(PROCESS_MODE::LIST, mode);
	}
	{
		auto[mode, cli] = ParseCommandLine(L"LhaForge.exe /t " + dir.wstring(), errorHandler);
		EXPECT_EQ(PROCESS_MODE::TEST, mode);
	}
	{
		auto[mode, cli] = ParseCommandLine(L"LhaForge.exe /m " + dir.wstring(), errorHandler);
		EXPECT_EQ(PROCESS_MODE::MANAGED, mode);
	}
	{
		auto [mode, cli] = ParseCommandLine(L"LhaForge.exe " + dir.wstring(), errorHandler);
		EXPECT_EQ(PROCESS_MODE::AUTOMATIC, mode);
	}

	//unknown option
	{
		auto[mode, cli] = ParseCommandLine(L"LhaForge.exe /unknown_parameter", errorHandler);
		EXPECT_EQ(PROCESS_MODE::INVALID, mode);
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
			EXPECT_EQ(PROCESS_MODE::COMPRESS, mode);
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
			EXPECT_EQ(PROCESS_MODE::COMPRESS, mode);
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
			EXPECT_EQ(PROCESS_MODE::COMPRESS, mode);
			ASSERT_EQ(files.size(), cli.FileList.size());
			EXPECT_EQ(std::set<std::wstring>(files.begin(), files.end()), std::set<std::wstring>(cli.FileList.begin(), cli.FileList.end()));
		}
	}

	//output directory
	{
		auto[mode, cli] = ParseCommandLine(L"LhaForge.exe /e /o " + dir.wstring(), errorHandler);
		EXPECT_EQ(PROCESS_MODE::EXTRACT, mode);
		EXPECT_EQ(OUTPUT_TO::NoOverride, cli.OutputToOverride);
		EXPECT_TRUE(cli.OutputDir.empty());
		//default values
		EXPECT_EQ(CMDLINEINFO::ACTION::Default, cli.IgnoreTopDirOverride);
		EXPECT_EQ(CMDLINEINFO::ACTION::Default, cli.DeleteAfterProcess);
	}
	{
		auto[mode, cli] = ParseCommandLine(Format(L"LhaForge.exe /e /o:%s %s", UtilGetTempPath().c_str(), dir.c_str()), errorHandler);
		EXPECT_EQ(PROCESS_MODE::EXTRACT, mode);
		EXPECT_EQ(OUTPUT_TO::SpecificDir, cli.OutputToOverride);
		EXPECT_EQ(UtilGetTempPath(), cli.OutputDir);
	}
	{
		auto[mode, cli] = ParseCommandLine(L"LhaForge.exe /e /od " + dir.wstring(), errorHandler);
		EXPECT_EQ(PROCESS_MODE::EXTRACT, mode);
		EXPECT_EQ(OUTPUT_TO::Desktop, cli.OutputToOverride);
		EXPECT_TRUE(cli.OutputDir.empty());
	}
	{
		auto[mode, cli] = ParseCommandLine(L"LhaForge.exe /e /os " + dir.wstring(), errorHandler);
		EXPECT_EQ(PROCESS_MODE::EXTRACT, mode);
		EXPECT_EQ(OUTPUT_TO::SameDir, cli.OutputToOverride);
		EXPECT_TRUE(cli.OutputDir.empty());
	}
	{
		auto[mode, cli] = ParseCommandLine(L"LhaForge.exe /e /oa " + dir.wstring(), errorHandler);
		EXPECT_EQ(PROCESS_MODE::EXTRACT, mode);
		EXPECT_EQ(OUTPUT_TO::AlwaysAsk, cli.OutputToOverride);
		EXPECT_TRUE(cli.OutputDir.empty());
	}
	//specific config file+output directory
	{
		auto ini = dir / L"lhaforge.ini";
		EXPECT_FALSE(std::filesystem::exists(ini));
		CAutoFile fp;
		fp.open(ini, L"w");
		fwprintf(fp, L"[Extract]\n");	//C:/temp does not need to exist
		fwprintf(fp, L"OutputDir=C:\\some_dummy_dir\n");
		fwprintf(fp, L"OutputDirType=2\n");	//C:/temp does not need to exist
		fp.close();

		// "/cfg" first
		{
			auto [mode, cli] = ParseCommandLine(Format(L"LhaForge.exe /cfg:%s /e /od %s", ini.c_str(), dir.c_str()), errorHandler);
			EXPECT_EQ(PROCESS_MODE::EXTRACT, mode);
			EXPECT_EQ(OUTPUT_TO::Desktop, cli.OutputToOverride);
			EXPECT_TRUE(cli.OutputDir.empty());
			EXPECT_EQ(ini, std::filesystem::path(cli.ConfigPath).lexically_normal());
		}
		{
			auto [mode, cli] = ParseCommandLine(Format(L"LhaForge.exe /cfg:%s /e /o:%s %s", ini.c_str(), UtilGetTempPath().c_str(), dir.c_str()), errorHandler);
			EXPECT_EQ(PROCESS_MODE::EXTRACT, mode);
			EXPECT_EQ(OUTPUT_TO::SpecificDir, cli.OutputToOverride);
			EXPECT_EQ(UtilGetTempPath(), cli.OutputDir);
		}
		// "/cfg" last
		{
			auto [mode, cli] = ParseCommandLine(Format(L"LhaForge.exe /e /od %s /cfg:%s", dir.c_str(), ini.c_str()), errorHandler);
			EXPECT_EQ(PROCESS_MODE::EXTRACT, mode);
			EXPECT_EQ(OUTPUT_TO::Desktop, cli.OutputToOverride);
			EXPECT_TRUE(cli.OutputDir.empty());
			EXPECT_EQ(ini, std::filesystem::path(cli.ConfigPath).lexically_normal());
		}
		{
			auto [mode, cli] = ParseCommandLine(Format(L"LhaForge.exe /e /o:%s %s /cfg:%s", UtilGetTempPath().c_str(), dir.c_str(), ini.c_str()), errorHandler);
			EXPECT_EQ(PROCESS_MODE::EXTRACT, mode);
			EXPECT_EQ(OUTPUT_TO::SpecificDir, cli.OutputToOverride);
			EXPECT_EQ(UtilGetTempPath(), cli.OutputDir);
		}
		UtilDeletePath(ini);
		EXPECT_FALSE(std::filesystem::exists(ini));
	}

	//directory control
	{
		auto[mode, cli] = ParseCommandLine(L"LhaForge.exe /e /mkdir " + dir.wstring(), errorHandler);
		EXPECT_EQ(PROCESS_MODE::EXTRACT, mode);
		EXPECT_EQ(EXTRACT_CREATE_DIR::Always, cli.CreateDirOverride);
	}
	{
		auto[mode, cli] = ParseCommandLine(L"LhaForge.exe /e /mkdir:always " + dir.wstring(), errorHandler);
		EXPECT_EQ(PROCESS_MODE::EXTRACT, mode);
		EXPECT_EQ(EXTRACT_CREATE_DIR::Always, cli.CreateDirOverride);
	}
	{
		auto[mode, cli] = ParseCommandLine(L"LhaForge.exe /e /mkdir:no " + dir.wstring(), errorHandler);
		EXPECT_EQ(PROCESS_MODE::EXTRACT, mode);
		EXPECT_EQ(EXTRACT_CREATE_DIR::Never, cli.CreateDirOverride);
	}
	{
		auto[mode, cli] = ParseCommandLine(L"LhaForge.exe /e /mkdir:single " + dir.wstring(), errorHandler);
		EXPECT_EQ(PROCESS_MODE::EXTRACT, mode);
		EXPECT_EQ(EXTRACT_CREATE_DIR::SkipIfSingleFileOrDir, cli.CreateDirOverride);
	}
	{
		auto [mode, cli] = ParseCommandLine(L"LhaForge.exe /e /mkdir:singled " + dir.wstring(), errorHandler);
		EXPECT_EQ(PROCESS_MODE::EXTRACT, mode);
		EXPECT_EQ(EXTRACT_CREATE_DIR::SkipIfSingleDirectory, cli.CreateDirOverride);
	}

	{
		auto[mode, cli] = ParseCommandLine(L"LhaForge.exe /e /popdir:no " + dir.wstring(), errorHandler);
		EXPECT_EQ(PROCESS_MODE::EXTRACT, mode);
		EXPECT_EQ(CMDLINEINFO::ACTION::False, cli.IgnoreTopDirOverride);
	}
	{
		auto[mode, cli] = ParseCommandLine(L"LhaForge.exe /e /popdir:yes " + dir.wstring(), errorHandler);
		EXPECT_EQ(PROCESS_MODE::EXTRACT, mode);
		EXPECT_EQ(CMDLINEINFO::ACTION::True, cli.IgnoreTopDirOverride);
	}
	{
		auto[mode, cli] = ParseCommandLine(L"LhaForge.exe /e /popdir " + dir.wstring(), errorHandler);
		EXPECT_EQ(PROCESS_MODE::EXTRACT, mode);
		EXPECT_EQ(CMDLINEINFO::ACTION::True, cli.IgnoreTopDirOverride);
	}

	//delete source file after process
	{
		auto[mode, cli] = ParseCommandLine(L"LhaForge.exe /e /delete " + dir.wstring(), errorHandler);
		EXPECT_EQ(PROCESS_MODE::EXTRACT, mode);
		EXPECT_EQ(CMDLINEINFO::ACTION::True, cli.DeleteAfterProcess);
	}
	{
		auto[mode, cli] = ParseCommandLine(L"LhaForge.exe /e /delete:yes " + dir.wstring(), errorHandler);
		EXPECT_EQ(PROCESS_MODE::EXTRACT, mode);
		EXPECT_EQ(CMDLINEINFO::ACTION::True, cli.DeleteAfterProcess);
	}
	{
		auto[mode, cli] = ParseCommandLine(L"LhaForge.exe /e /delete:no " + dir.wstring(), errorHandler);
		EXPECT_EQ(PROCESS_MODE::EXTRACT, mode);
		EXPECT_EQ(CMDLINEINFO::ACTION::False, cli.DeleteAfterProcess);
	}

	UtilDeletePath(UtilGetTempPath() / L"lhaforge_test/");
	EXPECT_FALSE(std::filesystem::exists(dir));
}
