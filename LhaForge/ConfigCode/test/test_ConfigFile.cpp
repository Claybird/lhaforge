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
#include "../ConfigFile.h"
#include "Utilities/FileOperation.h"
#include "resource.h"

#include "Utilities/FileOperation.h"

TEST(ConfigFile, basic)
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

		int64_t large_value = 1099511627776L;
		conf.setValue(L"int64",L"int64_t", large_value);
		EXPECT_EQ(large_value, conf.getInt64(L"int64", L"int64_t", 0));
	}
	UtilDeletePath(tmpPath);
	EXPECT_FALSE(std::filesystem::exists(tmpPath));
}

TEST(ConfigFile, get_set)
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

