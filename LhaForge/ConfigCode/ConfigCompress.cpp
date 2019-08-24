/*
 * Copyright (c) 2005-, Claybird
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

#include "stdafx.h"
#include "../ArchiverCode/arc_interface.h"
#include "ConfigManager.h"
#include "ConfigCompress.h"
#include "../Utilities/FileOperation.h"

void CConfigCompress::load(CONFIG_SECTION &Config)
{
	//出力先の種類
	OutputDirType=(OUTPUT_TO)Config.Data[_T("OutputDirType")].GetNParam(0,OUTPUT_TO_LAST_ITEM,OUTPUT_TO_DESKTOP);
	//出力先のパス
	CString Buffer=Config.Data[_T("OutputDir")];
	if(!Buffer.IsEmpty()){
		UtilGetCompletePathName(OutputDir,Buffer);
	}else{
		OutputDir=_T("");
	}
	//圧縮後フォルダを開くかどうか
	OpenDir=Config.Data[_T("OpenFolder")].GetNParam(TRUE);

	//出力ファイル名を指定するかどうか
	SpecifyOutputFilename=Config.Data[_T("SpecifyName")].GetNParam(FALSE);

	//同時に圧縮するファイル数を制限する
	LimitCompressFileCount=Config.Data[_T("LimitCompressFileCount")].GetNParam(FALSE);

	//同時に圧縮するファイル数の上限
	MaxCompressFileCount=max(1,(int)Config.Data[_T("MaxCompressFileCount")]);

	//デフォルト圧縮パラメータを使用するならtrue
	UseDefaultParameter=Config.Data[_T("UseDefaultParameter")].GetNParam(FALSE);

	//デフォルト圧縮パラメータ(形式指定)
	DefaultType=(PARAMETER_TYPE)Config.Data[_T("DefaultType")].GetNParam(0,PARAMETER_LAST_ITEM,PARAMETER_UNDEFINED);

	//デフォルト圧縮パラメータのオプション
	DefaultOptions=Config.Data[_T("DefaultOptions")].GetNParam(0);

	//B2E圧縮の情報
	DefaultB2EFormat=Config.Data[_T("DefaultB2EFormat")];
	DefaultB2EMethod=Config.Data[_T("DefaultB2EMethod")];

	//正常に圧縮できたファイルを削除
	DeleteAfterCompress=Config.Data[_T("DeleteAfterCompress")].GetNParam(FALSE);
	//圧縮後ファイルをごみ箱に移動
	MoveToRecycleBin=Config.Data[_T("MoveToRecycleBin")].GetNParam(TRUE);
	//確認せずに削除/ごみ箱に移動
	DeleteNoConfirm=Config.Data[_T("DeleteNoConfirm")].GetNParam(FALSE);
	//正常処理を確認できない形式でも削除
	ForceDelete=Config.Data[_T("ForceDelete")].GetNParam(FALSE);

	//「フォルダより下のファイルを圧縮」
	IgnoreTopDirectory=Config.Data[_T("IgnoreTopDirectory")].GetNParam(FALSE);
}

void CConfigCompress::store(CONFIG_SECTION &Config)const
{
	//出力先の種類
	Config.Data[_T("OutputDirType")]=OutputDirType;
	//出力先のパス
	Config.Data[_T("OutputDir")]=OutputDir;
	//圧縮後フォルダを開くかどうか
	Config.Data[_T("OpenFolder")]=OpenDir;

	//出力ファイル名を指定するかどうか
	Config.Data[_T("SpecifyName")]=SpecifyOutputFilename;

	//同時に圧縮するファイル数を制限する
	Config.Data[_T("LimitCompressFileCount")]=LimitCompressFileCount;

	//同時に圧縮するファイル数の上限
	Config.Data[_T("MaxCompressFileCount")]=MaxCompressFileCount;

	//デフォルト圧縮パラメータを使用するならtrue
	Config.Data[_T("UseDefaultParameter")]=UseDefaultParameter;

	//デフォルト圧縮パラメータ(形式指定)
	Config.Data[_T("DefaultType")]=DefaultType;

	//デフォルト圧縮パラメータのオプション
	Config.Data[_T("DefaultOptions")]=DefaultOptions;

	//B2E圧縮の情報
	Config.Data[_T("DefaultB2EFormat")]=DefaultB2EFormat;
	Config.Data[_T("DefaultB2EMethod")]=DefaultB2EMethod;

	//正常に圧縮できたファイルを削除
	Config.Data[_T("DeleteAfterCompress")]=DeleteAfterCompress;
	//圧縮後ファイルをごみ箱に移動
	Config.Data[_T("MoveToRecycleBin")]=MoveToRecycleBin;
	//確認せずに削除/ごみ箱に移動
	Config.Data[_T("DeleteNoConfirm")]=DeleteNoConfirm;
	//正常処理を確認できない形式でも削除
	Config.Data[_T("ForceDelete")]=ForceDelete;

	//「フォルダより下のファイルを圧縮」
	Config.Data[_T("IgnoreTopDirectory")]=IgnoreTopDirectory;
}

void CConfigCompress::load(CConfigManager &ConfMan)
{
	load(ConfMan.GetSection(_T("Compress")));
}

void CConfigCompress::store(CConfigManager &ConfMan)const
{
	store(ConfMan.GetSection(_T("Compress")));
}
