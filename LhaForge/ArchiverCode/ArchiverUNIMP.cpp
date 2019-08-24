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
#include "ArchiverUNIMP.h"
#include "../Utilities/FileOperation.h"
#include "../resource.h"
#include "../ConfigCode/ConfigExtract.h"
#include "../Utilities/OSUtil.h"

CArchiverUNIMP::CArchiverUNIMP()
{
	m_nRequiredVersion=17;
	m_nRequiredSubVersion=43;
	m_strDllName=_T("UnImp32.dll");
	m_AstrPrefix="UnImp";
}

CArchiverUNIMP::~CArchiverUNIMP()
{
	FreeDLL();
}

bool CArchiverUNIMP::Compress(LPCTSTR,std::list<CString>&,CConfigManager&,const PARAMETER_TYPE,int,LPCTSTR,LPCTSTR,LPCTSTR,CString &)
{
	//IMPは解凍のみ
	ASSERT(false);
	return false;
}

//bSafeArchiveは無視される
bool CArchiverUNIMP::Extract(LPCTSTR ArcFileName,CConfigManager&,const CConfigExtract &Config,bool,LPCTSTR OutputDir,CString &strLog)
{
	if(!IsOK()){
		return false;
	}

	//出力先移動
	CCurrentDirManager currentDir(OutputDir);
	//===========================
	// DLLに渡すオプションの設定
	//===========================
	TRACE(_T("DLLに渡すオプションの設定\n"));

	CString Param;//コマンドライン パラメータ バッファ
	//解凍パラメータ
	Param+=
		_T("x ")		//解凍
		_T("-a1 ")		//全属性の解凍
		_T("-jf1 ")		//ルート記号の削除(相対パスに変換)
		_T("-r1 ")		//サブディレクトリも検索
		_T("-jp1 ")		//不正パス(ユーザー問い合わせ)
	;
	if(Config.ForceOverwrite){
		//強制上書き
		Param+=_T("-u0 ");
	}
	else{
		Param+=_T("-u5 ");	//ユーザー問い合わせ
	}

	//アーカイブファイル名指定
	Param+=_T("-- \"");
	Param+=ArcFileName;
	Param+=_T("\" ");

	//出力先指定
	Param+=_T("\"");
	Param+=_T(".\\");//OutputDir;
	Param+=_T("\"");

	ASSERT(!Param.IsEmpty());
	TRACE(_T("ArchiveHandler Commandline Parameter:%s\n"),Param);

	TRACE(_T("ArchiveHandler呼び出し\n"));
	std::vector<char> szLog(LOG_BUFFER_SIZE);
	szLog[0]='\0';
	int Ret=ArchiveHandler(NULL,CT2A(Param),&szLog[0],LOG_BUFFER_SIZE-1);
	strLog=&szLog[0];

	return 0==Ret;
}

//=========================================================
// UnImpGetFileName()の出力結果を基に、格納されたファイルがパス
// 情報を持っているかどうか判別し、二重フォルダ作成を防ぐ
//=========================================================
bool CArchiverUNIMP::ExamineArchive(LPCTSTR ArcFileName,CConfigManager& ConfMan,bool bSkipDir,bool &bInFolder,bool &,CString &BaseDir,CString &strErr)
{
	if(bSkipDir){
		TRACE(_T("二重フォルダ判定回避\n"));
		return true;
	}
	return _ExamineArchiveFast(ArcFileName,ConfMan,bInFolder,BaseDir,strErr);
}

bool CArchiverUNIMP::InspectArchiveGetWriteTime(FILETIME &FileTime)
{
	if(!m_hInspectArchive){
		ASSERT(!"Open an Archive First!!!\n");
		return false;
	}
	//拡張版関数で時刻取得
	if(ArchiverGetWriteTimeEx){
		FILETIME TempTime;
		if(!ArchiverGetWriteTimeEx(m_hInspectArchive,&TempTime))return false;
		if(!LocalFileTimeToFileTime(&TempTime,&FileTime))return false;
		return true;
	}
	//通常版関数で時刻取得
	else if(ArchiverGetWriteTime){
		DWORD UnixTime=ArchiverGetWriteTime(m_hInspectArchive);
		if(-1==UnixTime){
			return false;
		}
		//time_tからFileTimeへ変換
		LONGLONG ll = Int32x32To64(UnixTime, 10000000) + 116444736000000000;
		FileTime.dwLowDateTime = (DWORD) ll;
		FileTime.dwHighDateTime = (DWORD)(ll >>32);
		return true;
	}
	else{
		//INDIVIDUALINFOから時刻取得
		FILETIME TempTime;
		if(!DosDateTimeToFileTime(m_IndividualInfo.wDate,m_IndividualInfo.wTime,&TempTime))return false;
		if(!LocalFileTimeToFileTime(&TempTime,&FileTime))return false;
		return true;
	}
}

