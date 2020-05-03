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
enum OUTPUT_TO;
enum CREATE_OUTPUT_DIR;
enum LFPROCESS_PRIORITY;
enum LF_ARCHIVE_FORMAT;

#include "ArchiverCode/arc_interface.h"
#include "Utilities/OSUtil.h"

// command line arguments
struct CMDLINEINFO{
	CMDLINEINFO() :
		CompressType(LF_FMT_INVALID),
		Options(0),
		bSingleCompression(false),
		OutputToOverride((OUTPUT_TO)-1),
		CreateDirOverride((CREATE_OUTPUT_DIR)-1),
		IgnoreTopDirOverride(-1),
		DeleteAfterProcess(-1),
		PriorityOverride(LFPRIOTITY_DEFAULT) {}

	std::vector<std::wstring> FileList;	//ファイル名リスト
	CString OutputDir;				//出力先フォルダ
	CString OutputFileName;			//出力先ファイル名
	LF_ARCHIVE_FORMAT CompressType;
	int Options;				//圧縮オプション
	bool bSingleCompression;	//ファイルを一つずつ圧縮するならtrue
	CString ConfigPath;			//設定ファイルのパス
	OUTPUT_TO OutputToOverride;
	CREATE_OUTPUT_DIR CreateDirOverride;
	int IgnoreTopDirOverride;	//-1:default,0:false,1:true
	int DeleteAfterProcess;	//-1:default,0:false, other:true
	LFPROCESS_PRIORITY PriorityOverride;	//default:no change, other:change priority
};

//コマンドラインを解釈する
PROCESS_MODE ParseCommandLine(CConfigManager&,CMDLINEINFO&);

