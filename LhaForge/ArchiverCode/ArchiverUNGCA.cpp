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

