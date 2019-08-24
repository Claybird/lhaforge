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

//Original code from http://hp.vector.co.jp/authors/VA016117/
//Modified by Claybird http://claybird.sakura.ne.jp/
#include "stdafx.h"
#include "DataObject.h"

HRESULT __stdcall CDataObject::QueryInterface(const IID& iid, void** ppv)
{
	HRESULT hr;

	if(iid == IID_IDataObject || iid == IID_IUnknown){
		hr = S_OK;
		*ppv = (void*)this;
		AddRef();
	}else{
		hr = E_NOINTERFACE;
		*ppv = 0;
	}
	return hr;
}


ULONG __stdcall CDataObject::AddRef()
{
	InterlockedIncrement(&_RefCount);
	return (ULONG)_RefCount;
}


ULONG __stdcall CDataObject::Release()
{
	ULONG ret = (ULONG)InterlockedDecrement(&_RefCount);
	if(ret == 0){
		delete this;
	}
	return (ULONG)_RefCount;
}

HRESULT __stdcall CDataObject::GetData(FORMATETC *pFormatetc, STGMEDIUM *pmedium)
{
	int	i;

	if(pFormatetc == NULL || pmedium == NULL){
		return E_INVALIDARG;
	}

	if (!(DVASPECT_CONTENT & pFormatetc->dwAspect)) return DV_E_DVASPECT;

	for(i = 0; i < _num; ++i){
		if(_Objects[i].fmt.cfFormat == pFormatetc->cfFormat && 
			(_Objects[i].fmt.tymed & pFormatetc->tymed) != 0){
			if(!CSTGMEDIUM::Dup(pmedium, &_Objects[i].fmt, &_Objects[i].medium)){
				return E_OUTOFMEMORY;
			}
			return S_OK;
		}
	}

	return DV_E_FORMATETC;
}

// GetData‚Æ“¯‚¶‚æ‚¤‚È‚à‚Ì‚ç‚µ‚¢‚ª‚æ‚­•ª‚©‚ç‚È‚¢
HRESULT __stdcall CDataObject::GetDataHere(FORMATETC *pFormatetc, STGMEDIUM *pmedium)
{
	return E_NOTIMPL;
}


HRESULT __stdcall CDataObject::QueryGetData(FORMATETC *pFormatetc)
{
	int	i;

	if(pFormatetc == NULL){
		return E_INVALIDARG;
	}

	if (!(DVASPECT_CONTENT & pFormatetc->dwAspect)) return DV_E_DVASPECT;

	for(i = 0; i < _num; ++i){
		if(_Objects[i].fmt.cfFormat == pFormatetc->cfFormat && 
			(_Objects[i].fmt.tymed & pFormatetc->tymed) != 0){
			return S_OK;
		}
	}

	return DV_E_FORMATETC;
}

HRESULT __stdcall CDataObject::EnumFormatEtc(DWORD dwDirection, IEnumFORMATETC **ppenumFormatetc)
{
	int	i;
	CEnumFORMATETC*	pfmt;

	if (ppenumFormatetc == NULL){
		return E_INVALIDARG;
	}
	
	*ppenumFormatetc = NULL;
	switch (dwDirection) {
		case DATADIR_GET:
			pfmt = new CEnumFORMATETC;
			if(pfmt == NULL){
				return E_OUTOFMEMORY;
			}
			
			for(i = 0; i < _num; ++i){
				pfmt->SetFormat(&_Objects[i].fmt);
			}
			*ppenumFormatetc = pfmt;
			break;
		default:
			return E_NOTIMPL;
	}

	return S_OK;
}

HRESULT __stdcall CDataObject::SetData(FORMATETC *pFormatetc, STGMEDIUM *pmedium, BOOL fRelease)
{
	if(pFormatetc == NULL || pmedium == NULL) return E_INVALIDARG;

	if(_num >= (signed)_Objects.size()) return E_OUTOFMEMORY;

	if(!_Objects[_num].Set(pFormatetc, pmedium, fRelease)) return E_OUTOFMEMORY;

	_num++;

	return S_OK;
}


HRESULT __stdcall CDataObject::GetCanonicalFormatEtc(FORMATETC *pFormatetcIn, FORMATETC *pFormatetcOut)
{
	return E_NOTIMPL;
}

HRESULT __stdcall CDataObject::DAdvise(FORMATETC *pFormatetc, DWORD advf, IAdviseSink *pAdvSink, DWORD *pdwConnection)
{
	return OLE_E_ADVISENOTSUPPORTED;
}

HRESULT __stdcall CDataObject::DUnadvise(DWORD dwConnection)
{
	return OLE_E_ADVISENOTSUPPORTED;
}

HRESULT __stdcall CDataObject::EnumDAdvise(IEnumSTATDATA **ppenumAdvise)
{
	return OLE_E_ADVISENOTSUPPORTED;
}

bool CDataObject::allocate(int num)
{
	_Objects.resize(num);
	return true;
}
