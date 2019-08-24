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
#include "STGMEDIUM.h"
//Original code from http://hp.vector.co.jp/authors/VA016117/
//Modified by Claybird http://claybird.sakura.ne.jp/

BOOL CSTGMEDIUM::Dup(STGMEDIUM *pdest, const FORMATETC* pFormatetc, const STGMEDIUM *pmedium)
{
	HANDLE	hVoid;

	switch (pmedium->tymed) {
	case TYMED_HGLOBAL:
		hVoid = OleDuplicateData(pmedium->hGlobal, pFormatetc->cfFormat, (UINT)NULL);
		pdest->hGlobal = (HGLOBAL)hVoid;
		break;
	case TYMED_GDI:
		hVoid = OleDuplicateData(pmedium->hBitmap, pFormatetc->cfFormat, (UINT)NULL);
		pdest->hBitmap = (HBITMAP)hVoid;
		break;
	case TYMED_MFPICT:
		hVoid = OleDuplicateData(pmedium->hMetaFilePict, pFormatetc->cfFormat, (UINT)NULL);
		pdest->hMetaFilePict = (HMETAFILEPICT)hVoid;
		break;
	case TYMED_ENHMF:
		hVoid = OleDuplicateData(pmedium->hEnhMetaFile, pFormatetc->cfFormat, (UINT)NULL);
		pdest->hEnhMetaFile = (HENHMETAFILE)hVoid;
		break;
	case TYMED_FILE:
		hVoid = OleDuplicateData(pmedium->lpszFileName, pFormatetc->cfFormat, (UINT)NULL);
		pdest->lpszFileName = (LPOLESTR)hVoid;
		break;
	case TYMED_NULL:
		hVoid = (HANDLE)1; //エラーにならないように
	case TYMED_ISTREAM:
	case TYMED_ISTORAGE:
	default:
		hVoid = NULL;
		break;
	}
	if(!hVoid) return FALSE;
	pdest->tymed = pmedium->tymed;
	pdest->pUnkForRelease = pmedium->pUnkForRelease;
	if(pmedium->pUnkForRelease)pmedium->pUnkForRelease->AddRef();

	return	TRUE;
}
