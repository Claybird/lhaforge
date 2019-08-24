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
#include "ArchiverUNLHA.h"
#include "../Utilities/FileOperation.h"
#include "../resource.h"
#include "../ConfigCode/ConfigExtract.h"
#include "../ConfigCode/ConfigUNLHA.h"
#include "../Utilities/OSUtil.h"

CArchiverUNLHA::CArchiverUNLHA():
	ArchiveHandlerW(NULL),
	ArchiverCheckArchiveW(NULL),
	ArchiverGetFileNameW(NULL),
	ArchiverOpenArchiveW(NULL),
	ArchiverFindFirstW(NULL),
	ArchiverFindNextW(NULL),
	ArchiverGetFileCountW(NULL),
	ArchiverGetMethodW(NULL)
{
	m_dwInspectMode=0x00000002L;	//M_REGARDLESS_INIT_FILE:レジストリの値無視
	m_nRequiredVersion=262;
	m_nRequiredSubVersion=227;
	m_strDllName=_T("UNLHA32.DLL");
	m_AstrPrefix="Unlha";
	m_AstrFindParam="*.*";
}

CArchiverUNLHA::~CArchiverUNLHA()
{
	FreeDLL();
}

void CArchiverUNLHA::FormatCompressCommand(const CConfigLZH& Config,LPCTSTR lpszMethod,CString &Param)
{
	int nType=Config.CompressType;

	if(lpszMethod && *lpszMethod!=_T('\0')){
			 if(0==_tcsicmp(lpszMethod,_T("lh0")))nType=LZH_COMPRESS_LH0;
		else if(0==_tcsicmp(lpszMethod,_T("lh1")))nType=LZH_COMPRESS_LH1;
		else if(0==_tcsicmp(lpszMethod,_T("lh5")))nType=LZH_COMPRESS_LH5;
		else if(0==_tcsicmp(lpszMethod,_T("lh6")))nType=LZH_COMPRESS_LH6;
		else if(0==_tcsicmp(lpszMethod,_T("lh7")))nType=LZH_COMPRESS_LH7;
	}

	switch(nType){
	case LZH_COMPRESS_LH5:
		Param+=_T("-jm2 ");
		break;
	case LZH_COMPRESS_LH0:
		Param+=_T("-jm0 ");
		break;
	case LZH_COMPRESS_LH1:
		Param+=_T("-jm1 ");
		break;
	case LZH_COMPRESS_LH6:
		Param+=_T("-jm3 ");
		break;
	case LZH_COMPRESS_LH7:
		Param+=_T("-jm4 ");
		break;
	}
}

