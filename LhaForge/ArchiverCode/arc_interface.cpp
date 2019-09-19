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
#include "arc_interface.h"
#include "../Utilities/FileOperation.h"
#include "../Utilities/OSUtil.h"

CArchiverDLL::CArchiverDLL():
	m_hInspectArchive(NULL),
	m_bInspectFirstTime(false),
	m_dwInspectMode(0),
	m_nRequiredVersion(0),
	m_nRequiredSubVersion(0),
	m_LoadLevel(LOAD_DLL_STANDARD),
	m_AstrFindParam("*"),
	m_hInstDLL(NULL),
	ArchiveHandler(NULL),
	ArchiverGetVersion(NULL),
	ArchiverGetSubVersion(NULL),
	ArchiverCheckArchive(NULL),
	ArchiverQueryFunctionList(NULL),
	ArchiverGetFileName(NULL),
	ArchiverOpenArchive(NULL),
	ArchiverCloseArchive(NULL),
	ArchiverFindFirst(NULL),
	ArchiverFindNext(NULL),
	ArchiverGetAttribute(NULL),
	ArchiverGetOriginalSizeEx(NULL),
	ArchiverGetCompressedSizeEx(NULL),
	ArchiverGetWriteTime(NULL),
	ArchiverGetWriteTimeEx(NULL),
	ArchiverGetMethod(NULL)
{
}

WORD CArchiverDLL::GetVersion()const
{
	if(!ArchiverGetVersion){
		ASSERT(ArchiverGetVersion);
		return 0;
	}
	return ArchiverGetVersion();
}

WORD CArchiverDLL::GetSubVersion()const
{
	if(!ArchiverGetSubVersion){
//		ASSERT(ArchiverGetSubVersion);
		return 0;
	}
	return ArchiverGetSubVersion();
}

bool CArchiverDLL::GetVersionString(CString &String)const
{
	if(!m_hInstDLL||!ArchiverGetVersion){
		String.LoadString(IDS_MESSAGE_DLL_NOT_AVAILABLE);
		return false;
	}
	if(!ArchiverGetSubVersion){
		//used sprintf
		String.Format(_T("Ver.%.2f"),ArchiverGetVersion()/100.0f);
	}
	else{
		String.Format(_T("Ver.%.2f.%.2f"),ArchiverGetVersion()/100.0f,ArchiverGetSubVersion()/100.0f);
	}
	return true;
}

BOOL CArchiverDLL::CheckArchive(LPCTSTR _szFileName)
{
	if(!ArchiverCheckArchive){
		ASSERT(ArchiverCheckArchive);
		return false;
	}
	return ArchiverCheckArchive(CT2A(_szFileName),CHECKARCHIVE_BASIC);
}


void CArchiverDLL::FreeDLL()
{
	ASSERT(NULL==m_hInspectArchive);
	ArchiveHandler				=NULL;
	ArchiverGetVersion			=NULL;
	ArchiverGetSubVersion		=NULL;
	ArchiverCheckArchive		=NULL;
	ArchiverGetFileName			=NULL;
	ArchiverQueryFunctionList	=NULL;
	ArchiverOpenArchive			=NULL;
	ArchiverCloseArchive		=NULL;
	ArchiverFindFirst			=NULL;
	ArchiverFindNext			=NULL;
	ArchiverGetAttribute		=NULL;
	ArchiverGetOriginalSizeEx	=NULL;
	ArchiverGetCompressedSizeEx	=NULL;
	ArchiverGetWriteTime		=NULL;
	ArchiverGetWriteTimeEx		=NULL;
	ArchiverGetMethod			=NULL;

	if(m_hInstDLL){
		FreeLibrary(m_hInstDLL);
		m_hInstDLL=NULL;
	}
}

