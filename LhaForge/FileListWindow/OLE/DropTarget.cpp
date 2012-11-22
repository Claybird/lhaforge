/*
 * Copyright (c) 2005-2012, Claybird
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
		UINT nFileCount = ::DragQueryFile((HDROP)medium.hGlobal, 0xFFFFFFFF, NULL, 0);

		//ファイル取得
		for(UINT i=0; i<nFileCount;i++){
			TCHAR szBuffer[_MAX_PATH+2]={0};
			::DragQueryFile((HDROP)medium.hGlobal, i, szBuffer, _MAX_PATH + 1);
			fileList.push_back(szBuffer);
		}
		//解放
		ReleaseStgMedium(&medium);
		return S_OK;
	}else{
		return hr;
	}
}
