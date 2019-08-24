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
	ArchiverGetFileCount(NULL),
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

int CArchiverDLL::GetFileCount(LPCTSTR _szFileName)
{
	if(!ArchiverGetFileCount){
		return -1;
	}
	return ArchiverGetFileCount(CT2A(_szFileName));
}

bool CArchiverDLL::_ExamineArchive(LPCTSTR ArcFileName,CConfigManager& ConfMan,bool &bInFolder,bool &bSafeArchive,const int MaxRepeat,CString &BaseDir,CString &strErr)
{
	ASSERT(IsOK());
	if(!IsOK()){
		return false;
	}

	if(!InspectArchiveBegin(ArcFileName,ConfMan)){
		strErr.Format(IDS_ERROR_OPEN_ARCHIVE,ArcFileName);
		return false;
	}

	bInFolder=true;
	bool bSureDir=false;	//BaseDirに入っている文字列が確かにフォルダであるならtrue
	TRACE(_T("========\n"));

	while(InspectArchiveNext()){
		CString Buffer;
		InspectArchiveGetFileName(Buffer);
		Buffer.Replace(_T('\\'),_T('/'));		//パス区切り文字の置き換え
		TRACE(_T("%s\n"),Buffer);

		/*
		Separator('/' or '\')は格納ファイルの先頭にいくら含まれていても無視すべきであるので、
		格納ファイル名の先頭にいくらSeparatorがあってもフォルダに格納された状態とは見なさない。
		SeparatorがMaxRepeatより多いと不正とする
		ただし、MaxRepeatが-1のときはエラーとはしない
		*/
		const int Length=Buffer.GetLength();
		int StartIndex=0;
		for(;StartIndex<Length;StartIndex++){
			//先頭の'/'をとばしていく
#if defined(_UNICODE)||defined(UNICODE)
			if(_T('/')!=Buffer[StartIndex])break;
#else
			if(_MBC_SINGLE==_mbsbtype((const unsigned char *)(LPCTSTR)Buffer,StartIndex)){
				if(_T('/')!=Buffer[StartIndex])break;
			}
			else{	//全角文字なら'/'であるはずがない
				break;
			}
#endif//defined(_UNICODE)||defined(UNICODE)
			if(-1!=MaxRepeat){
				if(StartIndex>=MaxRepeat){	//'/'がMaxRepeat個以上続く場合
					//危険なファイルと分かったので監査終了
					InspectArchiveEnd();
					bSafeArchive=false;
					bInFolder=false;
					return true;
				}
			}
		}
		if((-1!=Buffer.Find(_T("../"),StartIndex))||	//相対パス指定が見つかれば、それは安全なファイルではない
			(-1!=Buffer.Find(_T(':'),StartIndex))){	//ドライブ名が見つかれば、それは安全なファイルではない
			//危険なファイルと分かったので監査終了
			InspectArchiveEnd();
			bSafeArchive=false;
			bInFolder=false;
			return true;
		}

		//ここからは二重ディレクトリ判定
		//すでに二重ディレクトリ判定が付いている場合は安全判定のみに徹する

		int FoundIndex=0;
		while(bInFolder){
			FoundIndex=Buffer.Find(_T('/'),StartIndex);
			if(-1==FoundIndex){	//'/'が格納ファイル名の先頭以外に含まれない場合
				if(!BaseDir.IsEmpty()&&BaseDir==Buffer){
					bSureDir=true;	//BaseDirがフォルダであると確認された
					break;
				}
				else if(BaseDir.IsEmpty()){
					//フォルダ名の後ろに'/'が付かないアーカイバもある
					//そういうものが最初に出てきたときは、フォルダ名と仮定する
					BaseDir=Buffer;
					bSureDir=false;
					break;
				}
			}
			CString Dir=Buffer.Mid(StartIndex,FoundIndex-StartIndex);	//Separatorの前までの文字列(ディレクトリに相当)を抜き出してくる
			//これまでの調べでDirはEmptyではないことが保証されている
			//また、危険ではないことも分かっている
			TRACE(_T("Base=%s,Dir=%s\n"),BaseDir,Dir);

			if(_T('.')==Dir){	//./があればディレクトリ指定としては無視する
				StartIndex=FoundIndex+1;
				continue;
			}
			if(BaseDir.IsEmpty()){
				BaseDir=Dir;
				bSureDir=true;
			}
			else if(BaseDir!=Dir){
				bInFolder=false;
			}
			else bSureDir=true;	//BaseDirがディレクトリと確認された
			break;
		}
	}
	TRACE(_T("========\n"));

	InspectArchiveEnd();
	bSafeArchive=true;

	//フォルダに入っているようではあるが、ディレクトリと仮定されただけの場合
	if(bInFolder&&!bSureDir)bInFolder=false;
	return true;
}

