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

