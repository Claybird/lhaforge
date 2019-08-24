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
#include "ArchiverJACK.h"
#include "../resource.h"
#include "../ConfigCode/ConfigExtract.h"
#include "../ConfigCode/ConfigJACK.h"
#include "../Dialogs/JackVolumeSizeDlg.h"
#include "../Utilities/OSUtil.h"

CArchiverJACK::CArchiverJACK()
{
	m_nRequiredVersion=20;
	m_strDllName=_T("Jack32.dll");
	m_AstrPrefix="Jack";
	m_LoadLevel=LOAD_DLL_MINIMUM;
}

CArchiverJACK::~CArchiverJACK()
{
	FreeDLL();
}

/*
formatの指定は、B2E32.dllでのみ有効
levelの指定は、B2E32.dll以外で有効
*/
bool CArchiverJACK::Compress(LPCTSTR ArcFileName,std::list<CString> &ParamList,CConfigManager &ConfMan,const PARAMETER_TYPE Type,int Options,LPCTSTR lpszFormat,LPCTSTR,LPCTSTR,CString &strLog)
{
	LPCTSTR lpszSplitSize = lpszFormat;

	if(!IsOK()){
		return false;
	}

	//ArcFileNameは出力先フォルダ名
	ASSERT(0!=_tcslen(ArcFileName));
	TRACE(_T("ArcFileName=%s\n"),ArcFileName);

	//===========================
	// DLLに渡すオプションの設定
	//===========================
	TRACE(_T("DLLに渡すオプションの設定\n"));

	CString Param;//コマンドライン パラメータ バッファ

	CConfigJACK Config;
	Config.load(ConfMan);
	Param+=_T("-r ");	//分割
	if(Options&COMPRESS_SFX){
		Param+=_T("-m1 ");	//SFX
	}
	else{
		Param+=_T("-m0 ");	//通常
	}
	if(lpszSplitSize && _tcslen(lpszSplitSize)>0){
		CString Buf;
		Buf.Format(_T("-v:%s "),lpszSplitSize);
		Param+=Buf;//分割サイズ指定
	}else if(Config.SpecifyVolumeSizeAtCompress){
		CJackVolumeSizeDialog vsd;
		if(IDOK!=vsd.DoModal()){
			return false;
		}
		CString Buf;
		Buf.Format(_T("-v:%d "),vsd.VolumeSize);
		Param+=Buf;//分割サイズ指定
	}else{
		CString Buf;
		Buf.Format(_T("-v:%d "),Config.VolumeSize);
		Param+=Buf;//分割サイズ指定
	}

	//分割対象ファイル名指定
	Param+=_T("\"");
	Param+=*ParamList.begin();
	Param+=_T("\" ");

	//出力フォルダ指定
	Param+=_T("\"");
	Param+=ArcFileName;
	Param+=_T("\"");

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

bool CArchiverJACK::Extract(LPCTSTR ArcFileName,CConfigManager&,const CConfigExtract &Config,bool bSafeArchive,LPCTSTR OutputDir,CString &strLog)
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

	if(!Config.ForceOverwrite){//上書き確認する
		CString strFileName;

		//ファイルを読んで格納ファイル名を取得
		if(!GetContainedFileName(ArcFileName,strFileName)){
			//不正なファイル:CheckArchive()で弾かれていると期待できる
			return false;
		}

		//--------------
		// 存在チェック
		//--------------
		strFileName.Insert(0,OutputDir);
		if(PathFileExists(strFileName)){
			CString msg;
			msg.Format(IDS_CONFIRM_OVERWRITE_MESSAGE_SIMPLE,strFileName);
			if(IDYES!=MessageBox(NULL,msg,CString(MAKEINTRESOURCE(IDS_MESSAGE_CAPTION)),MB_YESNO|MB_ICONQUESTION)){
				return false;
			}
		}
	}


	CString Param;//コマンドライン パラメータ バッファ

	//結合パラメータ
	Param+=_T("-c ");	//結合

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
	std::vector<char> szLog(LOG_BUFFER_SIZE);
	szLog[0]='\0';
	int Ret=ArchiveHandler(NULL,CT2A(Param),&szLog[0],LOG_BUFFER_SIZE-1);
	strLog=&szLog[0];

	return 0==Ret;
}


