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

HRESULT UtilCreateShortcut(
	const std::filesystem::path& pathLink,
	const std::filesystem::path& pathTarget,
	const std::wstring& args,
	const std::filesystem::path& iconPath,
	int iIcon,
	LPCTSTR lpszDescription);

struct UTIL_SHORTCUTINFO {
	std::wstring title;
	std::filesystem::path cmd;
	std::wstring param;
	std::filesystem::path workingDir;
	CBitmap cIconBmpSmall;
};

HRESULT UtilGetShortcutInfo(const std::filesystem::path& path, UTIL_SHORTCUTINFO& info);

//Open a folder with explorer
void UtilNavigateDirectory(const std::filesystem::path& dir);

//retrieve environment variables as key=value pair
std::map<std::wstring, std::wstring> UtilGetEnvInfo();

//Copy text to clipboard
void UtilSetTextOnClipboard(const std::wstring& text);

std::pair<std::filesystem::path, int> UtilPathParseIconLocation(const std::wstring& path_and_index);

//moves to a directory, and comes back to the previous directory on destructor
class CCurrentDirManager
{
	DISALLOW_COPY_AND_ASSIGN(CCurrentDirManager);
protected:
	std::filesystem::path _prevDir;
public:
	CCurrentDirManager(const std::filesystem::path& chdirTo) {
		_prevDir = std::filesystem::current_path();
		try {
			std::filesystem::current_path(chdirTo);
		} catch (std::filesystem::filesystem_error) {
			RAISE_EXCEPTION(L"Failed to chdir to %s", chdirTo);
		}
	}
	virtual ~CCurrentDirManager() noexcept(false) {
		try {
			std::filesystem::current_path(_prevDir);
		} catch (std::filesystem::filesystem_error) {
			RAISE_EXCEPTION(L"Failed to chdir to %s", _prevDir.c_str());
		}
	}
};

class LFShellFileOpenDialog : public CShellFileOpenDialog
{
public:
	LFShellFileOpenDialog(LPCWSTR lpszFileName = nullptr, 
	                     DWORD dwOptions = FOS_FORCEFILESYSTEM | FOS_PATHMUSTEXIST | FOS_FILEMUSTEXIST, 
	                     LPCWSTR lpszDefExt = nullptr,
	                     const COMDLG_FILTERSPEC* arrFilterSpec = nullptr,
	                     UINT uFilterSpecCount = 0U) : CShellFileOpenDialog(lpszFileName ? std::filesystem::path(lpszFileName).filename().c_str() : nullptr, dwOptions, lpszDefExt, arrFilterSpec, uFilterSpecCount)
	{
		if (lpszFileName) {
			ATL::CComPtr<IShellItem> spItem;
			auto path = std::filesystem::path(lpszFileName).make_preferred();
			HRESULT hr = SHCreateItemFromParsingName(
				path.parent_path().c_str(),
				nullptr, IID_IShellItem, (void**)&spItem);
			if (SUCCEEDED(hr)) {
				GetPtr()->SetFolder(spItem);
			}
		}
	}

	virtual ~LFShellFileOpenDialog()
	{ }
	std::vector<CString> GetMultipleFiles() {
		std::vector<CString> files;
		{
			auto ptr = GetPtr();
			ATL::CComPtr<IShellItemArray> spArray;
			HRESULT hRet = ptr->GetResults(&spArray);

			if (SUCCEEDED(hRet)) {
				DWORD count;
				spArray->GetCount(&count);
				for (DWORD i = 0; i < count; i++) {
					ATL::CComPtr<IShellItem> spItem;
					spArray->GetItemAt(i, &spItem);
					CString path;
					GetFileNameFromShellItem(spItem, SIGDN_FILESYSPATH, path);
					files.push_back(path);
				}
			}
		}
		return files;
	}
};

class LFShellFileSaveDialog : public CShellFileSaveDialog
{
public:
	LFShellFileSaveDialog(LPCWSTR lpszFileName = nullptr,
		DWORD dwOptions = FOS_FORCEFILESYSTEM | FOS_PATHMUSTEXIST | FOS_FILEMUSTEXIST,
		LPCWSTR lpszDefExt = nullptr,
		const COMDLG_FILTERSPEC* arrFilterSpec = nullptr,
		UINT uFilterSpecCount = 0U) : CShellFileSaveDialog(lpszFileName ? std::filesystem::path(lpszFileName).filename().c_str() : nullptr, dwOptions, lpszDefExt, arrFilterSpec, uFilterSpecCount)
	{
		if (lpszFileName) {
			ATL::CComPtr<IShellItem> spItem;
			auto path = std::filesystem::path(lpszFileName).make_preferred();
			HRESULT hr = SHCreateItemFromParsingName(
				path.parent_path().c_str(),
				nullptr, IID_IShellItem, (void**)&spItem);
			if (SUCCEEDED(hr)) {
				GetPtr()->SetFolder(spItem);
			}
		}
	}

	virtual ~LFShellFileSaveDialog()
	{ }
};