bool CArchiverDLL::_ExamineArchiveFast(LPCTSTR ArcFileName,CConfigManager& ConfMan,bool &bInFolder,CString &BaseDir,CString &strErr)
{
	//不正なパスを持つファイルは全てDLL側が対処してくれていると考えるので、全てのアーカイブは安全であるとする

	TRACE(_T("---_ExamineArchiveFast() called.\n"));
	ASSERT(IsOK());
	if(!IsOK()){
		return false;
	}

	if(!InspectArchiveBegin(ArcFileName,ConfMan)){
		strErr.Format(IDS_ERROR_OPEN_ARCHIVE,ArcFileName);
		return false;
	}

	bool bSureDir=false;	//BaseDirに入っている文字列が確かにフォルダであるならtrue
	bInFolder=true;
	TRACE(_T("========\n"));
	while(InspectArchiveNext()){
		CString Buffer;
		InspectArchiveGetFileName(Buffer);
		Buffer.Replace(_T('\\'),_T('/'));		//パス区切り文字の置き換え
		TRACE(_T("%s\n"),Buffer);

		/*
		Separatorは格納ファイルの先頭にいくら含まれていても無視すべきであるので、
		格納ファイル名の先頭にいくら/があってもフォルダに格納された状態とは見なさない。
		*/
		const int Length=Buffer.GetLength();
		int StartIndex=0;
		for(;StartIndex<Length;StartIndex++){
			//先頭のSeparatorをとばしていく
#if defined(_UNICODE)||defined(UNICODE)
			if(_T('/')!=Buffer[StartIndex])break;
#else
			if(_MBC_SINGLE==_mbsbtype((const unsigned char *)(LPCTSTR)Buffer,StartIndex)){
				if(_T('/')!=Buffer[StartIndex])break;
			}
			else{	//全角文字ならSeparatorであるはずがない
				break;
			}
#endif//defined(_UNICODE)||defined(UNICODE)
		}

		int FoundIndex=0;
		while(bInFolder){
			FoundIndex=Buffer.Find(_T('/'),StartIndex);
			if(-1==FoundIndex){	//'/'が格納ファイル名の先頭以外に含まれない場合
				if(!BaseDir.IsEmpty()&&BaseDir==Buffer){
					bSureDir=true;	//BaseDirがフォルダであると確認された
					break;
				}
				else if(BaseDir.IsEmpty()){
					//フォルダ名の後ろに'/'が付かないアーカイバもある
					//そういうものが最初に出てきたときは、フォルダ名と仮定する
					BaseDir=Buffer;
					bSureDir=false;
					break;
				}
			}
			CString Dir=Buffer.Mid(StartIndex,FoundIndex-StartIndex);	//Separatorの前までの文字列(ディレクトリに相当)を抜き出してくる
			//これまでの調べでDirはEmptyではないことが保証されている
			//また、危険ではないことも分かっている
			TRACE(_T("Base=%s,Dir=%s\n"),BaseDir,Dir);

			if(_T('.')==Dir){	//./があればディレクトリ指定としては無視する
				StartIndex=FoundIndex+1;
				continue;
			}
			if(BaseDir.IsEmpty()){
				BaseDir=Dir;
				bSureDir=true;
			}
			else if(BaseDir!=Dir){
				bInFolder=false;
			}
			else bSureDir=true;	//BaseDirがディレクトリと確認された
			break;
		}
	}
	TRACE(_T("========\n"));

	InspectArchiveEnd();
	//フォルダに入っているようではあるが、ディレクトリと仮定されただけの場合
	if(bInFolder&&!bSureDir)bInFolder=false;
	return true;
}

