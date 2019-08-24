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
#include "ArchiverAISH.h"
#include "../Utilities/FileOperation.h"
#include "../resource.h"
#include "../ConfigCode/ConfigExtract.h"
#include "../ConfigCode/ConfigAISH.h"

CArchiverAISH::CArchiverAISH()
{
	m_nRequiredVersion=4;
	m_strDllName=_T("AISH32.DLL");
	m_AstrPrefix="Aish";
	m_LoadLevel=LOAD_DLL_MINIMUM;
}

CArchiverAISH::~CArchiverAISH()
{
	FreeDLL();
}

bool CArchiverAISH::Compress(LPCTSTR ArcFileName,std::list<CString> &ParamList,CConfigManager &ConfMan,const PARAMETER_TYPE Type,int,LPCTSTR,LPCTSTR lpszMethod,LPCTSTR lpszLevel,CString &strLog)
{
	ASSERT(0!=_tcslen(ArcFileName));
	TRACE(_T("ArcFileName=%s\n"),ArcFileName);


	//===========================
	// DLLに渡すオプションの設定
	//===========================
	TRACE(_T("DLLに渡すオプションの設定\n"));

	CString ParamOrg;//コマンドライン パラメータ バッファ

	CConfigAISH Config;
	Config.load(ConfMan);
	switch(Type){
	case PARAMETER_ISH:
		if(lpszMethod && *lpszMethod!=_T('\0')){
			ParamOrg+=_T("-");
			ParamOrg+=lpszMethod;
			ParamOrg+=_T(" ");
		}else{
			switch(Config.EncodeType){
			case ISH_SHIFT_JIS:
				ParamOrg+=_T("-s ");
				break;
			case ISH_JIS7:
				ParamOrg+=_T("-7 ");
				break;
			case ISH_JIS8:
				ParamOrg+=_T("-8 ");
				break;
			case ISH_NON_KANA_SHIFT_JIS:
				ParamOrg+=_T("-n ");
				break;
			}
		}
		break;
	case PARAMETER_UUE:
		ParamOrg+=_T("-u ");
		if(Config.UUEncodeChecksum){
			ParamOrg+=_T("-c ");
		}
		break;
	}
	ParamOrg+=_T("-o ");	//出力ファイル名指定

	//----------------------
	// 順番にファイルを処理
	//----------------------
	//一時出力先ファイル名リスト
	std::list<CString> TemporaryFileList;

	std::list<CString>::iterator ite;
	int Ret=0;
	strLog.Empty();
	for(ite=ParamList.begin();ite!=ParamList.end();ite++){
		//==========================
		// テンポラリファイル名取得
		//==========================

		TCHAR TempFileName[_MAX_PATH+1];
		FILL_ZERO(TempFileName);
		if(!UtilGetTemporaryFileName(TempFileName,_T("ish"))){
			strLog+=CString(MAKEINTRESOURCE(IDS_ERROR_TEMPORARY_FILE_CREATE));
			strLog+=_T("LhaForge : Failed to get temporary file.\r\n");
			Ret=-1;
			break;
		}
		ASSERT(0!=_tcslen(TempFileName));
		TemporaryFileList.push_back(TempFileName);

		CString Param=ParamOrg;
		Param+=_T("\"");
		Param+=TempFileName;
		Param+=_T("\" \".\\");
		Param+=*ite;
		Param+=_T("\"");

		ASSERT(!Param.IsEmpty());
		TRACE(_T("ArchiveHandler Commandline Parameter:%s\n"),Param);

		TRACE(_T("ArchiveHandler呼び出し\n"));
		//char szLog[LOG_BUFFER_SIZE]={0};
		std::vector<char> szLog(LOG_BUFFER_SIZE);
		szLog[0]='\0';
		int Ret=ArchiveHandler(NULL,CT2A(Param),&szLog[0],LOG_BUFFER_SIZE-1);
		strLog=&szLog[0];
		strLog+=_T("\r\n");
		if(Ret)break;
	}

	bool bDeleteOutput=false;	//出力先ファイルを削除する必要があるかどうか
	if(!Ret){
		strLog+=_T("LhaForge is Appending files...\r\n");

		HANDLE hArcFile=CreateFile(ArcFileName,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
		if(INVALID_HANDLE_VALUE==hArcFile){
			strLog+=_T("Error : Failed to open ");
			strLog+=ArcFileName;
			strLog+=_T(" for output.\r\n");
			Ret=-1;
		}
		else{
			std::list<CString>::iterator ite=TemporaryFileList.begin();
			const std::list<CString>::iterator end=TemporaryFileList.end();
			//テンポラリファイルを順次結合
			for(;ite!=end;ite++){
				HANDLE hFile=CreateFile(*ite,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
				int res=UtilAppendFile(hArcFile,hFile);
				if(0!=res){
					if(res<0){	//書き込みエラー
						strLog+=_T("\r\nError : Failed to write file ");
						strLog+=ArcFileName;
						strLog+=_T(".\r\n");
					}
					else{	//読み込みエラー
						strLog+=_T("\r\nError : Failed to read temporary file.\r\n");
					}
					Ret=-1;
					bDeleteOutput=true;
					break;
				}
				CloseHandle(hFile);
				strLog+=_T("o");
			}
			CloseHandle(hArcFile);
			if(!Ret){
				strLog+=_T(":done\r\n");
			}
			if(bDeleteOutput){
				DeleteFile(ArcFileName);
			}
		}
	}

	//使ったテンポラリファイルは消去
	{
		std::list<CString>::iterator ite=TemporaryFileList.begin();
		const std::list<CString>::iterator end=TemporaryFileList.end();
		for(;ite!=end;ite++){
			DeleteFile(*ite);
		}
	}

	return 0==Ret;
}

bool CArchiverAISH::Extract(LPCTSTR ArcFileName,CConfigManager&,const CConfigExtract &Config,bool,LPCTSTR OutputDir,CString &strLog)
{
	if(!IsOK()){
		return false;
	}

	//===========================
	// DLLに渡すオプションの設定
	//===========================
	TRACE(_T("DLLに渡すオプションの設定\n"));

	CString Param;//コマンドライン パラメータ バッファ

	//解凍パラメータはなし
	if(Config.ForceOverwrite){
		//デフォルトは強制上書き
	}else{
		Param+=_T("-f -h ");
/*	-f	すでに同名の展開ファイルが存在していた場合、別ファイル名にし
		て取り出します。ファイル名の最後に拡張子 .00 .99 をつけます。

	-h	-f オプション使用時、ファイル名の最後にではなく先頭に XX をつ
		けて区別します。元の拡張子を保存したい場合に使えます。
*/
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

bool CArchiverAISH::ExamineArchive(LPCTSTR ArcFileName,CConfigManager& ConfMan,bool,bool &bInFolder,bool &,CString &BaseDir,CString &strErr)
{
	bInFolder=false;
	return true;
}

//ish/uueファイルにファイルが一つ以上含まれるかどうか
bool CArchiverAISH::HasFiles(LPCTSTR ArcFileName)
{
	if(!IsOK()){
		ASSERT(!"Load DLL first!!!");
		return false;
	}

	CString Param;//コマンドライン パラメータ バッファ
	Param+=_T("-l ");

	//アーカイブファイル名指定
	Param+=_T("\"");
	Param+=ArcFileName;
	Param+=_T("\" ");

	ASSERT(!Param.IsEmpty());
	TRACE(_T("ArchiveHandler Commandline Parameter:%s\n"),Param);

	TRACE(_T("ArchiveHandler呼び出し\n"));
	CString strLog;
	try{
		//char szLog[LOG_BUFFER_SIZE]={0};
		std::vector<char> szLog(LOG_BUFFER_SIZE);
		szLog[0]='\0';
		int Ret=ArchiveHandler(NULL,CT2A(Param),&szLog[0],LOG_BUFFER_SIZE-1);
		strLog=&szLog[0];
		if(Ret)return false;
	}catch(...){
		return false;
	}

	//ログ解析
	strLog.Replace(_T("\r\n"),_T("\n"));
	int Index=strLog.Find(_T('\n'),0);
	int CopyFrom=0;
	while(true){
		if(-1==Index){
			if(strLog.IsEmpty())return false;
			else return true;
		}
		//行の切り出し
		if(Index==CopyFrom||Index+1==CopyFrom){
			CopyFrom=Index+1;
			Index=strLog.Find(_T('\n'),Index+1);
			continue;
		}
		return true;
	}
}

BOOL CArchiverAISH::CheckArchive(LPCTSTR _szFileName)
{
	if(!ArchiverCheckArchive){
		ASSERT(ArchiverCheckArchive);
		return false;
	}
	if(!ArchiverCheckArchive(CT2A(_szFileName),CHECKARCHIVE_RAPID))return FALSE;

	//AISH()のCheckArchiveはファイルの存在を確認して終わるだけなので、より判定を正確にするために
	//アーカイブ中にファイルが含まれていることを確認する
	if(!HasFiles(_szFileName))return FALSE;
	return TRUE;
}

//以下のコードでishの検査は出来るが、信頼性が微妙なため採用しない
//DEL bool CArchiverAISH::TestArchive(LPCSTR ArcFileName)
//DEL {
//DEL 	TRACE(_T("TestArchive() called.\n"));
//DEL 	ASSERT(IsOK());
//DEL 	if(!IsOK()){
//DEL 		return false;
//DEL 	}
//DEL 
//DEL 	//実際に展開してテストする
//DEL 	CString Param=_T(
//DEL 		"-o NUL "		//実際に展開するが、ファイルは保存しない
//DEL 
//DEL 		"\""
//DEL 	);
//DEL 	Param+=ArcFileName;
//DEL 	Param+=_T("\"");
//DEL 
//DEL 	//ファイル名をログに追加
//DEL 	CString Log=ArcFileName;
//DEL 	Log+=_T("\r\n\r\n");
//DEL 	//解説追加
//DEL 	Log+=_T(
//DEL 		"Status Examples:\r\n"
//DEL 		"o : OK\r\n"
//DEL 		"e : NG\r\n\r\n"
//DEL 
//DEL 		"----------\r\n"
//DEL 		);
//DEL 
//DEL 	CString Temp;
//DEL 	ArchiveHandler(NULL,Param,Temp.GetBuffer(LOG_BUFFER_SIZE),LOG_BUFFER_SIZE-1);
//DEL 	Temp.ReleaseBuffer();
//DEL 
//DEL 	Log+=Temp;
//DEL 
//DEL 	//ログ表示
//DEL 	CLogDialog LogDlg;
//DEL 	LogDlg.SetData(Log);
//DEL 	LogDlg.DoModal();
//DEL 	return true;
//DEL }
