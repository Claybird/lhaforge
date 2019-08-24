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
enum PARAMETER_TYPE;
struct CConfigCompress:public IConfigConverter{
public:
	OUTPUT_TO OutputDirType;
	CString OutputDir;
	BOOL OpenDir;
	BOOL SpecifyOutputFilename;
	BOOL LimitCompressFileCount;	//同時に圧縮するファイルの数を限定するならtrue
	int MaxCompressFileCount;		//同時に圧縮するファイルの数の上限
	BOOL UseDefaultParameter;	//デフォルト圧縮パラメータを使用するならtrue
	PARAMETER_TYPE DefaultType;	//デフォルト圧縮パラメータ(形式指定)
	int DefaultOptions;			//デフォルト圧縮パラメータのオプション
	CString DefaultB2EFormat;
	CString DefaultB2EMethod;

	BOOL DeleteAfterCompress;	//正常に圧縮できたファイルを削除
	BOOL MoveToRecycleBin;		//圧縮後ファイルをごみ箱に移動
	BOOL DeleteNoConfirm;		//確認せずに削除/ごみ箱に移動
	BOOL ForceDelete;			//正常処理を確認できない形式でも削除

	BOOL IgnoreTopDirectory;	//「フォルダより下のファイルを圧縮」
protected:
	virtual void load(CONFIG_SECTION&);	//設定をCONFIG_SECTIONから読み込む
	virtual void store(CONFIG_SECTION&)const;	//設定をCONFIG_SECTIONに書き込む
public:
	virtual ~CConfigCompress(){}
	virtual void load(CConfigManager&);
	virtual void store(CConfigManager&)const;
};

