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
#include "Dlg_assoc.h"
#include "../configwnd.h"
#include "../../Utilities/StringUtil.h"

#define ICONINDEX_EXTERNAL_SINGLE	21

//ASTYPE=TAR,LZH,etc.
//EXT=.lzh,.tar,etc.
#define LOAD_ASSOC_AND_SET_ICON(TYPE,EXT,INDEX,INDEX2)	{\
	AssocSettings[ASSOC_##TYPE].DefaultIconIndex=INDEX;\
	AssocSettings[ASSOC_##TYPE].DefaultIconIndex_Ex=INDEX2;\
	AssocSettings[ASSOC_##TYPE].Picture_Icon=GetDlgItem(IDC_STATIC_ASSOCIATION_##TYPE);\
	AssocSettings[ASSOC_##TYPE].Button_SetIcon=GetDlgItem(IDC_BUTTON_CHANGE_ICON_##TYPE);\
	AssocSettings[ASSOC_##TYPE].Check_SetAssoc=GetDlgItem(IDC_CHECK_ASSOCIATION_##TYPE);\
	AssocSettings[ASSOC_##TYPE].AssocInfo.Ext=EXT;\
	AssocSettings[ASSOC_##TYPE].Check_SetAssoc.SetCheck(FALSE);\
	if(AssocGetAssociation(AssocSettings[ASSOC_##TYPE].AssocInfo)){\
		AssocSettings[ASSOC_##TYPE].SetIconFromAssoc(Icon_SystemDefault);/*関連付け情報からアイコンを取得する*/\
		AssocSettings[ASSOC_##TYPE].CheckAssociation(m_strAssocDesired);	/*関連付け情報が正しいかどうかチェック*/\
		if(AssocSettings[ASSOC_##TYPE].bChanged)mr_ConfigDlg.RequireAssistant();	/*関連付け情報が誤っているのでLFAssistを要請*/\
		if(!AssocSettings[ASSOC_##TYPE].AssocInfo.bOrgStatus){\
			AssocSettings[ASSOC_##TYPE].Button_SetIcon.EnableWindow(false);\
			AssocSettings[ASSOC_##TYPE].Check_SetAssoc.SetCheck(FALSE);\
		}\
		else{\
			AssocSettings[ASSOC_##TYPE].Check_SetAssoc.SetCheck(TRUE);\
		}\
	}\
	else{\
		AssocSettings[ASSOC_##TYPE].Picture_Icon.SetIcon(Icon_SystemDefault);\
		AssocSettings[ASSOC_##TYPE].Button_SetIcon.EnableWindow(false);\
	}\
}

//--------------------------------------------

//ピクチャボックスにアイコンを指定
LRESULT CConfigDlgAssociation::OnInitDialog(HWND hWnd, LPARAM lParam)
{
	// メッセージループにメッセージフィルタとアイドルハンドラを追加
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	pLoop->AddMessageFilter(this);

	// 関連付けのShellOpenCommandを算出
	{
		TCHAR szModule[_MAX_PATH+1];
		GetModuleFileName(GetModuleHandle(NULL), szModule, _MAX_PATH);	//本体のパス取得
		CPath fullPath;
		try {
			fullPath = UtilGetCompletePathName(szModule).c_str();	//パスを正規化
		} catch (LF_EXCEPTION) {
			fullPath = szModule;
		}
		fullPath.QuoteSpaces();
		m_strAssocDesired=(LPCTSTR)fullPath;
		//m_strAssocDesired.MakeLower();	//小文字に正規化
		m_strAssocDesired+=_T(" /m \"%1\"");	//パラメータ
		TRACE(_T("ShellOpenCommand_Desired=%s\n"),m_strAssocDesired);
	}

	// システムデフォルトアイコンを取得
	if(Icon_SystemDefault.IsNull()){
		TCHAR Path[_MAX_PATH+1];
		FILL_ZERO(Path);
		GetSystemDirectory(Path,_MAX_PATH);
		PathAppend(Path,_T("shell32.dll"));
		Icon_SystemDefault.ExtractIcon(Path,0);
	}

	// 関連付け情報をチェック
	LOAD_ASSOC_AND_SET_ICON(LZH,	_T(".lzh"),	0,	0);
	LOAD_ASSOC_AND_SET_ICON(LZS,	_T(".lzs"),	0,	22);
	LOAD_ASSOC_AND_SET_ICON(LHA,	_T(".lha"),	0,	23);
	LOAD_ASSOC_AND_SET_ICON(ZIP,	_T(".zip"),	1,	1);
	LOAD_ASSOC_AND_SET_ICON(JAR,	_T(".jar"),	2,	2);
	LOAD_ASSOC_AND_SET_ICON(CAB,	_T(".cab"),	3,	3);
	LOAD_ASSOC_AND_SET_ICON(7Z,		_T(".7z"),	4,	4);
	LOAD_ASSOC_AND_SET_ICON(ARJ,	_T(".arj"),	14,	14);
	LOAD_ASSOC_AND_SET_ICON(RAR,	_T(".rar"),	5,	5);
	LOAD_ASSOC_AND_SET_ICON(JAK,	_T(".jak"),	10,	10);
	LOAD_ASSOC_AND_SET_ICON(GCA,	_T(".gca"),	6,	6);
	LOAD_ASSOC_AND_SET_ICON(IMP,	_T(".imp"),	17,	17);
	LOAD_ASSOC_AND_SET_ICON(ACE,	_T(".ace"),	13,	13);
	LOAD_ASSOC_AND_SET_ICON(YZ1,	_T(".yz1"),	12,	12);
	LOAD_ASSOC_AND_SET_ICON(HKI,	_T(".hki"),	11,	11);
	LOAD_ASSOC_AND_SET_ICON(BZA,	_T(".bza"),	15,	15);
	LOAD_ASSOC_AND_SET_ICON(GZA,	_T(".gza"),	16,	16);
	LOAD_ASSOC_AND_SET_ICON(ISH,	_T(".ish"),	18,	18);
	LOAD_ASSOC_AND_SET_ICON(UUE,	_T(".uue"),	19,	19);
	LOAD_ASSOC_AND_SET_ICON(BEL,	_T(".bel"),	20,	20);

	LOAD_ASSOC_AND_SET_ICON(TAR,	_T(".tar"),	7,	24);
	LOAD_ASSOC_AND_SET_ICON(GZ,		_T(".gz"),	7,	25);
	LOAD_ASSOC_AND_SET_ICON(TGZ,	_T(".tgz"),	7,	26);
	LOAD_ASSOC_AND_SET_ICON(BZ2,	_T(".bz2"),	7,	27);
	LOAD_ASSOC_AND_SET_ICON(TBZ,	_T(".tbz"),	7,	28);
	LOAD_ASSOC_AND_SET_ICON(XZ,		_T(".xz"),	7,	36);
	LOAD_ASSOC_AND_SET_ICON(TAR_XZ,	_T(".txz"),	7,	37);
	LOAD_ASSOC_AND_SET_ICON(LZMA,	_T(".lzma"),7,	38);
	LOAD_ASSOC_AND_SET_ICON(TAR_LZMA,_T(".tlz"),7,	39);
	LOAD_ASSOC_AND_SET_ICON(Z,		_T(".z"),	7,	29);
	LOAD_ASSOC_AND_SET_ICON(TAZ,	_T(".taz"),	7,	30);
	LOAD_ASSOC_AND_SET_ICON(CPIO,	_T(".cpio"),7,	31);
	LOAD_ASSOC_AND_SET_ICON(A,		_T(".a"),	7,	32);
	LOAD_ASSOC_AND_SET_ICON(LIB,	_T(".lib"),	9,	9);
	LOAD_ASSOC_AND_SET_ICON(RPM,	_T(".rpm"),	8,	33);
	LOAD_ASSOC_AND_SET_ICON(DEB,	_T(".deb"),	8,	34);
	LOAD_ASSOC_AND_SET_ICON(ISO,	_T(".iso"),	8,	35);
	return TRUE;
}


LRESULT CConfigDlgAssociation::OnCheckAssoc(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if(BN_CLICKED==wNotifyCode){
		for(int i=0;i<COUNTOF(AssocSettings);i++){
			if(hWndCtl==AssocSettings[i].Check_SetAssoc){
				BOOL bEnabled=AssocSettings[i].Check_SetAssoc.GetCheck();
				AssocSettings[i].Button_SetIcon.EnableWindow(bEnabled);
				if(!bEnabled){
					AssocSettings[i].SetIconFromAssoc(Icon_SystemDefault);
				}
				//LFAssist.exeの実行を要請
				mr_ConfigDlg.RequireAssistant();
				break;
			}
		}
	}
	return 0;
}


LRESULT CConfigDlgAssociation::OnChangeIcon(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if(BN_CLICKED==wNotifyCode){
		for(int i=0;i<COUNTOF(AssocSettings);i++){
			if(hWndCtl==AssocSettings[i].Button_SetIcon){
				CIconSelectDialog isd(AssocSettings[i].AssocInfo);
				if(IDOK==isd.DoModal()){
					TRACE(_T("IconFile=%s\n"),AssocSettings[i].AssocInfo.IconFile);
					AssocSettings[i].SetIcon(AssocSettings[i].AssocInfo.IconFile,AssocSettings[i].AssocInfo.IconIndex);
					AssocSettings[i].bChanged=true;
					//LFAssist.exeの実行を要請
					mr_ConfigDlg.RequireAssistant();
				}
				break;
			}
		}
	}
	return 0;
}

LRESULT CConfigDlgAssociation::OnSetAssoc(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if(BN_CLICKED!=wNotifyCode){
		return 0;
	}

	TCHAR ResourcePath[_MAX_PATH+1];	//アイコンファイル名
	int IconIndex=-1;
	if(IDC_BUTTON_ASSOC_SET_DEFAULT_ICON==wID||IDC_BUTTON_ASSOC_SET_DEFAULT_ICON_SINGLE==wID){
		//--------------
		// 標準アイコン
		//--------------
		FILL_ZERO(ResourcePath);
		GetModuleFileName(GetModuleHandle(NULL), ResourcePath, _MAX_PATH);	//本体のパス取得
		//EXEのパスを元にDLLのファイル名を組み立てる
		PathRemoveFileSpec(ResourcePath);
		PathAppend(ResourcePath,CString(MAKEINTRESOURCE(IDS_ICON_FILE_NAME_DEFAULT)));
	}else if(IDC_BUTTON_ASSOC_SET_EXTERNAL_ICON==wID||IDC_BUTTON_ASSOC_SET_EXTERNAL_ICON_SINGLE==wID){
		//------------------------------------
		// 外部のアイコンファイルをセットする
		//------------------------------------

		//アイコンを選択させる(デフォルトは標準アイコン)
		ASSOCINFO ac;
		CIconSelectDialog isd(ac);
		if(IDOK!=isd.DoModal()){
			return 0;
		}
		TRACE(_T("IconFile=%s\n"),ac.IconFile);
		_tcsncpy_s(ResourcePath,ac.IconFile,_MAX_PATH);
		IconIndex=ac.IconIndex;
		if(IDC_BUTTON_ASSOC_SET_EXTERNAL_ICON_SINGLE==wID&&-1==IconIndex){
			//アイコンが選択されていない
			return 0;
		}
	}
	for(int i=0;i<COUNTOF(AssocSettings);i++){
		switch(wID){
		case IDC_BUTTON_ASSOC_CHECK_TO_DEFAULT:	//標準の関連付け
			if(-1==index_of(NO_DEFAULT_ASSOCS, COUNTOF(NO_DEFAULT_ASSOCS), i)){
				AssocSettings[i].Button_SetIcon.EnableWindow(TRUE);
				AssocSettings[i].Check_SetAssoc.SetCheck(TRUE);
			}
			break;
		case IDC_BUTTON_ASSOC_CHECK_ALL:
			AssocSettings[i].Button_SetIcon.EnableWindow(TRUE);
			AssocSettings[i].Check_SetAssoc.SetCheck(TRUE);
			break;
		case IDC_BUTTON_ASSOC_UNCHECK_ALL:
			AssocSettings[i].Button_SetIcon.EnableWindow(FALSE);
			AssocSettings[i].Check_SetAssoc.SetCheck(FALSE);
			AssocSettings[i].SetIconFromAssoc(Icon_SystemDefault);
			break;
		case IDC_BUTTON_ASSOC_SET_DEFAULT_ICON:	//FALLTHROUGH
		case IDC_BUTTON_ASSOC_SET_EXTERNAL_ICON:
			//全て標準アイコンに/全て外部アイコンに
			{
				//アイコン数の取得
				long IconCount=(long)ExtractIcon(GetModuleHandle(NULL),ResourcePath,-1);
				const bool bExtraIcon=(IconCount>=35);	//アイコンの数が35より多ければ全て識別するタイプのアイコンだということになる
				if(AssocSettings[i].Check_SetAssoc.GetCheck()){
					AssocSettings[i].AssocInfo.IconIndex=bExtraIcon?AssocSettings[i].DefaultIconIndex_Ex:AssocSettings[i].DefaultIconIndex;
					AssocSettings[i].AssocInfo.IconFile=ResourcePath;
					AssocSettings[i].SetIcon(AssocSettings[i].AssocInfo.IconFile,AssocSettings[i].AssocInfo.IconIndex);
					AssocSettings[i].bChanged=true;
				}
			}
			break;
		case IDC_BUTTON_ASSOC_SET_DEFAULT_ICON_SINGLE:
			//全て標準単一アイコンに
			if(AssocSettings[i].Check_SetAssoc.GetCheck()){
				AssocSettings[i].AssocInfo.IconIndex=ICONINDEX_EXTERNAL_SINGLE;
				AssocSettings[i].AssocInfo.IconFile=ResourcePath;
				AssocSettings[i].SetIcon(AssocSettings[i].AssocInfo.IconFile,AssocSettings[i].AssocInfo.IconIndex);
				AssocSettings[i].bChanged=true;
			}
			break;
		case IDC_BUTTON_ASSOC_SET_EXTERNAL_ICON_SINGLE:
			//全て外部単一アイコンに
			if(AssocSettings[i].Check_SetAssoc.GetCheck()){
				AssocSettings[i].AssocInfo.IconIndex=IconIndex;
				AssocSettings[i].AssocInfo.IconFile=ResourcePath;
				AssocSettings[i].SetIcon(AssocSettings[i].AssocInfo.IconFile,AssocSettings[i].AssocInfo.IconIndex);
				AssocSettings[i].bChanged=true;
			}
			break;
		}
	}
	//LFAssist.exeの実行を要請
	mr_ConfigDlg.RequireAssistant();
	return 0;
}

LRESULT CConfigDlgAssociation::OnApply()
{
	std::wstring strIniName = mr_ConfigDlg.GetAssistantFile();

	CConfigManager tmp;
	tmp.setPath(strIniName);
	try {
		tmp.load();
		for (int i = 0; i < COUNTOF(AssocSettings); i++) {
			//レジストリ変更の必要がある場合
			bool Checked = (0 != AssocSettings[i].Check_SetAssoc.GetCheck());
			if ((AssocSettings[i].AssocInfo.bOrgStatus^Checked) || AssocSettings[i].bChanged) {
				if (AssocSettings[i].AssocInfo.bOrgStatus && !Checked) {
					//関連づけ解除要請
					tmp.setValue(AssocSettings[i].AssocInfo.Ext.operator LPCWSTR(), L"set", 0);
				} else {
					//関連づけ要請
					tmp.setValue(AssocSettings[i].AssocInfo.Ext.operator LPCWSTR(), L"set", L"1");
					tmp.setValue(AssocSettings[i].AssocInfo.Ext.operator LPCWSTR(), L"iconfile", AssocSettings[i].AssocInfo.IconFile);
					tmp.setValue(AssocSettings[i].AssocInfo.Ext.operator LPCWSTR(), L"iconindex", AssocSettings[i].AssocInfo.IconIndex);
				}
			}
		}
		tmp.save();
	} catch (const LF_EXCEPTION &e) {
		ErrorMessage(e.what());
	}

	return TRUE;
}


//==================================================================================

CIconSelectDialog::CIconSelectDialog(ASSOCINFO &ai)
{
	AssocInfo=&ai;
	if(AssocInfo->IconFile.IsEmpty()){
		//EXEのパスを元にDLLのファイル名を組み立てる
		CPath strResourcePath(UtilGetModuleDirectoryPath().c_str());
		strResourcePath+=CString(MAKEINTRESOURCE(IDS_ICON_FILE_NAME_DEFAULT));
		IconPath=(CString)strResourcePath;
		TRACE(_T("Set Default Icon Path\n"));
	}
	else{
		IconPath=AssocInfo->IconFile;
	}
}

LRESULT CIconSelectDialog::OnInitDialog(HWND hWnd, LPARAM lParam)
{
	ASSERT(AssocInfo);
	CenterWindow();
	ListView=GetDlgItem(IDC_LIST_ICON);
	//DDX情報アップデート
	DoDataExchange(FALSE);

	UpdateIcon();
	ListView.SetItemState(AssocInfo->IconIndex,LVIS_SELECTED,1);

	// ダイアログリサイズ初期化
	DlgResize_Init(true, true, WS_THICKFRAME | WS_CLIPCHILDREN);
	return TRUE;
}

void CIconSelectDialog::OnOK(UINT uNotifyCode, int nID, HWND hWndCtl)
{
	if(!DoDataExchange(TRUE))return;
	AssocInfo->IconFile=IconPath;
	const int ItemCount=ListView.GetItemCount();
	for(int i=0;i<ItemCount;i++){
		if(ListView.GetItemState(i,LVIS_SELECTED)){
			AssocInfo->IconIndex=i;
			break;
		}
	}
	EndDialog(nID);
}

void CIconSelectDialog::OnBrowse(UINT uNotifyCode, int nID, HWND hWndCtl)
{
	auto filter = UtilMakeFilterString(
		L"Icon File|*.dll;*.exe;*.ico;*.ocx;*.cpl;*.vbx;*.scr;*.icl|"
		L"All Files|*.*");

	if(!DoDataExchange(TRUE))return;
	CFileDialog dlg(TRUE, NULL, IconPath, OFN_HIDEREADONLY | OFN_NOCHANGEDIR, filter.c_str());
	if(IDCANCEL==dlg.DoModal()){	//キャンセル
		return;
	}

	IconPath=dlg.m_szFileName;
	DoDataExchange(FALSE);
	UpdateIcon();
}

void CIconSelectDialog::OnBrowseDefault(UINT uNotifyCode, int nID, HWND hWndCtl)
{
	TCHAR ResourcePath[_MAX_PATH+1];
	FILL_ZERO(ResourcePath);
	GetModuleFileName(GetModuleHandle(NULL), ResourcePath, _MAX_PATH);	//本体のパス取得
	//EXEのパスを元にDLLのファイル名を組み立てる
	PathRemoveFileSpec(ResourcePath);
	PathAppend(ResourcePath,CString(MAKEINTRESOURCE(IDS_ICON_FILE_NAME_DEFAULT)));
	IconPath=ResourcePath;

	DoDataExchange(FALSE);
	UpdateIcon();
}

bool CIconSelectDialog::UpdateIcon()
{
	ListView.DeleteAllItems();
	IconList.Destroy();
	//アイコン数の取得
	long IconCount=(long)ExtractIcon(GetModuleHandle(NULL),IconPath,-1);
	if(0==IconCount){
		ListView.EnableWindow(false);
		return false;
	}
	ListView.EnableWindow(true);
	IconList.Create(32,32,ILC_COLOR32 | ILC_MASK,IconCount,1);
	for(long i=0;i<IconCount;i++){
		CIcon Icon;
		Icon.ExtractIcon(IconPath,i);
		IconList.AddIcon(Icon);
	}
	ListView.SetImageList(IconList,LVSIL_NORMAL);
	for(long i=0;i<IconCount;i++){
		ListView.AddItem(i,0,_T(""),i);
	}
	return true;
}