bool CArchiverJACK::ExtractSpecifiedOnly(LPCTSTR ArcFileName,CConfigManager&,LPCTSTR OutputDir,std::list<CString>&,CString &,bool bUsePath)
{
	return false;
}

//=========================================================
// DTVの可能性のあるファイルかどうか直接確認する
//=========================================================
bool CArchiverJACK::ExamineArchive(LPCTSTR ArcFileName,CConfigManager& ConfMan,bool,bool &bInFolder,bool &bSafeArchive,CString &BaseDir,CString &strErr)
{
	bInFolder=false;
	bSafeArchive=false;

	CString strFileName;

	//ファイルを読んで格納ファイル名を取得
	if(!GetContainedFileName(ArcFileName,strFileName))return false;
	if(-1==strFileName.FindOneOf(_T(":\\/"))){//パス指定の文字が含まれていないので安全
		bSafeArchive=true;
	}

/*
	このコードでは解凍するファイル、一つしかファイル内容を確認していない。
	これでも問題がない理由は、JACK32.dllは展開時に出力ファイル名の一貫性をチェックしている模様だからである。
	[流れ]
	・n-1番目までのファイルには細工がされていない
		・n番目のファイルも細工されていない
			→正常解凍、n++
		・n番目のファイルが細工されている
			・n-1番目までのファイルをLhaForgeに与えた
				→安全だとして解凍を始めたものの、JAKの同一性チェックにかかる
			・n番目のファイルをLhaForgeに与えた
				→LhaForgeのDTVチェックにかかる

	[注意点]
	現在のコードでは、上書き確認機能を使うとき、同じJAKファイルを２回読むことになる。
	効率を求めるなら、ここのコードを無効化して、Extract()内部で安全かどうかチェックするようにすればよい。
	いまのところは、効率よりも見通しの良さを優先している。
*/

	return true;
}

//ヘッダ検索
// Original:JakTool.cpp/XacRett #49/(C)k.inaba
int CArchiverJACK::FindHeader(const BYTE* hdr,DWORD size)
{
	static const char Magic[]="This_Is_Jack_File";

	if(size<sizeof_JakHeader)
		return -1;

	if(0==strcmp((char*)hdr,Magic)){
		if(sizeof_JakHeader+((JakHeader*)hdr)->FileNameLen<size)
			return 0;
	}
	else if(hdr[0]=='M' && hdr[1]=='Z')	//自己解凍
	{
		DWORD prev = 0xffffffff;
		for( DWORD i=0; i<size-sizeof_JakHeader; i++ )
		{
			if( hdr[i]!='T' )continue;
			if( hdr[i+1]!='h' )continue;
			if( hdr[(++i)+1]!='i' )continue;
			if( hdr[(++i)+1]!='s' )continue;
			if( hdr[(++i)+1]!='_' )continue;
			if( 0!=strcmp((char*)hdr+(++i)+1,Magic+5) )continue;

			// スタブ内の文字列に引っかかることがあるので、
			// 二個"This_Is_..."があるときは一個Skip
			if( prev==0xffffffff )
				prev = (i-4);
			else
				return (i-4);
		}
		if(prev!=0xffffffff)
			return prev;
	}
	return -1;
}

bool CArchiverJACK::GetContainedFileName(LPCTSTR ArcFileName,CString &strFileName)
{
	strFileName.Empty();

	//ファイル内容バッファ
	std::vector<BYTE> Buffer;
	//ファイルオープン
	HANDLE hFile=CreateFile(ArcFileName,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
	if(INVALID_HANDLE_VALUE==hFile)return false;

	DWORD dwSize=GetFileSize(hFile,NULL);
		//4GBより大きなファイルサイズは取得できないが、現実的にはそのような大きなファイルは扱わない(JACKは分割用)
	Buffer.resize(dwSize);

	DWORD dwRead=0;
	if(dwSize<0){
		CloseHandle(hFile);
		return false;
	}

	//読み取り
	ReadFile(hFile,&Buffer[0],dwSize,&dwRead,NULL);
	CloseHandle(hFile);

	//ヘッダ検索
	int iPos=FindHeader(&Buffer[0],dwRead);
	if(-1==iPos){
		//Not Found
		return false;
	}

	JakHeader* lpHeader=(JakHeader*)&Buffer[iPos];

	for(unsigned int i=0;i<lpHeader->FileNameLen;i++){
		strFileName+=Buffer[iPos+sizeof_JakHeader+i];
	}
	return true;
}
