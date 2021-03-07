#pragma once
#include "FileListModel.h"
#include "FileListMessages.h"
#include "MenuCommand.h"
#include "resource.h"
#include "OLE/DropTarget.h"

//common part of CFileListView and CFileTreeView
template<typename T, typename BASE>
class CFileViewBase:public CWindowImpl<T, BASE>, public ILFDropCommunicator {
protected:
	CFileListModel &mr_Model;
	HWND m_hFrameWnd;
	//---accepting file drops
	CLFDropTarget m_DropTarget;

protected:
	std::vector<const ARCHIVE_ENTRY_INFO*> GetSelectedItems() = 0;
	//bOverwrite:trueなら存在するテンポラリファイルを削除してから解凍する
	bool OpenAssociation(bool bOverwrite, bool bOpen) {
		if (!mr_Model.CheckArchiveExists()) {	//存在しないならエラー
			CString msg;
			msg.Format(IDS_ERROR_FILE_NOT_FOUND, mr_Model.GetArchiveFileName());
			ErrorMessage((const wchar_t*)msg);
			return false;
		}

		//選択されたアイテムを列挙
		std::vector<ARCHIVE_ENTRY_INFO*> items;
		GetSelectedItems(items);
		std::vector<ARCHIVE_ENTRY_INFO*> itemsTmp(items.begin(), items.end());

		if (!items.empty()) {
			std::vector<std::wstring> filesList;
			std::wstring strLog;
			if (!mr_Model.MakeSureItemsExtracted(NULL, bOverwrite, mr_Model.GetRootNode(), itemsTmp, filesList, strLog)) {
				//TODO
				CLogListDialog LogDlg(L"Log");
				std::vector<ARCLOG> logs;
				logs.resize(1);
				logs.back().logs.resize(1);
				logs.back().logs.back().entryPath = mr_Model.GetArchiveFileName();
				logs.back().logs.back().message = strLog;
				LogDlg.SetLogArray(logs);
				LogDlg.DoModal(m_hFrameWnd);
			}
			if (bOpen)OpenAssociation(filesList);
		}

		return true;
	}

	void OpenAssociation(const std::vector<std::wstring> &filesList) {
		for (const auto& file : filesList) {
			//拒否されたら上書きも追加解凍もしない;ディレクトリなら拒否のみチェック
			bool bDenyOnly = BOOL2bool(::PathIsDirectory(file.c_str()));//lpNode->bDir;
			if (mr_Model.IsPathAcceptableToOpenAssoc(file.c_str(), bDenyOnly)) {
				::ShellExecute(GetDesktopWindow(), NULL, file.c_str(), NULL, NULL, SW_SHOW);
				TRACE(_T("%s\n"), file.c_str());
				//::ShellExecute(GetDesktopWindow(),_T("explore"),*ite,NULL,NULL,SW_SHOW);
			} else {
				::MessageBeep(MB_ICONEXCLAMATION);
			}
		}
	}


protected:
	void OnOpenWithUserApp(UINT uNotifyCode, int nID, HWND hWndCtrl) {
		if (nID < ID_MENUITEM_USERAPP_END) {
			//LhaForge custom command
			OnUserApp(MenuCommand_GetCmdArray(), nID - ID_MENUITEM_USERAPP_BEGIN);
		} else {
			//SendTo command
			OnSendToApp(nID - ID_MENUITEM_USERAPP_END);
		}
	}
	void OnOpenAssociation(UINT uNotifyCode, int nID, HWND hWndCtrl)
	{
		//trueなら存在するテンポラリファイルを削除してから解凍する
		const bool bOverwrite = (nID == ID_MENUITEM_OPEN_ASSOCIATION_OVERWRITE);
		OpenAssociation(bOverwrite, true);
	}

	void OnExtractTemporary(UINT uNotifyCode, int nID, HWND hWndCtrl)
	{
		//上書きはするが、開かない
		OpenAssociation(true, false);
	}


