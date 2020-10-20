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
#include "OSUtil.h"
#include "Utility.h"
#include "FileOperation.h"

HRESULT UtilCreateShortcut(
	const std::wstring& pathLink,
	const std::wstring& pathTarget,
	const std::wstring& args,
	const std::wstring& iconPath,
	int iIcon,
	LPCTSTR lpszDescription)
{
	CComPtr<IShellLinkW> pLink;
	HRESULT hr = pLink.CoCreateInstance(CLSID_ShellLink);
	if (FAILED(hr))return hr;

	// such as C:\Windows\notepad.exe
	pLink->SetPath(pathTarget.c_str());
	pLink->SetArguments(args.c_str());
	pLink->SetIconLocation(iconPath.c_str(),iIcon);
	pLink->SetDescription(lpszDescription);
	//save to file
	CComQIPtr<IPersistFile> pFile = pLink;
	if (pFile) {
		return pFile->Save(pathLink.c_str(), TRUE);
	} else {
		return E_FAIL;
	}
}

HRESULT UtilGetShortcutInfo(const std::wstring& path, UTIL_SHORTCUTINFO& info)
{
	CComPtr<IShellLinkW> pLink;
	HRESULT hr = pLink.CoCreateInstance(CLSID_ShellLink);
	if (FAILED(hr))return hr;

	pLink->SetPath(path.c_str());
	CComQIPtr<IPersistFile> pFile = pLink;
	if(pFile){
		hr = pFile->Load(path.c_str(), STGM_READ);
		if (FAILED(hr))return hr;
		info.title = std::filesystem::path(path).stem().c_str();

		{
			//The maximum path size that can be returned is MAX_PATH
			//https://docs.microsoft.com/en-us/windows/win32/api/shobjidl_core/nf-shobjidl_core-ishelllinkw-getpath
			wchar_t szTarget[_MAX_PATH + 1] = {};
			hr = pLink->GetPath(szTarget, COUNTOF(szTarget), NULL, 0);
			if (FAILED(hr))return hr;
			info.cmd = szTarget;
		}

		{
			wchar_t szArg[INFOTIPSIZE + 1] = {};
			hr = pLink->GetArguments(szArg, COUNTOF(szArg));
			if (FAILED(hr))return hr;
			info.param = szArg;
		}

		{
			wchar_t szDir[_MAX_PATH + 1] = {};
			hr = pLink->GetWorkingDirectory(szDir, COUNTOF(szDir));
			if (FAILED(hr))return hr;
			info.workingDir = szDir;
		}

		//get icon and convert to bitmap
		SHFILEINFO sfi={0};
		SHGetFileInfoW(path.c_str(), 0, &sfi, sizeof(sfi), SHGFI_ICON | SHGFI_SMALLICON);
		if(!info.cIconBmpSmall.IsNull())info.cIconBmpSmall.DeleteObject();
		UtilMakeDIBFromIcon(info.cIconBmpSmall, sfi.hIcon);
		DestroyIcon(sfi.hIcon);
	}
	return S_OK;
}


