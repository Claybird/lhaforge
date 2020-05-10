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
#include "ConfigManager.h"
#include "../FileListWindow/FileListModel.h"
#include "../FileListWindow/FileListFrame.h"
#include "../FileListWindow/FileListTabClient.h"
#include "../Utilities/StringUtil.h"
#include "ConfigFileListWindow.h"
#include "../resource.h"

void CConfigFileListWindow::load(CONFIG_SECTION &Config)
{
	//ウィンドウの設定を保存
	StoreSetting=Config.Data[_T("StoreSetting")].GetNParam(TRUE);
	//ウィンドウの幅と高さ
	Width=Config.Data[_T("Width")].GetNParam(FILELISTWINDOW_DEFAULT_WIDTH);
	if(Width<0){
		Width=FILELISTWINDOW_DEFAULT_WIDTH;
	}

	Height=Config.Data[_T("Height")].GetNParam(FILELISTWINDOW_DEFAULT_HEIGHT);
	if(Height<0){
		Height=FILELISTWINDOW_DEFAULT_HEIGHT;
	}

	//ツリービューの幅
	TreeWidth=Config.Data[_T("TreeWidth")].GetNParam(FILELISTWINDOW_DEFAULT_TREE_WIDTH);
	if(TreeWidth<0){
		TreeWidth=175;
	}
	//リストビューのスタイル(Default:LVS_ICON)
	ListStyle=Config.Data[_T("ListStyle")].GetNParam(0);
	if(
		(LVS_LIST!=ListStyle)&&
		(LVS_REPORT!=ListStyle)&&
		(LVS_SMALLICON!=ListStyle)&&
		(LVS_ICON!=ListStyle)
	)ListStyle=LVS_ICON;

	//---------
	//ソートの設定
	//カラム
	SortColumn=Config.Data[_T("SortColumn")].GetNParam(FILEINFO_INVALID,FILEINFO_LAST_ITEM,FILEINFO_FILENAME);
	//昇順/降順
	SortDescending=Config.Data[_T("Descending")].GetNParam(TRUE);
	//----------
	//ウィンドウの位置を保存
	StoreWindowPosition=Config.Data[_T("StoreWindowPosition")].GetNParam(FALSE);
	//ウィンドウの座標
	WindowPos_x=Config.Data[_T("x")].GetNParam(0);
	WindowPos_y=Config.Data[_T("y")].GetNParam(0);
	//---------
	//無意味なパスを無視するか
	IgnoreMeaninglessPath=Config.Data[_T("IgnoreMeaninglessPath")].GetNParam(TRUE);
	//階層構造を無視するか
	FileListMode=(FILELISTMODE)Config.Data[_T("FileListMode")].GetNParam(0,FILELISTMODE_LAST_ITEM,0);
	//---------
	//初めからツリーを展開
	ExpandTree=Config.Data[_T("ExpandTree")].GetNParam(FALSE);
	//バイト単位でファイルサイズを表記
	DisplayFileSizeInByte = Config.Data[_T("DisplayFileSizeInByte")].GetNParam(FALSE);
	//フルパスの欄にファイル名を表示しない
	DisplayPathOnly = Config.Data[_T("DisplayPathOnly")].GetNParam(FALSE);
	//[ESC]キーで終了
	ExitWithEscape=Config.Data[_T("ExitWithEscape")].GetNParam(FALSE);
	//タブ表示を使わないならTRUE
	DisableTab=Config.Data[_T("DisableTab")].GetNParam(FALSE);
	//ウィンドウを一つに保つならTRUE
	KeepSingleInstance=Config.Data[_T("KeepSingleInstance")].GetNParam(FALSE);

	//リストビューカラムの並び順
	{
		for(int i=0;i<FILEINFO_ITEM_COUNT;i++){
			//配列初期化
			ColumnOrderArray[i]=i;
		}
		CString Buffer=Config.Data[_T("ColumnOrder")];
		if(!Buffer.IsEmpty()){
			//カラムの並び順を取得
			std::vector<int> numArr = UtilStringToIntArray((const wchar_t*)Buffer);
			//並び順のチェック
			for(int idx = 0; idx < min((int)numArr.size(), FILEINFO_ITEM_COUNT); idx++){
				int columnPosition = numArr[idx];
				if(columnPosition<0)columnPosition = -1;
				if(columnPosition >= FILEINFO_ITEM_COUNT){
					columnPosition = idx;
				}
				ColumnOrderArray[idx] = columnPosition;
			}
		}
	}
	//リストビューカラムの幅
	{
		//配列初期化
		for(int i=0;i<COUNTOF(ColumnWidthArray);i++){
			ColumnWidthArray[i]=-1;
		}
		CString Buffer=Config.Data[_T("ColumnWidth")];
		//カラムの幅を取得
		std::vector<int> numArr = UtilStringToIntArray((const wchar_t*)Buffer);
		for(int idx = 0; idx < min((int)numArr.size(), FILEINFO_ITEM_COUNT); idx++){
			ColumnWidthArray[idx] = numArr[idx];
		}
	}

	//関連付けで開くを許可/拒否する拡張子
	if(has_key(Config.Data,_T("OpenAssocAccept"))){
		OpenAssoc.Accept=Config.Data[_T("OpenAssocAccept")];
	}else{
		OpenAssoc.Accept=CString(MAKEINTRESOURCE(IDS_FILELIST_OPENASSOC_DEFAULT_ACCEPT));
	}

	//拒否
	if(has_key(Config.Data,_T("OpenAssocDeny"))){
		OpenAssoc.Deny=Config.Data[_T("OpenAssocDeny")];
	}else{
		OpenAssoc.Deny=CString(MAKEINTRESOURCE(IDS_FILELIST_OPENASSOC_DEFAULT_DENY));
	}

	//%PATHEXT%で指定されたファイルを開かないならTRUE
	DenyPathExt=Config.Data[_T("DenyPathExt")].GetNParam(TRUE);

	//カスタムツールバー画像
	strCustomToolbarImage=Config.Data[_T("CustomToolbarImage")];
	//ツールバー表示/非表示
	ShowToolbar=Config.Data[_T("ShowToolbar")].GetNParam(TRUE);

	//ツリービュー表示/非表示
	ShowTreeView=Config.Data[_T("ShowTreeView")].GetNParam(TRUE);
}