	bool OnUserApp(const std::vector<CLFMenuCommandItem> &menuCommandArray, UINT nID)	//「プログラムで開く」のハンドラ
	{
		ASSERT(!menuCommandArray.empty());
		ASSERT(nID < menuCommandArray.size());
		if (nID >= menuCommandArray.size())return false;

		if (!mr_Model.IsOK())return false;

		auto items = GetSelectedItems();
		std::vector<ARCHIVE_ENTRY_INFO*> itemsTmp(items.begin(), items.end());

		std::vector<std::wstring> filesList;
		if (!items.empty()) {
			std::wstring strLog;
			if (!mr_Model.MakeSureItemsExtracted(NULL, mr_Model.GetRootNode(), false, itemsTmp, filesList, strLog)) {
				//TODO
				CLogListDialog LogDlg(L"Log");
				std::vector<ARCLOG> logs;
				logs.resize(1);
				logs.back().logs.resize(1);
				logs.back().logs.back().entryPath = mr_Model.GetArchiveFileName();
				logs.back().logs.back().message = strLog;
				LogDlg.SetLogArray(logs);
				LogDlg.DoModal(m_hFrameWnd);
				return false;
			}
		}

		//---実行情報取得
		//パラメータ展開に必要な情報
		auto envInfo = LF_make_expand_information(nullptr, nullptr);

		//コマンド・パラメータ展開
		auto strCmd = UtilExpandTemplateString(menuCommandArray[nID].Path, envInfo);	//コマンド
		auto strParam = UtilExpandTemplateString(menuCommandArray[nID].Param, envInfo);	//パラメータ
		auto strDir = UtilExpandTemplateString(menuCommandArray[nID].Dir, envInfo);	//ディレクトリ

		//引数置換
		if (std::wstring::npos != strParam.find(L"%F")) {
			//ファイル一覧を連結して作成
			CString strFileList;
			for (const auto& file : filesList) {
				CPath path = file.c_str();
				path.QuoteSpaces();
				strFileList += (LPCTSTR)path;
				strFileList += _T(" ");
			}
			strParam = replace(strParam, L"%F", strFileList);
			//---実行
			::ShellExecuteW(GetDesktopWindow(), NULL, strCmd.c_str(), strParam.c_str(), strDir.c_str(), SW_SHOW);
		} else if (std::wstring::npos != strParam.find(L"%S")) {
			for (const auto& file : filesList) {
				CPath path = file.c_str();
				path.QuoteSpaces();

				CString strParamTmp = strParam.c_str();
				strParamTmp.Replace(_T("%S"), (LPCTSTR)path);
				//---実行
				::ShellExecuteW(GetDesktopWindow(), NULL, strCmd.c_str(), strParamTmp, strDir.c_str(), SW_SHOW);
			}
		} else {
			::ShellExecuteW(GetDesktopWindow(), NULL, strCmd.c_str(), strParam.c_str(), strDir.c_str(), SW_SHOW);
		}

		return true;
	}