LOAD_RESULT CArchiverDLL::LoadDLL(CConfigManager&,CString &strErr)
{
	FreeDLL();

	//安全なパスに移動;DLL読み込み対策
	CCurrentDirManager cdm(UtilGetModuleDirectoryPath());

	if(NULL==(m_hInstDLL=LoadLibrary(m_strDllName))){
		strErr.Format(IDS_ERROR_DLL_LOAD,m_strDllName);
		TRACE(_T("%s\n"),(LPCTSTR)strErr);
		return LOAD_RESULT_NOT_FOUND;
	}

	ArchiveHandler=(COMMON_ARCHIVER_HANDLER)GetProcAddress(m_hInstDLL,m_AstrPrefix);
	if(NULL==ArchiveHandler){
		strErr.Format(IDS_ERROR_DLL_FUNCTION_GET,m_strDllName,(LPCTSTR)CA2T(m_AstrPrefix));
		FreeDLL();
		TRACE(_T("%s\n"),(LPCTSTR)strErr);
		return LOAD_RESULT_INVALID;
	}

	CStringA FunctionName=m_AstrPrefix;
	FunctionName+="GetVersion";
	ArchiverGetVersion=(COMMON_ARCHIVER_GETVERSION)GetProcAddress(m_hInstDLL,FunctionName);
	if(NULL==ArchiverGetVersion){
		strErr.Format(IDS_ERROR_DLL_FUNCTION_GET,m_strDllName,(LPCTSTR)CA2T(FunctionName));
		FreeDLL();
		TRACE(_T("%s\n"),(LPCTSTR)strErr);
		return LOAD_RESULT_INVALID;
	}

	//サブバージョンの取得できるDLLは限られている
	FunctionName=m_AstrPrefix;
	FunctionName+="GetSubVersion";
	ArchiverGetSubVersion=(COMMON_ARCHIVER_GETVERSION)GetProcAddress(m_hInstDLL,FunctionName);
	if(NULL==ArchiverGetSubVersion){
	//ロードに失敗してもエラーとはしない
		TRACE(_T("Failed to Load Function %s\n"),(LPCTSTR)CA2T(FunctionName));
	}
	//-------------------------
	// DLLバージョンのチェック
	//-------------------------
	if((ArchiverGetVersion()<m_nRequiredVersion)){
		//メジャーバージョンが低い
		strErr.Format(IDS_ERROR_DLL_TOO_OLD,m_strDllName);
		FreeDLL();
		TRACE(_T("%s\n"),(LPCTSTR)strErr);
		return LOAD_RESULT_TOO_OLD;
	}else if((ArchiverGetVersion()==m_nRequiredVersion)&&(NULL!=ArchiverGetSubVersion)&&(ArchiverGetSubVersion()<m_nRequiredSubVersion)){
		//マイナーバージョンが低い
		strErr.Format(IDS_ERROR_DLL_TOO_OLD,(LPCTSTR)m_strDllName);
		FreeDLL();
		TRACE(_T("%s\n"),(LPCTSTR)strErr);
		return LOAD_RESULT_TOO_OLD;
	}

	FunctionName=m_AstrPrefix;
	FunctionName+="CheckArchive";
	ArchiverCheckArchive=(COMMON_ARCHIVER_CHECKARCHIVE)GetProcAddress(m_hInstDLL,FunctionName);
	if(NULL==ArchiverCheckArchive){
		strErr.Format(IDS_ERROR_DLL_FUNCTION_GET,m_strDllName,(LPCTSTR)CA2T(FunctionName));
		FreeDLL();
		TRACE(_T("%s\n"),(LPCTSTR)strErr);
		return LOAD_RESULT_INVALID;
	}

	if(LOAD_DLL_MINIMUM==m_LoadLevel){
		TRACE(_T("Minimum Function Set Load\n"));
		return LOAD_RESULT_OK;
	}

	//------------
	// 追加関数群
	//------------
	FunctionName=m_AstrPrefix;
	FunctionName+="QueryFunctionList";
	ArchiverQueryFunctionList=(COMMON_ARCHIVER_QUERYFUNCTIONLIST)GetProcAddress(m_hInstDLL,FunctionName);
	if(NULL==ArchiverQueryFunctionList){
		strErr.Format(IDS_ERROR_DLL_FUNCTION_GET,m_strDllName,(LPCTSTR)CA2T(FunctionName));
		FreeDLL();
		TRACE(_T("%s\n"),(LPCTSTR)strErr);
		return LOAD_RESULT_INVALID;
	}

	FunctionName=m_AstrPrefix;
	FunctionName+="OpenArchive";
	if(!ArchiverQueryFunctionList(23)){//23:???OpenArchive
		strErr.Format(IDS_ERROR_UNAVAILABLE_API,(LPCTSTR)CA2T(FunctionName));
		FreeDLL();
		TRACE(_T("%s\n"),(LPCTSTR)strErr);
		return LOAD_RESULT_INVALID;
	}
	ArchiverOpenArchive=(COMMON_ARCHIVER_OPENARCHIVE)GetProcAddress(m_hInstDLL,FunctionName);
	if(NULL==ArchiverOpenArchive){
		strErr.Format(IDS_ERROR_DLL_FUNCTION_GET,m_strDllName,(LPCTSTR)CA2T(FunctionName));
		FreeDLL();
		TRACE(_T("%s\n"),(LPCTSTR)strErr);
		return LOAD_RESULT_INVALID;
	}

	FunctionName=m_AstrPrefix;
	FunctionName+="CloseArchive";
	if(!ArchiverQueryFunctionList(24)){//24:???CloseArchive
		strErr.Format(IDS_ERROR_UNAVAILABLE_API,(LPCTSTR)CA2T(FunctionName));
		FreeDLL();
		TRACE(_T("%s\n"),(LPCTSTR)strErr);
		return LOAD_RESULT_INVALID;
	}
	ArchiverCloseArchive=(COMMON_ARCHIVER_CLOSEARCHIVE)GetProcAddress(m_hInstDLL,FunctionName);
	if(NULL==ArchiverCloseArchive){
		strErr.Format(IDS_ERROR_DLL_FUNCTION_GET,m_strDllName,(LPCTSTR)CA2T(FunctionName));
		FreeDLL();
		TRACE(_T("%s\n"),(LPCTSTR)strErr);
		return LOAD_RESULT_INVALID;
	}

	//FindFirst
	FunctionName=m_AstrPrefix;
	FunctionName+="FindFirst";
	if(!ArchiverQueryFunctionList(25)){//25:???FindFirst
		strErr.Format(IDS_ERROR_UNAVAILABLE_API,(LPCTSTR)CA2T(FunctionName));
		FreeDLL();
		TRACE(_T("%s\n"),(LPCTSTR)strErr);
		return LOAD_RESULT_INVALID;
	}
	ArchiverFindFirst=(COMMON_ARCHIVER_FINDFIRST)GetProcAddress(m_hInstDLL,FunctionName);
	if(NULL==ArchiverFindFirst){
		strErr.Format(IDS_ERROR_DLL_FUNCTION_GET,m_strDllName,(LPCTSTR)CA2T(FunctionName));
		FreeDLL();
		TRACE(_T("%s\n"),(LPCTSTR)strErr);
		return LOAD_RESULT_INVALID;
	}

	//FindNext
	FunctionName=m_AstrPrefix;
	FunctionName+="FindNext";
	if(!ArchiverQueryFunctionList(26)){//26:???FindNext
		strErr.Format(IDS_ERROR_UNAVAILABLE_API,(LPCTSTR)CA2T(FunctionName));
		FreeDLL();
		TRACE(_T("%s\n"),(LPCTSTR)strErr);
		return LOAD_RESULT_INVALID;
	}
	ArchiverFindNext=(COMMON_ARCHIVER_FINDNEXT)GetProcAddress(m_hInstDLL,FunctionName);
	if(NULL==ArchiverFindNext){
		strErr.Format(IDS_ERROR_DLL_FUNCTION_GET,m_strDllName,(LPCTSTR)CA2T(FunctionName));
		FreeDLL();
		TRACE(_T("%s\n"),(LPCTSTR)strErr);
		return LOAD_RESULT_INVALID;
	}

	if(LOAD_DLL_SIMPLE_INSPECTION==m_LoadLevel){
		TRACE(_T("Simple Inspection Set Load\n"));
		return LOAD_RESULT_OK;
	}

	//ファイル名取得
	FunctionName=m_AstrPrefix;
	FunctionName+="GetFileName";
	if(!ArchiverQueryFunctionList(57)){//57:???GetFileName
		strErr.Format(IDS_ERROR_UNAVAILABLE_API,FunctionName);
		FreeDLL();
		TRACE(_T("%s\n"),(LPCTSTR)strErr);
		return LOAD_RESULT_INVALID;
	}
	ArchiverGetFileName=(COMMON_ARCHIVER_GETFILENAME)GetProcAddress(m_hInstDLL,FunctionName);
	if(NULL==ArchiverGetFileName){
		strErr.Format(IDS_ERROR_DLL_FUNCTION_GET,m_strDllName,(LPCTSTR)CA2T(FunctionName));
		FreeDLL();
		TRACE(_T("%s\n"),(LPCTSTR)strErr);
		return LOAD_RESULT_INVALID;
	}

	//ファイル属性取得
	FunctionName=m_AstrPrefix;
	FunctionName+="GetAttribute";
	//CAB32.DLLではCabGetAttribute()はCabQueryFunctionList()で利用不可と報告されるが
	//実際は利用可能であるため、対症療法的にここを無効にした。
/*	if(!ArchiverQueryFunctionList(64)){//64:???GetAttribute
		TRACE(_T("Function %s is not Available\n"),FunctionName);
	}
	else{*/
		ArchiverGetAttribute=(COMMON_ARCHIVER_GETATTRIBUTE)GetProcAddress(m_hInstDLL,FunctionName);
		if(NULL==ArchiverGetAttribute){
			TRACE(_T("Failed to Load Function %s\n"),(LPCTSTR)CA2T(FunctionName));
		}
//	}

	//ファイルサイズ取得(圧縮前)
	FunctionName=m_AstrPrefix;
	FunctionName+="GetOriginalSizeEx";
	if(!ArchiverQueryFunctionList(85)){//85:???GetOriginalSizeEx
		TRACE(_T("Function %s is not Available\n"),(LPCTSTR)CA2T(FunctionName));
	}
	else{
		ArchiverGetOriginalSizeEx=(COMMON_ARCHIVER_GETORIGINALSIZEEX)GetProcAddress(m_hInstDLL,FunctionName);
		if(NULL==ArchiverGetOriginalSizeEx){
			TRACE(_T("Failed to Load Function %s\n"),(LPCTSTR)CA2T(FunctionName));
		}
	}

	//ファイルサイズ取得(圧縮後)
	FunctionName=m_AstrPrefix;
	FunctionName+="GetCompressedSizeEx";
	if(!ArchiverQueryFunctionList(86)){//86:???GetCompressedSizeEx
		TRACE(_T("Function %s is not Available\n"),(LPCTSTR)CA2T(FunctionName));
	}
	else{
		ArchiverGetCompressedSizeEx=(COMMON_ARCHIVER_GETORIGINALSIZEEX)GetProcAddress(m_hInstDLL,FunctionName);
		if(NULL==ArchiverGetCompressedSizeEx){
			TRACE(_T("Failed to Load Function %s\n"),(LPCTSTR)CA2T(FunctionName));
		}
	}

	//ファイル日時取得(拡張)
	FunctionName=m_AstrPrefix;
	FunctionName+="GetWriteTimeEx";
	if(!ArchiverQueryFunctionList(70)){//70:???GetWriteTimeEx
		TRACE(_T("Function %s is not Available\n"),(LPCTSTR)CA2T(FunctionName));
	}
	else{
		ArchiverGetWriteTimeEx=(COMMON_ARCHIVER_GETWRITETIMEEX)GetProcAddress(m_hInstDLL,FunctionName);
		if(NULL==ArchiverGetWriteTimeEx){
			TRACE(_T("Failed to Load Function %s\n"),(LPCTSTR)CA2T(FunctionName));
		}
	}
	//ファイル日時取得(通常版;拡張版が使えないときのみ)
	if(NULL==ArchiverGetWriteTimeEx){
		FunctionName=m_AstrPrefix;
		FunctionName+="GetWriteTime";
		if(!ArchiverQueryFunctionList(67)){//67:???GetWriteTime
			TRACE(_T("Function %s is not Available\n"),(LPCTSTR)CA2T(FunctionName));
		}
		else{
			ArchiverGetWriteTime=(COMMON_ARCHIVER_GETWRITETIME)GetProcAddress(m_hInstDLL,FunctionName);
			if(NULL==ArchiverGetWriteTime){
				TRACE(_T("Failed to Load Function %s\n"),(LPCTSTR)CA2T(FunctionName));
			}
		}
	}

	//ファイル圧縮メソッド取得
	FunctionName=m_AstrPrefix;
	FunctionName+="GetMethod";
	if(!ArchiverQueryFunctionList(66)){//66:???GetMethod
		TRACE(_T("Function %s is not Available\n"),(LPCTSTR)CA2T(FunctionName));
	}
	else{
		ArchiverGetMethod=(COMMON_ARCHIVER_GETMETHOD)GetProcAddress(m_hInstDLL,FunctionName);
		if(NULL==ArchiverGetMethod){
			TRACE(_T("Failed to Load Function %s\n"),(LPCTSTR)CA2T(FunctionName));
		}
	}


	if(LOAD_DLL_STANDARD==m_LoadLevel){
		TRACE(_T("Standard Function Set Load\n"));
		return LOAD_RESULT_OK;
	}

	ASSERT(!"Unknown Load Set!!!");
	return LOAD_RESULT_INVALID;
}


