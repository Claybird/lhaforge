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
struct CConfigExtract:public IConfigConverter{
public:
	OUTPUT_TO OutputDirType;
	CString OutputDirUserSpecified;
	BOOL OpenDir;
	CREATE_OUTPUT_DIR CreateDir;
	BOOL ForceOverwrite;
	BOOL RemoveSymbolAndNumber;
	BOOL CreateNoFolderIfSingleFileOnly;
	BOOL LimitExtractFileCount;
	int MaxExtractFileCount;
	BOOL DeleteArchiveAfterExtract;	//正常に解凍できた圧縮ファイルを削除
	BOOL MoveToRecycleBin;			//解凍後ファイルをごみ箱に移動
	BOOL DeleteNoConfirm;			//確認せずに削除/ごみ箱に移動
	BOOL ForceDelete;				//解凍エラーを検知できない場合も削除
	BOOL DeleteMultiVolume;			//マルチボリュームもまとめて削除
	BOOL MinimumPasswordRequest;	//パスワード入力回数を最小にするならTRUE

	CString DenyExt;		//解凍対象から外す拡張子
protected:
	virtual void load(CONFIG_SECTION&);	//設定をCONFIG_SECTIONから読み込む
	virtual void store(CONFIG_SECTION&)const;	//設定をCONFIG_SECTIONに書き込む
public:
	virtual ~CConfigExtract(){}
	virtual void load(CConfigManager&);
	virtual void store(CConfigManager&)const;
};

