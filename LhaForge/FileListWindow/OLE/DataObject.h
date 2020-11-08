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

//Original code from http://hp.vector.co.jp/authors/VA016117/

#include "EnumFORMATETC.h"

class CLFDnDDataObject : public IDataObject
{
protected:
	struct OBJECT{
		FORMATETC fmt;
		STGMEDIUM medium;

		OBJECT(){
			medium.tymed = TYMED_NULL;
		}
		virtual ~OBJECT(){
			if(medium.tymed != TYMED_NULL) ReleaseStgMedium(&medium);
		}
		bool set(FORMATETC* pf, STGMEDIUM *pm, BOOL fRelease){
			fmt = *pf;
			if(fRelease){
				medium = *pm;
				return TRUE;
			} else{
				return duplicateMedium(&medium, pf, pm);
			}
		}
		static bool duplicateMedium(STGMEDIUM *pdest, const FORMATETC* pFormatetc, const STGMEDIUM *pmedium){
			HANDLE	hVoid = nullptr;
			pdest->tymed = pmedium->tymed;
			pdest->pUnkForRelease = pmedium->pUnkForRelease;
			if (pmedium->pUnkForRelease)pmedium->pUnkForRelease->AddRef();

			switch (pmedium->tymed) {
			case TYMED_HGLOBAL:
				hVoid = OleDuplicateData(pmedium->hGlobal, pFormatetc->cfFormat, (UINT)NULL);
				pdest->hGlobal = (HGLOBAL)hVoid;
				break;
			case TYMED_GDI:
				hVoid = OleDuplicateData(pmedium->hBitmap, pFormatetc->cfFormat, (UINT)NULL);
				pdest->hBitmap = (HBITMAP)hVoid;
				break;
			case TYMED_MFPICT:
				hVoid = OleDuplicateData(pmedium->hMetaFilePict, pFormatetc->cfFormat, (UINT)NULL);
				pdest->hMetaFilePict = (HMETAFILEPICT)hVoid;
				break;
			case TYMED_ENHMF:
				hVoid = OleDuplicateData(pmedium->hEnhMetaFile, pFormatetc->cfFormat, (UINT)NULL);
				pdest->hEnhMetaFile = (HENHMETAFILE)hVoid;
				break;
			case TYMED_FILE:
				hVoid = OleDuplicateData(pmedium->lpszFileName, pFormatetc->cfFormat, (UINT)NULL);
				pdest->lpszFileName = (LPOLESTR)hVoid;
				break;
			case TYMED_NULL:
				return true;
			case TYMED_ISTREAM:
			case TYMED_ISTORAGE:
			default:
				//nothing to do
				return false;
			}
			if (!hVoid) return false;
			return true;
		}
	};

	std::vector<std::shared_ptr<OBJECT> > _Objects;
	LONG _RefCount;
public:
	CLFDnDDataObject() : _RefCount(1){};
	~CLFDnDDataObject(){};

	virtual HRESULT __stdcall QueryInterface(const IID& iid, void** ppv)override {
		HRESULT hr;
		if (iid == IID_IDataObject || iid == IID_IUnknown) {
			hr = S_OK;
			*ppv = (void*)this;
			AddRef();
		} else {
			hr = E_NOINTERFACE;
			*ppv = 0;
		}
		return hr;
	}
	virtual ULONG __stdcall AddRef(void)override {
		InterlockedIncrement(&_RefCount);
		return (ULONG)_RefCount;
	}
	virtual ULONG __stdcall Release(void)override {
		ULONG ret = (ULONG)InterlockedDecrement(&_RefCount);
		if (ret == 0) {
			delete this;
		}
		return (ULONG)_RefCount;
	}

	virtual HRESULT __stdcall GetData(FORMATETC *pFormatetc, STGMEDIUM *pmedium)override {
		if (!pFormatetc || !pmedium) {
			return E_INVALIDARG;
		}

		if (!(DVASPECT_CONTENT & pFormatetc->dwAspect)) return DV_E_DVASPECT;

		for (const auto &item: _Objects) {
			if (item->fmt.cfFormat == pFormatetc->cfFormat &&
				(item->fmt.tymed & pFormatetc->tymed) != 0) {
				if (!OBJECT::duplicateMedium(pmedium, &item->fmt, &item->medium)) {
					return E_OUTOFMEMORY;
				}
				return S_OK;
			}
		}

		return DV_E_FORMATETC;
	}
	virtual HRESULT __stdcall GetDataHere(FORMATETC *pFormatetc, STGMEDIUM *pmedium)override {
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall QueryGetData(FORMATETC *pFormatetc)override {
		if (!pFormatetc) {
			return E_INVALIDARG;
		}

		if (!(DVASPECT_CONTENT & pFormatetc->dwAspect)) return DV_E_DVASPECT;

		for (const auto & item: _Objects) {
			if (item->fmt.cfFormat == pFormatetc->cfFormat &&
				(item->fmt.tymed & pFormatetc->tymed) != 0) {
				return S_OK;
			}
		}

		return DV_E_FORMATETC;
	}
	virtual HRESULT __stdcall GetCanonicalFormatEtc(FORMATETC *pFormatetcIn, FORMATETC *pFormatetcInOut)override {
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall SetData(FORMATETC *pFormatetc, STGMEDIUM *pmedium, BOOL fRelease)override {
		if (!pFormatetc || !pmedium ) return E_INVALIDARG;
		std::shared_ptr<OBJECT> ptr = std::make_shared<OBJECT>();
		if (!ptr->set(pFormatetc, pmedium, fRelease)) return E_OUTOFMEMORY;
		_Objects.push_back(ptr);
		return S_OK;
	}
	virtual HRESULT __stdcall EnumFormatEtc(DWORD dwDirection, IEnumFORMATETC **ppenumFormatetc)override {
		CLFDnDEnumFORMATETC* pfmt;

		if (!ppenumFormatetc )return E_INVALIDARG;
		*ppenumFormatetc = nullptr;
		switch (dwDirection) {
		case DATADIR_GET:
			pfmt = new CLFDnDEnumFORMATETC;
			if (pfmt == nullptr) {
				return E_OUTOFMEMORY;
			}

			for (auto& item : _Objects) {
				pfmt->SetFormat(item->fmt);
			}
			*ppenumFormatetc = pfmt;
			break;
		default:
			return E_NOTIMPL;
		}

		return S_OK;
	}
	virtual HRESULT __stdcall DAdvise(FORMATETC *pFormatetc, DWORD advf, IAdviseSink *pAdvSink, DWORD *pdwConnection)override {
		return OLE_E_ADVISENOTSUPPORTED;
	}
	virtual HRESULT __stdcall DUnadvise(DWORD dwConnection)override {
		return OLE_E_ADVISENOTSUPPORTED;
	}
	virtual HRESULT __stdcall EnumDAdvise(IEnumSTATDATA **ppenumAdvise)override {
		return OLE_E_ADVISENOTSUPPORTED;
	}
};

