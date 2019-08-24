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
#include "EnumFORMATETC.h"
//Original code from http://hp.vector.co.jp/authors/VA016117/
//Modified by Claybird http://claybird.sakura.ne.jp/

HRESULT __stdcall CEnumFORMATETC::QueryInterface(const IID& iid, void** ppv)
{
	HRESULT hr;

	if(iid == IID_IEnumFORMATETC || iid == IID_IUnknown){
		hr = S_OK;
		*ppv = (void*)this;
		AddRef();
	}else{
		hr = E_NOINTERFACE;
		*ppv = 0;
	}
	return hr;
}


ULONG __stdcall CEnumFORMATETC::AddRef()
{
	InterlockedIncrement(&_RefCount);
	return (ULONG)_RefCount;
}


ULONG __stdcall CEnumFORMATETC::Release()
{
	ULONG ret = (ULONG)InterlockedDecrement(&_RefCount);
	if(ret == 0){
		delete this;
	}
	return (ULONG)_RefCount;
}

HRESULT __stdcall CEnumFORMATETC::Next(ULONG celt, FORMATETC * rgelt, ULONG * pceltFetched)
{
	ULONG n = celt;
	
	if(pceltFetched != NULL) *pceltFetched = 0;
	
	if(celt <= 0 || rgelt == NULL || _current >= (signed)m_fmtArray.size())	return S_FALSE;
	
	/* celtが1の時だけpceltFetchedはNULLに出来る*/
	if(pceltFetched == NULL && celt != 1)	return S_FALSE;
	
	while(_current < (signed)m_fmtArray.size() && n > 0) {
		*rgelt++ = m_fmtArray[_current];
		++_current;
		--n;
	}
	if(pceltFetched != NULL) *pceltFetched = celt - n;
	
	return (n == 0)? S_OK : S_FALSE;
}

HRESULT __stdcall CEnumFORMATETC::Skip(ULONG celt)
{

	while(_current < (signed)m_fmtArray.size() && celt > 0) {
		++_current;
		--celt;
	}

	return (celt == 0)? S_OK : S_FALSE;
}

HRESULT __stdcall CEnumFORMATETC::Reset(void)
{
	_current = 0;
	return S_OK;
}

HRESULT __stdcall CEnumFORMATETC::Clone(IEnumFORMATETC ** ppenum)
{
	CEnumFORMATETC	*pfmt;
	
	if(!ppenum)return E_POINTER;

	pfmt = new CEnumFORMATETC;
	if(pfmt == NULL) return E_OUTOFMEMORY;

	pfmt->m_fmtArray=m_fmtArray;
	pfmt->_current = _current;
	*ppenum = pfmt;

	return S_OK;
}

BOOL CEnumFORMATETC::SetFormat(FORMATETC *fmt)
{
	if(!fmt)return FALSE;

	m_fmtArray.push_back(*fmt);

	return	TRUE;
}

