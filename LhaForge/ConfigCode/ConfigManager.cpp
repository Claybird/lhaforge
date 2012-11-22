/*
 * Copyright (c) 2005-2012, Claybird
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

#include "stdafx.h"
#include "ConfigManager.h"
#include "../Utilities/FileOperation.h"
//#include "../const.h"
#include "../resource.h"
#include "../ArchiverCode/arc_interface.h"


CConfigManager::CConfigManager()
{
	TRACE(_T("CConfigManager()\n"));
	//---CaldixのINIファイル名
	TCHAR szIniPath[_MAX_PATH+1]={0};
	_tcsncpy_s(szIniPath,UtilGetModuleDirectoryPath(),_MAX_PATH);
	PathAppend(szIniPath,CALDIX_INI_FILE_NAME);
	if(PathFileExists(szIniPath)){
		//LhaForgeフォルダと同じ場所にINIがあれば使用する
		m_strCaldixIniPath=szIniPath;
	}else{
		//CSIDL_COMMON_APPDATAにINIを用意し、使用する
		SHGetFolderPath(NULL,CSIDL_COMMON_APPDATA|CSIDL_FLAG_CREATE,NULL,SHGFP_TYPE_CURRENT,szIniPath);
		PathAppend(szIniPath,PROGRAMDIR_NAME);
		PathAddBackslash(szIniPath);
		UtilMakeSureDirectoryPathExists(szIniPath);
		PathAppend(szIniPath,CALDIX_INI_FILE_NAME);
		m_strCaldixIniPath=szIniPath;
	}

	//---LhaForge.ini/LFCaldix.iniの場所
	UtilGetDefaultFilePath(m_strIniPath,PROGRAMDIR_NAME,INI_FILE_NAME,m_bUserCommon);
}

CConfigManager::~CConfigManager()
{
}


void CConfigManager::SetConfigFile(LPCTSTR lpszFile)
{
	if(!lpszFile){
		//NULLを渡されたらデフォルト設定に
		UtilGetDefaultFilePath(m_strIniPath,PROGRAMDIR_NAME,INI_FILE_NAME,m_bUserCommon);
		return;
	}
	m_bUserCommon=false;
	UtilGetAbsPathName(m_strIniPath,lpszFile);
	TRACE(_T("Custom Config Path=%s\n"),m_strIniPath);
}

bool CConfigManager::LoadConfig(CString &strErr)
{
	m_Config.clear();
	if(PathFileExists(m_strIniPath)){
		//読み込み
		std::list<CONFIG_SECTION> sections;
		if(!UtilReadSectionedConfig(m_strIniPath,sections,strErr))return false;

		for(std::list<CONFIG_SECTION>::iterator ite=sections.begin();ite!=sections.end();++ite){
			GetSection((*ite).SectionName.c_str())=*ite;
		}

		m_Config.erase(_T("BH"));
		m_Config.erase(_T("YZ1"));
	}

	//caldix
	m_CaldixConfig.clear();
	if(PathFileExists(m_strCaldixIniPath)){
		std::list<CONFIG_SECTION> sections;
		if(!UtilReadSectionedConfig(m_strCaldixIniPath,sections,strErr))return false;

		for(std::list<CONFIG_SECTION>::iterator ite=sections.begin();ite!=sections.end();++ite){
			GetCaldixSection((*ite).SectionName.c_str())=*ite;
		}
	}

	return true;
}

bool CConfigManager::SaveConfig(CString &strErr)
{
	// 自分のバージョンを記述
	GetSection(_T("LhaForge")).Data[_T("Version")]=CString(MAKEINTRESOURCE(IDS_LHAFORGE_VERSION_STRING));

	//保存
	std::list<CONFIG_SECTION> tmpList;
	for(CONFIG_DICT::const_iterator ite=m_Config.begin();ite!=m_Config.end();++ite){
		tmpList.push_back((*ite).second);
	}
	if(!UtilWriteSectionedConfig(m_strIniPath,tmpList,strErr)){
		return false;
	}
	return true;
}

void CConfigManager::WriteUpdateTime(time_t LastTime)
{
	LPCTSTR SectionName=_T("Update");

	//更新キャンセル時など、LFCaldix.iniをLhaForgeが更新する必要があるとき
	//---LFCaldix.iniに記録
	UtilWritePrivateProfileInt(SectionName,_T("LastTime"),LastTime,m_strCaldixIniPath);
	//---ユーザー用ファイルに記録
	UtilWritePrivateProfileInt(SectionName,_T("LastTime"),LastTime,m_strIniPath);
}

CONFIG_SECTION &CConfigManager::GetSection(LPCTSTR lpszSection)
{
	CONFIG_DICT::iterator ite=m_Config.find(lpszSection);
	if(ite!=m_Config.end()){
		return (*ite).second;
	}else{
		CONFIG_SECTION &Conf=m_Config[lpszSection];
		Conf.SectionName=lpszSection;
		return Conf;
	}
}

CONFIG_SECTION &CConfigManager::GetCaldixSection(LPCTSTR lpszSection)
{
	CONFIG_DICT::iterator ite=m_CaldixConfig.find(lpszSection);
	if(ite!=m_CaldixConfig.end()){
		return (*ite).second;
	}else{
		CONFIG_SECTION &Conf=m_CaldixConfig[lpszSection];
		Conf.SectionName=lpszSection;
		return Conf;
	}
}

bool CConfigManager::HasSection(LPCTSTR lpszSection)const
{
	CONFIG_DICT::const_iterator ite=m_Config.find(lpszSection);
	return (ite!=m_Config.end());
}

void CConfigManager::DeleteSection(LPCTSTR lpszSection)
{
	m_Config.erase(lpszSection);
}
