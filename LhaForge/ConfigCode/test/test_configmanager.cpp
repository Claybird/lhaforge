#include "stdafx.h"
#ifdef UNIT_TEST
#include <gtest/gtest.h>
#include "ConfigCode/ConfigManager.h"
#include "Utilities/FileOperation.h"

TEST(ConfigManager, basic)
{
	CConfigManager conf;
	conf.setDefaultPath();
	ASSERT_EQ(std::filesystem::path(conf.m_iniPath).filename(), L"LhaForge.ini");

	auto tmpPath = UtilGetTemporaryFileName();
	ASSERT_TRUE(std::filesystem::exists(tmpPath));
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
		ASSERT_EQ(conf.m_iniPath, tmpPath);
		ASSERT_FALSE(conf.isUserCommon());
		conf.load();
		ASSERT_TRUE(conf.hasSection(L"section1"));
		ASSERT_TRUE(conf.hasSection(L"section2"));
		ASSERT_TRUE(conf.hasSection(L"Section1"));
		ASSERT_TRUE(conf.hasSection(L"Section2"));
		ASSERT_FALSE(conf.hasSection(L"Section3"));

		ASSERT_TRUE(conf.getBool(L"section1", L"key1", true));
		ASSERT_FALSE(conf.getBool(L"section1", L"key1", false));
		ASSERT_FALSE(conf.getBool(L"section1", L"key2", false));
		ASSERT_TRUE(conf.getBool(L"section1", L"key3", false));
		ASSERT_TRUE(conf.getBool(L"section2", L"key1", false));
		ASSERT_FALSE(conf.getBool(L"section2", L"key2", false));

		ASSERT_EQ(-1, conf.getInt(L"section1", L"key1", -1));
		ASSERT_EQ(2, conf.getInt(L"section1", L"key2", -1));
		ASSERT_EQ(-1, conf.getInt(L"section2", L"key2", -1));

		ASSERT_EQ(5, conf.getIntRange(L"section1", L"key2", 5, 6, 0));
		ASSERT_EQ(-1, conf.getIntRange(L"section1", L"key2", -2, -1, 0));

		ASSERT_EQ(5.67, conf.getDouble(L"section2", L"key2", 0.0));

		ASSERT_EQ(L"value1", conf.getText(L"section1", L"key1", L""));


		conf.deleteSection(L"Section3");	//nothing should happen
		conf.deleteSection(L"Section2");
		ASSERT_TRUE(conf.hasSection(L"Section1"));
		ASSERT_FALSE(conf.hasSection(L"Section2"));
		ASSERT_FALSE(conf.hasSection(L"Section3"));

		conf.setValue(L"section3", L"key_added", false);
		ASSERT_FALSE(conf.getBool(L"section3", L"key_added", true));
		conf.setValue(L"section3", L"key_added", 2);
		ASSERT_EQ(2, conf.getInt(L"section3", L"key_added", -1));
		conf.setValue(L"section3", L"key_added", 1.5);
		ASSERT_EQ(1.5, conf.getDouble(L"section3", L"key_added", 0.0));
		conf.setValue(L"section3", L"key_added", L"abc");
		ASSERT_EQ(L"abc", conf.getText(L"section3", L"key_added", L""));
	}
	UtilDeletePath(tmpPath);
	ASSERT_FALSE(std::filesystem::exists(tmpPath));
}

TEST(ConfigManager, get_set)
{
	CConfigManager conf;
	conf.setPath(UtilGetTemporaryFileName());

	//empty data set
	conf.setValue(L"section", L"key", L"");
	ASSERT_EQ(L"", conf.getText(L"section", L"key", L"abc"));
}

#endif

