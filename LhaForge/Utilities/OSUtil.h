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
	const std::wstring& pathLink,
	const std::wstring& pathTarget,
	const std::wstring& args,
	const std::wstring& iconPath,
	int iIcon,
	LPCTSTR lpszDescription);

struct UTIL_SHORTCUTINFO {
	std::wstring title;
	std::wstring cmd;
	std::wstring param;
	std::wstring workingDir;
	CBitmap cIconBmpSmall;
};

HRESULT UtilGetShortcutInfo(const std::wstring& path, UTIL_SHORTCUTINFO& info);

//Open a folder with explorer
void UtilNavigateDirectory(const std::wstring& dir);

//retrieve environment variables as key=value pair
std::map<std::wstring, std::wstring> UtilGetEnvInfo();

//convert icon into bitmap with alpha information
void UtilMakeDIBFromIcon(CBitmap&,HICON);

enum LFPROCESS_PRIORITY {
	LFPRIOTITY_DEFAULT = 0,
	LFPRIOTITY_LOW = 1,
	LFPRIOTITY_LOWER = 2,
	LFPRIOTITY_NORMAL = 3,
	LFPRIOTITY_HIGHER = 4,
	LFPRIOTITY_HIGH = 5,
	LFPRIOTITY_MAX_NUM = LFPRIOTITY_HIGH,
};

//Process priority
void UtilSetPriorityClass(DWORD dwPriorityClass);

//Copy text to clipboard
void UtilSetTextOnClipboard(const std::wstring& text);

std::pair<std::wstring, int> UtilPathParseIconLocation(const std::wstring& path_and_index);

//moves to a directory, and comes back to the previous directory on destructor
class CCurrentDirManager
{
	DISALLOW_COPY_AND_ASSIGN(CCurrentDirManager);
protected:
	std::filesystem::path _prevDir;
public:
	CCurrentDirManager(const std::wstring& chdirTo) {
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

