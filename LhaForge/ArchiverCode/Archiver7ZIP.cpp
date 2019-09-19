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
#include "Archiver7ZIP.h"
#include "../Utilities/FileOperation.h"
#include "../Utilities/OSUtil.h"
#include "../Utilities/StringUtil.h"
#include "../Utilities/TemporaryDirMgr.h"
#include "../resource.h"
#include "../ConfigCode/ConfigExtract.h"
#include "../ConfigCode/Config7Z.h"
#include "../ConfigCode/ConfigZIP.h"
#include "../Dialogs/SevenZipVolumeSizeDlg.h"
#include "../Utilities/DummyWindow.h"

CArchiver7ZIP::CArchiver7ZIP():
	ArchiverSetUnicodeMode(NULL),
	ArchiverGetArchiveType(NULL)
{
	m_nRequiredVersion=920;
	m_nRequiredSubVersion=2;
	m_strDllName=_T("7-ZIP32.DLL");
	m_AstrPrefix="SevenZip";
}

CArchiver7ZIP::~CArchiver7ZIP()
{
	FreeDLL();
}

LOAD_RESULT CArchiver7ZIP::LoadDLL(CConfigManager &ConfMan,CString &strErr)
{
	FreeDLL();

	//基底クラスのメソッドを呼ぶ
	LOAD_RESULT res=CArchiverDLL::LoadDLL(ConfMan,strErr);
	if(LOAD_RESULT_OK!=res){
		return res;
	}

	//UNICODEモード設定用
	CStringA strFunctionName;
	strFunctionName=m_AstrPrefix+"SetUnicodeMode";
	ArchiverSetUnicodeMode=(COMMON_ARCHIVER_SETUNICODEMODE)GetProcAddress(m_hInstDLL,strFunctionName);
	if(NULL==ArchiverSetUnicodeMode){
		strErr.Format(IDS_ERROR_DLL_FUNCTION_GET,m_strDllName,(LPCTSTR)CA2T(strFunctionName));
		FreeDLL();
		TRACE(_T("%s\n"),(LPCTSTR)strErr);
		return LOAD_RESULT_INVALID;
	}
	//ここでUNICODEモードに設定
	if(!ArchiverSetUnicodeMode(TRUE)){
		//UNICODEにできなかったら失敗とする
		return LOAD_RESULT_INVALID;
	}

	//アーカイブ種別判定用
	strFunctionName=m_AstrPrefix+"GetArchiveType";
	ArchiverGetArchiveType=(COMMON_ARCHIVER_GETARCHIVETYPE)GetProcAddress(m_hInstDLL,strFunctionName);
	if(NULL==ArchiverGetArchiveType){
		strErr.Format(IDS_ERROR_DLL_FUNCTION_GET,m_strDllName,(LPCTSTR)CA2T(strFunctionName));
		FreeDLL();
		TRACE(_T("%s\n"),(LPCTSTR)strErr);
		return LOAD_RESULT_INVALID;
	}
	return LOAD_RESULT_OK;
}

void CArchiver7ZIP::FreeDLL()
{
	if(m_hInstDLL){
		ArchiverSetUnicodeMode=NULL;
		ArchiverGetArchiveType=NULL;
		CArchiverDLL::FreeDLL();
	}
}

bool CArchiver7ZIP::FormatCompressCommandZIP(const CConfigZIP &ConfZIP,CString &Param,bool bZIPSFX,int Options,LPCTSTR lpszMethod,LPCTSTR lpszLevel,CString &strLog)
{
	Param+=
		_T("a ")			//圧縮
		_T("-tzip ")		//ZIP形式で圧縮
		_T("-r0 ")			//サブディレクトリも検索
	;
	bool bBadSFX=false;
	//圧縮方式
	if(lpszMethod && *lpszMethod!=_T('\0')){
		Param+=_T("-mm=");
		Param+=lpszMethod;
		Param+=_T(" ");
	}else{
		switch(ConfZIP.CompressType){
		case ZIP_COMPRESS_DEFLATE:
			Param+=_T("-mm=Deflate ");
			break;
		case ZIP_COMPRESS_DEFLATE64:
			Param+=_T("-mm=Deflate64 ");
			break;
		case ZIP_COMPRESS_BZIP2:
			bBadSFX=true;
			Param+=_T("-mm=BZip2 ");
			break;
		case ZIP_COMPRESS_COPY:
			Param+=_T("-mm=Copy ");
			break;
		case ZIP_COMPRESS_LZMA:
			bBadSFX=true;
			Param+=_T("-mm=LZMA ");
			break;
		case ZIP_COMPRESS_PPMD:
			bBadSFX=true;
			Param+=_T("-mm=PPMd ");
			break;
		}
	}
	//圧縮レベル
	if(lpszLevel && *lpszLevel!=_T('\0')){
		Param+=_T("-mx=");
		Param+=lpszLevel;
		Param+=_T(" ");
	}else{
		switch(ConfZIP.CompressLevel){
		case ZIP_COMPRESS_LEVEL0:
			Param+=_T("-mx=0 ");
			break;
		case ZIP_COMPRESS_LEVEL5:
			Param+=_T("-mx=5 ");
			break;
		case ZIP_COMPRESS_LEVEL9:
			Param+=_T("-mx=9 ");
			break;
		}
	}
	if((ZIP_COMPRESS_DEFLATE==ConfZIP.CompressType)||(ZIP_COMPRESS_DEFLATE64==ConfZIP.CompressType)){
		if(ConfZIP.SpecifyDeflateMemorySize){
			CString temp;
			temp.Format(_T("-mfb=%d "),ConfZIP.DeflateMemorySize);
			Param+=temp;
		}
		if(ConfZIP.SpecifyDeflatePassNumber){
			CString temp;
			temp.Format(_T("-mpass=%d "),ConfZIP.DeflatePassNumber);
			Param+=temp;
		}
	}
	//文字コード強制
	if(ConfZIP.ForceUTF8){
		bBadSFX=true;
		Param+=_T("-mcu=on ");
	}
	if(Options&COMPRESS_PASSWORD){	//パスワード付き
		Param+=_T("-p ");

		//暗号モード
		switch(ConfZIP.CryptoMode){
		case ZIP_CRYPTO_ZIPCRYPTO:
			Param+=_T("-mem=ZipCrypto ");
			break;
		case ZIP_CRYPTO_AES128:
			bBadSFX=true;
			Param+=_T("-mem=AES128 ");
			break;
		case ZIP_CRYPTO_AES192:
			bBadSFX=true;
			Param+=_T("-mem=AES192 ");
			break;
		case ZIP_CRYPTO_AES256:
			bBadSFX=true;
			Param+=_T("-mem=AES256 ");
			break;
		}
	}

	if(bZIPSFX){
		//BZip2の自己解凍はサポートされていない
		if(bBadSFX){
			strLog=CString(MAKEINTRESOURCE(IDS_ERROR_ZIP_SFX_DONT_SUPPORT_FORMAT));
			return false;
		}
	}

	return true;
}

