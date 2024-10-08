﻿/*
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
	const std::wstring &description);

struct UTIL_SHORTCUTINFO {
	std::wstring title;
	std::filesystem::path cmd;
	std::wstring param;
	std::filesystem::path workingDir;
	CBitmap cIconBmpSmall;
};

HRESULT UtilGetShortcutInfo(const std::filesystem::path& path, UTIL_SHORTCUTINFO& info);

//Open a folder with explorer
void UtilNavigateDirectory(const std::filesystem::path& path);

//retrieve environment variables as key=value pair
std::map<std::wstring, std::wstring> UtilGetEnvInfo();

std::wstring UtilGetWindowClassName(HWND hWnd);

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
	CCurrentDirManager(const std::filesystem::path& chdirTo);
	virtual ~CCurrentDirManager() noexcept(false);
};

