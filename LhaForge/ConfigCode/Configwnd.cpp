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

#include "stdafx.h"
#include "../ArchiverCode/arc_interface.h"
#include "configwnd.h"
#include "../Utilities/OSUtil.h"


CConfigDialog::CConfigDialog(CConfigManager &cfg)
	:mr_Config(cfg),
	hActiveDialogWnd(NULL),
	PageShellExt(*this),
	PageAssociation(*this),
	m_nAssistRequireCount(0)
{
	TRACE(_T("CConfigDialog()\n"));

	//テンポラリINIファイル名取得
	m_strAssistINI = UtilGetTemporaryFileName().c_str();

	//設定読み込み
	CString strErr;
	if(!mr_Config.LoadConfig(strErr))ErrorMessage((const wchar_t*)strErr);
}

CConfigDialog::~CConfigDialog()
{
}


LRESULT CConfigDialog::OnInitDialog(HWND hWnd, LPARAM lParam)
{
	// 大きいアイコン設定
	HICON hIcon = AtlLoadIconImage(IDI_APP, LR_DEFAULTCOLOR,::GetSystemMetrics(SM_CXICON),::GetSystemMetrics(SM_CYICON));
	SetIcon(hIcon, TRUE);

	// 小さいアイコン設定
	HICON hIconSmall = AtlLoadIconImage(IDI_APP, LR_DEFAULTCOLOR,::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON));
	SetIcon(hIconSmall, FALSE);

	//プロパティシートを貼り付けるためのスクロールコンテナの配置場所を取得
	CStatic StaticFrame;
	StaticFrame=GetDlgItem(IDC_STATIC_FRAME);
	RECT rect;
	StaticFrame.GetWindowRect(&rect);
//	StaticFrame.ShowWindow(SW_HIDE);
	ScreenToClient(&rect);

	//スクロールコンテナを配置
	ScrollWindow.Create(m_hWnd,rect,NULL,/*WS_TABSTOP|*/WS_CHILD|WS_VISIBLE , WS_EX_CLIENTEDGE|WS_EX_CONTROLPARENT);

	//--------------------
	// ツリービューの設定
	//--------------------
	SelectTreeView=GetDlgItem(IDC_TREE_SELECT_PROPPAGE);

#define ADD_PAGE(_DIALOG,_ROOTITEM) {\
	m_ConfigDlgList.insert(&_DIALOG);\
	_DIALOG.LoadConfig(mr_Config);\
	_DIALOG.Create(ScrollWindow);\
	CString strTitle;\
	_DIALOG.GetWindowText(strTitle);\
	HTREEITEM hItem=SelectTreeView.InsertItem(strTitle, _ROOTITEM, TVI_LAST);\
	SelectTreeView.SetItemData(hItem,(DWORD_PTR)_DIALOG.m_hWnd);\
}

	//--------------------------------
	// ダイアログの情報を追加していく
	//--------------------------------
	ADD_PAGE(PageGeneral,TVI_ROOT);
	ADD_PAGE(PageShellExt,TVI_ROOT);
	ADD_PAGE(PageShortcut,TVI_ROOT);
	ADD_PAGE(PageFileListWindow,TVI_ROOT);
	ADD_PAGE(PageCompressGeneral,TVI_ROOT);
	ADD_PAGE(PageExtractGeneral,TVI_ROOT);
	ADD_PAGE(PageAssociation,TVI_ROOT);
	ADD_PAGE(PageOpenAction,TVI_ROOT);
//	ADD_PAGE(PageAssociation2,TVI_ROOT);
	ADD_PAGE(PageVersion,TVI_ROOT);

	//----------------
	// 以下は詳細設定
	//----------------
	//ツリーの親
	m_ConfigDlgList.insert(&PageDetail);
	PageDetail.LoadConfig(mr_Config);
	PageDetail.Create(ScrollWindow);
	HTREEITEM hItemDetail;
	{
		TCHAR Buffer[_MAX_PATH+1]={0};
		PageDetail.GetWindowText(Buffer,_MAX_PATH);
		hItemDetail=SelectTreeView.InsertItem(Buffer, TVI_ROOT, TVI_LAST);
	}
	SelectTreeView.SetItemData(hItemDetail,(DWORD_PTR)PageDetail.m_hWnd);

	//ADD_PAGE(PageZIP,hItemDetail);
	//ADD_PAGE(Page7Z,hItemDetail);

	//------------------------
	// はじめに表示するページ
	//------------------------
	PageGeneral.ShowWindow(SW_SHOW);
	ScrollWindow.SetClient(PageGeneral);
	hActiveDialogWnd=PageGeneral;
	SelectTreeView.SetFocus();

	// ダイアログリサイズ初期化
	DlgResize_Init(true, true, WS_THICKFRAME | WS_CLIPCHILDREN);

	//---ユーザー間共通設定?
	if(mr_Config.IsUserCommon()){
		//ウィンドウタイトルを設定
		SetWindowText(CString(MAKEINTRESOURCE(IDS_CAPTION_CONFIG_USERCOMMON)));
	}

	//ウィンドウを中心に
	CenterWindow();

	return TRUE;
}

