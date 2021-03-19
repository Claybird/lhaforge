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
#include "FileListView.h"
#include "../resource.h"
#include "Dialogs/LogListDialog.h"
#include "../ConfigCode/ConfigFileListWindow.h"
#include "../Utilities/StringUtil.h"
#include "Dialogs/TextInputDlg.h"
#include "CommonUtil.h"
#include <sstream>

CFileListView::CFileListView(CFileListModel& rModel, const CConfigFileListWindow &r_confFLW):
	CFileViewBase(rModel,r_confFLW),
	m_bDisplayFileSizeInByte(false),
	m_bPathOnly(false)
{
	SetFrameWnd(m_hFrameWnd);
}

LRESULT CFileListView::OnCreate(LPCREATESTRUCT lpcs)
{
	LRESULT lRes=DefWindowProc();
	//SetFont(AtlGetDefaultGuiFont());

	// イメージリスト作成
	m_ShellDataManager.Init();
	SetImageList(m_ShellDataManager.GetImageList(true),LVSIL_NORMAL);
	SetImageList(m_ShellDataManager.GetImageList(false),LVSIL_SMALL);

	//カラムヘッダのソートアイコン
	m_SortImageList.CreateFromImage(MAKEINTRESOURCE(IDB_BITMAP_SORTMARK),16,1,CLR_DEFAULT,IMAGE_BITMAP,LR_CREATEDIBSECTION);

	mr_Model.addEventListener(m_hWnd);
	return lRes;
}

LRESULT CFileListView::OnDestroy()
{
/*	CConfigFileListWindow ConfFLW;
	ConfFLW.load(mr_Config);

	//ウィンドウ設定の保存
	if( ConfFLW.StoreSetting ) {
		//リストビューのスタイル
		ConfFLW.ListStyle = GetWindowLong(GWL_STYLE) % ( 0x0004 );

		ConfFLW.FileListMode = mr_Model.GetListMode();
		//カラムの並び順・カラムの幅
		GetColumnState(ConfFLW.ColumnOrderArray, ConfFLW.ColumnWidthArray);

		ConfFLW.store(mr_Config);
		CString strErr;
		if( !mr_Config.SaveConfig(strErr) ) {
			ErrorMessage(strErr);
		}
	}
	*/

	mr_Model.removeEventListener(m_hWnd);
	return 0;
}



