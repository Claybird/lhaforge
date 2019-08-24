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
#include "ArchiverUNBEL.h"
#include "../resource.h"
#include "../ConfigCode/ConfigExtract.h"
#include "../Utilities/FileOperation.h"
#include "../Utilities/OSUtil.h"

CArchiverUNBEL::CArchiverUNBEL()
{
	m_nRequiredVersion=45;
	m_nRequiredSubVersion=1;
	m_strDllName=_T("Unbel32.DLL");
	m_AstrPrefix="Unbel";
	m_LoadLevel=LOAD_DLL_MINIMUM;
}

CArchiverUNBEL::~CArchiverUNBEL()
{
	FreeDLL();
}

LOAD_RESULT CArchiverUNBEL::LoadDLL(CConfigManager &ConfigManager,CString &strErr)
{
	FreeDLL();

	LOAD_RESULT res=CArchiverDLL::LoadDLL(ConfigManager,strErr);
	if(LOAD_RESULT_OK!=res){
		return res;
	}

	CStringA FunctionName=m_AstrPrefix;
	FunctionName+="OpenArchive";
	ArchiverOpenArchive=(COMMON_ARCHIVER_OPENARCHIVE)GetProcAddress(m_hInstDLL,FunctionName);
	if(NULL==ArchiverOpenArchive){
		strErr.Format(IDS_ERROR_DLL_FUNCTION_GET,m_strDllName,FunctionName);
		FreeDLL();
		return LOAD_RESULT_INVALID;
	}

	FunctionName=m_AstrPrefix;
	FunctionName+="CloseArchive";
	ArchiverCloseArchive=(COMMON_ARCHIVER_CLOSEARCHIVE)GetProcAddress(m_hInstDLL,FunctionName);
	if(NULL==ArchiverCloseArchive){
		strErr.Format(IDS_ERROR_DLL_FUNCTION_GET,m_strDllName,FunctionName);
		FreeDLL();
		return LOAD_RESULT_INVALID;
	}

	//FindFirst
	FunctionName=m_AstrPrefix;
	FunctionName+="FindFirst";
	ArchiverFindFirst=(COMMON_ARCHIVER_FINDFIRST)GetProcAddress(m_hInstDLL,FunctionName);
	if(NULL==ArchiverFindFirst){
		strErr.Format(IDS_ERROR_DLL_FUNCTION_GET,m_strDllName,FunctionName);
		FreeDLL();
		return LOAD_RESULT_INVALID;
	}

	//ファイル名取得
	FunctionName=m_AstrPrefix;
	FunctionName+="GetFileName";
	ArchiverGetFileName=(COMMON_ARCHIVER_GETFILENAME)GetProcAddress(m_hInstDLL,FunctionName);
	if(NULL==ArchiverGetFileName){
		strErr.Format(IDS_ERROR_DLL_FUNCTION_GET,m_strDllName,FunctionName);
		FreeDLL();
		return LOAD_RESULT_INVALID;
	}
	return LOAD_RESULT_OK;
}

bool CArchiverUNBEL::Compress(LPCTSTR,std::list<CString>&,CConfigManager&,const PARAMETER_TYPE,int,LPCTSTR,LPCTSTR,LPCTSTR,CString &)
{
	//BELは解凍のみ
	ASSERT(false);
	return false;
}

bool CArchiverUNBEL::Extract(LPCTSTR ArcFileName,CConfigManager&,const CConfigExtract&,bool bSafeArchive,LPCTSTR OutputDir,CString &strLog)
{
	if(!IsOK()){
		return false;
	}
	if(!bSafeArchive){
		strLog.Format(IDS_ERROR_DANGEROUS_ARCHIVE,ArcFileName);
		return false;
	}

	//出力先移動
	CCurrentDirManager currentDir(OutputDir);
	//===========================
	// DLLに渡すオプションの設定
	//===========================
	TRACE(_T("DLLに渡すオプションの設定\n"));

	CString Param;//コマンドライン パラメータ バッファ

	//解凍パラメータはなし
//	if(Config.Common.Extract.ForceOverwrite){
		//強制上書きはできなさそう
//	}

	//アーカイブファイル名指定
	Param+=_T("\"");
	Param+=ArcFileName;
	Param+=_T("\" ");

	//出力先指定
	Param+=_T("\"");
	Param+=_T(".\\");
	Param+=_T("\"");

	ASSERT(!Param.IsEmpty());
	TRACE(_T("ArchiveHandler Commandline Parameter:%s\n"),Param);

	TRACE(_T("ArchiveHandler呼び出し\n"));
	//char szLog[LOG_BUFFER_SIZE]={0};
	std::vector<char> szLog(LOG_BUFFER_SIZE);
	szLog[0]='\0';
	int Ret=ArchiveHandler(NULL,CT2A(Param),&szLog[0],LOG_BUFFER_SIZE-1);
	strLog=&szLog[0];

	return 0==Ret;
}

//=========================================================
// Belon形式はファイルを一つだけしか格納できない上、
// ファイル名は拡張子3文字のみしか保持していない
// しかし、DTVは起こせる。
//=========================================================
bool CArchiverUNBEL::ExamineArchive(LPCTSTR ArcFileName,CConfigManager& ConfMan,bool,bool &bInFolder,bool &bSafeArchive,CString &BaseDir,CString &strErr)
{
	bInFolder=false;
	ASSERT(IsOK());
	if(!IsOK()){
		return false;
	}

	if(!InspectArchiveBegin(ArcFileName,ConfMan)){
		strErr.Format(IDS_ERROR_OPEN_ARCHIVE,ArcFileName);
		return false;
	}

	CString Buffer;
	InspectArchiveNext();		//単一ファイルのみ格納
	InspectArchiveGetFileName(Buffer);
	InspectArchiveEnd();

	Buffer.Replace(_T('\\'),_T('/'));		//パス区切り文字の置き換え
	TRACE(_T("%s\n"),Buffer);

	if(-1!=Buffer.Find(_T("../"))){
		bSafeArchive=false;
	}
	else{
		bSafeArchive=true;
	}
	return true;
}

bool CArchiverUNBEL::ExtractSpecifiedOnly(LPCTSTR ArcFileName,CConfigManager&,LPCTSTR OutputDir,std::list<CString>&,CString &,bool bUsePath)
{
	return false;
}