void CConfigFileListWindow::loadMenuCommand(CONFIG_SECTION &Config,CMenuCommandItem &mci)
{
	//「プログラムで開く」メニューのコマンド
	//プログラムのパス
	mci.Path=Config.Data[_T("Path")];
	//パラメータ
	mci.Param=Config.Data[_T("Param")];
	//ディレクトリ
	mci.Dir=Config.Data[_T("Dir")];
	//キャプション
	mci.Caption=Config.Data[_T("Caption")];
}


void CConfigFileListWindow::store(CONFIG_SECTION &Config)const
{
	//ウィンドウの設定を保存
	Config.Data[_T("StoreSetting")]=StoreSetting;
	if(StoreSetting){
		//ウィンドウの幅と高さ
		Config.Data[_T("Width")]=Width;
		Config.Data[_T("Height")]=Height;
		//ツリービューの幅
		Config.Data[_T("TreeWidth")]=TreeWidth;
		//リストビューのスタイル
		Config.Data[_T("ListStyle")]=ListStyle;
		//---------
		//ソートの設定
		//カラム
		Config.Data[_T("SortColumn")]=SortColumn;
		//昇順/降順
		Config.Data[_T("Descending")]=SortDescending;
	}
	//----------
	//ウィンドウの位置を保存
	Config.Data[_T("StoreWindowPosition")]=StoreWindowPosition;
	if(StoreWindowPosition){
		//ウィンドウの座標
		Config.Data[_T("x")]=WindowPos_x;
		Config.Data[_T("y")]=WindowPos_y;
	}
	//---------
	//無意味なパスを無視するか
	Config.Data[_T("IgnoreMeaninglessPath")]=IgnoreMeaninglessPath;
	//階層構造を無視するか
	Config.Data[_T("FileListMode")]=FileListMode;
	//---------
	//初めからツリーを展開
	Config.Data[_T("ExpandTree")]=ExpandTree;
	//バイト単位でファイルサイズを表記
	Config.Data[_T("DisplayFileSizeInByte")]=DisplayFileSizeInByte;
	//フルパスの欄にファイル名を表示しない
	Config.Data[_T("DisplayPathOnly")]=DisplayPathOnly;
	//[ESC]キーで終了
	Config.Data[_T("ExitWithEscape")]=ExitWithEscape;
	//タブ表示を使わないならTRUE
	Config.Data[_T("DisableTab")]=DisableTab;
	//ウィンドウを一つに保つならTRUE
	Config.Data[_T("KeepSingleInstance")]=KeepSingleInstance;

	//リストビューカラムの並び順
	{
		CString Buffer;
		for(int i=0;i<FILEINFO_ITEM_COUNT;i++){
			Buffer.AppendFormat(_T("%d"),ColumnOrderArray[i]);
			if(i!=FILEINFO_ITEM_COUNT-1){
				Buffer+=_T(",");
			}
		}
		Config.Data[_T("ColumnOrder")]=Buffer;
	}
	//リストビューカラムの幅
	{
		CString Buffer;
		for(int i=0;i<FILEINFO_ITEM_COUNT;i++){
			Buffer.AppendFormat(_T("%d"),ColumnWidthArray[i]);
			if(i!=FILEINFO_ITEM_COUNT-1){
				Buffer+=_T(",");
			}
		}
		Config.Data[_T("ColumnWidth")]=Buffer;
	}
	//関連付けで開くを許可/拒否する拡張子
	//許可
	Config.Data[_T("OpenAssocAccept")]=OpenAssoc.Accept;
	//拒否
	Config.Data[_T("OpenAssocDeny")]=OpenAssoc.Deny;

	//%PATHEXT%で指定されたファイルを開かないならTRUE
	Config.Data[_T("DenyPathExt")]=DenyPathExt;

	//カスタムツールバー画像
	Config.Data[_T("CustomToolbarImage")]=strCustomToolbarImage;
	//ツールバー表示/非表示
	Config.Data[_T("ShowToolbar")]=ShowToolbar;
	//ツリービュー表示/非表示
	Config.Data[_T("ShowTreeView")]=ShowTreeView;
}

