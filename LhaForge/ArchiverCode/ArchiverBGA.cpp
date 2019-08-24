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
#include "ArchiverBGA.h"
#include "../Utilities/FileOperation.h"
#include "../Utilities/StringUtil.h"
#include "../Utilities/TemporaryDirMgr.h"
#include "../resource.h"
#include "../ConfigCode/ConfigExtract.h"
#include "../ConfigCode/ConfigBGA.h"
#include "../Utilities/OSUtil.h"

CArchiverBGA::CArchiverBGA()
{
	m_LoadLevel=LOAD_DLL_SIMPLE_INSPECTION;
	m_nRequiredVersion=37;
	m_strDllName=_T("Bga32.dll");
	m_AstrPrefix="Bga";
	m_AstrFindParam="*.*";
}

CArchiverBGA::~CArchiverBGA()
{ 
	FreeDLL();
}

bool CArchiverBGA::Compress(LPCTSTR ArcFileName,std::list<CString> &ParamList,CConfigManager &ConfMan,const PARAMETER_TYPE Type,int Options,LPCTSTR,LPCTSTR,LPCTSTR lpszLevel,CString &strLog)
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
	if(!UtilGetTemporaryFileName(ResponceFileName,_T("bga"))){
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

	//====================================================
	// レスポンスファイル内に圧縮対象ファイル名を記入する
	//====================================================
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
		_T("a ")		//圧縮
		_T("-a ")		//全属性を対象
	;

	CConfigBGA Config;
	Config.load(ConfMan);
	if(lpszLevel && *lpszLevel!=_T('\0')){
		switch(Type){
		case PARAMETER_GZA:
			Param.AppendFormat(_T("-m1 -l%s "),lpszLevel);
			break;
		case PARAMETER_BZA:
			Param.AppendFormat(_T("-m2 -l%s "),lpszLevel);
			break;
		}
	}else{
		switch(Type){
		case PARAMETER_GZA:
			Param.AppendFormat(_T("-m1 -l%d "),Config.GZALevel);
			break;
		case PARAMETER_BZA:
			Param.AppendFormat(_T("-m2 -l%d "),Config.BZALevel);
			break;
		}
	}

	//出力先ファイル名指定
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
	//char szLog[LOG_BUFFER_SIZE]={0};
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
	Param=_T("s ");		//変換

	Param+=_T("\"");
	Param+=SFXTemporaryFileName;	//変換元ファイル名
	Param+=_T("\" ");

	//------------------------------------------------------------------------------------------
	//注意:
	// 自己解凍ファイルのファイル名はテンポラリファイル名の拡張子を.exeに変えたものになっている
	//------------------------------------------------------------------------------------------

	//出力先フォルダ(テンポラリフォルダ)を指定
	CString strDir(SFXTemporaryFileName);
	UtilPathGetDirectoryPart(strDir);
	Param+=_T("\"");
	Param+=strDir;
	Param+=_T("\" ");


	ASSERT(!Param.IsEmpty());
	TRACE(_T("ArchiveHandler Commandline Parameter(SFX):%s\n"),Param);

	TRACE(_T("SFX用ArchiveHandler呼び出し\n"));
	std::vector<char> LogSFX(LOG_BUFFER_SIZE);
	LogSFX[0]='\0';
	Ret=ArchiveHandler(NULL,CT2A(Param),&LogSFX[0],LOG_BUFFER_SIZE-1);
	strLog+=&LogSFX[0];

	DeleteFile(SFXTemporaryFileName);
	if(!Ret){	//正常終了時
		//変換後の自己解凍ファイル名を出す
		PathRemoveExtension(SFXTemporaryFileName);
		PathAddExtension(SFXTemporaryFileName,_T(".exe"));
		MoveFile(SFXTemporaryFileName,ArcFileName);
	}

	return 0==Ret;
}

bool CArchiverBGA::Extract(LPCTSTR ArcFileName,CConfigManager&,const CConfigExtract &Config,bool bSafeArchive,LPCTSTR OutputDir,CString &strLog)
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
		_T("x ")		//パス付き解凍
		_T("-a ")		//全属性解凍
	;
	if(Config.ForceOverwrite){
		//強制上書き
		Param+=_T("-o ");
	}

	//アーカイブファイル名指定
	Param+=_T("\"");
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
// BgaGetFileName()の出力結果を基に、格納されたファイルがパス
//情報を持っているかどうか判別し、二重フォルダ作成を防ぐ
// 安全確認も行う
//=========================================================
bool CArchiverBGA::ExamineArchive(LPCTSTR ArcFileName,CConfigManager& ConfMan,bool,bool &bInFolder,bool &bSafeArchive,CString &BaseDir,CString &strErr)
{
	return _ExamineArchive(ArcFileName,ConfMan,bInFolder,bSafeArchive,-1,BaseDir,strErr);
}

