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
#include "FileOperation.h"
#include "Utility.h"
#include "StringUtil.h"
#include "OSUtil.h"


#if !defined(_UNICODE)&&!defined(UNICODE)
 #include <imagehlp.h> //MakeSureDirectoryPathExists()
#endif

LPCTSTR UtilGetTempPath()
{
	static CPath s_tempPath;
	if(_tcslen(s_tempPath)==0){		//初期設定
		//環境変数取得
		std::map<stdString,stdString> envs;
		UtilGetEnvInfo(envs);
		if(!has_key(envs,_T("TMP")) && !has_key(envs,_T("TEMP"))){
			//%TMP%/%TEMP%が存在しなければ自前の一時フォルダを使う(C:\Users\xxx\AppData\Roaming\LhaForge\temp)
			TCHAR szPath[_MAX_PATH+1]={0};
			SHGetFolderPath(NULL,CSIDL_APPDATA|CSIDL_FLAG_CREATE,NULL,SHGFP_TYPE_CURRENT,szPath);
			s_tempPath=szPath;
			s_tempPath.Append(_T("lhaforge\\temp\\"));
			UtilMakeSureDirectoryPathExists(s_tempPath);
		}else{
			//通常のパス
			std::vector<TCHAR> buffer(GetTempPath(0,NULL)+1);
			GetTempPath(buffer.size(),&buffer[0]);
			buffer.back()=_T('\0');
			s_tempPath=&buffer[0];
			s_tempPath.AddBackslash();
		}
	}
	return s_tempPath;
}

bool UtilGetTemporaryFileName(LPTSTR fname,LPCTSTR prefix)
{
	if(!GetTempFileName(UtilGetTempPath(),prefix,0,fname)){
		return false;
	}
	return true;
}

bool UtilDeletePath(LPCTSTR PathName)
{
	if( PathIsDirectory(PathName) ) {//ディレクトリ
		//ファイルの属性を標準に戻す
		if( UtilDeleteDir(PathName, true) )return true;
	} else if( PathFileExists(PathName) ) {//ファイル
		//ファイルの属性を標準に戻す
		SetFileAttributes(PathName, FILE_ATTRIBUTE_NORMAL);
		if( DeleteFile(PathName) )return true;
	}
	return false;
}

//bDeleteParent=trueのとき、Path自身も削除する
//bDeleteParent=falseのときは、Pathの中身だけ削除する
bool UtilDeleteDir(LPCTSTR Path,bool bDeleteParent)
{
	std::vector<WIN32_FIND_DATA> lps;

	TCHAR FindParam[_MAX_PATH+1];
	FILL_ZERO(FindParam);
	_tcsncpy_s(FindParam,Path,_MAX_PATH);
	PathAppend(FindParam, _T("*"));

	{
		WIN32_FIND_DATA fd;
		HANDLE h = FindFirstFile(FindParam, &fd);
		do {
			lps.push_back(fd);
		} while( FindNextFile(h, &fd) );
		FindClose(h);
	}

	bool bRet=true;

	for( std::vector<WIN32_FIND_DATA>::const_iterator ite = lps.begin(); ite != lps.end();++ite ) {
		const WIN32_FIND_DATA& lp = *ite;
		if( ( lp.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) && 0 != _tcscmp(lp.cFileName, _T("..")) && 0 != _tcscmp(lp.cFileName, _T(".")) ) {
			CString SubPath = Path;
			SubPath += _T("\\");
			SubPath += lp.cFileName;

			bRet = bRet&&UtilDeleteDir(SubPath, true);
		}
		if( ( lp.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) != FILE_ATTRIBUTE_DIRECTORY ) {
			// lp.cFileNameでファイル名が分かる
			TCHAR FileName[_MAX_PATH + 1];
			FILL_ZERO(FileName);
			_tcsncpy_s(FileName, Path, _MAX_PATH);
			PathAppend(FileName, lp.cFileName);

			SetFileAttributes(FileName, FILE_ATTRIBUTE_NORMAL);
			if( !DeleteFile(FileName) )bRet = false;
		}
	}
	if(bDeleteParent){
		//bRet=bRet&&UtilDeletePath(Path);
		if(!RemoveDirectory(Path))bRet=false;
	}

	return bRet;
}


