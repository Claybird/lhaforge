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
#include "FileListWindow/FileListModel.h"
#include "FileListWindow/FileListFrame.h"
#include "FileListWindow/FileListTabClient.h"
#include "Utilities/StringUtil.h"
#include "Utilities/Utility.h"
#include "ConfigFileListWindow.h"
#include "resource.h"

void CConfigFileListWindow::load_sub(const CConfigManager& Config)
{
	const auto section = L"FileListWindow";
	//ウィンドウの設定を保存
	StoreSetting=Config.getBool(section, L"StoreSetting", true);
	//ウィンドウの幅と高さ
	Width = Config.getInt(section, L"Width", FILELISTWINDOW_DEFAULT_WIDTH);
	if(Width<0){
		Width=FILELISTWINDOW_DEFAULT_WIDTH;
	}

	Height=Config.getInt(section, L"Height", FILELISTWINDOW_DEFAULT_HEIGHT);
	if(Height<0){
		Height=FILELISTWINDOW_DEFAULT_HEIGHT;
	}

	//ツリービューの幅
	TreeWidth=Config.getInt(section, L"TreeWidth", FILELISTWINDOW_DEFAULT_TREE_WIDTH);
	if(TreeWidth<0){
		TreeWidth=175;
	}
	//リストビューのスタイル(Default:LVS_ICON)
	ListStyle=Config.getInt(section, L"ListStyle", 0);
	if(
		(LVS_LIST!=ListStyle)&&
		(LVS_REPORT!=ListStyle)&&
		(LVS_SMALLICON!=ListStyle)&&
		(LVS_ICON!=ListStyle)
	)ListStyle=LVS_ICON;

	//---------
	//ソートの設定
	//カラム
	SortColumn = Config.getIntRange(section, L"SortColumn", FILEINFO_INVALID, FILEINFO_LAST_ITEM, FILEINFO_FILENAME);
	//昇順/降順
	SortDescending = Config.getBool(section, L"Descending", true);
	//----------
	//ウィンドウの位置を保存
	StoreWindowPosition = Config.getBool(section, L"StoreWindowPosition", false);
	//ウィンドウの座標
	WindowPos_x = Config.getInt(section, L"x", 0);
	WindowPos_y = Config.getInt(section, L"y", 0);
	//---------
	//無意味なパスを無視するか
	IgnoreMeaninglessPath = Config.getBool(section, L"IgnoreMeaninglessPath", true);
	//---------
	//初めからツリーを展開
	ExpandTree=Config.getBool(section, L"ExpandTree", false);
	//バイト単位でファイルサイズを表記
	DisplayFileSizeInByte = Config.getBool(section, L"DisplayFileSizeInByte", false);
	//フルパスの欄にファイル名を表示しない
	DisplayPathOnly = Config.getBool(section, L"DisplayPathOnly", false);
	//[ESC]キーで終了
	ExitWithEscape=Config.getBool(section, L"ExitWithEscape", false);
	//タブ表示を使わないならTRUE
	DisableTab=Config.getBool(section, L"DisableTab", false);
	//ウィンドウを一つに保つならTRUE
	KeepSingleInstance=Config.getBool(section, L"KeepSingleInstance", false);

	//リストビューカラムの並び順
	{
		for(int i=0;i<FILEINFO_ITEM_COUNT;i++){
			//配列初期化
			ColumnOrderArray[i]=i;
		}
		auto buf=Config.getText(section, L"ColumnOrder", L"");
		if(!buf.empty()){
			//カラムの並び順を取得
			std::vector<int> numArr = UtilStringToIntArray(buf);
			//並び順のチェック
			for(int idx = 0; idx < std::min((int)numArr.size(), (int)FILEINFO_ITEM_COUNT); idx++){
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
		auto buf = Config.getText(section, L"ColumnWidth", L"");
		//カラムの幅を取得
		std::vector<int> numArr = UtilStringToIntArray(buf);
		for(int idx = 0; idx < std::min((int)numArr.size(), (int)FILEINFO_ITEM_COUNT); idx++){
			ColumnWidthArray[idx] = numArr[idx];
		}
	}

	//関連付けで開くを許可/拒否する拡張子
	OpenAssoc.Accept = Config.getText(section, L"OpenAssocAccept", UtilLoadString(IDS_FILELIST_OPENASSOC_DEFAULT_ACCEPT));

	//拒否
	OpenAssoc.Deny=Config.getText(section, L"OpenAssocDeny", UtilLoadString(IDS_FILELIST_OPENASSOC_DEFAULT_DENY));

	//%PATHEXT%で指定されたファイルを開かないならTRUE
	DenyPathExt=Config.getBool(section, L"DenyPathExt", true);

	//カスタムツールバー画像
	strCustomToolbarImage=Config.getText(section, L"CustomToolbarImage", L"");
	//ツールバー表示/非表示
	ShowToolbar=Config.getBool(section, L"ShowToolbar", true);

	//ツリービュー表示/非表示
	ShowTreeView=Config.getBool(section, L"ShowTreeView", true);
}

void CConfigFileListWindow::loadMenuCommand(const CConfigManager &Config)
{
	//「プログラムで開く」メニューのコマンド
	//古い情報の破棄
	MenuCommandArray.clear();
	for (int iIndex = 0; iIndex < USERAPP_MAX_NUM; iIndex++) {
		auto section = Format(L"UserApp%d", iIndex);
		if (!Config.hasSection(section)) {
			break;
		} else {
			CLFMenuCommandItem mci;

			//「プログラムで開く」メニューのコマンド
			//プログラムのパス
			mci.Path = Config.getText(section, L"Path", L"");
			//パラメータ
			mci.Param = Config.getText(section, L"Param", L"");
			//ディレクトリ
			mci.Dir = Config.getText(section, L"Dir", L"");
			//キャプション
			mci.Caption = Config.getText(section, L"Caption", L"");

			MenuCommandArray.push_back(mci);
		}
	}
}


void CConfigFileListWindow::store_sub(CConfigManager &Config)const
{
	const auto section = L"FileListWindow";
	//ウィンドウの設定を保存
	Config.setValue(section, L"StoreSetting", StoreSetting);
	if(StoreSetting){
		//ウィンドウの幅と高さ
		Config.setValue(section, L"Width", Width);
		Config.setValue(section, L"Height", Height);
		//ツリービューの幅
		Config.setValue(section, L"TreeWidth", TreeWidth);
		//リストビューのスタイル
		Config.setValue(section, L"ListStyle", ListStyle);
		//---------
		//ソートの設定
		//カラム
		Config.setValue(section, L"SortColumn", SortColumn);
		//昇順/降順
		Config.setValue(section, L"Descending", SortDescending);
	}
	//----------
	//ウィンドウの位置を保存
	Config.setValue(section, L"StoreWindowPosition", StoreWindowPosition);
	if(StoreWindowPosition){
		//ウィンドウの座標
		Config.setValue(section, L"x", WindowPos_x);
		Config.setValue(section, L"y", WindowPos_y);
	}
	//---------
	//無意味なパスを無視するか
	Config.setValue(section, L"IgnoreMeaninglessPath", IgnoreMeaninglessPath);
	//---------
	//初めからツリーを展開
	Config.setValue(section, L"ExpandTree", ExpandTree);
	//バイト単位でファイルサイズを表記
	Config.setValue(section, L"DisplayFileSizeInByte", DisplayFileSizeInByte);
	//フルパスの欄にファイル名を表示しない
	Config.setValue(section, L"DisplayPathOnly", DisplayPathOnly);
	//[ESC]キーで終了
	Config.setValue(section, L"ExitWithEscape", ExitWithEscape);
	//タブ表示を使わないならTRUE
	Config.setValue(section, L"DisableTab", DisableTab);
	//ウィンドウを一つに保つならTRUE
	Config.setValue(section, L"KeepSingleInstance", KeepSingleInstance);

	//リストビューカラムの並び順
	{
		std::wstring buf;
		for(int i=0;i<FILEINFO_ITEM_COUNT;i++){
			buf += Format(L"%d",ColumnOrderArray[i]);
			if(i!=FILEINFO_ITEM_COUNT-1){
				buf+=L",";
			}
		}
		Config.setValue(section, L"ColumnOrder", buf);
	}
	//リストビューカラムの幅
	{
		std::wstring buf;
		for(int i=0;i<FILEINFO_ITEM_COUNT;i++){
			buf += Format(L"%d",ColumnWidthArray[i]);
			if(i!=FILEINFO_ITEM_COUNT-1){
				buf+=L",";
			}
		}
		Config.setValue(section, L"ColumnWidth", buf);
	}
	//関連付けで開くを許可/拒否する拡張子
	//許可
	Config.setValue(section, L"OpenAssocAccept", OpenAssoc.Accept);
	//拒否
	Config.setValue(section, L"OpenAssocDeny", OpenAssoc.Deny);

	//%PATHEXT%で指定されたファイルを開かないならTRUE
	Config.setValue(section, L"DenyPathExt", DenyPathExt);

	//カスタムツールバー画像
	Config.setValue(section, L"CustomToolbarImage", strCustomToolbarImage);
	//ツールバー表示/非表示
	Config.setValue(section, L"ShowToolbar", ShowToolbar);
	//ツリービュー表示/非表示
	Config.setValue(section, L"ShowTreeView", ShowTreeView);
}

void CConfigFileListWindow::storeMenuCommand(CConfigManager &Config)const
{
	//「プログラムで開く」メニューのコマンド
	//---古いセクションの破棄
	for (int iIndex = 0; iIndex < USERAPP_MAX_NUM; iIndex++) {
		auto section = Format(L"UserApp%d", iIndex);
		if (!Config.hasSection(section)) {
			break;
		} else {
			Config.deleteSection(section);
		}
	}
	//---データの上書き
	for (size_t iIndex = 0; iIndex < MenuCommandArray.size(); iIndex++) {
		auto section = Format(L"UserApp%d", iIndex);
		const auto& mci = MenuCommandArray[iIndex];

		//プログラムのパス
		Config.setValue(section, L"Path", mci.Path);
		//パラメータ
		Config.setValue(section, L"Param", mci.Param);
		//ディレクトリ
		Config.setValue(section, L"Dir", mci.Dir);
		//キャプション
		Config.setValue(section, L"Caption", mci.Caption);
	}
}

//checks file extension whether file is allowed to be opened.
bool CConfigFileListWindow::isPathAcceptableToOpenAssoc(LPCTSTR lpszPath, bool bDenyOnly)const
{
	auto denyExt = OpenAssoc.Deny;
	if (DenyPathExt) {
		auto envs = UtilGetEnvInfo();
		denyExt += envs[L"PATHEXT"];
	}

	const auto denyList = UtilSplitString(denyExt, L";");
	for (const auto& deny : denyList) {
		if (UtilExtMatchSpec(lpszPath, deny)) {
			return false;
		}
	}
	if (bDenyOnly) {
		return true;
	} else {
		const auto acceptList = UtilSplitString(OpenAssoc.Accept, L";");
		for (const auto& accept : acceptList) {
			if (UtilExtMatchSpec(lpszPath, accept)) {
				return true;
			}
		}
	}
	return false;
}
