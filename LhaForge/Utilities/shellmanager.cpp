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

//---IA32
//Context menu
// {713B479F-6F2B-48e9-B545-5591CCFE398F}
static const GUID CLSID_ShellExtShellMenu32 = 
{ 0x713b479f, 0x6f2b, 0x48e9, { 0xb5, 0x45, 0x55, 0x91, 0xcc, 0xfe, 0x39, 0x8f } };

//Drag menu
// {5E5B692B-D6ED-4103-A1FA-9A71A93DAC88}
static const GUID CLSID_ShellExtDragMenu32 = 
{ 0x5e5b692b, 0xd6ed, 0x4103, { 0xa1, 0xfa, 0x9a, 0x71, 0xa9, 0x3d, 0xac, 0x88 } };

//---AMD64
//Context menu
// {B7584D74-DE0C-4db5-80DD-42EEEDF42665}
static const GUID CLSID_ShellExtShellMenu64 = 
{ 0xb7584d74, 0xde0c, 0x4db5, { 0x80, 0xdd, 0x42, 0xee, 0xed, 0xf4, 0x26, 0x65 } };

//Drag menu
// {00521ADB-148D-45c9-8021-7446EE35609D}
static const GUID CLSID_ShellExtDragMenu64 = 
{ 0x521adb, 0x148d, 0x45c9, { 0x80, 0x21, 0x74, 0x46, 0xee, 0x35, 0x60, 0x9d } };

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
	BOOL iswow64 = FALSE;
	IsWow64Process(GetCurrentProcess(), &iswow64);
	if(iswow64)flag|=KEY_WOW64_64KEY;
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
	return (
		IsShellExtensionRegistered(CLSID_ShellExtShellMenu32) ||
		IsShellExtensionRegistered(CLSID_ShellExtShellMenu64)
		);
}
