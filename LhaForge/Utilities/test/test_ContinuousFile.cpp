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
#include "../ContinuousFile.h"
#include "../FileOperation.h"
#include "../Utility.h"
#include "../StringUtil.h"
#include "../OSUtil.h"
#include "resource.h"


TEST(CContinuousFile, basic) {
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
	//0 -> 10; file00/01
	memset(buffer, 0, sizeof(buffer));
	EXPECT_EQ(10, cfp.read(buffer, 10));
	EXPECT_STREQ("aaaaabbbbb", buffer);
	EXPECT_EQ(10, cfp.tell());

	//10 -> 11; file02
	memset(buffer, 0, sizeof(buffer));
	EXPECT_EQ(1, cfp.read(buffer, 1));
	EXPECT_STREQ("c", buffer);
	EXPECT_EQ(11, cfp.tell());

	//11 -> 12; file02
	memset(buffer, 0, sizeof(buffer));
	EXPECT_EQ(1, cfp.read(buffer, 1));
	EXPECT_STREQ("c", buffer);
	EXPECT_EQ(12, cfp.tell());

	//12 -> 50; file09
	memset(buffer, 0, sizeof(buffer));
	EXPECT_EQ(50-12, cfp.read(buffer, 50));
	EXPECT_STREQ("cccdddddeeeeefffffggggghhhhhiiiiijjjjj", buffer);
	EXPECT_EQ(50, cfp.tell());

	//---seek
	memset(buffer, 0, sizeof(buffer));
	//0; file00
	EXPECT_TRUE(cfp.seek(0, SEEK_SET));
	EXPECT_EQ(0, cfp.tell());
	//0->10; file00/01
	EXPECT_EQ(10, cfp.read(buffer, 10));
	EXPECT_STREQ("aaaaabbbbb", buffer);
	EXPECT_EQ(10, cfp.tell());

	memset(buffer, 0, sizeof(buffer));
	//10->30; end of file05
	cfp.seek(20, SEEK_CUR);
	EXPECT_EQ(30, cfp.tell());
	EXPECT_EQ(10, cfp.read(buffer, 10));
	//30->40; file06/07
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

TEST(CContinuousFile, fail) {
	//create test files
	std::filesystem::path fname = UtilGetTempPath() / L"lhaforge_test/some_non_existing_file";
	EXPECT_FALSE(std::filesystem::exists(fname));

	std::vector<std::filesystem::path> files;
	files.push_back(fname);

	//test subject
	CContinuousFile cfp;
	EXPECT_FALSE(cfp.openFiles(files));
	EXPECT_EQ(0, cfp.read(nullptr, 0));
	EXPECT_TRUE(cfp.is_error());
}

