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
	//params from config
	struct ELEMENT {
		const std::wstring key;
		const std::vector<std::wstring> valid_values;
		std::wstring value;
		std::wstring operator()()const {
			if (value.empty()) {
				return valid_values.front();
			} else {
				return value;
			}
		}
	};
	virtual std::vector<const ELEMENT*> params()const = 0;
	virtual std::vector<ELEMENT*> params() = 0;
public:
	const std::wstring section_name;
public:
	CConfigCompressFormatBase(
		const std::wstring &_section_name
	) :section_name(_section_name) {}
	virtual ~CConfigCompressFormatBase() {}

	virtual void load(const CConfigFile& Config) override{
		for (auto& p : params()) {
			const auto &key = p->key;

			const auto &valid_values = p->valid_values;
			const auto &defaultValue = valid_values.front();
			auto value = Config.getText(section_name, key, defaultValue);
			if (isIn(valid_values, value)) {
				//valid
				p->value = value;
			} else {
				//use default value
				p->value = defaultValue;
			}
		}
	}
	virtual void store(CConfigFile& Config)const override {
		for (const auto& p : params()) {
			auto key = p->key;
			auto value = p->value;
			Config.setValue(section_name, key, value);
		}
	}
	virtual std::map<std::string, std::string> as_dict()const {	//for libarchive
		std::map<std::string, std::string> dict;
		for (const auto& p : params()) {
			dict[UtilToUTF8(p->key)] = UtilToUTF8((*p)());
		}
		return dict;
	}
};


class CConfigCompressFormatZIP :public CConfigCompressFormatBase {
public:
	ELEMENT compression;
	ELEMENT compression_level;
	ELEMENT encryption;
	ELEMENT zip64;
public:
	CConfigCompressFormatZIP() :CConfigCompressFormatBase(L"format_zip"),
		compression{ L"compression", {
			L"deflate",L"bzip2",L"lzma",L"zstd",L"xz",L"store" } },
		compression_level{ L"compression-level",{
			L"9",L"8",L"7",L"6",L"5",L"4",L"3",L"2",L"1",L"0"} },
		encryption{ L"encryption",{
			L"zipcrypto",L"aes256",L"aes192",L"aes128"} },
		zip64{ L"zip64",{
			L"auto",L"force",L"disable"} }
	{}
	virtual ~CConfigCompressFormatZIP() {}
	virtual std::vector<const ELEMENT*> params()const override { return { &compression, &compression_level, &encryption, &zip64}; }
	virtual std::vector<ELEMENT*> params()override { return { &compression, &compression_level, &encryption, &zip64 }; }
};

class CConfigCompressFormat7Z :public CConfigCompressFormatBase {
public:
	ELEMENT compression;
	ELEMENT compression_level;
public:
	CConfigCompressFormat7Z() :CConfigCompressFormatBase(L"format_7z"),
		compression{ L"compression",{
			L"deflate",L"store",L"bzip2",L"lzma1",L"lzma2",L"PPMd",} },
		compression_level{ L"compression-level",{
			L"9",L"8",L"7",L"6",L"5",L"4",L"3",L"2",L"1",L"0"} }
	{}
	virtual ~CConfigCompressFormat7Z() {}
	virtual std::vector<const ELEMENT*> params()const override { return { &compression, &compression_level }; }
	virtual std::vector<ELEMENT*> params()override { return { &compression, &compression_level }; }
};

class CConfigCompressFormatTAR :public CConfigCompressFormatBase {
public:
	ELEMENT hdrcharset;
public:
	CConfigCompressFormatTAR() :CConfigCompressFormatBase(L"format_tar"),
		hdrcharset{ L"hdrcharset",{L"UTF-8",L"CP_ACP"} }
	{}
	virtual ~CConfigCompressFormatTAR() {}
	virtual std::vector<const ELEMENT*> params()const override { return { &hdrcharset }; }
	virtual std::vector<ELEMENT*> params()override { return { &hdrcharset }; }
};

