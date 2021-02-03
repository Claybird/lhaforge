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
class ILFDropCommunicator{
public:
	virtual ~ILFDropCommunicator(){}
	virtual HRESULT DragEnter(IDataObject*,POINTL&,DWORD&)=0;
	virtual HRESULT DragLeave()=0;
	virtual HRESULT DragOver(IDataObject*,POINTL&,DWORD&)=0;
	virtual HRESULT Drop(IDataObject*,POINTL&,DWORD&)=0;
};

class CLFDropTarget : public IDropTarget
{
protected:
	LONG _RefCount;
	ILFDropCommunicator *m_lpCommunicator;
	IDataObject *m_lpDataObject;
public:
	CLFDropTarget(ILFDropCommunicator *lpComm):_RefCount(1),m_lpCommunicator(lpComm),m_lpDataObject(NULL){}
	virtual ~CLFDropTarget(){};

	//---interface implementation
	virtual HRESULT __stdcall QueryInterface(const IID& iid, void** ppv)override {
		HRESULT hr;

		if (iid == IID_IDropTarget || iid == IID_IUnknown) {
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
		return (ULONG)_RefCount;
	}

	virtual HRESULT __stdcall DragEnter(IDataObject* pDataObject, DWORD grfKeyState, POINTL ptl, DWORD* pdwEffect)override {
		m_lpDataObject = pDataObject;
		return m_lpCommunicator->DragEnter(m_lpDataObject, ptl, *pdwEffect);
	}
	virtual HRESULT __stdcall DragOver(DWORD grfKeyState, POINTL ptl, DWORD* pdwEffect)override {
		return m_lpCommunicator->DragOver(m_lpDataObject, ptl, *pdwEffect);
	}
	virtual HRESULT __stdcall DragLeave()override {
		m_lpDataObject = NULL;
		return m_lpCommunicator->DragLeave();
	}
	virtual HRESULT __stdcall Drop(IDataObject* pDataObject, DWORD grfKeyState, POINTL ptl, DWORD* pdwEffect)override {
		m_lpDataObject = NULL;
		return m_lpCommunicator->Drop(pDataObject, ptl, *pdwEffect);
	}

	//---
	//check for format
	bool QueryFormat(CLIPFORMAT cfFormat) {
		ASSERT(m_lpDataObject);
		if (!m_lpDataObject)return false;
		FORMATETC fmt;

		fmt.cfFormat = cfFormat;
		fmt.ptd = NULL;
		fmt.dwAspect = DVASPECT_CONTENT;
		fmt.lindex = -1;
		fmt.tymed = TYMED_HGLOBAL;
		return S_OK == m_lpDataObject->QueryGetData(&fmt);
	}
	std::pair<HRESULT,std::vector<std::wstring> > GetDroppedFiles(IDataObject *lpDataObject) {
		FORMATETC fmt;
		STGMEDIUM medium;

		fmt.cfFormat = CF_HDROP;
		fmt.ptd = NULL;
		fmt.dwAspect = DVASPECT_CONTENT;
		fmt.lindex = -1;
		fmt.tymed = TYMED_HGLOBAL;

		std::vector<std::wstring> files;

		HRESULT hr = lpDataObject->GetData(&fmt, &medium);
		if (S_OK == hr) {
			//files dropped
			UINT nFileCount = ::DragQueryFileW((HDROP)medium.hGlobal, 0xFFFFFFFF, NULL, 0);

			//get files
			for (UINT i = 0; i < nFileCount; i++) {
				auto size = ::DragQueryFileW((HDROP)medium.hGlobal, i, nullptr, 0) + 1;
				std::vector<wchar_t> szBuffer(size);
				::DragQueryFileW((HDROP)medium.hGlobal, i, &szBuffer[0], size);
				files.push_back(&szBuffer[0]);
			}
			ReleaseStgMedium(&medium);
		}
		return std::make_pair(hr, files);
	}
};