ARCRESULT CArchiverDLL::TestArchive(LPCTSTR lpszFile,CString &strMsg)
{
	ASSERT(IsOK());
	if(!IsOK())return TEST_ERROR;
	TRACE(_T("TestArchive() called.\n"));
	//デフォルトではテストは実装されていない

	strMsg.Format(IDS_TESTARCHIVE_UNAVAILABLE,lpszFile,GetName());
	return TEST_NOTIMPL;
}


//レスポンスファイルにファイル名をエスケープを実行した上で書き込む。
//有効なファイルハンドルとNULLでないファイル名を渡すこと。
void CArchiverDLL::WriteResponceFile(HANDLE hFile,LPCTSTR fname,bool bQuoteSpaces)
{
	CPath path(fname);
	if(bQuoteSpaces){
		path.QuoteSpaces();
	}
	char szBuffer[_MAX_PATH+1]={0};

	strncpy_s(szBuffer,CT2A(path),_MAX_PATH);

	TRACE(_T("%s\n"),(LPCTSTR)CA2T(szBuffer));

	DWORD dwWritten=0;
	//ファイル名を出力
	WriteFile(hFile,szBuffer,strlen(szBuffer)*sizeof(char),&dwWritten,NULL);

	//ファイル名を区切るための改行
	char CRLF[]="\r\n";
	WriteFile(hFile,&CRLF,strlen(CRLF)*sizeof(char),&dwWritten,NULL);
}

