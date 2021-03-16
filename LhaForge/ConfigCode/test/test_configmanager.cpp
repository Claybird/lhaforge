#include "stdafx.h"
#ifdef UNIT_TEST
#include <gtest/gtest.h>
#include "ConfigCode/ConfigFile.h"
#include "Utilities/FileOperation.h"

TEST(ConfigManager, basic)
{
	CConfigFile conf;
	conf.setDefaultPath();
	EXPECT_EQ(std::filesystem::path(conf.m_iniPath).filename(), L"LhaForge.ini");

	auto tmpPath = UtilGetTemporaryFileName();
	EXPECT_TRUE(std::filesystem::exists(tmpPath));
	{
		CAutoFile fp;
		fp.open(tmpPath, L"w");
		fwprintf(fp,
			L"[section1]\n"
			L"key1=value1\n"
			L"key2=2\n"
			L"key3=1\n"
			L"[Section2]\n"
			L"Key1=true\n"
			L"key2=5.67\n"
		);
		fp.close();

		conf.setPath(tmpPath);
		EXPECT_EQ(conf.m_iniPath, tmpPath);
		EXPECT_FALSE(conf.isUserCommon());
		conf.load();
		EXPECT_TRUE(conf.hasSection(L"section1"));
		EXPECT_TRUE(conf.hasSection(L"section2"));
		EXPECT_TRUE(conf.hasSection(L"Section1"));
		EXPECT_TRUE(conf.hasSection(L"Section2"));
		EXPECT_FALSE(conf.hasSection(L"Section3"));

		EXPECT_TRUE(conf.getBool(L"section1", L"key1", true));
		EXPECT_FALSE(conf.getBool(L"section1", L"key1", false));
		EXPECT_FALSE(conf.getBool(L"section1", L"key2", false));
		EXPECT_TRUE(conf.getBool(L"section1", L"key3", false));
		EXPECT_TRUE(conf.getBool(L"section2", L"key1", false));
		EXPECT_FALSE(conf.getBool(L"section2", L"key2", false));

		EXPECT_EQ(-1, conf.getInt(L"section1", L"key1", -1));
		EXPECT_EQ(2, conf.getInt(L"section1", L"key2", -1));
		EXPECT_EQ(-1, conf.getInt(L"section2", L"key2", -1));

		EXPECT_EQ(5, conf.getIntRange(L"section1", L"key2", 5, 6, 0));
		EXPECT_EQ(-1, conf.getIntRange(L"section1", L"key2", -2, -1, 0));

		EXPECT_EQ(5.67, conf.getDouble(L"section2", L"key2", 0.0));

		EXPECT_EQ(L"value1", conf.getText(L"section1", L"key1", L""));


		conf.deleteSection(L"Section3");	//nothing should happen
		conf.deleteSection(L"Section2");
		EXPECT_TRUE(conf.hasSection(L"Section1"));
		EXPECT_FALSE(conf.hasSection(L"Section2"));
		EXPECT_FALSE(conf.hasSection(L"Section3"));

		conf.setValue(L"section3", L"key_added", false);
		EXPECT_FALSE(conf.getBool(L"section3", L"key_added", true));
		conf.setValue(L"section3", L"key_added", 2);
		EXPECT_EQ(2, conf.getInt(L"section3", L"key_added", -1));
		conf.setValue(L"section3", L"key_added", 1.5);
		EXPECT_EQ(1.5, conf.getDouble(L"section3", L"key_added", 0.0));
		conf.setValue(L"section3", L"key_added", L"abc");
		EXPECT_EQ(L"abc", conf.getText(L"section3", L"key_added", L""));
	}
	UtilDeletePath(tmpPath);
	EXPECT_FALSE(std::filesystem::exists(tmpPath));
}

TEST(ConfigManager, get_set)
{
	auto path = UtilGetTemporaryFileName();
	CConfigFile conf;
	conf.setPath(path);

	//empty data set
	conf.setValue(L"section", L"key", L"");
	EXPECT_EQ(L"", conf.getText(L"section", L"key", L"abc"));

	UtilDeletePath(path);
	EXPECT_FALSE(std::filesystem::exists(path));
}

#endif

