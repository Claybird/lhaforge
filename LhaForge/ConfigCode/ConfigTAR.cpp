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
