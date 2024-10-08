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

//Show message box
int ErrorMessage(const std::wstring& message);
int UtilMessageBox(HWND hWnd, const std::wstring& message, UINT uType);

std::wstring UtilGetLastErrorMessage(DWORD langID = MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), DWORD errorCode = GetLastError());

//read filelist from response file
std::vector<std::wstring> UtilReadFromResponseFile(const std::filesystem::path& respFile, UTIL_CODEPAGE uSrcCodePage);

//checks if path extension matches specific patterns
//pattern_string may contain only one pattern, such as "*.txt" and/or "*.do?"
bool UtilExtMatchSpec(const std::filesystem::path& path, const std::wstring& pattern_string);
//checks if path matches specific patterns
bool UtilPathMatchSpec(const std::filesystem::path& path, const std::wstring& pattern_string);

//Message loop utility
class CCustomMessageLoop :public CMessageLoop {
public:
	CCustomMessageLoop() {}
	virtual ~CCustomMessageLoop() {}
	int OneRun()
	{
		int nIdleCount = 0;
		BOOL bRet;

		if(!::PeekMessage(&m_msg, NULL, 0, 0, PM_NOREMOVE)){
			OnIdle(nIdleCount++);
			return FALSE;
		}

		if (m_msg.message == WM_QUIT) {
			return FALSE;
		}

		bRet = ::GetMessage(&m_msg, NULL, 0, 0);

		if (bRet == -1) {
			ATLTRACE2(atlTraceUI, 0, _T("::GetMessage returned -1 (error)\n"));
			return FALSE;   // error, don't process
		}/* else if (!bRet) {
			ATLTRACE2(atlTraceUI, 0, _T("CMessageLoop::Run - exiting\n"));
			break;   // WM_QUIT, exit message loop
		}*/

		if (!PreTranslateMessage(&m_msg)) {
			::TranslateMessage(&m_msg);
			::DispatchMessage(&m_msg);
		}

		return TRUE;
	}
};
bool UtilDoMessageLoop();

template <typename mapclass,typename keyclass>
bool has_key(const mapclass &theMap,keyclass theKey){
	return theMap.find(theKey)!=theMap.end();
}

//return index where "theValue" exists in "theArray", if there is. otherwise, return -1
template <typename arrayclass,typename valueclass>
int index_of(const arrayclass &theArray,valueclass theValue){
	for(size_t i=0;i<theArray.size();++i){
		if(theArray[i]==theValue){
			return (signed)i;
		}
	}
	return -1;
}

template <typename T, typename valueclass>
int index_of(const T* ptr, size_t count, valueclass theValue) {
	for (size_t i = 0; i < count; ++i) {
		if (ptr[i] == theValue) {
			return (signed)i;
		}
	}
	return -1;
}

//remove items from container if theValue==ite
template <typename arrayclass,typename valueclass>
void remove_item(arrayclass &theArray,const valueclass &theValue){
	theArray.erase(std::remove(theArray.begin(), theArray.end(), theValue), theArray.end());
}

//remove items from container if cond(ite)==true
template <typename arrayclass, typename COND>
void remove_item_if(arrayclass &theArray, const COND &cond) {
	theArray.erase(std::remove_if(theArray.begin(), theArray.end(), cond), theArray.end());
}

template<typename T, typename U>
bool isIn(const T &collection, U value)
{
	for (const auto &i : collection) {
		if (i == value)return true;
	}
	return false;
}

template<typename T>
void merge_map(T &dest, const T &src) {	//merge, overwriting
	for (const auto &item : src) {
		dest[item.first] = item.second;
	}
	//dest.merge will modify src and will not overwrite existing item
}

FILETIME UtilUnixTimeToFileTime(__time64_t t);
__time64_t UtilFileTimeToUnixTime(FILETIME ft);
