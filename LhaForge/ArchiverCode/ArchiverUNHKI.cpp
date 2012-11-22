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
#include "ArchiverUNHKI.h"
#include "../Utilities/FileOperation.h"
#include "../Utilities/OSUtil.h"
#include "../resource.h"
#include "../ConfigCode/ConfigExtract.h"
#include "../ConfigCode/ConfigUNHKI.h"

CArchiverUNHKI::CArchiverUNHKI()
{
	m_nRequiredVersion=3;
	m_nRequiredSubVersion=19;
	m_strDllName=_T("UnHki32.DLL");
	m_AstrPrefix="UnHki";
}

CArchiverUNHKI::~CArchiverUNHKI()
{
	FreeDLL();
}

bool CArchiverUNHKI::Compress(LPCTSTR ArcFileName,std::list<CString> &ParamList,CConfigManager &ConfMan,const PARAMETER_TYPE,int Options,LPCTSTR,LPCTSTR,LPCTSTR lpszLevel,CString &strLog)
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
	if(!UtilGetTemporaryFileName(ResponceFileName,_T("hki"))){
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
		//2段階作成する
		if(!UtilGetTemporaryFileName(SFXTemporaryFileName,_T("sfx"))){
			strLog=CString(MAKEINTRESOURCE(IDS_ERROR_TEMPORARY_FILE_CREATE));
			return false;
		}
		ASSERT(0!=_tcslen(SFXTemporaryFileName));
	}

	//==========================================
	// レスポンスファイル内にアーカイブ名および
	// 圧縮対象ファイル名を記入する
	//==========================================
	{
		HANDLE hFile=CreateFile(ResponceFileName,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
		if(INVALID_HANDLE_VALUE==hFile){
			strLog=CString(MAKEINTRESOURCE(IDS_ERROR_TEMPORARY_FILE_ACCESS));
			return false;
		}

		TRACE(_T("レスポンスファイルへの書き込み\n"));

		std::list<CString>::iterator ite;
		for(ite=ParamList.begin();ite!=ParamList.end();ite++){
			WriteResponceFile(hFile,*ite);
		}
		CloseHandle(hFile);
	}

	//===========================
	// DLLに渡すオプションの設定
	//===========================
	TRACE(_T("DLLに渡すオプションの設定\n"));

	CString Param;//コマンドライン パラメータ バッファ

	//圧縮パラメータ
	Param+=
		_T("a ")	//圧縮
		_T("-a1 ")	//属性保存
		_T("-r0 ")	//再帰的
		_T("-jf0 ")	//ルート記号削除
	;

	CConfigHKI Config;
	Config.load(ConfMan);
	//パスワード
	if(Options&COMPRESS_PASSWORD){
		Param+=_T("-mp ");
		switch(Config.EncryptAlgorithm){
		case HKI_ENCRYPT_NONE:
			Param+=_T("-me0 ");
			break;
		case HKI_ENCRYPT_RIJNDAEL128:
			Param+=_T("-me1 ");
			break;
		case HKI_ENCRYPT_RIJNDAEL256:
			Param+=_T("-me2 ");
			break;
		case HKI_ENCRYPT_SINGLE_DES:
			Param+=_T("-me3 ");
			break;
		case HKI_ENCRYPT_TRIPLE_DES:
			Param+=_T("-me4 ");
			break;
		case HKI_ENCRYPT_BLOWFISH448:
			Param+=_T("-me5 ");
			break;
		case HKI_ENCRYPT_TWOFISH128:
			Param+=_T("-me6 ");
			break;
		case HKI_ENCRYPT_TWOFISH256:
			Param+=_T("-me7 ");
			break;
		case HKI_ENCRYPT_SQUARE:
			Param+=_T("-me8 ");
			break;
		}
	}

	//圧縮レベル
	if(lpszLevel && *lpszLevel!=_T('\0')){
		Param.AppendFormat(_T("-ml%s "),lpszLevel);
	}else{
		switch(Config.CompressLevel){
		case HKI_COMPRESS_LEVEL_NONE:
			Param+=_T("-ml0 ");
			break;
		case HKI_COMPRESS_LEVEL_FAST:
			Param+=_T("-ml1 ");
			break;
		case HKI_COMPRESS_LEVEL_NORMAL:
			Param+=_T("-ml2 ");
			break;
		case HKI_COMPRESS_LEVEL_HIGH:
			Param+=_T("-ml3 ");
			break;
		}
	}

	//圧縮先ファイル名の設定
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

	//エラー時
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
	Param=_T("s ");		//変換

	//SFXモジュールパス
	TCHAR SFXModulePath[_MAX_PATH+1];
	FILL_ZERO(SFXModulePath);
	LPTSTR lptemp;
	{
		//安全なパスに移動;DLL読み込み対策
		CCurrentDirManager cdm(UtilGetModuleDirectoryPath());
		if(!SearchPath(NULL,_T("HKI.SFX"),NULL,_MAX_PATH,SFXModulePath,&lptemp)){
			strLog.Format(IDS_ERROR_SFX_MODULE_NOT_FOUND,_T("HKI.SFX"));
			DeleteFile(SFXTemporaryFileName);
			return false;
		}
	}
	PathQuoteSpaces(SFXModulePath);
	Param+=_T("-mx");
	Param+=SFXModulePath;
	Param+=_T(" ");

	Param+=("-gr\"");	//変換先ファイル名
	Param+=ArcFileName;
	Param+=_T("\" \"");
	Param+=SFXTemporaryFileName;	//変換元ファイル名
	Param+=_T("\" ");


	ASSERT(!Param.IsEmpty());
	TRACE(_T("ArchiveHandler Commandline Parameter(SFX):%s\n"),Param);

	TRACE(_T("SFX用ArchiveHandler呼び出し\n"));
	std::vector<char> LogSFX(LOG_BUFFER_SIZE);
	LogSFX[0]='\0';
	Ret=ArchiveHandler(NULL,CT2A(Param),&LogSFX[0],LOG_BUFFER_SIZE-1);
	strLog+=&LogSFX[0];

	DeleteFile(SFXTemporaryFileName);

	return 0==Ret;
}

