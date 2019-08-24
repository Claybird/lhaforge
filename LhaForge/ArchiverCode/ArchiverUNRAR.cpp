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

#include "stdafx.h"
#include "ArchiverUNRAR.h"
#include "../Utilities/FileOperation.h"
#include "../Utilities/StringUtil.h"
#include "../resource.h"
#include "../ConfigCode/ConfigExtract.h"
#include "../Utilities/OSUtil.h"

CArchiverUNRAR::CArchiverUNRAR()
{
	m_nRequiredVersion=12;
	m_strDllName=_T("unrar32.dll");
	m_AstrPrefix="Unrar";
	m_bUTF8=false;

	m_strDllDisplayName=m_strDllName;
}

CArchiverUNRAR::~CArchiverUNRAR()
{
	FreeDLL();
}

LOAD_RESULT CArchiverUNRAR::LoadDLL(CConfigManager &ConfMan,CString &strErr)
{
	FreeDLL();

	//基底クラスのメソッドを呼ぶ
	LOAD_RESULT res=CArchiverDLL::LoadDLL(ConfMan,strErr);
	if(LOAD_RESULT_OK!=res){
		return res;
	}

	//UNICODEモード設定用
	CStringA strFunctionName;
	strFunctionName=m_AstrPrefix+"SetUnicodeMode";
	ArchiverSetUnicodeMode=(COMMON_ARCHIVER_SETUNICODEMODE)GetProcAddress(m_hInstDLL,strFunctionName);
	if(NULL==ArchiverSetUnicodeMode){
		m_bUTF8=false;
	}
	//ここでUNICODEモードに設定: UNICODE非対応版DLLなら何もしない
	if(ArchiverSetUnicodeMode && ArchiverSetUnicodeMode(TRUE)){
		m_bUTF8=true;
	}else{
		m_bUTF8=false;
	}

	m_strDllDisplayName=m_bUTF8 ? m_strDllName+_T("[UTF8]") : m_strDllName;

	return LOAD_RESULT_OK;
}

void CArchiverUNRAR::FreeDLL()
{
	if(m_hInstDLL){
		ArchiverSetUnicodeMode=NULL;
		m_strDllDisplayName=m_strDllName;
		CArchiverDLL::FreeDLL();
	}
}

//レスポンスファイルにファイル名をエスケープを実行した上で書き込む。
//有効なファイルハンドルとNULLでないファイル名を渡すこと。
void CArchiverUNRAR::WriteResponceFile(HANDLE hFile,LPCTSTR fname)
{
	if(m_bUTF8){
		CPath strPath=fname;

		strPath.QuoteSpaces();

		DWORD dwWritten=0;
		//ファイル名+改行を出力
		std::vector<BYTE> cArray;
		UtilToUTF8(cArray,strPath+_T("\r\n"));
		WriteFile(hFile,&cArray[0],(cArray.size()-1)*sizeof(BYTE),&dwWritten,NULL);
	}else{
		CArchiverDLL::WriteResponceFile(hFile,fname);
	}
}

bool CArchiverUNRAR::Compress(LPCTSTR,std::list<CString>&,CConfigManager&,const PARAMETER_TYPE,int,LPCTSTR,LPCTSTR,LPCTSTR,CString &)
{
	//RARは解凍のみ
	ASSERT(false);
	return false;
}

//bSafeArchiveは無視される
bool CArchiverUNRAR::Extract(LPCTSTR ArcFileName,CConfigManager&,const CConfigExtract &Config,bool,LPCTSTR OutputDir,CString &strLog)
{
	if(!IsOK()){
		return false;
	}

	//===========================
	// DLLに渡すオプションの設定
	//===========================
	TRACE(_T("DLLに渡すオプションの設定\n"));

	CString Param;//コマンドライン パラメータ バッファ
	//出力先移動
	CCurrentDirManager currentDir(OutputDir);

	//解凍パラメータ
	Param+=
		_T("-x ")		//解凍
		_T("-e2 ")		//セキュリティレベル(基準ディレクトリ以外に展開されるようなパスを可能な限りどうにかします)
	;
	if(Config.ForceOverwrite){
		//強制上書き
		Param+=_T("-o ");
	}

	//アーカイブファイル名指定
	Param+=_T("\"");
	Param+=ArcFileName;
	Param+=_T("\" ");

	ASSERT(!Param.IsEmpty());
	TRACE(_T("ArchiveHandler Commandline Parameter:%s\n"),Param);

	TRACE(_T("ArchiveHandler呼び出し\n"));
	//char szLog[LOG_BUFFER_SIZE]={0};
	std::vector<BYTE> szLog(LOG_BUFFER_SIZE);
	szLog[0]='\0';
	int Ret;
	if(m_bUTF8){
		Ret=ArchiveHandler(NULL,C2UTF8(Param),(LPSTR)&szLog[0],LOG_BUFFER_SIZE-1);
		CString strTmp;
		UtilToUNICODE(strTmp,&szLog[0],szLog.size()-1,UTILCP_UTF8);
		strLog=strTmp;
	}else{
		Ret=ArchiveHandler(NULL,CT2A(Param),(LPSTR)&szLog[0],LOG_BUFFER_SIZE-1);
		strLog=&szLog[0];
	}

	return 0==Ret;
}

