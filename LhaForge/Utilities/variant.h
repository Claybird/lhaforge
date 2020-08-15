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

//自由にキャストできる変数
//Perlのような変数変換を与える

class[[deprecated("will be removed")]] CVariant{
protected:
	CString m_str;
public:
	CVariant(){}
	CVariant(const CVariant& v):m_str(v.m_str){}
	CVariant(LPCTSTR lpsz):m_str(lpsz){}
	virtual ~CVariant(){}

	LPCTSTR getRawValue()const;

	//---empty?
	bool isEmpty()const;

	//---cast check
	bool isNumber()const;

	//---cast
	operator int()const;
	operator float()const;
	operator LPCTSTR()const;

	//---compare
	bool operator==(int nValue)const;
	bool operator==(float fValue)const;
	bool operator==(LPCTSTR lpszValue)const;
	bool operator==(const CVariant &v)const;

	//---let
	CVariant& operator=(int nValue);
	CVariant& operator=(float fValue);
	CVariant& operator=(LPCTSTR lpszValue);
	CVariant& operator=(const CVariant& v);

	//デフォルト値を指定できるatoi/atof
	float GetFParam(float fDefault);
	int GetNParam(int nDefault);
	//範囲を指定できるatoi;範囲はnMin以上nMax以下
	int GetNParam(int nMin,int nMax,int nDefault);
};

bool operator==(int nValue, const CVariant& v);
bool operator==(LPCTSTR lpszValue, const CVariant& v);
bool operator==(float fValue, const CVariant& v);
