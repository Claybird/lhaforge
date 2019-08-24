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

class CArchiverJACK:public CArchiverDLL{
protected:
	struct JakHeader
	{
		char Magic[18];		// This_Is_Jack_File\0
		char dummy1[2];
		DWORD No;			// 切り裂きファイルの連番( 1～ )
		DWORD FileNameLen;	// ファイル名の長さ
		DWORD FileSize;		// 切り裂く前のファイルサイズ
		FILETIME FileTime;	// ファイルの更新日付
		char dummy2[3];
	//	char* OrigFileName;	// ファイル名 ( NULL 終端なし )
	};

	enum{sizeof_JakHeader=43};
	int FindHeader(const BYTE* hdr,DWORD size);
	bool GetContainedFileName(LPCTSTR,CString &strFileName);
public:
	CArchiverJACK();
	virtual ~CArchiverJACK();
	virtual bool Compress(LPCTSTR,std::list<CString>&,CConfigManager&,const PARAMETER_TYPE,int,LPCTSTR,LPCTSTR,LPCTSTR,CString &)override;
	virtual bool Extract(LPCTSTR,CConfigManager&,const CConfigExtract&,bool,LPCTSTR,CString &)override;
	virtual bool ExtractSpecifiedOnly(LPCTSTR ArcFileName,CConfigManager&,LPCTSTR OutputDir,std::list<CString>&,CString &,bool bUsePath=false)override;
	virtual bool ExamineArchive(LPCTSTR,CConfigManager&,bool,bool&,bool&,CString&,CString &strErr)override;
	virtual bool InspectArchiveNext()override{return false;}
	virtual bool QueryInspectSupported()const override{return false;}
	virtual bool QueryExtractSpecifiedOnlySupported(LPCTSTR)const override{return false;}
	virtual bool IsWeakErrorCheck()const override{return true;}	//%Prefix%()のエラーチェックが甘い(XacRettのように)ならtrue
};
