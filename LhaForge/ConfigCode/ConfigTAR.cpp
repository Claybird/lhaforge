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
#include "../ArchiverCode/ArchiverTAR.h"
#include "ConfigManager.h"
#include "ConfigTAR.h"

// TAR/GZ/BZ2圧縮設定
void CConfigTAR::load(CONFIG_SECTION &Config)
{
	//GZIPの圧縮レベル
	GzipCompressLevel=Config.Data[_T("GZIPLevel")].GetNParam(GZIP_COMPRESS_LEVEL_LOWEST,GZIP_COMPRESS_LEVEL_HIGHEST,9);
	//BZIP2の圧縮レベル
	Bzip2CompressLevel=Config.Data[_T("BZIP2Level")].GetNParam(BZIP2_COMPRESS_LEVEL_LOWEST,BZIP2_COMPRESS_LEVEL_HIGHEST,9);
	//XZの圧縮レベル
	XZCompressLevel=Config.Data[_T("XZLevel")].GetNParam(XZ_COMPRESS_LEVEL_LOWEST,XZ_COMPRESS_LEVEL_HIGHEST,6);
	//LZMAの圧縮レベル
	LZMACompressLevel=Config.Data[_T("LZMALevel")].GetNParam(LZMA_COMPRESS_LEVEL_LOWEST,LZMA_COMPRESS_LEVEL_HIGHEST,6);
	//文字コード変換
	bConvertCharset=Config.Data[_T("ConvertCharset")].GetNParam(1);
	//ソートモード
	SortBy=Config.Data[_T("SortBy")].GetNParam(TAR_SORT_BY_NONE,TAR_SORT_BY_MAX,TAR_SORT_BY_NONE);
}

void CConfigTAR::store(CONFIG_SECTION &Config)const
{
	//GZIPの圧縮レベル
	Config.Data[_T("GZIPLevel")]=GzipCompressLevel;
	//BZIP2の圧縮レベル
	Config.Data[_T("BZIP2Level")]=Bzip2CompressLevel;
	//XZの圧縮レベル
	Config.Data[_T("XZLevel")]=XZCompressLevel;
	//LZMAの圧縮レベル
	Config.Data[_T("LZMALevel")]=LZMACompressLevel;
	//文字コード変換
	Config.Data[_T("ConvertCharset")]=bConvertCharset;
	//ソートモード
	Config.Data[_T("SortBy")]=SortBy;
}

void CConfigTAR::load(CConfigManager &ConfMan)
{
	load(ConfMan.GetSection(_T("TAR")));
}

void CConfigTAR::store(CConfigManager &ConfMan)const
{
	store(ConfMan.GetSection(_T("TAR")));
}
