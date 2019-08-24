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
