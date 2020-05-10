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

#pragma once
//設定ファイル操作系

#include "variant.h"

//from http://ml.tietew.jp/cppll/cppll/article/12168
struct less_ignorecase {
	bool operator()(const stdString& a, const stdString& b) const {
		return _tcsicmp(a.c_str(), b.c_str()) < 0;
	}
};

//---セクション構造のない設定;データは「Key=Value」
typedef std::map<stdString,CVariant,less_ignorecase> FLATCONFIG;
//---セクションごとに分かれたデータ
struct[[deprecated("will be replaced")]] CONFIG_SECTION{	//設定ファイルのセクションのデータ
	virtual ~CONFIG_SECTION(){}
	stdString SectionName;	//セクションの名前
	FLATCONFIG Data;
};

//設定ファイルの読み込み
bool UtilReadSectionedConfig(LPCTSTR,std::list<CONFIG_SECTION>&,CString &strErr);

//設定ファイルの書き込み
bool UtilWriteSectionedConfig(LPCTSTR,const std::list<CONFIG_SECTION>&,CString &strErr);

void UtilDumpFlatConfig(const FLATCONFIG&);

//INIに数字を文字列として書き込む
[[deprecated("will be removed")]]
BOOL UtilWritePrivateProfileInt(LPCTSTR lpAppName, LPCTSTR lpKeyName, LONG nData, LPCTSTR lpFileName);

