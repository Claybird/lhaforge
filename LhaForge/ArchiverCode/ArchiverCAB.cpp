/*
 * Copyright (c) 2005-2012, Claybird
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
#include "ArchiverCAB.h"
#include "../Utilities/FileOperation.h"
#include "../resource.h"
#include "../ConfigCode/ConfigExtract.h"
#include "../ConfigCode/ConfigCAB.h"
#include "../Utilities/OSUtil.h"

CArchiverCAB::CArchiverCAB()
{
	m_nRequiredVersion=98;
	m_strDllName=_T("CAB32.DLL");
	m_AstrPrefix="Cab";
	m_AstrFindParam="**";
}

CArchiverCAB::~CArchiverCAB()
{
	FreeDLL();
}

bool CArchiverCAB::Compress(LPCTSTR ArcFileName,std::list<CString> &ParamList,CConfigManager &ConfMan,const PARAMETER_TYPE,int Options,LPCTSTR,LPCTSTR lpszMethod,LPCTSTR lpszLevel,CString &strLog)
{
	if(!IsOK()){
		return false;
	}

	ASSERT(0!=_tcslen(ArcFileName));
	TRACE(_T("ArcFileName=%s\n"),ArcFileName);

	//==============================================
	// レスポンスファイル用テンポラリファイル名取得
	//==============================================
	TCHAR ResponceFileName[_MAX_PATH+1];
	FILL_ZERO(ResponceFileName);
	if(!UtilGetTemporaryFileName(ResponceFileName,_T("cab"))){
		strLog=CString(MAKEINTRESOURCE(IDS_ERROR_TEMPORARY_FILE_CREATE));
		return false;
	}
	ASSERT(0!=_tcslen(ResponceFileName));

	//============================================
	// 自己解凍ファイル用テンポラリファイル名取得
	//============================================
	TCHAR SFXTemporaryFileName[_MAX_PATH+1];
	FILL_ZERO(SFXTemporaryFileName);
	bool bSFX=(0!=(Options&COMPRESS_SFX));
	if(bSFX){
		//3段階作成する
		if(!UtilGetTemporaryFileName(SFXTemporaryFileName,_T("sfx"))){
			strLog=CString(MAKEINTRESOURCE(IDS_ERROR_TEMPORARY_FILE_CREATE));
			return false;
		}
		ASSERT(0!=_tcslen(SFXTemporaryFileName));
		DeleteFile(SFXTemporaryFileName);//ゴミファイル消去

		PathRemoveExtension(SFXTemporaryFileName);
		PathAddExtension(SFXTemporaryFileName,_T(".cab"));
	}

	//======================================================
	// レスポンスファイル内に圧縮対象ファイル名を記入する
	// アーカイブファイル名はコマンドラインで直接指定する
	// サブフォルダ内のファイルも直接指定してやる。DLLの
	// 再起処理に頼ると予定外のファイルまで圧縮対象にされる
	//======================================================
	{
		HANDLE hFile=CreateFile(ResponceFileName,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
		if(INVALID_HANDLE_VALUE==hFile){
			strLog=CString(MAKEINTRESOURCE(IDS_ERROR_TEMPORARY_FILE_ACCESS));
			return false;
		}

		TRACE(_T("レスポンスファイルへの書き込み\n"));
		std::list<CString>::iterator ite;
		for(ite=ParamList.begin();ite!=ParamList.end();ite++){
			if(PathIsDirectory(*ite)){
				EnumAndWriteSubDir(hFile,*ite);
			}
			else WriteResponceFile(hFile,*ite);
		}
		CloseHandle(hFile);
	}

	//===========================
	// DLLに渡すオプションの設定
	//===========================
	TRACE(_T("DLLに渡すオプションの設定\n"));

	CString Param;//コマンドライン パラメータ バッファ

	//圧縮パラメータ
	Param+=_T("-a ");	//圧縮
//	Param+=_T("-r ");	//再起的に処理	これはLhaForge側でファイル名を列挙することで対処

	CConfigCAB Config;
	Config.load(ConfMan);
	int nCompressType=Config.CompressType;
	int LZX_Level=Config.LZX_Level;
	if(lpszMethod && *lpszMethod!=_T('\0')){
		if(0==_tcsicmp(lpszMethod,_T("mszip"))){
			nCompressType=CAB_COMPRESS_MSZIP;
		}else if(0==_tcsicmp(lpszMethod,_T("lzx"))){
			nCompressType=CAB_COMPRESS_LZX;
		}else if(0==_tcsicmp(lpszMethod,_T("plain"))){
			nCompressType=CAB_COMPRESS_PLAIN;
		}else{
			nCompressType=-1;
		}
	}
	if(lpszLevel && *lpszLevel!=_T('\0')){
		LZX_Level=_ttoi(lpszLevel);
	}


	switch(nCompressType){
	case -1:
		Param.AppendFormat(_T("-m%s "),lpszMethod);	//コマンドラインに従う
		break;
	case CAB_COMPRESS_MSZIP:
		Param+=_T("-mz ");	//MSZIP形式で圧縮
		break;
	case CAB_COMPRESS_LZX:
		Param+=_T("-ml:");	//LZX形式
		{
			CString buf;
			buf.Format(_T("%d"),LZX_Level);
			Param+=buf;
		}
		Param+=_T(" ");
		break;
	case CAB_COMPRESS_PLAIN:
		Param+=_T("-ms ");	//無圧縮
		break;
	}

	//圧縮先ファイル名指定
	if(bSFX){
		Param+=_T("\"");
		Param+=SFXTemporaryFileName;
		Param+=_T("\" ");
	}else{
		Param+=_T("\"");
		Param+=ArcFileName;
		Param+=_T("\" ");
	}

	//レスポンスファイル名指定
	Param+=_T("\"@");
	Param+=ResponceFileName;
	Param+=_T("\"");

	ASSERT(!Param.IsEmpty());
	TRACE(_T("ArchiveHandler Commandline Parameter:%s\n"),Param);

	TRACE(_T("ArchiveHandler呼び出し\n"));
	std::vector<char> szLog(LOG_BUFFER_SIZE);
	szLog[0]='\0';
	int Ret=ArchiveHandler(NULL,CT2A(Param),&szLog[0],LOG_BUFFER_SIZE-1);
	strLog=&szLog[0];

	//使ったレスポンスファイルは消去
	DeleteFile(ResponceFileName);

	//エラー時ログ出力
	if(!bSFX||0!=Ret){
		if(bSFX){
			DeleteFile(SFXTemporaryFileName);
		}
		return 0==Ret;
	}

	//====================
	// 自己解凍書庫に変換
	//====================
	//変換パラメータ
	Param=_T("-f ");		//変換
	Param+=_T("\"");
	Param+=SFXTemporaryFileName;	//変換元ファイル名
	Param+=_T("\" ");

	//----------
	//出力先フォルダ名
	TCHAR Buffer[_MAX_PATH+1];
	FILL_ZERO(Buffer);
	GetTempPath(_MAX_PATH,Buffer);
	PathAddBackslash(Buffer);

	Param+=_T(" \"");
	Param+=Buffer;
	Param+=_T("\" ");

	ASSERT(!Param.IsEmpty());
	TRACE(_T("ArchiveHandler Commandline Parameter(SFX):%s\n"),Param);

	TRACE(_T("SFX用ArchiveHandler呼び出し\n"));
	std::vector<char> LogSFX(LOG_BUFFER_SIZE);
	LogSFX[0]='\0';
	Ret=ArchiveHandler(NULL,CT2A(Param),&LogSFX[0],LOG_BUFFER_SIZE-1);
	strLog+=&LogSFX[0];
	DeleteFile(SFXTemporaryFileName);

	if(!Ret){	//成功した場合、ファイル名を変更
		//まずは変換された自己解凍ファイル名を作成
		PathStripPath(SFXTemporaryFileName);
		PathRemoveExtension(SFXTemporaryFileName);
		PathAddExtension(SFXTemporaryFileName,_T(".exe"));
		PathAppend(Buffer,SFXTemporaryFileName);
		//ファイル移動
		MoveFile(Buffer,ArcFileName);
	}

	return 0==Ret;
}

//ファイルを解凍；通常版
bool CArchiverCAB::Extract(LPCTSTR ArcFileName,CConfigManager &ConfMan,const CConfigExtract &Config,bool bSafeArchive,LPCTSTR OutputDir,CString &strLog)
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
	Param+=_T("-x ");		//解凍
//	Param+=_T("-k ");		//マルチボリューム;やめた方がいい

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
	std::vector<char> szLog(LOG_BUFFER_SIZE);
	szLog[0]='\0';
	int Ret=ArchiveHandler(NULL,CT2A(Param),&szLog[0],LOG_BUFFER_SIZE-1);
	strLog=&szLog[0];

	return 0==Ret;
}


//============================================================
// CabGetFileName()の出力結果を基に、格納されたファイルがパス
// 情報を持っているかどうか判別し、二重フォルダ作成を防ぐ
// 解凍しても安全かどうかも判断する
//============================================================
bool CArchiverCAB::ExamineArchive(LPCTSTR ArcFileName,CConfigManager& ConfMan,bool,bool &bInFolder,bool &bSafeArchive,CString &BaseDir,CString &strErr)
{
	return _ExamineArchive(ArcFileName,ConfMan,bInFolder,bSafeArchive,2,BaseDir,strErr);
}


// サブディレクトリおよびその中のファイルを列挙し、
// ファイル名をレスポンスファイルに書き込む
void CArchiverCAB::EnumAndWriteSubDir(HANDLE hFile,LPCTSTR Path)
{
	WIN32_FIND_DATA lp;

	CString Buffer=Path;
	Buffer+=_T("\\*");
	HANDLE h=FindFirstFile(Buffer,&lp);

	do{
		if((lp.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)&& 0!=_tcscmp(lp.cFileName,_T("..")) && 0!=_tcscmp(lp.cFileName,_T(".")))
		{
			CString SubPath=Path;
			SubPath+=_T("\\");
			SubPath+=lp.cFileName;

			EnumAndWriteSubDir(hFile,SubPath);
		}
		if((lp.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)!=FILE_ATTRIBUTE_DIRECTORY)
		{
			// lp.cFileNameでファイル名が分かる
			// wsprintf(出力文字列,"%s%s",temp,lp.cFileName);とすれば
			// ファイルのフルパスが分かる
			CString FileName=Path;
			FileName+=_T("\\");
			FileName+=lp.cFileName;

			if(0==_tcsnccmp(FileName,_T(".\\"),2)){
				//カレントディレクトリ指定があれば削除しておく
				FileName.Delete(0,2);
			}

			WriteResponceFile(hFile,FileName);
		}
	}while(FindNextFile(h,&lp));

	FindClose(h);
}

bool CArchiverCAB::ExtractSpecifiedOnly(LPCTSTR ArcFileName,CConfigManager&,LPCTSTR OutputDir,std::list<CString> &FileList,CString &strLog,bool bUsePath)
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
	if(!UtilGetTemporaryFileName(ResponceFileName,_T("cab"))){
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
	Param+=_T("-x ");		//解凍
	if(!bUsePath){
		Param+=_T("-j ");		//フォルダを無視して圧縮または解凍
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

	//出力先ディレクトリを作成しておく
	if(!UtilMakeSureDirectoryPathExists(OutputDir)){
		strLog.Format(IDS_ERROR_CANNOT_MAKE_DIR,OutputDir);
		return false;
	}

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


ARCRESULT CArchiverCAB::TestArchive(LPCTSTR ArcFileName,CString &strLog)
{
	TRACE(_T("TestArchive() called.\n"));
	ASSERT(IsOK());
	if(!IsOK()){
		return TEST_ERROR;
	}

	//tコマンドによるテストが実装されている
	CString Param=
		_T("-t ")			//テスト
		_T("\"")
	;
	Param+=ArcFileName;
	Param+=_T("\"");

	std::vector<char> szLog(LOG_BUFFER_SIZE);
	szLog[0]='\0';
	int Ret=ArchiveHandler(NULL,CT2A(Param),&szLog[0],LOG_BUFFER_SIZE-1);
	strLog=&szLog[0];

	if(0==Ret){
		strLog+=_T("\r\n");
		strLog+=CString(MAKEINTRESOURCE(IDS_TESTARCHIVE_RESULT_OK));
		return TEST_OK;
	}
	else return TEST_NG;
}
