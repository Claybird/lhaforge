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
	virtual bool InspectArchiveBegin(LPCTSTR,CConfigManager&)override;				//èëå…ì‡í≤ç∏äJén

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
