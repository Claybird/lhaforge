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

//LhaForge用

//デバッグ用関数および便利な関数群
#pragma once
#include "StringUtil.h"
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

//remove items from container if theValue==ite
template <typename arrayclass,typename valueclass>
void remove_item(arrayclass &theArray,const valueclass &theValue){
	theArray.erase(std::remove(theArray.begin(), theArray.end(), theValue), theArray.end());
}

//remove items from container if cond(ite)==true
template <typename arrayclass, typename COND>
void remove_item_if(arrayclass &theArray, const COND &cond) {
	theArray.erase(std::remove_if(theArray.begin(), theArray.end(), cond), theArray.end());
}

template<typename T, typename U>
bool isIn(const T &collection, U value)
{
	for (const auto &i : collection) {
		if (i == value)return true;
	}
	return false;
}

