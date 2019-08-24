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


typedef BOOL (WINAPI *B2ESETSCRIPTDIRECTORY)(LPCSTR);	//B2ESetScriptDirectory
typedef int  (WINAPI *B2ESCRIPTGETCOUNT)();				//B2EScriptGetCount
typedef BOOL (WINAPI *B2ESCRIPTGETABILITY)(const UINT,LPWORD);	//B2EScriptGetAbility
typedef BOOL (WINAPI *B2ESCRIPTGETCOMPRESSTYPE)(const UINT,LPSTR,const DWORD);	//B2EScriptGetCompressType
typedef BOOL (WINAPI *B2ESCRIPTGETCOMPRESSMETHOD)(const UINT,const UINT,LPSTR,const DWORD);	//B2EScriptGetCompressMethod
typedef int  (WINAPI *B2ESCRIPTGETDEFAULTCOMPRESSMETHOD)(const UINT);	//B2EScriptGetDefaultCompressMethod
typedef UINT (WINAPI *B2ESCRIPTGETEXTRACTORINDEX)(LPCSTR);	//B2EScriptGetExtractorIndex
typedef BOOL (WINAPI *B2ESCRIPTGETNAME)(const UINT,LPSTR,const DWORD);	//B2EScriptGetName


#define B2EABILITY_CHECK		1       //CheckArchive()相当の処理が可能
#define B2EABILITY_MELT			2       //解凍処理が可能
#define B2EABILITY_LIST			4       //書庫内ファイルの列挙が可能
#define B2EABILITY_MELT_EACH	8       //指定したファイルのみの解凍が可能
#define B2EABILITY_COMPRESS		16      //圧縮が可能
#define B2EABILITY_ARCHIVE		32      //複数ファイルをまとめることが可能(cf.GZip)
#define B2EABILITY_SFX			64      //自己解凍ファイルを作成可能
#define B2EABILITY_ADD			128		/*ファイルの追加が可能*/
#define B2EABILITY_DELETE		256		/*ファイルの削除が可能*/

//B2Eスクリプトの情報
struct B2ESCRIPTINFO{
	UINT uIndex;
	WORD wAbility;
	char szFormat[_MAX_PATH+1];	//圧縮形式名
	std::vector<CStringA> MethodArray;
	int nDefaultMethod;
};

class CArchiverB2E:public CArchiverDLL{
protected:
	B2ESETSCRIPTDIRECTORY				B2ESetScriptDirectory;
	B2ESCRIPTGETCOUNT					B2EScriptGetCount;
	B2ESCRIPTGETABILITY					B2EScriptGetAbility;
	B2ESCRIPTGETCOMPRESSTYPE			B2EScriptGetCompressType;
	B2ESCRIPTGETCOMPRESSMETHOD			B2EScriptGetCompressMethod;
	B2ESCRIPTGETDEFAULTCOMPRESSMETHOD	B2EScriptGetDefaultCompressMethod;
	B2ESCRIPTGETEXTRACTORINDEX			B2EScriptGetExtractorIndex;
	B2ESCRIPTGETNAME					B2EScriptGetName;
protected:
	//---internal functions
	bool B2ECompress(LPCTSTR ArcFileName,std::list<CString> &ParamList,CConfigManager&,LPCTSTR lpszFormat,LPCTSTR lpszMethod,bool bSFX,CString &);
public:
	CArchiverB2E();
	virtual ~CArchiverB2E();
	virtual LOAD_RESULT LoadDLL(CConfigManager&,CString &strErr)override;
	virtual void FreeDLL()override;
	virtual bool Compress(LPCTSTR,std::list<CString>&,CConfigManager&,const PARAMETER_TYPE,int,LPCTSTR lpszFormat,LPCTSTR lpszMethod,LPCTSTR,CString &)override;
	virtual bool Extract(LPCTSTR,CConfigManager&,const CConfigExtract&,bool,LPCTSTR,CString &)override;
	virtual bool ExtractSpecifiedOnly(LPCTSTR ArcFileName,CConfigManager&,LPCTSTR OutputDir,std::list<CString>&,CString &,bool bUsePath=false)override;
	virtual bool ExamineArchive(LPCTSTR,CConfigManager&,bool,bool&,bool&,CString&,CString &strErr)override;
	virtual bool IsWeakErrorCheck()const override{return true;}	//%Prefix%()のエラーチェックが甘い(XacRettのように)ならtrue
//	virtual bool IsWeakCheckArchive()const{return true;}	解凍候補を自動的に探させるため
	virtual bool QueryExtractSpecifiedOnlySupported(LPCTSTR)const override;
	virtual bool AddItemToArchive(LPCTSTR ArcFileName,bool bEncrypted,const std::list<CString>&,CConfigManager&,LPCTSTR lpDestDir,CString&)override;
	virtual bool QueryAddItemToArchiveSupported(LPCTSTR ArcFileName)const override;

	//アーカイブから指定したファイルを削除
	virtual bool DeleteItemFromArchive(LPCTSTR ArcFileName,CConfigManager&,const std::list<CString>&,CString &)override;
	virtual bool QueryDeleteItemFromArchiveSupported(LPCTSTR ArcFileName)const override;

	//---独自
	bool EnumCompressB2EScript(std::vector<B2ESCRIPTINFO>&);
	bool EnumActiveB2EScriptNames(std::vector<CString> &ScriptNames);
};

