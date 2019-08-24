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
	
	/* celtÇ™1ÇÃéûÇæÇØpceltFetchedÇÕNULLÇ…èoóàÇÈ*/
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

