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
#include "resource.h"
#include "ArchiverCode/archive.h"
#include "Utilities/OSUtil.h"

class CLogListDialog:public CDialogImpl<CLogListDialog>,public CDialogResize<CLogListDialog>, public CCustomDraw<CLogListDialog>
{
protected:
	CListViewCtrl	m_ItemListView;
	CEdit			m_MsgEdit;
	CEdit			m_PathEdit;

	std::vector<ARCLOG>	m_LogArray;
	std::wstring m_strCaption;

	bool m_bAllOK;
	int m_nSortColumn;
	bool m_bSortDescending;
public:
	CLogListDialog(LPCTSTR lpszCaption) :
		m_strCaption(lpszCaption),
		m_nSortColumn(-1),
		m_bSortDescending(false),
		m_bAllOK(true) {}
	enum { IDD = IDD_DIALOG_LOGLIST };
	enum {
		COLUMN_FILENAME = 0,
		COLUMN_RESULTS = 1,
	};

	void SetLogArray(const std::vector<ARCLOG>& rLog) {
		m_LogArray = rLog;

		m_bAllOK = true;
		for (const auto &log : m_LogArray) {
			if (log._overallResult != LF_RESULT::OK) {
				m_bAllOK = false;
				break;
			}
		}
	}
protected:
	BEGIN_MSG_MAP_EX(CLogListDialog)
		NOTIFY_CODE_HANDLER_EX(LVN_GETDISPINFO, OnGetDispInfo)
		MSG_WM_INITDIALOG(OnInitDialog)
		COMMAND_ID_HANDLER_EX(IDCANCEL, OnEnd)
		NOTIFY_CODE_HANDLER_EX(LVN_ITEMCHANGED, OnItemChanged)
		MSG_WM_CTLCOLORSTATIC(OnCtrlColorEdit)
		NOTIFY_CODE_HANDLER_EX(LVN_COLUMNCLICK, OnSortItem)
		CHAIN_MSG_MAP(CCustomDraw<CLogListDialog>)
		CHAIN_MSG_MAP(CDialogResize<CLogListDialog>)
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(CLogListDialog)
		DLGRESIZE_CONTROL(IDC_LIST_LOGINFO_ITEMS, DLSZ_SIZE_X | DLSZ_SIZE_Y)
		DLGRESIZE_CONTROL(IDC_STATIC_MOVABLE1, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_STATIC_MOVABLE2, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_EDIT_LOGINFO_FILE, DLSZ_SIZE_X | DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_STATIC_LOGINFO, DLSZ_SIZE_X | DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_EDIT_LOGINFO_MSG, DLSZ_MOVE_X | DLSZ_SIZE_X | DLSZ_SIZE_Y)
	END_DLGRESIZE_MAP()

protected:
	LRESULT OnInitDialog(HWND hWnd, LPARAM lParam) {
		CenterWindow();
		//---initialize controls
		m_MsgEdit = GetDlgItem(IDC_EDIT_LOGINFO_MSG);
		m_PathEdit = GetDlgItem(IDC_EDIT_LOGINFO_FILE);

		//set icons
		// large
		HICON hIcon = AtlLoadIconImage(IDI_APP, LR_DEFAULTCOLOR, ::GetSystemMetrics(SM_CXICON), ::GetSystemMetrics(SM_CYICON));
		SetIcon(hIcon, TRUE);
		// small
		HICON hIconSmall = AtlLoadIconImage(IDI_APP, LR_DEFAULTCOLOR, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON));
		SetIcon(hIconSmall, FALSE);

		//caption
		SetWindowText((m_strCaption + (m_bAllOK ? L"[All Clear]" : L"[NG]")).c_str());

		//display processing mode
		CStatic StaticInfo = GetDlgItem(IDC_STATIC_LOGINFO);
		StaticInfo.SetWindowText(UtilLoadString(IDS_LOGINFO_OPERATION_TESTARCHIVE).c_str());

		//------------------------