//bSafeArchiveは無視される
bool CArchiverUNHKI::Extract(LPCTSTR ArcFileName,CConfigManager&,const CConfigExtract &Config,bool,LPCTSTR OutputDir,CString &strLog)
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
		_T("x ")			//解凍
		_T("-a1 ")			//属性保存
//		_T("-r1 ")			//再帰的
		_T("-x1 ")			//パス情報保持
	;
	if(Config.ForceOverwrite){
		//強制上書き
		Param+=_T("-gf1 ");
	}else{
		Param+=_T("-gf0 ");
	}

	//アーカイブファイル名指定
	Param+=_T("\"");
	Param+=ArcFileName;
	Param+=_T("\" ");

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
// UnHkiGetFileName()の出力結果を基に、格納されたファイルがパス
// 情報を持っているかどうか判別し、二重フォルダ作成を防ぐ
//=========================================================
bool CArchiverUNHKI::ExamineArchive(LPCTSTR ArcFileName,CConfigManager& ConfMan,bool bSkipDir,bool &bInFolder,bool &bSafeArchive,CString &BaseDir,CString &strErr)
{
	if(bSkipDir){
		TRACE(_T("二重フォルダ判定回避\n"));
		return true;
	}
	return _ExamineArchiveFast(ArcFileName,ConfMan,bInFolder,BaseDir,strErr);
}

BOOL CArchiverUNHKI::CheckArchive(LPCTSTR _szFileName)
{
	if(!ArchiverCheckArchive){
		ASSERT(ArchiverCheckArchive);
		return false;
	}
	return ArchiverCheckArchive(CT2A(_szFileName),CHECKARCHIVE_RAPID);
}

