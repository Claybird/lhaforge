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

struct CConfigFileListWindow:public IConfigConverter{
public:
	int Width;					//ウィンドウの幅
	int Height;					//ウィンドウの高さ
	int TreeWidth;				//ツリービューの幅

	int SortColumn;				//どのカラムでソートするか
	BOOL SortDescending;		//ソートの昇順・降順
	int ListStyle;				//リストビューの形式

	BOOL StoreSetting;			//ウィンドウの設定を保存

	int WindowPos_x;			//ウィンドウの座標(x)
	int WindowPos_y;			//ウィンドウの座標(y)
	BOOL StoreWindowPosition;	//ウィンドウの位置を保存する

	BOOL IgnoreMeaninglessPath;	//空白や.のみのパス指定は無視する
	FILELISTMODE FileListMode;	//ファイル表示の階層モード
	BOOL ExpandTree;			//起動時にツリービューを展開しておく
	BOOL DisplayFileSizeInByte;	//バイト単位でファイルサイズを表記する
	BOOL DisplayPathOnly;		//フルパスの欄にファイル名を表示しない
	int ColumnOrderArray[FILEINFO_ITEM_COUNT];	//リストビューカラムの並び順
	int ColumnWidthArray[FILEINFO_ITEM_COUNT];	//リストビューカラムの幅
	BOOL ExitWithEscape;		//[ESC]キーで終了
	BOOL DisableTab;			//タブ表示を使わないならTRUE
	BOOL KeepSingleInstance;	//ウィンドウを一つに保つならTRUE
	BOOL DenyPathExt;			//%PATHEXT%で指定されたファイルを開かないならTRUE

	CString strCustomToolbarImage;	//カスタムツールバー画像
	BOOL ShowToolbar;			//ツールバーを表示するならTRUE
	BOOL ShowTreeView;			//ツリービューを表示するならTRUE

	struct tagOpenAssoc{
		virtual ~tagOpenAssoc(){}
		CString Accept;
		CString Deny;
	}OpenAssoc;

	std::vector<CMenuCommandItem> MenuCommandArray;	//「プログラムで開く」のコマンド
protected:
	virtual void load(CONFIG_SECTION&);	//設定をCONFIG_SECTIONから読み込む
	virtual void store(CONFIG_SECTION&)const;	//設定をCONFIG_SECTIONに書き込む
	void loadMenuCommand(CONFIG_SECTION&,CMenuCommandItem&);
	void storeMenuCommand(CONFIG_SECTION&,const CMenuCommandItem&)const;
public:
	virtual ~CConfigFileListWindow(){}
	virtual void load(CConfigManager&);
	virtual void store(CConfigManager&)const;
};