	bool OnSendToApp(UINT nID)	//「プログラムで開く」のハンドラ
	{
		ASSERT(nID < MenuCommand_GetSendToCmdArray().size());
		if (nID >= MenuCommand_GetSendToCmdArray().size())return false;

		if (!mr_Model.IsOK())return false;

		//---選択解凍開始
		//選択されたアイテムを列挙
		std::vector<ARCHIVE_ENTRY_INFO*> items;
		GetSelectedItems(items);
		std::vector<ARCHIVE_ENTRY_INFO*> itemsTmp(items.begin(), items.end());

		std::vector<std::wstring> filesList;
		if (!items.empty()) {
			std::wstring strLog;
			if (!mr_Model.MakeSureItemsExtracted(NULL, false, mr_Model.GetRootNode(), itemsTmp, filesList, strLog)) {
				//TODO
				CLogListDialog LogDlg(L"Log");
				std::vector<ARCLOG> logs;
				logs.resize(1);
				logs.back().logs.resize(1);
				logs.back().logs.back().entryPath = mr_Model.GetArchiveFileName();
				logs.back().logs.back().message = strLog;
				LogDlg.SetLogArray(logs);
				LogDlg.DoModal(m_hFrameWnd);
				return false;
			}
		}

		//引数置換
		const auto& sendToCmd = MenuCommand_GetSendToCmdArray();
		if (PathIsDirectory(sendToCmd[nID].cmd.c_str())) {
			//対象はディレクトリなので、コピー
			CString strFiles;
			for (const auto& file : filesList) {
				CPath file_ = file.c_str();
				file_.RemoveBackslash();
				strFiles += file_;
				strFiles += _T('|');
			}
			strFiles += _T('|');
			//TRACE(strFiles);
			auto filter = UtilMakeFilterString((const wchar_t*)strFiles);

			CPath destDir = sendToCmd[nID].cmd.c_str();
			destDir.AddBackslash();
			//Windows標準のコピー動作
			SHFILEOPSTRUCT fileOp = { 0 };
			fileOp.wFunc = FO_COPY;
			fileOp.fFlags = FOF_NOCONFIRMMKDIR | FOF_NOCOPYSECURITYATTRIBS | FOF_NO_CONNECTED_ELEMENTS;
			fileOp.pFrom = &filter[0];
			fileOp.pTo = destDir;

			//コピー実行
			if (::SHFileOperation(&fileOp)) {
				//エラー
				ErrorMessage((const wchar_t*)CString(MAKEINTRESOURCE(IDS_ERROR_FILE_COPY)));
				return false;
			} else if (fileOp.fAnyOperationsAborted) {
				//キャンセル
				ErrorMessage((const wchar_t*)CString(MAKEINTRESOURCE(IDS_ERROR_USERCANCEL)));
				return false;
			}
			return true;
		} else {
			//送り先はプログラム
			//ファイル一覧を連結して作成
			CString strFileList;
			for (const auto& file : filesList) {
				CPath path = file.c_str();
				path.QuoteSpaces();
				strFileList += (LPCTSTR)path;
				strFileList += _T(" ");
			}
			CString strParam = CString(sendToCmd[nID].param.c_str()) + _T(" ") + strFileList;
			//---実行
			CPath cmd = sendToCmd[nID].cmd.c_str();
			cmd.QuoteSpaces();
			CPath workDir = sendToCmd[nID].workingDir.c_str();
			workDir.QuoteSpaces();
			::ShellExecute(GetDesktopWindow(), NULL, cmd, strParam, workDir, SW_SHOW);
		}

		return true;
	}
	void OnContextMenu(HWND hWndCtrl, CPoint &Point) {
		//enumerate selected items
		std::vector<const ARCHIVE_ENTRY_INFO*> items = GetSelectedItems();
		if (items.empty())return;
		const ARCHIVE_ENTRY_INFO* item = items.front();

		if (-1 == Point.x&&-1 == Point.y) {
			//from keyboard; menu is shown on the left top corner
			Point.x = Point.y = 0;
			ClientToScreen(&Point);
		}

		CMenu cMenu;
		CMenuHandle cSubMenu;
		HWND hWndSendTo = NULL;
		if (item->_parent) {
			hWndSendTo = m_hWnd;
			//standard menu
			cMenu.LoadMenu(IDR_FILELIST_POPUP);
			cSubMenu = cMenu.GetSubMenu(0);

			//add command to sub-menu
			auto menuCount = cSubMenu.GetMenuItemCount();
			int iIndex = -1;
			for (auto i = 0; i <= menuCount; i++) {
				if (-1 == cSubMenu.GetMenuItemID(i)) {	//popup
					iIndex = i;
					break;
				}
			}
			ASSERT(-1 != iIndex);
			if (-1 != iIndex) {
				MenuCommand_MakeUserAppMenu(cSubMenu.GetSubMenu(iIndex));
				MenuCommand_MakeSendToMenu(cSubMenu.GetSubMenu(iIndex + 1));
			}
		} else {
			hWndSendTo = m_hFrameWnd;
			//root menu
			cMenu.LoadMenu(IDR_ARCHIVE_POPUP);
			cSubMenu = cMenu.GetSubMenu(0);
		}
		cSubMenu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, Point.x, Point.y, hWndSendTo, NULL);
	}
	void OnDelete(UINT uNotifyCode, int nID, HWND hWndCtrl){
		auto items = GetSelectedItems();
		ASSERT(!items.empty());
		if (items.empty()) {
			return;
		}

		if (!mr_Model.IsModifySupported()) {
			//not supported
			if (1 == uNotifyCode) {	//from keyboard
				MessageBeep(MB_OK);
			} else {	//from menu
				ErrorMessage(
					UtilLoadString(IDS_ERROR_FILELIST_DELETE_SELECTED_NOT_SUPPORTED)
				);
			}
			return;
		}

		//confirm
		if (IDYES != UtilMessageBox(m_hWnd,
			UtilLoadString(IDS_ASK_FILELIST_DELETE_SELECTED),
			MB_YESNO | MB_DEFBUTTON2 | MB_ICONEXCLAMATION)
			) {
			return;
		}

		//::EnableWindow(m_hFrameWnd,FALSE);	TODO: is this necessary? modal dialog will disable the frame automatically
		ARCLOG arcLog;
		try {
			mr_Model.DeleteItems(items, CLFProgressHandlerGUI(m_hFrameWnd), arcLog);
		} catch (...) {
			CLogListDialog LogDlg(L"Log");
			std::vector<ARCLOG> logs = { arcLog };
			LogDlg.SetLogArray(logs);
			LogDlg.DoModal(m_hFrameWnd);
		}
		//::EnableWindow(m_hFrameWnd,TRUE);
		//SetForegroundWindow(m_hFrameWnd);

		//request content re-scan
		::PostMessage(m_hFrameWnd, WM_FILELIST_REFRESH, 0, 0);
	}
	void EnableDropTarget(bool bEnable) {
		if (bEnable) {
			::RegisterDragDrop(m_hWnd, &m_DropTarget);
		} else {
			::RevokeDragDrop(m_hWnd);
		}
	}

