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
#include "Dlg_Base.h"
#include "resource.h"
#include "Utilities/CustomControl.h"
#include "ConfigCode/ConfigCompressFormat.h"


class CConfigDlgFormat : public LFConfigDialogBase<CConfigDlgFormat>
{
protected:
	CLFComboListViewCtrl _listView;
	std::vector<CConfigCompressFormatBase*> _configs;
	std::vector<CLFComboListViewCtrl::CONTENT_DATA> _data;
	void clearData() {
		for (auto& d : _data) {
			if (d.userData) {
				delete (USER_DATA*)d.userData;
			}
		}
		_data.clear();
	}
	struct USER_DATA{
		CConfigCompressFormatBase* c;	//item in _configs
		std::string key;
	};
public:
	CConfigDlgFormat() {}
	virtual ~CConfigDlgFormat() {
		for (auto& c : _configs) {
			delete c;
		}
		_configs.clear();

		clearData();
	}
	template<typename T>
	void AddConfig() {	//call this before OnInitDialog()
		_configs.push_back(new T);
	}

	enum { IDD = IDD_PROPPAGE_CONFIG_FORMAT };

	BEGIN_MSG_MAP_EX(CConfigDlgFormat)
		MSG_WM_INITDIALOG(OnInitDialog)
		REFLECT_NOTIFICATIONS_EX()	//reflect to CLFComboListViewCtrl
	END_MSG_MAP()

	LRESULT OnInitDialog(HWND hWnd, LPARAM lParam);
	LRESULT OnApply(){return TRUE;}

	void LoadConfig(CConfigFile& Config){
		for (auto& c : _configs) {
			c->load(Config);
		}
	}
	void StoreConfig(CConfigFile& Config, CConfigFile& assistant) {
		_data = _listView.GetContentData();
		for (auto& d : _data) {
			if (d.userData) {
				auto p = (USER_DATA*)d.userData;
				const auto& k_v = p->c->key_and_valid_values;
				int curSel = d.selection;
				auto ite = k_v.find(UtilUTF8toUNICODE(p->key));

				if (ite != k_v.end()) {
					const auto& raw_options = (*ite).second;
					p->c->params[p->key] = UtilToUTF8(raw_options[curSel]);
				}
			}
		}

		for (auto& c : _configs) {
			c->store(Config);
		}
	}
};