bool CFileListView::SetColumnState(const int* pColumnOrderArray, const int *pFileInfoWidthArray)
{
	//既存のカラムを削除
	if(GetHeader().IsWindow()){
		int nCount=GetHeader().GetItemCount();
		for(;nCount>0;nCount--){
			DeleteColumn(nCount-1);
		}
	}

//========================================
//      リストビューにカラム追加
//========================================
//リストビューにカラムを追加するためのマクロ
#define ADD_COLUMNITEM(x,width,pos) \
{if(-1!=index_of(pColumnOrderArray,FILEINFO_ITEM_COUNT,FILEINFO_##x)){\
	int nIndex=InsertColumn(FILEINFO_##x, CString(MAKEINTRESOURCE(IDS_FILELIST_COLUMN_##x)), pos, width,-1);\
	if(nIndex<0||nIndex>=FILEINFO_ITEM_COUNT)return false;\
	m_ColumnIndexArray[nIndex]=FILEINFO_##x;\
}}

	m_ColumnIndexArray.fill(-1);

	//ファイル名
	ADD_COLUMNITEM(FILENAME,100,LVCFMT_LEFT);
	//フルパス名
	ADD_COLUMNITEM(FULLPATH,200,LVCFMT_LEFT);
	//ファイルサイズ
	ADD_COLUMNITEM(ORIGINALSIZE,90,LVCFMT_RIGHT);
	//ファイル種類
	ADD_COLUMNITEM(TYPENAME,120,LVCFMT_LEFT);
	//更新日時
	ADD_COLUMNITEM(FILETIME,120,LVCFMT_LEFT);
	//属性
	ADD_COLUMNITEM(ATTRIBUTE,60,LVCFMT_LEFT);
	//圧縮後サイズ
	ADD_COLUMNITEM(COMPRESSEDSIZE,90,LVCFMT_RIGHT);
	//圧縮メソッド
	ADD_COLUMNITEM(METHOD,60,LVCFMT_LEFT);
	//圧縮率
	ADD_COLUMNITEM(RATIO,60,LVCFMT_RIGHT);
	//CRC
	ADD_COLUMNITEM(CRC,60,LVCFMT_LEFT);


	//----------------------
	// カラムの並び順を設定
	//----------------------
	int Count=0;
	for(;Count<FILEINFO_ITEM_COUNT;Count++){
		//有効なアイテム数を求める
		if(-1==pColumnOrderArray[Count])break;
	}
	//並び順を変換
	int TemporaryArray[FILEINFO_ITEM_COUNT];

	for(int i=0;i<FILEINFO_ITEM_COUNT;i++){
		TemporaryArray[i]=pColumnOrderArray[i];
	}
	for(int i=0;i<Count;i++){
		int nIndex=index_of(m_ColumnIndexArray,TemporaryArray[i]);
		ASSERT(-1!=nIndex);
		if(-1!=nIndex){
			TemporaryArray[i]=nIndex;
		}
	}
	SetColumnOrderArray(Count,TemporaryArray);
	for( int i = 0; i < Count; i++ ) {
		SetColumnWidth(TemporaryArray[i], pFileInfoWidthArray[i]);
	}

	//カラムヘッダのソートアイコン
	if(GetHeader().IsWindow()){
		GetHeader().SetImageList(m_SortImageList);
	}
	UpdateSortIcon();
	return true;
}

void CFileListView::GetColumnState(int* pColumnOrderArray, int *pFileInfoWidthArray)
{
	//カラムの並び順取得
	const int nCount=GetHeader().GetItemCount();
	ASSERT(nCount<=FILEINFO_ITEM_COUNT);

	int TemporaryArray[FILEINFO_ITEM_COUNT];
	memset(TemporaryArray,-1,sizeof(TemporaryArray));
	GetColumnOrderArray(nCount,TemporaryArray);
	//並び順を変換
	memset(pColumnOrderArray,-1,FILEINFO_ITEM_COUNT*sizeof(int));
	for(int i=0;i<nCount;i++){
		pColumnOrderArray[i]=m_ColumnIndexArray[TemporaryArray[i]];
	}

	for( int i = 0; i < nCount; i++ ) {
		pFileInfoWidthArray[i] = GetColumnWidth(TemporaryArray[i]);
	}
}

std::vector<const ARCHIVE_ENTRY_INFO*> CFileListView::GetSelectedItems()
{
	std::vector<const ARCHIVE_ENTRY_INFO*> items;
	int nIndex=-1;
	for(;;){
		nIndex = GetNextItem(nIndex, LVNI_ALL | LVNI_SELECTED);
		if(-1==nIndex)break;
		const ARCHIVE_ENTRY_INFO* lpNode=mr_Model.GetFileListItemByIndex(nIndex);

		ASSERT(lpNode);

		items.push_back(lpNode);
	}
	return items;
}

LRESULT CFileListView::OnFileListNewContent(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	TRACE(__FUNCTIONW__ _T("\n"));

	DeleteAllItems();
	SetItemCount(0);
	if(mr_Model.IsOK()){
		auto lpCurrent = mr_Model.getCurrentDir();
		ASSERT(lpCurrent);
		if(lpCurrent){
			SetItemCount(lpCurrent->getNumChildren());
			SetItemState(0,LVIS_FOCUSED,LVIS_FOCUSED);
		}
	}

	//ファイルドロップの受け入れ:受け入れ拒否はDragOverが行う
	EnableDropTarget(true);
	return 0;
}

LRESULT CFileListView::OnFileListUpdated(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	TRACE(__FUNCTIONW__ _T("\n"));

	Invalidate();

	return 0;
}

LRESULT CFileListView::OnDblClick(LPNMHDR pnmh)
{
	TRACE(__FUNCTIONW__ _T("\n"));
	//----------------------------------------------------------------------
	//選択アイテムが複数の時:
	// 選択を関連付けで開く
	//Shiftが押されていた時:
	// 選択を関連付けで開く
	//選択アイテムが一つだけの時:
	// フォルダをダブルクリックした/Enterを押した場合にはそのフォルダを開く
	// 選択がファイルだった場合には関連付けで開く
	//----------------------------------------------------------------------

	//選択アイテムが複数の時
	//もしShiftが押されていたら、関連付けで開く
	if(GetKeyState(VK_SHIFT)<0||GetSelectedCount()>=2){
		auto files = extractItemToTemporary(false, GetSelectedItems());
		openAssociation(files);
		return 0;
	} else {
		//選択されたアイテムを取得
		auto items = GetSelectedItems();
		if (items.empty())return 0;
		auto lpNode = items.front();

		if (lpNode->is_directory()) {
			mr_Model.MoveDownDir(lpNode);
		} else {
			auto files = extractItemToTemporary(false, GetSelectedItems());
			openAssociation(files);
		}
	}
	return 0;
}


//カラム表示のOn/Offを切り替える
//表示中:該当カラムを非表示にし、配列を詰める
//非表示:使われていない部分に指定カラムを追加
void _ToggleColumn(int *lpArray,size_t size,FILEINFO_TYPE type)
{
	ASSERT(lpArray);
	if(!lpArray)return;

	for(size_t i=0;i<size;i++){
		if(type==lpArray[i]){
			//配列を詰める
			for(size_t j=i;j<size-1;j++){
				lpArray[j]=lpArray[j+1];
			}
			lpArray[size-1]=-1;
			return;
		}
		else if(-1==lpArray[i]){
			lpArray[i]=type;
			return;
		}
	}
}


//カラムヘッダを左/右クリック
LRESULT CFileListView::OnColumnRClick(int /*idCtrl*/, LPNMHDR pnmh, BOOL& bHandled)
{
	if(pnmh->hwndFrom!=GetHeader()){
		//メッセージは処理しなかったことにする
		bHandled = FALSE;
		return 0;
	}

	//右クリックメニュー表示
	POINT point;
	GetCursorPos(&point);
	CMenu cMenu;
	cMenu.LoadMenu(IDR_LISTVIEW_HEADER_MENU);
	CMenuHandle cSubMenu(cMenu.GetSubMenu(0));

	//--------------------------------
	// 各メニューアイテムの有効・無効
	//--------------------------------

	int columnOrderArray[FILEINFO_ITEM_COUNT];
	int columnWidthArray[FILEINFO_ITEM_COUNT];
	GetColumnState(columnOrderArray, columnWidthArray);

	struct{
		FILEINFO_TYPE idx;
		UINT nMenuID;
	}menuTable[]={
		{FILEINFO_FULLPATH,			ID_MENUITEM_LISTVIEW_COLUMN_FULLPATH},
		{FILEINFO_ORIGINALSIZE,		ID_MENUITEM_LISTVIEW_COLUMN_ORIGINALSIZE},
		{FILEINFO_TYPENAME,			ID_MENUITEM_LISTVIEW_COLUMN_TYPENAME},
		{FILEINFO_FILETIME,			ID_MENUITEM_LISTVIEW_COLUMN_FILETIME},
		{FILEINFO_ATTRIBUTE,		ID_MENUITEM_LISTVIEW_COLUMN_ATTRIBUTE},
		{FILEINFO_COMPRESSEDSIZE,	ID_MENUITEM_LISTVIEW_COLUMN_COMPRESSEDSIZE},
		{FILEINFO_METHOD,			ID_MENUITEM_LISTVIEW_COLUMN_METHOD},
		{FILEINFO_RATIO,			ID_MENUITEM_LISTVIEW_COLUMN_RATIO},
		{FILEINFO_CRC,				ID_MENUITEM_LISTVIEW_COLUMN_CRC},
	};

	for(size_t i=0;i<COUNTOF(menuTable);i++){
		bool bEnabled=(-1!=index_of(columnOrderArray,COUNTOF(columnOrderArray),menuTable[i].idx));
		cSubMenu.CheckMenuItem(menuTable[i].nMenuID,MF_BYCOMMAND|(bEnabled?MF_CHECKED:MF_UNCHECKED));
	}

	//メニュー表示:選択したコマンドが返ってくる
	int nRet=cSubMenu.TrackPopupMenu(TPM_NONOTIFY|TPM_RETURNCMD|TPM_LEFTALIGN|TPM_RIGHTBUTTON,point.x, point.y, m_hWnd,NULL);
	if(0==nRet){
		//Not Selected
		return 0;
	}else if(ID_MENUITEM_LISTVIEW_COLUMN_RESET==nRet){
		//初期化
		for(size_t i=0;i<COUNTOF(columnOrderArray);i++){
			columnOrderArray[i]=i;
		}
	}else{
		for(size_t i=0;i<COUNTOF(menuTable);i++){
			if(menuTable[i].nMenuID==nRet){
				_ToggleColumn(columnOrderArray,COUNTOF(columnOrderArray),menuTable[i].idx);
			}
		}
	}

	SetColumnState(columnOrderArray, columnWidthArray);

	return 0;
}


//キー入力によるファイル検索
LRESULT CFileListView::OnFindItem(LPNMHDR pnmh)
{
	auto lpCurrent = mr_Model.getCurrentDir();
	ASSERT(lpCurrent);
	if(!lpCurrent)return -1;

	int iCount=lpCurrent->getNumChildren();
	if(iCount<=0)return -1;

	LPNMLVFINDITEM lpFindInfo = (LPNMLVFINDITEM)pnmh;
	int iStart=lpFindInfo->iStart;
	if(iStart<0)iStart=0;
	if(lpFindInfo->lvfi.flags & LVFI_STRING || lpFindInfo->lvfi.flags & LVFI_PARTIAL){	//ファイル名で検索
		LPCTSTR lpFindString=lpFindInfo->lvfi.psz;
		size_t nLength=_tcslen(lpFindString);
		//前方一致で検索
		for(int i=iStart;i<iCount;i++){
			auto lpNode=mr_Model.GetFileListItemByIndex(i);
			ASSERT(lpNode);
			if(0==_tcsnicmp(lpFindString,lpNode->_entryName.c_str(),nLength)){
				return i;
			}
		}
		if(lpFindInfo->lvfi.flags & LVFI_WRAP){
			for(int i=0;i<iStart;i++){
				auto lpNode=mr_Model.GetFileListItemByIndex(i);
				ASSERT(lpNode);
				if(0==_tcsnicmp(lpFindString,lpNode->_entryName.c_str(),nLength)){
					return i;
				}
			}
		}
		return -1;
	}else{
		return -1;
	}
}



//ソート
LRESULT CFileListView::OnSortItem(LPNMHDR pnmh)
{
	LPNMLISTVIEW lpNMLV=(LPNMLISTVIEW)pnmh;
	int iCol=lpNMLV->iSubItem;
	SortItem(iCol);
	return 0;
}


void CFileListView::SortItem(int iCol)
{
	if(!(iCol >= 0 && iCol < FILEINFO_ITEM_COUNT))return;

	if(iCol==mr_Model.GetSortKeyType()){
		if(mr_Model.GetSortMode()){
			mr_Model.SetSortMode(false);
		}else{	//ソート解除
			mr_Model.SetSortKeyType(FILEINFO_INVALID);
			mr_Model.SetSortMode(true);
		}
	}else{
		mr_Model.SetSortKeyType(iCol);
		mr_Model.SetSortMode(true);
	}

	UpdateSortIcon();
}

void CFileListView::UpdateSortIcon()
{
	if(!IsWindow())return;
	//ソート状態を元にカラムを変更する
	CHeaderCtrl hc=GetHeader();
	ASSERT(hc.IsWindow());
	if(hc.IsWindow()){
		int count=hc.GetItemCount();
		//アイコン解除
		for(int i=0;i<count;i++){
			HDITEM hdi={0};
			hdi.mask=HDI_FORMAT;
			hc.GetItem(i,&hdi);
			if((hdi.fmt & HDF_IMAGE)){
				hdi.fmt&=~HDF_IMAGE;
				hc.SetItem(i,&hdi);
			}
		}

		//アイコンセット
		int iCol=mr_Model.GetSortKeyType();
		if(iCol!=FILEINFO_INVALID && iCol<count){
			HDITEM hdi={0};
			hdi.mask=HDI_FORMAT;

			hc.GetItem(iCol,&hdi);
			hdi.mask|=HDI_FORMAT|HDI_IMAGE;
			hdi.fmt|=HDF_IMAGE|HDF_BITMAP_ON_RIGHT;
			hdi.iImage=mr_Model.GetSortMode() ? 0 : 1;
			hc.SetItem(iCol,&hdi);
		}
	}
}

//カスタムドロー
DWORD CFileListView::OnPrePaint(int nID, LPNMCUSTOMDRAW lpnmcd)
{
	if(lpnmcd->hdr.hwndFrom == m_hWnd)
		return CDRF_NOTIFYITEMDRAW;
	else
		return CDRF_DODEFAULT;
}

DWORD CFileListView::OnItemPrePaint(int nID, LPNMCUSTOMDRAW lpnmcd)
{
#pragma message("Need to think again if this was useful")
	/*if(lpnmcd->hdr.hwndFrom == m_hWnd){
		LPNMLVCUSTOMDRAW lpnmlv = (LPNMLVCUSTOMDRAW)lpnmcd;

		ARCHIVE_ENTRY_INFO* lpNode=mr_Model.GetFileListItemByIndex(lpnmcd->dwItemSpec);
		if(lpNode){
			if(!lpNode->bSafe){
				//危険なアーカイブなので色を付ける
				lpnmlv->clrText = RGB(255, 255, 255);
				lpnmlv->clrTextBk = RGB(255, 0, 0);
			}
		}
	}*/
	return CDRF_DODEFAULT;
}


//仮想リストビューのアイテム取得に反応
LRESULT CFileListView::OnGetDispInfo(LPNMHDR pnmh)
{
	LV_DISPINFO* pstLVDInfo=(LV_DISPINFO*)pnmh;

	auto lpNode=mr_Model.GetFileListItemByIndex(pstLVDInfo->item.iItem);
	//ASSERT(lpNode);
	if(!lpNode)return 0;

	//添え字チェック
	ASSERT(pstLVDInfo->item.iSubItem>=0 && pstLVDInfo->item.iSubItem<FILEINFO_ITEM_COUNT);
	if(pstLVDInfo->item.iSubItem<0||pstLVDInfo->item.iSubItem>=FILEINFO_ITEM_COUNT)return 0;

	CString strBuffer;
	LPCTSTR lpText=NULL;
	switch(m_ColumnIndexArray[pstLVDInfo->item.iSubItem]){
	case FILEINFO_FILENAME:	//ファイル名
		if(pstLVDInfo->item.mask & LVIF_TEXT)lpText=lpNode->_entryName.c_str();
		if(pstLVDInfo->item.mask & LVIF_IMAGE)pstLVDInfo->item.iImage=m_ShellDataManager.GetIconIndex(lpNode->getExt().c_str());
		break;
	case FILEINFO_FULLPATH:	//格納パス
		if(pstLVDInfo->item.mask & LVIF_TEXT){
			if(m_bPathOnly){
				//'\\'ではなく'/'でパスが区切られていればtrue
				bool bSlash = (std::wstring::npos!=lpNode->_fullpath.find(_T('/')));
				//一度置き換え
				strBuffer = lpNode->_fullpath.c_str();
				strBuffer.Replace(_T('/'),_T('\\'));

				//ファイル名除去
				std::filesystem::path buf = strBuffer.operator LPCWSTR();
				buf = buf.parent_path();
				if(buf.empty()){
					//何もなくなった、つまりルートディレクトリにある
					strBuffer = _T("\\");
				}else{
					strBuffer = UtilPathAddLastSeparator(buf).c_str();
				}

				if(bSlash){
					//元に戻す置き換え
					strBuffer.Replace(_T('\\'),_T('/'));
				}
				lpText = strBuffer;
			}else{
				lpText=lpNode->_fullpath.c_str();
			}
		}
		break;
	case FILEINFO_ORIGINALSIZE:	//サイズ(圧縮前)
		if(pstLVDInfo->item.mask & LVIF_TEXT){
			FormatFileSize(strBuffer,lpNode->_originalSize);
			lpText=strBuffer;
		}
		break;
	case FILEINFO_TYPENAME:	//ファイルタイプ
		if (pstLVDInfo->item.mask & LVIF_TEXT) {
			strBuffer = m_ShellDataManager.GetTypeName(lpNode->getExt().c_str()).c_str();
			lpText = strBuffer;
		}
		break;
	case FILEINFO_FILETIME:	//ファイル日時
		if(pstLVDInfo->item.mask & LVIF_TEXT){
			strBuffer = UtilFormatTime(lpNode->stat.st_mtime).c_str();
			lpText=strBuffer;
		}
		break;
	case FILEINFO_ATTRIBUTE:	//属性
	case FILEINFO_METHOD:	//圧縮メソッド
	case FILEINFO_RATIO:	//圧縮率
	case FILEINFO_CRC:	//CRC
	case FILEINFO_COMPRESSEDSIZE:	//サイズ(圧縮後)
		if (pstLVDInfo->item.mask & LVIF_TEXT) {
			lpText = L"";
		}
		break;
	}

	if(lpText){
		_tcsncpy_s(pstLVDInfo->item.pszText,pstLVDInfo->item.cchTextMax, lpText,pstLVDInfo->item.cchTextMax);
	}
	return 0;
}

//仮想リストビューのツールチップ取得に反応
LRESULT CFileListView::OnGetInfoTip(LPNMHDR pnmh)
{
	LPNMLVGETINFOTIP pGetInfoTip=(LPNMLVGETINFOTIP)pnmh;

	auto lpNode=mr_Model.GetFileListItemByIndex(pGetInfoTip->iItem);
	ASSERT(lpNode);
	if(!lpNode)return 0;
	CString strInfo;
	CString strBuffer;

	//ファイル名
	strInfo+=CString(MAKEINTRESOURCE(IDS_FILELIST_COLUMN_FILENAME));
	strInfo+=_T(" : ");	strInfo+=lpNode->_entryName.c_str();		strInfo+=_T("\n");
	//格納パス
	strInfo+=CString(MAKEINTRESOURCE(IDS_FILELIST_COLUMN_FULLPATH));
	strInfo+=_T(" : ");	strInfo+=lpNode->_fullpath.c_str();	strInfo+=_T("\n");
	//圧縮前サイズ
	FormatFileSize(strBuffer,lpNode->_originalSize);
	strInfo+=CString(MAKEINTRESOURCE(IDS_FILELIST_COLUMN_ORIGINALSIZE));
	strInfo+=_T(" : ");	strInfo+=strBuffer;		strInfo+=_T("\n");
	//ファイルタイプ
	strInfo+=CString(MAKEINTRESOURCE(IDS_FILELIST_COLUMN_TYPENAME));
	strInfo+=_T(" : ");	strInfo+=m_ShellDataManager.GetTypeName(lpNode->getExt().c_str()).c_str();	strInfo+=_T("\n");
	//ファイル日時
	strBuffer = UtilFormatTime(lpNode->stat.st_mtime).c_str();
	strInfo+=CString(MAKEINTRESOURCE(IDS_FILELIST_COLUMN_FILETIME));
	strInfo+=_T(" : ");	strInfo+=strBuffer;		strInfo+=_T("\n");

	if(lpNode->is_directory()){
		//--------------
		// ディレクトリ
		//--------------
		//子ノードの数を表示
		strInfo+=_T("\n");
		strInfo+=CString(MAKEINTRESOURCE(IDS_FILELIST_SUBITEM));
		strInfo+=_T(" : ");
		strInfo.AppendFormat(IDS_FILELIST_ITEMCOUNT,lpNode->getNumChildren());
	}

	//InfoTipに設定
	_tcsncpy_s(pGetInfoTip->pszText,pGetInfoTip->cchTextMax,strInfo,pGetInfoTip->cchTextMax);
	return 0;
}

void CFileListView::FormatFileSizeInBytes(CString &Info, UINT64 Size)
{
	CString format(MAKEINTRESOURCE(IDS_ORDERUNIT_BYTE));

	std::wstringstream ss;
	ss.imbue(std::locale(""));
	ss << Size;

	Info.Format(format,ss.str().c_str());
}

void CFileListView::FormatFileSize(CString &Info, UINT64 Size)
{
	bool bInByte=m_bDisplayFileSizeInByte;
	Info.Empty();
	if(-1==Size){
		Info=L"---";
		return;
	}

	static CString OrderUnit[]={
		MAKEINTRESOURCE(IDS_ORDERUNIT_BYTE),
		MAKEINTRESOURCE(IDS_ORDERUNIT_KILOBYTE),
		MAKEINTRESOURCE(IDS_ORDERUNIT_MEGABYTE),
		MAKEINTRESOURCE(IDS_ORDERUNIT_GIGABYTE),
		MAKEINTRESOURCE(IDS_ORDERUNIT_TERABYTE),
	};	//サイズの単位
	static const int MAX_ORDERUNIT=COUNTOF(OrderUnit);

	if(bInByte){	//ファイルサイズをバイト単位で表記する
		FormatFileSizeInBytes(Info,Size);
		return;
	}

	int Order=0;
	for(;Order<MAX_ORDERUNIT;Order++){
		if(Size<1024*1024){
			break;
		}
		Size = Size / 1024;
	}
	if(0==Order && Size<1024){
		//1KBに満たないのでバイト単位でそのまま表記
		FormatFileSizeInBytes(Info,Size);
	}else{
		TCHAR Buffer[64]={0};
		if(Order<MAX_ORDERUNIT-1){
			double SizeToDisplay = Size / 1024.0;
			Order++;
			Info.Format(OrderUnit[Order], SizeToDisplay);
		}else{
			//過大サイズ
			Info.Format(OrderUnit[Order], Size);
		}
	}
}

//-------

void CFileListView::OnAddItems(UINT uNotifyCode,int nID,HWND hWndCtrl)
{
	ASSERT(mr_Model.IsModifySupported());
	if(!mr_Model.IsModifySupported())return;

	auto dest = mr_Model.getCurrentDir();

	std::vector<std::filesystem::path> files;
	if(nID==ID_MENUITEM_ADD_FILE){		//ファイル追加
		const COMDLG_FILTERSPEC filter[] = {
			{ L"All Files", L"*.*" },
		};

		LFShellFileOpenDialog dlg(nullptr, FOS_FORCEFILESYSTEM | FOS_FILEMUSTEXIST | FOS_PATHMUSTEXIST | FOS_ALLOWMULTISELECT, nullptr, filter, COUNTOF(filter));
		if(IDOK==dlg.DoModal()){
			files = dlg.GetMultipleFiles();
		}
	}else{
		//add directory
		LFShellFileOpenDialog dlg(nullptr, FOS_FORCEFILESYSTEM | FOS_FILEMUSTEXIST | FOS_PATHMUSTEXIST | FOS_PICKFOLDERS);
		if(IDOK==dlg.DoModal()){
			CString path;
			dlg.GetFilePath(path);
			files.push_back(path.operator LPCWSTR());
		}
	}

	if(!files.empty()){
		AddItemsToDirectory(dest, files);
	}
}


//-----------------------------
// ドラッグ&ドロップによる解凍
//-----------------------------

LRESULT CFileListView::OnBeginDrag(LPNMHDR pnmh)
{
	if(!mr_Model.CheckArchiveExists()){	//存在しないならエラー
		return 0;
	}
	//選択されたアイテムを列挙
	auto items = GetSelectedItems();
	if(items.empty()){	//本来あり得ない
		ASSERT(!"This code cannot be run");
		return 0;
	}

	if(!mr_Model.ClearTempDir()){
		//テンポラリディレクトリを空に出来ない
		ErrorMessage(UtilLoadString(IDS_ERROR_CANT_CLEAR_TEMPDIR));
		return 0;
	}else{
		::EnableWindow(m_hFrameWnd,FALSE);

		//ドラッグ&ドロップで解凍
		ARCLOG arcLog;
		HRESULT hr=m_DnDSource.DoDragDrop(
			mr_Model,
			items,
			mr_Model.getCurrentDir(),
			mr_Model.getTempDir(),
			m_hFrameWnd,
			arcLog);
		if(FAILED(hr)){
			//TODO
			CLogListDialog LogDlg(L"Log");
			std::vector<ARCLOG> logs = { arcLog };
			LogDlg.SetLogArray(logs);
			LogDlg.DoModal(m_hFrameWnd);
		}

		::EnableWindow(m_hFrameWnd,TRUE);
		::SetForegroundWindow(m_hFrameWnd);
	}
	return 0;
}

void CFileListView::OnSelectAll(UINT,int,HWND)
{
	SetItemState(-1, LVIS_SELECTED, LVIS_SELECTED);
	SetFocus();
}


void CFileListView::OnCopyInfo(UINT uNotifyCode,int nID,HWND hWndCtrl)
{
	//選択されたアイテムを列挙
	auto items = GetSelectedItems();

	CString info;

	switch(nID){
	case ID_MENUITEM_COPY_FILENAME:
		for(const auto &item: items){
			info.AppendFormat(_T("%s\n"),(LPCTSTR)item->_entryName.c_str());
		}
		break;
	case ID_MENUITEM_COPY_PATH:
		for (const auto &item : items) {
			info.AppendFormat(_T("%s\n"), item->_fullpath.c_str());
		}
		break;
	case ID_MENUITEM_COPY_ORIGINAL_SIZE:
		for (const auto &item : items) {
			info.AppendFormat(_T("%I64d\n"), item->_originalSize);
		}
		break;
	case ID_MENUITEM_COPY_FILETYPE:
		for (const auto &item : items) {
			info.AppendFormat(_T("%s\n"),m_ShellDataManager.GetTypeName(item->getExt().c_str()));
		}
		break;
	case ID_MENUITEM_COPY_FILETIME:
		for (const auto &item : items) {
			CString strBuffer = UtilFormatTime(item->stat.st_mtime).c_str();
			info.AppendFormat(_T("%s\n"),(LPCTSTR)strBuffer);
		}
		break;
	case ID_MENUITEM_COPY_ATTRIBUTE:
	case ID_MENUITEM_COPY_COMPRESSED_SIZE:
	case ID_MENUITEM_COPY_METHOD:
	case ID_MENUITEM_COPY_COMPRESSION_RATIO:
	case ID_MENUITEM_COPY_CRC:
#pragma message("FIXME!")
		//TODO
		break;
	case ID_MENUITEM_COPY_ALL:
		info=_T("FileName\tFullPath\tOriginalSize\tFileType\tFileTime\tAttribute\tCompressedSize\tMethod\tCompressionRatio\tCRC\n");
		for (const auto &item : items) {
			CString strFileTime = UtilFormatTime(item->stat.st_mtime).c_str();

			info.AppendFormat(L"%s\t%s\t%I64d\t%s\t%s\n",
				(LPCTSTR)item->_entryName.c_str(),
				(LPCTSTR)item->_fullpath.c_str(),
				item->_originalSize,
				m_ShellDataManager.GetTypeName(item->getExt().c_str()),
				(LPCTSTR)strFileTime);
		}
		break;
	default:
		ASSERT(!"Unknown command");
	}
	//MessageBox(info);
	UtilSetTextOnClipboard((const wchar_t*)info);
}

void CFileListView::OnFindItem(UINT uNotifyCode,int nID,HWND hWndCtrl)
{
	if(ID_MENUITEM_FINDITEM_END==nID){
		mr_Model.EndFindItem();
	}else{
		CTextInputDialog dlg(CString(MAKEINTRESOURCE(IDS_INPUT_FIND_PARAM)));

		if(IDOK == dlg.DoModal()){
			mr_Model.EndFindItem();
			auto lpFound = mr_Model.FindItem(dlg.GetInputText(), mr_Model.getCurrentDir());
			mr_Model.setCurrentDir(lpFound);
		}
	}
}

void CFileListView::OnShowCustomizeColumn(UINT,int,HWND)
{
	//カラムヘッダ編集メニューを表示するため、カラムヘッダの右クリックをエミュレート
	BOOL bTemp;
	NMHDR nmhdr;
	nmhdr.hwndFrom=GetHeader();
	OnColumnRClick(0, &nmhdr, bTemp);
}
