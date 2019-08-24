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

class CArchiverUNRAR:public CArchiverDLL{
protected:
	COMMON_ARCHIVER_SETUNICODEMODE ArchiverSetUnicodeMode;
	bool m_bUTF8;
	CString m_strDllDisplayName;
protected:
	bool IsHeaderEncrypted(LPCTSTR);
	virtual void WriteResponceFile(HANDLE,LPCTSTR);;
public:
	CArchiverUNRAR();
	virtual ~CArchiverUNRAR();
	virtual LOAD_RESULT LoadDLL(CConfigManager&,CString &strErr)override;
	virtual void FreeDLL()override;
	virtual bool Compress(LPCTSTR,std::list<CString>&,CConfigManager&,const PARAMETER_TYPE,int,LPCTSTR,LPCTSTR,LPCTSTR,CString &)override;
	virtual bool Extract(LPCTSTR,CConfigManager&,const CConfigExtract&,bool,LPCTSTR,CString &)override;
	virtual bool ExtractSpecifiedOnly(LPCTSTR ArcFileName,CConfigManager&,LPCTSTR OutputDir,std::list<CString>&,CString &,bool bUsePath=false)override;
	virtual bool ExamineArchive(LPCTSTR,CConfigManager&,bool,bool&,bool&,CString&,CString &strErr)override;
	virtual ARCRESULT TestArchive(LPCTSTR,CString &)override;	//アーカイブが正しいかどうかチェックする
	virtual LPCTSTR GetName()const override{return m_strDllDisplayName;}	//DLL名を返す
	virtual bool IsUnicodeCapable()const{return true;}	//UNICODE対応DLLならtrueを返す

	virtual BOOL CheckArchive(LPCTSTR)override;
	virtual int GetFileCount(LPCTSTR)override;	//アーカイブ中のファイル数を返す
	// 書庫内検査用メソッド
	virtual bool InspectArchiveBegin(LPCTSTR,CConfigManager&)override;				//書庫内調査開始
	virtual bool InspectArchiveGetFileName(CString&)override;		//書庫内ファイル名取得
	virtual bool InspectArchiveGetMethodString(CString&)override;			//書庫内ファイル格納モード取得
};

