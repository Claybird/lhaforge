/*
 * Copyright (c) 2005-, Claybird
 * All rights reserved.

 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:

 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Claybird nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.

 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
 * THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
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

