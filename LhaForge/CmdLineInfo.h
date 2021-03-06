﻿/*
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

//LhaForgeでのファイル処理の方法
enum PROCESS_MODE{
	PROCESS_INVALID,
	PROCESS_CONFIGURE,
	PROCESS_COMPRESS,
	PROCESS_EXTRACT,
	PROCESS_AUTOMATIC,
	PROCESS_LIST,
	PROCESS_TEST,
};



class CConfigManager;
enum DLL_ID;
enum OUTPUT_TO;
enum CREATE_OUTPUT_DIR;
enum LFPROCESS_PRIORITY;
//コマンドライン解釈の結果を格納するためのクラス
class CMDLINEINFO{
public:
	CMDLINEINFO();
	virtual ~CMDLINEINFO(){}
	std::list<CString> FileList;	//ファイル名リスト
	CString OutputDir;				//出力先フォルダ
	CString OutputFileName;			//出力先ファイル名
	PARAMETER_TYPE CompressType;
	int Options;				//圧縮オプション
	DLL_ID idForceDLL;			//使用を強制するDLL
	bool bSingleCompression;	//ファイルを一つずつ圧縮するならtrue
	CString ConfigPath;			//設定ファイルのパス
	OUTPUT_TO OutputToOverride;
	CREATE_OUTPUT_DIR CreateDirOverride;
	int IgnoreTopDirOverride;	//-1:default,0:false,1:true
	int DeleteAfterProcess;	//-1:default,0:false, other:true
	LFPROCESS_PRIORITY PriorityOverride;	//default:no change, other:change priority

	CString strMethod;
	CString strFormat;
	CString strLevel;

	CString strSplitSize;	//分割ボリュームサイズ
};

//コマンドラインを解釈する
PROCESS_MODE ParseCommandLine(CConfigManager&,CMDLINEINFO&);

