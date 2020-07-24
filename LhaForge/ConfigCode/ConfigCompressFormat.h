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
#include "ConfigManager.h"

struct CConfigCompressFormatBase :public IConfigConverter {
public:
	const std::wstring section_name;
	//stores key values; default is the first item
	const std::map<std::string, std::vector<std::string>> key_and_valid_values;
	std::map<std::string, std::string> params;
protected:
	virtual void load(CONFIG_SECTION& config) override {
		for (const auto& ite : key_and_valid_values) {
			auto key = ite.first;
			auto data_ite = config.Data.find(UtilUTF8toUNICODE(key.c_str(), key.length()));
			if (data_ite != config.Data.end()) {
				auto value = UtilToUTF8((const wchar_t*)(*data_ite).second);
				if (isIn(ite.second, value)) {
					//invalid
					params[key] = value;
				} else {
					//use default value
					params[key] = ite.second.front();
				}
			} else {
				//use default value
				params[key] = ite.second.front();
			}
		}
	}
	virtual void store(CONFIG_SECTION& config)const override {
		for (const auto& ite : params) {
			auto key = ite.first;
			config.Data[UtilUTF8toUNICODE(key)] = UtilUTF8toUNICODE(ite.second).c_str();
		}
	}
public:
	CConfigCompressFormatBase(
		const std::wstring &_section_name,
		const std::map<std::string, std::vector<std::string>>& _knd
	) :section_name(_section_name), key_and_valid_values(_knd) {}
	virtual ~CConfigCompressFormatBase() {}
	virtual void load(CConfigManager& mngr) override{
		load(mngr.GetSection(section_name.c_str()));
	}
	virtual void store(CConfigManager& mngr)const override {
		store(mngr.GetSection(section_name.c_str()));
	}
};


class CConfigCompressFormatZIP :public CConfigCompressFormatBase {
public:
	CConfigCompressFormatZIP() :CConfigCompressFormatBase(L"format_zip", {
		{"compression",{
			"deflate","store"}},
		{"compression-level",{
			"9","8","7","6","5","4","3","2","1","0"}},
		{"encryption",{
			"ZipCrypt","aes128","aes256"}},
		//"experimental"	don't use this
		//"fakecrc32"	don't use this
		{"hdrcharset",{
			"UTF-8","CP_ACP"}},
		{"zip64",{
				"enabled"/*any non-empty string*/,""/*empty is to avoid zip64*/}},
	}) {}
	virtual ~CConfigCompressFormatZIP() {}
};

class CConfigCompressFormat7Z :public CConfigCompressFormatBase {
public:
	CConfigCompressFormat7Z() :CConfigCompressFormatBase(L"format_7z", {
		{"compression",{
			"deflate","store","bzip2","lzma1","lzma2","PPMd",}},
		{"compression-level",{
			"9","8","7","6","5","4","3","2","1","0"}},
		}) {}
	virtual ~CConfigCompressFormat7Z() {}
};

class CConfigCompressFormatTAR :public CConfigCompressFormatBase {
public:
	CConfigCompressFormatTAR() :CConfigCompressFormatBase(L"format_tar", {
		{"hdrcharset",{
			"UTF-8","CP_ACP"}},
		}) {}
	virtual ~CConfigCompressFormatTAR() {}
};

class CConfigCompressFormatGZ :public CConfigCompressFormatBase {
public:
	CConfigCompressFormatGZ() :CConfigCompressFormatBase(L"format_gz", {
		{"compression-level",{
			"9","8","7","6","5","4","3","2","1","0"}},
		}) {}
	virtual ~CConfigCompressFormatGZ() {}
};

class CConfigCompressFormatBZ2 :public CConfigCompressFormatBase {
public:
	CConfigCompressFormatBZ2() :CConfigCompressFormatBase(L"format_bz2", {
		{"compression-level",{
			"9","8","7","6","5","4","3","2","1","0"}},
		}) {}
	virtual ~CConfigCompressFormatBZ2() {}
};

class CConfigCompressFormatXZ :public CConfigCompressFormatBase {
public:
	CConfigCompressFormatXZ() :CConfigCompressFormatBase(L"format_xz", {
		{"compression-level",{
			"9","8","7","6","5","4","3","2","1","0"}},
		{"threads",{
			"0"/*full cpu cores*/,"1"/*single*/}},	//can take arbitrary integer, but might not be necessary
		}) {}
	virtual ~CConfigCompressFormatXZ() {}
};

class CConfigCompressFormatLZMA :public CConfigCompressFormatBase {
public:
	CConfigCompressFormatLZMA() :CConfigCompressFormatBase(L"format_lzma", {
		{"compression-level",{
			"9","8","7","6","5","4","3","2","1","0"}},
		}) {}
	virtual ~CConfigCompressFormatLZMA() {}
};

class CConfigCompressFormatZSTD :public CConfigCompressFormatBase {
public:
	CConfigCompressFormatZSTD() :CConfigCompressFormatBase(L"format_zstd", {
		{"compression-level",{
			"3"/*default*/,"1"/*fastest*/,"9"/*high*/,"15"/*even higher*/,"22"/*ultra*/}},
		}) {}
	virtual ~CConfigCompressFormatZSTD() {}
};

/*
is this necessary?

class CConfigCompressFormats :public IConfigConverter
{
	CConfigCompressFormat7Z _fmt7z;
	CConfigCompressFormatBZ2 _fmtbz2;
	CConfigCompressFormatGZ _fmtgz;
	CConfigCompressFormatLZMA _fmtlzma;
	CConfigCompressFormatTAR _fmttar;
	CConfigCompressFormatXZ _fmtxz;
	CConfigCompressFormatZIP _fmtzip;
	CConfigCompressFormatZSTD _fmtzstd;
protected:
	virtual void load(CONFIG_SECTION& config) override {}
	virtual void store(CONFIG_SECTION& config)const override {}
public:
	CConfigCompressFormats() {}
	virtual ~CConfigCompressFormats() {}
	virtual void load(CConfigManager& mngr) override {
		_fmt7z.load(mngr);
		_fmtbz2.load(mngr);
		_fmtgz.load(mngr);
		_fmtlzma.load(mngr);
		_fmttar.load(mngr);
		_fmtxz.load(mngr);
		_fmtzip.load(mngr);
		_fmtzstd.load(mngr);
	}
	virtual void store(CConfigManager& mngr)const override {
		_fmt7z.store(mngr);
		_fmtbz2.store(mngr);
		_fmtgz.store(mngr);
		_fmtlzma.store(mngr);
		_fmttar.store(mngr);
		_fmtxz.store(mngr);
		_fmtzip.store(mngr);
		_fmtzstd.store(mngr);
	}
};*/
