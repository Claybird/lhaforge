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