void CConfigDialog::OnOK(UINT uNotifyCode, int nID, HWND hWndCtl)
{
	//直前にメニューエディタなどで設定が変更されていた場合、ここで単純に上書きすると、変更後の情報が消えてしまうので
	//再読み込みを行い、それを元に上書きする
	CString tmp;
	mr_Config.LoadConfig(tmp);

	//各ダイアログのOnApplyを呼ぶ
	bool bRet=true;
	for(std::set<IConfigDlgBase*>::iterator ite=m_ConfigDlgList.begin();ite!=m_ConfigDlgList.end();++ite){
		bRet=bRet && (*ite)->OnApply();
		(*ite)->StoreConfig(mr_Config);
	}

	//UAC回避のアシスタントが要請されている
	if(m_nAssistRequireCount>0){
		//64bitなら64bit専用処理を先に走らせる
		BOOL iswow64 = FALSE;
		IsWow64Process(GetCurrentProcess(), &iswow64);
		if(iswow64){
			//先にLFAssist(64bit)に処理を渡す。ただしINI削除は行わない。
			//---アシスタント(64bit)のパスを取得
			CPath strExePath(UtilGetModuleDirectoryPath().c_str());
			strExePath+=_T("LFAssist64.exe");
			if(strExePath.FileExists()){	//ファイルが存在するときのみ
				strExePath.QuoteSpaces();

				//---実行:CreateProcessではUACをチェックして実行してもらえない
				SHELLEXECUTEINFO shei={0};
				shei.fMask=SEE_MASK_FLAG_DDEWAIT;	//瞬時に終了する可能性があるのでこれを指定する
				shei.cbSize=sizeof(shei);
				shei.lpFile=strExePath;
				shei.lpParameters=m_strAssistINI;
				shei.nShow=SW_SHOW;
				if(!ShellExecuteEx(&shei)){
					//実行エラー
					auto strLastError = UtilGetLastErrorMessage();

					CString msg;
					msg.Format(IDS_ERROR_CANNOT_EXECUTE,strExePath,strLastError.c_str());

					ErrorMessage((const wchar_t*)msg);
				}
			}
		}

		//LFAssist(32bit)にINI削除を要請
		WritePrivateProfileString(_T("PostProcess"),_T("DeleteMe"),_T("Please_Delete_Me"),m_strAssistINI);

		//変更を実行
		//---アシスタントのパスを取得
		CPath strExePath(UtilGetModuleDirectoryPath().c_str());
		strExePath+=_T("LFAssist.exe");
		strExePath.QuoteSpaces();

		//---実行:CreateProcessではUACをチェックして実行してもらえない
		SHELLEXECUTEINFO shei={0};
		shei.fMask=SEE_MASK_FLAG_DDEWAIT;	//瞬時に終了する可能性があるのでこれを指定する
		shei.cbSize=sizeof(shei);
		shei.lpFile=strExePath;
		shei.lpParameters=m_strAssistINI;
		shei.nShow=SW_SHOW;
		if(!ShellExecuteEx(&shei)){
			//実行エラー
			auto strLastError = UtilGetLastErrorMessage();

			CString msg;
			msg.Format(IDS_ERROR_CANNOT_EXECUTE,strExePath,strLastError.c_str());

			ErrorMessage((const wchar_t*)msg);
		}else{
			Sleep(100);
			::SHChangeNotify(SHCNE_ASSOCCHANGED,SHCNF_FLUSH,NULL,NULL);	//関連付けの変更をシェルに通知(変更されていないかもしれないが簡便のため)
		}
	}else{
		//テンポラリINIを削除
		if(!m_strAssistINI.IsEmpty()){
			::DeleteFile(m_strAssistINI);
		}
	}

	if(bRet)EndDialog(nID);
}

void CConfigDialog::OnCancel(UINT uNotifyCode, int nID, HWND hWndCtl)
{
	EndDialog(nID);
}

LRESULT CConfigDialog::OnTreeSelect(LPNMHDR pnmh)
{
	if(pnmh->hwndFrom==SelectTreeView){
		HTREEITEM hItem=SelectTreeView.GetSelectedItem();
		if(!hItem)return 0;

		HWND hSelectedDialogWnd = (HWND)SelectTreeView.GetItemData(hItem);
		if (hSelectedDialogWnd && hSelectedDialogWnd != hActiveDialogWnd) {
			SetRedraw(FALSE);
			::ShowWindow(hActiveDialogWnd,SW_HIDE);
			//ScrollWindow.SetClient(NULL);
			ScrollWindow.SetClient(hSelectedDialogWnd);
			::ShowWindow(hSelectedDialogWnd,SW_SHOW);
			hActiveDialogWnd = hSelectedDialogWnd;
			SetRedraw(TRUE);
			RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_FRAME | RDW_UPDATENOW | RDW_ALLCHILDREN);
		}
	}
	return 0;
}

void CConfigDialog::OnSize(UINT, CSize&)
{
	SetMsgHandled(false);
	PostMessage(WM_USER_WM_SIZE);
}

LRESULT CConfigDialog::OnUserSize(UINT, WPARAM, LPARAM, BOOL& bHandled)
{
	//プロパティシートを貼り付けるためのスクロールコンテナの配置場所を取得
	CStatic StaticFrame;
	StaticFrame=GetDlgItem(IDC_STATIC_FRAME);
	RECT rect;
	StaticFrame.GetWindowRect(&rect);
	ScreenToClient(&rect);

	//スクロールコンテナを移動
	ScrollWindow.MoveWindow(&rect);
	return 0;
}

//LFAssist.exeの要請カウントを操作
void CConfigDialog::RequireAssistant()
{
	m_nAssistRequireCount++;
	Button_SetElevationRequiredState(GetDlgItem(IDOK),m_nAssistRequireCount);
}

void CConfigDialog::UnrequireAssistant()
{
	ASSERT(m_nAssistRequireCount>0);
	if(m_nAssistRequireCount>0){
		m_nAssistRequireCount--;
		Button_SetElevationRequiredState(GetDlgItem(IDOK),m_nAssistRequireCount);
	}
}
