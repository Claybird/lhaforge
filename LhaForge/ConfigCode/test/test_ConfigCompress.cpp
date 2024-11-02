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
#include "../ConfigFile.h"
#include "Compress.h"
#include "../ConfigCompress.h"
#include "Utilities/FileOperation.h"


TEST(config, CConfigCompress)
{
	CConfigFile emptyFile;
	CConfigCompress conf;
	conf.load(emptyFile);

	EXPECT_EQ((int)OUTPUT_TO::Desktop, conf.OutputDirType);
	EXPECT_TRUE(conf.OutputDirUserSpecified.empty());
	EXPECT_TRUE(conf.OpenDir);
	EXPECT_FALSE(conf.SpecifyOutputFilename);
	EXPECT_FALSE(conf.LimitCompressFileCount);
	EXPECT_EQ(1, conf.MaxCompressFileCount);
	EXPECT_FALSE(conf.UseDefaultParameter);
	EXPECT_EQ(LF_ARCHIVE_FORMAT::INVALID, conf.DefaultType);
	EXPECT_EQ(0, conf.DefaultOptions);

	EXPECT_FALSE(conf.DeleteAfterCompress);
	EXPECT_TRUE(conf.MoveToRecycleBin);
	EXPECT_FALSE(conf.DeleteNoConfirm);

	EXPECT_EQ((int)COMPRESS_IGNORE_TOP_DIR::None, conf.IgnoreTopDirectory);
}
