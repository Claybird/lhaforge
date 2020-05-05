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
	const wchar_t* lpszPathLink,
	const wchar_t* lpszPathTarget,
	const wchar_t* lpszArgs,
	const wchar_t* lpszIconPath,
	int iIcon,
	LPCTSTR lpszDescription)
{
	CComPtr<IShellLinkW> pLink;
	HRESULT hr = pLink.CoCreateInstance(CLSID_ShellLink);
	if (FAILED(hr))return hr;

	// such as C:\Windows\notepad.exe
	pLink->SetPath(lpszPathTarget);
	pLink->SetArguments(lpszArgs);
	pLink->SetIconLocation(lpszIconPath,iIcon);
	pLink->SetDescription(lpszDescription);
	//save to file
	CComQIPtr<IPersistFile> pFile = pLink;
	if (pFile) {
		return pFile->Save(lpszPathLink, TRUE);
	} else {
		return E_FAIL;
	}
}

HRESULT UtilGetShortcutInfo(const wchar_t* lpPath, UTIL_SHORTCUTINFO& info)
{
	CComPtr<IShellLinkW> pLink;
	HRESULT hr = pLink.CoCreateInstance(CLSID_ShellLink);
	if (FAILED(hr))return hr;

	pLink->SetPath(lpPath);
	CComQIPtr<IPersistFile> pFile = pLink;
	if(pFile){
		hr = pFile->Load(lpPath, STGM_READ);
		if (FAILED(hr))return hr;
		info.title = std::filesystem::path(lpPath).stem().c_str();

		{
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
		SHGetFileInfo(lpPath, 0, &sfi, sizeof(sfi), SHGFI_ICON | SHGFI_SMALLICON);
		UtilMakeDIBFromIcon(info.cIconBmpSmall, sfi.hIcon);
		DestroyIcon(sfi.hIcon);
	}
	return S_OK;
}


void UtilNavigateDirectory(LPCTSTR lpszDir)
{
	//Explorerで開く
	TCHAR szExplorerPath[_MAX_PATH+1];
	FILL_ZERO(szExplorerPath);
	GetWindowsDirectory(szExplorerPath,_MAX_PATH);
	PathAppend(szExplorerPath,_T("Explorer.exe"));
	ShellExecute(NULL, _T("open"), szExplorerPath, lpszDir, NULL, SW_SHOWNORMAL);
}

//環境変数を参照し、辞書形式で取得する
void UtilGetEnvInfo(std::map<stdString,stdString> &envInfo)
{
	LPTSTR lpEnvStr = GetEnvironmentStrings();
	LPCTSTR lpStr    = lpEnvStr;

	for(;*lpStr!='\0';){
		LPCTSTR lpSplit=lpStr+1;	//環境変数名の先頭が'='だった場合に備える
		for(;*lpSplit!=L'\0' && *lpSplit != L'=';lpSplit++)continue;

		//---環境変数名
		stdString strKey(lpStr,lpSplit);
		//大文字に正規化
		std::transform(strKey.begin(), strKey.end(), strKey.begin(), std::toupper);
		//---環境変数の値
		CString strValue=lpSplit+1;
		envInfo[strKey]=strValue;

		for(lpStr=lpSplit;*lpStr!=L'\0';lpStr++)continue;
		lpStr++;
	}

	FreeEnvironmentStrings(lpEnvStr);
}

//UtilExpandTemplateString()のパラメータ展開に必要な情報を構築する
void UtilMakeExpandInformation(std::map<stdString,CString> &envInfo)
{
	//環境変数で構築
	std::map<stdString,stdString> envs;
	UtilGetEnvInfo(envs);
	for(std::map<stdString,stdString>::iterator ite=envs.begin();ite!=envs.end();++ite){
		//%ENVIRONMENT%の形式に変換
		envInfo[L'%'+(*ite).first+L'%']=(*ite).second.c_str();
	}

	//---LhaForge本体の情報
	envInfo[_T("ProgramPath")]=UtilGetModulePath();
	envInfo[_T("ProgramFileName")]=PathFindFileName(UtilGetModulePath());

	CPath dir=UtilGetModuleDirectoryPath();
	dir.RemoveBackslash();
	envInfo[_T("ProgramDir")]=(LPCTSTR)dir;

	dir.StripToRoot();
	//末尾のBackslashを取り除く;RemoveBackslashではドライブ名直後のBackslashが取り除けない
	dir.RemoveBackslash();
	envInfo[_T("ProgramDrive")]=(LPCTSTR)dir;
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


//プロセス優先度の設定
void UtilSetPriorityClass(DWORD dwPriorityClass)
{
	HANDLE hProcess=GetCurrentProcess();
	SetPriorityClass(hProcess,dwPriorityClass);
}


//クリップボードにテキストを設定
void UtilSetTextOnClipboard(LPCTSTR lpszText)
{
	HGLOBAL hMem;
	LPTSTR lpBuff;

	hMem = GlobalAlloc((GHND|GMEM_SHARE),(_tcslen(lpszText) + 1) * sizeof(TCHAR));
	if(hMem){
		lpBuff = (LPTSTR)GlobalLock(hMem);
		if (lpBuff){
			_tcscpy_s(lpBuff, _tcslen(lpszText)+1, lpszText);
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