bool CArchiverUNLHA::Compress(LPCTSTR ArcFileName,std::list<CString> &ParamList,CConfigManager &ConfMan,const PARAMETER_TYPE,int Options,LPCTSTR,LPCTSTR lpszMethod,LPCTSTR,CString &strLog)
{
	if(!IsOK()){
		return false;
	}
	bool bSFX=(0!=(Options&COMPRESS_SFX));	//自己解凍作成ならtrue

	ASSERT(0!=_tcslen(ArcFileName));
	TRACE(_T("ArcFileName=%s\n"),ArcFileName);

	//==============================================
	// レスポンスファイル用テンポラリファイル名取得
	//==============================================
	TCHAR ResponceFileName[_MAX_PATH+1];
	FILL_ZERO(ResponceFileName);
	if(!UtilGetTemporaryFileName(ResponceFileName,_T("lzh"))){
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
		if(INVALID_HANDLE_VALUE==hFile)
		{
			strLog=CString(MAKEINTRESOURCE(IDS_ERROR_TEMPORARY_FILE_ACCESS));
			return false;
		}

		TRACE(_T("レスポンスファイルへの書き込み\n"));
		//圧縮先ファイル名書き込み
		WriteResponceFile(hFile,ArcFileName);

		std::list<CString>::iterator ite;
		for(ite=ParamList.begin();ite!=ParamList.end();++ite){
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
		_T("-d1 ")		//フォルダごと・全属性ファイルを対象
		_T("-+1 ")		//UNLHA32.DLLのレジストリの設定を無視
		_T("-jf0 ")		//ルート記号を削除
		_T("-jso1 ")	//SH_DENYNO でのオープンを行わない
		_T("-jsm1 ")	//サウンドを使う
	;
	//作業ディレクトリ
	Param+=_T("\"-w");
	Param+=UtilGetTempPath();
	Param+=_T("\" ");

	CConfigLZH Config;
	Config.load(ConfMan);
	FormatCompressCommand(Config,lpszMethod,Param);
	if(bSFX){
		//自己解凍作成
		if(Config.ConfigSFX){
			Param+=_T("-gw3 ");	//設定付きWinSFX32M
		}
		else{
			Param+=_T("-gw4 ");	//設定なしWinSFX32M
		}
	}

	//レスポンスファイル名指定
	Param+=_T("@\"");
	Param+=ResponceFileName;
	Param+=_T("\"");

	ASSERT(!Param.IsEmpty());
	TRACE(_T("ArchiveHandler Commandline Parameter:%s\n"),Param);

	TRACE(_T("ArchiveHandler呼び出し\n"));
	//char szLog[LOG_BUFFER_SIZE]={0};
	std::vector<WCHAR> szLog(LOG_BUFFER_SIZE);
	szLog[0]=L'\0';
	int Ret=ArchiveHandlerW(NULL,Param,&szLog[0],LOG_BUFFER_SIZE-1);
	strLog=&szLog[0];

	//使ったレスポンスファイルは消去
	DeleteFile(ResponceFileName);

	return 0==Ret;
}

//bSafeArchiveは無視される
bool CArchiverUNLHA::Extract(LPCTSTR ArcFileName,CConfigManager&,const CConfigExtract &Config,bool,LPCTSTR OutputDir,CString &strLog)
{
	if(!IsOK()){
		return false;
	}

	//===========================
	// DLLに渡すオプションの設定
	//===========================
	TRACE(_T("DLLに渡すオプションの設定\n"));

	//出力先移動
	CCurrentDirManager currentDir(OutputDir);
	CString Param;//コマンドライン パラメータ バッファ

	//解凍パラメータ
	Param+=
		_T("e ")			//解凍
		_T("-x1 ")			//パスを有効に
		_T("-jyc ")			//フォルダ作成の確認キャンセル
		_T("-a1 ")			//ファイルの属性を展開
		_T("-c1 ")			//タイムスタンプ検査を行う
		_T("-+1 ")			//UNLHA32.DLLのレジストリの設定を無視
		_T("-jf0 ")			//ルート記号を削除
		_T("-jso1 ")		//SH_DENYNO でのオープンを行わない
		_T("-jsm1 ")		//サウンドを使う
		_T("-jsp23 ")		//不正なパス/制御文字を拒否
		_T("-jsc ")			//展開できなかったファイルの数を返す
	;
	//作業ディレクトリ
	Param+=_T("\"-w");
	Param+=UtilGetTempPath();
	Param+=_T("\" ");
	if(Config.ForceOverwrite){
		//強制上書き
		Param+=_T("-jyo ");
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
	//char szLog[LOG_BUFFER_SIZE]={0};
	std::vector<WCHAR> szLog(LOG_BUFFER_SIZE);
	szLog[0]=L'\0';
	int Ret=ArchiveHandlerW(NULL,Param,&szLog[0],LOG_BUFFER_SIZE-1);
	strLog=&szLog[0];

	return 0==Ret;
}

//レスポンスファイルにファイル名をエスケープを実行した上で書き込む。
//有効なファイルハンドルとNULLでないファイル名を渡すこと。
void CArchiverUNLHA::WriteResponceFile(HANDLE hFile,LPCTSTR fname)
{
	CPath strBuffer;

	//ファイル名の先頭が'-'なら-gb(ファイル名)とする
#if defined(_UNICODE)||defined(UNICODE)
	if(_T('-')==fname[0]){
#else
#error("not implemented or tested")
	if(_MBC_SINGLE==_mbsbtype((const unsigned char*)fname,0)&&_T('-')==fname[0]){
#endif//defined(_UNICODE)||defined(UNICODE)
		strBuffer=_T("-gb");
		strBuffer+=fname;
	}
	else{
		strBuffer=fname;
	}
	strBuffer.QuoteSpaces();

	DWORD dwWritten=0;
	//ファイル名を出力
	WriteFile(hFile,(LPCBYTE)(LPCTSTR)strBuffer,_tcslen(strBuffer)*sizeof(TCHAR),&dwWritten,NULL);

	//ファイル名を区切るための改行
	TCHAR CRLF[]=_T("\r\n");
	WriteFile(hFile,&CRLF,(DWORD)_tcslen(CRLF)*sizeof(TCHAR),&dwWritten,NULL);
}


//=========================================================
// UnlhaGetFileName()の出力結果を基に、格納されたファイルがパス
// 情報を持っているかどうか判別し、二重フォルダ作成を防ぐ
//=========================================================
bool CArchiverUNLHA::ExamineArchive(LPCTSTR ArcFileName,CConfigManager& ConfMan,bool bSkipDir,bool &bInFolder,bool &,CString &BaseDir,CString &strErr)
{
	//"-jsp" スイッチを拡張し， Unicode 制御文字を不正として扱えるようにした。
	if(bSkipDir){
		TRACE(_T("二重フォルダ判定回避\n"));
		return true;
	}
	return _ExamineArchiveFast(ArcFileName,ConfMan,bInFolder,BaseDir,strErr);
}

bool CArchiverUNLHA::ExtractSpecifiedOnly(LPCTSTR ArcFileName,CConfigManager&,LPCTSTR OutputDir,std::list<CString> &FileList,CString &strLog,bool bUsePath)
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
	if(!UtilGetTemporaryFileName(ResponceFileName,_T("lzh"))){
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
		for(;ite!=end;++ite){
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
		_T("e ")			//解凍
		_T("-p1 ")			//全パス名で合致
		_T("-jyc ")			//フォルダ作成の確認キャンセル
		_T("-a2 ")			//ファイルの属性を展開
		_T("-c1 ")			//タイムスタンプ検査を行う
		_T("-+1 ")			//UNLHA32.DLLのレジストリの設定を無視
		_T("-jf0 ")			//ルート記号を削除
		_T("-jso1 ")		//SH_DENYNO でのオープンを行わない
		_T("-jsm1 ")		//サウンドを使う
		_T("-jsp23 ")		//不正なパス/制御文字を拒否
		_T("-jsc ")			//展開できなかったファイルの数を返す
	;
	//作業ディレクトリ
	Param+=_T("\"-w");
	Param+=UtilGetTempPath();
	Param+=_T("\" ");
	if(bUsePath){
		Param+=_T("-x1 ");			//パスを有効に
	}
	else{
		Param+=_T("-x0 ");			//パスを無効に
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
	Param+=_T("@\"");
	Param+=ResponceFileName;
	Param+=_T("\"");

	TRACE(_T("ArchiveHandler呼び出し\nCommandline Parameter:%s\n"),Param);
	std::vector<WCHAR> szLog(LOG_BUFFER_SIZE);
	szLog[0]=L'\0';
	int Ret=ArchiveHandlerW(NULL,Param,&szLog[0],LOG_BUFFER_SIZE-1);
	strLog=&szLog[0];
	//使ったレスポンスファイルは消去
	DeleteFile(ResponceFileName);

	return 0==Ret;
}

bool CArchiverUNLHA::DeleteItemFromArchive(LPCTSTR ArcFileName,CConfigManager&,const std::list<CString>& FileList,CString &strLog)
{
	if(!IsOK()){
		return false;
	}

	//==============================================
	// レスポンスファイル用テンポラリファイル名取得
	//==============================================
	TCHAR ResponceFileName[_MAX_PATH+1];
	FILL_ZERO(ResponceFileName);
	if(!UtilGetTemporaryFileName(ResponceFileName,_T("lzh"))){
		strLog=CString(MAKEINTRESOURCE(IDS_ERROR_TEMPORARY_FILE_CREATE));
		return false;
	}
	ASSERT(0!=_tcslen(ResponceFileName));

	//削除対象ファイルをレスポンスファイルに書き出す
	{
		HANDLE hFile=CreateFile(ResponceFileName,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
		if(INVALID_HANDLE_VALUE==hFile)
		{
			strLog=CString(MAKEINTRESOURCE(IDS_ERROR_TEMPORARY_FILE_ACCESS));
			return false;
		}

		std::list<CString>::const_iterator ite=FileList.begin();
		const std::list<CString>::const_iterator end=FileList.end();
		for(;ite!=end;++ite){
			CString tmp=*ite;
			//ディレクトリ名末尾の/を取り除く
#if defined(_UNICODE)||defined(UNICODE)
			if(_T('/')==tmp[tmp.GetLength()-1]){
				tmp.Delete(tmp.GetLength()-1);
			}
#else
			if(_MBC_SINGLE==_mbsbtype((const unsigned char*)(LPCTSTR)*ite,(*ite).GetLength()-1)){
				if(_T('/')==tmp[tmp.GetLength()-1]){
					tmp.Delete(tmp.GetLength()-1);
				}
			}
#endif//defined(_UNICODE)||defined(UNICODE)
			WriteResponceFile(hFile,tmp);
		}
		CloseHandle(hFile);
	}

	//===========================
	// DLLに渡すオプションの設定
	//===========================
	CString Param;

	//削除パラメータ
	Param+=
		_T("d ")			//削除
		_T("-r0 ")			//非再帰モード(デフォルト)
		_T("-p1 ")			//全パス名で合致
		_T("-+1 ")			//UNLHA32.DLLのレジストリの設定を無視
		_T("-jso1 ")		//SH_DENYNO でのオープンを行わない
		_T("-jsm1 ")		//サウンドを使う
		_T("-d ")			//ディレクトリの格納
	;
	//作業ディレクトリ
	Param+=_T("\"-w");
	Param+=UtilGetTempPath();
	Param+=_T("\" ");
	//アーカイブファイル名指定
	Param+=_T("\"");
	Param+=ArcFileName;
	Param+=_T("\" ");

	//レスポンスファイル名指定
	Param+=_T("@\"");
	Param+=ResponceFileName;
	Param+=_T("\"");

	TRACE(_T("ArchiveHandler呼び出し\nCommandline Parameter:%s\n"),Param);
	//char szLog[LOG_BUFFER_SIZE]={0};
	std::vector<WCHAR> szLog(LOG_BUFFER_SIZE);
	szLog[0]=L'\0';
	int Ret=ArchiveHandlerW(NULL,Param,&szLog[0],LOG_BUFFER_SIZE-1);
	strLog=&szLog[0];
	//使ったレスポンスファイルは消去
	DeleteFile(ResponceFileName);

	return 0==Ret;
}

ARCRESULT CArchiverUNLHA::TestArchive(LPCTSTR ArcFileName,CString &strLog)
{
	TRACE(_T("TestArchive() called.\n"));
	ASSERT(IsOK());
	if(!IsOK()){
		return TEST_ERROR;
	}

	//tコマンドによるテストが実装されている
	CString Param=
		_T("t ")			//テスト
		_T("-+1 ")			//UNLHA32.DLLのレジストリの設定を無視
		_T("-jso1 ")		//SH_DENYNO でのオープンを行わない
		_T("-jsc ")			//展開できなかったファイルの数を返す
//		_T("-jsp23 ")		//不正なパス/制御文字を拒否->エラーとならない

		_T("\"")
	;
	Param+=ArcFileName;
	Param+=_T("\" ");
	//作業ディレクトリ
	Param+=_T("\"-w");
	Param+=UtilGetTempPath();
	Param+=_T("\" ");

	//char szLog[LOG_BUFFER_SIZE]={0};
	std::vector<WCHAR> szLog(LOG_BUFFER_SIZE);
	szLog[0]=L'\0';
	int Ret=ArchiveHandlerW(NULL,Param,&szLog[0],LOG_BUFFER_SIZE-1);
	strLog=&szLog[0];

	if(Ret==0)return TEST_OK;
	else return TEST_NG;
}

BOOL CArchiverUNLHA::CheckArchive(LPCTSTR _szFileName)
{
	if(!ArchiverCheckArchiveW){
		ASSERT(ArchiverCheckArchiveW);
		return false;
	}
	return ArchiverCheckArchiveW(_szFileName,CHECKARCHIVE_BASIC);
}

int CArchiverUNLHA::GetFileCount(LPCTSTR _szFileName)
{
	if(!ArchiverGetFileCountW){
		return -1;
	}
	return ArchiverGetFileCountW(_szFileName);
}

void CArchiverUNLHA::FreeDLL()
{
	if(m_hInstDLL){
		ArchiveHandlerW				=NULL;
		ArchiverCheckArchiveW		=NULL;
		ArchiverGetFileCountW		=NULL;
		ArchiverGetFileNameW		=NULL;
		ArchiverOpenArchiveW		=NULL;
		ArchiverFindFirstW			=NULL;
		ArchiverFindNextW			=NULL;
		ArchiverGetMethodW			=NULL;
		CArchiverDLL::FreeDLL();
	}
}

LOAD_RESULT CArchiverUNLHA::LoadDLL(CConfigManager &ConfigManager,CString &strErr)
{
	FreeDLL();

	//基底クラスのメソッドを呼ぶ
	LOAD_RESULT res=CArchiverDLL::LoadDLL(ConfigManager,strErr);
	if(LOAD_RESULT_OK!=res){
		return res;
	}
	//ANSI版は間違って使うことのないようにリセットする
	ArchiveHandler				=NULL;
	ArchiverCheckArchive		=NULL;
	ArchiverGetFileCount		=NULL;
	ArchiverGetFileName			=NULL;
	ArchiverOpenArchive			=NULL;
	ArchiverFindFirst			=NULL;
	ArchiverFindNext			=NULL;
	ArchiverGetMethod			=NULL;

	ASSERT(ArchiverGetWriteTimeEx);
	if(!ArchiverGetWriteTimeEx){	//UNLHA32.DLLでは実装されているはず
		return LOAD_RESULT_INVALID;
	}

	//---UNICODE版をロードし直す;能力チェックは既に済んでいるので行わない

	ArchiveHandlerW=(COMMON_ARCHIVER_HANDLERW)GetProcAddress(m_hInstDLL,m_AstrPrefix+"W");
	if(NULL==ArchiveHandlerW){
		strErr.Format(IDS_ERROR_DLL_FUNCTION_GET,m_strDllName,m_AstrPrefix+"W");
		FreeDLL();
		return LOAD_RESULT_INVALID;
	}

	CStringA FunctionName=m_AstrPrefix;
	FunctionName+="CheckArchiveW";
	ArchiverCheckArchiveW=(COMMON_ARCHIVER_CHECKARCHIVEW)GetProcAddress(m_hInstDLL,FunctionName);
	if(NULL==ArchiverCheckArchiveW){
		strErr.Format(IDS_ERROR_DLL_FUNCTION_GET,m_strDllName,FunctionName);
		FreeDLL();
		return LOAD_RESULT_INVALID;
	}

	//XacRett.dllとAish32.dllだけがサポートしていない
	FunctionName=m_AstrPrefix;
	FunctionName+="GetFileCountW";
	ArchiverGetFileCountW=(COMMON_ARCHIVER_GETFILECOUNTW)GetProcAddress(m_hInstDLL,FunctionName);
	if(NULL==ArchiverGetFileCountW){
	//ロードに失敗してもエラーとはしない
		TRACE(_T("Failed to Load Function %s\n"),FunctionName);
	}

	//------------
	// 追加関数群
	//------------
	FunctionName=m_AstrPrefix;
	FunctionName+="OpenArchiveW";
	ArchiverOpenArchiveW=(COMMON_ARCHIVER_OPENARCHIVEW)GetProcAddress(m_hInstDLL,FunctionName);
	if(NULL==ArchiverOpenArchiveW){
		strErr.Format(IDS_ERROR_DLL_FUNCTION_GET,m_strDllName,FunctionName);
		FreeDLL();
		return LOAD_RESULT_INVALID;
	}

	//FindFirst
	FunctionName=m_AstrPrefix;
	FunctionName+="FindFirstW";
	ArchiverFindFirstW=(COMMON_ARCHIVER_FINDFIRSTW)GetProcAddress(m_hInstDLL,FunctionName);
	if(NULL==ArchiverFindFirstW){
		strErr.Format(IDS_ERROR_DLL_FUNCTION_GET,m_strDllName,FunctionName);
		FreeDLL();
		return LOAD_RESULT_INVALID;
	}

	//FindNext
	FunctionName=m_AstrPrefix;
	FunctionName+="FindNextW";
	ArchiverFindNextW=(COMMON_ARCHIVER_FINDNEXTW)GetProcAddress(m_hInstDLL,FunctionName);
	if(NULL==ArchiverFindNextW){
		strErr.Format(IDS_ERROR_DLL_FUNCTION_GET,m_strDllName,FunctionName);
		FreeDLL();
		return LOAD_RESULT_INVALID;
	}

	//ファイル名取得
	FunctionName=m_AstrPrefix;
	FunctionName+="GetFileNameW";
	ArchiverGetFileNameW=(COMMON_ARCHIVER_GETFILENAMEW)GetProcAddress(m_hInstDLL,FunctionName);
	if(NULL==ArchiverGetFileNameW){
		strErr.Format(IDS_ERROR_DLL_FUNCTION_GET,m_strDllName,FunctionName);
		FreeDLL();
		return LOAD_RESULT_INVALID;
	}

	//ファイル圧縮メソッド取得
	FunctionName=m_AstrPrefix;
	FunctionName+="GetMethodW";
	ArchiverGetMethodW=(COMMON_ARCHIVER_GETMETHODW)GetProcAddress(m_hInstDLL,FunctionName);
	if(NULL==ArchiverGetMethodW){
		strErr.Format(IDS_ERROR_DLL_FUNCTION_GET,m_strDllName,FunctionName);
		FreeDLL();
		return LOAD_RESULT_INVALID;
	}


	TRACE(_T("Standard Function Set Load\n"));
	return LOAD_RESULT_OK;
}

bool CArchiverUNLHA::InspectArchiveBegin(LPCTSTR ArcFileName,CConfigManager&)
{
	TRACE(_T("CArchiverUNLHA::InspectArchiveBegin()\n"));
	ASSERT(ArchiverOpenArchiveW);
	if(!ArchiverOpenArchiveW){
		return false;
	}
	if(m_hInspectArchive){
		ASSERT(!"Close the Archive First!!!\n");
		return false;
	}
	m_hInspectArchive=ArchiverOpenArchiveW(NULL,ArcFileName,m_dwInspectMode);
	if(!m_hInspectArchive){
		TRACE(_T("Failed to Open Archive\n"));
		return false;
	}
	m_bInspectFirstTime=true;

	FILL_ZERO(m_IndividualInfoW);
	return true;
}

bool CArchiverUNLHA::InspectArchiveGetFileName(CString &FileName)
{
	if(!m_hInspectArchive){
		ASSERT(!"Open an Archive First!!!\n");
		return false;
	}
	if(ArchiverGetFileNameW){
		std::vector<WCHAR> szBuffer(FNAME_MAX32*2+1);
		ArchiverGetFileNameW(m_hInspectArchive,&szBuffer[0],FNAME_MAX32*2);
		FileName=&szBuffer[0];
		return true;
	}else{
		return false;
	}
}


bool CArchiverUNLHA::InspectArchiveNext()
{
	ASSERT(ArchiverFindFirstW);
	ASSERT(ArchiverFindNextW);
	if(!ArchiverFindFirstW||!ArchiverFindNextW){
		return false;
	}
	if(!m_hInspectArchive){
		ASSERT(!"Open an Archive First!!!\n");
		return false;
	}

	FILL_ZERO(m_IndividualInfoW);
	bool bRet;
	if(m_bInspectFirstTime){
		m_bInspectFirstTime=false;
		bRet=(0==ArchiverFindFirstW(m_hInspectArchive,CStringW(m_AstrFindParam),&m_IndividualInfoW));
	}
	else{
		bRet=(0==ArchiverFindNextW(m_hInspectArchive,&m_IndividualInfoW));
	}

	return bRet;
}

//書庫内ファイルCRC取得
DWORD CArchiverUNLHA::InspectArchiveGetCRC()
{
	if(!m_hInspectArchive){
		ASSERT(!"Open an Archive First!!!\n");
		return -1;
	}
	return m_IndividualInfoW.dwCRC;
}

//書庫内ファイル圧縮率取得
WORD CArchiverUNLHA::InspectArchiveGetRatio()
{
	if(!m_hInspectArchive){
		ASSERT(!"Open an Archive First!!!\n");
		return -1;
	}

	return m_IndividualInfoW.wRatio;
}

//書庫内ファイル格納モード取得
bool CArchiverUNLHA::InspectArchiveGetMethodString(CString &strMethod)
{
	if(!m_hInspectArchive){
		ASSERT(!"Open an Archive First!!!\n");
		return false;
	}

	ASSERT(ArchiverGetMethodW);


	//\0で終わっているかどうか保証できない
	WCHAR szBuffer[32]={0};

	if(ArchiverGetMethodW){
		if(0!=ArchiverGetMethodW(m_hInspectArchive,szBuffer,31)){
			//構造体から取得
			wcsncpy_s(szBuffer,m_IndividualInfoW.szMode,8);
		}
	}
	else return false;

	//情報格納
	strMethod=szBuffer;
	return true;
}

bool CArchiverUNLHA::AddItemToArchive(LPCTSTR ArcFileName,bool bEncrypted,const std::list<CString> &FileList,CConfigManager &ConfMan,LPCTSTR lpDestDir,CString &strLog)
{
	//---カレントディレクトリ設定のために\で終わる基点パスを取得
	CString strBasePath;
	UtilGetBaseDirectory(strBasePath,FileList);
	TRACE(_T("%s\n"),strBasePath);

	//カレントディレクトリ設定
	SetCurrentDirectory(strBasePath);

	//==============================================
	// レスポンスファイル用テンポラリファイル名取得
	//==============================================
	TCHAR ResponceFileName[_MAX_PATH+1];
	FILL_ZERO(ResponceFileName);
	if(!UtilGetTemporaryFileName(ResponceFileName,_T("lzh"))){
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
		if(INVALID_HANDLE_VALUE==hFile)
		{
			strLog=CString(MAKEINTRESOURCE(IDS_ERROR_TEMPORARY_FILE_ACCESS));
			return false;
		}

		TRACE(_T("レスポンスファイルへの書き込み\n"));
		//圧縮先ファイル名書き込み
		WriteResponceFile(hFile,ArcFileName);

		const int BasePathLength=strBasePath.GetLength();
		std::list<CString>::const_iterator ite;
		for(ite=FileList.begin();ite!=FileList.end();++ite){
			//ベースパスを元に相対パス取得 : 共通である基底パスの文字数分だけカットする
			LPCTSTR lpszPath=(LPCTSTR)(*ite)+BasePathLength;

			if(*lpszPath==_T('\0')){
				//もしBasePathと同じになって空になってしまったパスがあれば、カレントディレクトリ指定を代わりに入れておく
				WriteResponceFile(hFile,_T("."));
			}
			else{
				WriteResponceFile(hFile,lpszPath);
			}
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
		_T("-d1 ")		//フォルダごと・全属性ファイルを対象
		_T("-+1 ")		//UNLHA32.DLLのレジストリの設定を無視
		_T("-jf0 ")		//ルート記号を削除
		_T("-jso1 ")	//SH_DENYNO でのオープンを行わない
		_T("-jsm1 ")	//サウンドを使う
	;
	//作業ディレクトリ
	Param+=_T("\"-w");
	Param+=UtilGetTempPath();
	Param+=_T("\" ");

	CConfigLZH Config;
	Config.load(ConfMan);
	FormatCompressCommand(Config,NULL,Param);

	//出力先フォルダの末尾のバックスラッシュを削除
	CPath destPath(lpDestDir);
	destPath.RemoveBackslash();
	//追加圧縮先
	Param+=_T("-jb\"");
	Param+=(CString)destPath;
	Param+=_T("\" ");

	//レスポンスファイル名指定
	Param+=_T("@\"");
	Param+=ResponceFileName;
	Param+=_T("\"");

	ASSERT(!Param.IsEmpty());
	TRACE(_T("ArchiveHandler Commandline Parameter:%s\n"),Param);

	TRACE(_T("ArchiveHandler呼び出し\n"));
	//char szLog[LOG_BUFFER_SIZE]={0};
	std::vector<WCHAR> szLog(LOG_BUFFER_SIZE);
	szLog[0]=L'\0';
	int Ret=ArchiveHandlerW(NULL,Param,&szLog[0],LOG_BUFFER_SIZE-1);
	strLog=&szLog[0];

	//使ったレスポンスファイルは消去
	DeleteFile(ResponceFileName);

	return 0==Ret;
}
