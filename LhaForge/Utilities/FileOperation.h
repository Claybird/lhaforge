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

LPCTSTR UtilGetTempPath();
bool UtilGetTemporaryFileName(LPTSTR fname,LPCTSTR prefix);
bool UtilDeletePath(LPCTSTR PathName);
bool UtilDeleteDir(LPCTSTR Path,bool);
int UtilAppendFile(HANDLE hWriteTo,HANDLE hReadFrom);
void UtilModifyPath(CString&);	//DTVを起こす可能性のあるパスを修正する

BOOL UtilMoveFileToRecycleBin(LPCTSTR);	//ファイルをごみ箱に移動
BOOL UtilMoveFileToRecycleBin(const std::list<CString>&);	//ファイルをごみ箱に移動

//フォルダ内ファイル(ディレクトリは除く)を再帰検索
bool UtilRecursiveEnumFile(LPCTSTR lpszRoot,std::list<CString>&);

//フルパスかつ絶対パスの取得
enum PATHERROR{
	PATHERROR_NONE,		//成功
	PATHERROR_INVALID,	//パラメータ指定が不正
	PATHERROR_ABSPATH,	//絶対パスの取得に失敗
	PATHERROR_NOTFOUND,	//ファイルもしくはフォルダが見つからない
	PATHERROR_LONGNAME,	//ロングファイル名取得失敗
};
PATHERROR UtilGetCompletePathName(CString &_FullPath,LPCTSTR lpszFileName);
//絶対パスの取得
bool UtilGetAbsPathName(CString &_FullPath,LPCTSTR lpszFileName);

//ワイルドカードの展開
bool UtilPathExpandWild(std::list<CString> &r_outList,const std::list<CString> &r_inList);
bool UtilPathExpandWild(std::list<CString> &r_outList,const CString &r_inParam);

//パスのディレクトリ部分だけを取り出す
void UtilPathGetDirectoryPart(CString&);

//自分のプログラムのファイル名を返す
LPCTSTR UtilGetModulePath();

//自分のプログラムのおいてあるディレクトリのパス名を返す
LPCTSTR UtilGetModuleDirectoryPath();

//複数階層のディレクトリを一気に作成する
BOOL UtilMakeSureDirectoryPathExists(LPCTSTR lpszPath);

//TCHARファイル名をSJISファイル名に変換する。正しく変換できない場合には、falseを返す
bool UtilPathT2A(CStringA&,LPCTSTR,bool bOnDisk);

//パスに共通する部分を取り出し、基底パスを取り出す
void UtilGetBaseDirectory(CString &BasePath,const std::list<CString> &PathList);

//ファイル名に使えない文字列を置き換える
void UtilFixFileName(CString &,LPCTSTR lpszOrg,TCHAR replace);

LPCTSTR UtilPathNextSeparator(LPCTSTR lpStr);
bool UtilPathNextSection(LPCTSTR lpStart,LPCTSTR& r_lpStart,LPCTSTR& r_lpEnd,bool bSkipMeaningless);
//Pathが'/'もしくは'\\'で終わっているならtrue
bool UtilPathEndWithSeparator(LPCTSTR lpPath);
void UtilPathGetLastSection(CString &strSection,LPCTSTR lpPath);

//ファイルを丸ごと、もしくは指定されたところまで読み込み(-1で丸ごと)
bool UtilReadFile(LPCTSTR lpFile,std::vector<BYTE> &cReadBuffer,DWORD dwLimit=-1);

struct FILELINECONTAINER{
	virtual ~FILELINECONTAINER(){}
	std::vector<WCHAR> data;
	std::vector<LPCWSTR> lines;
};
bool UtilReadFileSplitted(LPCTSTR lpFile,FILELINECONTAINER&);