public:
	CFileViewBase(): m_DropTarget(this){}
	virtual ~CFileViewBase() {}
	void SetFrameWnd(HWND hWnd) { m_hFrameWnd = hWnd; }

	//---ILFDropCommunicator
	HRESULT DragEnter(IDataObject *lpDataObject, POINTL &pt, DWORD &dwEffect)
	{
		return DragOver(lpDataObject, pt, dwEffect);
	}

	HRESULT DragLeave()
	{
		//全てのハイライトを無効に
		if (m_hDropHilight) {
			SetItemState(m_hDropHilight, ~LVIS_DROPHILITED, LVIS_DROPHILITED);
		}
		m_hDropHilight = NULL;
		return S_OK;
	}

	HRESULT DragOver(IDataObject *, POINTL &pt, DWORD &dwEffect)
	{
		//フォーマットに対応した処理をする
		if (!m_DropTarget.QueryFormat(CF_HDROP) || !mr_Model.IsModifySupported()) {	//ファイル専用
			//ファイルではないので拒否
			dwEffect = DROPEFFECT_NONE;
		} else {
			//---ドロップ先アイテムを取得
			CPoint ptTemp(pt.x, pt.y);
			ScreenToClient(&ptTemp);
			HTREEITEM hItem = HitTest(ptTemp, NULL);

			//---ちらつきを押さえるため、前と同じアイテムがハイライトされるならハイライトをクリアしない
			if (hItem != m_hDropHilight) {
				//前のハイライトを無効に
				if (m_hDropHilight) {
					SetItemState(m_hDropHilight, ~LVIS_DROPHILITED, LVIS_DROPHILITED);
				}
				//新しいハイライト
				if (hItem) {
					SelectDropTarget(hItem);
					m_hDropHilight = hItem;
				}
			}
			dwEffect = hItem ? DROPEFFECT_COPY : DROPEFFECT_NONE;
		}
		return S_OK;
	}

	//ファイルのドロップ
	HRESULT Drop(IDataObject *lpDataObject, POINTL &pt, DWORD &dwEffect)
	{
		//前のハイライトを無効に
		if (m_hDropHilight) {
			SetItemState(m_hDropHilight, ~LVIS_DROPHILITED, LVIS_DROPHILITED);
		} else {
			return E_HANDLE;
		}

		//ファイル取得
		auto[hr, files] = m_DropTarget.GetDroppedFiles(lpDataObject);
		if (S_OK == hr) {
			//ファイルがドロップされた
			dwEffect = DROPEFFECT_COPY;

			//---ドロップ先を特定
			CPoint ptTemp(pt.x, pt.y);
			ScreenToClient(&ptTemp);
			HTREEITEM hItem = HitTest(ptTemp, NULL);

			CString strDest;	//放り込む先
			ARCHIVE_ENTRY_INFO* lpNode = (ARCHIVE_ENTRY_INFO*)GetItemData(hItem);
			if (lpNode) {		//アイテム上にDnD
				//アイテムがフォルダだったらそのフォルダに追加
				ASSERT(lpNode->is_directory());
				strDest = lpNode->getRelativePath(mr_Model.GetRootNode()).c_str();
			} else {
				return E_HANDLE;
			}
			TRACE(_T("Target:%s\n"), (LPCTSTR)strDest);

			//追加開始
			::EnableWindow(m_hFrameWnd, FALSE);
			CString strLog;

			HRESULT hr = mr_Model.AddItem(files, strDest, strLog);

			::EnableWindow(m_hFrameWnd, TRUE);
			::SetForegroundWindow(m_hFrameWnd);

			if (FAILED(hr) || S_FALSE == hr) {
				CString msg;
				switch (hr) {
				case E_LF_SAME_INPUT_AND_OUTPUT:	//アーカイブ自身を追加しようとした
					msg.Format(IDS_ERROR_SAME_INPUT_AND_OUTPUT, mr_Model.GetArchiveFileName());
					ErrorMessage((const wchar_t*)msg);
					break;
				case E_LF_UNICODE_NOT_SUPPORTED:	//ファイル名にUNICODE文字を持つファイルを圧縮しようとした
					ErrorMessage((const wchar_t*)CString(MAKEINTRESOURCE(IDS_ERROR_UNICODEPATH)));
					break;
				case S_FALSE:	//追加処理に問題
				{
					//TODO
					CLogListDialog LogDlg(L"Log");
					std::vector<ARCLOG> logs;
					logs.resize(1);
					logs.back().logs.resize(1);
					logs.back().logs.back().entryPath = mr_Model.GetArchiveFileName();
					logs.back().logs.back().message = strLog;
					LogDlg.SetLogArray(logs);
					LogDlg.DoModal(m_hFrameWnd);
				}
				break;
				default:
					ASSERT(!"This code cannot be run");
				}
				return E_INVALIDARG;
			}
			//アーカイブ更新
			::PostMessage(m_hFrameWnd, WM_FILELIST_REFRESH, 0, 0);
			return S_OK;
		} else {
			//受け入れできない形式
			dwEffect = DROPEFFECT_NONE;
			return S_FALSE;	//S_OK
		}
	}

};

