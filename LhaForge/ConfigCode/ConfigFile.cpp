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
#include "ConfigFile.h"
#include "Utilities/FileOperation.h"
#include "resource.h"

void CConfigFile::setDefaultPath()
{
	const wchar_t* INI_FILE_NAME = L"LhaForge.ini";
	const wchar_t* PROGRAMDIR_NAME = L"LhaForge";	//directory name in ApplicationData

	//user common configuration
	{
		//.ini file is in same as the executable; for portable usage
		auto candidate = std::filesystem::path(UtilGetModuleDirectoryPath()) / INI_FILE_NAME;
		if (std::filesystem::is_regular_file(candidate)) {
			m_bUserCommon = true;
			m_iniPath = candidate;
			return;
		}
	}
	{
		//.ini is in FOLDERID_ProgramData (formerly CSIDL_COMMON_APPDATA)
		wchar_t* ptr = nullptr;
		if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_ProgramData, 0, nullptr, &ptr))) {
			auto candidate = std::filesystem::path(ptr) / PROGRAMDIR_NAME / INI_FILE_NAME;
			CoTaskMemFree(ptr);
			if (std::filesystem::is_regular_file(candidate)) {
				m_bUserCommon = true;
				m_iniPath = candidate;
				return;
			}
		}
	}

	//--------------------
	//user specific configuration
	{
		//.ini is in FOLDERID_RoamingAppData (formerly CSIDL_APPDATA)
		wchar_t* ptr = nullptr;
		if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, nullptr, &ptr))) {
			auto candidate = std::filesystem::path(ptr) / PROGRAMDIR_NAME / INI_FILE_NAME;
			CoTaskMemFree(ptr);
			m_bUserCommon = false;
			m_iniPath = candidate;
			return;
		}
	}

	//default fallback
	m_iniPath = UtilGetTemporaryFileName();
	m_bUserCommon = true;
}

void CConfigFile::setPath(const std::wstring& path)
{
	m_bUserCommon=false;
	try {
		m_iniPath = UtilGetCompletePathName(path);
	} catch (const LF_EXCEPTION&) {
		m_iniPath = path;
	}
}

void CConfigFile::load()
{
	if (std::filesystem::is_regular_file(m_iniPath)) {
		auto rc = m_Config.LoadFile(m_iniPath.c_str());
		if (rc < 0) {
			//TODO: resource
			RAISE_EXCEPTION(L"Failed to load config file %s", m_iniPath.c_str());
		}
	} else {
		m_Config.Reset();
	}
}

void CConfigFile::save()
{
	//version
	m_Config.SetValue(L"lhaforge", L"version", UtilLoadString(IDS_LHAFORGE_VERSION_STRING).c_str());

	//save
	auto rc = m_Config.SaveFile(m_iniPath.c_str(), false);
	if (rc < 0) {
		//TODO: resource
		RAISE_EXCEPTION(L"Failed to save config file %s", m_iniPath.c_str());
	}
}
