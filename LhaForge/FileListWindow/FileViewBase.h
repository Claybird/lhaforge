#pragma once
#include "FileListModel.h"
#include "FileListMessages.h"
#include "MenuCommand.h"
#include "resource.h"
#include "OLE/DropTarget.h"
#include "ConfigCode/ConfigFileListWindow.h"
#include "Dialogs/LogListDialog.h"

//common part of CFileListView and CFileTreeView
template<typename TBase, typename TWinTraits>
class CFileViewBase:public CWindowImpl<TBase, TWinTraits>, public ILFDropCommunicator {
protected:
	CFileListModel &mr_Model;
	HWND m_hFrameWnd;
	//---accepting file drops
	CLFDropTarget m_DropTarget;
	const CConfigFileListWindow &mr_confFLW;

	struct HIGHLIGHT {
		HTREEITEM itemTree;
		int itemList;
		HIGHLIGHT() { invalidate(); }
		virtual ~HIGHLIGHT() {}
		void invalidate() { itemTree = nullptr; itemList = -1; }
		bool isValid()const { return itemTree != nullptr || itemList != -1; }
		HTREEITEM operator=(HTREEITEM n) { itemTree = n; return n; }
		int operator=(int n) { itemList = n; return n; }
		operator HTREEITEM()const { return itemTree; }
		operator int()const { return itemList; }
		bool operator==(const HIGHLIGHT& a)const { return a.itemTree == itemTree || a.itemList == itemList; }
		bool operator!=(const HIGHLIGHT& a)const { return a.itemTree != itemTree || a.itemList != itemList; }
	}m_dropHighlight;

protected:
	virtual void displayLog(ARCLOG& arcLog) {
		CLogListDialog LogDlg(L"Log");
		std::vector<ARCLOG> logs;
		logs.push_back(arcLog);
		LogDlg.SetLogArray(logs);
		LogDlg.DoModal(m_hFrameWnd);
	}
	virtual std::vector<const ARCHIVE_ENTRY_INFO*> GetSelectedItems() = 0;
	virtual std::vector<std::filesystem::path> extractItemToTemporary(bool bOverwrite, std::vector<const ARCHIVE_ENTRY_INFO*>& items) {
		if (!mr_Model.CheckArchiveExists()) {	//Error: file not exist
			ErrorMessage(Format(UtilLoadString(IDS_ERROR_FILE_NOT_FOUND), mr_Model.GetArchiveFileName().c_str()));
			return {};
		}

		if (items.empty()) {
			return {};
		}

		ARCLOG arcLog;
		auto options = bOverwrite ? overwrite_options::overwrite : overwrite_options::skip;
		try {
			auto output_dir = mr_Model.getTempDir();
			//this will return all entries of extracted items; not of the selected item
			mr_Model.MakeSureItemsExtracted(
				items,
				output_dir,
				mr_Model.GetRootNode(),
				CLFProgressHandlerGUI(m_hFrameWnd),
				options,
				arcLog);

			std::vector<std::filesystem::path> selected_extracted;
			for (const auto item : items) {
				selected_extracted.push_back(output_dir / LF_sanitize_pathname(item->_entry.path));
			}
			return selected_extracted;
		} catch (...) {
			displayLog(arcLog);
			return {};
		}
	}
	virtual void openAssociation(const std::vector<std::filesystem::path> &filesList)const {
		for (const auto& file : filesList) {
			bool bDenyOnly = std::filesystem::is_directory(file);
			if (mr_confFLW.isPathAcceptableToOpenAssoc(file.c_str(), bDenyOnly)) {
				::ShellExecuteW(GetDesktopWindow(), nullptr, file.c_str(), nullptr, nullptr, SW_SHOW);
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
	void OnOpenAssociation(UINT uNotifyCode, int nID, HWND hWndCtrl){
		bool bOverwrite = (nID == ID_MENUITEM_OPEN_ASSOCIATION_OVERWRITE);
		auto files = extractItemToTemporary(bOverwrite, GetSelectedItems());
		openAssociation(files);
	}

	void OnExtractTemporary(UINT uNotifyCode, int nID, HWND hWndCtrl){
		//just overwrite, not to open
		extractItemToTemporary(true, GetSelectedItems());
	}

	bool OnUserApp(const std::vector<CLFMenuCommandItem> &menuCommandArray, UINT nID){
		ASSERT(!menuCommandArray.empty());
		ASSERT(nID < menuCommandArray.size());
		if (nID >= menuCommandArray.size())return false;

		if (!mr_Model.IsOK())return false;

		auto files = extractItemToTemporary(false, GetSelectedItems());

		//expand commands and parameters
		auto envInfo = LF_make_expand_information(nullptr, nullptr);
		auto strCmd = UtilExpandTemplateString(menuCommandArray[nID].Path, envInfo);
		auto strParam = UtilExpandTemplateString(menuCommandArray[nID].Param, envInfo);
		auto strDir = UtilExpandTemplateString(menuCommandArray[nID].Dir, envInfo);

		if (std::wstring::npos != strParam.find(L"%F")) {
			//join filenames
			std::wstring strFileList;
			for (const auto& file : files) {
				strFileList += Format(L"\"%s\" ", file.c_str());
			}
			strParam = replace(strParam, L"%F", strFileList);
			//---exec
			::ShellExecuteW(GetDesktopWindow(), nullptr, strCmd.c_str(), strParam.c_str(), strDir.c_str(), SW_SHOW);
		} else if (std::wstring::npos != strParam.find(L"%S")) {
			for (const auto& file : files) {
				auto path = Format(L"\"%s\"", file.c_str());
				auto strParamTmp = replace(strParam, L"%S", path);
				//---exec
				::ShellExecuteW(GetDesktopWindow(), nullptr, strCmd.c_str(), strParamTmp.c_str(), strDir.c_str(), SW_SHOW);
			}
		} else {
			::ShellExecuteW(GetDesktopWindow(), nullptr, strCmd.c_str(), strParam.c_str(), strDir.c_str(), SW_SHOW);
		}

		return true;
	}

	bool OnSendToApp(UINT nID){
		ASSERT(nID < MenuCommand_GetSendToCmdArray().size());
		if (nID >= MenuCommand_GetSendToCmdArray().size())return false;

		if (!mr_Model.IsOK())return false;

		auto files = extractItemToTemporary(false, GetSelectedItems());

		const auto& sendToCmd = MenuCommand_GetSendToCmdArray();
		if (std::filesystem::is_directory(sendToCmd[nID].cmd)) {
			//---destination is directory
			std::wstring strFiles;
			for (const auto& file : files) {
				strFiles += UtilPathRemoveLastSeparator(file);
				strFiles += L'|';
			}
			strFiles += L'|';
			auto filter = UtilMakeFilterString(strFiles);

			auto destDir = UtilPathAddLastSeparator(sendToCmd[nID].cmd);
			//copy file
			SHFILEOPSTRUCTW fileOp = { 0 };
			fileOp.wFunc = FO_COPY;
			fileOp.fFlags = FOF_NOCONFIRMMKDIR | FOF_NOCOPYSECURITYATTRIBS | FOF_NO_CONNECTED_ELEMENTS;
			fileOp.pFrom = &filter[0];
			fileOp.pTo = destDir.c_str();

			if (::SHFileOperationW(&fileOp)) {
				ErrorMessage(UtilLoadString(IDS_ERROR_FILE_COPY));
				return false;
			} else if (fileOp.fAnyOperationsAborted) {
				//cancel
				ErrorMessage(UtilLoadString(IDS_ERROR_USERCANCEL));
				return false;
			}
			return true;
		} else {
			//---destination is a program
			std::wstring strFileList;
			for (const auto& file : files) {
				strFileList += Format(L"\"%s\" ", file.c_str());
			}
			auto strParam = L"\"" + sendToCmd[nID].param + L"\" " + strFileList;

			auto cmd = L"\"" + sendToCmd[nID].cmd.wstring() + L"\"";
			auto workDir = L"\"" + sendToCmd[nID].workingDir.wstring() + L"\"";
			::ShellExecuteW(GetDesktopWindow(), nullptr, cmd.c_str(), strParam.c_str(), workDir.c_str(), SW_SHOW);
		}

		return true;
	}
	void OnContextMenu(HWND hWndCtrl, CPoint &Point) {
		//enumerate selected items
		auto items = GetSelectedItems();
		if (items.empty())return;
		const ARCHIVE_ENTRY_INFO* item = items.front();

		if (-1 == Point.x&&-1 == Point.y) {
			//from keyboard; menu is shown on the left top corner
			Point.x = Point.y = 0;
			ClientToScreen(&Point);
		}

		CMenu cMenu;
		CMenuHandle cSubMenu;
		HWND hWndSendTo = nullptr;
		if (item->_parent) {
			hWndSendTo = m_hWnd;
			//standard menu
			cMenu.LoadMenu(IDR_FILELIST_POPUP);
			cSubMenu = cMenu.GetSubMenu(0);
			if (!mr_Model.IsModifySupported()) {
				cMenu.EnableMenuItem(ID_MENUITEM_DELETE_SELECTED, MF_GRAYED | MF_BYCOMMAND);
			}

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
		cSubMenu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, Point.x, Point.y, hWndSendTo, nullptr);
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
	virtual void OnExtractItem(UINT, int nID, HWND){
		if (!mr_Model.IsOK()) {
			return;// false;
		}

		auto items = GetSelectedItems();
		if (items.empty()) {
			//no file selected
			ErrorMessage(UtilLoadString(IDS_ERROR_FILELIST_NOT_SELECTED));
			return;// false;
		}

		const bool bSameDir = (ID_MENUITEM_EXTRACT_SELECTED_SAMEDIR == nID);
		std::filesystem::path pathOutputDir = mr_Model.GetArchiveFileName().parent_path();
		if (!bSameDir) {
			//show dialog
			LFShellFileOpenDialog dlg(pathOutputDir.c_str(), FOS_FORCEFILESYSTEM | FOS_FILEMUSTEXIST | FOS_PATHMUSTEXIST | FOS_PICKFOLDERS);
			if (IDOK != dlg.DoModal()) {
				return;	//cancel
			}
			CString tmp;
			dlg.GetFilePath(tmp);
			pathOutputDir = tmp.operator LPCWSTR();
		}

		ARCLOG arcLog;
		try {
			mr_Model.ExtractItems(items, pathOutputDir, mr_Model.getCurrentDir(), CLFProgressHandlerGUI(m_hFrameWnd), arcLog);
		} catch (...) {
			CLogListDialog LogDlg(L"Log");
			std::vector<ARCLOG> logs = { arcLog };
			LogDlg.SetLogArray(logs);
			LogDlg.DoModal(m_hFrameWnd);
		}
	}

	void EnableDropTarget(bool bEnable) {
		if (bEnable) {
			::RegisterDragDrop(m_hWnd, &m_DropTarget);
		} else {
			::RevokeDragDrop(m_hWnd);
		}
	}

public:
	CFileViewBase(CFileListModel& rModel, const CConfigFileListWindow &r_confFLW):
		mr_Model(rModel),
		mr_confFLW(r_confFLW),
		m_DropTarget(this){}
	virtual ~CFileViewBase() {}
	virtual void SetFrameWnd(HWND hWnd) { m_hFrameWnd = hWnd; }

	//---ILFDropCommunicator
	virtual HRESULT DragEnter(IDataObject *lpDataObject, POINTL &pt, DWORD &dwEffect)override {
		return DragOver(lpDataObject, pt, dwEffect);
	}
	virtual HRESULT DragLeave()override {
		//disable all highlights
		if (m_dropHighlight.isValid()) {
			SetItemState(m_dropHighlight, ~LVIS_DROPHILITED, LVIS_DROPHILITED);
		}
		m_dropHighlight.invalidate();
		return S_OK;
	}
	virtual bool IsValidDropTarget(const HIGHLIGHT&)const = 0;
	virtual HRESULT DragOver(IDataObject *, POINTL &pt, DWORD &dwEffect)override {
		if (!mr_Model.IsModifySupported()) {
			dwEffect = DROPEFFECT_NONE;
		} else if (!m_DropTarget.QueryFormat(CF_HDROP)) {
			//not a file nor directory, reject
			dwEffect = DROPEFFECT_NONE;
		} else {
			//get drop target
			CPoint ptTemp(pt.x, pt.y);
			ScreenToClient(&ptTemp);
			HIGHLIGHT target;
			target = HitTest(ptTemp, NULL);
			if (!IsValidDropTarget(target))target.invalidate();

			//---stop flicker
			if (target != m_dropHighlight) {
				if (m_dropHighlight.isValid()) {
					SetItemState(m_dropHighlight, ~LVIS_DROPHILITED, LVIS_DROPHILITED);
				}
				if (target.isValid()) {
					SetItemState(target, LVIS_DROPHILITED, LVIS_DROPHILITED);
				}
				m_dropHighlight = target;
			}
			dwEffect = target.isValid() ? DROPEFFECT_COPY : DROPEFFECT_NONE;
		}
		return S_OK;
	}
	virtual HRESULT AddItemsToDirectory(const ARCHIVE_ENTRY_INFO* target, const std::vector<std::filesystem::path>& files) {
		ASSERT(target);

		//::EnableWindow(m_hFrameWnd,FALSE);	TODO: is this necessary? modal dialog will disable the frame automatically
		ARCLOG arcLog;
		try {
			mr_Model.AddItem(files, target, CLFProgressHandlerGUI(m_hFrameWnd), arcLog);
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
		return S_OK;
	}
};

