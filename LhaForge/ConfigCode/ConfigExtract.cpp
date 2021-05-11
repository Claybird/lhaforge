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

	//解凍後フォルダを開くかどうか
	OpenDir = Config.getBool(section, L"OpenFolder", true);

	//解凍時フォルダを二重にするかどうか
	CreateDir = (CREATE_OUTPUT_DIR)Config.getIntRange(section, L"CreateDir", 0, CREATE_OUTPUT_DIR_LAST_ITEM, CREATE_OUTPUT_DIR_ALWAYS);

	//解凍時ファイルを確認せずに上書きするかどうか
	ForceOverwrite = Config.getBool(section, L"ForceOverwrite", false);

	//解凍時フォルダ名から数字と記号を削除する
	RemoveSymbolAndNumber = Config.getBool(section, L"RemoveSymbolAndNumber", false);

	//解凍時ファイル・フォルダが一つだけの時フォルダを作らない
	CreateNoFolderIfSingleFileOnly = Config.getBool(section, L"CreateNoFolderIfSingleFileOnly", false);

	//同時に解凍するファイル数を制限する
	LimitExtractFileCount = Config.getBool(section, L"LimitExtractFileCount", false);

	//同時に解凍するファイル数の上限
	MaxExtractFileCount = std::max(1, Config.getInt(section, L"MaxExtractFileCount", 1));

	//正常に解凍できた圧縮ファイルを削除
	DeleteArchiveAfterExtract = Config.getBool(section, L"DeleteArchiveAfterExtract", false);

	//解凍後ファイルをごみ箱に移動
	MoveToRecycleBin = Config.getBool(section, L"MoveToRecycleBin", true);

	//確認せずにアーカイブを削除/ごみ箱に移動
	DeleteNoConfirm = Config.getBool(section, L"DeleteNoConfirm", false);

	//解凍エラーを検知できない場合も削除
	ForceDelete = Config.getBool(section, L"ForceDelete", false);

	//パスワード入力回数を最小にするならTRUE
	MinimumPasswordRequest = Config.getBool(section, L"MinimumPasswordRequest", false);

	//解凍対象から外す拡張子
	DenyExt = Config.getText(section, L"DenyExt", UtilLoadString(IDS_DENYEXT_DEFAULT));
}

void CConfigExtract::store(CConfigFile &Config)const
{
	const auto section = L"Extract";
	Config.setValue(section, L"OutputDirType", OutputDirType);

	Config.setValue(section, L"OutputDir", OutputDirUserSpecified);

	//解凍後フォルダを開くかどうか
	Config.setValue(section, L"OpenFolder", OpenDir);

	//解凍時フォルダを二重にするかどうか
	Config.setValue(section, L"CreateDir", CreateDir);

	//解凍時ファイルを確認せずに上書きするかどうか
	Config.setValue(section, L"ForceOverwrite", ForceOverwrite);

	//解凍時フォルダ名から数字と記号を削除する
	Config.setValue(section, L"RemoveSymbolAndNumber", RemoveSymbolAndNumber);

	//解凍時ファイル・フォルダが一つだけの時フォルダを作らない
	Config.setValue(section, L"CreateNoFolderIfSingleFileOnly", CreateNoFolderIfSingleFileOnly);

	//同時に解凍するファイル数を制限する
	Config.setValue(section, L"LimitExtractFileCount", LimitExtractFileCount);

	//同時に解凍するファイル数の上限
	Config.setValue(section, L"MaxExtractFileCount", MaxExtractFileCount);

	//正常に解凍できた圧縮ファイルを削除
	Config.setValue(section, L"DeleteArchiveAfterExtract", DeleteArchiveAfterExtract);

	//解凍後ファイルをごみ箱に移動
	Config.setValue(section, L"MoveToRecycleBin", MoveToRecycleBin);

	//確認せずにアーカイブを削除/ごみ箱に移動
	Config.setValue(section, L"DeleteNoConfirm", DeleteNoConfirm);

	//解凍エラーを検知できない場合も削除
	Config.setValue(section, L"ForceDelete", ForceDelete);

	//パスワード入力回数を最小にするならTRUE
	Config.setValue(section, L"MinimumPasswordRequest", MinimumPasswordRequest);

	//解凍対象から外す拡張子
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
