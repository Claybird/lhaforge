#include "stdafx.h"
#include "Dlg_format.h"


inline std::wstring mapConfigKeyToHumanReadableName(
	const std::wstring& section_name,
	const wchar_t* key,
	const wchar_t* value)
{
	std::wstring map_name = section_name;
	if (key) {
		map_name += L"/";
		map_name += key;
		if (value) {
			map_name += L"/";
			map_name += value;
		}
	}

	const std::map<std::wstring, std::wstring> name_map = {
		{L"format_zip",L"ZIP"},
		{L"format_zip/compression",L"Compression method"},
		{L"format_zip/compression/deflate",L"Deflate"},
		{L"format_zip/compression/store",L"Store(No compression)"},
		{L"format_zip/compression-level",L"Compression Level"},
		//{L"format_zip/compression-level/{level}"},	default name
		{L"format_zip/encryption",L"Encryption method"},
		{L"format_zip/encryption/ZipCrypt",L"ZipCrypt"},
		{L"format_zip/encryption/aes128",L"AES128"},
		{L"format_zip/encryption/aes256",L"AES256"},
		{L"format_zip/hdrcharset",L"Charset"},
		{L"format_zip/hdrcharset/UTF-8",L"UTF-8"},
		{L"format_zip/hdrcharset/CP_ACP",L"System Local(Not recommended)"},
		{L"format_zip/zip64",L"Force ZIP64"},
		{L"format_zip/zip64/enabled",L"On"},
		{L"format_zip/zip64/",L"Off"},	//value is "", zero length string
		//---
		{L"format_7z",L"7z"},
		{L"format_7z/compression",L"Compression method"},
		{L"format_7z/compression/deflate",L"Deflate"},
		{L"format_7z/compression/store",L"Store(No compression)"},
		{L"format_7z/compression/bzip2",L"BZIP2"},
		{L"format_7z/compression/lzma1",L"LZMA1"},
		{L"format_7z/compression/lzma2",L"LZMA2"},
		{L"format_7z/compression/PPMd",L"PPMd"},
		{L"format_7z/compression-level",L"Compression Level"},
		//{L"format_7z/compression-level/{level}"},	default name
		//---
		{L"format_tar",L"TAR"},
		{L"format_tar/hdrcharset",L"Charset"},
		{L"format_tar/hdrcharset/UTF-8",L"UTF-8"},
		{L"format_tar/hdrcharset/CP_ACP",L"System Local(Not recommended)"},
		//---
		{L"format_gz",L"gzip"},
		{L"format_gz/compression-level",L"Compression Level"},
		//{L"format_gz/compression-level/{level}",}, default name
		//---
		{L"format_bz2",L"bzip2"},
		{L"format_bz2/compression-level",L"Compression Level"},
		//{L"format_bz2/compression-level/{level}",}, default name
		//---
		{L"format_xz",L"xz"},
		{L"format_xz/compression-level",L"Compression Level"},
		//{L"format_xz/compression-level/{level}",}, default name
		{L"format_xz/threads",L"Multithread process"},
		{L"format_xz/threads/0",L"Use full cpu"},
		{L"format_xz/threads/1",L"Single thread"},
		//---
		{L"format_lzma",L"lzma"},
		{L"format_lzma/compression-level",L"Compression Level"},
		//{L"format_lzma/compression-level/{level}",}, default name
		//---
		{L"format_zstd",L"ZStandard"},
		{L"format_zstd/compression-level",L"Compression Level"},
		{L"format_zstd/compression-level/3",L"Default"},
		{L"format_zstd/compression-level/1",L"Fastest"},
		{L"format_zstd/compression-level/9",L"High"},
		{L"format_zstd/compression-level/15",L"Even higher"},
		{L"format_zstd/compression-level/22",L"Ultra"},
	};

	auto ite = name_map.find(map_name);
	if (ite == name_map.end()) {
		//fallback
		if (key) {
			if (value) return value;
			else return key;
		} else return section_name;
	} else {
		return (*ite).second;
	}
}


LRESULT CConfigDlgFormat::OnInitDialog(HWND hWnd, LPARAM lParam)
{
	_listView.SetSubjectWindow(GetDlgItem(IDC_LIST_BASE));
	clearData();
	for (auto& c : _configs) {
		CLFComboListViewCtrl::CONTENT_DATA separator = {
			mapConfigKeyToHumanReadableName(c->section_name,nullptr,nullptr),
			{},-1,nullptr };
		_data.push_back(separator);

		for (const auto& item : c->key_and_valid_values) {
			auto key = UtilToUTF8(item.first);
			const auto& options = item.second;
			int curSel = index_of(options, UtilUTF8toUNICODE(c->params[key]));
			curSel = std::max(0, curSel);

			std::vector<std::wstring> mapped_options = item.second;
			for (auto& mo : mapped_options) {
				mo = mapConfigKeyToHumanReadableName(c->section_name, item.first.c_str(), mo.c_str());
			}

			auto userData = new INTERNAL_DATA{ c,key };
			CLFComboListViewCtrl::CONTENT_DATA d = {
				mapConfigKeyToHumanReadableName(c->section_name,item.first.c_str(),nullptr),
				mapped_options,
				curSel,
				userData
			};
			_data.push_back(d);
		}
	}

	_listView.SetContentData(
		UtilLoadString(IDS_CONFIG_FORMAT_KEY).c_str(),
		UtilLoadString(IDS_CONFIG_FORMAT_VALUE).c_str(),
		_data);

	return TRUE;
}
