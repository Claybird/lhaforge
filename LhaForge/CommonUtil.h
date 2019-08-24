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

enum OUTPUT_TO;
struct CConfigGeneral;

//設定から出力先フォルダを読み込む
//r_bUseForAll:今後も同じフォルダ設定を使うならtrue
HRESULT GetOutputDirPathFromConfig(OUTPUT_TO,LPCTSTR lpszOrgFile,LPCTSTR lpszSpecific,CPath &r_pathOutputDir,bool &r_bUseForAll,CString &strErr);

//S_FALSEが返ったときには、何らかの方法で新しい出力先を選択させる(「名前をつけて保存」ダイアログを開く)
HRESULT ConfirmOutputDir(const CConfigGeneral &Config,LPCTSTR lpszOutputDir,CString &strErr);

//UtilExpandTemplateString()のパラメータ展開に必要な情報を構築する
void MakeExpandInformationEx(std::map<stdString,CString> &envInfo,LPCTSTR lpOpenDir,LPCTSTR lpOutputFile);
