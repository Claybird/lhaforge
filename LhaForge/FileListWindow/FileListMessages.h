/*
 * Copyright (c) 2005-2012, Claybird
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
