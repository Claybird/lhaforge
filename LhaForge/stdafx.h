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
//#define WINVER 0x0500
#define _WIN32_WINNT 0x0600
#define NOMINMAX

#define ATL_NO_LEAN_AND_MEAN

#include <windows.h>
#include <windowsx.h>
#include <tchar.h>
#include <assert.h>
#include <atlstr.h>
#include <atlbase.h>
#include <atlapp.h>
extern CAppModule _Module;
#include <atlwin.h>
#include <atlcrack.h>
#include <atlctrls.h>
#include <atlctrlx.h>
#include <atlddx.h>
#include <atldlgs.h>
#include <atlframe.h>
#include <atlmisc.h>
#include <atlpath.h>
#include <atlscrl.h>
#include <atlsplit.h>

#include <algorithm>
#include <array>
#include <cctype>
#include <comcat.h>
#include <filesystem>
#include <fcntl.h>
#include <functional>
#include <inttypes.h>
#include <io.h>
#include <list>
#include <Lmcons.h>
#include <locale.h>
#include <map>
#include <regex>
#include <set>
#include <stack>
#include <string>
#include <sys/utime.h>
#include <sys/stat.h>
#include <time.h>
#include <unordered_map>
#include <unordered_set>
#include <vector>


#include <SimpleIni.h>

#if defined _M_IX86
  #pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
  #pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
  #pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
  #pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif

//enum
#define ENUM_COUNT_AND_LASTITEM ItemCount,LastItem=(ItemCount-1)
#define COUNTOF(x) (sizeof(x)/sizeof(x[0]))

#define ASSERT(x)	assert(x)
#include "Utilities/StringUtil.h"
//TRACE
#if defined(_DEBUG) || defined(DEBUG)
#define TRACE(fmt, ...)	OutputDebugString(Format(fmt, __VA_ARGS__).c_str())
#else
#define TRACE
#endif


// A macro to disallow the copy constructor and operator= functions
// This should be used in the private: declarations for a class
#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
  TypeName(const TypeName&)=delete;        \
  void operator=(const TypeName&)=delete;

#ifdef _MSC_VER
 #define WEAK_SYMBOL __declspec(selectany)
#else
 #define WEAK_SYMBOL __attribute__((weak))
#endif


struct LF_EXCEPTION {
	std::wstring _msg;
	LF_EXCEPTION(const std::wstring &err) {
		_msg = err;
	}
	virtual ~LF_EXCEPTION() {}
	const wchar_t* what()const { return _msg.c_str(); }
};

struct LF_USER_CANCEL_EXCEPTION: LF_EXCEPTION {
	LF_USER_CANCEL_EXCEPTION() :LF_EXCEPTION(L"Cancel") {}
	virtual ~LF_USER_CANCEL_EXCEPTION() {}
};

#define RAISE_EXCEPTION(...) throw LF_EXCEPTION(Format(__VA_ARGS__))
#define CANCEL_EXCEPTION() throw LF_USER_CANCEL_EXCEPTION()


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

#ifdef UNIT_TEST
#include <gtest/gtest.h>
inline std::filesystem::path LF_PROJECT_DIR() {
	return std::filesystem::path(__FILEW__).parent_path();
}
#endif

