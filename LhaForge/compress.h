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
#include "Utilities/Utility.h"
#include "ArchiverCode/arc_interface.h"

class CMDLINEINFO;

//アーカイブファイル名決定
struct CConfigCompress;
struct CConfigGeneral;
HRESULT GetArchiveName(CPath &r_pathArcFileName,const std::list<CString> &,const PARAMETER_TYPE,const int,const CConfigCompress&,const CConfigGeneral&,CMDLINEINFO &,bool bUnicodeCapable,CString &strErr);

//圧縮対象のファイルを探す;戻り値は
//0:ファイルは含まれていない
//1:単一ファイルのみが含まれている
//2:複数ファイルが含まれている
int FindFileToCompress(LPCTSTR lpszPath,CPath &);

//圧縮を行う。引数には必ずフルパスを渡すこと
bool Compress(const std::list<CString>&,const PARAMETER_TYPE,CConfigManager&,LPCTSTR lpszFormat,LPCTSTR lpszMethod,LPCTSTR lpszLevel,CMDLINEINFO&);

const LPCTSTR LHAFORGE_COMPRESS_SEMAPHORE_NAME=_T("LhaForgeCompressLimitSemaphore");

//コマンドラインパラメータとCompressに渡るパラメータの対応表
const struct COMPRESS_COMMANDLINE_PARAMETER{
	LPCTSTR Param;
	LPCTSTR Ext;
	PARAMETER_TYPE Type;
	int Options;
	WORD FormatName;
}CompressParameterArray[]={
	{_T("/c:lzh"),		_T(".lzh"),		PARAMETER_LZH,		0					,IDS_FORMAT_NAME_LZH},
	{_T("/c:lzhsfx"),	_T(".exe"),		PARAMETER_LZH,		COMPRESS_SFX		,IDS_FORMAT_NAME_LZH_SFX},
	{_T("/c:zip"),		_T(".zip"),		PARAMETER_ZIP,		0					,IDS_FORMAT_NAME_ZIP},
	{_T("/c:zippass"),	_T(".zip"),		PARAMETER_ZIP,		COMPRESS_PASSWORD	,IDS_FORMAT_NAME_ZIP_PASS},
	{_T("/c:zipsfx"),	_T(".exe"),		PARAMETER_ZIP,		COMPRESS_SFX		,IDS_FORMAT_NAME_ZIP_SFX},
	{_T("/c:zippasssfx"),_T(".exe"),	PARAMETER_ZIP,		COMPRESS_PASSWORD|COMPRESS_SFX,IDS_FORMAT_NAME_ZIP_PASS_SFX},
	{_T("/c:zipsplit"),	_T(".zip"),		PARAMETER_ZIP,		COMPRESS_SPLIT		,IDS_FORMAT_NAME_ZIP_SPLIT},
	{_T("/c:zippasssplit"),_T(".zip"),	PARAMETER_ZIP,		COMPRESS_PASSWORD|COMPRESS_SPLIT,IDS_FORMAT_NAME_ZIP_PASS_SPLIT},
	{_T("/c:cab"),		_T(".cab"),		PARAMETER_CAB,		0					,IDS_FORMAT_NAME_CAB},
	{_T("/c:cabsfx"),	_T(".exe"),		PARAMETER_CAB,		COMPRESS_SFX		,IDS_FORMAT_NAME_CAB_SFX},
	{_T("/c:7z"),		_T(".7z"),		PARAMETER_7Z,		0					,IDS_FORMAT_NAME_7Z},
	{_T("/c:7zpass"),	_T(".7z"),		PARAMETER_7Z,		COMPRESS_PASSWORD	,IDS_FORMAT_NAME_7Z_PASS},
	{_T("/c:7zsfx"),	_T(".exe"),		PARAMETER_7Z,		COMPRESS_SFX		,IDS_FORMAT_NAME_7Z_SFX},
	{_T("/c:7zsplit"),	_T(".7z"),		PARAMETER_7Z,		COMPRESS_SPLIT		,IDS_FORMAT_NAME_7Z_SPLIT},
	{_T("/c:7zpasssplit"),_T(".7z"),	PARAMETER_7Z,		COMPRESS_PASSWORD|COMPRESS_SPLIT,IDS_FORMAT_NAME_7Z_PASS_SPLIT},
	{_T("/c:tar"),		_T(".tar"),		PARAMETER_TAR,		0					,IDS_FORMAT_NAME_TAR},
	{_T("/c:gz"),		_T(".gz"),		PARAMETER_GZ,		0					,IDS_FORMAT_NAME_GZ},
	{_T("/c:bz2"),		_T(".bz2"),		PARAMETER_BZ2,		0					,IDS_FORMAT_NAME_BZ2},
	{_T("/c:xz"),		_T(".xz"),		PARAMETER_XZ,		0					,IDS_FORMAT_NAME_XZ},
	{_T("/c:lzma"),		_T(".lzma"),	PARAMETER_LZMA,		0					,IDS_FORMAT_NAME_LZMA},
	{_T("/c:tgz"),		_T(".tgz"),		PARAMETER_TAR_GZ,	0					,IDS_FORMAT_NAME_TGZ},
	{_T("/c:tbz"),		_T(".tbz"),		PARAMETER_TAR_BZ2,	0					,IDS_FORMAT_NAME_TBZ},
	{_T("/c:txz"),		_T(".tar.xz"),	PARAMETER_TAR_XZ,	0					,IDS_FORMAT_NAME_TAR_XZ},
	{_T("/c:tlz"),		_T(".tar.lzma"),PARAMETER_TAR_LZMA,	0					,IDS_FORMAT_NAME_TAR_LZMA},
	{_T("/c:jak"),		_T(".jak"),		PARAMETER_JACK,		0					,IDS_FORMAT_NAME_JACK},
	{_T("/c:jaksfx"),	_T(".exe"),		PARAMETER_JACK,		COMPRESS_SFX		,IDS_FORMAT_NAME_JACK_SFX},
	{_T("/c:yz1"),		_T(".yz1"),		PARAMETER_YZ1,		0					,IDS_FORMAT_NAME_YZ1},
	{_T("/c:yz1sfx"),	_T(".exe"),		PARAMETER_YZ1,		COMPRESS_SFX		,IDS_FORMAT_NAME_YZ1_SFX},
	{_T("/c:yz1pass"),	_T(".yz1"),		PARAMETER_YZ1,		COMPRESS_PASSWORD	,IDS_FORMAT_NAME_YZ1_PASS},
	{_T("/c:yz1passsfx"),_T(".exe"),	PARAMETER_YZ1,		COMPRESS_PASSWORD|COMPRESS_SFX,IDS_FORMAT_NAME_YZ1_PASS_SFX},
	{_T("/c:yz1pubpass"),_T(".yz1"),	PARAMETER_YZ1,		COMPRESS_PUBLIC_PASSWORD,IDS_FORMAT_NAME_YZ1_PUB_PASS},
	{_T("/c:yz1pubpasssfx"),_T(".exe"),	PARAMETER_YZ1,		COMPRESS_PUBLIC_PASSWORD|COMPRESS_SFX,IDS_FORMAT_NAME_YZ1_PUB_PASS_SFX},
	{_T("/c:hki"),		_T(".hki"),		PARAMETER_HKI,		0					,IDS_FORMAT_NAME_HKI},
	{_T("/c:hkipass"),	_T(".hki"),		PARAMETER_HKI,		COMPRESS_PASSWORD	,IDS_FORMAT_NAME_HKI_PASS},
	{_T("/c:hkisfx"),	_T(".exe"),		PARAMETER_HKI,		COMPRESS_SFX		,IDS_FORMAT_NAME_HKI_SFX},
//	{_T("/c:hkipasssfx"),_T(".exe"),	PARAMETER_HKI,		COMPRESS_PASSWORD|COMPRESS_SFX,IDS_FORMAT_NAME_HKI}, Not Supported by WinHKI v 1.61
	{_T("/c:bza"),		_T(".bza"),		PARAMETER_BZA,		0					,IDS_FORMAT_NAME_BZA},
	{_T("/c:bzasfx"),	_T(".exe"),		PARAMETER_BZA,		COMPRESS_SFX		,IDS_FORMAT_NAME_BZA_SFX},
	{_T("/c:gza"),		_T(".gza"),		PARAMETER_GZA,		0					,IDS_FORMAT_NAME_GZA},
	{_T("/c:gzasfx"),	_T(".exe"),		PARAMETER_GZA,		COMPRESS_SFX		,IDS_FORMAT_NAME_GZA_SFX},
	{_T("/c:ish"),		_T(".ish"),		PARAMETER_ISH,		0					,IDS_FORMAT_NAME_ISH},
	{_T("/c:uue"),		_T(".uue"),		PARAMETER_UUE,		0					,IDS_FORMAT_NAME_UUE},

//---B2Eの為の特別扱い
	{_T(""),			_T(".archive"),	PARAMETER_B2E,		0					,0},
	{_T(""),			_T(".exe"),	PARAMETER_B2E,		COMPRESS_SFX		,0},
};
const int COMPRESS_PARAM_COUNT=COUNTOF(CompressParameterArray);