bool CArchiver7ZIP::FormatCompressCommand7Z(const CConfig7Z &Conf7Z,CString &Param,int Options,LPCTSTR lpszMethod,LPCTSTR lpszLevel,CString &strLog)
{
	Param+=
		_T("a ")			//圧縮
		_T("-t7z ")			//7-Zip形式で圧縮
		_T("-r0 ")			//サブディレクトリも検索
	;
	if(lpszMethod && *lpszMethod!=_T('\0')){
		Param+=_T("-m0=");
		Param+=lpszMethod;
		Param+=_T(" ");

		if(lpszLevel && *lpszLevel!=_T('\0')){
			Param+=_T("-mx=");
			Param+=lpszLevel;
			Param+=_T(" ");
		}
	}else{
		if(lpszLevel && *lpszLevel!=_T('\0')){
			Param+=_T("-mx=");
			Param+=lpszLevel;
			Param+=_T(" ");
		}else if(Conf7Z.UsePreset){
			//プリセット圧縮モード
			switch(Conf7Z.CompressLevel){	//圧縮レベル
			case SEVEN_ZIP_COMPRESS_LEVEL0:
				Param+=_T("-mx=0 ");
				break;
			case SEVEN_ZIP_COMPRESS_LEVEL1:
				Param+=_T("-mx=1 ");
				break;
			case SEVEN_ZIP_COMPRESS_LEVEL5:
				Param+=_T("-mx=5 ");
				break;
			case SEVEN_ZIP_COMPRESS_LEVEL7:
				Param+=_T("-mx=7 ");
				break;
			case SEVEN_ZIP_COMPRESS_LEVEL9:
				Param+=_T("-mx=9 ");
				break;
			}
		}
		if(!Conf7Z.UsePreset){
			switch(Conf7Z.CompressType){	//圧縮方式
			case SEVEN_ZIP_COMPRESS_LZMA:
				Param+=_T("-m0=LZMA:a=");
				switch(Conf7Z.LZMA_Mode){	//LZMA圧縮モード
				case SEVEN_ZIP_LZMA_MODE0:
					Param+=_T("0 ");
					break;
				case SEVEN_ZIP_LZMA_MODE1:
					Param+=_T("1 ");
					break;
				//case SEVEN_ZIP_LZMA_MODE2:
				//	Param+=_T("2 ");
				//	break;
				}
				break;
			case SEVEN_ZIP_COMPRESS_PPMD:
				Param+=_T("-m0=PPMd ");
				break;
			case SEVEN_ZIP_COMPRESS_BZIP2:
				Param+=_T("-m0=BZip2 ");
				break;
			case SEVEN_ZIP_COMPRESS_DEFLATE:
				Param+=_T("-m0=Deflate ");
				break;
			case SEVEN_ZIP_COMPRESS_COPY:
				Param+=_T("-m0=Copy ");
				break;
			case SEVEN_ZIP_COMPRESS_LZMA2:
				Param+=_T("-m0=LZMA2:a=");
				switch(Conf7Z.LZMA_Mode){	//LZMA圧縮モード
				case SEVEN_ZIP_LZMA_MODE0:
					Param+=_T("0 ");
					break;
				case SEVEN_ZIP_LZMA_MODE1:
					Param+=_T("1 ");
					break;
				}
			}
		}
	}
	if(Conf7Z.SolidMode){	//ソリッドモード
		Param+=_T("-ms=on ");
	}else{
		Param+=_T("-ms=off ");
	}
	//------------
	// ヘッダ圧縮
	//------------
	if(Conf7Z.HeaderCompression){	//ヘッダ圧縮
		Param+=_T("-mhc=on ");
	}else{
		Param+=_T("-mhc=off ");
		Param+=_T("-mhcf=off ");
	}

	if(Options&COMPRESS_PASSWORD){	//パスワード付き
		Param+=_T("-p ");
		if(Conf7Z.HeaderEncryption){	//ヘッダ暗号化
			Param+=_T("-mhe=on ");
		}
		else{
			Param+=_T("-mhe=off ");
		}
	}
	if(Options&COMPRESS_SFX){
		//BZip2の自己解凍はサポートされていない
		if(!Conf7Z.UsePreset && SEVEN_ZIP_COMPRESS_BZIP2==Conf7Z.CompressType){
			strLog=CString(MAKEINTRESOURCE(IDS_ERROR_7Z_SFX_DONT_SUPPORT_BZIP2));
			return false;
		}
		Param+=_T("-sfx ");
	}
	return true;
}

