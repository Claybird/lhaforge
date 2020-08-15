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
#include "../Utilities/Utility.h"
#include "../FileListWindow/MenuCommand.h"
#include "../Utilities/ConfigFile.h"

//----------
// 設定管理
//----------
//INIファイル名
const LPCTSTR INI_FILE_NAME=_T("LhaForge.ini");
const LPCTSTR PROGRAMDIR_NAME=_T("LhaForge");	//ApplicationDataに入れるときに必要なディレクトリ名


class CConfigManager;
struct IConfigConverter{
protected:
	virtual void load(CONFIG_SECTION&)=0;	//設定をCONFIG_SECTIONから読み込む
	virtual void store(CONFIG_SECTION&)const=0;	//設定をCONFIG_SECTIONに書き込む
public:
	virtual ~IConfigConverter(){}
	virtual void load(CConfigManager&)=0;
	virtual void store(CConfigManager&)const=0;
};

//====================================
// 設定管理クラス
//====================================

class CConfigManager
{
protected:
	CString m_strIniPath;
	bool m_bUserCommon;	//ユーザー間で共通の設定を使う場合はtrue;表示で使うためだけに用意
	typedef std::map<stdString,CONFIG_SECTION,less_ignorecase> CONFIG_DICT;
	CONFIG_DICT m_Config;
public:
	CConfigManager();
	virtual ~CConfigManager();
	void SetConfigFile(LPCTSTR);	//設定ファイルを指定
	bool LoadConfig(CString &strErr);
	bool SaveConfig(CString &strErr);
	CONFIG_SECTION &GetSection(LPCTSTR lpszSection);
	void DeleteSection(LPCTSTR lpszSection);
	bool HasSection(LPCTSTR lpszSection)const;
	bool IsUserCommon(){return m_bUserCommon;}	//ユーザー間共通設定？
};

