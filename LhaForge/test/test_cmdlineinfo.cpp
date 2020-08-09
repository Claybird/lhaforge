#include "stdafx.h"
#ifdef UNIT_TEST
#include <gtest/gtest.h>
#include "CmdLineInfo.h"


struct RAW_CMDLINE {
	std::vector<std::pair<std::wstring, std::wstring>> switches;
	std::vector<std::wstring> files;
};

RAW_CMDLINE getCommandLineArgs(const wchar_t* cmdline);

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
	auto [mode, cli] = ParseCommandLine(L"LhaForge.exe /c:zip c:/data");
	EXPECT_EQ(PROCESS_COMPRESS, mode);
	EXPECT_EQ(1, cli.FileList.size());

}

#endif
