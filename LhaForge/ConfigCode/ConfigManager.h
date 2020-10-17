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

struct CConfigManager
{
	std::filesystem::path m_iniPath;
	bool m_bUserCommon;	//true to use user common configuration
	CSimpleIniW m_Config;

	CConfigManager() {
		m_Config.SetUnicode(true);
		setDefaultPath();
	}
	virtual ~CConfigManager() {}
	void setDefaultPath();
	void setPath(const std::wstring& path);
	void load();
	void save();
	void deleteSection(const std::wstring& section) {
		m_Config.Delete(section.c_str(), nullptr);
	}
	bool hasSection(const std::wstring& section)const {
		return nullptr != m_Config.GetSection(section.c_str());
	}
	bool isUserCommon()const { return m_bUserCommon; }

	bool getBool(const std::wstring& section, const std::wstring& key, bool defaultValue)const {
		return m_Config.GetBoolValue(section.c_str(), key.c_str(), defaultValue);
	}
	int getInt(const std::wstring& section, const std::wstring& key, int defaultValue)const {
		return m_Config.GetLongValue(section.c_str(), key.c_str(), defaultValue);
	}
	int getIntRange(const std::wstring& section, const std::wstring& key, int nMin, int nMax, int defaultValue)const {
		auto value = getInt(section, key, defaultValue);
		return std::max(std::min(value, nMax), nMin);
	}
	double getDouble(const std::wstring& section, const std::wstring& key, double defaultValue)const {
		return m_Config.GetDoubleValue(section.c_str(), key.c_str(), defaultValue);
	}
	std::wstring getText(const std::wstring& section, const std::wstring& key, const std::wstring& defaultValue)const {
		return m_Config.GetValue(section.c_str(), key.c_str(), defaultValue.c_str());
	}

	void setValue(const std::wstring& section, const std::wstring& key, bool value) {
		m_Config.SetBoolValue(section.c_str(), key.c_str(), value);
	}
	void setValue(const std::wstring& section, const std::wstring& key, int value) {
		m_Config.SetLongValue(section.c_str(), key.c_str(), value);
	}
	void setValue(const std::wstring& section, const std::wstring& key, double value) {
		m_Config.SetDoubleValue(section.c_str(), key.c_str(), value);
	}
	void setValue(const std::wstring& section, const std::wstring& key, const std::wstring& value) {
		m_Config.SetValue(section.c_str(), key.c_str(), value.c_str());
	}
	void setValue(const std::wstring& section, const std::wstring& key, const wchar_t* value) {
		m_Config.SetValue(section.c_str(), key.c_str(), value);
	}
};

struct IConfigIO {
	virtual ~IConfigIO() {}
	virtual void load(const CConfigManager&) = 0;
	virtual void store(CConfigManager&)const = 0;
};