bool CArchiverDLL::ExtractItems(LPCTSTR lpszArcFile,CConfigManager &ConfMan,const ARCHIVE_ENTRY_INFO_TREE *lpBase, const std::list<ARCHIVE_ENTRY_INFO_TREE *> &items, LPCTSTR lpszOutputBaseDir, bool bCollapseDir, CString &strLog)
{
	//アーカイブの存在は既に確認済みとする
	//文字コード関係は既に確認済みとする

	//---リストからファイルとディレクトリを分離
	ARCHIVE_ENTRY_INFO_TREE *lpFileParent=NULL;	//ファイルの親
	std::list<CString> files;			//ファイル専用
	std::list<ARCHIVE_ENTRY_INFO_TREE*> dirs;	//ディレクトリ専用

	std::list<ARCHIVE_ENTRY_INFO_TREE*>::const_iterator ite=items.begin();
	const std::list<ARCHIVE_ENTRY_INFO_TREE*>::const_iterator end=items.end();
	for(;ite!=end;++ite){
		ARCHIVE_ENTRY_INFO_TREE* lpNode=*ite;
		if(lpNode->bDir){
			dirs.push_back(lpNode);
		}else{
			lpFileParent=lpNode->lpParent;
			files.push_back(lpNode->strFullPath);
		}
	}

	// ファイルを処理
	if(!files.empty()){
		CString pathOutputDir;
		if(!bCollapseDir){
			ArcEntryInfoTree_GetNodePathRelative(lpFileParent,lpBase,pathOutputDir);
		}
		pathOutputDir=lpszOutputBaseDir+pathOutputDir;
		bool bRet=ExtractSpecifiedOnly(lpszArcFile,ConfMan,pathOutputDir,files,strLog);
		if(!bRet){
			return false;
		}
	}

	// ディレクトリを処理
	if(!dirs.empty()){
		if(!ExtractSubDirectories(lpszArcFile,ConfMan,lpBase,dirs,lpszOutputBaseDir,bCollapseDir,strLog))return false;
	}
	return true;
}

