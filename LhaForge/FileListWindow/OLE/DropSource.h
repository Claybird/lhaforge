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
#include "ArchiverCode/arc_interface.h"
#include "FileListWindow/FileListModel.h"

class CFileListModel;
class CLFDropSource : public IDropSource
{
protected:
	LONG _RefCount;
	CFileListModel& _rModel;
	std::vector<ARCHIVE_ENTRY_INFO*> _items;
	std::wstring _outputDir;
	ARCHIVE_ENTRY_INFO* _lpBase;
	DWORD _dwEffect;
public:
	std::wstring _strLog;
	bool _bRet;
public:
	CLFDropSource(CFileListModel &rModel,
		const std::vector<ARCHIVE_ENTRY_INFO*> &items,
		const std::wstring &outputDir,
		ARCHIVE_ENTRY_INFO* lpBase) :
		_RefCount(1),
		_rModel(rModel),
		_items(items),
		_outputDir(outputDir),
		_lpBase(lpBase),
		_dwEffect(0),
		_bRet(true)
	{}
	virtual ~CLFDropSource(){};

	virtual HRESULT __stdcall QueryInterface(const IID& iid, void** ppv)override {
		HRESULT hr;

		if (iid == IID_IDropSource || iid == IID_IUnknown) {
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

	virtual HRESULT __stdcall QueryContinueDrag(BOOL fEscapePressed, DWORD grfKeyState)override {
		// [Escape key pressed] or [right & left mouse buttons are pressed]
		if (fEscapePressed || (MK_LBUTTON | MK_RBUTTON) == (grfKeyState & (MK_LBUTTON | MK_RBUTTON))) {
			return DRAGDROP_S_CANCEL;
		}else if ((grfKeyState & (MK_LBUTTON | MK_RBUTTON)) == 0) {
			// dropped, then extract
			if (DROPEFFECT_NONE != _dwEffect) {
				std::vector<std::wstring> extractedFiles;
				_bRet = _rModel.MakeSureItemsExtracted(
					_outputDir.c_str(),
					true,
					_lpBase,
					_items,
					extractedFiles,
					_strLog);
				if (!_bRet) {
					return DRAGDROP_S_CANCEL;
				}
			}
			return DRAGDROP_S_DROP;
		}
		return S_OK;
	}
	virtual HRESULT __stdcall GiveFeedback(DWORD dwEffect)override {
		_dwEffect = dwEffect;
		//default mouse cursor
		return DRAGDROP_S_USEDEFAULTCURSORS;
	}
};

