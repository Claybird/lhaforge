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

//統合アーカイバプロジェクト仕様のファイル属性
#define FA_RDONLY		0x001			// 書き込み保護属性
#define FA_HIDDEN		0x002			// 隠し属性
#define FA_SYSTEM		0x004			// システム属性
#define FA_LABEL		0x008			// ボリューム・ラベル
#define FA_DIREC		0x010			// ディレクトリ
#define FA_ARCH			0x020			// アーカイブ属性
#define FA_ENCRYPTED	0x040			// パスワード保護されたファイル
#define FA_UNKNOWN		0x100			// 不明な属性(LhaForgeの独自拡張)


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
