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
#include "ShellDataManager.h"
#include "FileListWindow/ArcFileContent.h"

const LF_SHELLDATA& CLFShellDataManager::makeSureDataRegistered(
	const wchar_t* extension,
	DWORD Attribute)
{
	const wchar_t* FOLDER_EXTENSION_STRING = L"***";
	if (!extension)extension = FOLDER_EXTENSION_STRING;

	auto ite = _shellDataMap.find(extension);
	if (ite == _shellDataMap.end()) {
		//if not found, register data
		LF_SHELLDATA ShellData;
		//dummy extension for folder

		//file icon index
		SHFILEINFO shfi;
		SHGetFileInfoW(extension, Attribute, &shfi, sizeof(shfi), SHGFI_USEFILEATTRIBUTES | SHGFI_ICON | SHGFI_LARGEICON | SHGFI_SYSICONINDEX);
		ShellData.IconIndex = shfi.iIcon;

		//filetype name
		SHGetFileInfoW(extension, Attribute, &shfi, sizeof(shfi), SHGFI_USEFILEATTRIBUTES | SHGFI_TYPENAME);
		ShellData.TypeName = shfi.szTypeName;

		auto result = _shellDataMap.insert(std::make_pair(extension, ShellData));
		return (*result.first).second;
	} else {
		return (*ite).second;
	}
}

void CLFShellDataManager::Init()
{
	SHFILEINFO shfi;
	_imageListLarge = (HIMAGELIST)SHGetFileInfo(L"", 0, &shfi, sizeof(shfi), SHGFI_SYSICONINDEX | SHGFI_ICON | SHGFI_LARGEICON);
	_imageListSmall = (HIMAGELIST)SHGetFileInfo(L"", 0, &shfi, sizeof(shfi), SHGFI_SYSICONINDEX | SHGFI_ICON | SHGFI_SMALLICON);

	//register for folder
	makeSureDataRegistered(nullptr, FILE_ATTRIBUTE_DIRECTORY);
}
