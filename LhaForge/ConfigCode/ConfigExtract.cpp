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
#include "ConfigExtract.h"
#include "Utilities/FileOperation.h"
#include "Utilities/Utility.h"
#include "resource.h"

void CConfigExtract::load(const CConfigFile &Config)
{
	const auto section = L"Extract";
	OutputDirType = (OUTPUT_TO)Config.getIntRange(section, L"OutputDirType", 0, OUTPUT_TO_LAST_ITEM, OUTPUT_TO_DESKTOP);

	auto value = Config.getText(section, L"OutputDir", L"");
	if (value.empty()) {
		OutputDirUserSpecified = L"";
	} else {
		try {
			OutputDirUserSpecified = UtilGetCompletePathName(value);
		} catch (const LF_EXCEPTION&) {
			OutputDirUserSpecified = L"";
		}
	}

	OpenDir = Config.getBool(section, L"OpenFolder", true);
	CreateDir = (CREATE_OUTPUT_DIR)Config.getIntRange(section, L"CreateDir", 0, CREATE_OUTPUT_DIR_LAST_ITEM, CREATE_OUTPUT_DIR_ALWAYS);
	ForceOverwrite = Config.getBool(section, L"ForceOverwrite", false);

	RemoveSymbolAndNumber = Config.getBool(section, L"RemoveSymbolAndNumber", false);
	CreateNoFolderIfSingleFileOnly = Config.getBool(section, L"CreateNoFolderIfSingleFileOnly", false);

	LimitExtractFileCount = Config.getBool(section, L"LimitExtractFileCount", false);
	MaxExtractFileCount = std::max(1, Config.getInt(section, L"MaxExtractFileCount", 1));
	DeleteArchiveAfterExtract = Config.getBool(section, L"DeleteArchiveAfterExtract", false);

	MoveToRecycleBin = Config.getBool(section, L"MoveToRecycleBin", true);
	DeleteNoConfirm = Config.getBool(section, L"DeleteNoConfirm", false);
	DenyExt = Config.getText(section, L"DenyExt", UtilLoadString(IDS_DENYEXT_DEFAULT));
}

void CConfigExtract::store(CConfigFile &Config)const
{
	const auto section = L"Extract";
	Config.setValue(section, L"OutputDirType", OutputDirType);

	Config.setValue(section, L"OutputDir", OutputDirUserSpecified);

	Config.setValue(section, L"OpenFolder", OpenDir);
	Config.setValue(section, L"CreateDir", CreateDir);
	Config.setValue(section, L"ForceOverwrite", ForceOverwrite);
	Config.setValue(section, L"RemoveSymbolAndNumber", RemoveSymbolAndNumber);
	Config.setValue(section, L"CreateNoFolderIfSingleFileOnly", CreateNoFolderIfSingleFileOnly);
	Config.setValue(section, L"LimitExtractFileCount", LimitExtractFileCount);
	Config.setValue(section, L"MaxExtractFileCount", MaxExtractFileCount);
	Config.setValue(section, L"DeleteArchiveAfterExtract", DeleteArchiveAfterExtract);
	Config.setValue(section, L"MoveToRecycleBin", MoveToRecycleBin);
	Config.setValue(section, L"DeleteNoConfirm", DeleteNoConfirm);
	Config.setValue(section, L"DenyExt", DenyExt);
}

//checks file extension
bool CConfigExtract::isPathAcceptableToExtract(const std::filesystem::path& path)const
{
	const auto denyList = UtilSplitString(DenyExt, L";");
	for (const auto& deny : denyList) {
		if (UtilExtMatchSpec(path, deny)) {
			return false;
		}
	}
	return true;
}

#ifdef UNIT_TEST
TEST(config, CConfigExtract)
{
	CConfigFile emptyFile;
	CConfigExtract conf;
	conf.load(emptyFile);

	conf.DenyExt = L".docx;;.exe;.zipx";

	EXPECT_TRUE(conf.isPathAcceptableToExtract(L"/path/ext/file.txt"));
	EXPECT_FALSE(conf.isPathAcceptableToExtract(L"/path/ext/file.docx"));
	EXPECT_TRUE(conf.isPathAcceptableToExtract(L"/path/ext/file.zip"));
	EXPECT_FALSE(conf.isPathAcceptableToExtract(L"/path/ext/file.zipx"));
}
#endif
