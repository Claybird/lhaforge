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