		//---initialize listview
		m_ItemListView = GetDlgItem(IDC_LIST_LOGINFO_ITEMS);
		m_ItemListView.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_HEADERDRAGDROP);
		//set columns
		// filename
		m_ItemListView.InsertColumn(COLUMN_FILENAME, UtilLoadString(IDS_FILELIST_COLUMN_FILENAME).c_str(), LVCFMT_LEFT, 160, -1);
		// results(OK/NG)
		CRect rc;
		m_ItemListView.GetClientRect(rc);
		m_ItemListView.InsertColumn(COLUMN_RESULTS, UtilLoadString(IDS_LOGINFO_RESULT).c_str(), LVCFMT_LEFT, rc.Width() - 160 - 15, -1);

		// report mode
		DWORD Style = m_ItemListView.GetWindowLong(GWL_STYLE);
		Style &= ~(LVS_ICON | LVS_REPORT | LVS_SMALLICON | LVS_LIST);
		m_ItemListView.SetWindowLong(GWL_STYLE, Style | LVS_REPORT);

		// set number of items
		m_ItemListView.SetItemCount(m_LogArray.size());
		if (!m_LogArray.empty()) {
			m_ItemListView.SetItemState(0, LVNI_SELECTED, LVNI_SELECTED);
		}

		//---focus to listview
		m_ItemListView.SetFocus();

		//---enable dialog resize
		DlgResize_Init(true, true, WS_THICKFRAME | WS_CLIPCHILDREN);
		//---activate window
		SetForegroundWindow(hWnd);

		return TRUE;
	}

	//--------------------------
	// list view implementation
	LRESULT OnGetDispInfo(LPNMHDR pnmh) {
		// compose data for list view
		LV_DISPINFO* pstLVDInfo = (LV_DISPINFO*)pnmh;

		if (pstLVDInfo->item.iItem < 0 || (unsigned int)pstLVDInfo->item.iItem >= m_LogArray.size())return 0;

		const auto &log = m_LogArray[pstLVDInfo->item.iItem];

		switch (pstLVDInfo->item.iSubItem) {
		case COLUMN_FILENAME:
			if (pstLVDInfo->item.mask & LVIF_TEXT) {
				//omit path, filenames only
				wcsncpy_s(pstLVDInfo->item.pszText,
					pstLVDInfo->item.cchTextMax,
					log._archiveFilename.c_str(),
					pstLVDInfo->item.cchTextMax);
			}
			break;
		case COLUMN_RESULTS:
			if (pstLVDInfo->item.mask & LVIF_TEXT) {
				WORD wCaption = -1;
				switch (log._overallResult) {
				case LF_RESULT::OK:
					wCaption = IDS_LOGINFO_RESULT_OK;
					break;
				case LF_RESULT::CANCELED:
				case LF_RESULT::NG:
					wCaption = IDS_LOGINFO_RESULT_NG;
					break;
				case LF_RESULT::NOTARCHIVE:	//FALLTHROUGH
					wCaption = IDS_LOGINFO_RESULT_NOTARCHIVE;
					break;
				case LF_RESULT::NOTIMPL:	//FALLTHROUGH
				default:
					wCaption = IDS_LOGINFO_RESULT_NOTIMPL;
					break;
				}
				wcsncpy_s(pstLVDInfo->item.pszText,
					pstLVDInfo->item.cchTextMax,
					UtilLoadString(wCaption).c_str(),
					pstLVDInfo->item.cchTextMax);
			}
			break;
		}
		return 0;
	}
	LRESULT OnItemChanged(LPNMHDR pnmh) {
		if (pnmh->hwndFrom != m_ItemListView) {
			return FALSE;
		}

		int iItem = m_ItemListView.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
		//iItem==-1 if not found
		if (iItem < 0 || (unsigned)iItem >= m_LogArray.size())return FALSE;
		m_PathEdit.SetWindowText(m_LogArray[iItem]._archivePath.c_str());
		m_MsgEdit.SetWindowText(m_LogArray[iItem].toString().c_str());
		return TRUE;
	}
	LRESULT OnSortItem(LPNMHDR pnmh) {
		if (pnmh->hwndFrom != m_ItemListView) {
			return 0;
		}

		LPNMLISTVIEW lpNMLV = (LPNMLISTVIEW)pnmh;
		int nCol = lpNMLV->iSubItem;
		if (nCol == m_nSortColumn) {
			//same column is clicked, then reverse
			m_bSortDescending = !m_bSortDescending;
			std::reverse(m_LogArray.begin(), m_LogArray.end());
		} else {
			switch (nCol) {
			case COLUMN_FILENAME:
				std::sort(m_LogArray.begin(), m_LogArray.end(), [](const ARCLOG &x, const ARCLOG &y) {
					return _wcsicmp(x._archiveFilename.c_str(), y._archiveFilename.c_str()) < 0;
				});
				break;
			case COLUMN_RESULTS:
				std::sort(m_LogArray.begin(), m_LogArray.end(), [](const ARCLOG &x, const ARCLOG &y) {
					return (x._overallResult < y._overallResult);
				});
				break;
			default:
				ASSERT(!"This code cannot be run");
			}
		}

		m_nSortColumn = nCol;
		m_ItemListView.Invalidate();
		return 0;
	}


	void OnEnd(UINT uNotifyCode, int nID, HWND hWndCtl){
		EndDialog(nID);
	}

public:
	//custom draw
	DWORD OnPrePaint(int nID, LPNMCUSTOMDRAW lpnmcd) {
		if (lpnmcd->hdr.idFrom == IDC_LIST_LOGINFO_ITEMS)return CDRF_NOTIFYITEMDRAW;
		return CDRF_DODEFAULT;
	}
	DWORD OnItemPrePaint(int nID, LPNMCUSTOMDRAW lpnmcd) {
		if (lpnmcd->hdr.idFrom == IDC_LIST_LOGINFO_ITEMS) {
			//change color to indicate results
			LPNMLVCUSTOMDRAW lpnmlv = (LPNMLVCUSTOMDRAW)lpnmcd;
			switch (m_LogArray[lpnmcd->dwItemSpec]._overallResult) {
			case LF_RESULT::CANCELED:	//FALLTHROUGH
			case LF_RESULT::NG:
				lpnmlv->clrText = RGB(0, 0, 0);
				lpnmlv->clrTextBk = RGB(255, 153, 153);
				break;
			case LF_RESULT::NOTIMPL:
				lpnmlv->clrText = RGB(0, 0, 0);
				lpnmlv->clrTextBk = RGB(204, 204, 255);
				break;
			case LF_RESULT::NOTARCHIVE:
				lpnmlv->clrText = RGB(0, 0, 0);
				lpnmlv->clrTextBk = RGB(240, 240, 0);
				break;
			case LF_RESULT::OK:	//FALLTHROUGH
			default:
				return CDRF_DODEFAULT;
			}
			return CDRF_NOTIFYITEMDRAW;
		}
		return CDRF_DODEFAULT;
	}

	//color for read-only edit box
	HBRUSH OnCtrlColorEdit(HDC, HWND hWnd) {
		if (hWnd == m_MsgEdit || hWnd == m_PathEdit) {
			return (HBRUSH)GetStockObject(WHITE_BRUSH);
		} else return (HBRUSH)CDialogImpl<CLogListDialog>::DefWindowProc();
	}
};
