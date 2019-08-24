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

// GetDataと同じようなものらしいがよく分からない
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