//読み取り元ファイルを書き込み先ファイルのseekのある箇所から後にコピーして書き込む
//戻り値はエラーなしなら0,読み取りエラーは1,書き込みエラーは-1
int UtilAppendFile(HANDLE hWriteTo,HANDLE hReadFrom)
{
	//16KBずつコピー
	DWORD dwRead=0,dwWrite=0;
	std::vector<BYTE> Buffer(16*1024);
	for(;;){
		if(!ReadFile(hReadFrom,&Buffer.at(0),16*1024,&dwRead,NULL)){
			return 1;
		}
		if(0==dwRead){
			break;
		}
		if(!WriteFile(hWriteTo,&Buffer.at(0),dwRead,&dwWrite,NULL)){
			return -1;
		}
		if(dwRead!=dwWrite){
			return -1;
		}
	}
	return 0;
}

void UtilModifyPath(CString &strPath)
{
	// パスの修正
	strPath.Replace(_T("/"),_T("\\"));	//パス区切り文字

	int Ret=0;
	do{
		Ret=strPath.Replace(_T("\\\\"),_T("\\"));	//\\を\に置き換えていく
	}while(Ret!=0);
	strPath.Replace(_T("..\\"),_T("__\\"));	//相対パス指定..は__に置き換える
	strPath.Replace(_T(":"),_T("__"));	//ドライブ名も置き換える(C:からC__へ)
}

BOOL UtilMoveFileToRecycleBin(LPCTSTR lpFileName)
{
	ASSERT(lpFileName);
	if(!lpFileName)return false;
	const UINT len=_tcslen(lpFileName);
	std::vector<TCHAR> Buffer(len+2);
	_tcsncpy_s(&Buffer[0],len+1,lpFileName,len+1);
	Buffer.back()=_T('\0');

	SHFILEOPSTRUCT shfo={0};
	shfo.wFunc=FO_DELETE;	//削除
	shfo.pFrom=&Buffer[0];//ファイル名のリスト末尾は\0\0で終わる必要有り
	shfo.fFlags=FOF_SILENT/*進捗状況を表示しない*/|FOF_ALLOWUNDO/*UNDO情報付加;ごみ箱へ*/|FOF_NOCONFIRMATION/*確認ダイアログを表示しない*/;
	return SHFileOperation(&shfo);
}

BOOL UtilMoveFileToRecycleBin(const std::list<CString> &fileList)
{
	ASSERT(!fileList.empty());
	if(fileList.empty())return false;

	//引数作成
	CString Param;
	for(std::list<CString>::const_iterator ite=fileList.begin();ite!=fileList.end();++ite){
		Param+=*ite;
		Param+=_T('|');
	}
	Param+=_T('|');

	auto filter = UtilMakeFilterString(Param);

	SHFILEOPSTRUCT shfo={0};
	shfo.wFunc=FO_DELETE;	//削除
	shfo.pFrom=&filter[0];//ファイル名のリスト末尾は\0\0で終わる必要有り
	shfo.fFlags=FOF_SILENT/*進捗状況を表示しない*/|FOF_ALLOWUNDO/*UNDO情報付加;ごみ箱へ*/|FOF_NOCONFIRMATION/*確認ダイアログを表示しない*/;
	return SHFileOperation(&shfo);
}

//フォルダ内ファイル(ディレクトリは除く)を再帰検索
bool UtilRecursiveEnumFile(LPCTSTR lpszRoot,std::list<CString> &rFileList)
{
	CFindFile cFindFile;
	TCHAR szPath[_MAX_PATH+1];
	_tcsncpy_s(szPath,lpszRoot,_MAX_PATH);
	PathAppend(szPath,_T("*"));

	BOOL bContinue=cFindFile.FindFile(szPath);
	while(bContinue){
		if(!cFindFile.IsDots()){
			if(cFindFile.IsDirectory()){
				UtilRecursiveEnumFile(cFindFile.GetFilePath(),rFileList);
			}else{
				rFileList.push_back(cFindFile.GetFilePath());
			}
		}
		bContinue=cFindFile.FindNextFile();
	}

	return !rFileList.empty();
}

