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
enum CREATE_OUTPUT_DIR;
struct CConfigExtract:public IConfigIO{
public:
	OUTPUT_TO OutputDirType;
	std::wstring OutputDirUserSpecified;
	bool OpenDir;
	CREATE_OUTPUT_DIR CreateDir;
	bool ForceOverwrite;
	bool RemoveSymbolAndNumber;
	bool CreateNoFolderIfSingleFileOnly;
	bool LimitExtractFileCount;
	int MaxExtractFileCount;
	bool DeleteArchiveAfterExtract;	//正常に解凍できた圧縮ファイルを削除
	bool MoveToRecycleBin;			//解凍後ファイルをごみ箱に移動
	bool DeleteNoConfirm;			//確認せずに削除/ごみ箱に移動
	bool ForceDelete;				//解凍エラーを検知できない場合も削除
	bool MinimumPasswordRequest;	//パスワード入力回数を最小にするならTRUE

	std::wstring DenyExt;		//解凍対象から外す拡張子
public:
	virtual ~CConfigExtract(){}
	virtual void load(const CConfigManager&);
	virtual void store(CConfigManager&)const;
};

