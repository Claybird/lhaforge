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

	const std::map<std::wstring, UINT> name_map = {
		{L"format_zip",IDS_FORMAT_NAME_ZIP},
		{L"format_zip/compression",IDS_COMPRESS_METHOD},
		{L"format_zip/compression/deflate",IDS_COMPRESS_METHOD_DEFLATE},
		{L"format_zip/compression/store",IDS_COMPRESS_METHOD_STORE},
		{L"format_zip/compression-level",IDS_COMPRESS_LEVEL},
		//{L"format_zip/compression-level/{level}"},	default name
		{L"format_zip/encryption",IDS_ENCRYPTION_METHOD},
		{L"format_zip/encryption/ZipCrypt",IDS_ENCRYPTION_ZIPCRYPT},
		{L"format_zip/encryption/aes128",IDS_ENCRYPTION_AES128},
		{L"format_zip/encryption/aes256",IDS_ENCRYPTION_AES256},
		{L"format_zip/hdrcharset",IDS_COMPRESS_CHARSET},
		{L"format_zip/hdrcharset/UTF-8",IDS_COMPRESS_CHARSET_UTF8},
		{L"format_zip/hdrcharset/CP_ACP",IDS_COMPRESS_CHARSET_ACP},
		{L"format_zip/zip64",IDS_COMPRESSION_FORCE_ZIP64},
		{L"format_zip/zip64/enabled",IDS_COMPRESSION_FORCE_ZIP64_ON},
		{L"format_zip/zip64/",IDS_COMPRESSION_FORCE_ZIP64_OFF},	//value is "", zero length string
		//---
		{L"format_7z",IDS_FORMAT_NAME_7Z},
		{L"format_7z/compression",IDS_COMPRESS_METHOD},
		{L"format_7z/compression/deflate",IDS_COMPRESS_METHOD_DEFLATE},
		{L"format_7z/compression/store",IDS_COMPRESS_METHOD_STORE},
		{L"format_7z/compression/bzip2",IDS_COMPRESS_METHOD_BZIP2},
		{L"format_7z/compression/lzma1",IDS_COMPRESS_METHOD_LZMA1},
		{L"format_7z/compression/lzma2",IDS_COMPRESS_METHOD_LZMA2},
		{L"format_7z/compression/PPMd",IDS_COMPRESS_METHOD_PPMD},
		{L"format_7z/compression-level",IDS_COMPRESS_LEVEL},
		//{L"format_7z/compression-level/{level}"},	default name
		//---
		{L"format_tar",IDS_FORMAT_NAME_TAR},
		{L"format_tar/hdrcharset",IDS_COMPRESS_CHARSET},
		{L"format_tar/hdrcharset/UTF-8",IDS_COMPRESS_CHARSET_UTF8},
		{L"format_tar/hdrcharset/CP_ACP",IDS_COMPRESS_CHARSET_ACP},
		//---
		{L"format_gz",IDS_FORMAT_NAME_GZ},
		{L"format_gz/compression-level",IDS_COMPRESS_LEVEL},
		//{L"format_gz/compression-level/{level}",}, default name
		//---
		{L"format_bz2",IDS_FORMAT_NAME_BZ2},
		{L"format_bz2/compression-level",IDS_COMPRESS_LEVEL},
		//{L"format_bz2/compression-level/{level}",}, default name
		//---
		{L"format_xz",IDS_FORMAT_NAME_XZ},
		{L"format_xz/compression-level",IDS_COMPRESS_LEVEL},
		//{L"format_xz/compression-level/{level}",}, default name
		{L"format_xz/threads",IDS_COMPRESS_MULTI_THREAD},
		{L"format_xz/threads/0",IDS_COMPRESS_MT_ALL_CORES},
		{L"format_xz/threads/1",IDS_COMPRESS_MT_SINGLE_THREAD},
		//---
		{L"format_lzma",IDS_FORMAT_NAME_LZMA},
		{L"format_lzma/compression-level",IDS_COMPRESS_LEVEL},
		//{L"format_lzma/compression-level/{level}",}, default name
		//---
		{L"format_zstd",IDS_FORMAT_NAME_ZSTD},
		{L"format_zstd/compression-level",IDS_COMPRESS_LEVEL},
		{L"format_zstd/compression-level/3",IDS_COMPRESS_LEVEL_DEFAULT},
		{L"format_zstd/compression-level/1",IDS_COMPRESS_LEVEL_FASTEST},
		{L"format_zstd/compression-level/9",IDS_COMPRESS_LEVEL_HIGH},
		{L"format_zstd/compression-level/15",IDS_COMPRESS_LEVEL_EVEN_HIGHER},
		{L"format_zstd/compression-level/22",IDS_COMPRESS_LEVEL_ULTRA},
	};

	auto ite = name_map.find(map_name);
	if (ite == name_map.end()) {
		//fallback
		if (key) {
			if (value) return value;
			else return key;
		} else return section_name;
	} else {
		return UtilLoadString((*ite).second);
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