//フルパスかつ絶対パスの取得
PATHERROR UtilGetCompletePathName(CString &_FullPath,LPCTSTR lpszFileName)
{
	ASSERT(lpszFileName&&_tcslen(lpszFileName)>0);
	if(!lpszFileName||_tcslen(lpszFileName)<=0){
		TRACE(_T("ファイル名が指定されていない\n"));
		return PATHERROR_INVALID;
	}

	//---絶対パス取得
	TCHAR szAbsPath[_MAX_PATH+1]={0};
	{
		TCHAR Buffer[_MAX_PATH+1]={0};	//ルートかどうかのチェックを行う
		_tcsncpy_s(Buffer,lpszFileName,_MAX_PATH);
		PathAddBackslash(Buffer);
		if(PathIsRoot(Buffer)){
			//ドライブ名だけが指定されている場合、
			//_tfullpathはそのドライブのカレントディレクトリを取得してしまう
			_tcsncpy_s(szAbsPath,lpszFileName,_MAX_PATH);
		}
		else if(!_tfullpath(szAbsPath,lpszFileName,_MAX_PATH)){
			TRACE(_T("絶対パス取得失敗\n"));
			return PATHERROR_ABSPATH;
		}
	}

	if(!PathFileExists(szAbsPath)&&!PathIsDirectory(szAbsPath)){
		//パスがファイルもしくはディレクトリとして存在しないなら、エラーとする
		TRACE(_T("ファイルが存在しない\n"));
		return PATHERROR_NOTFOUND;
	}
	if(!GetLongPathName(szAbsPath,szAbsPath,_MAX_PATH)){
		TRACE(_T("ロングファイル名取得失敗\n"));
		return PATHERROR_LONGNAME;
	}
	_FullPath=szAbsPath;
	return PATHERROR_NONE;
}

//絶対パスの取得
bool UtilGetAbsPathName(CString &_FullPath,LPCTSTR lpszFileName)
{
	ASSERT(lpszFileName&&_tcslen(lpszFileName)>0);
	if(!lpszFileName||_tcslen(lpszFileName)<=0){
		TRACE(_T("ファイル名が指定されていない\n"));
		return false;
	}

	//---絶対パス取得
	TCHAR szAbsPath[_MAX_PATH+1]={0};
	{
		TCHAR Buffer[_MAX_PATH+1]={0};	//ルートかどうかのチェックを行う
		_tcsncpy_s(Buffer,lpszFileName,_MAX_PATH);
		PathAddBackslash(Buffer);
		if(PathIsRoot(Buffer)){
			//ドライブ名だけが指定されている場合、
			//_tfullpathはそのドライブのカレントディレクトリを取得してしまう
			_tcsncpy_s(szAbsPath,lpszFileName,_MAX_PATH);
		}
		else if(!_tfullpath(szAbsPath,lpszFileName,_MAX_PATH)){
			TRACE(_T("絶対パス取得失敗\n"));
			return false;
		}
	}

	_FullPath=szAbsPath;
	return true;
}

//ワイルドカードの展開
bool UtilPathExpandWild(std::list<CString> &r_outList,const std::list<CString> &r_inList)
{
	std::list<CString> tempList;
	std::list<CString>::const_iterator ite=r_inList.begin();
	const std::list<CString>::const_iterator end=r_inList.end();
	for(;ite!=end;++ite){
		if(-1==(*ite).FindOneOf(_T("*?"))){	//ワイルド展開可能な文字はない
			tempList.push_back(*ite);
		}else{
			//ワイルド展開
			CFindFile cFindFile;
			BOOL bContinue=cFindFile.FindFile(*ite);
			while(bContinue){
				if(!cFindFile.IsDots()){
					tempList.push_back(cFindFile.GetFilePath());
				}
				bContinue=cFindFile.FindNextFile();
			}
		}
	}
	r_outList=tempList;
	return true;
}


bool UtilPathExpandWild(std::list<CString> &r_outList,const CString &r_inParam)
{
	std::list<CString> tempList;
	if(-1==r_inParam.FindOneOf(_T("*?"))){	//ワイルド展開可能な文字はない
		tempList.push_back(r_inParam);
	}else{
		//ワイルド展開
		CFindFile cFindFile;
		BOOL bContinue=cFindFile.FindFile(r_inParam);
		while(bContinue){
			if(!cFindFile.IsDots()){
				tempList.push_back(cFindFile.GetFilePath());
			}
			bContinue=cFindFile.FindNextFile();
		}
	}
	r_outList=tempList;
	return true;
}

