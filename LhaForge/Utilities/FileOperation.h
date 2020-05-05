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

#pragma once

std::wstring UtilGetTempPath();
std::wstring UtilGetTemporaryFileName();

bool UtilDeletePath(const wchar_t* PathName);
bool UtilDeleteDir(const wchar_t* Path, bool bDeleteParent);

bool UtilMoveFileToRecycleBin(const std::vector<std::wstring>& fileList);

//フォルダ内ファイル(ディレクトリは除く)を再帰検索
bool UtilRecursiveEnumFile(LPCTSTR lpszRoot,std::list<CString>&);

//フルパスかつ絶対パスの取得
enum PATHERROR{
	PATHERROR_NONE,		//成功
	PATHERROR_INVALID,	//パラメータ指定が不正
	PATHERROR_ABSPATH,	//絶対パスの取得に失敗
	PATHERROR_NOTFOUND,	//ファイルもしくはフォルダが見つからない
	PATHERROR_LONGNAME,	//ロングファイル名取得失敗
};
PATHERROR UtilGetCompletePathName(CString &_FullPath,LPCTSTR lpszFileName);
//絶対パスの取得
bool UtilGetAbsPathName(CString &_FullPath,LPCTSTR lpszFileName);

//ワイルドカードの展開
bool UtilPathExpandWild(std::list<CString> &r_outList,const std::list<CString> &r_inList);
bool UtilPathExpandWild(std::list<CString> &r_outList,const CString &r_inParam);

//パスのディレクトリ部分だけを取り出す
void UtilPathGetDirectoryPart(CString&);

//自分のプログラムのファイル名を返す
LPCTSTR UtilGetModulePath();

//自分のプログラムのおいてあるディレクトリのパス名を返す
LPCTSTR UtilGetModuleDirectoryPath();

//複数階層のディレクトリを一気に作成する
[[deprecated("use std::filesystem::create_directories instead")]]
BOOL UtilMakeSureDirectoryPathExists(LPCTSTR lpszPath);

//TCHARファイル名をSJISファイル名に変換する。正しく変換できない場合には、falseを返す
bool UtilPathT2A(CStringA&,LPCTSTR,bool bOnDisk);

//パスに共通する部分を取り出し、基底パスを取り出す
void UtilGetBaseDirectory(CString &BasePath,const std::list<CString> &PathList);

//ファイル名に使えない文字列を置き換える
void UtilFixFileName(CString &,LPCTSTR lpszOrg,TCHAR replace);

LPCTSTR UtilPathNextSeparator(LPCTSTR lpStr);
bool UtilPathNextSection(LPCTSTR lpStart,LPCTSTR& r_lpStart,LPCTSTR& r_lpEnd,bool bSkipMeaningless);
//Pathが'/'もしくは'\\'で終わっているならtrue
bool UtilPathEndWithSeparator(LPCTSTR lpPath);
void UtilPathGetLastSection(CString &strSection,LPCTSTR lpPath);

//ファイルを丸ごと、もしくは指定されたところまで読み込み(-1で丸ごと)
bool UtilReadFile(LPCTSTR lpFile,std::vector<BYTE> &cReadBuffer,DWORD dwLimit=-1);

struct FILELINECONTAINER{
	virtual ~FILELINECONTAINER(){}
	std::vector<WCHAR> data;
	std::vector<LPCWSTR> lines;
};
bool UtilReadFileSplitted(LPCTSTR lpFile,FILELINECONTAINER&);

//https://support.microsoft.com/ja-jp/help/167296/how-to-convert-a-unix-time-t-to-a-win32-filetime-or-systemtime
void UtilUnixTimeToFileTime(time_t t, LPFILETIME pft);


struct LF_BUFFER_INFO {
	//contains buffer and its size in libarchive's internal memory
	size_t size;	//0 if it reaches EOF
	int64_t offset;
	const void* buffer;
	bool is_eof()const { return NULL == buffer; }
	void make_eof() {
		size = 0;
		offset = 0;
		buffer = NULL;
	}
};

struct FILE_READER {
	FILE* fp;
	LF_BUFFER_INFO ibi;
	std::vector<unsigned char> buffer;
	FILE_READER() : fp(NULL) {
		ibi.make_eof();
		buffer.resize(1024 * 1024);
	}
	virtual ~FILE_READER() {
		close();
	}
	const LF_BUFFER_INFO& operator()() {
		if (!fp || feof(fp)) {
			ibi.make_eof();
		} else {
			ibi.size = fread(&buffer[0], 1, buffer.size(), fp);
			ibi.buffer = &buffer[0];
			ibi.offset = _ftelli64(fp);
		}
		return ibi;
	}
	void open(const wchar_t* path) {
		close();
		_wfopen_s(&fp, path, L"rb");
	}
	void close() {
		if (fp)fclose(fp);
		fp = NULL;
	}
};

class CAutoFile {
protected:
	FILE *_fp;
	CAutoFile(const CAutoFile&) = delete;
	const CAutoFile& operator=(const CAutoFile&) = delete;
public:
	CAutoFile() :_fp(NULL){}
	virtual ~CAutoFile() {
		close();
	}
	operator FILE*() { return _fp; }
	bool is_opened() const { return _fp != NULL; }
	void close() {
		if (_fp) {
			fclose(_fp);
			_fp = NULL;
		}
	}
	void open(const wchar_t* fname, const wchar_t* mode = L"r") {
		close();
		_wfopen_s(&_fp, fname, mode);
	}
};