/*
formatの指定は、B2E32.dllでのみ有効
levelの指定は、B2E32.dll以外で有効
*/
bool CArchiver7ZIP::Compress(LPCTSTR ArcFileName,std::list<CString> &ParamList,CConfigManager &ConfMan,const PARAMETER_TYPE Type,int Options,LPCTSTR lpszFormat,LPCTSTR lpszMethod,LPCTSTR lpszLevel,CString &strLog)
{
	LPCTSTR lpszSplitSize = lpszFormat;

	if(!IsOK()){
		return false;
	}

	ASSERT(0!=_tcslen(ArcFileName));
	TRACE(_T("ArcFileName=%s\n"),ArcFileName);

	//分割書庫の場合は自己解凍は出来ない
	if((Options&COMPRESS_SPLIT)&&(Options&COMPRESS_SFX)){
		ErrorMessage(CString(MAKEINTRESOURCE(IDS_ERROR_CANNOT_SPLIT_SFX)));
		strLog=CString(MAKEINTRESOURCE(IDS_ERROR_CANNOT_SPLIT_SFX));
		Options&=~COMPRESS_SFX;
	}

	//==============================================
	// レスポンスファイル用テンポラリファイル名取得
	//==============================================
	TCHAR ResponceFileName[_MAX_PATH+1];
	FILL_ZERO(ResponceFileName);
	if(!UtilGetTemporaryFileName(ResponceFileName,_T("zip"))){
		strLog=CString(MAKEINTRESOURCE(IDS_ERROR_TEMPORARY_FILE_CREATE));
		return false;
	}
	ASSERT(0!=_tcslen(ResponceFileName));

	//============================================
	// 自己解凍ファイル用テンポラリファイル名取得
	//============================================
	TCHAR SFXTemporaryFileName[_MAX_PATH+1];
	FILL_ZERO(SFXTemporaryFileName);
	TCHAR SFXModulePath[_MAX_PATH+1];
	FILL_ZERO(SFXModulePath);
	bool bZIPSFX=((0!=(Options&COMPRESS_SFX))&&(PARAMETER_ZIP==Type));
	if(bZIPSFX){
		//2段階作成する
		if(!UtilGetTemporaryFileName(SFXTemporaryFileName,_T("sfx"))){
			strLog=CString(MAKEINTRESOURCE(IDS_ERROR_TEMPORARY_FILE_CREATE));
			return false;
		}
		ASSERT(0!=_tcslen(SFXTemporaryFileName));
		DeleteFile(SFXTemporaryFileName);//ゴミファイル消去

		//---SFXモジュール
		LPTSTR lptemp;
		{
			//安全なパスに移動;DLL読み込み対策
			CCurrentDirManager cdm(UtilGetModuleDirectoryPath());
			if(!SearchPath(NULL,_T("SFX32GUI.DAT"),NULL,_MAX_PATH,SFXModulePath,&lptemp)){
				strLog.Format(IDS_ERROR_SFX_MODULE_NOT_FOUND,_T("SFX32GUI.DAT"));
				return false;
			}
		}
	}

	//====================================================
	// レスポンスファイル内に圧縮対象ファイル名を記入する
	// アーカイブファイル名はコマンドラインで直接指定する
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
			CPath strPath=*ite;

			if(strPath.IsDirectory()){
				strPath.Append(_T("*"));
			}
			WriteResponceFile(hFile,strPath);
		}
		CloseHandle(hFile);
	}

	//===========================
	// DLLに渡すオプションの設定
	//===========================
	TRACE(_T("DLLに渡すオプションの設定\n"));

	CString Param;//コマンドライン パラメータ バッファ

	CConfigZIP confZIP;
	CConfig7Z  conf7Z;
	switch(Type){
	case PARAMETER_ZIP:	//ZIP形式で圧縮
		confZIP.load(ConfMan);
		break;
	case PARAMETER_7Z:	//7z形式で圧縮
		conf7Z.load(ConfMan);
		break;
	}

	switch(Type){
	case PARAMETER_ZIP:	//ZIP形式で圧縮
		if(!FormatCompressCommandZIP(confZIP,Param,bZIPSFX,Options,lpszMethod,lpszLevel,strLog)){
			DeleteFile(ResponceFileName);
			return false;
		}
		break;
	case PARAMETER_7Z:	//7z形式で圧縮
		if(!FormatCompressCommand7Z(conf7Z,Param,Options,lpszMethod,lpszLevel,strLog)){
			DeleteFile(ResponceFileName);
			return false;
		}
		break;
	}
	//分割
	if(Options&COMPRESS_SPLIT){
		if(lpszSplitSize && _tcslen(lpszSplitSize)>0){
			CString temp;
			temp.Format(_T("-v%s "),lpszSplitSize);
			Param+=temp;
		}else{
			int unitIndex=-1;
			int size=-1;
			switch(Type){
			case PARAMETER_ZIP:	//ZIP形式で圧縮
				if(confZIP.SpecifySplitSize){
					unitIndex = confZIP.SplitSizeUnit;
					size = confZIP.SplitSize;
				}
				break;
			case PARAMETER_7Z:	//7z形式で圧縮
				if(conf7Z.SpecifySplitSize){
					unitIndex = conf7Z.SplitSizeUnit;
					size = conf7Z.SplitSize;
				}
				break;
			}

			if(size==-1 && unitIndex==-1){
				C7Zip32VolumeSizeDialog vsd;
				if(IDOK!=vsd.DoModal())return false;
				unitIndex = vsd.SelectIndex;
				size = vsd.VolumeSize;
			}

			CString temp;
			temp.Format(_T("-v%d%s "),size,ZIP_VOLUME_UNIT[unitIndex].ParamName);
			Param+=temp;
		}
	}

	Param+=_T("-scsUTF-8 ");	//レスポンスファイルのコードページ指定

	//作業ディレクトリ
	Param+=_T("\"-w");
	Param+=UtilGetTempPath();
	Param+=_T("\" ");

	//圧縮先ファイル名指定
	if(bZIPSFX){
		Param+=_T("\"");
		Param+=SFXTemporaryFileName;
		Param+=_T("\" ");
	}
	else{
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
	std::vector<BYTE> szLog(LOG_BUFFER_SIZE);
	szLog[0]='\0';
	int Ret=ArchiveHandler(NULL,C2UTF8(Param),(LPSTR)&szLog[0],LOG_BUFFER_SIZE-1);
	CString strTmp;
	UtilToUNICODE(strTmp,&szLog[0],szLog.size()-1,UTILCP_UTF8);
	//strLog=&szLog[0];
	strLog=strTmp;

	//使ったレスポンスファイルは消去
	DeleteFile(ResponceFileName);

	//エラー時ログ出力
	if(!bZIPSFX||0!=Ret){
		if(bZIPSFX){
			DeleteFile(SFXTemporaryFileName);
		}
		return 0==Ret;
	}

	//==================================
	// 自己解凍書庫に変換(バイナリ結合)
	//==================================
	//出力先ファイルを開く
	HANDLE hArcFile=CreateFile(ArcFileName,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
	if(INVALID_HANDLE_VALUE==hArcFile){
		strLog.Format(IDS_ERROR_ACCESS_OUTPUT_FILE,ArcFileName);
		DeleteFile(ArcFileName);
		DeleteFile(SFXTemporaryFileName);
		return false;
	}
	{
		//SFXモジュールを読み取りモードで開く
		HANDLE hSFXModuleFile=CreateFile(SFXModulePath,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
		if(INVALID_HANDLE_VALUE==hSFXModuleFile){
			strLog.Format(IDS_ERROR_SFX_MODULE_CANNOT_ACCESS,SFXModulePath);
			CloseHandle(hArcFile);
			DeleteFile(ArcFileName);
			DeleteFile(SFXTemporaryFileName);
			return false;
		}
		//SFXモジュールの中身をコピー
		int Result=UtilAppendFile(hArcFile,hSFXModuleFile);
		if(Result>0){	//読み取りエラー
			strLog.Format(IDS_ERROR_SFX_MODULE_CANNOT_ACCESS,SFXModulePath);
			CloseHandle(hArcFile);
			CloseHandle(hSFXModuleFile);
			DeleteFile(ArcFileName);
			DeleteFile(SFXTemporaryFileName);
			return false;
		}
		else if(Result<0){	//書き込みエラー
			strLog.Format(IDS_ERROR_ACCESS_OUTPUT_FILE,ArcFileName);
			CloseHandle(hArcFile);
			CloseHandle(hSFXModuleFile);
			DeleteFile(ArcFileName);
			DeleteFile(SFXTemporaryFileName);
			return false;
		}
		CloseHandle(hSFXModuleFile);
	}
	{
		//テンポラリファイルを読み取りモードで開く
		HANDLE hSFXTempFile=CreateFile(SFXTemporaryFileName,GENERIC_READ,0,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
		if(INVALID_HANDLE_VALUE==hSFXTempFile){
			strLog=CString(MAKEINTRESOURCE(IDS_ERROR_TEMPORARY_FILE_ACCESS));
			CloseHandle(hArcFile);
			DeleteFile(ArcFileName);
			DeleteFile(SFXTemporaryFileName);
			return false;
		}
		//テンポラリファイルの中身をコピー
		int Result=UtilAppendFile(hArcFile,hSFXTempFile);
		if(Result>0){	//読み取りエラー
			strLog=CString(MAKEINTRESOURCE(IDS_ERROR_TEMPORARY_FILE_ACCESS));
			CloseHandle(hArcFile);
			CloseHandle(hSFXTempFile);
			DeleteFile(ArcFileName);
			DeleteFile(SFXTemporaryFileName);
			return false;
		}
		else if(Result<0){	//書き込みエラー
			strLog.Format(IDS_ERROR_ACCESS_OUTPUT_FILE,ArcFileName);
			CloseHandle(hArcFile);
			CloseHandle(hSFXTempFile);
			DeleteFile(ArcFileName);
			DeleteFile(SFXTemporaryFileName);
			return false;
		}
		CloseHandle(hSFXTempFile);
		DeleteFile(SFXTemporaryFileName);
	}
	CloseHandle(hArcFile);

	return true;
}

bool CArchiver7ZIP::Extract(LPCTSTR ArcFileName,CConfigManager&,const CConfigExtract& Config,bool bSafeArchive,LPCTSTR OutputDir,CString &strLog)
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
	Param+=_T("x ");			//解凍
	if(Config.ForceOverwrite){
		//強制上書き
		Param+=_T("-aoa ");
	}

	Param+=_T("-scsUTF-8 ");	//レスポンスファイルのコードページ指定

	//作業ディレクトリ
	Param+=_T("\"-w");
	Param+=UtilGetTempPath();
	Param+=_T("\" ");

	//アーカイブファイル名指定
	Param+=_T("\"");
	Param+=ArcFileName;
	Param+=_T("\" ");

	ASSERT(!Param.IsEmpty());
	TRACE(_T("ArchiveHandler Commandline Parameter:%s\n"),Param);

	TRACE(_T("ArchiveHandler呼び出し\n"));
	std::vector<BYTE> szLog(LOG_BUFFER_SIZE);
	szLog[0]='\0';
	int Ret=ArchiveHandler(NULL,C2UTF8(Param),(LPSTR)&szLog[0],LOG_BUFFER_SIZE-1);
	CString strTmp;
	UtilToUNICODE(strTmp,&szLog[0],szLog.size()-1,UTILCP_UTF8);
	//strLog=&szLog[0];
	strLog=strTmp;

	return 0==Ret;
}

//レスポンスファイルにファイル名をエスケープを実行した上で書き込む。
//有効なファイルハンドルとNULLでないファイル名を渡すこと。
void CArchiver7ZIP::WriteResponceFile(HANDLE hFile,LPCTSTR fname)
{
	CPath strPath=fname;

	strPath.QuoteSpaces();

	DWORD dwWritten=0;
	//ファイル名+改行を出力
	std::vector<BYTE> cArray;
	UtilToUTF8(cArray,strPath+_T("\r\n"));
	WriteFile(hFile,&cArray[0],(cArray.size()-1)*sizeof(BYTE),&dwWritten,NULL);
}


//=============================================================
// SevenZipGetFileName()の出力結果を基に、格納されたファイルが
// パス情報を持っているかどうか判別し、二重フォルダ作成を防ぐ
//=============================================================
bool CArchiver7ZIP::ExamineArchive(LPCTSTR ArcFileName,CConfigManager& ConfMan,bool bSkipDir,bool &bInFolder,bool &bSafeArchive,CString &BaseDir,CString &strErr)
{
	/*
	Separator('/' or '\')は格納ファイルの先頭にいくら含まれていても無視すべきであるので、
	格納ファイル名の先頭にいくらSeparatorがあってもフォルダに格納された状態とは見なさない。
	SeparatorがMaxRepeatより多いと不正とする
	ただし、MaxRepeatが-1のときはエラーとはしない
	*/
	const int MaxRepeat = -1;

	ASSERT(IsOK());
	if (!IsOK()) {
		return false;
	}

	ARCHIVE_FILE arc;
	if (!InspectArchiveBegin(arc, ArcFileName, ConfMan)) {
		strErr.Format(IDS_ERROR_OPEN_ARCHIVE, ArcFileName);
		return false;
	}

	bInFolder = true;
	bool bSureDir = false;	//BaseDirに入っている文字列が確かにフォルダであるならtrue
	TRACE(_T("========\n"));

	while (InspectArchiveNext(arc)) {
		CString Buffer;
		InspectArchiveGetFileName(arc,Buffer);
		Buffer.Replace(_T('\\'), _T('/'));		//パス区切り文字の置き換え
		TRACE(_T("%s\n"), Buffer);

		const int Length = Buffer.GetLength();
		int StartIndex = 0;
		for (; StartIndex < Length; StartIndex++) {
			//先頭の'/'をとばしていく
#if defined(_UNICODE)||defined(UNICODE)
			if (_T('/') != Buffer[StartIndex])break;
#else
			if (_MBC_SINGLE == _mbsbtype((const unsigned char *)(LPCTSTR)Buffer, StartIndex)) {
				if (_T('/') != Buffer[StartIndex])break;
			} else {	//全角文字なら'/'であるはずがない
				break;
			}
#endif//defined(_UNICODE)||defined(UNICODE)
			if (-1 != MaxRepeat) {
				if (StartIndex >= MaxRepeat) {	//'/'がMaxRepeat個以上続く場合
					//危険なファイルと分かったので監査終了
					InspectArchiveEnd(arc);
					bSafeArchive = false;
					bInFolder = false;
					return true;
				}
			}
		}
		if ((-1 != Buffer.Find(_T("../"), StartIndex)) ||	//相対パス指定が見つかれば、それは安全なファイルではない
			(-1 != Buffer.Find(_T(':'), StartIndex))) {	//ドライブ名が見つかれば、それは安全なファイルではない
			//危険なファイルと分かったので監査終了
			InspectArchiveEnd(arc);
			bSafeArchive = false;
			bInFolder = false;
			return true;
		}

		//ここからは二重ディレクトリ判定
		//すでに二重ディレクトリ判定が付いている場合は安全判定のみに徹する

		int FoundIndex = 0;
		while (bInFolder) {
			FoundIndex = Buffer.Find(_T('/'), StartIndex);
			if (-1 == FoundIndex) {	//'/'が格納ファイル名の先頭以外に含まれない場合
				if (!BaseDir.IsEmpty() && BaseDir == Buffer) {
					bSureDir = true;	//BaseDirがフォルダであると確認された
					break;
				} else if (BaseDir.IsEmpty()) {
					//フォルダ名の後ろに'/'が付かないアーカイバもある
					//そういうものが最初に出てきたときは、フォルダ名と仮定する
					BaseDir = Buffer;
					bSureDir = false;
					break;
				}
			}
			CString Dir = Buffer.Mid(StartIndex, FoundIndex - StartIndex);	//Separatorの前までの文字列(ディレクトリに相当)を抜き出してくる
			//これまでの調べでDirはEmptyではないことが保証されている
			//また、危険ではないことも分かっている
			TRACE(_T("Base=%s,Dir=%s\n"), BaseDir, Dir);

			if (_T('.') == Dir) {	//./があればディレクトリ指定としては無視する
				StartIndex = FoundIndex + 1;
				continue;
			}
			if (BaseDir.IsEmpty()) {
				BaseDir = Dir;
				bSureDir = true;
			} else if (BaseDir != Dir) {
				bInFolder = false;
			} else bSureDir = true;	//BaseDirがディレクトリと確認された
			break;
		}
	}
	TRACE(_T("========\n"));

	InspectArchiveEnd(arc);
	bSafeArchive = true;

	//フォルダに入っているようではあるが、ディレクトリと仮定されただけの場合
	if (bInFolder && !bSureDir)bInFolder = false;
	return true;

}

bool CArchiver7ZIP::ExtractSpecifiedOnly(LPCTSTR ArcFileName,CConfigManager&,LPCTSTR OutputDir,std::list<CString> &FileList,CString &strLog,bool bUsePath)
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
	if(!UtilGetTemporaryFileName(ResponceFileName,_T("7zp"))){
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
			TRACE(_T("%s=>%s\n"),LPCTSTR(*ite),OutputDir);
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
		Param+=_T("x ");	//パス付き解凍
	}else{
		Param+=_T("e ");		//ディレクトリなしで解凍
	}
	Param+=
		_T("-r- ")		//再帰検索無効
		_T("-scsUTF-8 ")	//レスポンスファイルのコードページ指定
	;

	//作業ディレクトリ
	Param+=_T("\"-w");
	Param+=UtilGetTempPath();
	Param+=_T("\" ");

	//アーカイブファイル名指定
	Param+=_T("\"");
	Param+=ArcFileName;
	Param+=_T("\" ");

	//出力先指定
	Param+=_T("-o\"");
	Param+=OutputDir;
	Param+=_T("\" ");

	//レスポンスファイル名指定
	Param+=_T("\"@");
	Param+=ResponceFileName;
	Param+=_T("\"");

	TRACE(_T("ArchiveHandler呼び出し\nCommandline Parameter:%s\n"),Param);
	std::vector<BYTE> szLog(LOG_BUFFER_SIZE);
	szLog[0]='\0';
	int Ret=ArchiveHandler(NULL,C2UTF8(Param),(LPSTR)&szLog[0],LOG_BUFFER_SIZE-1);
	CString strTmp;
	UtilToUNICODE(strTmp,&szLog[0],szLog.size()-1,UTILCP_UTF8);
	//strLog=&szLog[0];
	strLog=strTmp;
	//使ったレスポンスファイルは消去
	DeleteFile(ResponceFileName);

	return 0==Ret;
}


bool CArchiver7ZIP::DeleteItemFromArchive(LPCTSTR ArcFileName,CConfigManager&,const std::list<CString>& FileList,CString &strLog)
{
	if(!IsOK()){
		return false;
	}

	//==============================================
	// レスポンスファイル用テンポラリファイル名取得
	//==============================================
	TCHAR ResponceFileName[_MAX_PATH+1];
	FILL_ZERO(ResponceFileName);
	if(!UtilGetTemporaryFileName(ResponceFileName,_T("7zp"))){
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
			CString tmp=*ite;
			if(tmp[tmp.GetLength()-1]==_T('\\'))tmp.Delete(tmp.GetLength()-1);
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
		_T("d ")	//削除
		_T("-r- ")	//再帰検索無効
		_T("-scsUTF-8 ")	//レスポンスファイルのコードページ指定
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
	Param+=_T("\"@");
	Param+=ResponceFileName;
	Param+=_T("\"");

	TRACE(_T("ArchiveHandler呼び出し\nCommandline Parameter:%s\n"),Param);
	//char szLog[LOG_BUFFER_SIZE]={0};
	std::vector<BYTE> szLog(LOG_BUFFER_SIZE);
	szLog[0]='\0';
	int Ret=ArchiveHandler(NULL,C2UTF8(Param),(LPSTR)&szLog[0],LOG_BUFFER_SIZE-1);
	CString strTmp;
	UtilToUNICODE(strTmp,&szLog[0],szLog.size()-1,UTILCP_UTF8);
	//strLog=&szLog[0];
	strLog=strTmp;
	//使ったレスポンスファイルは消去
	DeleteFile(ResponceFileName);

	return 0==Ret;
}

ARCRESULT CArchiver7ZIP::TestArchive(LPCTSTR ArcFileName,CString &strLog)
{
	TRACE(_T("TestArchive() called.\n"));
	ASSERT(IsOK());
	if(!IsOK()){
		return TEST_ERROR;
	}

	//tコマンドによるテストが実装されている
	CString Param=
		_T("t ")			//テスト
		_T("\"")
	;
	Param+=ArcFileName;
	Param+=_T("\"");

	//char szLog[LOG_BUFFER_SIZE]={0};
	std::vector<BYTE> szLog(LOG_BUFFER_SIZE);
	szLog[0]='\0';
	int Ret=ArchiveHandler(NULL,C2UTF8(Param),(LPSTR)&szLog[0],LOG_BUFFER_SIZE-1);
	CString strTmp;
	UtilToUNICODE(strTmp,&szLog[0],szLog.size()-1,UTILCP_UTF8);
	//strLog=&szLog[0];
	strLog=strTmp;



	if(Ret==0)return TEST_OK;
	else return TEST_NG;
}

bool CArchiver7ZIP::AddItemToArchive(LPCTSTR ArcFileName, bool bEncrypted, const std::list<CString> &FileList, CConfigManager &ConfMan, LPCTSTR lpDestDir, CString &strLog)
{
	// レスポンスファイル用テンポラリファイル名取得
	TCHAR ResponceFileName[_MAX_PATH + 1];
	FILL_ZERO(ResponceFileName);
	if (!UtilGetTemporaryFileName(ResponceFileName, _T("zip"))) {
		strLog = CString(MAKEINTRESOURCE(IDS_ERROR_TEMPORARY_FILE_CREATE));
		return false;
	}
	ASSERT(0 != _tcslen(ResponceFileName));

	//===一時的にファイルをコピー
	//---\で終わる基点パスを取得
	CPath strBasePath;
	UtilGetBaseDirectory(strBasePath, FileList);
	TRACE(_T("%s\n"), strBasePath);

	//---テンポラリに対象ファイルをコピー
	//テンポラリ準備
	CTemporaryDirectoryManager tdm(_T("lhaf"));
	CPath strDestPath(tdm.GetDirPath());
	strDestPath += lpDestDir;
	UtilMakeSureDirectoryPathExists(strDestPath);

	// 圧縮対象ファイル名を修正する
	const int BasePathLength = ((CString)strBasePath).GetLength();
	CString strSrcFiles;	//コピー元ファイルの一覧
	CString strDestFiles;	//コピー先ファイルの一覧
	std::list<CString>::const_iterator ite;
	for (ite = FileList.begin(); ite != FileList.end(); ++ite) {
		//ベースパスを元に相対パス取得 : 共通である基底パスの文字数分だけカットする
		LPCTSTR lpSrc((LPCTSTR)(*ite) + BasePathLength);

		//送り側ファイル名指定
		strSrcFiles += (strBasePath + lpSrc);	//PathAppend相当
		strSrcFiles += _T('|');
		//受け側ファイル名指定
		strDestFiles += strDestPath + lpSrc;
		strDestFiles += _T('|');
	}
	strSrcFiles += _T('|');
	strDestFiles += _T('|');

	//'|'を'\0'に変換する
	std::vector<TCHAR> srcBuf(strSrcFiles.GetLength() + 1);
	UtilMakeFilterString(strSrcFiles, &srcBuf[0], srcBuf.size());
	std::vector<TCHAR> destBuf(strDestFiles.GetLength() + 1);
	UtilMakeFilterString(strDestFiles, &destBuf[0], destBuf.size());

	//ファイル操作内容
	SHFILEOPSTRUCT fileOp = { 0 };
	fileOp.wFunc = FO_COPY;
	fileOp.fFlags = FOF_MULTIDESTFILES | FOF_NOCONFIRMATION | FOF_NOCONFIRMMKDIR | FOF_NOCOPYSECURITYATTRIBS | FOF_NO_CONNECTED_ELEMENTS;
	fileOp.pFrom = &srcBuf[0];
	fileOp.pTo = &destBuf[0];

	//コピー実行
	if (::SHFileOperation(&fileOp)) {
		//エラー
		strLog = CString(MAKEINTRESOURCE(IDS_ERROR_FILE_COPY));
		return false;
	} else if (fileOp.fAnyOperationsAborted) {
		//キャンセル
		strLog = CString(MAKEINTRESOURCE(IDS_ERROR_USERCANCEL));
		return false;
	}

	//カレントディレクトリ設定
	::SetCurrentDirectory(tdm.GetDirPath());
	// 同時に、レスポンスファイル内にアーカイブ名および圧縮対象ファイル名を記入する
	{
		HANDLE hFile = CreateFile(ResponceFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (INVALID_HANDLE_VALUE == hFile) {
			strLog = CString(MAKEINTRESOURCE(IDS_ERROR_TEMPORARY_FILE_ACCESS));
			return false;
		}
		//レスポンスファイルへの書き込み
		//全て圧縮
		WriteResponceFile(hFile, _T("*"));
		CloseHandle(hFile);
	}


	//===========================
	// DLLに渡すオプションの設定
	//===========================
	TRACE(_T("DLLに渡すオプションの設定\n"));

	CString Param;//コマンドライン パラメータ バッファ

	int option = 0;
	if (bEncrypted) {
		option |= COMPRESS_PASSWORD;
	}
	CConfigZIP confZIP;
	CConfig7Z  conf7Z;
	ASSERT(ArchiverGetArchiveType);
	switch (ArchiverGetArchiveType(C2UTF8(ArcFileName))) {
	case 1:	//ZIP形式で圧縮
		confZIP.load(ConfMan);
		if (!FormatCompressCommandZIP(confZIP, Param, false, option, NULL, NULL, strLog)) {
			DeleteFile(ResponceFileName);
			return false;
		}
		break;
	case 2:	//7z形式で圧縮
		conf7Z.load(ConfMan);
		if (!FormatCompressCommand7Z(conf7Z, Param, option, NULL, NULL, strLog)) {
			DeleteFile(ResponceFileName);
			return false;
		}
		break;
	default:
		ASSERT(!"This code cannot be run");
		//エラー処理が面倒なので放っておく。
		return false;
	}

	Param += _T("-scsUTF-8 ");	//レスポンスファイルのコードページ指定

	//作業ディレクトリ
	Param += _T("\"-w");
	Param += UtilGetTempPath();
	Param += _T("\" ");

	//圧縮先ファイル名指定
	Param += _T("\"");
	Param += ArcFileName;
	Param += _T("\" ");

	//レスポンスファイル名指定
	Param += _T("\"@");
	Param += ResponceFileName;
	Param += _T("\"");

	ASSERT(!Param.IsEmpty());
	TRACE(_T("ArchiveHandler Commandline Parameter:%s\n"), Param);

	TRACE(_T("ArchiveHandler呼び出し\n"));
	//char szLog[LOG_BUFFER_SIZE]={0};
	std::vector<BYTE> szLog(LOG_BUFFER_SIZE);
	szLog[0] = '\0';
	int Ret = ArchiveHandler(NULL, C2UTF8(Param), (LPSTR)&szLog[0], LOG_BUFFER_SIZE - 1);
	CString strTmp;
	UtilToUNICODE(strTmp, &szLog[0], szLog.size() - 1, UTILCP_UTF8);
	//strLog=&szLog[0];
	strLog = strTmp;

	//使ったレスポンスファイルは消去
	DeleteFile(ResponceFileName);

	return 0 == Ret;
}


//指定されたディレクトリ内部を展開する;高速実装
bool CArchiver7ZIP::ExtractDirectoryEntry(LPCTSTR lpszArcFile, CConfigManager &ConfMan, const ARCHIVE_ENTRY_INFO_TREE* lpBase, const ARCHIVE_ENTRY_INFO_TREE* lpDir, LPCTSTR lpszOutputBaseDir, bool bCollapseDir, CString &strLog)
{
	//---一時フォルダ中にまとめて展開し、後からフォルダ構造を切り出す
	std::list<CString> files;

	CString strPath;

	bool bRestoreDir = false;
	if (lpDir->strFullPath.IsEmpty()) {
		//ディレクトリが登録されていないのでパス名を算出する
		ArcEntryInfoTree_GetNodePathRelative(lpDir, lpBase, strPath);
		strPath.Replace(_T('/'), _T('\\'));

		CPath tmpPath(strPath);
		tmpPath.RemoveBackslash();	//ディレクトリだったら裸にする
		tmpPath.RemoveFileSpec();	//親ディレクトリまで切りつめる
		tmpPath.Append(_T("*"));	//特定ディレクトリ以下の全てのファイルを展開
		files.push_back(tmpPath);

		bRestoreDir = true;
	} else {
		//ディレクトリもアーカイブ中にエントリとして登録されているなら出力する
		CString tmpPath(lpDir->strFullPath);
		if (tmpPath[tmpPath.GetLength() - 1] == _T('\\') || tmpPath[tmpPath.GetLength() - 1] == _T('/')) {
			//末尾の\もしくは/を削除
			tmpPath.Delete(tmpPath.GetLength() - 1);
		}
		files.push_back(tmpPath);

		strPath = lpDir->strFullPath;
		strPath.Replace(_T('/'), _T('\\'));
	}

	//--------------------------------------
	// 修正された出力ディレクトリパスを算出
	//--------------------------------------
	//---本来の出力先
	CString strOutputDir = lpszOutputBaseDir + strPath;
	if (strOutputDir.GetLength() > _MAX_PATH) {
		//フォルダ名が長くなりすぎた
		strLog = CString(MAKEINTRESOURCE(IDS_ERROR_MAX_PATH));
		return false;
	}

	//FileListには１件しか入っていないはず
	ASSERT(files.size() == 1);

	//一時フォルダ
	CTemporaryDirectoryManager tdm(_T("lhaf"));
	CPath strTempOutput(tdm.GetDirPath());
	if (bRestoreDir) {	//ディレクトリを手作業で復元
		strTempOutput += strPath;
		strTempOutput.AddBackslash();
		TRACE(_T("手作業でのディレクトリ復元:%s\n"), (LPCTSTR)strTempOutput);
		if (!UtilMakeSureDirectoryPathExists(strTempOutput)) {
			strLog.Format(IDS_ERROR_CANNOT_MAKE_DIR, strTempOutput);
			return false;
		}
	}
	//---一時フォルダ中にまとめて展開し、後からフォルダ構造を切り出す
	// ファイルを展開
	if (!ExtractSpecifiedOnly(lpszArcFile, ConfMan, strTempOutput, files, strLog, true)) {
		return false;
	}

	//送り側ファイル名指定
	CPath tmp(strTempOutput);
	tmp += (LPCTSTR)strPath;	//PathAppend相当
	tmp.RemoveBackslash();
	CString strSrcFiles(tmp);
	strSrcFiles += _T("||");
	//受け側ファイル名指定
	tmp = lpszOutputBaseDir;
	{
		CString strTmp;
		ArcEntryInfoTree_GetNodePathRelative(lpDir, lpBase, strTmp);
		strTmp.Replace(_T('/'), _T('\\'));
		tmp += (LPCTSTR)strTmp;
	}
	tmp.AddBackslash();
	CString strDestFiles(tmp);
	strDestFiles += _T("||");

	//'|'を'\0'に変換する
	std::vector<TCHAR> srcBuf(strSrcFiles.GetLength() + 1);
	UtilMakeFilterString(strSrcFiles, &srcBuf[0], srcBuf.size());
	std::vector<TCHAR> destBuf(strDestFiles.GetLength() + 1);
	UtilMakeFilterString(strDestFiles, &destBuf[0], destBuf.size());

	//ファイル操作内容
	SHFILEOPSTRUCT fileOp = { 0 };
	fileOp.wFunc = FO_MOVE;
	fileOp.fFlags = FOF_MULTIDESTFILES |/*FOF_NOCONFIRMATION|*/FOF_NOCONFIRMMKDIR | FOF_NOCOPYSECURITYATTRIBS | FOF_NO_CONNECTED_ELEMENTS;
	fileOp.pFrom = &srcBuf[0];
	fileOp.pTo = &destBuf[0];

	//移動実行
	if (::SHFileOperation(&fileOp)) {
		//エラー
		strLog = CString(MAKEINTRESOURCE(IDS_ERROR_FILE_MOVE));
		return false;
	} else if (fileOp.fAnyOperationsAborted) {
		//キャンセル
		strLog = CString(MAKEINTRESOURCE(IDS_ERROR_USERCANCEL));
		return false;
	}

	return true;

}


//-------------------------------
//---UNICODE版をオーバーライド---
//-------------------------------
BOOL CArchiver7ZIP::CheckArchive(LPCTSTR _szFileName)
{
	if(!ArchiverCheckArchive){
		ASSERT(ArchiverCheckArchive);
		return false;
	}
	//7-Zip32.dllは、ヘッダ暗号化ファイルのCheckArchive時、ウィンドウハンドルが存在しないと失敗する
	CDummyWindow dummy;
	dummy.Create(NULL,CWindow::rcDefault);
	BOOL bRet=ArchiverCheckArchive(C2UTF8(_szFileName),CHECKARCHIVE_BASIC);
	dummy.DestroyWindow();
	return bRet;
}

bool CArchiver7ZIP::isContentSingleFile(LPCTSTR _szFileName)
{
	ARCHIVE_FILE a;
	int r = archive_read_open_filename_w(a, _szFileName, 10240);
	if (r == ARCHIVE_OK) {
		return true;
	} else {
		return false;
	}
	for (size_t count = 0; count < 2; count++) {
		struct archive_entry *entry;
		if (ARCHIVE_OK != archive_read_next_header(a, &entry)) {
			return true;
		}
	}
	return false;
}


bool CArchiver7ZIP::InspectArchiveBegin(ARCHIVE_FILE& arc, LPCTSTR ArcFileName, CConfigManager&)
{
	int r = archive_read_open_filename_w(arc, ArcFileName, 10240);
	if (r == ARCHIVE_OK) {
		return true;
	} else {
		return false;
	}
}

bool CArchiver7ZIP::InspectArchiveGetFileName(ARCHIVE_FILE& arc, CString &FileName)
{
	ASSERT(arc._arc);
	if (arc._arc) {
		FileName = archive_entry_pathname_w(arc);
		return true;
	} else {
		return false;
	}
}

bool CArchiver7ZIP::InspectArchiveEnd(ARCHIVE_FILE& arc)
{
	arc.close();
	return true;
}


bool CArchiver7ZIP::InspectArchiveNext(ARCHIVE_FILE& arc)
{
	return ARCHIVE_OK == archive_read_next_header(arc, &arc._entry);
}

int CArchiver7ZIP::InspectArchiveGetAttribute(ARCHIVE_FILE& arc)
{
	//TODO:statをそのまま読めるようにする
	const auto *stat = archive_entry_stat(arc);
	int ret = 0;
	if (stat->st_mode & S_IFDIR) {
		ret |= FA_DIREC;
	}
	return ret;
}

bool CArchiver7ZIP::InspectArchiveGetOriginalFileSize(ARCHIVE_FILE& arc, LARGE_INTEGER &FileSize)
{
	if (archive_entry_size_is_set(arc)) {
		const auto size = archive_entry_size(arc);
		FileSize.QuadPart = size;
		return true;
	} else {
		return false;
	}
}

bool CArchiver7ZIP::InspectArchiveGetWriteTime(ARCHIVE_FILE& arc,FILETIME &FileTime)
{
	const auto mtime = archive_entry_mtime(arc);
	UtilUnixTimeToFileTime(mtime, &FileTime);
	return true;
}