//パスのディレクトリ部分だけを取り出す
void UtilPathGetDirectoryPart(CString &str)
{
	LPTSTR lpszBuf=str.GetBuffer(_MAX_PATH+1);
	PathRemoveFileSpec(lpszBuf);
	PathAddBackslash(lpszBuf);
	str.ReleaseBuffer();
}

//自分のプログラムのファイル名を返す
LPCTSTR UtilGetModulePath()
{
	static TCHAR s_szExePath[_MAX_PATH+1]={0};
	if(s_szExePath[0]!=_T('\0'))return s_szExePath;

	GetModuleFileName(GetModuleHandle(NULL), s_szExePath, _MAX_PATH);	//本体のパス取得
	return s_szExePath;
}

//自分のプログラムのおいてあるディレクトリのパス名を返す
LPCTSTR UtilGetModuleDirectoryPath()
{
	static TCHAR s_szDirPath[_MAX_PATH+1]={0};
	if(s_szDirPath[0]!=_T('\0'))return s_szDirPath;

	GetModuleFileName(GetModuleHandle(NULL), s_szDirPath, _MAX_PATH);	//本体のパス取得
	PathRemoveFileSpec(s_szDirPath);
	return s_szDirPath;
}

//複数階層のディレクトリを一気に作成する
BOOL UtilMakeSureDirectoryPathExists(LPCTSTR _lpszPath)
{
#if defined(_UNICODE)||defined(UNICODE)
	CPath path(_lpszPath);
	path.RemoveFileSpec();
	path.AddBackslash();

	//TODO:UNICODE版のみでチェックを入れているのでANSIビルド時には適宜処理し直すべき
	CString tmp(path);
	if(-1!=tmp.Find(_T(" \\"))||-1!=tmp.Find(_T(".\\"))){	//パスとして処理できないファイル名がある
		ASSERT(!"Unacceptable Directory Name");
		return FALSE;
	}

	//UNICODE版のみで必要なチェック
	if(path.IsRoot())return TRUE;	//ドライブルートなら作成しない(できない)

	int nRet=SHCreateDirectoryEx(NULL,path,NULL);
	switch(nRet){
	case ERROR_SUCCESS:
		return TRUE;
	case ERROR_ALREADY_EXISTS:
		if(path.IsDirectory())return TRUE;	//すでにディレクトリがある場合だけセーフとする
		else return FALSE;
	default:
		return FALSE;
	}
#else//defined(_UNICODE)||defined(UNICODE)
  #pragma comment(lib, "Dbghelp.lib")
	return MakeSureDirectoryPathExists(_lpszPath);
#endif//defined(_UNICODE)||defined(UNICODE)
}

//TCHARファイル名をSJISファイル名に変換する。正しく変換できない場合には、falseを返す
bool UtilPathT2A(CStringA &strA,LPCTSTR lpPath,bool bOnDisk)
{
#if defined(_UNICODE)||defined(UNICODE)
	CStringA strTemp(lpPath);
	if(strTemp==lpPath){
		//欠損無く変換できた
		strA=strTemp;
		return true;
	}
	if(!bOnDisk){
		//ディスク上のファイルではないので、以降の代替手段は執れない
		return false;
	}
	//ショートファイル名で代用してみる
	WCHAR szPath[_MAX_PATH+1];
	GetShortPathNameW(lpPath,szPath,_MAX_PATH+1);

	//欠損無くSJISに変換できているかどうかチェックする
	return UtilPathT2A(strA,szPath,false);

#else//defined(_UNICODE)||defined(UNICODE)
	//SJISそのまま
	strA=lpPath;
	return true;
#endif//defined(_UNICODE)||defined(UNICODE)
}

