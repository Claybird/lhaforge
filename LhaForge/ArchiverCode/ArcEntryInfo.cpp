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
#include "ArcEntryInfo.h"
#include "../Utilities/FileOperation.h"

ARCHIVE_ENTRY_INFO_TREE* ARCHIVE_ENTRY_INFO_TREE::GetChild(size_t idx)const
{
	if(0<=idx && idx<GetNumChildren()){
		return childrenArray[idx];
	}else return NULL;
}

ARCHIVE_ENTRY_INFO_TREE* ARCHIVE_ENTRY_INFO_TREE::GetChild(LPCTSTR lpName)const
{
	CString strTmp=lpName;
	strTmp.MakeLower();
	DICT::const_iterator ite=childrenDict.find((LPCTSTR)strTmp);
	if(ite!=childrenDict.end()){
		return (*ite).second;
	}else return NULL;
}

void ARCHIVE_ENTRY_INFO_TREE::EnumFiles(std::list<CString> &rFileList)const
{
	if(bDir){
		size_t size=childrenArray.size();
		for(size_t i=0;i<size;i++){
			childrenArray[i]->EnumFiles(rFileList);
		}
	}
	if(!strFullPath.IsEmpty()){
		rFileList.push_back(strFullPath);
	}
}


//---------------
void ArcEntryInfoTree_GetNodePathRelative(const ARCHIVE_ENTRY_INFO_TREE* lpNode,const ARCHIVE_ENTRY_INFO_TREE* lpBase,CString &strPath)
{
	//ここで面倒なことをしているのは、
	//lpBaseがフルパス名を持っていない(=LhaForgeが仮想的に作り出したフォルダ)ことがあるから。
	strPath=_T("");

	for(; lpNode->lpParent && lpBase!=lpNode ;lpNode=lpNode->lpParent){
		CString strTmp=lpNode->strTitle;
		strTmp.Replace(_T('/'),_T('\\'));

		CPath strBuffer=strTmp;
		if(-1!=strTmp.Find(_T('\\'))){
			//ディレクトリノードの名前にパス区切り文字が入るのは、階層構造を無視した一覧の時のみ。
			//このときは、パス区切りを全部飛ばす必要がある。
			strBuffer.StripPath();	//一番最後の部分を残して、パスをカット
			strBuffer.RemoveBackslash();	//パス区切り文字を一旦削除
		}
		if(lpNode->bDir){
			strBuffer.AddBackslash();
		}
		strPath.Insert(0,strBuffer);
	}
	// 出力パスの修正
	UtilModifyPath(strPath);
}

