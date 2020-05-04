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

//標準の設定ファイルのパスを取得
//bUserCommonはユーザー間で共通設定を使う場合にtrueが代入される
//lpszDirはApplicationDataに入れるときに必要なディレクトリ名
//lpszFileは探すファイル名
void GetDefaultFilePath(CString &strPath, LPCTSTR lpszDir, LPCTSTR lpszFile, bool &bUserCommon)
{
	//---ユーザー間で共通の設定を用いる
	//LhaForgeフォルダと同じ場所にINIがあれば使用する
	{
		TCHAR szCommonIniPath[_MAX_PATH + 1] = { 0 };
		_tcsncpy_s(szCommonIniPath, UtilGetModuleDirectoryPath(), _MAX_PATH);
		PathAppend(szCommonIniPath, lpszFile);
		if (PathFileExists(szCommonIniPath)) {
			//共通設定
			bUserCommon = true;
			strPath = szCommonIniPath;
			TRACE(_T("Common INI(Old Style) '%s' found.\n"), strPath);
			return;
		}
	}
	//CSIDL_COMMON_APPDATAにINIがあれば使用する
	{
		TCHAR szCommonIniPath[_MAX_PATH + 1] = { 0 };
		SHGetFolderPath(NULL, CSIDL_COMMON_APPDATA | CSIDL_FLAG_CREATE, NULL, SHGFP_TYPE_CURRENT, szCommonIniPath);
		PathAppend(szCommonIniPath, lpszDir);
		PathAppend(szCommonIniPath, lpszFile);
		if (PathFileExists(szCommonIniPath)) {
			//共通設定
			bUserCommon = true;
			strPath = szCommonIniPath;
			TRACE(_T("Common INI '%s' found.\n"), strPath);
			return;
		}
	}

	//--------------------

	//---ユーザー別設定を用いる
	//LhaForgeインストールフォルダ以下にファイルが存在する場合、それを使用
	{
		//ユーザー名取得
		TCHAR UserName[UNLEN + 1] = { 0 };
		DWORD Length = UNLEN;
		GetUserName(UserName, &Length);

		TCHAR szIniPath[_MAX_PATH + 1];
		_tcsncpy_s(szIniPath, UtilGetModuleDirectoryPath(), _MAX_PATH);
		PathAppend(szIniPath, UserName);
		PathAddBackslash(szIniPath);
		//MakeSureDirectoryPathExists(szIniPath);

		PathAppend(szIniPath, lpszFile);

		if (PathFileExists(szIniPath)) {
			bUserCommon = false;
			strPath = szIniPath;
			TRACE(_T("Personal INI(Old Style) '%s' found.\n"), strPath);
			return;
		}
	}
	//---デフォルト
	//CSIDL_APPDATAにINIがあれば使用する:Vistaではこれ以外はアクセス権限不足になる可能性がある
	TCHAR szIniPath[_MAX_PATH + 1] = { 0 };
	SHGetFolderPath(NULL, CSIDL_APPDATA | CSIDL_FLAG_CREATE, NULL, SHGFP_TYPE_CURRENT, szIniPath);
	PathAppend(szIniPath, lpszDir);
	PathAddBackslash(szIniPath);
	UtilMakeSureDirectoryPathExists(szIniPath);
	PathAppend(szIniPath, lpszFile);
	bUserCommon = false;
	strPath = szIniPath;
	TRACE(_T("Personal INI '%s' found.\n"), strPath);
	return;
}



CConfigManager::CConfigManager()
{
	TRACE(_T("CConfigManager()\n"));

	//---LhaForge.iniの場所
	GetDefaultFilePath(m_strIniPath,PROGRAMDIR_NAME,INI_FILE_NAME,m_bUserCommon);
}

CConfigManager::~CConfigManager()
{
}


void CConfigManager::SetConfigFile(LPCTSTR lpszFile)
{
	if(!lpszFile){
		//NULLを渡されたらデフォルト設定に
		GetDefaultFilePath(m_strIniPath,PROGRAMDIR_NAME,INI_FILE_NAME,m_bUserCommon);
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

bool CConfigManager::HasSection(LPCTSTR lpszSection)const
{
	CONFIG_DICT::const_iterator ite=m_Config.find(lpszSection);
	return (ite!=m_Config.end());
}

void CConfigManager::DeleteSection(LPCTSTR lpszSection)
{
	m_Config.erase(lpszSection);
}
