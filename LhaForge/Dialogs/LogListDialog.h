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
#include "../resource.h"
#include "../TestArchive.h"

class CLogListDialog:public CDialogImpl<CLogListDialog>,public CDialogResize<CLogListDialog>, public CCustomDraw<CLogListDialog>
{
protected:
	LRESULT OnInitDialog(HWND hWnd, LPARAM lParam);

	//CSplitterWindow	m_SplitterWindow;	// スプリッタウィンドウ
	CListViewCtrl	m_ItemListView;		// ファイル一覧
	CEdit			m_MsgEdit;			// 検査結果のメッセージ表示用
	CEdit			m_PathEdit;			// 検査結果のファイル名表示用

	std::vector<ARCLOG>	m_LogArray;		//検査結果の配列
	bool				m_bAllOK;		//すべてがOKならtrue
	CString				m_strCaption;	//ダイアログのキャプション

	//--------------
	// リストビュー
	//--------------
	LRESULT OnGetDispInfo(LPNMHDR pnmh);	//仮想リストビューのデータ取得
	LRESULT OnItemChanged(LPNMHDR pnmh);	//選択の変更
	LRESULT OnSortItem(LPNMHDR pnmh);		//ソート
	int m_nSortColumn;						//ソートに使った列
	bool m_bSortDescending;					//ソートが昇順ならfalse


	// メッセージマップ
	BEGIN_MSG_MAP_EX(CLogListDialog)
		NOTIFY_CODE_HANDLER_EX(LVN_GETDISPINFO, OnGetDispInfo)
		MSG_WM_INITDIALOG(OnInitDialog)
//		COMMAND_ID_HANDLER_EX(IDOK, OnEnd)
		COMMAND_ID_HANDLER_EX(IDCANCEL, OnEnd)
		NOTIFY_CODE_HANDLER_EX(LVN_ITEMCHANGED, OnItemChanged)
		MSG_WM_CTLCOLORSTATIC(OnCtrlColorEdit)
		NOTIFY_CODE_HANDLER_EX(LVN_COLUMNCLICK, OnSortItem)
		CHAIN_MSG_MAP(CCustomDraw<CLogListDialog>)    // CCustomDrawクラスへチェーン
		CHAIN_MSG_MAP(CDialogResize<CLogListDialog>)    // CDialogResizeクラスへのチェーン
//		REFLECT_NOTIFICATIONS()
	END_MSG_MAP()

	void OnEnd(UINT uNotifyCode, int nID, HWND hWndCtl){
		EndDialog(nID);
	}

	// ダイアログリサイズマップ
	BEGIN_DLGRESIZE_MAP(CLogListDialog)
		DLGRESIZE_CONTROL(IDC_LIST_LOGINFO_ITEMS,DLSZ_SIZE_X | DLSZ_SIZE_Y)
		DLGRESIZE_CONTROL(IDC_STATIC_MOVABLE1,DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_STATIC_MOVABLE2,DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_EDIT_LOGINFO_FILE,DLSZ_SIZE_X|DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_STATIC_LOGINFO,DLSZ_SIZE_X|DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_EDIT_LOGINFO_MSG,DLSZ_MOVE_X|DLSZ_SIZE_X|DLSZ_SIZE_Y)
	END_DLGRESIZE_MAP()

	//カスタムドロー
	DWORD OnPrePaint(int nID, LPNMCUSTOMDRAW);
	DWORD OnItemPrePaint(int nID, LPNMCUSTOMDRAW);

	//コントロールの色:読み取り専用エディット
	HBRUSH OnCtrlColorEdit(HDC,HWND);
public:
	CLogListDialog(LPCTSTR lpszCaption):m_strCaption(lpszCaption),m_nSortColumn(-1),m_bSortDescending(false){}
	enum {IDD = IDD_DIALOG_LOGLIST};

	//ログ情報をセット
	void SetLogArray(const std::vector<ARCLOG>&);
};