//レスポンスファイルにファイル名を書き込む。
//有効なファイルハンドルとNULLでないファイル名を渡すこと。
void CArchiverUNIMP::WriteResponceFile(HANDLE hFile,LPCTSTR fname)
{
	CArchiverDLL::WriteResponceFile(hFile,fname,false);
}

bool CArchiverUNIMP::ExtractSpecifiedOnly(LPCTSTR ArcFileName,CConfigManager&,LPCTSTR OutputDir,std::list<CString> &FileList,CString &strLog,bool bUsePath)
{
	if(!IsOK()){
		return false;
	}
	//出力先移動
	CCurrentDirManager currentDir(OutputDir);

	//==============================================
	// レスポンスファイル用テンポラリファイル名取得
	//==============================================
	TCHAR ResponceFileName[_MAX_PATH+1];
	FILL_ZERO(ResponceFileName);
	if(!UtilGetTemporaryFileName(ResponceFileName,_T("imp"))){
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
			WriteResponceFile(hFile,*ite);
		}
		CloseHandle(hFile);
	}

	//===========================
	// DLLに渡すオプションの設定
	//===========================
	CString Param;

	//解凍パラメータ
	if(bUsePath){
		Param+=_T("x ");	//パスあり解凍
	}else{
		Param+=_T("e ");	//パス無し解凍
	}
	Param+=
		_T("-a1 ")		//全属性の解凍
		_T("-jf1 ")		//ルート記号の削除(相対パスに変換)
		_T("-r0 ")		//サブディレクトリは検索しない
		_T("-jp1 ")		//不正パス(ユーザー問い合わせ)
		_T("-u5 ")		//上書き(ユーザー問い合わせ)
	;

	//アーカイブファイル名指定
	Param+=_T("-- \"");
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
	std::vector<char> szLog(LOG_BUFFER_SIZE);
	szLog[0]='\0';
	int Ret=ArchiveHandler(NULL,CT2A(Param),&szLog[0],LOG_BUFFER_SIZE-1);
	strLog=&szLog[0];
	//使ったレスポンスファイルは消去
	DeleteFile(ResponceFileName);


	return 0==Ret;
}

ARCRESULT CArchiverUNIMP::TestArchive(LPCTSTR ArcFileName,CString &strLog)
{
	TRACE(_T("TestArchive() called.\n"));
	ASSERT(IsOK());
	if(!IsOK()){
		return TEST_ERROR;
	}

	//このDLLのチェック能力は低い

/*	//CheckArchiveによるテストが実装されている
	strLog.Format(IDS_TESTARCHIVE_WITH_CHECKARCHIVE,ArcFileName,GetName());	//APIによるチェック
	strLog+=_T("\r\n\r\n");

	const int CHECKARCHIVE_FULLCRC=2;
	if(ArchiverCheckArchive(ArcFileName,CHECKARCHIVE_FULLCRC)){
		//正常
		strLog+=CString(MAKEINTRESOURCE(IDS_TESTARCHIVE_RESULT_OK));
		return TEST_OK;
	}
	else{
		//異常
		strLog+=CString(MAKEINTRESOURCE(IDS_TESTARCHIVE_RESULT_NG));
		return TEST_NG;
	}*/

	//tコマンドによるテストが実装されている
	CString Param=
		_T("t ")			//テスト
		_T("-- \"")
	;
	Param+=ArcFileName;
	Param+=_T("\"");

	TRACE(_T("%s\n"),Param);

	//ファイル名をログに追加
	strLog=ArcFileName;
	strLog+=_T("\n\n");

	//char szLog[LOG_BUFFER_SIZE]={0};
	std::vector<char> szLog(LOG_BUFFER_SIZE);
	szLog[0]='\0';
	int Ret=ArchiveHandler(NULL,CT2A(Param),&szLog[0],LOG_BUFFER_SIZE-1);
	strLog+=&szLog[0];

	if(Ret==0)return TEST_OK;
	else return TEST_NG;
}
