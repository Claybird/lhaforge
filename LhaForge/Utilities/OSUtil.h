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
	const wchar_t* lpszPathLink,
	const wchar_t* lpszPathTarget,
	const wchar_t* lpszArgs,
	const wchar_t* lpszIconPath,
	int iIcon,
	LPCTSTR lpszDescription);

struct UTIL_SHORTCUTINFO {
	std::wstring title;
	std::wstring cmd;
	std::wstring param;
	std::wstring workingDir;
	CBitmap cIconBmpSmall;
};

HRESULT UtilGetShortcutInfo(const wchar_t* lpPath, UTIL_SHORTCUTINFO& info);

enum LFPROCESS_PRIORITY {
	LFPRIOTITY_DEFAULT = 0,
	LFPRIOTITY_LOW = 1,
	LFPRIOTITY_LOWER = 2,
	LFPRIOTITY_NORMAL = 3,
	LFPRIOTITY_HIGHER = 4,
	LFPRIOTITY_HIGH = 5,
	LFPRIOTITY_MAX_NUM = LFPRIOTITY_HIGH,
};

//特定のフォルダをExplorerで開く
void UtilNavigateDirectory(LPCTSTR lpszDir);

//環境変数を参照し、辞書形式で取得する
void UtilGetEnvInfo(std::map<stdString,stdString> &envInfo);

//UtilExpandTemplateString()のパラメータ展開に必要な情報を構築する
void UtilMakeExpandInformation(std::map<stdString,CString> &envInfo);

//アイコンを透明度付きビットマップに変換する
void UtilMakeDIBFromIcon(CBitmap&,HICON);

//プロセス優先度の設定
void UtilSetPriorityClass(DWORD dwPriorityClass);

//ディレクトリ制御
class CCurrentDirManager
{
	DISALLOW_COPY_AND_ASSIGN(CCurrentDirManager);
protected:
	TCHAR _prevDir[_MAX_PATH+1];
public:
	CCurrentDirManager(LPCTSTR lpPath){
		::GetCurrentDirectory(COUNTOF(_prevDir),_prevDir);
		::SetCurrentDirectory(lpPath);
	}
	virtual ~CCurrentDirManager(){
		::SetCurrentDirectory(_prevDir);
	}
};

//クリップボードにテキストを設定
void UtilSetTextOnClipboard(LPCTSTR lpszText);
