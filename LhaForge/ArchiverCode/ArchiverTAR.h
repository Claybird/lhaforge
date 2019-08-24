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
#include "arc_interface.h"
#include "../resource.h"

class CArchiverTAR:public CArchiverDLL{
protected:
	COMMON_ARCHIVER_OPENARCHIVE2 ArchiverOpenArchive2;
public:
	CArchiverTAR();
	virtual ~CArchiverTAR();
	virtual LOAD_RESULT LoadDLL(CConfigManager&,CString &strErr)override;
	virtual void FreeDLL()override;
	virtual bool InspectArchiveBegin(LPCTSTR,CConfigManager&)override;				//書庫内調査開始

	virtual bool Compress(LPCTSTR,std::list<CString>&,CConfigManager&,const PARAMETER_TYPE,int,LPCTSTR,LPCTSTR,LPCTSTR,CString &)override;
	virtual bool Extract(LPCTSTR,CConfigManager&,const CConfigExtract&,bool,LPCTSTR,CString &)override;
	virtual bool ExtractSpecifiedOnly(LPCTSTR ArcFileName,CConfigManager&,LPCTSTR OutputDir,std::list<CString>&,CString &,bool bUsePath=false)override;
	virtual bool ExamineArchive(LPCTSTR,CConfigManager&,bool,bool&,bool&,CString&,CString &strErr)override;
};

enum{
	GZIP_COMPRESS_LEVEL_LOWEST=1,
	GZIP_COMPRESS_LEVEL_HIGHEST=9,
	BZIP2_COMPRESS_LEVEL_LOWEST=1,
	BZIP2_COMPRESS_LEVEL_HIGHEST=9,
	XZ_COMPRESS_LEVEL_LOWEST=0,
	XZ_COMPRESS_LEVEL_HIGHEST=9,
	LZMA_COMPRESS_LEVEL_LOWEST=0,
	LZMA_COMPRESS_LEVEL_HIGHEST=9,

	TAR_SORT_BY_NONE=0,
	TAR_SORT_BY_EXT=1,
	TAR_SORT_BY_PATH=2,
	TAR_SORT_BY_MAX=TAR_SORT_BY_PATH,
	TAR_SORT_BY_ITEM_COUNT=TAR_SORT_BY_MAX+1,
};
