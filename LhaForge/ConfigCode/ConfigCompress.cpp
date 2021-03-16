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
#include "ConfigCompress.h"
#include "Utilities/FileOperation.h"

void CConfigCompress::load(const CConfigFile &Config)
{
	const auto section = L"Compress";
	//出力先の種類
	OutputDirType = (OUTPUT_TO)Config.getIntRange(section, L"OutputDirType", 0, OUTPUT_TO_LAST_ITEM, OUTPUT_TO_DESKTOP);
	//出力先のパス
	auto value=Config.getText(section, L"OutputDir", L"");
	if (value.empty()) {
		OutputDirUserSpecified = L"";
	} else {
		try {
			OutputDirUserSpecified = UtilGetCompletePathName(value);
		} catch (const LF_EXCEPTION&) {
			OutputDirUserSpecified = L"";
		}
	}
	//圧縮後フォルダを開くかどうか
	OpenDir=Config.getBool(section, L"OpenFolder", true);

	//出力ファイル名を指定するかどうか
	SpecifyOutputFilename=Config.getBool(section, L"SpecifyName", false);

	//同時に圧縮するファイル数を制限する
	LimitCompressFileCount=Config.getBool(section, L"LimitCompressFileCount", false);

	//同時に圧縮するファイル数の上限
	MaxCompressFileCount = std::max(1, Config.getInt(section, L"MaxCompressFileCount", 0));

	//デフォルト圧縮パラメータを使用するならtrue
	UseDefaultParameter=Config.getBool(section, L"UseDefaultParameter", false);

	//デフォルト圧縮パラメータ(形式指定)
	//TODO DefaultType=(PARAMETER_TYPE)Config.Data[_T("DefaultType")].GetNParam(0,PARAMETER_LAST_ITEM,PARAMETER_UNDEFINED);
	DefaultType = LF_FMT_INVALID;

	//デフォルト圧縮パラメータのオプション
	DefaultOptions=Config.getInt(section, L"DefaultOptions", 0);

	//正常に圧縮できたファイルを削除
	DeleteAfterCompress=Config.getBool(section, L"DeleteAfterCompress", false);
	//圧縮後ファイルをごみ箱に移動
	MoveToRecycleBin=Config.getBool(section, L"MoveToRecycleBin", true);
	//確認せずに削除/ごみ箱に移動
	DeleteNoConfirm=Config.getBool(section, L"DeleteNoConfirm", false);
	//正常処理を確認できない形式でも削除
	ForceDelete=Config.getBool(section, L"ForceDelete", false);

	//「フォルダより下のファイルを圧縮」
	IgnoreTopDirectory=Config.getBool(section, L"IgnoreTopDirectory", false);
}

void CConfigCompress::store(CConfigFile &Config)const
{
	const auto section = L"Compress";
	//出力先の種類
	Config.setValue(section, L"OutputDirType", OutputDirType);
	//出力先のパス
	Config.setValue(section, L"OutputDir", OutputDirUserSpecified);
	//圧縮後フォルダを開くかどうか
	Config.setValue(section, L"OpenFolder", OpenDir);

	//出力ファイル名を指定するかどうか
	Config.setValue(section, L"SpecifyName", SpecifyOutputFilename);

	//同時に圧縮するファイル数を制限する
	Config.setValue(section, L"LimitCompressFileCount", LimitCompressFileCount);

	//同時に圧縮するファイル数の上限
	Config.setValue(section, L"MaxCompressFileCount", MaxCompressFileCount);

	//デフォルト圧縮パラメータを使用するならtrue
	Config.setValue(section, L"UseDefaultParameter", UseDefaultParameter);

	//デフォルト圧縮パラメータ(形式指定)
	Config.setValue(section, L"DefaultType", DefaultType);

	//デフォルト圧縮パラメータのオプション
	Config.setValue(section, L"DefaultOptions", DefaultOptions);

	//正常に圧縮できたファイルを削除
	Config.setValue(section, L"DeleteAfterCompress", DeleteAfterCompress);
	//圧縮後ファイルをごみ箱に移動
	Config.setValue(section, L"MoveToRecycleBin", MoveToRecycleBin);
	//確認せずに削除/ごみ箱に移動
	Config.setValue(section, L"DeleteNoConfirm", DeleteNoConfirm);
	//正常処理を確認できない形式でも削除
	Config.setValue(section, L"ForceDelete", ForceDelete);

	//「フォルダより下のファイルを圧縮」
	Config.setValue(section, L"IgnoreTopDirectory", IgnoreTopDirectory);
}
