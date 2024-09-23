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
#include "shellmanager.h"
#include "Utilities/Utility.h"
#include "Utilities/OSUtil.h"
#include "resource.h"


const int CLSID_STRING_SIZE=(39 + 1);

//right click context menu
// {9127BC06-94AC-46F7-99DB-3D4423B8E813}
const GUID CLSID_LFContextMenu =
{ 0x9127bc06, 0x94ac, 0x46f7, { 0x99, 0xdb, 0x3d, 0x44, 0x23, 0xb8, 0xe8, 0x13 } };

//right drag menu
// {8CE5DDDA-DDAF-476F-86FD-505DA9B94967}
const GUID CLSID_LFDragMenu =
{ 0x8ce5ddda, 0xddaf, 0x476f, { 0x86, 0xfd, 0x50, 0x5d, 0xa9, 0xb9, 0x49, 0x67 } };

std::wstring CLSIDtoSTRING(REFCLSID inClsid)
{
	WCHAR buf[CLSID_STRING_SIZE];
	StringFromGUID2(inClsid, buf, CLSID_STRING_SIZE);
	return buf;
}

bool IsShellExtensionRegistered(const GUID inGUID)
{
	auto clsid = CLSIDtoSTRING(inGUID);

	auto key = L"CLSID\\" + clsid;

	// check for registory
	HKEY hChildKey;
	int flag = KEY_READ;
	if (ERROR_SUCCESS != ::RegOpenKeyExW(HKEY_CLASSES_ROOT, key.c_str(), 0, flag, &hChildKey)) {
		return false;
	}

	//is data associated to key?
	DWORD dwLength=0;
	::RegQueryValueExW(hChildKey, nullptr, nullptr, nullptr, nullptr, &dwLength);
	bool Result = false;

	if(dwLength>1){
		for(int idx=0;;idx++){
			constexpr int bufsize = 256;
			wchar_t buffer[bufsize];
			DWORD actualSize = bufsize;

			if (S_OK == ::RegEnumKeyExW(hChildKey, idx, buffer, &actualSize, nullptr, nullptr, nullptr, nullptr)) {
				if (_wcsicmp(buffer, L"InprocServer32") == 0) {
					Result = true;
					break;
				}
			} else {
				break;
			}
		}
	}

	// Close the child.
	::RegCloseKey(hChildKey);

	return Result;
}


bool ShellRegistCheck()
{
	return IsShellExtensionRegistered(CLSID_LFContextMenu);
}

