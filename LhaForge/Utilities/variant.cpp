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


#include "stdafx.h"
#include "variant.h"

LPCTSTR CVariant::getRawValue()const
{
	return m_str;
}

bool CVariant::isEmpty()const
{
	return m_str.IsEmpty();
}

bool CVariant::isNumber()const
{
	float f;
	int ret=_stscanf_s(m_str,_T("%f"),&f);
	if(0==ret||EOF==ret)return false;
	return true;
}

//---

CVariant::operator int()const
{
	return _ttoi(m_str);
}

CVariant::operator float()const
{
	return (float)_tstof(m_str);
}

CVariant::operator LPCTSTR()const
{
	return (LPCTSTR)m_str;
}

//---compare
bool CVariant::operator==(int nValue)const
{
	return operator int()==nValue;
}

bool CVariant::operator==(float fValue)const
{
	return operator float()==fValue;
}

bool CVariant::operator==(LPCTSTR lpszValue)const
{
	return m_str==lpszValue;
}

bool CVariant::operator==(const CVariant &v)const
{
	return m_str==v.m_str;
}

//---let
CVariant& CVariant::operator=(int nValue)
{
	m_str.Format(_T("%d"),nValue);
	return *this;
}

CVariant& CVariant::operator=(float fValue)
{
	m_str.Format(_T("%.10g"),fValue);
	return *this;
}

CVariant& CVariant::operator=(LPCTSTR lpszValue)
{
	m_str=lpszValue;
	return *this;
}

CVariant& CVariant::operator=(const CVariant& v)
{
	m_str=v.m_str;
	return *this;
}


//----------
float CVariant::GetFParam(float fDefault)
{
	if(m_str.IsEmpty())return fDefault;
	return (float)_tstof(m_str);
}

int CVariant::GetNParam(int nDefault)
{
	if(m_str.IsEmpty())return nDefault;
	return _tstoi(m_str);
}

int CVariant::GetNParam(int nMin,int nMax,int nDefault)
{
	if(m_str.IsEmpty())return nDefault;
	int ret=_tstoi(m_str);
	if(ret<nMin || nMax<ret){
		return nDefault;
	}else{
		return ret;
	}
}


//----------
bool operator==(int nValue, const CVariant& v)
{
	return v==nValue;
}

bool operator==(LPCTSTR lpszValue, const CVariant& v)
{
	return v==lpszValue;
}

bool operator==(float fValue, const CVariant& v)
{
	return v==fValue;
}

