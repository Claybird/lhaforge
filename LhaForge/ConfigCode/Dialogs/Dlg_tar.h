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
#include "../../ArchiverCode/ArchiverTAR.h"
#include "../ConfigTAR.h"
#include "../../resource.h"

//====================================
// TARの設定項目
//====================================
class CConfigDlgTAR : public CDialogImpl<CConfigDlgTAR>,public CMessageFilter,public IConfigDlgBase
{
protected:
	CConfigTAR	m_Config;

	CEdit			Edit_GZIP_Level;
	CTrackBarCtrl	Track_GZIP_Level;

	CEdit			Edit_BZIP2_Level;
	CTrackBarCtrl	Track_BZIP2_Level;

	CEdit			Edit_XZ_Level;
	CTrackBarCtrl	Track_XZ_Level;

	CEdit			Edit_LZMA_Level;
	CTrackBarCtrl	Track_LZMA_Level;

	CButton	Check_ConvertCharset;
	CButton	Radio_SortBy[TAR_SORT_BY_ITEM_COUNT];

	virtual BOOL PreTranslateMessage(MSG* pMsg){
		return IsDialogMessage(pMsg);
	}
public:
	enum { IDD = IDD_PROPPAGE_CONFIG_TAR };

	// メッセージマップ
	BEGIN_MSG_MAP_EX(CConfigDlgTAR)
		MSG_WM_INITDIALOG(OnInitDialog)
		MSG_WM_HSCROLL(OnHScroll)
		MSG_WM_DESTROY(OnDestroy)
	END_MSG_MAP()

	LRESULT OnInitDialog(HWND hWnd, LPARAM lParam);
	LRESULT OnApply();
	void OnHScroll(int, short, HWND);

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

