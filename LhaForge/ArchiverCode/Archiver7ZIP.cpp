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

CArchiver7ZIP::CArchiver7ZIP()
{
}

CArchiver7ZIP::~CArchiver7ZIP()
{
	FreeDLL();
}

LOAD_RESULT CArchiver7ZIP::LoadDLL(CConfigManager &ConfMan,CString &strErr)
{
	return LOAD_RESULT_OK;
}

void CArchiver7ZIP::FreeDLL()
{
}

/*
formatの指定は、B2E32.dllでのみ有効
levelの指定は、B2E32.dll以外で有効
*/
bool CArchiver7ZIP::Compress(LPCTSTR ArcFileName,std::list<CString> &ParamList,CConfigManager &ConfMan,const PARAMETER_TYPE Type,int Options,LPCTSTR lpszFormat,LPCTSTR lpszMethod,LPCTSTR lpszLevel,CString &strLog)
{
	return false;
}

bool CArchiver7ZIP::Extract(LPCTSTR ArcFileName,CConfigManager&,const CConfigExtract& Config,bool bSafeArchive,LPCTSTR OutputDir,CString &strLog)
{
	return false;
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

bool CArchiver7ZIP::ExtractSpecifiedOnly(LPCTSTR ArcFileName, CConfigManager&, LPCTSTR OutputDir, std::list<CString> &FileList, CString &strLog, bool bUsePath)
{
	//出力先移動
	CCurrentDirManager currentDir(OutputDir);
	return false;
}


bool CArchiver7ZIP::DeleteItemFromArchive(LPCTSTR ArcFileName,CConfigManager&,const std::list<CString>& FileList,CString &strLog)
{
	return false;
}

bool CArchiver7ZIP::AddItemToArchive(LPCTSTR ArcFileName, bool bEncrypted, const std::list<CString> &FileList, CConfigManager &ConfMan, LPCTSTR lpDestDir, CString &strLog)
{
	return false;
}


BOOL CArchiver7ZIP::CheckArchive(LPCTSTR _szFileName)
{
	return true;
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

