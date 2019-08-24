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
//---文字列処理

//文字コード
enum UTIL_CODEPAGE{
	UTILCP_SJIS,
	UTILCP_UTF8,
	UTILCP_UTF16
};


//MFCスタイルでCFileDialogのフィルター文字列を作る
void UtilMakeFilterString(LPCTSTR,LPTSTR,int);


//適当な文字コード->UNICODE
//dwSizeはUTILCP_UTF16のときのみ必要
bool UtilToUNICODE(CString &strRet,LPCBYTE lpcByte,DWORD dwSize,UTIL_CODEPAGE uSrcCodePage);
//UTF16-BE/UTF16-LE/SJISを自動判定してUNICODEに
void UtilGuessToUNICODE(CString &strRet,LPCBYTE lpcByte,DWORD dwSize);
//UNICODE->UTF8
bool UtilToUTF8(std::vector<BYTE> &cArray,LPCWSTR strSrc);

//UNICODEをUTF8に変換するためのアダプタクラス
class C2UTF8{
protected:
	std::vector<BYTE> m_cArray;
public:
	C2UTF8(LPCWSTR str){
		UtilToUTF8(m_cArray,str);
	}
	virtual ~C2UTF8(){}
	operator LPCSTR(){return (LPCSTR)&m_cArray[0];}
};

//UNICODEとして安全ならtrue
bool UtilIsSafeUnicode(LPCTSTR);

//TCHARファイル名がSJISファイル名で表現できるならtrue
bool UtilCheckT2A(LPCTSTR);
bool UtilCheckT2AList(const std::list<CString>&);	//複数ファイルのうち、一つでもUNICODE専用ファイル名があればfalse

//末尾から指定された文字を削る
void UtilTrimString(CStringW&,LPCWSTR lpszSubject);

//指定されたフォーマットで書かれた文字列を展開する
void UtilExpandTemplateString(CString &strOut,LPCTSTR lpszFormat,const std::map<stdString,CString> &env);

void UtilAssignSubString(CString &strOut,LPCTSTR lpStart,LPCTSTR lpEnd);

//文字列を分解し数値配列として取得
void UtilStringToIntArray(LPCTSTR, std::vector<int>&);

