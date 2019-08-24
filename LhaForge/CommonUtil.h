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
