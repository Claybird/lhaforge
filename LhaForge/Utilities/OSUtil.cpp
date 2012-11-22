/*
 * Copyright (c) 2005-2012, Claybird
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
#include "OSUtil.h"
#include "Utility.h"
#include "FileOperation.h"

//=======================================================
//From http://www.athomejp.com/goldfish/api/shortcut.asp
//=======================================================
HRESULT UtilCreateShortcut(LPCTSTR lpszPathLink,LPCTSTR lpszPathTarget,LPCTSTR lpszArgs,LPCTSTR lpszIconPath,int iIcon,LPCTSTR lpszDescription)
{
	HRESULT hRes;
	IShellLink* psl =NULL;

	// IShellLink インターフェイスを取得
	hRes = CoCreateInstance(
		CLSID_ShellLink,NULL,
		CLSCTX_INPROC_SERVER,
		IID_IShellLink,
		(LPVOID *)&psl);

	if(FAILED(hRes)){
		//失敗
		//ErrorMessage(CString(MAKEINTRESOURCE(IDS_ERROR_CREATE_SHORTCUT)));
		return hRes;
	}

	IPersistFile* ppf=NULL;
	// Linkオブジェクトのパスを設定(たとえば、C:\Windows\notepad.exeなど)
	psl->SetPath(lpszPathTarget);
	//引数設定
	psl->SetArguments(lpszArgs);
	//アイコン設定
	psl->SetIconLocation(lpszIconPath,iIcon);
	//説明文設定
	psl->SetDescription(lpszDescription);
	// IPersistFileインターフェイスを取得後Linkパスファイル名を保存
	hRes =psl->QueryInterface( IID_IPersistFile,(void**)&ppf);
	if(FAILED(hRes)){
		//ppfの取得に失敗
		psl->Release();
		return hRes;
	}

#if defined(_UNICODE)||defined(UNICODE)
	hRes = ppf->Save(lpszPathLink, TRUE);
#else
	WCHAR wsz[_MAX_PATH+1];

	MultiByteToWideChar(CP_ACP, 0,lpszPathLink, -1,wsz, _MAX_PATH);
	// ディスクに保存する
	hRes = ppf->Save(wsz, TRUE);
#endif
	ppf->Release();
	psl->Release();
	return hRes;
}

HRESULT UtilGetShortcutInfo(LPCTSTR lpPath,CString &strTargetPath,CString &strParam,CString &strWorkingDir)
{
	CComPtr<IShellLink> pLink;

	// IShellLink インターフェイスを取得
	HRESULT hr = pLink.CoCreateInstance(CLSID_ShellLink);

	if(SUCCEEDED(hr)){
		pLink->SetPath(lpPath);
		CComQIPtr<IPersistFile> pFile=pLink;
		if(pFile){
			hr=pFile->Load(lpPath,STGM_READ);
			if(SUCCEEDED(hr)){
				{
					WCHAR szBuffer[_MAX_PATH+1];
					hr=pLink->GetPath(szBuffer,COUNTOF(szBuffer),NULL,0);
					if(FAILED(hr))return hr;
					strTargetPath=szBuffer;
				}

				{
					WCHAR szArg[INFOTIPSIZE+1];
					hr=pLink->GetArguments(szArg,COUNTOF(szArg));
					if(FAILED(hr))return hr;
					strParam=szArg;
				}

				{
					WCHAR szDir[_MAX_PATH+1];
					hr=pLink->GetWorkingDirectory(szDir,COUNTOF(szDir));
					if(FAILED(hr))return hr;
					strWorkingDir=szDir;
				}
			}
		}
	}
	return hr;
}

void UtilGetShortcutInfo(const std::vector<CString> &files,std::vector<SHORTCUTINFO> &info)
{
	CComPtr<IShellLink> pLink;

	// IShellLink インターフェイスを取得
	HRESULT hr = pLink.CoCreateInstance(CLSID_ShellLink);

	info.clear();
	if(SUCCEEDED(hr)){
		size_t size=files.size();
		info.resize(size);
		for(size_t i=0;i<size;i++){
			pLink->SetPath(files[i]);
			CComQIPtr<IPersistFile> pFile=pLink;
			if(pFile){
				hr=pFile->Load(files[i],STGM_READ);
				if(FAILED(hr))continue;

				WCHAR szTarget[_MAX_PATH+1];
				hr=pLink->GetPath(szTarget,COUNTOF(szTarget),NULL,0);
				if(FAILED(hr))continue;

				WCHAR szArg[INFOTIPSIZE+1];
				hr=pLink->GetArguments(szArg,COUNTOF(szArg));
				if(FAILED(hr))continue;

				WCHAR szDir[_MAX_PATH+1];
				hr=pLink->GetWorkingDirectory(szDir,COUNTOF(szDir));
				if(FAILED(hr))continue;

				//通常の情報
				CPath title=files[i];
				title.StripPath();
				title.RemoveExtension();
				info[i].strTitle=(LPCTSTR)title;
				info[i].strCmd=szTarget;
				info[i].strParam=szArg;
				info[i].strWorkingDir=szDir;

				//アイコン情報:アイコンを取得し、ビットマップに変換
				SHFILEINFO sfi={0};
				SHGetFileInfo(files[i],0,&sfi,sizeof(sfi),SHGFI_ICON | SHGFI_SMALLICON);
				UtilMakeDIBFromIcon(info[i].cIconBmpSmall,sfi.hIcon);
				DestroyIcon(sfi.hIcon);
			}
		}
	}
}

bool UtilIsWindowsNT()
{
	OSVERSIONINFO osi;
	FILL_ZERO(osi);
	osi.dwOSVersionInfoSize=sizeof(osi);
	GetVersionEx(&osi);
	return (VER_PLATFORM_WIN32_NT==osi.dwPlatformId);
}


//From http://techtips.belution.com/ja/vc/0012/
void UtilSetAbsoluteForegroundWindow(HWND hWnd)
{
	int nTargetID, nForegroundID;
	DWORD sp_time;

	// フォアグラウンドウィンドウを作成したスレッドのIDを取得
	nForegroundID = GetWindowThreadProcessId(GetForegroundWindow(), NULL);
	// 目的のウィンドウを作成したスレッドのIDを取得
	nTargetID = GetWindowThreadProcessId(hWnd, NULL );

	// スレッドのインプット状態を結び付ける
	AttachThreadInput(nTargetID, nForegroundID, TRUE );  // TRUE で結び付け

	// 現在の設定を sp_time に保存
	SystemParametersInfo( SPI_GETFOREGROUNDLOCKTIMEOUT,0,&sp_time,0);
	// ウィンドウの切り替え時間を 0ms にする
	SystemParametersInfo( SPI_SETFOREGROUNDLOCKTIMEOUT,0,(LPVOID)0,0);

	// ウィンドウをフォアグラウンドに持ってくる
	SetForegroundWindow(hWnd);

	// 設定を元に戻す
	SystemParametersInfo(SPI_SETFOREGROUNDLOCKTIMEOUT,0,&sp_time,0);

	// スレッドのインプット状態を切り離す
	AttachThreadInput(nTargetID, nForegroundID, FALSE );  // FALSE で切り離し
}


//現在のOSがWindowsVistaもしくはそれ以上ならtrueを返す
bool UtilIsOSVistaOrHigher()
{
	OSVERSIONINFO osv={0};
	osv.dwOSVersionInfoSize=sizeof(osv);
	if(!::GetVersionEx(&osv)){
		ASSERT(!"GetVersionEx failed");
		return false;
	}
	if(osv.dwMajorVersion>6||(osv.dwMajorVersion==6&&osv.dwMinorVersion>=0))return true;
	return false;
}


BOOL UtilIsWow64()
{
	typedef BOOL (WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);

	BOOL bIsWow64 = FALSE;

	LPFN_ISWOW64PROCESS fnIsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress(GetModuleHandle(_T("kernel32")),"IsWow64Process");

	if(fnIsWow64Process){
		if(!fnIsWow64Process(GetCurrentProcess(),&bIsWow64)){
			return FALSE;
		}
	}
	return bIsWow64;
}


//コマンドライン引数を取得(個数を返す)
int UtilGetCommandLineParams(std::vector<CString> &rParams)
{
#if defined(_UNICODE)||defined(UNICODE)
	int nArgc=0;
	LPWSTR *lplpArgs=CommandLineToArgvW(GetCommandLine(), &nArgc);
	rParams.resize(nArgc);
	for(int i=0;i<nArgc;i++){
		rParams[i]=lplpArgs[i];
	}
	LocalFree(lplpArgs);
	return nArgc;
#else//defined(_UNICODE)||defined(UNICODE)
	rParams.resize(__argc);
	for(int i=0;i<__argc;i++){
		rParams[i]=__argv[i];
	}
	return __argc;
#endif//defined(_UNICODE)||defined(UNICODE)
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

