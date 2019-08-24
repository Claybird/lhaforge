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
#include "ArchiverTAR.h"
#include "../Utilities/FileOperation.h"
#include "../resource.h"
#include "../ConfigCode/ConfigExtract.h"
#include "../ConfigCode/ConfigTAR.h"
#include "../Utilities/OSUtil.h"

CArchiverTAR::CArchiverTAR():ArchiverOpenArchive2(NULL)
{
	m_dwInspectMode=0x00000002L;//M_REGARDLESS_INIT_FILE
	m_nRequiredVersion=236;
	m_strDllName=_T("TAR32.DLL");
	m_AstrPrefix="Tar";
}

CArchiverTAR::~CArchiverTAR()
{
	FreeDLL();
}

LOAD_RESULT CArchiverTAR::LoadDLL(CConfigManager &ConfigManager,CString &strErr)
{
	FreeDLL();

	//基底クラスのメソッドを呼ぶ
	LOAD_RESULT res=CArchiverDLL::LoadDLL(ConfigManager,strErr);
	if(LOAD_RESULT_OK!=res){
		return res;
	}
	//OpenArchiveは無効化
	ArchiverOpenArchive=NULL;
	//OpenArchive2
	CStringA FunctionName=m_AstrPrefix;
	FunctionName+="OpenArchive2";
	if(!ArchiverQueryFunctionList(91)){//91:???OpenArchive2
		strErr.Format(IDS_ERROR_UNAVAILABLE_API,(LPCTSTR)CA2T(FunctionName));
		FreeDLL();
		TRACE(_T("%s\n"),(LPCTSTR)strErr);
		return LOAD_RESULT_INVALID;
	}
	ArchiverOpenArchive2=(COMMON_ARCHIVER_OPENARCHIVE2)GetProcAddress(m_hInstDLL,FunctionName);
	if(NULL==ArchiverOpenArchive2){
		strErr.Format(IDS_ERROR_DLL_FUNCTION_GET,m_strDllName,(LPCTSTR)CA2T(FunctionName));
		FreeDLL();
		TRACE(_T("%s\n"),(LPCTSTR)strErr);
		return LOAD_RESULT_INVALID;
	}

	TRACE(_T("Standard Function Set Load\n"));
	return LOAD_RESULT_OK;
}

void CArchiverTAR::FreeDLL()
{
	if(m_hInstDLL){
		ArchiverOpenArchive2=NULL;
		CArchiverDLL::FreeDLL();
	}
}

bool CArchiverTAR::InspectArchiveBegin(LPCTSTR ArcFileName,CConfigManager &ConfMan)
{
	TRACE(_T("CArchiverTAR::InspectArchiveBegin()\n"));
	ASSERT(ArchiverOpenArchive2);
	if(!ArchiverOpenArchive2){
		return false;
	}
	if(m_hInspectArchive){
		ASSERT(!"Close the Archive First!!!\n");
		return false;
	}

	CConfigTAR ConfTAR;
	ConfTAR.load(ConfMan);

	CStringA strOpt;
	if(ConfTAR.bConvertCharset){
		strOpt="--convert-charset=auto";
	}else{
		strOpt="--convert-charset=no";
	}

	m_hInspectArchive=ArchiverOpenArchive2(NULL,CT2A(ArcFileName),m_dwInspectMode,strOpt);
	if(!m_hInspectArchive){
		TRACE(_T("Failed to Open Archive\n"));
		return false;
	}
	m_bInspectFirstTime=true;

	FILL_ZERO(m_IndividualInfo);
	return true;
}

