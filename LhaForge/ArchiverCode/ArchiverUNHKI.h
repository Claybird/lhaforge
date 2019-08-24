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

class CArchiverUNHKI:public CArchiverDLL{
public:
	CArchiverUNHKI();
	virtual ~CArchiverUNHKI();
	virtual bool Compress(LPCTSTR,std::list<CString>&,CConfigManager&,const PARAMETER_TYPE,int,LPCTSTR,LPCTSTR,LPCTSTR,CString &)override;
	virtual bool Extract(LPCTSTR,CConfigManager&,const CConfigExtract&,bool,LPCTSTR,CString &)override;
	virtual bool ExtractSpecifiedOnly(LPCTSTR ArcFileName,CConfigManager&,LPCTSTR OutputDir,std::list<CString>&,CString &,bool bUsePath=false)override;
	virtual bool ExamineArchive(LPCTSTR,CConfigManager&,bool,bool&,bool&,CString&,CString &strErr)override;
	virtual BOOL CheckArchive(LPCTSTR)override;
	virtual bool InspectArchiveGetWriteTime(FILETIME &FileTime)override;

	virtual bool DeleteItemFromArchive(LPCTSTR ArcFileName,CConfigManager&,const std::list<CString>&,CString &strLog)override;
	virtual bool QueryDeleteItemFromArchiveSupported(LPCTSTR ArcFileName)const override{return true;}		//DeleteFileがサポートされているかどうか
	virtual ARCRESULT TestArchive(LPCTSTR,CString &)override;	//アーカイブが正しいかどうかチェックする
};

enum HKI_COMPRESS_LEVEL{
	HKI_COMPRESS_LEVEL_NORMAL,
	HKI_COMPRESS_LEVEL_NONE,
	HKI_COMPRESS_LEVEL_FAST,
	HKI_COMPRESS_LEVEL_HIGH,

	ENUM_COUNT_AND_LASTITEM(HKI_COMPRESS_LEVEL),
};

enum HKI_ENCRYPT_ALGORITHM{
	HKI_ENCRYPT_NONE,
	HKI_ENCRYPT_RIJNDAEL128,
	HKI_ENCRYPT_RIJNDAEL256,
	HKI_ENCRYPT_SINGLE_DES,
	HKI_ENCRYPT_TRIPLE_DES,
	HKI_ENCRYPT_BLOWFISH448,
	HKI_ENCRYPT_TWOFISH128,
	HKI_ENCRYPT_TWOFISH256,
	HKI_ENCRYPT_SQUARE,

	ENUM_COUNT_AND_LASTITEM(HKI_ENCRYPT_ALGORITHM),
};