bool CArchiverUNHKI::InspectArchiveGetWriteTime(FILETIME &FileTime)
{
	if(!m_hInspectArchive){
		ASSERT(!"Open an Archive First!!!\n");
		return false;
	}
	//拡張版関数で時刻取得
	if(ArchiverGetWriteTimeEx){
		if(!ArchiverGetWriteTimeEx(m_hInspectArchive,&FileTime))return false;
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

bool CArchiverUNHKI::ExtractSpecifiedOnly(LPCTSTR ArcFileName,CConfigManager&,LPCTSTR OutputDir,std::list<CString> &FileList,CString &strLog,bool bUsePath)
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
	if(!UtilGetTemporaryFileName(ResponceFileName,_T("hki"))){
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
		Param+=_T("x ");				//パスあり解凍
	}else{
		Param+=_T("e ");			//パス無し解凍
	}
	Param+=
		_T("-a1 ")			//属性保存
//		_T("-r1 ")			//再帰的
//		_T("-x1 ")			//パス情報保持
		_T("-gf0 ")			//上書き確認
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

bool CArchiverUNHKI::DeleteItemFromArchive(LPCTSTR ArcFileName,CConfigManager&,const std::list<CString>& FileList,CString &strLog)
{
	if(!IsOK()){
		return false;
	}

	//==============================================
	// レスポンスファイル用テンポラリファイル名取得
	//==============================================
	TCHAR ResponceFileName[_MAX_PATH+1];
	FILL_ZERO(ResponceFileName);
	if(!UtilGetTemporaryFileName(ResponceFileName,_T("hki"))){
		strLog=CString(MAKEINTRESOURCE(IDS_ERROR_TEMPORARY_FILE_CREATE));
		return false;
	}
	ASSERT(0!=_tcslen(ResponceFileName));

	//削除対象ファイルをレスポンスファイルに書き出す
	{
		HANDLE hFile=CreateFile(ResponceFileName,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
		if(INVALID_HANDLE_VALUE==hFile){
			strLog=CString(MAKEINTRESOURCE(IDS_ERROR_TEMPORARY_FILE_ACCESS));
			return false;
		}

		std::list<CString>::const_iterator ite=FileList.begin();
		const std::list<CString>::const_iterator end=FileList.end();
		for(;ite!=end;ite++){
			WriteResponceFile(hFile,*ite);
		}
		CloseHandle(hFile);
	}

	//===========================
	// DLLに渡すオプションの設定
	//===========================
	CString Param;

	//削除パラメータ
	Param+=_T("d ");		//削除

	//アーカイブファイル名指定
	Param+=_T("\"");
	Param+=ArcFileName;
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

ARCRESULT CArchiverUNHKI::TestArchive(LPCTSTR ArcFileName,CString &strLog)
{
	TRACE(_T("TestArchive() called.\n"));
	ASSERT(IsOK());
	if(!IsOK()){
		return TEST_ERROR;
	}

	//tコマンドによるテストが実装されている
	CString Param=
		_T("t ")			//テスト
		_T("-mt2 ")			//検査モード:完全なCRCチェック(CHECKARCHIVE_FULLCRC相当)

		_T("\"")
	;
	Param+=ArcFileName;
	Param+=_T("\"");

	//ファイル名をログに追加
	strLog=ArcFileName;
	strLog+=_T("\r\n\r\n");
	//char szLog[LOG_BUFFER_SIZE]={0};
	std::vector<char> szLog(LOG_BUFFER_SIZE);
	szLog[0]='\0';
	int Ret=ArchiveHandler(NULL,CT2A(Param),&szLog[0],LOG_BUFFER_SIZE-1);
	strLog=&szLog[0];

	strLog+=_T("\r\n");
	if(0==Ret){
		strLog+=CString(MAKEINTRESOURCE(IDS_TESTARCHIVE_RESULT_OK));
		return TEST_OK;
	}
	else{
		strLog+=CString(MAKEINTRESOURCE(IDS_TESTARCHIVE_RESULT_NG));
		return TEST_NG;
	}
}
