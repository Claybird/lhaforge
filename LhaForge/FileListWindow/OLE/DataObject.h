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
//Modified by Claybird http://claybird.sakura.ne.jp/

#include "EnumFORMATETC.h"
#include "STGMEDIUM.h"

class CDataObject : public IDataObject
{
protected:
	class CObject{
	public:
		FORMATETC fmt;
		STGMEDIUM medium;
	public:
		CObject(){
			medium.tymed = TYMED_NULL;
		}
		~CObject(){
			if(medium.tymed != TYMED_NULL) ReleaseStgMedium(&medium);
		}
		BOOL Set(FORMATETC* pf, STGMEDIUM *pm, BOOL fRelease){
			fmt = *pf;
			if(fRelease){
				medium = *pm;
				return	TRUE;
			} else{
				return CSTGMEDIUM::Dup(&medium, pf, pm);
			}
		}
	};

	std::vector<CObject> _Objects;
	LONG _RefCount;
	int _num;
public:
	CDataObject() : _RefCount(1),_num(0){};
	~CDataObject(){};

	virtual HRESULT __stdcall QueryInterface(const IID& iid, void** ppv);
	virtual ULONG __stdcall AddRef(void);
	virtual ULONG __stdcall Release(void);

	virtual HRESULT __stdcall GetData(FORMATETC *pFormatetc, STGMEDIUM *pmedium);
	virtual HRESULT __stdcall GetDataHere(FORMATETC *pFormatetc, STGMEDIUM *pmedium);
	virtual HRESULT __stdcall QueryGetData(FORMATETC *pFormatetc);
	virtual HRESULT __stdcall GetCanonicalFormatEtc(FORMATETC *pFormatetcIn, FORMATETC *pFormatetcInOut);
	virtual HRESULT __stdcall SetData(FORMATETC *pFormatetc, STGMEDIUM *pmedium, BOOL fRelease); /*fRelease=TRUEの時DataObject*/
	virtual HRESULT __stdcall EnumFormatEtc(DWORD dwDirection, IEnumFORMATETC **ppenumFormatetc);
	virtual HRESULT __stdcall DAdvise(FORMATETC *pFormatetc, DWORD advf, IAdviseSink *pAdvSink, DWORD *pdwConnection);
	virtual HRESULT __stdcall DUnadvise(DWORD dwConnection);
	virtual HRESULT __stdcall EnumDAdvise(IEnumSTATDATA **ppenumAdvise);

	bool allocate(int num);
};

