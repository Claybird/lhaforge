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

	// IShellLink �C���^�[�t�F�C�X���擾
	hRes = CoCreateInstance(
		CLSID_ShellLink,NULL,
		CLSCTX_INPROC_SERVER,
		IID_IShellLink,
		(LPVOID *)&psl);

	if(FAILED(hRes)){
		//���s
		//ErrorMessage(CString(MAKEINTRESOURCE(IDS_ERROR_CREATE_SHORTCUT)));
		return hRes;
	}

	IPersistFile* ppf=NULL;
	// Link�I�u�W�F�N�g�̃p�X��ݒ�(���Ƃ��΁AC:\Windows\notepad.exe�Ȃ�)
	psl->SetPath(lpszPathTarget);
	//�����ݒ�
	psl->SetArguments(lpszArgs);
	//�A�C�R���ݒ�
	psl->SetIconLocation(lpszIconPath,iIcon);
	//�������ݒ�
	psl->SetDescription(lpszDescription);
	// IPersistFile�C���^�[�t�F�C�X���擾��Link�p�X�t�@�C������ۑ�
	hRes =psl->QueryInterface( IID_IPersistFile,(void**)&ppf);
	if(FAILED(hRes)){
		//ppf�̎擾�Ɏ��s
		psl->Release();
		return hRes;
	}

#if defined(_UNICODE)||defined(UNICODE)
	hRes = ppf->Save(lpszPathLink, TRUE);
#else
	WCHAR wsz[_MAX_PATH+1];

	MultiByteToWideChar(CP_ACP, 0,lpszPathLink, -1,wsz, _MAX_PATH);
	// �f�B�X�N�ɕۑ�����
	hRes = ppf->Save(wsz, TRUE);
#endif
	ppf->Release();
	psl->Release();
	return hRes;
}

HRESULT UtilGetShortcutInfo(LPCTSTR lpPath,CString &strTargetPath,CString &strParam,CString &strWorkingDir)
{
	CComPtr<IShellLink> pLink;

	// IShellLink �C���^�[�t�F�C�X���擾
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

	// IShellLink �C���^�[�t�F�C�X���擾
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

				//�ʏ�̏��
				CPath title=files[i];
				title.StripPath();
				title.RemoveExtension();
				info[i].strTitle=(LPCTSTR)title;
				info[i].strCmd=szTarget;
				info[i].strParam=szArg;
				info[i].strWorkingDir=szDir;

				//�A�C�R�����:�A�C�R�����擾���A�r�b�g�}�b�v�ɕϊ�
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

	// �t�H�A�O���E���h�E�B���h�E���쐬�����X���b�h��ID���擾
	nForegroundID = GetWindowThreadProcessId(GetForegroundWindow(), NULL);
	// �ړI�̃E�B���h�E���쐬�����X���b�h��ID���擾
	nTargetID = GetWindowThreadProcessId(hWnd, NULL );

	// �X���b�h�̃C���v�b�g��Ԃ����ѕt����
	AttachThreadInput(nTargetID, nForegroundID, TRUE );  // TRUE �Ō��ѕt��

	// ���݂̐ݒ�� sp_time �ɕۑ�
	SystemParametersInfo( SPI_GETFOREGROUNDLOCKTIMEOUT,0,&sp_time,0);
	// �E�B���h�E�̐؂�ւ����Ԃ� 0ms �ɂ���
	SystemParametersInfo( SPI_SETFOREGROUNDLOCKTIMEOUT,0,(LPVOID)0,0);

	// �E�B���h�E���t�H�A�O���E���h�Ɏ����Ă���
	SetForegroundWindow(hWnd);

	// �ݒ�����ɖ߂�
	SystemParametersInfo(SPI_SETFOREGROUNDLOCKTIMEOUT,0,&sp_time,0);

	// �X���b�h�̃C���v�b�g��Ԃ�؂藣��
	AttachThreadInput(nTargetID, nForegroundID, FALSE );  // FALSE �Ő؂藣��
}


//���݂�OS��WindowsVista�������͂���ȏ�Ȃ�true��Ԃ�
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


//�R�}���h���C���������擾(����Ԃ�)
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
	//Explorer�ŊJ��
	TCHAR szExplorerPath[_MAX_PATH+1];
	FILL_ZERO(szExplorerPath);
	GetWindowsDirectory(szExplorerPath,_MAX_PATH);
	PathAppend(szExplorerPath,_T("Explorer.exe"));
	ShellExecute(NULL, _T("open"), szExplorerPath, lpszDir, NULL, SW_SHOWNORMAL);
}

//���ϐ����Q�Ƃ��A�����`���Ŏ擾����
void UtilGetEnvInfo(std::map<stdString,stdString> &envInfo)
{
	LPTSTR lpEnvStr = GetEnvironmentStrings();
	LPCTSTR lpStr    = lpEnvStr;

	for(;*lpStr!='\0';){
		LPCTSTR lpSplit=lpStr+1;	//���ϐ����̐擪��'='�������ꍇ�ɔ�����
		for(;*lpSplit!=L'\0' && *lpSplit != L'=';lpSplit++)continue;

		//---���ϐ���
		stdString strKey(lpStr,lpSplit);
		//�啶���ɐ��K��
		std::transform(strKey.begin(), strKey.end(), strKey.begin(), std::toupper);
		//---���ϐ��̒l
		CString strValue=lpSplit+1;
		envInfo[strKey]=strValue;

		for(lpStr=lpSplit;*lpStr!=L'\0';lpStr++)continue;
		lpStr++;
	}

	FreeEnvironmentStrings(lpEnvStr);
}

//UtilExpandTemplateString()�̃p�����[�^�W�J�ɕK�v�ȏ����\�z����
void UtilMakeExpandInformation(std::map<stdString,CString> &envInfo)
{
	//���ϐ��ō\�z
	std::map<stdString,stdString> envs;
	UtilGetEnvInfo(envs);
	for(std::map<stdString,stdString>::iterator ite=envs.begin();ite!=envs.end();++ite){
		//%ENVIRONMENT%�̌`���ɕϊ�
		envInfo[L'%'+(*ite).first+L'%']=(*ite).second.c_str();
	}

	//---LhaForge�{�̂̏��
	envInfo[_T("ProgramPath")]=UtilGetModulePath();
	envInfo[_T("ProgramFileName")]=PathFindFileName(UtilGetModulePath());

	CPath dir=UtilGetModuleDirectoryPath();
	dir.RemoveBackslash();
	envInfo[_T("ProgramDir")]=(LPCTSTR)dir;

	dir.StripToRoot();
	//������Backslash����菜��;RemoveBackslash�ł̓h���C�u�������Backslash����菜���Ȃ�
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


//�v���Z�X�D��x�̐ݒ�
void UtilSetPriorityClass(DWORD dwPriorityClass)
{
	HANDLE hProcess=GetCurrentProcess();
	SetPriorityClass(hProcess,dwPriorityClass);
}

