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
#include "../ArchiverCode/Archiver7ZIP.h"
#include "ConfigManager.h"
#include "ConfigZIP.h"
#include "../Dialogs/SevenZipVolumeSizeDlg.h"

// ZIP圧縮設定
void CConfigZIP::load(CONFIG_SECTION &Config)
{
	//圧縮形式
	CompressType=(ZIP_COMPRESS_TYPE)Config.Data[_T("CompressType")].GetNParam(0,ZIP_COMPRESS_TYPE_LAST_ITEM,0);
	//圧縮レベル
	CompressLevel=(ZIP_COMPRESS_LEVEL)Config.Data[_T("CompressLevel")].GetNParam(0,ZIP_COMPRESS_LEVEL_LAST_ITEM,0);
	//上級設定
	//優先するメモリのバイト数を指定するかどうか
	SpecifyDeflateMemorySize=Config.Data[_T("SpecifyDeflateMemorySize")].GetNParam(FALSE);
	//優先するメモリのバイト数
	DeflateMemorySize=Config.Data[_T("DeflateMemorySize")].GetNParam(ZIP_DEFLATE_MEMORY_SIZE_LOWEST,ZIP_DEFLATE_MEMORY_SIZE_HIGHEST,32);
	//エンコーダのパス数を指定するかどうか
	SpecifyDeflatePassNumber=Config.Data[_T("SpecifyDeflatePassNumber")].GetNParam(FALSE);
	//エンコーダのパス数
	DeflatePassNumber=Config.Data[_T("DeflatePassNumber")].GetNParam(ZIP_DEFLATE_PASS_NUMBER_LOWEST,ZIP_DEFLATE_PASS_NUMBER_HIGHEST,1);
	//常にUTF-8でファイル名を格納
	ForceUTF8=Config.Data[_T("ForceUTF8")].GetNParam(FALSE);

	//暗号化モード
	CryptoMode=(ZIP_CRYPTO_MODE)Config.Data[_T("CryptoMode")].GetNParam(0,ZIP_CRYPTO_MODE_LAST_ITEM,0);

	//分割サイズをあらかじめ指定
	SpecifySplitSize = Config.Data[_T("SpecifySplitSize")].GetNParam(FALSE);
	SplitSize = Config.Data[_T("SplitSize")].GetNParam(1,INT_MAX,10);
	SplitSizeUnit = Config.Data[_T("SplitSizeUnit")].GetNParam(0,ZIP_VOLUME_UNIT_MAX_NUM,0);
}

void CConfigZIP::store(CONFIG_SECTION &Config)const
{
	//圧縮形式
	Config.Data[_T("CompressType")]=CompressType;
	//圧縮レベル
	Config.Data[_T("CompressLevel")]=CompressLevel;
	//上級設定
	//優先するメモリのバイト数を指定するかどうか
	Config.Data[_T("SpecifyDeflateMemorySize")]=SpecifyDeflateMemorySize;
	//優先するメモリのバイト数
	Config.Data[_T("DeflateMemorySize")]=DeflateMemorySize;
	//エンコーダのパス数を指定するかどうか
	Config.Data[_T("SpecifyDeflatePassNumber")]=SpecifyDeflatePassNumber;
	//エンコーダのパス数
	Config.Data[_T("DeflatePassNumber")]=DeflatePassNumber;
	//常にUTF-8でファイル名を格納
	Config.Data[_T("ForceUTF8")]=ForceUTF8;

	//暗号化モード
	Config.Data[_T("CryptoMode")]=CryptoMode;

	//分割サイズをあらかじめ指定
	Config.Data[_T("SpecifySplitSize")] = SpecifySplitSize;
	Config.Data[_T("SplitSize")] = SplitSize;
	Config.Data[_T("SplitSizeUnit")] = SplitSizeUnit;

	// パスワード関連;現在はパスワード消去用コードのみ
	//指定されたパスワードを強制的に削除
	Config.Data.erase(_T("UseFixedPassword"));
	Config.Data.erase(_T("PasswordLength"));
	Config.Data.erase(_T("Password"));
}

void CConfigZIP::load(CConfigManager &ConfMan)
{
	load(ConfMan.GetSection(_T("ZIP")));
}

void CConfigZIP::store(CConfigManager &ConfMan)const
{
	store(ConfMan.GetSection(_T("ZIP")));
}
