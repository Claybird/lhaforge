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

//共通してincludeすべきファイルがすべて書いてある
//プリコンパイルドヘッダ

#pragma once
//#define WINVER 0x0500
#define _WIN32_WINNT 0x0600
//#define _ATL_NO_COM
//#define _WTL_NO_WTYPES
#define _WTL_NO_UNION_CLASSES
#define _ATL_NO_MSIMG
#define _ATL_NO_OPENGL
#define ATL_NO_LEAN_AND_MEAN
#define _ATL_USE_CSTRING_FLOAT	//CStringのFormatで小数が出力できるようになる

//#if (defined _DEBUG)||(defined DEBUG)
//#define _CRTDBG_MAP_ALLOC
//#include <stdlib.h>
//#include <crtdbg.h>
//#define new new(_NORMAL_BLOCK, __FILE__, __LINE__)
//#endif

#define _USE_32BIT_TIME_T	//time_tを32bitにする

#define _STLP_USE_NEWALLOC	//STLで標準のアロケータを使う
//#define _STLP_LEAKS_PEDANTIC	//STLのメモリリークを消す

//ATLのCStringを使う
//(cf.)http://hp.vector.co.jp/authors/VA022575/c/cstring.html
//#define _WTL_FORWARD_DECLARE_CSTRING
//#define _ATL_NO_AUTOMATIC_NAMESPACE
#define _WTL_NO_CSTRING

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
#include <atlmisc.h>
#include <atldlgs.h>
#include <atlctrls.h>
#include <atlframe.h>
#include <atlddx.h>		// DDX/DDVを使用するため
#if !defined(_UNICODE)&&!defined(UNICODE)
 #include <mbctype.h>
#endif//!defined(_UNICODE)&&!defined(UNICODE)
#include <atlframe.h>
#include <atlsplit.h>
#include <atlctrlx.h>
#include <time.h>

#include <algorithm>
#include <list>
#include <vector>
#include <string>
#include <map>
#include <stack>
#include <unordered_map>
#include <set>

#include <comcat.h>

#include <atlddx.h>		// DDX/DDVを使用するため
#include <atlscrl.h>
#include <Lmcons.h>
#include <atlpath.h>
#include <locale.h>
#include <cctype>

#define WM_USER_WM_SIZE		(WM_APP+1)	//WM_SIZEが来たら、PostMessageされる;ダイアログリサイズのトリックのため

#if defined _M_IX86
  #pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
  #pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
  #pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
  #pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif

//文字型の定義
#if defined(_UNICODE)||defined(UNICODE)
typedef std::wstring stdString;
#else//defined(_UNICODE)||defined(UNICODE)
typedef std::string stdString;
#endif//defined(_UNICODE)||defined(UNICODE)

//個数を数えるマクロ
#define COUNTOF(x) (sizeof(x)/sizeof(x[0]))

#define FILL_ZERO(x)	::ZeroMemory(&x,sizeof(x))
#define ASSERT(x)	assert(x)

//TRACE
void UtilDebugTrace(LPCTSTR pszFormat, ...);
#if defined(_DEBUG) || defined(DEBUG)
#define TRACE UtilDebugTrace
#else	// Releaseのとき
#define TRACE
#endif	//_DEBUG

//enum
#define ENUM_COUNT_AND_LASTITEM(x) x##_ITEM_COUNT,x##_LAST_ITEM=(x##_ITEM_COUNT-1)


//エラー定数
//NOTE:10100 00000000000 0000000000000000b = 0xA0000000
//NOTE:00100 00000000000 0000000000000000b = 0x20000000
#define E_LF_CANNOT_DECIDE_FILENAME		((HRESULT)(0xA0000000|0x001))
#define E_LF_OVERWRITE_SOURCE			((HRESULT)(0xA0000000|0x002))
#define E_LF_UNKNOWN_FORMAT				((HRESULT)(0xA0000000|0x003))
#define E_LF_FILELIST_NOT_SUPPORTED		((HRESULT)(0xA0000000|0x004))
#define E_LF_SAME_INPUT_AND_OUTPUT		((HRESULT)(0xA0000000|0x008))
#define E_LF_UNICODE_NOT_SUPPORTED		((HRESULT)(0xA0000000|0x010))
#define E_LF_FILE_NOT_FOUND				((HRESULT)(0xA0000000|0x020))

#define S_LF_ARCHIVE_EXISTS				((HRESULT)(0x20000000|0x001))

// A macro to disallow the copy constructor and operator= functions
// This should be used in the private: declarations for a class
#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
  TypeName(const TypeName&);               \
  void operator=(const TypeName&)