//指定されたディレクトリ内部を展開する;基本実装
bool CArchiverDLL::ExtractSubDirectories(LPCTSTR lpszArcFile,CConfigManager &ConfMan,const ARCHIVE_ENTRY_INFO_TREE* lpBase,const std::list<ARCHIVE_ENTRY_INFO_TREE*> &dirs,LPCTSTR lpszOutputDir,bool bCollapseDir,CString &strLog)
{
	std::list<ARCHIVE_ENTRY_INFO_TREE*>::const_iterator ite=dirs.begin();
	const std::list<ARCHIVE_ENTRY_INFO_TREE*>::const_iterator end=dirs.end();
	for(;ite!=end;++ite){
		if(!ExtractDirectoryEntry(lpszArcFile,ConfMan,bCollapseDir?(*ite)->lpParent:lpBase,*ite,lpszOutputDir,bCollapseDir,strLog))return false;
	}
	return true;
}

//指定されたディレクトリ内部を展開する;基本実装
bool CArchiverDLL::ExtractDirectoryEntry(LPCTSTR lpszArcFile,CConfigManager &ConfMan,const ARCHIVE_ENTRY_INFO_TREE* lpBase,const ARCHIVE_ENTRY_INFO_TREE* lpDir,LPCTSTR lpszOutputBaseDir,bool bCollapseDir,CString &strLog)
{
	//ディレクトリ中のファイルを列挙
	std::list<CString> files;			//ファイル専用
	std::list<ARCHIVE_ENTRY_INFO_TREE*> dirs;	//ディレクトリ専用

	//ディレクトリがアーカイブ中にエントリとして登録されていても無視する
	//->通常通り出力すると出力先と誤解されることがある
	UINT nItems=lpDir->GetNumChildren();
	for(UINT i=0;i<nItems;i++){
		ARCHIVE_ENTRY_INFO_TREE* lpNode=lpDir->GetChild(i);

		if(lpNode->bDir){
			dirs.push_back(lpNode);
		}else{
			files.push_back(lpNode->strFullPath);
		}
	}

	//--------------------------------------
	// 修正された出力ディレクトリパスを算出
	//--------------------------------------
	CString pathOutputDir;
	ArcEntryInfoTree_GetNodePathRelative(lpDir,lpBase,pathOutputDir);

	//出力ディレクトリ名と結合
	pathOutputDir=lpszOutputBaseDir+pathOutputDir;

	if(pathOutputDir.GetLength()>_MAX_PATH){
		//フォルダ名が長くなりすぎた
		strLog=CString(MAKEINTRESOURCE(IDS_ERROR_MAX_PATH));
		return false;
	}

	if(files.empty() && dirs.empty() && lpDir->strFullPath.IsEmpty()){	//空フォルダがアーカイブ中に存在した場合
		//空ディレクトリエントリを手作業で復元する
		if(!UtilMakeSureDirectoryPathExists(pathOutputDir)){
			strLog.Format(IDS_ERROR_CANNOT_MAKE_DIR,(LPCTSTR)pathOutputDir);
			return false;
		}
		return true;
	}

	//---再帰的に処理

	//----------------
	// ファイルを処理
	//----------------
	if(!files.empty()){
		if(!ExtractSpecifiedOnly(lpszArcFile,ConfMan,pathOutputDir,files,strLog)){
			return false;
		}
	}

	//--------------------
	// ディレクトリを処理
	//--------------------
	if(!dirs.empty()){
		ExtractSubDirectories(lpszArcFile,ConfMan,bCollapseDir?lpDir:lpBase,dirs,lpszOutputBaseDir,bCollapseDir,strLog);
	}
	return true;
}
