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
//#include "DataObject.h"
//Original code from http://hp.vector.co.jp/authors/VA016117/
//Modified by Claybird http://claybird.sakura.ne.jp/

struct ARCHIVE_ENTRY_INFO;
class CFileListModel;
class CDropSource : public IDropSource
{
protected:
	LONG _RefCount;
	CFileListModel& _rModel;
	std::list<ARCHIVE_ENTRY_INFO*> _items;
	CString _strOutputDir;
	ARCHIVE_ENTRY_INFO* _lpBase;
	DWORD _dwEffect;
public:
	CDropSource(CFileListModel &rModel,const std::list<ARCHIVE_ENTRY_INFO*> &items,LPCTSTR lpOutputDir,ARCHIVE_ENTRY_INFO* lpBase)
		:_RefCount(1),_rModel(rModel),_items(items),_strOutputDir(lpOutputDir),_lpBase(lpBase),_dwEffect(0),_bRet(true)
	{}
	virtual ~CDropSource(){};

	virtual HRESULT __stdcall QueryInterface(const IID& iid, void** ppv);
	virtual ULONG __stdcall AddRef(void);
	virtual ULONG __stdcall Release(void);

	virtual HRESULT __stdcall QueryContinueDrag(BOOL fEscapePressed, DWORD grfKeyState);
	virtual HRESULT __stdcall GiveFeedback(DWORD dwEffect);

public:
	CString _strLog;
	bool _bRet;
};

