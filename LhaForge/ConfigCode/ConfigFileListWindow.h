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
#include "FileListWindow/MenuCommand.h"

struct CConfigFileListWindow:public IConfigIO{
public:
	int Width;					//ウィンドウの幅
	int Height;					//ウィンドウの高さ
	int TreeWidth;				//ツリービューの幅

	int SortColumn;				//どのカラムでソートするか
	bool SortDescending;		//ソートの昇順・降順
	int ListStyle;				//リストビューの形式

	bool StoreSetting;			//ウィンドウの設定を保存

	int WindowPos_x;			//ウィンドウの座標(x)
	int WindowPos_y;			//ウィンドウの座標(y)
	bool StoreWindowPosition;	//ウィンドウの位置を保存する

	bool IgnoreMeaninglessPath;	//空白や.のみのパス指定は無視する
	FILELISTMODE FileListMode;	//ファイル表示の階層モード
	bool ExpandTree;			//起動時にツリービューを展開しておく
	bool DisplayFileSizeInByte;	//バイト単位でファイルサイズを表記する
	bool DisplayPathOnly;		//フルパスの欄にファイル名を表示しない
	int ColumnOrderArray[FILEINFO_ITEM_COUNT];	//リストビューカラムの並び順
	int ColumnWidthArray[FILEINFO_ITEM_COUNT];	//リストビューカラムの幅
	bool ExitWithEscape;		//[ESC]キーで終了
	bool DisableTab;			//タブ表示を使わないならTRUE
	bool KeepSingleInstance;	//ウィンドウを一つに保つならTRUE
	bool DenyPathExt;			//%PATHEXT%で指定されたファイルを開かないならTRUE

	std::wstring strCustomToolbarImage;	//カスタムツールバー画像
	bool ShowToolbar;			//ツールバーを表示するならTRUE
	bool ShowTreeView;			//ツリービューを表示するならTRUE

	struct tagOpenAssoc{
		virtual ~tagOpenAssoc() {}
		std::wstring Accept;
		std::wstring Deny;
	}OpenAssoc;

	std::vector<CLFMenuCommandItem> MenuCommandArray;	//「プログラムで開く」のコマンド
protected:
	void loadMenuCommand(const CConfigManager&);
	void storeMenuCommand(CConfigManager&)const;

	void load_sub(const CConfigManager&);
	void store_sub(CConfigManager&)const;
public:
	virtual ~CConfigFileListWindow(){}
	virtual void load(const CConfigManager &Config) {
		load_sub(Config);
		loadMenuCommand(Config);
	}
	virtual void store(CConfigManager& Config)const {
		store_sub(Config);
		storeMenuCommand(Config);
	}
};

