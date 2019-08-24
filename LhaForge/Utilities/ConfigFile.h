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

#pragma once
//設定ファイル操作系

#include "variant.h"

//from http://ml.tietew.jp/cppll/cppll/article/12168
struct less_ignorecase {
	bool operator()(const stdString& a, const stdString& b) const {
		return _tcsicmp(a.c_str(), b.c_str()) < 0;
	}
};

//---セクション構造のない設定;データは「Key=Value」
typedef std::map<stdString,CVariant,less_ignorecase> FLATCONFIG;
//---セクションごとに分かれたデータ
struct CONFIG_SECTION{	//設定ファイルのセクションのデータ
	virtual ~CONFIG_SECTION(){}
	stdString SectionName;	//セクションの名前
	FLATCONFIG Data;
};

//設定ファイルの読み込み
bool UtilReadSectionedConfig(LPCTSTR,std::list<CONFIG_SECTION>&,CString &strErr);

//設定ファイルの書き込み
bool UtilWriteSectionedConfig(LPCTSTR,const std::list<CONFIG_SECTION>&,CString &strErr);

void UtilDumpFlatConfig(const FLATCONFIG&);
