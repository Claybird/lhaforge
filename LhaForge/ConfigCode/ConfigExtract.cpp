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