void CArchiverDLL::FreeDLL()
{
	ASSERT(NULL==m_hInspectArchive);
	ArchiveHandler				=NULL;
	ArchiverGetVersion			=NULL;
	ArchiverGetSubVersion		=NULL;
	ArchiverCheckArchive		=NULL;
	ArchiverGetFileCount		=NULL;
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

	//XacRett.dllとAish32.dllだけがサポートしていない
	FunctionName=m_AstrPrefix;
	FunctionName+="GetFileCount";
	ArchiverGetFileCount=(COMMON_ARCHIVER_GETFILECOUNT)GetProcAddress(m_hInstDLL,FunctionName);
	if(NULL==ArchiverGetFileCount){
	//ロードに失敗してもエラーとはしない
		TRACE(_T("Failed to Load Function %s\n"),(LPCTSTR)CA2T(FunctionName));
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

bool CArchiverDLL::InspectArchiveBegin(LPCTSTR ArcFileName,CConfigManager&)
{
	TRACE(_T("CArchiverDLL::InspectArchiveBegin()\n"));
	ASSERT(ArchiverOpenArchive);
	if(!ArchiverOpenArchive){
		return false;
	}
	if(m_hInspectArchive){
		ASSERT(!"Close the Archive First!!!\n");
		return false;
	}
	m_hInspectArchive=ArchiverOpenArchive(NULL,CT2A(ArcFileName),m_dwInspectMode);
	if(!m_hInspectArchive){
		TRACE(_T("Failed to Open Archive\n"));
		return false;
	}
	m_bInspectFirstTime=true;

	FILL_ZERO(m_IndividualInfo);
	return true;
}

bool CArchiverDLL::InspectArchiveEnd()
{
	ASSERT(ArchiverCloseArchive);
	if(!ArchiverCloseArchive){
		return false;
	}
	if(!m_hInspectArchive){
		ASSERT(!"The Archive is Already Closed!!!\n");
		return false;
	}
	ArchiverCloseArchive(m_hInspectArchive);
	m_hInspectArchive=NULL;
	return true;
}

bool CArchiverDLL::InspectArchiveGetFileName(CString &FileName)
{
	if(!m_hInspectArchive){
		ASSERT(!"Open an Archive First!!!\n");
		return false;
	}
	if(ArchiverGetFileName){
		char szBuffer[FNAME_MAX32*2+1]={0};
		ArchiverGetFileName(m_hInspectArchive,szBuffer,FNAME_MAX32*2);
		FileName=szBuffer;
		return true;
	}
	else{
		ASSERT(LOAD_DLL_STANDARD!=m_LoadLevel);
		FileName=m_IndividualInfo.szFileName;
		return true;
	}
}

bool CArchiverDLL::InspectArchiveNext()
{
	ASSERT(ArchiverFindFirst);
	ASSERT(ArchiverFindNext);
	if(!ArchiverFindFirst||!ArchiverFindNext){
		return false;
	}
	if(!m_hInspectArchive){
		ASSERT(!"Open an Archive First!!!\n");
		return false;
	}

	FILL_ZERO(m_IndividualInfo);
	bool bRet;
	if(m_bInspectFirstTime){
		m_bInspectFirstTime=false;
		bRet=(0==ArchiverFindFirst(m_hInspectArchive,m_AstrFindParam,&m_IndividualInfo));
	}
	else{
		bRet=(0==ArchiverFindNext(m_hInspectArchive,&m_IndividualInfo));
	}

	return bRet;
}

int CArchiverDLL::InspectArchiveGetAttribute()
{
//	ASSERT(ArchiverGetAttribute);
	//対応していないDLL(CAB32.dllなど)ではNULLになっている
	if(!ArchiverGetAttribute){
		return -1;
	}
	if(!m_hInspectArchive){
		ASSERT(!"Open an Archive First!!!\n");
		return -1;
	}

	return ArchiverGetAttribute(m_hInspectArchive);
}

bool CArchiverDLL::InspectArchiveGetOriginalFileSize(LARGE_INTEGER &FileSize)
{
	if(!m_hInspectArchive){
		ASSERT(!"Open an Archive First!!!\n");
		return false;
	}
	if(ArchiverGetOriginalSizeEx){
		return (FALSE!=ArchiverGetOriginalSizeEx(m_hInspectArchive,&FileSize));
	}
	else{
		//APIによるファイルサイズ取得に失敗したので構造体のデータを利用する
		if(-1==m_IndividualInfo.dwOriginalSize)return false;
		FileSize.HighPart=0;
		FileSize.LowPart=m_IndividualInfo.dwOriginalSize;
		return true;
	}
}

bool CArchiverDLL::InspectArchiveGetCompressedFileSize(LARGE_INTEGER &FileSize)
{
	if(!m_hInspectArchive){
		ASSERT(!"Open an Archive First!!!\n");
		return false;
	}
	if(ArchiverGetCompressedSizeEx){
		return (FALSE!=ArchiverGetCompressedSizeEx(m_hInspectArchive,&FileSize));
	}
	else{
		//APIによるファイルサイズ取得に失敗したので構造体のデータを利用する
		if(-1==m_IndividualInfo.dwCompressedSize)return false;
		FileSize.HighPart=0;
		FileSize.LowPart=m_IndividualInfo.dwCompressedSize;
		return true;
	}
}

bool CArchiverDLL::InspectArchiveGetWriteTime(FILETIME &FileTime)
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

//書庫内ファイルCRC取得
DWORD CArchiverDLL::InspectArchiveGetCRC()
{
	if(!m_hInspectArchive){
		ASSERT(!"Open an Archive First!!!\n");
		return -1;
	}
	return m_IndividualInfo.dwCRC;
}

//書庫内ファイル圧縮率取得
WORD CArchiverDLL::InspectArchiveGetRatio()
{
	if(!m_hInspectArchive){
		ASSERT(!"Open an Archive First!!!\n");
		return -1;
	}

	return m_IndividualInfo.wRatio;
}

//書庫内ファイル格納モード取得
//ANSI版
bool CArchiverDLL::InspectArchiveGetMethodString(CString &strMethod)
{
	if(!m_hInspectArchive){
		ASSERT(!"Open an Archive First!!!\n");
		return false;
	}

	//\0で終わっているかどうか保証できない
	char szBuffer[32]={0};

	if(ArchiverGetMethod){
		if(0!=ArchiverGetMethod(m_hInspectArchive,szBuffer,31)){
			//構造体から取得
			strncpy_s(szBuffer,m_IndividualInfo.szMode,8);
		}
	}
	else{
		//構造体から取得
		strncpy_s(szBuffer,m_IndividualInfo.szMode,8);
	}

	//情報格納
	strMethod=szBuffer;
	return true;
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

	TRACE(_T("%s\n"),CA2T(szBuffer));

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
