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
#include "ConfigExtract.h"
#include "../Utilities/FileOperation.h"

void CConfigExtract::load(CONFIG_SECTION &Config)
{
	OutputDirType=(OUTPUT_TO)Config.Data[_T("OutputDirType")].GetNParam(0,OUTPUT_TO_LAST_ITEM,OUTPUT_TO_DESKTOP);

	CString Buffer=Config.Data[_T("OutputDir")];
	if(!Buffer.IsEmpty()){
		UtilGetCompletePathName(OutputDir,Buffer);
	}else{
		OutputDir=_T("");
	}

	//解凍後フォルダを開くかどうか
	OpenDir=Config.Data[_T("OpenFolder")].GetNParam(TRUE);

	//解凍時フォルダを二重にするかどうか
	CreateDir=(CREATE_OUTPUT_DIR)Config.Data[_T("CreateDir")].GetNParam(0,CREATE_OUTPUT_DIR_LAST_ITEM,CREATE_OUTPUT_DIR_ALWAYS);

	//解凍時ファイルを確認せずに上書きするかどうか
	ForceOverwrite=Config.Data[_T("ForceOverwrite")].GetNParam(FALSE);

	//解凍時フォルダ名から数字と記号を削除する
	RemoveSymbolAndNumber=Config.Data[_T("RemoveSymbolAndNumber")].GetNParam(FALSE);

	//解凍時ファイル・フォルダが一つだけの時フォルダを作らない
	CreateNoFolderIfSingleFileOnly=Config.Data[_T("CreateNoFolderIfSingleFileOnly")].GetNParam(FALSE);

	//同時に解凍するファイル数を制限する
	LimitExtractFileCount=Config.Data[_T("LimitExtractFileCount")].GetNParam(FALSE);

	//同時に解凍するファイル数の上限
	MaxExtractFileCount=max(1,Config.Data[_T("MaxExtractFileCount")].GetNParam(1));

	//正常に解凍できた圧縮ファイルを削除
	DeleteArchiveAfterExtract=Config.Data[_T("DeleteArchiveAfterExtract")].GetNParam(FALSE);

	//解凍後ファイルをごみ箱に移動
	MoveToRecycleBin=Config.Data[_T("MoveToRecycleBin")].GetNParam(TRUE);

	//確認せずにアーカイブを削除/ごみ箱に移動
	DeleteNoConfirm=Config.Data[_T("DeleteNoConfirm")].GetNParam(FALSE);

	//解凍エラーを検知できない場合も削除
	ForceDelete=Config.Data[_T("ForceDelete")].GetNParam(FALSE);

	//マルチボリュームも削除
	DeleteMultiVolume=Config.Data[_T("DeleteMultiVolume")].GetNParam(FALSE);

	//パスワード入力回数を最小にするならTRUE
	MinimumPasswordRequest=Config.Data[_T("MinimumPasswordRequest")].GetNParam(FALSE);

	//解凍対象から外す拡張子
	if(has_key(Config.Data,_T("DenyExt"))){
		DenyExt=Config.Data[_T("DenyExt")];
	}else{
		DenyExt.LoadString(IDS_DENYEXT_DEFAULT);
	}
}

void CConfigExtract::store(CONFIG_SECTION &Config)const
{
	Config.Data[_T("OutputDirType")]=OutputDirType;

	Config.Data[_T("OutputDir")]=OutputDir;

	//解凍後フォルダを開くかどうか
	Config.Data[_T("OpenFolder")]=OpenDir;

	//解凍時フォルダを二重にするかどうか
	Config.Data[_T("CreateDir")]=CreateDir;

	//解凍時ファイルを確認せずに上書きするかどうか
	Config.Data[_T("ForceOverwrite")]=ForceOverwrite;

	//解凍時フォルダ名から数字と記号を削除する
	Config.Data[_T("RemoveSymbolAndNumber")]=RemoveSymbolAndNumber;

	//解凍時ファイル・フォルダが一つだけの時フォルダを作らない
	Config.Data[_T("CreateNoFolderIfSingleFileOnly")]=CreateNoFolderIfSingleFileOnly;

	//同時に解凍するファイル数を制限する
	Config.Data[_T("LimitExtractFileCount")]=LimitExtractFileCount;

	//同時に解凍するファイル数の上限
	Config.Data[_T("MaxExtractFileCount")]=MaxExtractFileCount;

	//正常に解凍できた圧縮ファイルを削除
	Config.Data[_T("DeleteArchiveAfterExtract")]=DeleteArchiveAfterExtract;

	//解凍後ファイルをごみ箱に移動
	Config.Data[_T("MoveToRecycleBin")]=MoveToRecycleBin;

	//確認せずにアーカイブを削除/ごみ箱に移動
	Config.Data[_T("DeleteNoConfirm")]=DeleteNoConfirm;

	//解凍エラーを検知できない場合も削除
	Config.Data[_T("ForceDelete")]=ForceDelete;

	//マルチボリュームも削除
	Config.Data[_T("DeleteMultiVolume")]=DeleteMultiVolume;

	//パスワード入力回数を最小にするならTRUE
	Config.Data[_T("MinimumPasswordRequest")]=MinimumPasswordRequest;

	//解凍対象から外す拡張子
	Config.Data[_T("DenyExt")]=DenyExt;
}

void CConfigExtract::load(CConfigManager &ConfMan)
{
	load(ConfMan.GetSection(_T("Extract")));
}

void CConfigExtract::store(CConfigManager &ConfMan)const
{
	store(ConfMan.GetSection(_T("Extract")));
}
