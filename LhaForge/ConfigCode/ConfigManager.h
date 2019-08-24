/*
 * Copyright (c) 2005-, Claybird
 * All rights reserved.

 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:

 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Claybird nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.

 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
 * THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
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
const LPCTSTR CALDIX_INI_FILE_NAME=_T("LFCaldix.ini");
const LPCTSTR PROGRAMDIR_NAME=_T("LhaForge");	//ApplicationDataに入れるときに必要なディレクトリ名


enum PARAMETER_TYPE;

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
	CString m_strCaldixIniPath;
	bool m_bUserCommon;	//ユーザー間で共通の設定を使う場合はtrue;表示で使うためだけに用意
	typedef std::map<stdString,CONFIG_SECTION,less_ignorecase> CONFIG_DICT;
	CONFIG_DICT m_Config;
	CONFIG_DICT m_CaldixConfig;
public:
	CConfigManager();
	virtual ~CConfigManager();
	void SetConfigFile(LPCTSTR);	//設定ファイルを指定
	bool LoadConfig(CString &strErr);
	bool SaveConfig(CString &strErr);
	CONFIG_SECTION &GetSection(LPCTSTR lpszSection);
	CONFIG_SECTION &GetCaldixSection(LPCTSTR lpszSection);
	void DeleteSection(LPCTSTR lpszSection);
	bool HasSection(LPCTSTR lpszSection)const;
	void WriteUpdateTime(time_t=time(NULL));
	bool IsUserCommon(){return m_bUserCommon;}	//ユーザー間共通設定？
};

