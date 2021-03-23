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

//convert icon into bitmap with alpha information
void makeDIBFromIcon(CBitmap &bitmap, HICON icon)
{
	ICONINFO ii;
	::GetIconInfo(icon, &ii);
	BITMAP bm;
	GetObject(ii.hbmColor, sizeof(bm), &bm);

	BITMAPINFO bmpinfo = { 0 };
	bmpinfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmpinfo.bmiHeader.biWidth = bm.bmWidth;
	bmpinfo.bmiHeader.biHeight = bm.bmWidth;
	bmpinfo.bmiHeader.biPlanes = 1;
	bmpinfo.bmiHeader.biBitCount = 32;
	bmpinfo.bmiHeader.biCompression = BI_RGB;

	HDC hTempDC = ::GetDC(NULL);
	CDC hDestDC, hSrcDC;
	hDestDC.CreateCompatibleDC(hTempDC);
	hSrcDC.CreateCompatibleDC(hDestDC);
	bitmap.CreateDIBSection(NULL, &bmpinfo, DIB_RGB_COLORS, NULL, NULL, NULL);
	HBITMAP hOldDest = hDestDC.SelectBitmap(bitmap);
	HBITMAP hOldSrc = hSrcDC.SelectBitmap(ii.hbmColor);

	//hDestDC.MaskBlt(0,0,16,16,hSrcDC,0,0,ii.hbmMask,0,0,MAKEROP4(SRCCOPY, PATCOPY));
	hDestDC.BitBlt(0, 0, bm.bmWidth, bm.bmHeight, hSrcDC, 0, 0, SRCCOPY);

	hDestDC.SelectBitmap(hOldDest);
	hSrcDC.SelectBitmap(hOldSrc);

	DeleteObject(ii.hbmColor);
	DeleteObject(ii.hbmMask);
	ReleaseDC(NULL, hTempDC);
}