bool CArchiverBGA::ExtractSpecifiedOnly(LPCTSTR ArcFileName,CConfigManager&,LPCTSTR OutputDir,std::list<CString> &FileList,CString &strLog,bool bUsePath)
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
	if(!UtilGetTemporaryFileName(ResponceFileName,_T("bga"))){
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
		_T("x ")			//解凍
		_T("-a ")			//全属性解凍
	;
	if(!bUsePath)Param+=_T("-j ");		//パス無視
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

bool CArchiverBGA::DeleteItemFromArchive(LPCTSTR ArcFileName,CConfigManager&,const std::list<CString> &FileList,CString &strLog)
{
	if(!IsOK()){
		return false;
	}

	//==============================================
	// レスポンスファイル用テンポラリファイル名取得
	//==============================================
	TCHAR ResponceFileName[_MAX_PATH+1];
	FILL_ZERO(ResponceFileName);
	if(!UtilGetTemporaryFileName(ResponceFileName,_T("bga"))){
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
	Param+=
		_T("d ")		//削除
		_T("-i ")		//確認しない
	;
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

bool CArchiverBGA::AddItemToArchive(LPCTSTR ArcFileName,bool bEncrypted,const std::list<CString> &FileList,CConfigManager &ConfMan,LPCTSTR lpDestDir,CString &strLog)
{
	// レスポンスファイル用テンポラリファイル名取得
	TCHAR ResponceFileName[_MAX_PATH+1];
	FILL_ZERO(ResponceFileName);
	if(!UtilGetTemporaryFileName(ResponceFileName,_T("bga"))){
		strLog=CString(MAKEINTRESOURCE(IDS_ERROR_TEMPORARY_FILE_CREATE));
		return false;
	}
	ASSERT(0!=_tcslen(ResponceFileName));

	//===一時的にファイルをコピー
	//---\で終わる基点パスを取得
	CPath strBasePath;
	UtilGetBaseDirectory(strBasePath,FileList);
	TRACE(_T("%s\n"),strBasePath);

	//---テンポラリに対象ファイルをコピー
	//テンポラリ準備
	CTemporaryDirectoryManager tdm(_T("lhaf"));
	CPath strDestPath(tdm.GetDirPath());
	strDestPath+=lpDestDir;
	UtilMakeSureDirectoryPathExists(strDestPath);

	// 圧縮対象ファイル名を修正する
	const int BasePathLength=((CString)strBasePath).GetLength();
	CString strSrcFiles;	//コピー元ファイルの一覧
	CString strDestFiles;	//コピー先ファイルの一覧
	std::list<CString>::const_iterator ite;
	for(ite=FileList.begin();ite!=FileList.end();++ite){
		//ベースパスを元に相対パス取得 : 共通である基底パスの文字数分だけカットする
		LPCTSTR lpSrc((LPCTSTR)(*ite)+BasePathLength);

		//送り側ファイル名指定
		strSrcFiles+=(strBasePath+lpSrc);	//PathAppend相当
		strSrcFiles+=_T('|');
		//受け側ファイル名指定
		strDestFiles+=strDestPath+lpSrc;
		strDestFiles+=_T('|');
	}
	strSrcFiles+=_T('|');
	strDestFiles+=_T('|');

	//'|'を'\0'に変換する
	std::vector<TCHAR> srcBuf(strSrcFiles.GetLength()+1);
	UtilMakeFilterString(strSrcFiles,&srcBuf[0],srcBuf.size());
	std::vector<TCHAR> destBuf(strDestFiles.GetLength()+1);
	UtilMakeFilterString(strDestFiles,&destBuf[0],destBuf.size());

	//ファイル操作内容
	SHFILEOPSTRUCT fileOp={0};
	fileOp.wFunc=FO_COPY;
	fileOp.fFlags=FOF_MULTIDESTFILES|FOF_NOCONFIRMATION|FOF_NOCONFIRMMKDIR|FOF_NOCOPYSECURITYATTRIBS|FOF_NO_CONNECTED_ELEMENTS;
	fileOp.pFrom=&srcBuf[0];
	fileOp.pTo=&destBuf[0];

	//コピー実行
	if(::SHFileOperation(&fileOp)){
		//エラー
		strLog=CString(MAKEINTRESOURCE(IDS_ERROR_FILE_COPY));
		return false;
	}else if(fileOp.fAnyOperationsAborted){
		//キャンセル
		strLog=CString(MAKEINTRESOURCE(IDS_ERROR_USERCANCEL));
		return false;
	}

	//カレントディレクトリ設定
	::SetCurrentDirectory(tdm.GetDirPath());
	// 同時に、レスポンスファイル内にアーカイブ名および圧縮対象ファイル名を記入する
	{
		HANDLE hFile=CreateFile(ResponceFileName,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
		if(INVALID_HANDLE_VALUE==hFile){
			strLog=CString(MAKEINTRESOURCE(IDS_ERROR_TEMPORARY_FILE_ACCESS));
			return false;
		}
		//レスポンスファイルへの書き込み
		//全て圧縮
		WriteResponceFile(hFile,_T("*"));
		CloseHandle(hFile);
	}


	//===========================
	// DLLに渡すオプションの設定
	//===========================
	TRACE(_T("DLLに渡すオプションの設定\n"));

	CString Param;//コマンドライン パラメータ バッファ

	//圧縮パラメータ
	Param+=
		_T("a ")		//圧縮
		_T("-a ")		//全属性を対象
	;

	//ファイルの種類に応じたパラメータを指定
	CConfigBGA Config;
	Config.load(ConfMan);
	CString level;
	switch(CheckArchive(ArcFileName)){
	case 1:	//GZA
		TRACE(_T("This is GZA\n"));
		Param+=_T("-m1 ");
		level.Format(_T("-l%d "),Config.GZALevel);
		Param+=level;
		break;
	case 2:	//BZA
		TRACE(_T("This is BZA\n"));
		Param+=_T("-m2 ");
		level.Format(_T("-l%d "),Config.BZALevel);
		Param+=level;
		break;
	default:
		ASSERT(!"This code cannot be run");
		//エラー処理が面倒なので放っておく。別に指定が無くてもある程度は動くので。
	}

	//アーカイブファイル名指定
	Param+=_T("\"");
	Param+=ArcFileName;
	Param+=_T("\" ");

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

	return 0==Ret;
}
