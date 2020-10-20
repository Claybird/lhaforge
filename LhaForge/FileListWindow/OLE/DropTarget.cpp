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
#include "DropTarget.h"

HRESULT __stdcall CDropTarget::QueryInterface(const IID& iid, void** ppv)
{
	HRESULT hr;

	if(iid == IID_IDropTarget || iid == IID_IUnknown){
		hr = S_OK;
		*ppv = (void*)this;
		AddRef();
	}else{
		hr = E_NOINTERFACE;
		*ppv = 0;
	}
	return hr;
}


ULONG __stdcall CDropTarget::AddRef()
{
	InterlockedIncrement(&_RefCount);
	return (ULONG)_RefCount;
}


ULONG __stdcall CDropTarget::Release()
{
	ULONG ret = (ULONG)InterlockedDecrement(&_RefCount);
/*	if(ret == 0){
		delete this;
	}*/	//newしていないのでdeleteしない
	return (ULONG)_RefCount;
}

HRESULT __stdcall CDropTarget::DragEnter(IDataObject* pDataObject, DWORD grfKeyState, POINTL ptl, DWORD* pdwEffect)
{
	m_lpDataObject=pDataObject;
	return m_lpCommunicator->DragEnter(m_lpDataObject,ptl,*pdwEffect);
}


HRESULT __stdcall CDropTarget::DragOver(DWORD grfKeyState, POINTL ptl, DWORD* pdwEffect)
{
	return m_lpCommunicator->DragOver(m_lpDataObject,ptl,*pdwEffect);
}


HRESULT __stdcall CDropTarget::DragLeave()
{
	m_lpDataObject=NULL;
	return m_lpCommunicator->DragLeave();
}

HRESULT __stdcall CDropTarget::Drop(IDataObject* pDataObject, DWORD grfKeyState, POINTL ptl, DWORD* pdwEffect)
{
	m_lpDataObject=NULL;
	return m_lpCommunicator->Drop(pDataObject,ptl,*pdwEffect);
}

//------------
//フォーマットを調べる
bool CDropTarget::QueryFormat(CLIPFORMAT cfFormat)
{
	ASSERT(m_lpDataObject);
	if(!m_lpDataObject)return false;
	FORMATETC fmt;

	fmt.cfFormat = cfFormat;
	fmt.ptd = NULL;
	fmt.dwAspect = DVASPECT_CONTENT;
	fmt.lindex = -1;
	fmt.tymed = TYMED_HGLOBAL;
	return S_OK==m_lpDataObject->QueryGetData(&fmt);
}


HRESULT CDropTarget::GetDroppedFiles(IDataObject *lpDataObject,std::list<CString> &fileList)
{
	FORMATETC fmt;
	STGMEDIUM medium;

	fmt.cfFormat = CF_HDROP;
	fmt.ptd = NULL;
	fmt.dwAspect = DVASPECT_CONTENT;
	fmt.lindex = -1;
	fmt.tymed = TYMED_HGLOBAL;

	fileList.clear();

	HRESULT hr=lpDataObject->GetData(&fmt, &medium);
	if(S_OK==hr){
		//ファイルがドロップされた
		//---ファイル数を取得
		UINT nFileCount = ::DragQueryFileW((HDROP)medium.hGlobal, 0xFFFFFFFF, NULL, 0);

		//ファイル取得
		for(UINT i=0; i<nFileCount;i++){
			auto size = ::DragQueryFileW((HDROP)medium.hGlobal, i, nullptr, 0);
			std::vector<wchar_t> szBuffer(size);
			::DragQueryFileW((HDROP)medium.hGlobal, i, &szBuffer[0], size);
			fileList.push_back(&szBuffer[0]);
		}
		//解放
		ReleaseStgMedium(&medium);
		return S_OK;
	}else{
		return hr;
	}
}
