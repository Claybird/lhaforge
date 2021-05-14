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
#include "ArchiverCode/archive.h"
#include "ConfigFile.h"
#include "Compress.h"
#include "ConfigCompress.h"
#include "Utilities/FileOperation.h"

void CConfigCompress::load(const CConfigFile &Config)
{
	const auto section = L"Compress";
	OutputDirType = (OUTPUT_TO)Config.getIntRange(section, L"OutputDirType", 0, OUTPUT_TO_LAST_ITEM, OUTPUT_TO_DESKTOP);
	auto value=Config.getText(section, L"OutputDir", L"");
	if (value.empty()) {
		OutputDirUserSpecified = L"";
	} else {
		try {
			OutputDirUserSpecified = UtilGetCompletePathName(value);
		} catch (const LF_EXCEPTION&) {
			OutputDirUserSpecified = L"";
		}
	}
	OpenDir=Config.getBool(section, L"OpenFolder", true);

	SpecifyOutputFilename=Config.getBool(section, L"SpecifyName", false);
	LimitCompressFileCount=Config.getBool(section, L"LimitCompressFileCount", false);
	MaxCompressFileCount = std::max(1, Config.getInt(section, L"MaxCompressFileCount", 0));

	UseDefaultParameter=Config.getBool(section, L"UseDefaultParameter", false);
	DefaultType = (LF_ARCHIVE_FORMAT)Config.getIntRange(section,
		L"DefaultType", LF_FMT_INVALID, LF_ARCHIVE_FORMAT_LAST_ITEM, LF_FMT_INVALID);
	DefaultOptions=Config.getInt(section, L"DefaultOptions", 0);

	DeleteAfterCompress=Config.getBool(section, L"DeleteAfterCompress", false);
	MoveToRecycleBin=Config.getBool(section, L"MoveToRecycleBin", true);
	DeleteNoConfirm=Config.getBool(section, L"DeleteNoConfirm", false);

	IgnoreTopDirectory = Config.getIntRange(section,
		L"IgnoreTopDirectory", 0, (int)COMPRESS_IGNORE_TOP_DIR::LastItem, (int)COMPRESS_IGNORE_TOP_DIR::None);
}

void CConfigCompress::store(CConfigFile &Config)const
{
	const auto section = L"Compress";
	Config.setValue(section, L"OutputDirType", OutputDirType);
	Config.setValue(section, L"OutputDir", OutputDirUserSpecified);
	Config.setValue(section, L"OpenFolder", OpenDir);

	Config.setValue(section, L"SpecifyName", SpecifyOutputFilename);
	Config.setValue(section, L"LimitCompressFileCount", LimitCompressFileCount);
	Config.setValue(section, L"MaxCompressFileCount", MaxCompressFileCount);
	Config.setValue(section, L"UseDefaultParameter", UseDefaultParameter);
	Config.setValue(section, L"DefaultType", DefaultType);
	Config.setValue(section, L"DefaultOptions", DefaultOptions);
	Config.setValue(section, L"DeleteAfterCompress", DeleteAfterCompress);
	Config.setValue(section, L"MoveToRecycleBin", MoveToRecycleBin);
	Config.setValue(section, L"DeleteNoConfirm", DeleteNoConfirm);
	Config.setValue(section, L"IgnoreTopDirectory", (int)IgnoreTopDirectory);
}

#ifdef UNIT_TEST
TEST(config, CConfigCompress)
{
	CConfigFile emptyFile;
	CConfigCompress conf;
	conf.load(emptyFile);

	EXPECT_EQ(OUTPUT_TO_DESKTOP, conf.OutputDirType);
	EXPECT_TRUE(conf.OutputDirUserSpecified.empty());
	EXPECT_TRUE(conf.OpenDir);
	EXPECT_FALSE(conf.SpecifyOutputFilename);
	EXPECT_FALSE(conf.LimitCompressFileCount);
	EXPECT_EQ(1, conf.MaxCompressFileCount);
	EXPECT_FALSE(conf.UseDefaultParameter);
	EXPECT_EQ(LF_ARCHIVE_FORMAT::LF_FMT_INVALID, conf.DefaultType);
	EXPECT_EQ(0, conf.DefaultOptions);

	EXPECT_FALSE(conf.DeleteAfterCompress);
	EXPECT_TRUE(conf.MoveToRecycleBin);
	EXPECT_FALSE(conf.DeleteNoConfirm);

	EXPECT_EQ((int)COMPRESS_IGNORE_TOP_DIR::None, conf.IgnoreTopDirectory);
}
#endif
