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

#include "Association.h"
#include "Utilities/OSUtil.h"

const wchar_t* DEFAULT_ICON_FILENAME = L"icons\\archive.ico";
const wchar_t* ASSOC_PREFIX = L"LhaForge2Archive_";

bool getDefaultRegKeyValue(const std::wstring& key, std::wstring& value)
{
	CRegKey hKey;
	auto Ret = hKey.Open(HKEY_CLASSES_ROOT, key.c_str(), KEY_READ);
	if (ERROR_SUCCESS != Ret) {
		return false;
	}

	DWORD dwSize = 0;
	hKey.QueryStringValue(nullptr, nullptr, &dwSize);
	value.resize(dwSize + 1, L'\0');
	Ret = hKey.QueryStringValue(nullptr, &value[0], &dwSize);
	if (ERROR_SUCCESS != Ret) {
		return false;
	}
	value = value.c_str();
	return true;
}

bool AssocGetAssociation(const std::wstring &ext, ASSOCINFO& AssocInfo)
{
	if(ext.empty()){
		return false;
	}
	AssocInfo.Ext = toLower(ext);
	std::wstring fileType;
	if (!getDefaultRegKeyValue(AssocInfo.Ext, fileType)) {
		return false;
	}
	{
		if (0 == toLower(fileType).find(toLower(ASSOC_PREFIX))) {
			AssocInfo.isAssociated = true;
		} else {
			AssocInfo.isAssociated = false;
		}
	}

	//---icons
	{
		std::wstring iconDefs;
		if (!getDefaultRegKeyValue(fileType + L"\\DefaultIcon", iconDefs)) {
			return false;
		}

		auto path_and_index = UtilPathParseIconLocation(iconDefs);
		AssocInfo.IconFile = path_and_index.first.c_str();
		AssocInfo.IconIndex = path_and_index.second;

		AssocInfo.prevIconFile = AssocInfo.IconFile;
		AssocInfo.prevIconIndex = AssocInfo.IconIndex;
	}

	//---commands for Shell\Open
	if (AssocInfo.isAssociated) {
		if (!getDefaultRegKeyValue(fileType + L"\\shell\\open\\command", AssocInfo.ShellOpenCommand)) {
			return false;
		}
	}

	return true;
}
