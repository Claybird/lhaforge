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

enum OUTPUT_TO;
enum LF_ARCHIVE_FORMAT;
struct CConfigCompress:public IConfigIO{
public:
	OUTPUT_TO OutputDirType;
	std::wstring OutputDirUserSpecified;
	bool OpenDir;
	bool SpecifyOutputFilename;
	bool LimitCompressFileCount;	//同時に圧縮するファイルの数を限定するならtrue
	int MaxCompressFileCount;		//同時に圧縮するファイルの数の上限
	bool UseDefaultParameter;	//デフォルト圧縮パラメータを使用するならtrue
	LF_ARCHIVE_FORMAT DefaultType;	//デフォルト圧縮パラメータ(形式指定)
	int DefaultOptions;			//デフォルト圧縮パラメータのオプション

	bool DeleteAfterCompress;	//正常に圧縮できたファイルを削除
	bool MoveToRecycleBin;		//圧縮後ファイルをごみ箱に移動
	bool DeleteNoConfirm;		//確認せずに削除/ごみ箱に移動
	bool ForceDelete;			//正常処理を確認できない形式でも削除

	bool IgnoreTopDirectory;	//「フォルダより下のファイルを圧縮」
	bool IgnoreTopDirectoryRecursively;	//「再帰的にフォルダより下のファイルを圧縮」:TODO
public:
	virtual ~CConfigCompress(){}
	virtual void load(const CConfigManager&);
	virtual void store(CConfigManager&)const;
};

