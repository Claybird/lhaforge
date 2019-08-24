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

#include "stdafx.h"
#include "DnDSource.h"
#include "../../Utilities/StringUtil.h"
#include "../../Dialogs/LogDialog.h"
#include "../FileListModel.h"

HRESULT COLEDnDSource::DragDrop(CFileListModel &rModel,const std::list<ARCHIVE_ENTRY_INFO_TREE*> &items,ARCHIVE_ENTRY_INFO_TREE *lpBase,LPCTSTR lpszOutputDir,CString &strLog)
{
	//DnD対象ファイル名を取得
	std::list<stdString> filesList;
	for(std::list<ARCHIVE_ENTRY_INFO_TREE*>::const_iterator ite=items.begin();items.end()!=ite;++ite){
		CString strNodePath;
		ArcEntryInfoTree_GetNodePathRelative(*ite,lpBase,strNodePath);

		CPath strPath=lpszOutputDir;
		strPath+=strNodePath;
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
				CDropSource *lpDropSource = new CDropSource(rModel,items,lpszOutputDir,lpBase);
				ret = ::DoDragDrop(lpDataObject, lpDropSource, DROPEFFECT_COPY,&dwEffect);
				if(DRAGDROP_S_DROP!=ret){
					if(ret==DRAGDROP_S_CANCEL){
						//解凍キャンセルの可能性がある
						if(!lpDropSource->_bRet){
							//解凍はキャンセルされた
							strLog=lpDropSource->_strLog;
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

	size_t bufsize=(strFiles.length()+1)*sizeof(strFiles[0]);
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

	wchar_t *buf = (wchar_t *)(&lpDropFile[1]);
	UtilMakeFilterString(strFiles.c_str(),buf,bufsize/sizeof(strFiles[0]));
	::GlobalUnlock(hDrop);

	return hDrop;
}
