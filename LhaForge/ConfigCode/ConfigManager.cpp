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
