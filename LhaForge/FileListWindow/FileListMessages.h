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



//---イベント
#define WM_FILELIST_MODELCHANGED	(WM_APP+100)	//モデルが変更になった
#define WM_FILELIST_ARCHIVE_LOADED	(WM_APP+101)	//新規に構造を解析
#define WM_FILELIST_NEWCONTENT		(WM_APP+102)	//別フォルダへ移動したなど
#define WM_FILELIST_UPDATED			(WM_APP+103)	//並び順などが変更に

//---コマンド
#define WM_FILELIST_REFRESH			(WM_APP+150)	//再度解析コマンド
#define WM_FILELIST_OPEN_BY_PROPNAME	(WM_APP+151)	//ファイル閲覧依頼コマンド


//---イベント
#define WM_FILELIST_WND_STATE_CHANGED		(WM_APP+200)	//ファイル一覧ウィンドウの状態が変化した

#define WM_LHAFORGE_FILELIST_ACTIVATE_FILE		(WM_APP+300)
