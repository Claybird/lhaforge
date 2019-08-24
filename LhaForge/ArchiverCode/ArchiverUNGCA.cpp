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
#include "ArchiverUNGCA.h"
#include "../Utilities/FileOperation.h"
#include "../resource.h"
#include "../ConfigCode/ConfigExtract.h"
#include "../Utilities/OSUtil.h"

CArchiverUNGCA::CArchiverUNGCA()
{
	m_nRequiredVersion=11;
	m_nRequiredSubVersion=200;
	m_strDllName=_T("UnGCA32.DLL");
	m_AstrPrefix="UnGCA";
}

CArchiverUNGCA::~CArchiverUNGCA()
{
	FreeDLL();
}

bool CArchiverUNGCA::Compress(LPCTSTR,std::list<CString>&,CConfigManager&,const PARAMETER_TYPE,int,LPCTSTR,LPCTSTR,LPCTSTR,CString &)
{
	//GCAは解凍のみ
	ASSERT(false);
	return false;
}

bool CArchiverUNGCA::Extract(LPCTSTR ArcFileName,CConfigManager&,const CConfigExtract &Config,bool bSafeArchive,LPCTSTR OutputDir,CString &strLog)
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

	//解凍パラメータ
	Param+=
		_T("e ")			//解凍
		_T("-sx1 ")			//進捗ダイアログ表示
	;
	if(Config.ForceOverwrite){
		//強制上書き
		Param+=_T("-yx0 ");
	}else{
		Param+=_T("-yx1 ");
	}

	//アーカイブファイル名指定
	Param+=_T("\"");
	Param+=ArcFileName;
	Param+=_T("\" ");

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
// UnGCAGetFileName()の出力結果を基に、格納されたファイルがパス
//情報を持っているかどうか判別し、二重フォルダ作成を防ぐ
// 安全確認も行う
//=========================================================
bool CArchiverUNGCA::ExamineArchive(LPCTSTR ArcFileName,CConfigManager& ConfMan,bool,bool &bInFolder,bool &bSafeArchive,CString &BaseDir,CString &strErr)
{
	return _ExamineArchive(ArcFileName,ConfMan,bInFolder,bSafeArchive,0,BaseDir,strErr);
}

BOOL CArchiverUNGCA::CheckArchive(LPCTSTR _szFileName)
{
	if(!ArchiverCheckArchive){
		ASSERT(ArchiverCheckArchive);
		return false;
	}
	return ArchiverCheckArchive(CT2A(_szFileName),CHECKARCHIVE_ALL);
}

bool CArchiverUNGCA::InspectArchiveGetWriteTime(FILETIME &FileTime)
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

bool CArchiverUNGCA::ExtractSpecifiedOnly(LPCTSTR ArcFileName,CConfigManager&,LPCTSTR OutputDir,std::list<CString> &FileList,CString &strLog,bool bUsePath)
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
	if(!UtilGetTemporaryFileName(ResponceFileName,_T("gca"))){
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
	Param+=
		_T("ex ")		//指定ファイル解凍
		_T("-sx1 ")		//進捗ダイアログ表示
		_T("-yx1 ")		//上書き確認
	;

	if(bUsePath){
		Param+=_T("-xx1 ");	//パス有効
	}

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
	std::vector<char> szLog(LOG_BUFFER_SIZE);
	szLog[0]='\0';
	int Ret=ArchiveHandler(NULL,CT2A(Param),&szLog[0],LOG_BUFFER_SIZE-1);
	strLog=&szLog[0];
	//使ったレスポンスファイルは消去
	DeleteFile(ResponceFileName);

	return 0==Ret;
}

