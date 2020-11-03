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

class LF_SHELLDATA{
public:
	int IconIndex;
	std::wstring TypeName;
};

class CLFShellDataManager
{
protected:
	CImageList _imageListSmall;
	CImageList _imageListLarge;
	std::unordered_map<std::wstring, LF_SHELLDATA> _shellDataMap;
	const LF_SHELLDATA& makeSureDataRegistered(const wchar_t* extension, DWORD Attribute = FILE_ATTRIBUTE_NORMAL);
public:
	virtual ~CLFShellDataManager(){}
	void Init();
	HIMAGELIST GetImageList(bool bLarge) {
		if (bLarge) {
			return _imageListLarge;
		} else {
			return _imageListSmall;
		}
	}
	int GetIconIndex(const wchar_t* extension) {
		return makeSureDataRegistered(extension).IconIndex;
	}
	const std::wstring GetTypeName(const wchar_t* extension){
		return makeSureDataRegistered(extension).TypeName;
	}
};
