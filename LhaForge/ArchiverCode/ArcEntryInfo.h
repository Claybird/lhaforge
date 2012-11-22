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

//統合アーカイバプロジェクト仕様のファイル属性
#define FA_RDONLY		0x01			// 書き込み保護属性
#define FA_HIDDEN		0x02			// 隠し属性
#define FA_SYSTEM		0x04			// システム属性
#define FA_LABEL		0x08			// ボリューム・ラベル
#define FA_DIREC		0x10			// ディレクトリ
#define FA_ARCH			0x20			// アーカイブ属性
#define FA_UNKNOWN		0x40			// 不明な属性(LhaForgeの独自拡張)


//フォルダの識別文字列(拡張子)
const LPCTSTR FOLDER_EXTENSION_STRING=_T("***");

//アーカイブ中のファイル/フォルダのエントリを保持
struct ARCHIVE_ENTRY_INFO{	//ファイルアイテム情報保持
	virtual ~ARCHIVE_ENTRY_INFO(){}

	CString			strFullPath;	//格納されたときの名前
	CString			strExt;			//ファイル拡張子
	int				nAttribute;		//属性;自分がフォルダかどうかなどの情報
	CString			strMethod;		//圧縮メソッド
	WORD			wRatio;			//圧縮率
	DWORD			dwCRC;			//CRC
	LARGE_INTEGER	llOriginalSize;		//格納ファイルの圧縮前のサイズ(ディレクトリなら、中に入っているファイルサイズの合計)
	LARGE_INTEGER	llCompressedSize;	//格納ファイルの圧縮後のサイズ(ディレクトリなら、中に入っているファイルサイズの合計)
	FILETIME		cFileTime;		//格納ファイル最終更新日時

	bool bSafe;
};

struct ARCHIVE_ENTRY_INFO_TREE:public ARCHIVE_ENTRY_INFO{
	virtual ~ARCHIVE_ENTRY_INFO_TREE(){}
	typedef std::hash_map<stdString,ARCHIVE_ENTRY_INFO_TREE*> DICT;
	std::vector<ARCHIVE_ENTRY_INFO_TREE*> childrenArray;
	DICT					childrenDict;
	ARCHIVE_ENTRY_INFO_TREE	*lpParent;
	CString					strTitle;
	bool					bDir;			//ディレクトリかどうか

	size_t GetNumChildren()const{return childrenArray.size();}
	ARCHIVE_ENTRY_INFO_TREE* GetChild(size_t idx)const;
	ARCHIVE_ENTRY_INFO_TREE* GetChild(LPCTSTR lpName)const;
	void Clear(){
		lpParent=NULL;
		childrenArray.clear();
		childrenDict.clear();

		nAttribute=-1;
		wRatio=0xFFFF;
		dwCRC=-1;
		llOriginalSize.HighPart=-1;
		llOriginalSize.LowPart=-1;
		llCompressedSize.HighPart=-1;
		llCompressedSize.LowPart=-1;
		cFileTime.dwLowDateTime=-1;
		cFileTime.dwHighDateTime=-1;
		bSafe=true;
	}

	//自分以下のファイルを列挙
	void EnumFiles(std::list<CString> &rFileList)const;
};

//ルートからみて自分までのパスを取得
void ArcEntryInfoTree_GetNodePathRelative(const ARCHIVE_ENTRY_INFO_TREE* lpDir,const ARCHIVE_ENTRY_INFO_TREE* lpBase,CString &strPath);
