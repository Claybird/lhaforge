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
#include "../ConfigManager.h"
#include "../../resource.h"
#include "../../ArchiverCode/ArchiverJACK.h"
#include "../ConfigJACK.h"

//====================================
// JACKの設定項目
//====================================
class CConfigDlgJACK : public CDialogImpl<CConfigDlgJACK>,public CWinDataExchange<CConfigDlgJACK>,public CMessageFilter,public IConfigDlgBase
{
protected:
	CConfigJACK	m_Config;

	CButton			Radio_SpecifyVolumeSize[2];
	int				VolumeSize;
	virtual BOOL PreTranslateMessage(MSG* pMsg){
		return IsDialogMessage(pMsg);
	}


public:
	enum { IDD = IDD_PROPPAGE_CONFIG_JACK };
	// DDXマップ
	BEGIN_DDX_MAP(CConfigDlgJACK)
		DDX_INT(IDC_EDIT_JACK_VOLUME_SIZE, VolumeSize)
	END_DDX_MAP()

	// メッセージマップ
	BEGIN_MSG_MAP_EX(CConfigDlgJACK)
		MSG_WM_INITDIALOG(OnInitDialog)
		COMMAND_RANGE_HANDLER(IDC_RADIO_JACK_VOLUME_SIZE_USE_STORED_SETTING,IDC_RADIO_JACK_VOLUME_SIZE_SPECIFY_AT_COMPRESS, OnRadioVolumeSize)
		MSG_WM_DESTROY(OnDestroy)
	END_MSG_MAP()

	LRESULT OnInitDialog(HWND hWnd, LPARAM lParam);
	LRESULT OnApply();
	LRESULT OnRadioVolumeSize(WORD,WORD,HWND,BOOL&);

	LRESULT OnDestroy(){
		CMessageLoop* pLoop = _Module.GetMessageLoop();
		pLoop->RemoveMessageFilter(this);

		return TRUE;
	}

	void LoadConfig(CConfigManager& Config){
		m_Config.load(Config);
	}
	void StoreConfig(CConfigManager& Config){
		m_Config.store(Config);
	}
};

