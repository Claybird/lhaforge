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
#include "DataObject.h"
#include "DropSource.h"
#include "Utilities/StringUtil.h"
#include "FileListWindow/FileListModel.h"
#include "resource.h"

class CFileListModel;
struct ARCHIVE_ENTRY_INFO;

class CLFDnDSource{
protected:
	HDROP createHDrop(const std::vector<std::wstring>& files) {
		std::wstring strFiles;
		for (const auto& file:files) {
			strFiles += file;
			strFiles += L'|';
		}
		strFiles += L'|';
		strFiles += L'|';

		auto buf = UtilMakeFilterString(strFiles);

		size_t bufsize = buf.size() * sizeof(buf[0]);
		HDROP hDrop = (HDROP)::GlobalAlloc(GHND, sizeof(DROPFILES) + bufsize);
		if (!hDrop) {
			return nullptr;
		}

		// copy file names after header (fileA\0fileB\0fileC\0\0\0)
		LPDROPFILES lpDropFile = (LPDROPFILES)::GlobalLock(hDrop);
		lpDropFile->pFiles = sizeof(DROPFILES);
		lpDropFile->pt.x = 0;
		lpDropFile->pt.y = 0;
		lpDropFile->fNC = FALSE;
		lpDropFile->fWide = TRUE;

		wchar_t *p = (wchar_t *)(&lpDropFile[1]);
		memcpy(p, &buf[0], bufsize);
		::GlobalUnlock(hDrop);

		return hDrop;
	}

	void createMedium(CLIPFORMAT cfFormat, HANDLE hObject, FORMATETC *pFormatetc, STGMEDIUM *pmedium) {
		pFormatetc->cfFormat = cfFormat;
		pFormatetc->dwAspect = DVASPECT_CONTENT;
		pFormatetc->lindex = -1;
		pFormatetc->ptd = NULL;
		pFormatetc->tymed = TYMED_HGLOBAL;

		pmedium->hGlobal = hObject;
		pmedium->tymed = TYMED_HGLOBAL;
		pmedium->pUnkForRelease = NULL;
	}
public:
	CLFDnDSource(){}
	virtual ~CLFDnDSource(){}
	HRESULT DoDragDrop(
		CFileListModel &rModel,
		const std::vector<ARCHIVE_ENTRY_INFO*>& items,
		ARCHIVE_ENTRY_INFO *lpBase,
		const std::wstring& outputDir,
		std::wstring &strLog)
	{
		std::vector<std::wstring> files;
		for (const auto& item: items) {
			std::filesystem::path path = outputDir;
			path /= item->getRelativePath(lpBase);
			files.push_back(UtilPathRemoveLastSeparator(path));
		}
		std::sort(files.begin(), files.end());
		files.erase(std::unique(files.begin(), files.end()), files.end());

		HRESULT hRes = E_OUTOFMEMORY;
		strLog = L"E_OUTOFMEMORY";

		CLFDnDDataObject dataObject;
		HANDLE hObject = createHDrop(files);
		if (hObject) {
			FORMATETC fmt;
			STGMEDIUM medium;
			createMedium(CF_HDROP, hObject, &fmt, &medium);

			if (dataObject.SetData(&fmt, &medium, TRUE) == S_OK) {
				std::vector<ARCHIVE_ENTRY_INFO*> itemsTmp(items.begin(), items.end());
				CLFDropSource *lpDropSource = new CLFDropSource(rModel, itemsTmp, outputDir, lpBase);
				DWORD dwEffect;
				auto ret = ::DoDragDrop(&dataObject, lpDropSource, DROPEFFECT_COPY, &dwEffect);
				if (DRAGDROP_S_DROP != ret) {
					if (ret == DRAGDROP_S_CANCEL) {
						//cancel?
						if (!lpDropSource->_bRet) {
							//extract aborted
							strLog = lpDropSource->_strLog.c_str();
							hRes = E_ABORT;
						} else {
							//cancelled Drag & Drop; not an error
							strLog = L"";
							hRes = S_OK;
						}
					} else {
						hRes = E_FAIL;
						strLog = UtilLoadString(IDS_ERROR_DND_FAILED);
						//Drag & Drop failed
					}
				} else {
					strLog = L"";
					hRes = S_OK;
				}
				if (lpDropSource)lpDropSource->Release();
			}
		}
		if (hObject)GlobalFree(hObject);

		//if (lpDataObject)lpDataObject->Release();
		return hRes;
	}
};