class CConfigCompressFormatGZ :public CConfigCompressFormatBase {
public:
	ELEMENT compression_level;
public:
	CConfigCompressFormatGZ() :CConfigCompressFormatBase(L"format_gz"),
		compression_level{ L"compression-level",{
			L"9",L"8",L"7",L"6",L"5",L"4",L"3",L"2",L"1",L"0"} }
	{}
	virtual ~CConfigCompressFormatGZ() {}
	virtual std::vector<const ELEMENT*> params()const override { return { &compression_level }; }
	virtual std::vector<ELEMENT*> params()override { return { &compression_level }; }
};

class CConfigCompressFormatBZ2 :public CConfigCompressFormatBase {
public:
	ELEMENT compression_level;
public:
	CConfigCompressFormatBZ2() :CConfigCompressFormatBase(L"format_bz2"),
		compression_level{ L"compression-level",{
			L"9",L"8",L"7",L"6",L"5",L"4",L"3",L"2",L"1",L"0"} }
	{}
	virtual ~CConfigCompressFormatBZ2() {}
	virtual std::vector<const ELEMENT*> params()const override { return { &compression_level }; }
	virtual std::vector<ELEMENT*> params()override { return { &compression_level }; }
};

class CConfigCompressFormatXZ :public CConfigCompressFormatBase {
public:
	ELEMENT compression_level;
	ELEMENT threads;
public:
	CConfigCompressFormatXZ() :CConfigCompressFormatBase(L"format_xz"),
		compression_level{ L"compression-level",{
			L"9",L"8",L"7",L"6",L"5",L"4",L"3",L"2",L"1",L"0"} },
		threads{ L"threads",{
			L"0"/*full cpu cores*/,L"1"/*single*/},	//can take arbitrary integer, but might not be necessary
	}
	{}
	virtual ~CConfigCompressFormatXZ() {}
	virtual std::vector<const ELEMENT*> params()const override { return { &compression_level,&threads }; }
	virtual std::vector<ELEMENT*> params()override { return { &compression_level,&threads }; }
};

class CConfigCompressFormatLZMA :public CConfigCompressFormatBase {
public:
	ELEMENT compression_level;
public:
	CConfigCompressFormatLZMA() :CConfigCompressFormatBase(L"format_lzma"),
		compression_level{ L"compression-level",{
			L"9",L"8",L"7",L"6",L"5",L"4",L"3",L"2",L"1",L"0"} }
	{}
	virtual ~CConfigCompressFormatLZMA() {}
	virtual std::vector<const ELEMENT*> params()const override { return { &compression_level }; }
	virtual std::vector<ELEMENT*> params()override { return { &compression_level }; }
};

class CConfigCompressFormatZSTD :public CConfigCompressFormatBase {
public:
	ELEMENT compression_level;
public:
	CConfigCompressFormatZSTD() :CConfigCompressFormatBase(L"format_zstd"),
		compression_level{ L"compression-level",{
			L"3"/*default*/,L"1"/*fastest*/,L"9"/*high*/,L"15"/*even higher*/,L"22"/*ultra*/} }
	{}
	virtual ~CConfigCompressFormatZSTD() {}
	virtual std::vector<const ELEMENT*> params()const override { return { &compression_level }; }
	virtual std::vector<ELEMENT*> params()override { return { &compression_level }; }
};

class CConfigCompressFormatLZ4 :public CConfigCompressFormatBase {
public:
	ELEMENT compression_level;
public:
	CConfigCompressFormatLZ4() :CConfigCompressFormatBase(L"format_lz4"),
		compression_level{ L"compression-level",{
			L"1",L"2",L"3",L"4",L"5",L"6",L"7",L"8",L"9"} }
	{}
	virtual ~CConfigCompressFormatLZ4() {}
	virtual std::vector<const ELEMENT*> params()const override { return { &compression_level }; }
	virtual std::vector<ELEMENT*> params()override { return { &compression_level }; }
};

