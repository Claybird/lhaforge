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
#include "Config7Z.h"
#include "../Dialogs/SevenZipVolumeSizeDlg.h"

void CConfig7Z::load(CONFIG_SECTION &Config)
{
	//プリセットを使うかどうか
	UsePreset=Config.Data[_T("UsePreset")].GetNParam(TRUE);
	//圧縮形式
	CompressType=(SEVEN_ZIP_COMPRESS_TYPE)Config.Data[_T("CompressType")].GetNParam(0,SEVEN_ZIP_COMPRESS_TYPE_LAST_ITEM,0);
	//圧縮レベル
	CompressLevel=(SEVEN_ZIP_COMPRESS_LEVEL)Config.Data[_T("CompressLevel")].GetNParam(0,SEVEN_ZIP_COMPRESS_LEVEL_LAST_ITEM,0);
	//LZMA圧縮モード
	LZMA_Mode=(SEVEN_ZIP_LZMA_MODE)Config.Data[_T("LZMAMode")].GetNParam(0,SEVEN_ZIP_LZMA_MODE_LAST_ITEM,0);
	//ソリッド圧縮
	SolidMode=Config.Data[_T("SolidMode")].GetNParam(TRUE);
	//ヘッダ圧縮
	HeaderCompression=Config.Data[_T("HeaderCompression")].GetNParam(TRUE);
	//ヘッダ完全圧縮
	//FullHeaderCompression=Config.Data[_T("HeaderFullCompression")].GetNParam(TRUE);
	//ヘッダ暗号化
	HeaderEncryption=Config.Data[_T("HeaderEncryption")].GetNParam(FALSE);

	//上級設定
	//PPMdのモデルサイズを指定するかどうか
	SpecifyPPMdModelSize=Config.Data[_T("SpecifyPPMdModelSize")].GetNParam(FALSE);
	//PPMdのモデルサイズ
	PPMdModelSize=Config.Data[_T("PPMdModelSize")].GetNParam(SEVEN_ZIP_PPMD_MODEL_SIZE_LOWEST,SEVEN_ZIP_PPMD_MODEL_SIZE_HIGHEST,6);

	//分割サイズをあらかじめ指定
	SpecifySplitSize = Config.Data[_T("SpecifySplitSize")].GetNParam(FALSE);
	SplitSize = Config.Data[_T("SplitSize")].GetNParam(1,INT_MAX,10);
	SplitSizeUnit = Config.Data[_T("SplitSizeUnit")].GetNParam(0,ZIP_VOLUME_UNIT_MAX_NUM,0);
}

void CConfig7Z::store(CONFIG_SECTION &Config)const
{
	//プリセットを使うかどうか
	Config.Data[_T("UsePreset")]=UsePreset;
	//圧縮形式
	Config.Data[_T("CompressType")]=CompressType;
	//圧縮レベル
	Config.Data[_T("CompressLevel")]=CompressLevel;
	//LZMA圧縮モード
	Config.Data[_T("LZMAMode")]=LZMA_Mode;
	//ソリッド圧縮
	Config.Data[_T("SolidMode")]=SolidMode;
	//ヘッダ圧縮
	Config.Data[_T("HeaderCompression")]=HeaderCompression;
	//ヘッダ完全圧縮
	//Config.Data[_T("HeaderFullCompression")]=FullHeaderCompression;
	//ヘッダ暗号化
	Config.Data[_T("HeaderEncryption")]=HeaderEncryption;

	//上級設定
	//PPMdのモデルサイズを指定するかどうか
	Config.Data[_T("SpecifyPPMdModelSize")]=SpecifyPPMdModelSize;
	//PPMdのモデルサイズ
	Config.Data[_T("PPMdModelSize")]=PPMdModelSize;

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


void CConfig7Z::load(CConfigManager &ConfMan)
{
	load(ConfMan.GetSection(_T("7Z")));
}

void CConfig7Z::store(CConfigManager &ConfMan)const
{
	store(ConfMan.GetSection(_T("7Z")));
	UtilDumpFlatConfig(ConfMan.GetSection(_T("7Z")).Data);
}

