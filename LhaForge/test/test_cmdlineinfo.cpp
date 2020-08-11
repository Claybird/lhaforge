#include "stdafx.h"
#ifdef UNIT_TEST
#include <gtest/gtest.h>
#include "CmdLineInfo.h"
#include "compress.h"


struct RAW_CMDLINE {
	std::vector<std::pair<std::wstring, std::wstring>> switches;
	std::vector<std::wstring> files;
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
	std::filesystem::path dir = UtilGetTempPath() + L"lhaforge_test/ParseCommandLine";
	UtilDeletePath(dir);
	EXPECT_FALSE(std::filesystem::exists(dir));
	std::filesystem::create_directories(dir);
	EXPECT_TRUE(std::filesystem::exists(dir));

	auto errorHandler = [](const std::wstring& msg) {};

	for(const auto &p:g_CompressionCmdParams){
		auto[mode, cli] = ParseCommandLine(Format(L"LhaForge.exe /c:%s %s", p.name, dir.c_str()), errorHandler);
		EXPECT_EQ(PROCESS_COMPRESS, mode);
		EXPECT_EQ(1, cli.FileList.size());
		EXPECT_EQ(p.Type, cli.CompressType);
	}
	{
		auto[mode, cli] = ParseCommandLine(L"LhaForge.exe /cfg:%temp%\\lhaforge.ini", errorHandler);
		EXPECT_EQ(PROCESS_CONFIGURE, mode);
		EXPECT_EQ(std::filesystem::temp_directory_path() / L"lhaforge.ini", std::filesystem::path(cli.ConfigPath).lexically_normal());
	}

	//unknown option
	{
		auto[mode, cli] = ParseCommandLine(L"LhaForge.exe /unknown_parameter", errorHandler);
		EXPECT_EQ(PROCESS_INVALID, mode);
	}
	//---file list
	{
		std::vector<std::wstring> files = { L"a.txt", L"b.txt", L"c.txt", L"d.txt" };
		std::wstring responseFile = dir / L"filelist.txt";
		{
			CAutoFile fp;
			fp.open(responseFile, L"w");
			for (const auto &n : files) {
				fprintf(fp, "%s\n", UtilToUTF8(n).c_str());
			}
			fp.close();
		}
		files.push_back(L"e.txt");
		for (const auto &n : files) {
			touchFile(dir / n);
		}
		for (auto &n : files) {
			n = (dir / n).make_preferred();
		}

		CCurrentDirManager mngr(dir);
		{
			auto[mode, cli] = ParseCommandLine(Format(L"LhaForge.exe /c:zip /cp:utf-8 /@:%s e.txt", responseFile.c_str()), errorHandler);
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
			auto[mode, cli] = ParseCommandLine(Format(L"LhaForge.exe /c:zip /cp:utf-8 /$:%s e.txt", responseFile.c_str()), errorHandler);
			EXPECT_EQ(PROCESS_COMPRESS, mode);
			ASSERT_EQ(files.size(), cli.FileList.size());
			EXPECT_EQ(files[0], cli.FileList[1]);
			EXPECT_EQ(files[1], cli.FileList[2]);
			EXPECT_EQ(files[2], cli.FileList[3]);
			EXPECT_EQ(files[3], cli.FileList[4]);
			EXPECT_EQ(files[4], cli.FileList[0]);
			EXPECT_FALSE(std::filesystem::exists(responseFile));
		}
	}

	UtilDeletePath(UtilGetTempPath() + L"lhaforge_test/");
	EXPECT_FALSE(std::filesystem::exists(dir));
}

#endif