void UtilNavigateDirectory(const std::wstring& dir)
{
	//The maximum size of the buffer specified by the lpBuffer parameter, in TCHARs.
	//This value should be set to MAX_PATH.
	ShellExecuteW(nullptr, L"open", dir.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
}

//retrieve environment variables as key=value pair
std::map<std::wstring, std::wstring> UtilGetEnvInfo()
{
	/*
	https://docs.microsoft.com/ja-jp/windows/win32/procthread/environment-variables
	Var1=Value1\0
	Var2=Value2\0
	Var3=Value3\0
	...
	VarN=ValueN\0\0

	The name of an environment variable cannot include an equal sign (=).
	*/

	std::map<std::wstring, std::wstring> envInfo;
	wchar_t* lpRaw = GetEnvironmentStringsW();
	const wchar_t* lpEnvStr = lpRaw;
	if (!lpEnvStr) {
		return envInfo;
	}

	for (; *lpEnvStr != L'\0'; ) {
		std::wstring block = lpEnvStr;
		//TRACE(L"%s\n", block.c_str());
		lpEnvStr += block.length()+1;

		auto pos = block.find_first_of(L'=');
		if (pos == std::wstring::npos) continue;	//mis-formatted string
		
		auto key = block.substr(0, pos);
		auto value = block.substr(pos+1);
		if (!key.empty()) {
			envInfo[toUpper(key)] = value;
		}
	}

	FreeEnvironmentStringsW(lpRaw);
	return envInfo;
}

void UtilMakeDIBFromIcon(CBitmap &bitmap,HICON icon)
{
	ICONINFO ii;
	::GetIconInfo(icon,&ii);
	BITMAP bm;
	GetObject(ii.hbmColor,sizeof(bm),&bm);

	BITMAPINFO bmpinfo={0};
	bmpinfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmpinfo.bmiHeader.biWidth = bm.bmWidth;
	bmpinfo.bmiHeader.biHeight = bm.bmWidth;
	bmpinfo.bmiHeader.biPlanes = 1;
	bmpinfo.bmiHeader.biBitCount = 32;
	bmpinfo.bmiHeader.biCompression = BI_RGB;

	HDC hTempDC=::GetDC(NULL);
	CDC hDestDC,hSrcDC;
	hDestDC.CreateCompatibleDC(hTempDC);
	hSrcDC.CreateCompatibleDC(hDestDC);
	bitmap.CreateDIBSection(NULL,&bmpinfo,DIB_RGB_COLORS,NULL,NULL,NULL);
	HBITMAP hOldDest=hDestDC.SelectBitmap(bitmap);
	HBITMAP hOldSrc=hSrcDC.SelectBitmap(ii.hbmColor);

	//hDestDC.MaskBlt(0,0,16,16,hSrcDC,0,0,ii.hbmMask,0,0,MAKEROP4(SRCCOPY, PATCOPY));
	hDestDC.BitBlt(0,0,bm.bmWidth,bm.bmHeight,hSrcDC,0,0,SRCCOPY);

	hDestDC.SelectBitmap(hOldDest);
	hSrcDC.SelectBitmap(hOldSrc);

	DeleteObject(ii.hbmColor);
	DeleteObject(ii.hbmMask);
	ReleaseDC(NULL,hTempDC);
}


//Process priority
void UtilSetPriorityClass(DWORD dwPriorityClass)
{
	HANDLE hProcess=GetCurrentProcess();
	SetPriorityClass(hProcess,dwPriorityClass);
}


//Copy text to clipboard
void UtilSetTextOnClipboard(const std::wstring& text)
{
	HGLOBAL hMem;
	wchar_t* lpBuff;

	hMem = GlobalAlloc((GHND|GMEM_SHARE),(text.length() + 1) * sizeof(wchar_t));
	if(hMem){
		lpBuff = (wchar_t*)GlobalLock(hMem);
		if (lpBuff){
			wcscpy_s(lpBuff, text.length() + 1, text.c_str());
			GlobalUnlock( hMem );

			if ( OpenClipboard(NULL) ){
				EmptyClipboard();
				SetClipboardData( CF_UNICODETEXT, hMem );
				CloseClipboard();
			}else{
				GlobalFree( hMem );
			}
		}else{
			GlobalFree( hMem );
		}
	}
}

std::pair<std::wstring, int> UtilPathParseIconLocation(const std::wstring& path_and_index)
{
	std::wregex re_path(L"^(.+?),(-?\\d+)$");

	std::wcmatch results;
	if (std::regex_search(path_and_index.c_str(), results, re_path)) {
		std::wstring path = results[1];
		std::wstring index = results[2];
		return std::make_pair<>(path, _wtoi(index.c_str()));
	}
	return std::make_pair<>(path_and_index, 0);
}
