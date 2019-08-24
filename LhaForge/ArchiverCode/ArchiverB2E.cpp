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
#include "ArchiverB2E.h"
#include "../Utilities/FileOperation.h"
#include "../Utilities/TemporaryDirMgr.h"
#include "../Utilities/StringUtil.h"
#include "../resource.h"
#include "../ConfigCode/ConfigExtract.h"
#include "../ConfigCode/ConfigB2E.h"

CArchiverB2E::CArchiverB2E():
	B2ESetScriptDirectory(NULL),
	B2EScriptGetCount(NULL),
	B2EScriptGetAbility(NULL),
	B2EScriptGetCompressType(NULL),
	B2EScriptGetCompressMethod(NULL),
	B2EScriptGetDefaultCompressMethod(NULL),
	B2EScriptGetExtractorIndex(NULL),
	B2EScriptGetName(NULL)
{
	m_nRequiredVersion=7;
	m_strDllName=_T("B2E32.DLL");
	m_AstrPrefix="B2E";
}

CArchiverB2E::~CArchiverB2E()
{
	FreeDLL();
}

LOAD_RESULT CArchiverB2E::LoadDLL(CConfigManager &ConfMan,CString &strErr)
{
	FreeDLL();

	//基底クラスのメソッドを呼ぶ
	LOAD_RESULT res=CArchiverDLL::LoadDLL(ConfMan,strErr);
	if(LOAD_RESULT_OK!=res){
		return res;
	}

	//独自実装API群
	B2ESetScriptDirectory=(B2ESETSCRIPTDIRECTORY)GetProcAddress(m_hInstDLL,"B2ESetScriptDirectory");
	if(NULL==B2ESetScriptDirectory){
		strErr.Format(IDS_ERROR_DLL_FUNCTION_GET,m_strDllName,_T("B2ESetScriptDirectory"));
		FreeDLL();
		return LOAD_RESULT_INVALID;
	}

	B2EScriptGetCount=(B2ESCRIPTGETCOUNT)GetProcAddress(m_hInstDLL,"B2EScriptGetCount");
	if(NULL==B2EScriptGetCount){
		strErr.Format(IDS_ERROR_DLL_FUNCTION_GET,m_strDllName,_T("B2EScriptGetCount"));
		FreeDLL();
		return LOAD_RESULT_INVALID;
	}

	B2EScriptGetAbility=(B2ESCRIPTGETABILITY)GetProcAddress(m_hInstDLL,"B2EScriptGetAbility");
	if(NULL==B2EScriptGetAbility){
		strErr.Format(IDS_ERROR_DLL_FUNCTION_GET,m_strDllName,_T("B2EScriptGetAbility"));
		FreeDLL();
		return LOAD_RESULT_INVALID;
	}

	B2EScriptGetCompressType=(B2ESCRIPTGETCOMPRESSTYPE)GetProcAddress(m_hInstDLL,"B2EScriptGetCompressType");
	if(NULL==B2EScriptGetCompressType){
		strErr.Format(IDS_ERROR_DLL_FUNCTION_GET,m_strDllName,_T("B2EScriptGetCompressType"));
		FreeDLL();
		return LOAD_RESULT_INVALID;
	}

	B2EScriptGetCompressMethod=(B2ESCRIPTGETCOMPRESSMETHOD)GetProcAddress(m_hInstDLL,"B2EScriptGetCompressMethod");
	if(NULL==B2EScriptGetCompressMethod){
		strErr.Format(IDS_ERROR_DLL_FUNCTION_GET,m_strDllName,_T("B2EScriptGetCompressMethod"));
		FreeDLL();
		return LOAD_RESULT_INVALID;
	}

	B2EScriptGetDefaultCompressMethod=(B2ESCRIPTGETDEFAULTCOMPRESSMETHOD)GetProcAddress(m_hInstDLL,"B2EScriptGetDefaultCompressMethod");
	if(NULL==B2EScriptGetDefaultCompressMethod){
		strErr.Format(IDS_ERROR_DLL_FUNCTION_GET,m_strDllName,_T("B2EScriptGetDefaultCompressMethod"));
		FreeDLL();
		return LOAD_RESULT_INVALID;
	}

	B2EScriptGetExtractorIndex=(B2ESCRIPTGETEXTRACTORINDEX)GetProcAddress(m_hInstDLL,"B2EScriptGetExtractorIndex");
	if(NULL==B2EScriptGetExtractorIndex){
		strErr.Format(IDS_ERROR_DLL_FUNCTION_GET,m_strDllName,_T("B2EScriptGetExtractorIndex"));
		FreeDLL();
		return LOAD_RESULT_INVALID;
	}

	B2EScriptGetName=(B2ESCRIPTGETNAME)GetProcAddress(m_hInstDLL,"B2EScriptGetName");
	if(NULL==B2EScriptGetName){
		strErr.Format(IDS_ERROR_DLL_FUNCTION_GET,m_strDllName,_T("B2EScriptGetName"));
		FreeDLL();
		return LOAD_RESULT_INVALID;
	}

	// ロード完了
	//===================
	// 初期設定
	CConfigB2E Config;
	Config.load(ConfMan);
	if(_tcslen(Config.ScriptDirectory)>0 && PathIsDirectory(Config.ScriptDirectory)){
		B2ESetScriptDirectory(CT2A(Config.ScriptDirectory));
	}

	return LOAD_RESULT_OK;
}

