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

//自由にキャストできる変数
//Perlのような変数変換を与える

class CVariant{
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
