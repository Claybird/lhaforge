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

//関連付け用API

#pragma once

const LPCTSTR ASSOC_PREFIX=_T("LhaForgeArchive_");


class ASSOCINFO{
public:
	CString Ext;	//拡張子;.付きの名前を渡すこと
//	CString FileType;
	CString OrgFileType;
	bool bOrgStatus;	//関連付けされていたらtrue

	CString OrgIconFile;
	int OrgIconIndex;

	CString IconFile;
	int IconIndex;

	CString ShellOpenCommand;
	ASSOCINFO():IconIndex(0),bOrgStatus(false){}
	virtual ~ASSOCINFO(){}
};

#ifdef __LFASSIST__
//関連付けを作成
bool AssocSetAssociation(const ASSOCINFO&);
//関連付けを削除
bool AssocDeleteAssociation(LPCTSTR Ext);
#endif//__LFASSIST__
//関連付けを取得
bool AssocGetAssociation(ASSOCINFO&);

#ifdef __LFASSIST__
//レジストリ再帰的削除
bool AssocRecursiveDeleteKey(HKEY,LPCTSTR);
bool AssocRegExistKey(HKEY,LPCTSTR);
#endif//__LFASSIST__