//=========================================================
// UnrarGetFileName()の出力結果を基に、格納されたファイルがパス
// 情報を持っているかどうか判別し、二重フォルダ作成を防ぐ
//=========================================================
bool CArchiverUNRAR::ExamineArchive(LPCTSTR ArcFileName,CConfigManager& ConfMan,bool bSkipDir,bool &bInFolder,bool &,CString &BaseDir,CString &strErr)
{
	if(bSkipDir){
		TRACE(_T("二重フォルダ判定回避\n"));
		return true;
	}
	if(IsHeaderEncrypted(ArcFileName)){
		//ヘッダ暗号化済み?
		CConfigExtract ConfExtract;
		ConfExtract.load(ConfMan);
		if(ConfExtract.MinimumPasswordRequest){
			//パスワード入力回数を最小限に;二重判定をスキップする
			bInFolder=false;
			return true;
		}
	}
	return _ExamineArchiveFast(ArcFileName,ConfMan,bInFolder,BaseDir,strErr);
}


bool CArchiverUNRAR::ExtractSpecifiedOnly(LPCTSTR ArcFileName,CConfigManager&,LPCTSTR OutputDir,std::list<CString> &FileList,CString &strLog,bool bUsePath)
{
	if(!IsOK()){
		return false;
	}

	//==============================================
	// レスポンスファイル用テンポラリファイル名取得
	//==============================================
	TCHAR ResponceFileName[_MAX_PATH+1];
	FILL_ZERO(ResponceFileName);
	if(!UtilGetTemporaryFileName(ResponceFileName,_T("rar"))){
		strLog=CString(MAKEINTRESOURCE(IDS_ERROR_TEMPORARY_FILE_CREATE));
		return false;
	}
	ASSERT(0!=_tcslen(ResponceFileName));

	//解凍対象ファイルをレスポンスファイルに書き出す
	{
		HANDLE hFile=CreateFile(ResponceFileName,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
		if(INVALID_HANDLE_VALUE==hFile){
			strLog=CString(MAKEINTRESOURCE(IDS_ERROR_TEMPORARY_FILE_ACCESS));
			return false;
		}

		std::list<CString>::iterator ite=FileList.begin();
		const std::list<CString>::iterator end=FileList.end();
		for(;ite!=end;ite++){
			CPath target = *ite;
			WriteResponceFile(hFile,*ite);
		}
		CloseHandle(hFile);
	}
	//出力先移動
	CCurrentDirManager currentDir(OutputDir);

	//===========================
	// DLLに渡すオプションの設定
	//===========================
	CString Param;

	//解凍パラメータ
	if(bUsePath){
		Param+=_T("-x ");	//パス付き解凍
	}else{
		Param+=_T("-e ");		//パス無し解凍
	}
	Param+=
		_T("-s ")		//厳密に比較
		_T("-e2 ")		//セキュリティレベル(基準ディレクトリ以外に展開されるようなパスを可能な限りどうにかします)
	;

	//アーカイブファイル名指定
	Param+=_T("\"");
	Param+=ArcFileName;
	Param+=_T("\" ");

	//出力先フォルダ
	Param+=_T("\"");
	Param+=OutputDir;
	Param+=_T("\" ");

	//レスポンスファイル名指定
	Param+=_T("\"@");
	Param+=ResponceFileName;
	Param+=_T("\"");

	TRACE(_T("ArchiveHandler呼び出し\nCommandline Parameter:%s\n"),Param);
	//char szLog[LOG_BUFFER_SIZE]={0};
	std::vector<BYTE> szLog(LOG_BUFFER_SIZE);
	szLog[0]='\0';
	int Ret;
	if(m_bUTF8){
		Ret=ArchiveHandler(NULL,C2UTF8(Param),(LPSTR)&szLog[0],LOG_BUFFER_SIZE-1);
		CString strTmp;
		UtilToUNICODE(strTmp,&szLog[0],szLog.size()-1,UTILCP_UTF8);
		strLog=strTmp;
	}else{
		Ret=ArchiveHandler(NULL,CT2A(Param),(LPSTR)&szLog[0],LOG_BUFFER_SIZE-1);
		strLog=&szLog[0];
	}
	//使ったレスポンスファイルは消去
	DeleteFile(ResponceFileName);

	return 0==Ret;
}

ARCRESULT CArchiverUNRAR::TestArchive(LPCTSTR ArcFileName,CString &strLog)
{
	TRACE(_T("TestArchive() called.\n"));
	ASSERT(IsOK()&&ArchiverCheckArchive);
	if(!IsOK()||!ArchiverCheckArchive){
		return TEST_ERROR;
	}

	//CheckArchiveによるテストが実装されている
	strLog.Format(IDS_TESTARCHIVE_WITH_CHECKARCHIVE,ArcFileName,GetName());	//APIによるチェック
	strLog+=_T("\r\n\r\n");

	if(ArchiverCheckArchive(m_bUTF8 ? (LPCSTR)C2UTF8(ArcFileName) : CT2A(ArcFileName),CHECKARCHIVE_FULLCRC|CHECKARCHIVE_NOT_ASK_PASSWORD)){
		//正常
		strLog+=CString(MAKEINTRESOURCE(IDS_TESTARCHIVE_RESULT_OK));
		return TEST_OK;
	}else{
		//異常
		strLog+=CString(MAKEINTRESOURCE(IDS_TESTARCHIVE_RESULT_NG));
		return TEST_NG;
	}
}

BOOL CArchiverUNRAR::CheckArchive(LPCTSTR _szFileName)
{
	if(!ArchiverCheckArchive){
		ASSERT(ArchiverCheckArchive);
		return false;
	}
	return ArchiverCheckArchive(m_bUTF8 ? (LPCSTR)C2UTF8(_szFileName) : CT2A(_szFileName),CHECKARCHIVE_BASIC|CHECKARCHIVE_NOT_ASK_PASSWORD);
}

bool CArchiverUNRAR::IsHeaderEncrypted(LPCTSTR _szFileName)
{
	return ERROR_PASSWORD_FILE==CheckArchive(_szFileName);
}

int CArchiverUNRAR::GetFileCount(LPCTSTR _szFileName)
{
	if(!ArchiverGetFileCount){
		return -1;
	}
	return ArchiverGetFileCount(m_bUTF8 ? (LPCSTR)C2UTF8(_szFileName) : CT2A(_szFileName));
}

bool CArchiverUNRAR::InspectArchiveBegin(LPCTSTR ArcFileName,CConfigManager& config)
{
	if(m_bUTF8){
		ASSERT(ArchiverOpenArchive);
		if(!ArchiverOpenArchive){
			return false;
		}
		if(m_hInspectArchive){
			ASSERT(!"Close the Archive First!!!\n");
			return false;
		}
		m_hInspectArchive=ArchiverOpenArchive(NULL,m_bUTF8 ? (LPCSTR)C2UTF8(ArcFileName) : CT2A(ArcFileName),m_dwInspectMode);
		if(!m_hInspectArchive){
			return false;
		}
		m_bInspectFirstTime=true;

		FILL_ZERO(m_IndividualInfo);
		return true;
	}else{
		return __super::InspectArchiveBegin(ArcFileName, config);
	}
}

bool CArchiverUNRAR::InspectArchiveGetFileName(CString &FileName)
{
	if(m_bUTF8){
		if(ArchiverGetFileName){
			if(!m_hInspectArchive){
				ASSERT(!"Open an Archive First!!!\n");
				return false;
			}
			std::vector<BYTE> szBuffer(FNAME_MAX32*2+1);
			ArchiverGetFileName(m_hInspectArchive,(LPCSTR)&szBuffer[0],FNAME_MAX32*2);
			UtilToUNICODE(FileName,&szBuffer[0],szBuffer.size(),UTILCP_UTF8);
			return true;
		}
		else{
			ASSERT(LOAD_DLL_STANDARD!=m_LoadLevel);
			if(!m_hInspectArchive){
				ASSERT(!"Open an Archive First!!!\n");
				return false;
			}
			UtilToUNICODE(FileName,(LPCBYTE)m_IndividualInfo.szFileName,sizeof(m_IndividualInfo.szFileName),UTILCP_UTF8);
			return true;
		}
	}else{
		return __super::InspectArchiveGetFileName(FileName);
	}
}

bool CArchiverUNRAR::InspectArchiveGetMethodString(CString &strMethod)
{
	if(m_bUTF8){
		if(!m_hInspectArchive){
			ASSERT(!"Open an Archive First!!!\n");
			return false;
		}

		//\0で終わっているかどうか保証できない
		char szBuffer[32]={0};

		if(ArchiverGetMethod){
			if(0!=ArchiverGetMethod(m_hInspectArchive,szBuffer,31)){
				//構造体から取得
				strncpy_s(szBuffer,m_IndividualInfo.szMode,8);
			}
		}
		else{
			//構造体から取得
			strncpy_s(szBuffer,m_IndividualInfo.szMode,8);
		}

		//情報格納
		UtilToUNICODE(strMethod,(LPCBYTE)szBuffer,9,UTILCP_UTF8);
		return true;
	}else{
		return __super::InspectArchiveGetMethodString(strMethod);
	}
}
