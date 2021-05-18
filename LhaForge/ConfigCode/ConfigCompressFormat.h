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
#include "ConfigFile.h"
#include "Utilities/Utility.h"

struct CConfigCompressFormatBase :public IConfigIO {
public:
	const std::wstring section_name;
	//stores key values; default is the first item
	const std::map<std::wstring, std::vector<std::wstring>> key_and_valid_values;

	//params from config
	std::map<std::string, std::string> params;
public:
	CConfigCompressFormatBase(
		const std::wstring &_section_name,
		const std::map<std::wstring, std::vector<std::wstring>>& _knd
	) :section_name(_section_name), key_and_valid_values(_knd) {}
	virtual ~CConfigCompressFormatBase() {}
	virtual void load(const CConfigFile& Config) override{
		for (const auto& ite : key_and_valid_values) {
			const auto &key = ite.first;
			auto key_utf8 = UtilToUTF8(key);

			const auto &valid_values = ite.second;
			const auto &defaultValue = valid_values.front();
			auto value = Config.getText(section_name, key, defaultValue);
			if (isIn(valid_values, value)) {
				//valid
				params[key_utf8] = UtilToUTF8(value);
			} else {
				//use default value
				params[key_utf8] = UtilToUTF8(defaultValue);
			}
		}
	}
	virtual void store(CConfigFile& Config)const override {
		for (const auto& ite : params) {
			auto key = UtilUTF8toUNICODE(ite.first);
			auto value = UtilUTF8toUNICODE(ite.second);
			Config.setValue(section_name, key, value);
		}
	}
};


class CConfigCompressFormatZIP :public CConfigCompressFormatBase {
public:
	CConfigCompressFormatZIP() :CConfigCompressFormatBase(L"format_zip", {
		{L"compression",{
			L"deflate",L"store"}},
		{L"compression-level",{
			L"9",L"8",L"7",L"6",L"5",L"4",L"3",L"2",L"1",L"0"}},
		{L"encryption",{
			L"ZipCrypt",L"aes128",L"aes256"}},
		//"experimental"	don't use this
		//"fakecrc32"	don't use this
		{L"hdrcharset",{
			L"UTF-8",L"CP_ACP"}},
		{L"zip64",{
				L""/*empty is to avoid zip64*/,L"enabled"/*any non-empty string*/}},
	}) {}
	virtual ~CConfigCompressFormatZIP() {}
};

class CConfigCompressFormat7Z :public CConfigCompressFormatBase {
public:
	CConfigCompressFormat7Z() :CConfigCompressFormatBase(L"format_7z", {
		{L"compression",{
			L"deflate",L"store",L"bzip2",L"lzma1",L"lzma2",L"PPMd",}},
		{L"compression-level",{
			L"9",L"8",L"7",L"6",L"5",L"4",L"3",L"2",L"1",L"0"}},
		}) {}
	virtual ~CConfigCompressFormat7Z() {}
};

class CConfigCompressFormatTAR :public CConfigCompressFormatBase {
public:
	CConfigCompressFormatTAR() :CConfigCompressFormatBase(L"format_tar", {
		{L"hdrcharset",{
			L"UTF-8",L"CP_ACP"}},
		}) {}
	virtual ~CConfigCompressFormatTAR() {}
};

class CConfigCompressFormatGZ :public CConfigCompressFormatBase {
public:
	CConfigCompressFormatGZ() :CConfigCompressFormatBase(L"format_gz", {
		{L"compression-level",{
			L"9",L"8",L"7",L"6",L"5",L"4",L"3",L"2",L"1",L"0"}},
		}) {}
	virtual ~CConfigCompressFormatGZ() {}
};

class CConfigCompressFormatBZ2 :public CConfigCompressFormatBase {
public:
	CConfigCompressFormatBZ2() :CConfigCompressFormatBase(L"format_bz2", {
		{L"compression-level",{
			L"9",L"8",L"7",L"6",L"5",L"4",L"3",L"2",L"1",L"0"}},
		}) {}
	virtual ~CConfigCompressFormatBZ2() {}
};

class CConfigCompressFormatXZ :public CConfigCompressFormatBase {
public:
	CConfigCompressFormatXZ() :CConfigCompressFormatBase(L"format_xz", {
		{L"compression-level",{
			L"9",L"8",L"7",L"6",L"5",L"4",L"3",L"2",L"1",L"0"}},
		{L"threads",{
			L"0"/*full cpu cores*/,L"1"/*single*/}},	//can take arbitrary integer, but might not be necessary
		}) {}
	virtual ~CConfigCompressFormatXZ() {}
};

class CConfigCompressFormatLZMA :public CConfigCompressFormatBase {
public:
	CConfigCompressFormatLZMA() :CConfigCompressFormatBase(L"format_lzma", {
		{L"compression-level",{
			L"9",L"8",L"7",L"6",L"5",L"4",L"3",L"2",L"1",L"0"}},
		}) {}
	virtual ~CConfigCompressFormatLZMA() {}
};

class CConfigCompressFormatZSTD :public CConfigCompressFormatBase {
public:
	CConfigCompressFormatZSTD() :CConfigCompressFormatBase(L"format_zstd", {
		{L"compression-level",{
			L"3"/*default*/,L"1"/*fastest*/,L"9"/*high*/,L"15"/*even higher*/,L"22"/*ultra*/}},
		}) {}
	virtual ~CConfigCompressFormatZSTD() {}
};

