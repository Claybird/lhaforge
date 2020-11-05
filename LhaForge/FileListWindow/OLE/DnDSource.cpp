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
#include "DnDSource.h"
#include "../../Utilities/StringUtil.h"
#include "../FileListModel.h"
#include "resource.h"

HRESULT COLEDnDSource::DragDrop(CFileListModel &rModel,const std::list<ARCHIVE_ENTRY_INFO*> &items,ARCHIVE_ENTRY_INFO *lpBase,LPCTSTR lpszOutputDir,CString &strLog)
{
	//DnD対象ファイル名を取得
	std::list<stdString> filesList;
	for(std::list<ARCHIVE_ENTRY_INFO*>::const_iterator ite=items.begin();items.end()!=ite;++ite){
		auto strNodePath = (*ite)->getRelativePath(lpBase);

		CPath strPath=lpszOutputDir;
		strPath+=strNodePath.c_str();
		strPath.RemoveBackslash();
		filesList.push_back((LPCTSTR)strPath);
	}
	//重複削除
	filesList.sort();
	filesList.unique();

	HRESULT hRes=E_OUTOFMEMORY;
	strLog=_T("E_OUTOFMEMORY");
	//CDataObjectを作成しCF_HDROP形式のデータを登録	
	CDataObject *lpDataObject = new CDataObject();
	if(lpDataObject->allocate(2)){
		HANDLE hObject = NULL;
		if((hObject = CreateHDrop(filesList)) != NULL){
			FORMATETC	fmt;
			STGMEDIUM	medium;
			DWORD		dwEffect;
			int ret;

			CreateMedium(CF_HDROP, hObject, &fmt, &medium);

			if(lpDataObject->SetData(&fmt, &medium, TRUE) == S_OK){	//解放はDataObjectに任せる
				std::vector<ARCHIVE_ENTRY_INFO*> itemsTmp(items.begin(), items.end());
				CLFDropSource *lpDropSource = new CLFDropSource(rModel,itemsTmp,lpszOutputDir,lpBase);
				ret = ::DoDragDrop(lpDataObject, lpDropSource, DROPEFFECT_COPY,&dwEffect);
				if(DRAGDROP_S_DROP!=ret){
					if(ret==DRAGDROP_S_CANCEL){
						//解凍キャンセルの可能性がある
						if(!lpDropSource->_bRet){
							//解凍はキャンセルされた
							strLog=lpDropSource->_strLog.c_str();
							hRes=E_ABORT;
						}else{
							//DnDそのものをキャンセルしたのでエラーではない
							hRes=S_OK;
						}
					}else{
						hRes=E_FAIL;
						strLog=CString(MAKEINTRESOURCE(IDS_ERROR_DND_FAILED));
						//DnDに失敗
					}
				}else hRes=S_OK;
				if(lpDropSource)lpDropSource->Release();

				//完了したので削除
				//:Windows98ではこのタイミングで削除するとファイルを見つけられずにエラーとなる
				//UtilDeleteDir(OutputDir);
			}
		}
		if(hObject)GlobalFree(hObject);
	}

	if(lpDataObject)lpDataObject->Release();
	return hRes;
}

void COLEDnDSource::CreateMedium(CLIPFORMAT cfFormat, HANDLE hObject, FORMATETC *pFormatetc, STGMEDIUM *pmedium)
{
	pFormatetc->cfFormat = cfFormat;
	pFormatetc->dwAspect = DVASPECT_CONTENT;
	pFormatetc->lindex = -1;
	pFormatetc->ptd = NULL;
	pFormatetc->tymed = TYMED_HGLOBAL;

	pmedium->hGlobal = hObject;
	pmedium->tymed = TYMED_HGLOBAL;
	pmedium->pUnkForRelease = NULL;
}

HDROP COLEDnDSource::CreateHDrop(const std::list<stdString> &filesList)
{
	stdString strFiles;
	for(std::list<stdString>::const_iterator ite=filesList.begin();ite!=filesList.end();++ite){
		strFiles += *ite;
		strFiles += _T('|');
	}
	strFiles += _T('|');
	strFiles += _T('|');

	auto buf = UtilMakeFilterString(strFiles.c_str());

	size_t bufsize = buf.size() * sizeof(strFiles[0]);
	HDROP hDrop = (HDROP)::GlobalAlloc(GHND,sizeof(DROPFILES) + bufsize);
	if(!hDrop){
		return NULL;
	}

	// 構造体の後ろにファイル名のリストをコピーする。(ファイル名\0ファイル名\0ファイル名\0\0\0)
	LPDROPFILES lpDropFile = (LPDROPFILES)::GlobalLock(hDrop);
	lpDropFile->pFiles = sizeof(DROPFILES);		// ファイル名のリストまでのオフセット
	lpDropFile->pt.x = 0;
	lpDropFile->pt.y = 0;
	lpDropFile->fNC = FALSE;
	lpDropFile->fWide = TRUE;					// ワイドキャラの場合は TRUE

	wchar_t *p = (wchar_t *)(&lpDropFile[1]);
	memcpy(p, &buf[0], bufsize);
	::GlobalUnlock(hDrop);

	return hDrop;
}
