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

class CArchiverAISH:public CArchiverDLL{
protected:
	bool HasFiles(LPCTSTR);
public:
	CArchiverAISH();
	virtual ~CArchiverAISH();
	virtual bool Compress(LPCTSTR,std::list<CString>&,CConfigManager&,const PARAMETER_TYPE,int,LPCTSTR,LPCTSTR,LPCTSTR,CString &)override;
	virtual bool Extract(LPCTSTR,CConfigManager&,const CConfigExtract&,bool,LPCTSTR,CString &)override;
	virtual bool ExtractSpecifiedOnly(LPCTSTR ArcFileName,CConfigManager&,LPCTSTR OutputDir,std::list<CString>&,CString &,bool bUsePath=false)override{return false;}
	virtual bool ExamineArchive(LPCTSTR,CConfigManager&,bool bSkipDir,bool &,bool &,CString&,CString &strErr)override;
	virtual bool InspectArchiveNext()override{return false;}
	virtual bool QueryInspectSupported()const override{return false;}
	virtual bool QueryExtractSpecifiedOnlySupported(LPCTSTR)const override{return false;}
	virtual BOOL CheckArchive(LPCTSTR)override;
	virtual bool IsWeakCheckArchive()const override{return true;}	//CheckArchiveの機能が貧弱(UNBEL/AISHのように)かどうか
	virtual bool IsWeakErrorCheck()const override{return true;}	//%Prefix%()のエラーチェックが甘い(XacRettのように)ならtrue
};


enum ISH_ENCODE_TYPE{
	ISH_SHIFT_JIS,
	ISH_JIS7,
	ISH_JIS8,
	ISH_NON_KANA_SHIFT_JIS,

	ENUM_COUNT_AND_LASTITEM(ISH_ENCODE_TYPE),
};
