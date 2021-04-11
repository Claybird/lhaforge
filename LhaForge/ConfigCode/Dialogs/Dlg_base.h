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

template <class T>
class LFWinDataExchange :public CWinDataExchange<T>
{
public:
	BOOL DDX_Text(UINT nID, ATL::CString& strText, int cbSize, BOOL bSave, BOOL bValidate = FALSE, int nLength = 0) {
		return __super::DDX_Text(nID, strText, cbSize, bSave, bValidate, nLength);
	}

	BOOL DDX_Text(UINT nID, std::wstring& strText, int /*cbSize*/, BOOL bSave, BOOL bValidate = FALSE, int nLength = 0)
	{
		T* pT = static_cast<T*>(this);
		BOOL bSuccess = TRUE;

		if (bSave) {
			HWND hWndCtrl = pT->GetDlgItem(nID);
			int nLen = ::GetWindowTextLength(hWndCtrl);
			int nRetLen = -1;
			std::vector<wchar_t> buf(nLen + 1);
			LPTSTR lpstr = &buf[0];
			if (lpstr != NULL) {
				nRetLen = ::GetWindowText(hWndCtrl, lpstr, nLen + 1);
			}
			strText = lpstr;
			if (nRetLen < nLen)
				bSuccess = FALSE;
		} else {
			bSuccess = pT->SetDlgItemText(nID, strText.c_str());
		}

		if (!bSuccess) {
			pT->OnDataExchangeError(nID, bSave);
		} else if (bSave && bValidate)   // validation
		{
			ATLASSERT(nLength > 0);
			if ((int)strText.length() > nLength) {
				_XData data = { ddxDataText };
				data.textData.nLength = strText.length();
				data.textData.nMaxLength = nLength;
				pT->OnDataValidateError(nID, bSave, data);
				bSuccess = FALSE;
			}
		}
		return bSuccess;
	}
};

#include "ConfigCode/ConfigFile.h"

class IConfigDlgBase {
public:
	virtual ~IConfigDlgBase() {}
	virtual LRESULT OnApply() = 0;
	virtual void LoadConfig(CConfigFile&) = 0;
	virtual void StoreConfig(CConfigFile&, CConfigFile& assistant) = 0;
};


template <typename T>
class LFConfigDialogBase : public CDialogImpl<T>, public CMessageFilter, public LFWinDataExchange<T>, public IConfigDlgBase
{
public:
	virtual ~LFConfigDialogBase() {}
};