void CArchiverB2E::FreeDLL()
{
	if(m_hInstDLL){
		B2ESetScriptDirectory=NULL;
		B2EScriptGetCount=NULL;
		B2EScriptGetAbility=NULL;
		B2EScriptGetCompressType=NULL;
		B2EScriptGetCompressMethod=NULL;
		B2EScriptGetDefaultCompressMethod=NULL;
		B2EScriptGetExtractorIndex=NULL;
		CArchiverDLL::FreeDLL();
	}
}

bool CArchiverB2E::Compress(LPCTSTR ArcFileName,std::list<CString> &ParamList,CConfigManager &ConfigManager,const PARAMETER_TYPE,int Options,LPCTSTR lpszFormat,LPCTSTR lpszMethod,LPCTSTR,CString &strLog)
{
	return B2ECompress(ArcFileName,ParamList,ConfigManager,lpszFormat,lpszMethod,(Options & COMPRESS_SFX) !=0 ,strLog);
}

bool CArchiverB2E::B2ECompress(LPCTSTR ArcFileName,std::list<CString> &ParamList,CConfigManager &ConfigManager,LPCTSTR lpszFormat,LPCTSTR lpszMethod,bool bSFX,CString &strLog)
{
	if(!IsOK()||!lpszFormat){
		return false;
	}

	ASSERT(0!=_tcslen(ArcFileName));
	TRACE(_T("ArcFileName=%s\n"),ArcFileName);

	//==============================================
	// レスポンスファイル用テンポラリファイル名取得
	//==============================================
	TCHAR ResponceFileName[_MAX_PATH+1];
	FILL_ZERO(ResponceFileName);
	if(!UtilGetTemporaryFileName(ResponceFileName,_T("b2e"))){
		strLog=CString(MAKEINTRESOURCE(IDS_ERROR_TEMPORARY_FILE_CREATE));
		return false;
	}
	ASSERT(0!=_tcslen(ResponceFileName));

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
		//圧縮先ファイル名書き込み
		/*
		 * B2E用の出力ファイル名には、拡張子.archiveが付いたものが指定される。
		 * これは、既存のファイルが誤って削除されないようにするためである
		 * ここで改めて余計な拡張子を削除する
		 */
		CPath tmpName=ArcFileName;
		tmpName.RemoveExtension();
		WriteResponceFile(hFile,tmpName);

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

	//---圧縮パラメータ
	//圧縮形式
	Param =_T("\"-c");
	Param+=lpszFormat;
	Param+=_T("\" ");

	//圧縮メソッド
	if(lpszMethod&&_tcslen(lpszMethod)>0){
		Param+=_T("\"-m");
		Param+=lpszMethod;
		Param+=_T("\" ");
	}

	//自己解凍
	if(bSFX){
		Param+=_T("-s ");
	}

	//レスポンスファイル名指定
	Param+=_T("\"-@");
	Param+=ResponceFileName;
	Param+=_T("\"");

	ASSERT(!Param.IsEmpty());
	TRACE(_T("ArchiveHandler Commandline Parameter:%s\n"),Param);

	TRACE(_T("ArchiveHandler呼び出し\n"));

	//---実行
	std::vector<char> szLog(LOG_BUFFER_SIZE);
	szLog[0]='\0';
	int Ret=ArchiveHandler(NULL,CT2A(Param),&szLog[0],LOG_BUFFER_SIZE-1);
	strLog=&szLog[0];

	//使ったレスポンスファイルは消去
	DeleteFile(ResponceFileName);

	return 0==Ret;
}