bool CArchiverTAR::Compress(LPCTSTR ArcFileName,std::list<CString> &ParamList,CConfigManager &ConfMan,const PARAMETER_TYPE Type,int,LPCTSTR,LPCTSTR,LPCTSTR,CString &strLog)
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
	if(!UtilGetTemporaryFileName(ResponceFileName,_T("tar"))){
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
		WriteResponceFile(hFile,ArcFileName);
		//以降はオプションではない
		WriteResponceFile(hFile,_T("--"));

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

	CConfigTAR Config;
	Config.load(ConfMan);
	switch(Config.SortBy){
	case TAR_SORT_BY_EXT:
		Param+=_T("--sort-by-ext ");
		break;
	case TAR_SORT_BY_PATH:
		Param+=_T("--sort-by-path ");
		break;
	}


	switch(Type){
	case PARAMETER_TAR:
		Param+=_T("-cvf ");
		break;
	case PARAMETER_GZ:
		Param.Format(_T("-cvfGz%d "),Config.GzipCompressLevel);
		break;
	case PARAMETER_BZ2:
		Param.Format(_T("-cvfGB%d "),Config.Bzip2CompressLevel);
		break;
	case PARAMETER_XZ:
		Param.Format(_T("-cvfGJ%d "),Config.XZCompressLevel);
		break;
	case PARAMETER_LZMA:
		Param.Format(_T(" --lzma=%d -cvfG "),Config.LZMACompressLevel);
		break;

	case PARAMETER_TAR_GZ:
		Param.Format(_T("-cvfz%d "),Config.GzipCompressLevel);
		break;
	case PARAMETER_TAR_BZ2:
		Param.Format(_T("-cvfB%d "),Config.Bzip2CompressLevel);
		break;
	case PARAMETER_TAR_XZ:
		Param.Format(_T("-cvfJ%d "),Config.XZCompressLevel);
		break;
	case PARAMETER_TAR_LZMA:
		Param.Format(_T(" --lzma=%d -cvf "),Config.LZMACompressLevel);
		break;
	}


	//レスポンスファイル名指定
	Param+=_T("@\"");
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

//bSafeArchiveは無視される
bool CArchiverTAR::Extract(LPCTSTR ArcFileName,CConfigManager &ConfMan,const CConfigExtract &Config,bool,LPCTSTR OutputDir,CString &strLog)
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

	
	//強制上書きかどうか
	if(!Config.ForceOverwrite){
		Param+=_T("--confirm-overwrite ");
	}

	CConfigTAR ConfTAR;
	ConfTAR.load(ConfMan);

	//文字コード変換
	if(ConfTAR.bConvertCharset){
		Param+="--convert-charset=auto ";
	}else{
		Param+="--convert-charset=no ";
	}

	//解凍パラメータ
	Param+=_T("-xvf ");			//解凍

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
// TarGetFileName()の出力結果を基に、格納されたファイルがパス
// 情報を持っているかどうか判別し、二重フォルダ作成を防ぐ
//=========================================================
bool CArchiverTAR::ExamineArchive(LPCTSTR ArcFileName,CConfigManager& ConfMan,bool bSkipDir,bool &bInFolder,bool &,CString &BaseDir,CString &strErr)
{
	if(bSkipDir){
		TRACE(_T("二重フォルダ判定回避\n"));
		return true;
	}
	return _ExamineArchiveFast(ArcFileName,ConfMan,bInFolder,BaseDir,strErr);
}


bool CArchiverTAR::ExtractSpecifiedOnly(LPCTSTR ArcFileName,CConfigManager &ConfMan,LPCTSTR OutputDir,std::list<CString> &FileList,CString &strLog,bool bUsePath)
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
	if(!UtilGetTemporaryFileName(ResponceFileName,_T("tar"))){
		strLog=CString(MAKEINTRESOURCE(IDS_ERROR_TEMPORARY_FILE_CREATE));
		return false;
	}
	ASSERT(0!=_tcslen(ResponceFileName));

	//解凍対象ファイルをレスポンスファイルに書き出す
	{
		HANDLE hFile=CreateFile(ResponceFileName,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
		if(INVALID_HANDLE_VALUE==hFile)
		{
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
	Param+=_T("-xv ");			//解凍
	if(!bUsePath)Param+=_T("--use-directory=0 ");	//パス情報無効化

	CConfigTAR ConfTAR;
	ConfTAR.load(ConfMan);

	//文字コード変換
	if(ConfTAR.bConvertCharset){
		Param+="--convert-charset=auto ";
	}else{
		Param+="--convert-charset=no ";
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
	Param+=_T("-- @\"");
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

