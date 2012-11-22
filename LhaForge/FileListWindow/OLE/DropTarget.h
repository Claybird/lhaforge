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

#pragma once

//Original code from http://hp.vector.co.jp/authors/VA016117/
//Modified by Claybird http://claybird.sakura.ne.jp/

class IDropCommunicator{	//CDropTargetとドロップ受け入れウィンドウが通信するために使うインターフェイス;ドラッグ先と処理部が別なためこのような構造になっている
public:
	virtual ~IDropCommunicator(){}
	virtual HRESULT DragEnter(IDataObject*,POINTL&,DWORD&)=0;
	virtual HRESULT DragLeave()=0;
	virtual HRESULT DragOver(IDataObject*,POINTL&,DWORD&)=0;
	virtual HRESULT Drop(IDataObject*,POINTL&,DWORD&)=0;
};

class CDropTarget : public IDropTarget
{
protected:
	LONG _RefCount;
	IDropCommunicator *m_lpCommunicator;
	IDataObject *m_lpDataObject;
public:
	CDropTarget(IDropCommunicator *lpComm):_RefCount(1),m_lpCommunicator(lpComm),m_lpDataObject(NULL){}
	virtual ~CDropTarget(){};

	virtual HRESULT __stdcall QueryInterface(const IID& iid, void** ppv);
	virtual ULONG __stdcall AddRef(void);
	virtual ULONG __stdcall Release(void);

	virtual HRESULT __stdcall DragEnter(IDataObject* pDataObject, DWORD grfKeyState, POINTL ptl, DWORD* pdwEffect);
	virtual HRESULT __stdcall DragOver(DWORD grfKeyState, POINTL ptl, DWORD* pdwEffect);
	virtual HRESULT __stdcall DragLeave();
	virtual HRESULT __stdcall Drop(IDataObject* pDataObject, DWORD grfKeyState, POINTL ptl, DWORD* pdwEffect);

	//---
	bool QueryFormat(CLIPFORMAT cfFormat);
	HRESULT GetDroppedFiles(IDataObject *lpDataObject,std::list<CString>&);
};