bool CArchiverB2E::Extract(LPCTSTR ArcFileName,CConfigManager&,const CConfigExtract &Config,bool bSafeArchive,LPCTSTR OutputDir,CString &strLog)
{
	if(!IsOK()){
		return false;
	}

	if(!bSafeArchive){
		//不正なパスがあれば、続行するかどうか確認する
		//ただし、パスチェック不可能なときにはこの警告すら出ない。
		CString msg;
		msg.Format(IDS_ASK_CONTINUE_EXTRACT,ArcFileName);
		if(IDYES!=MessageBox(NULL,msg,UtilGetMessageCaption(),MB_YESNO|MB_ICONQUESTION|MB_DEFBUTTON2)){
			return false;
		}
	}

	//===========================
	// DLLに渡すオプションの設定
	//===========================
	CString Param;//コマンドライン パラメータ バッファ

	//解凍パラメータ
	Param+=_T("-e ");	//解凍
	//強制上書きには未対応

	//出力先指定
	Param+=_T("\"-o");
	Param+=OutputDir;
	Param+=_T("\" ");

	//アーカイブファイル名指定
	Param+=_T("\"");
	Param+=ArcFileName;
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
// B2EGetFileName()の出力結果を基に、格納されたファイルがパス
// 情報を持っているかどうか判別し、二重フォルダ作成を防ぐ
//=========================================================
bool CArchiverB2E::ExamineArchive(LPCTSTR ArcFileName,CConfigManager& ConfMan,bool,bool &bInFolder,bool &bSafeArchive,CString &BaseDir,CString &strErr)
{
	if(!IsOK()){
		return false;
	}
/*	UINT uIndex=B2EScriptGetExtractorIndex(ArcFileName);
	if(uIndex==0xFFFFFFFF)return false;	//未対応形式かエラー

	WORD wAbility;
	if(!B2EScriptGetAbility(uIndex,&wAbility))return false;
	if(!(wAbility&B2EABILITY_LIST))return false;	//検査できない*/

	//------
	//検査できないときには、ExamineArchiveがその旨報告してくれるのでいちいち処理する必要はない

	return _ExamineArchive(ArcFileName,ConfMan,bInFolder,bSafeArchive,0,BaseDir,strErr);
}

bool CArchiverB2E::ExtractSpecifiedOnly(LPCTSTR ArcFileName,CConfigManager&,LPCTSTR OutputDir,std::list<CString> &FileList,CString &strLog,bool bUsePath)
{
	if(!IsOK()){
		return false;
	}

	//==============================================
	// レスポンスファイル用テンポラリファイル名取得
	//==============================================
	TCHAR ResponceFileName[_MAX_PATH+1];
	FILL_ZERO(ResponceFileName);
	if(!UtilGetTemporaryFileName(ResponceFileName,_T("b2e"))){
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
	Param+=_T("-e ");			//解凍
	if(!bUsePath)Param+=_T("-g ");		//パスを無効に

	//アーカイブファイル名指定
	Param+=_T("\"");
	Param+=ArcFileName;
	Param+=_T("\" ");

	//出力先フォルダ
	Param+=_T("\"-o");
	Param+=OutputDir;
	Param+=_T("\" ");

	//レスポンスファイル名指定
	Param+=_T("\"-@");
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

bool CArchiverB2E::QueryExtractSpecifiedOnlySupported(LPCTSTR ArcFileName)const
{
	UINT uIndex=B2EScriptGetExtractorIndex(CT2A(ArcFileName));
	if(uIndex==0xFFFFFFFF)return false;	//未対応形式かエラー

	WORD wAbility;
	if(!B2EScriptGetAbility(uIndex,&wAbility))return false;
	if(wAbility&B2EABILITY_MELT_EACH)return true;	//個別解凍できない
	return false;
}

bool CArchiverB2E::EnumCompressB2EScript(std::vector<B2ESCRIPTINFO> &ScriptInfoArray)
{
	ScriptInfoArray.clear();

	//スクリプトの数を取得
	const UINT uScriptCount=B2EScriptGetCount();

	if(uScriptCount==-1)return false;	//エラー

	for(UINT uIndex=0;uIndex<uScriptCount;uIndex++){
		B2ESCRIPTINFO Info;
		Info.uIndex=uIndex;

		//能力を取得
		B2EScriptGetAbility(uIndex,&Info.wAbility);

		if(Info.wAbility&B2EABILITY_COMPRESS){	//---圧縮能力あり
			//圧縮形式名を取得
			B2EScriptGetCompressType(uIndex,Info.szFormat,_MAX_PATH);
			//圧縮メソッド名を取得
			{
				int i=0;
				char szBuffer[_MAX_PATH+1]={0};
				while(B2EScriptGetCompressMethod(uIndex,i,szBuffer,_MAX_PATH)){
					Info.MethodArray.push_back(szBuffer);
					i++;
				}
			}

			//デフォルト圧縮メソッド番号を取得
			Info.nDefaultMethod=B2EScriptGetDefaultCompressMethod(uIndex);
			//---情報追加
			ScriptInfoArray.push_back(Info);
		}
	}
	return true;
}

bool CArchiverB2E::EnumActiveB2EScriptNames(std::vector<CString> &ScriptNames)
{
	ScriptNames.clear();

	//スクリプトの数を取得
	const UINT uScriptCount=B2EScriptGetCount();

	if(uScriptCount==-1)return false;	//エラー

	for(UINT uIndex=0;uIndex<uScriptCount;uIndex++){
		B2ESCRIPTINFO Info;
		Info.uIndex=uIndex;

		//能力を取得
		B2EScriptGetAbility(uIndex,&Info.wAbility);

		if(Info.wAbility){	//---有効なスクリプト
			char buffer[_MAX_PATH];
			B2EScriptGetName(uIndex,buffer,COUNTOF(buffer));
			ScriptNames.push_back(CString(buffer));
		}
	}
	return true;
}


bool CArchiverB2E::AddItemToArchive(LPCTSTR ArcFileName,bool bEncrypted,const std::list<CString> &FileList,CConfigManager &ConfMan,LPCTSTR lpDestDir,CString &strLog)
{
	// レスポンスファイル用テンポラリファイル名取得
	TCHAR ResponceFileName[_MAX_PATH+1];
	FILL_ZERO(ResponceFileName);
	if(!UtilGetTemporaryFileName(ResponceFileName,_T("b2e"))){
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
		WriteResponceFile(hFile,_T("*.*"));
		CloseHandle(hFile);
	}

	//===========================
	// DLLに渡すオプションの設定
	//===========================
	TRACE(_T("DLLに渡すオプションの設定\n"));

	CString Param;//コマンドライン パラメータ バッファ

	//---圧縮パラメータ
	//圧縮形式
	Param =_T("-a ");

	//アーカイブファイル名指定
	Param+=_T("\"");
	Param+=ArcFileName;
	Param+=_T("\" ");

	//レスポンスファイル名指定
	Param+=_T("\"-@");
	Param+=ResponceFileName;
	Param+=_T("\"");

	ASSERT(!Param.IsEmpty());
	TRACE(_T("ArchiveHandler Commandline Parameter:%s\n"),Param);

	//---実行
	std::vector<char> szLog(LOG_BUFFER_SIZE);
	szLog[0]='\0';
	int Ret=ArchiveHandler(NULL,CT2A(Param),&szLog[0],LOG_BUFFER_SIZE-1);
	strLog=&szLog[0];

	//使ったレスポンスファイルは消去
	DeleteFile(ResponceFileName);

	return 0==Ret;
}

bool CArchiverB2E::QueryAddItemToArchiveSupported(LPCTSTR ArcFileName)const
{
	LPCTSTR lpExt=PathFindExtension(ArcFileName);
	if(!lpExt)return false;
	CString strExt=lpExt;
	strExt+=_T('.');	//".ext."の形にする

	const int ScriptCount=B2EScriptGetCount();  //スクリプトの数を取得

	for(int i=0;i<ScriptCount;i++){
		WORD ability;
		B2EScriptGetAbility(i,&ability);        //能力を取得
		if(ability & B2EABILITY_ADD){
			char Buffer[_MAX_PATH+1];
			B2EScriptGetName(i,Buffer,_MAX_PATH);   //スクリプト名を取得
			CString tmp=Buffer;
			tmp.Replace(_T("#"),_T(""));
			tmp=_T('.')+tmp;
			if(tmp.Find(strExt)!=-1){
				//対応拡張子(".ext.")あり
				return true;
			}
		}
	}
	return false;
}

bool CArchiverB2E::DeleteItemFromArchive(LPCTSTR ArcFileName,CConfigManager &ConfMan,const std::list<CString> &FileList,CString &strLog)
{
	if(!IsOK()){
		return false;
	}

	//==============================================
	// レスポンスファイル用テンポラリファイル名取得
	//==============================================
	TCHAR ResponceFileName[_MAX_PATH+1];
	FILL_ZERO(ResponceFileName);
	if(!UtilGetTemporaryFileName(ResponceFileName,_T("b2e"))){
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

	//パラメータ
	Param+=_T("-d ");			//削除

	//アーカイブファイル名指定
	Param+=_T("\"");
	Param+=ArcFileName;
	Param+=_T("\" ");

	//レスポンスファイル名指定
	Param+=_T("\"-@");
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

bool CArchiverB2E::QueryDeleteItemFromArchiveSupported(LPCTSTR ArcFileName)const
{
	LPCTSTR lpExt=PathFindExtension(ArcFileName);
	if(!lpExt)return false;
	CString strExt=lpExt;
	strExt+=_T('.');	//".ext."の形にする

	const int ScriptCount=B2EScriptGetCount();  //スクリプトの数を取得
	
	for(int i=0;i<ScriptCount;i++){
		WORD ability;
		B2EScriptGetAbility(i,&ability);        //能力を取得
		if(ability & B2EABILITY_DELETE){
			char Buffer[_MAX_PATH+1];
			B2EScriptGetName(i,Buffer,_MAX_PATH);   //スクリプト名を取得
			CString tmp=Buffer;
			tmp.Replace(_T("#"),_T(""));
			tmp=_T('.')+tmp;
			if(tmp.Find(strExt)!=-1){
				//対応拡張子(".ext.")あり
				return true;
			}
		}
	}
	return false;
}
