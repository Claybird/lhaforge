/*
 * Copyright (c) 2005-2012, Claybird
 * All rights reserved.

 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:

 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Claybird nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.

 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
 * THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
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
};

//コマンドラインを解釈する
PROCESS_MODE ParseCommandLine(CConfigManager&,CMDLINEINFO&);

