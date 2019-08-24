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
#include "DropSource.h"
#include "../FileListModel.h"
//Original code from http://hp.vector.co.jp/authors/VA016117/
//Modified by Claybird http://claybird.sakura.ne.jp/


HRESULT __stdcall CDropSource::QueryContinueDrag(BOOL fEscapePressed, DWORD grfKeyState)
{
	// ドラッグを継続するかどうかを決める

	// ESCが押された場合やマウスのボタンが両方押されたときは中止
	if(fEscapePressed || (MK_LBUTTON | MK_RBUTTON) == (grfKeyState & (MK_LBUTTON | MK_RBUTTON))){
		return DRAGDROP_S_CANCEL;
	}

	// マウスボタンが離されたときはドロップ
	if((grfKeyState & (MK_LBUTTON | MK_RBUTTON)) == 0){
		//解凍開始;失敗で中止
		if(DROPEFFECT_NONE!=_dwEffect){
			std::list<CString> dummyList;
			_bRet=_rModel.MakeSureItemsExtracted(_strOutputDir,_lpBase,_items,dummyList,true,_strLog);
			if(!_bRet){
				return DRAGDROP_S_CANCEL;
			}
		}
		return DRAGDROP_S_DROP;
	}
	return S_OK;
}

HRESULT __stdcall CDropSource::QueryInterface(const IID& iid, void** ppv)
{
	HRESULT hr;

	if(iid == IID_IDropSource || iid == IID_IUnknown){
		hr = S_OK;
		*ppv = (void*)this;
		AddRef();
	}else{
		hr = E_NOINTERFACE;
		*ppv = 0;
	}
	return hr;
}


ULONG __stdcall CDropSource::AddRef()
{
	InterlockedIncrement(&_RefCount);
	return (ULONG)_RefCount;
}


ULONG __stdcall CDropSource::Release()
{
	ULONG ret = (ULONG)InterlockedDecrement(&_RefCount);
	if(ret == 0){
		delete this;
	}
	return (ULONG)_RefCount;
}

HRESULT __stdcall CDropSource::GiveFeedback(DWORD dwEffect)
{
	_dwEffect=dwEffect;
	/* マウスカーソルを変えたり、特別な表示をするときはここで行う */

	//標準のマウスカーソルを使う
	return DRAGDROP_S_USEDEFAULTCURSORS;
}
