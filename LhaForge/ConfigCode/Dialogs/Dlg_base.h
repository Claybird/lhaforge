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

#include "ConfigCode/ConfigFile.h"

class IConfigDlgBase {
public:
	virtual ~IConfigDlgBase() {}
	virtual LRESULT OnApply() = 0;
	virtual void LoadConfig(CConfigFile&) = 0;
	virtual void StoreConfig(CConfigFile&, CConfigFile& assistant) = 0;

	virtual HWND GetDialogHandle() = 0;
	virtual HWND Create(HWND hWndParent, LPARAM dwInitParam = NULL) = 0;
};


template <typename T>
class LFConfigDialogBase : public CDialogImpl<T>, public LFWinDataExchange<T>, public IConfigDlgBase
{
public:
	virtual ~LFConfigDialogBase() {}
	virtual HWND GetDialogHandle()override { return m_hWnd; }
	virtual HWND Create(HWND hWndParent, LPARAM dwInitParam = NULL) override {
		return CDialogImpl<T>::Create(hWndParent, dwInitParam);
	}
};
