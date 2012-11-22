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

//LhaForge用

//デバッグ用関数および便利な関数群
#pragma once
//#pragma warning(disable:4786)

#if defined(_DEBUG) || defined(DEBUG)

void TraceLastError();

#else
// Releaseのとき
#define TraceLastError()

#endif	//_DEBUG

//=============================================
// 共通便利関数
//=============================================

//エラーメッセージを表示
int ErrorMessage(LPCTSTR);
//メッセージキャプションを取得
LPCTSTR UtilGetMessageCaption();
void UtilGetLastErrorMessage(CString &strMsg);

#define BOOL2bool(x)	(FALSE!=x)

//配列の中に指定された数字が有ればその位置を返す;見つからなければ-1を返す
int UtilCheckNumberArray(const int *lpcArray,int size,int c);

enum UTIL_CODEPAGE;

//レスポンスファイルを読み取る
bool UtilReadFromResponceFile(LPCTSTR lpszRespFile,UTIL_CODEPAGE,std::list<CString> &FileList);

//INIに数字を文字列として書き込む
BOOL UtilWritePrivateProfileInt(LPCTSTR lpAppName,LPCTSTR lpKeyName,LONG nData,LPCTSTR lpFileName);

//INIに指定されたセクションがあるならtrueを返す
bool UtilCheckINISectionExists(LPCTSTR lpAppName,LPCTSTR lpFileName);

//文字列を入力させる
bool UtilInputText(LPCTSTR lpszMessage,CString &strInput);

//与えられたファイル名がマルチボリューム書庫と見なせるなら検索文字列を作成し、trueを返す
bool UtilIsMultiVolume(LPCTSTR lpszPath,CString &r_strFindParam);

//標準の設定ファイルのパスを取得
void UtilGetDefaultFilePath(CString &strPath,LPCTSTR lpszDir,LPCTSTR lpszFile,bool &bUserCommon);

//ファイル名が指定したパターンに当てはまればtrue
bool UtilExtMatchSpec(LPCTSTR lpszPath,LPCTSTR lpPattern);

//ファイル名が指定した2つの条件で[許可]されるかどうか;拒否が優先;bDenyOnly=trueなら、Denyのチェックのみ行う
bool UtilPathAcceptSpec(LPCTSTR,LPCTSTR lpDeny,LPCTSTR lpAccept,bool bDenyOnly);

//強制的にメッセージループを回す
bool UtilDoMessageLoop();
VOID CALLBACK UtilMessageLoopTimerProc(HWND,UINT,UINT,DWORD);

//指定されたmapがキーを持っているかどうか
template <typename mapclass,typename keyclass>
bool has_key(const mapclass &theMap,keyclass theKey){
	return theMap.find(theKey)!=theMap.end();
}

//指定された値が配列中にあればそのインデックスを探す;無ければ-1
template <typename arrayclass,typename valueclass>
int index_of(const arrayclass &theArray,valueclass theValue){
	for(unsigned int i=0;i<theArray.size();++i){
		if(theArray[i]==theValue){
			return (signed)i;
		}
	}
	return -1;
}
//コンテナの要素を削除する
template <typename arrayclass,typename valueclass>
void remove_item(arrayclass &theArray,const valueclass &theValue){
	theArray.erase(std::remove(theArray.begin(), theArray.end(), theValue), theArray.end());
}
