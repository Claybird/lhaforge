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