//パスに共通する部分を取り出し、基底パスを取り出す
void UtilGetBaseDirectory(CString &BasePath,const std::list<CString> &PathList)
{
	bool bFirst=true;
	size_t ElementsCount=0;	//共通項目数
	std::vector<CString> PathElements;	//共通項目の配列

	std::list<CString>::const_iterator ite,end;
	end=PathList.end();
	for(ite=PathList.begin();ite!=end;++ite){
		if(!bFirst&&0==ElementsCount){
			//既に共通している要素が配列内に存在しないとき
			break;
		}

		//親ディレクトリまでで終わるパスを作る
		TCHAR Path[_MAX_PATH+1];
		FILL_ZERO(Path);
		_tcsncpy_s(Path,*ite,_MAX_PATH);
		PathRemoveFileSpec(Path);
		PathAddBackslash(Path);

		CString buffer;
		size_t iElement=0;	//一致しているパスの要素のカウント用(ループ内)
		size_t iIndex=0;	//パス内の文字のインデックス
		const size_t Length=_tcslen(Path);
		for(;iIndex<Length;iIndex++){
#if !defined(_UNICODE)&&!defined(UNICODE)
			if(_MBC_SINGLE==_mbsbtype((const unsigned char *)Path,iIndex)){
#endif
				if(_T('\\')==Path[iIndex]){
					//初回ならパスを詰め込み、そうでなければ要素を比較
					if(bFirst){
						PathElements.push_back(buffer);
						buffer.Empty();
						continue;
					}
					else{
						//大文字小文字区別せずに比較
						if(0==PathElements[iElement].CompareNoCase(buffer)){
							//要素が共通しているうちはOK
							iElement++;
							if(iElement>=ElementsCount)break;
							else{
								buffer.Empty();
								continue;
							}
						}
						else
						{
							//要素数を減らす
							//0オリジンのi番目で不一致ならばi個まで一致
							ElementsCount=iElement;
							break;
						}
					}
				}
#if !defined(_UNICODE)&&!defined(UNICODE)
			}
#endif
			buffer+=Path[iIndex];
		}
		if(bFirst){
			bFirst=false;
			ElementsCount=PathElements.size();
		}
		else if(ElementsCount>iElement){
			//パスが短かった場合、不一致なしのまま処理が終了する場合がある
			ElementsCount=iElement;
		}
	}

	BasePath.Empty();
	for(size_t i=0;i<ElementsCount;i++){
		BasePath+=PathElements[i];
		BasePath+=_T("\\");
	}
	TRACE(_T("BasePath=%s\n"),BasePath);
}

const LPCTSTR c_InvalidPathChar=_T("\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1a\x1b\x1c\x1d\x1e\x1f\"<>|:*?\\/\b\t");

//ファイル名に使えない文字列を置き換える
void UtilFixFileName(CString &rStr,LPCTSTR lpszOrg,TCHAR replace)
{
	rStr=lpszOrg;
	int length=_tcslen(c_InvalidPathChar);
	for(int i=0;i<length;i++){
		rStr.Replace(c_InvalidPathChar[i],replace);
	}
}


LPCTSTR UtilPathNextSeparator(LPCTSTR lpStr)
{
	for(;*lpStr!=_T('\0');lpStr++){
		if(*lpStr==_T('/') || *lpStr==_T('\\')){
			break;
		}
	}
	//終端
	return lpStr;
}

bool UtilPathNextSection(LPCTSTR lpStart,LPCTSTR& r_lpStart,LPCTSTR& r_lpEnd,bool bSkipMeaningless)
{
	LPCTSTR lpEnd=UtilPathNextSeparator(lpStart);
	if(bSkipMeaningless){
		while(true){
			//---無効なパスかどうかチェック
			//2文字以上のパスは有効であると見なす

			int length=lpEnd-lpStart;
			if(length==1){
				if(_T('.')==*lpStart || _T('\\')==*lpStart || _T('/')==*lpStart){
					//無効なパス
					//次の要素を取ってくる
					lpStart=lpEnd;
					lpEnd=UtilPathNextSeparator(lpStart);
				}else{
					break;
				}
			}else if(length==0){
				if(_T('\0')==*lpEnd){	//もうパスの要素がなくなったので返る
					return false;
				}else{
					lpEnd++;
					//次の要素を取ってくる
					lpStart=lpEnd;
					lpEnd=UtilPathNextSeparator(lpStart);
				}
			}else{
				break;
			}
		}
	}
	r_lpStart=lpStart;
	r_lpEnd=lpEnd;
	return true;
}

