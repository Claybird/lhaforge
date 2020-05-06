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

std::wstring UtilGetTempPath()
{
	auto tempDir = std::filesystem::temp_directory_path();
	return UtilPathAddLastSeparator(tempDir.c_str());
}

std::wstring UtilGetTemporaryFileName()
{
	wchar_t buf[MAX_PATH] = {};
	GetTempFileNameW(UtilGetTempPath().c_str(), L"lhf", 0, buf);
	return buf;
}

bool UtilDeletePath(const wchar_t* PathName)
{
	if( PathIsDirectoryW(PathName) ) {
		//directory
		if( UtilDeleteDir(PathName, true) )return true;
	} else if( PathFileExistsW(PathName) ) {
		//file
		//reset file attribute
		SetFileAttributesW(PathName, FILE_ATTRIBUTE_NORMAL);
		if( DeleteFileW(PathName) )return true;
	}
	return false;
}

//bDeleteParent=true: delete Path itself
//bDeleteParent=false: delete only children of Path
bool UtilDeleteDir(const wchar_t* Path,bool bDeleteParent)
{
	std::wstring FindParam = std::filesystem::path(Path) / L"*";

	bool bRet = true;

	CFindFile cFindFile;
	BOOL bContinue = cFindFile.FindFile(FindParam.c_str());
	while (bContinue) {
		if (!cFindFile.IsDots()) {
			if (cFindFile.IsDirectory()) {
				//directory
				if (!UtilDeleteDir(cFindFile.GetFilePath(), true)) {
					bRet = false;
				}
			} else {
				//reset file attribute
				SetFileAttributesW(cFindFile.GetFilePath(), FILE_ATTRIBUTE_NORMAL);
				if (!DeleteFileW(cFindFile.GetFilePath()))bRet = false;
			}
		}
		bContinue = cFindFile.FindNextFile();
	}

	if(bDeleteParent){
		if(!RemoveDirectoryW(Path))bRet=false;
	}

	return bRet;
}

bool UtilMoveFileToRecycleBin(const std::vector<std::wstring>& fileList)
{
	ASSERT(!fileList.empty());
	if(fileList.empty())return false;

	std::wstring param;
	for(const auto& item: fileList){
		param += item;
		param += L'|';
	}
	param += L'|';

	auto filter = UtilMakeFilterString(param.c_str());

	SHFILEOPSTRUCTW shfo={0};
	shfo.wFunc = FO_DELETE;
	shfo.pFrom = &filter[0];
	shfo.fFlags =
		FOF_SILENT |	//do not show progress
		FOF_ALLOWUNDO |	//allow undo i.e., to recycle bin
		FOF_NOCONFIRMATION;	//no confirm window
	return 0 == SHFileOperationW(&shfo);
}

//recursively enumerates files (no directories) in specified directory
std::vector<std::wstring> UtilRecursiveEnumFile(const wchar_t* lpszRoot)
{
	CFindFile cFindFile;

	std::vector<std::wstring> files;
	BOOL bContinue = cFindFile.FindFile((std::filesystem::path(lpszRoot) / L"*").c_str());
	while(bContinue){
		if(!cFindFile.IsDots()){
			if(cFindFile.IsDirectory()){
				auto subFiles = UtilRecursiveEnumFile(cFindFile.GetFilePath());
				files.insert(files.end(), subFiles.begin(), subFiles.end());
			}else{
				files.push_back((const wchar_t*)cFindFile.GetFilePath());
			}
		}
		bContinue=cFindFile.FindNextFile();
	}

	return files;
}

bool UtilPathIsRoot(const wchar_t* path)
{
	auto p = std::filesystem::path(path);
	p.make_preferred();
	if (p == p.root_path() || p == p.root_name())return true;
	return false;
}

std::wstring UtilPathAddLastSeparator(const wchar_t* path)
{
	std::wstring p = path;
	if (p.empty() || (p.back() != L'/' && p.back() != L'\\')) {
		p += std::filesystem::path::preferred_separator;
	}
	return p;
}

//get full & absolute path
std::wstring UtilGetCompletePathName(const wchar_t* lpszFileName)
{
	if(!lpszFileName||wcslen(lpszFileName)<=0){
		RAISE_EXCEPTION(L"empty pathname");
	}

	std::filesystem::path abs_path = lpszFileName;
	if(!UtilPathIsRoot(abs_path.c_str())){
		//when only drive letter is given, _wfullpath returns current directory on that drive
		wchar_t* buf = _wfullpath(nullptr, lpszFileName, 0);
		if (!buf) {
			RAISE_EXCEPTION(L"failed to get fullpath");
		}
		abs_path = buf;
		free(buf);
	}

	if (std::filesystem::exists(abs_path)) {
		DWORD bufSize = GetLongPathNameW(abs_path.c_str(), nullptr, 0);
		std::wstring buf;
		buf.resize(bufSize);
		if (!GetLongPathNameW(abs_path.c_str(), &buf[0], bufSize)) {
			RAISE_EXCEPTION(L"failed to get long filename");
		}
		abs_path = buf.c_str();
	}
	return abs_path.make_preferred().wstring();
}

//returns filenames that matches to the given pattern
std::vector<std::wstring> UtilPathExpandWild(const wchar_t* pattern)
{
	std::vector<std::wstring> out;
	std::wstring p = pattern;
	if(std::wstring::npos == p.find_first_of(L"*?")){	//no characters to expand
		out.push_back(pattern);
	}else{
		//expand wild
		CFindFile cFindFile;
		BOOL bContinue=cFindFile.FindFile(pattern);
		while(bContinue){
			if(!cFindFile.IsDots()){
				out.push_back((const wchar_t*)cFindFile.GetFilePath());
			}
			bContinue=cFindFile.FindNextFile();
		}
	}
	return out;
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
		auto cp = UtilGuessCodepage((const char*)&cReadBuffer[0], cReadBuffer.size());
		CStringW strData = UtilToUNICODE((const char*)&cReadBuffer[0], cReadBuffer.size(), cp).c_str();
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

