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
#include "../../ArchiverCode/ArchiverCAB.h"
#include "../ConfigCAB.h"

//====================================
// CABの設定項目
//====================================
class CConfigDlgCAB : public CDialogImpl<CConfigDlgCAB>,public CMessageFilter,public IConfigDlgBase
{
protected:
	CConfigCAB	m_Config;

	CButton			Radio_CompressType[CAB_COMPRESS_TYPE_ITEM_COUNT];
	CEdit			Edit_LZX_Level;
	CTrackBarCtrl	Track_LZX_Level;

	virtual BOOL PreTranslateMessage(MSG* pMsg){
		return IsDialogMessage(pMsg);
	}

public:
	enum { IDD = IDD_PROPPAGE_CONFIG_CAB };

	// メッセージマップ
	BEGIN_MSG_MAP_EX(CConfigDlgCAB)
		MSG_WM_INITDIALOG(OnInitDialog)
		MSG_WM_HSCROLL(OnHScroll)
		COMMAND_RANGE_HANDLER(IDC_RADIO_CAB_MSZIP,IDC_RADIO_CAB_PLAIN, OnRadioCompressType)
		MSG_WM_DESTROY(OnDestroy)
	END_MSG_MAP()

	LRESULT OnInitDialog(HWND hWnd, LPARAM lParam);
	LRESULT OnApply();
	void OnHScroll(int, short, HWND);
	LRESULT OnRadioCompressType(WORD,WORD,HWND,BOOL&);

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

