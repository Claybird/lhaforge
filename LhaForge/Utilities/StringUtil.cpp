﻿/*
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
#include "StringUtil.h"
#include "Utility.h"

void UtilTrimString(CStringW &strTarget,LPCWSTR lpszSubject)
{
	const std::wstring strSubject(lpszSubject);
	int idx;
	for(idx=strTarget.GetLength()-1;idx>=0;idx--){
		wchar_t ch=strTarget.operator[](idx);
		if(-1==index_of(strSubject,ch))break;
	}
	strTarget=strTarget.Left(idx+1);
}

//MFCスタイルでCFileDialogのフィルター文字列を作る
void UtilMakeFilterString(LPCTSTR lpszIn,LPTSTR lpszOut,int outSize)
{
	ASSERT(outSize>=2);
	if(outSize<2)return;
	_tcsncpy_s(lpszOut,outSize,lpszIn,outSize);
	lpszOut[outSize-1]=_T('\0');
	lpszOut[outSize-2]=_T('\0');

	LPTSTR lpChar=lpszOut;
	for(;*lpChar!=_T('\0');lpChar++){
#if !defined(_UNICODE)&&!defined(UNICODE)
		if(_MBC_SINGLE==_mbsbtype((LPCBYTE)lpChar,0))
#endif
			if(*lpChar==_T('|')){
				*lpChar=_T('\0');
			}
	}
	*lpChar++;
	*lpChar=_T('\0');
}

//TCHARファイル名がSJISファイル名で表現できるかチェックする
bool UtilCheckT2A(LPCTSTR lpPath)
{
#if defined(_UNICODE)||defined(UNICODE)
	//欠損無く変換できているかどうかチェックする
	CStringA strTempA(lpPath);
	CStringW strTempW(strTempA);
	return (strTempW==lpPath);
#else//defined(_UNICODE)||defined(UNICODE)
	//SJISそのまま
	return true;
#endif//defined(_UNICODE)||defined(UNICODE)
}

//複数ファイルのうち、一つでもUNICODE専用ファイル名があればfalse
bool UtilCheckT2AList(const std::list<CString> &strList)
{
	for(std::list<CString>::const_iterator ite=strList.begin();ite!=strList.end();++ite){
		if(!UtilCheckT2A(*ite))return false;
	}
	return true;
}


//適当な文字コード->UNICODE
bool UtilToUNICODE(CString &strRet,LPCBYTE lpcByte,DWORD dwSize,UTIL_CODEPAGE uSrcCodePage)
{
#if defined(_UNICODE)||defined(UNICODE)
	switch(uSrcCodePage){
	case UTILCP_SJIS:
		strRet=CStringA((LPCSTR)lpcByte);
		return true;
	case UTILCP_UTF16:
		if(*((LPCWSTR)lpcByte)==0xFEFF){
			//UTF16LE
			strRet=((LPCWSTR)lpcByte)+1;
		}else if(*((LPCWSTR)lpcByte)==0xFFFE){
			//UTF16BE
			//エンディアン変換
			std::vector<BYTE> cArray(dwSize-2);
			_swab((char*)const_cast<LPBYTE>(lpcByte+2),(char*)&cArray[0],dwSize-2);
			strRet=(LPCWSTR)&cArray[0];
			return true;
		}
		else{
			strRet=(LPCWSTR)lpcByte;
		}
		return true;
	case UTILCP_UTF8:
		{
			if(lpcByte[0]==0xEF && lpcByte[1]==0xBB && lpcByte[2]==0xBF){	//BOM check
				lpcByte+=3;
			}
			std::vector<wchar_t> buf(::MultiByteToWideChar(CP_UTF8,0,(LPCSTR)lpcByte,-1,NULL,0));	//バッファ確保
			//変換
			if(!::MultiByteToWideChar(CP_UTF8,0,(LPCSTR)lpcByte,-1,&buf[0],buf.size())){
				TRACE(_T("文字コード変換失敗(UTF8->UTF16)"));
				return false;
			}
			strRet=(LPCWSTR)&buf[0];
			return true;
		}
	default:
		ASSERT(!"This code cannot be run");
		return false;
	}
#else//defined(_UNICODE)||defined(UNICODE)
#error("not implemented")
	return false;
#endif//defined(_UNICODE)||defined(UNICODE)
}

//適当な入力文字の文字コードを推定し、UNICODE(UTF16)に変換
void UtilGuessToUNICODE(CString &strRet,LPCBYTE lpcByte,DWORD dwSize)
{
#if defined(_UNICODE)||defined(UNICODE)
	if(*((LPCWSTR)lpcByte)==0xFEFF){
		//UTF16LE
		TRACE(_T("入力テキストはUTF-16LEと推定される\n"));
		strRet=((LPCWSTR)lpcByte)+1;
	}else if(*((LPCWSTR)lpcByte)==0xFFFE){
		//UTF16BE
		//エンディアン変換
		TRACE(_T("入力テキストはUTF-16BEと推定される\n"));
		std::vector<BYTE> cArray(dwSize-2);
		_swab((char*)const_cast<LPBYTE>(lpcByte+2),(char*)&cArray[0],dwSize-2);
		strRet=(LPCWSTR)&cArray[0];
	}else if(lpcByte[0]==0xEF && lpcByte[1]==0xBB && lpcByte[2]==0xBF){	//BOM check
		TRACE(_T("入力テキストはUTF-8 with BOMと推定される\n"));
		UtilToUNICODE(strRet,lpcByte,dwSize,UTILCP_UTF8);
	}else{
		TRACE(_T("入力テキストは非UNICODE(UTF16)と推定される\n"));
		strRet=CStringA((LPCSTR)lpcByte);
	}
#else//defined(_UNICODE)||defined(UNICODE)
#error("not implemented")
	return false;
#endif//defined(_UNICODE)||defined(UNICODE)
}

inline bool between(WCHAR a,WCHAR begin,WCHAR end){
	return (begin<=a && a<=end);
}

//UNICODEとして安全ならtrue
bool UtilIsSafeUnicode(LPCTSTR lpChar)
{
#if defined(_UNICODE)||defined(UNICODE)
	ASSERT(lpChar);
	if(!lpChar){
		return true;
	}
	const static WCHAR chars[]={
		0x001e,
		0x001f,
		0x00ad,
		0xFEFF,
		0xFFF9,
		0xFFFA,
		0xFFFB,
		0xFFFE,
	};
	const static UINT len=COUNTOF(chars);
	for(;*lpChar!=L'\0';lpChar++){
		WCHAR c=*lpChar;
		//単独文字との比較
		for(UINT i=0;i<len;i++){
			if(c==chars[i])return false;
		}

		//範囲のある文字との比較
		if(between(c,0x200b,0x200f))return false;
		if(between(c,0x202a,0x202e))return false;
		if(between(c,0x2060,0x2063))return false;
		if(between(c,0x206a,0x206f))return false;
	}
	return true;

#else//defined(_UNICODE)||defined(UNICODE)
	return true;
#endif//defined(_UNICODE)||defined(UNICODE)
}


//指定されたフォーマットで書かれた文字列を展開する
void UtilExpandTemplateString(CString &strOut,LPCTSTR lpszFormat,const std::map<stdString,CString> &env)
{
	strOut=lpszFormat;
	for(std::map<stdString,CString>::const_iterator ite=env.begin();ite!=env.end();++ite){
		//%で始まるブロックは環境変数と見なすので{}による修飾は行わない
		stdString strKey=( (*ite).first[0]==L'%' ? (*ite).first : _T("{")+(*ite).first+_T("}") );
		strOut.Replace(strKey.c_str(),(*ite).second);
	}
}

void UtilAssignSubString(CString &strOut,LPCTSTR lpStart,LPCTSTR lpEnd)
{
	ASSERT(lpStart);
	ASSERT(lpEnd);
	strOut=_T("");
	for(;lpStart!=lpEnd;++lpStart){
		strOut+=*lpStart;
	}
}


//文字列を分解し数値配列として取得
void UtilStringToIntArray(LPCTSTR str, std::vector<int>& numArr)
{
	numArr.clear();

	for(;_T('\0')!=*str;){
		CString Temp;
		for(;;){
			if(_T(',')==*str||_T('\0')==*str){
				str++;
				break;
			}else{
				Temp += *str;
				str++;
			}
		}
		int num = _ttoi(Temp);
		numArr.push_back(num);
	}
}

