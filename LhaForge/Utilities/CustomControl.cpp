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
#include "CustomControl.h"

#ifdef UNIT_TEST
TEST(CLFBytesEdit, ParseSize) {
	EXPECT_EQ(-1LL, CLFBytesEdit::ParseSize(L"aaa"));
	EXPECT_EQ(-1LL, CLFBytesEdit::ParseSize(L"b"));
	EXPECT_EQ(1LL, CLFBytesEdit::ParseSize(L"1b"));
	EXPECT_EQ(1LL, CLFBytesEdit::ParseSize(L"1B"));

	EXPECT_EQ(1024LL, CLFBytesEdit::ParseSize(L"1k"));
	EXPECT_EQ(1024LL, CLFBytesEdit::ParseSize(L"1kb"));
	EXPECT_EQ(1024LL, CLFBytesEdit::ParseSize(L"1K"));
	EXPECT_EQ(1024LL, CLFBytesEdit::ParseSize(L"1KB"));

	EXPECT_EQ(1024LL * 1024LL, CLFBytesEdit::ParseSize(L"1m"));
	EXPECT_EQ(1024LL * 1024LL, CLFBytesEdit::ParseSize(L"1mb"));
	EXPECT_EQ(1024LL * 1024LL, CLFBytesEdit::ParseSize(L"1M"));
	EXPECT_EQ(1024LL * 1024LL, CLFBytesEdit::ParseSize(L"1MB"));

	EXPECT_EQ(1024LL * 1024LL * 1024LL, CLFBytesEdit::ParseSize(L"1g"));
	EXPECT_EQ(1024LL * 1024LL * 1024LL, CLFBytesEdit::ParseSize(L"1gb"));
	EXPECT_EQ(1024LL * 1024LL * 1024LL, CLFBytesEdit::ParseSize(L"1G"));
	EXPECT_EQ(1024LL * 1024LL * 1024LL, CLFBytesEdit::ParseSize(L"1GB"));

	EXPECT_EQ(1024LL * 1024LL * 1024LL * 1024LL, CLFBytesEdit::ParseSize(L"1t"));
	EXPECT_EQ(1024LL * 1024LL * 1024LL * 1024LL, CLFBytesEdit::ParseSize(L"1tb"));
	EXPECT_EQ(1024LL * 1024LL * 1024LL * 1024LL, CLFBytesEdit::ParseSize(L"1T"));
	EXPECT_EQ(1024LL * 1024LL * 1024LL * 1024LL, CLFBytesEdit::ParseSize(L"1TB"));

	EXPECT_EQ(1024LL * 1024LL * 1024LL * 1024LL * 1024LL, CLFBytesEdit::ParseSize(L"1p"));
	EXPECT_EQ(1024LL * 1024LL * 1024LL * 1024LL * 1024LL, CLFBytesEdit::ParseSize(L"1pb"));
	EXPECT_EQ(1024LL * 1024LL * 1024LL * 1024LL * 1024LL, CLFBytesEdit::ParseSize(L"1P"));
	EXPECT_EQ(1024LL * 1024LL * 1024LL * 1024LL * 1024LL, CLFBytesEdit::ParseSize(L"1PB"));

	EXPECT_EQ(1024LL * 1024LL * 1024LL * 1024LL * 1024LL * 1024LL, CLFBytesEdit::ParseSize(L"1e"));
	EXPECT_EQ(1024LL * 1024LL * 1024LL * 1024LL * 1024LL * 1024LL, CLFBytesEdit::ParseSize(L"1eb"));
	EXPECT_EQ(1024LL * 1024LL * 1024LL * 1024LL * 1024LL * 1024LL, CLFBytesEdit::ParseSize(L"1E"));
	EXPECT_EQ(1024LL * 1024LL * 1024LL * 1024LL * 1024LL * 1024LL, CLFBytesEdit::ParseSize(L"1EB"));

	EXPECT_EQ(1024LL * 1024LL * 1024LL * 1024LL * 1024LL * 1024LL, CLFBytesEdit::ParseSize(L"1024p"));
}
#endif
