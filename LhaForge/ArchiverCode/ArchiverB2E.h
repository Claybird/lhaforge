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