HRESULT UtilCreateShortcut(
	const std::filesystem::path& pathLink,
	const std::filesystem::path& pathTarget,
	const std::wstring& args,
	const std::filesystem::path& iconPath,
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

HRESULT UtilGetShortcutInfo(const std::filesystem::path& path, UTIL_SHORTCUTINFO& info)
{
	CComPtr<IShellLinkW> pLink;
	HRESULT hr = pLink.CoCreateInstance(CLSID_ShellLink);
	if (FAILED(hr))return hr;

	pLink->SetPath(path.c_str());
	CComQIPtr<IPersistFile> pFile = pLink;
	if(pFile){
		hr = pFile->Load(path.c_str(), STGM_READ);
		if (FAILED(hr))return hr;
		info.title = path.stem().c_str();

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
		makeDIBFromIcon(info.cIconBmpSmall, sfi.hIcon);
		DestroyIcon(sfi.hIcon);
	}
	return S_OK;
}

#ifdef UNIT_TEST
TEST(OSUtil, UtilCreateShortcut_UtilGetShortcutInfo) {
	auto temp_dir = std::filesystem::temp_directory_path();
	auto link_file = temp_dir / "test.lnk";
	const wchar_t* target = LR"(C:\Windows\notepad.exe)";
	const wchar_t* args = L"";
	const wchar_t* icon_file = LR"(C:\Windows\System32\SHELL32.dll)";
	const int icon_index = 5;
	const wchar_t* desc = L"test link";

	EXPECT_FALSE(std::filesystem::exists(link_file));
	EXPECT_EQ(S_OK, UtilCreateShortcut(
		link_file.c_str(),
		target,
		args,
		icon_file,
		icon_index,
		desc));
	EXPECT_TRUE(std::filesystem::exists(link_file));

	UTIL_SHORTCUTINFO info;
	EXPECT_EQ(S_OK, UtilGetShortcutInfo(link_file, info));
	EXPECT_EQ(toLower(target), toLower(info.cmd));
	EXPECT_EQ(args, info.param);
	EXPECT_EQ(L"", info.workingDir);
	std::filesystem::remove(link_file);
}
#endif

void UtilNavigateDirectory(const std::filesystem::path& dir)
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

#ifdef UNIT_TEST
TEST(OSUtil, UtilGetEnvInfo) {
	auto envInfo = UtilGetEnvInfo();
	EXPECT_TRUE(has_key(envInfo, L"PATH"));
	for (const auto& item : envInfo) {
		wchar_t buf[_MAX_ENV] = {};
		size_t s = 0;
		_wgetenv_s(&s, buf, item.first.c_str());
		std::wstring env = buf;
		EXPECT_EQ(std::wstring(env), item.second);
	}
}
#endif

std::wstring UtilGetWindowClassName(HWND hWnd)
{
	std::wstring name;
	name.resize(256);
	for (;;) {
		int bufsize = (int)name.size();
		auto nCopied = GetClassNameW(hWnd, &name[0], bufsize);
		if (nCopied < bufsize) {
			break;
		} else {
			name.resize(name.size() * 2);
		}
	}
	return name.c_str();
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

			if ( OpenClipboard(nullptr) ){
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

#ifdef UNIT_TEST
TEST(OSUtil, UtilSetTextOnClipboard)
{
	const auto string = L"abcdeあいうえお";

	UtilSetTextOnClipboard(string);
	ASSERT_TRUE(IsClipboardFormatAvailable(CF_UNICODETEXT));
	ASSERT_TRUE(OpenClipboard(nullptr));
	HGLOBAL hg = nullptr;
	hg = GetClipboardData(CF_UNICODETEXT);
	ASSERT_NE(nullptr, hg);
	std::wstring p = (const wchar_t*)GlobalLock(hg);

	GlobalUnlock(hg);
	CloseClipboard();

	EXPECT_EQ(string, p);
}
#endif

std::pair<std::filesystem::path, int> UtilPathParseIconLocation(const std::wstring& path_and_index)
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

#ifdef UNIT_TEST

TEST(OSUtil, UtilPathParseIconLocation)
{
	{
		auto path_and_index = UtilPathParseIconLocation(L"c:/te,st/icon.dll,5");
		EXPECT_EQ(path_and_index.first, L"c:/te,st/icon.dll");
		EXPECT_EQ(path_and_index.second, 5);
	}

	{
		auto path_and_index = UtilPathParseIconLocation(L"c:/test/icon.dll,-1");
		EXPECT_EQ(path_and_index.first, L"c:/test/icon.dll");
		EXPECT_EQ(path_and_index.second, -1);
	}

	{
		auto path_and_index = UtilPathParseIconLocation(L"c:/test/icon.dll");
		EXPECT_EQ(path_and_index.first, L"c:/test/icon.dll");
		EXPECT_EQ(path_and_index.second, 0);
	}

	{
		auto path_and_index = UtilPathParseIconLocation(L"c:/test/icon.dll,");
		EXPECT_EQ(path_and_index.first, L"c:/test/icon.dll,");
		EXPECT_EQ(path_and_index.second, 0);
	}
}


TEST(OSUtil, CurrentDirManager) {
	auto prevPath = std::filesystem::current_path();
	{
		CCurrentDirManager cdm(std::filesystem::temp_directory_path().c_str());
		auto currentPath = UtilPathAddLastSeparator(std::filesystem::current_path());
		EXPECT_EQ(UtilPathAddLastSeparator(std::filesystem::temp_directory_path()),
			currentPath);
	}
	auto currentPath = std::filesystem::current_path();
	EXPECT_EQ(prevPath.wstring(), currentPath.wstring());

	auto path = std::filesystem::temp_directory_path() / L"lf_path_test";
	{
		std::filesystem::create_directories(path);
		CCurrentDirManager cdm(path.c_str());
		//what if previous directory does not exist?
		EXPECT_THROW({
			CCurrentDirManager cdm2(prevPath.c_str());
			std::filesystem::remove(path);
			EXPECT_FALSE(std::filesystem::exists(path));
			}, LF_EXCEPTION);
	}
}


#endif

