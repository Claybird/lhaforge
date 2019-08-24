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