void CConfigFileListWindow::storeMenuCommand(CONFIG_SECTION &Config,const CMenuCommandItem &mci)const
{
	//プログラムのパス
	Config.Data[_T("Path")]=mci.Path;
	//パラメータ
	Config.Data[_T("Param")]=mci.Param;
	//ディレクトリ
	Config.Data[_T("Dir")]=mci.Dir;
	//キャプション
	Config.Data[_T("Caption")]=mci.Caption;
}

void CConfigFileListWindow::load(CConfigManager &ConfMan)
{
	load(ConfMan.GetSection(_T("FileListWindow")));

	//「プログラムで開く」メニューのコマンド
	//古い情報の破棄
	MenuCommandArray.clear();
	for(UINT iIndex=0;iIndex<USERAPP_MAX_NUM;iIndex++){
		CString strSectionName;
		strSectionName.Format(_T("UserApp%d"),iIndex);
		if(!ConfMan.HasSection(strSectionName)){
			break;
		}else{
			CMenuCommandItem mci;
			loadMenuCommand(ConfMan.GetSection(strSectionName),mci);
			MenuCommandArray.push_back(mci);
		}
	}
}

void CConfigFileListWindow::store(CConfigManager &ConfMan)const
{
	store(ConfMan.GetSection(_T("FileListWindow")));

	//「プログラムで開く」メニューのコマンド
	//---古いセクションの破棄
	for(UINT iIndex=0;iIndex<USERAPP_MAX_NUM;iIndex++){
		CString strSectionName;
		strSectionName.Format(_T("UserApp%d"),iIndex);
		if(!ConfMan.HasSection(strSectionName)){
			break;
		}else{
			ConfMan.DeleteSection(strSectionName);
		}
	}
	//---データの上書き
	for(UINT iIndex=0;iIndex<MenuCommandArray.size();iIndex++){
		CString strSectionName;
		strSectionName.Format(_T("UserApp%d"),iIndex);
		storeMenuCommand(ConfMan.GetSection(strSectionName),MenuCommandArray[iIndex]);
	}
}