//Pathが'/'もしくは'\\'で終わっているならtrue
bool UtilPathEndWithSeparator(LPCTSTR lpPath)
{
	UINT length=_tcslen(lpPath);
	TCHAR c=lpPath[length-1];
	return (_T('/')==c || _T('\\')==c);
}

//パス名の最後の部分を取り出す
void UtilPathGetLastSection(CString &strSection,LPCTSTR lpPath)
{
	CString strPath=lpPath;
	strPath.Replace(_T('\\'),_T('/'));
	while(true){
		int idx=strPath.ReverseFind(_T('/'));
		if(-1==idx){	//そのまま
			strSection=lpPath;
			return;
		}else if(idx<strPath.GetLength()-1){
			//末尾がSeparatorではない
			strSection=lpPath+idx+1;
			return;
		}else{	//末尾がSeparator
			//末尾を削る->削った後はループで再度処理;strSectionにはSeparator付きの文字が格納される
			strPath.Delete(idx,strPath.GetLength());
		}
	}
}



//ファイルを丸ごと、もしくは指定されたところまで読み込み(-1で丸ごと)
bool UtilReadFile(LPCTSTR lpFile,std::vector<BYTE> &cReadBuffer,DWORD dwLimit)
{
	ASSERT(lpFile);
	if(!lpFile)return false;
	if(!PathFileExists(lpFile))return false;

	//Open
	HANDLE hFile=CreateFile(lpFile,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
	if(INVALID_HANDLE_VALUE==hFile)return false;

	//4GB越えファイルは扱わないのでファイルサイズ取得はこれでよい
	DWORD dwSize=GetFileSize(hFile,NULL);

	//読み込み範囲
	if(-1!=dwLimit){	//範囲制限されている
		dwSize=min(dwSize,dwLimit);
	}

	cReadBuffer.resize(dwSize);
	DWORD dwRead,dwIndex=0;
	//---読み込み
	while(true){
		const DWORD BLOCKSIZE=1024*10;	//10KBごとに読み込み
		DWORD readsize=BLOCKSIZE;
		if(dwIndex+readsize>dwSize){
			readsize=dwSize-dwIndex;
		}
		if(!ReadFile(hFile,&cReadBuffer[dwIndex],readsize,&dwRead,NULL)){
			CloseHandle(hFile);
			return false;
		}
		dwIndex+=dwRead;
		if(readsize<BLOCKSIZE)break;	//読み切った
	}
	CloseHandle(hFile);

	if(dwSize!=dwIndex){
		return false;
	}

	return true;
}

bool UtilReadFileSplitted(LPCTSTR lpFile,FILELINECONTAINER &container)
{
	//行バッファをクリア
	container.data.clear();
	container.lines.clear();

	//---読み込み
	std::vector<BYTE> cReadBuffer;
	if(!UtilReadFile(lpFile,cReadBuffer))return false;
	//終端の0追加
	cReadBuffer.resize(cReadBuffer.size()+2);
	cReadBuffer[cReadBuffer.size() - 1] = 0;
	cReadBuffer[cReadBuffer.size() - 2] = 0;

	{
		CStringW strData;
		UtilGuessToUNICODE(strData,&cReadBuffer[0],cReadBuffer.size());
		container.data.assign((LPCWSTR)strData,(LPCWSTR)strData+strData.GetLength());
		container.data.push_back(L'\0');
	}


	LPWSTR p=&container.data[0];
	const LPCWSTR end=p+container.data.size();
	LPCWSTR lastHead=p;

	//解釈
	for(;p!=end && *p!=L'\0';p++){
		if(*p==_T('\n')||*p==_T('\r')){
			if(lastHead<p){	//空行は飛ばす
				container.lines.push_back(lastHead);
			}
			lastHead=p+1;
			*p=L'\0';
		}
	}
	return true;
}

//https://support.microsoft.com/ja-jp/help/167296/how-to-convert-a-unix-time-t-to-a-win32-filetime-or-systemtime
void UtilUnixTimeToFileTime(time_t t, LPFILETIME pft)
{
	// Note that LONGLONG is a 64-bit value
	LONGLONG ll;

	ll = Int32x32To64(t, 10000000) + 116444736000000000;
	pft->dwLowDateTime = (DWORD)ll;
	pft->dwHighDateTime = ll >> 32;
}

