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
#include "compress.h"
#include "Dialogs/LogListDialog.h"

#include "resource.h"

#include "Utilities/Semaphore.h"
#include "Utilities/StringUtil.h"
#include "Utilities/FileOperation.h"
#include "Utilities/OSUtil.h"
#include "ConfigCode/ConfigManager.h"
#include "ConfigCode/ConfigCompress.h"
#include "ConfigCode/ConfigGeneral.h"
#include "CommonUtil.h"
#include "CmdLineInfo.h"

bool DeleteOriginalFiles(const CConfigCompress &ConfCompress,const std::list<CString>& fileList);

HRESULT CheckArchiveName(LPCTSTR lpszArcFile, const std::list<CString> &rOrgFileList, bool bOverwrite, CString &strErr)
{
	// ファイル名が長すぎたとき
	if (_tcslen(lpszArcFile) >= _MAX_PATH) {
		strErr = CString(MAKEINTRESOURCE(IDS_ERROR_MAX_PATH));
		return E_LF_CANNOT_DECIDE_FILENAME;
	}

	// できあがったファイル名が入力元のファイル名と同じか
	std::list<CString>::const_iterator ite, end;
	end = rOrgFileList.end();
	for (ite = rOrgFileList.begin(); ite != end; ++ite) {
		if (0 == (*ite).CompareNoCase(lpszArcFile)) {
			strErr.Format(IDS_ERROR_SAME_INPUT_AND_OUTPUT, lpszArcFile);
			return E_LF_OVERWRITE_SOURCE;
		}
	}

	// ファイルが既に存在するかどうか
	if (!bOverwrite) {
		if (PathFileExists(lpszArcFile)) {
			// ファイルが存在したら問題あり
			// この場合はエラーメッセージを出さずにファイル名を入力させる
			return S_LF_ARCHIVE_EXISTS;
		}
	}

	return S_OK;
}


//上書き確認など
HRESULT ConfirmOutputFile(CPath &r_pathArcFileName, const std::list<CString> &rOrgFileNameList, LPCTSTR lpExt, BOOL bAskName)
{
	//----------------------------
	// ファイル名指定が適切か確認
	//----------------------------
	bool bForceOverwrite = false;
	BOOL bInputFilenameFirst = bAskName;//Config.Common.Compress.SpecifyOutputFilename;

	HRESULT hStatus = E_UNEXPECTED;
	while (S_OK != hStatus) {
		if (!bInputFilenameFirst) {
			//ファイル名の長さなど確認
			CString strLocalErr;
			hStatus = CheckArchiveName(r_pathArcFileName, rOrgFileNameList, bForceOverwrite, strLocalErr);
			if (FAILED(hStatus)) {
				ErrorMessage(strLocalErr);
			}
			if (S_OK == hStatus) {
				break;	//完了
			}
		}

		//最初のファイル名は仮生成したものを使う
		//選択されたファイル名が返ってくる
		CPath pathFile;
		if (E_LF_CANNOT_DECIDE_FILENAME == hStatus) {
			//ファイル名が決定できない場合、カレントディレクトリを現行パスとする
			TCHAR szBuffer[_MAX_PATH + 1] = { 0 };
			GetCurrentDirectory(_MAX_PATH, szBuffer);
			pathFile = szBuffer;
			pathFile.Append(CString(MAKEINTRESOURCE(IDS_UNTITLED)) + lpExt);
		} else {
			pathFile = r_pathArcFileName;
		}

		//==================
		// 名前を付けて保存
		//==================
		//フィルタ文字列生成
		CString strFilter(MAKEINTRESOURCE(IDS_COMPRESS_FILE));
		strFilter.AppendFormat(_T(" (*%s)|*%s"), lpExt, lpExt);
		auto filter = UtilMakeFilterString(strFilter);

		CFileDialog dlg(FALSE, lpExt, pathFile, OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR, filter.c_str());
		if (IDCANCEL == dlg.DoModal()) {	//キャンセル
			return E_ABORT;
		}

		r_pathArcFileName = dlg.m_szFileName;
		//DELETED:ファイル名のロングパスを取得

		bForceOverwrite = true;
		bInputFilenameFirst = false;
	}
	return S_OK;
}


bool isAllowedCombination(LF_ARCHIVE_FORMAT fmt, int option)
{
	const auto &cap = get_archive_capability(fmt);
	for (auto allowed : cap.allowed_combinations) {
		if (allowed == option) {
			return true;
		}
	}
	return false;
}

//returns extension with first "."
std::wstring getArchiveFileExtension(LF_ARCHIVE_FORMAT fmt, LPCTSTR lpszOrgExt, int option)
{
	const auto &cap = get_archive_capability(fmt);
	for (auto allowed : cap.allowed_combinations) {
		if (allowed == option) {
			if (cap.multi_file_archive) {
				if (option & LF_WOPT_SFX) {
					return L".exe";
				} else {
					return cap.extension;
				}
			} else {
				if (option & LF_WOPT_SFX) {
					return std::wstring(lpszOrgExt) + L".exe";
				} else {
					return std::wstring(lpszOrgExt) + L"." + cap.extension;
				}
			}
		}
	}
	RAISE_EXCEPTION(L"unexpected format");
}

/*************************************************************
アーカイブファイル名を決定する。

 基本的には、コマンドライン引数で与えられるファイル名のうち、
最初のファイル名を元にして、拡張子だけを変えたファイルを作る。
*************************************************************/
HRESULT GetArchiveName(
	CPath &r_pathArcFileName,
	const std::list<CString> &OrgFileNameList,
	LF_ARCHIVE_FORMAT format,
	int option,
	const CConfigCompress &ConfCompress,
	const CConfigGeneral &ConfGeneral,
	CMDLINEINFO &rCmdLineInfo,
	CString &strErr)
{
	ASSERT(!OrgFileNameList.empty());

	//==================
	// ファイル名の生成
	//==================
	BOOL bDirectory = FALSE;	//圧縮対象に指定された物がフォルダならtrue;フォルダとファイルで末尾の'.'の扱いを変える
	BOOL bDriveRoot = FALSE;	//圧縮対象がドライブのルート(X:\ etc.)ならtrue
	CPath pathOrgFileName;


	bool bFileNameSpecified = (0 < rCmdLineInfo.OutputFileName.GetLength());

	//---元のファイル名
	if (bFileNameSpecified) {
		//TODO:ここで処理するのはおかしい
		pathOrgFileName = (LPCTSTR)rCmdLineInfo.OutputFileName;
		pathOrgFileName.StripPath();	//パス名を取り除く
	} else {
		//先頭のファイル名を元に暫定的にファイル名を作り出す
		pathOrgFileName = (LPCTSTR)*OrgFileNameList.begin();

		//---判定
		//ディレクトリかどうか
		bDirectory = pathOrgFileName.IsDirectory();
		//ドライブのルートかどうか
		CPath pathRoot = pathOrgFileName;
		pathRoot.AddBackslash();
		bDriveRoot = pathRoot.IsRoot();
		if (bDriveRoot) {
			//ボリューム名取得
			TCHAR szVolume[MAX_PATH];
			GetVolumeInformation(pathRoot, szVolume, MAX_PATH, NULL, NULL, NULL, NULL, 0);
			//ドライブを丸ごと圧縮する場合には、ボリューム名をファイル名とする
			UtilFixFileName(pathOrgFileName, szVolume, _T('_'));
		}
	}

	CString strExt;
	if (isAllowedCombination(format, option)) {
		//拡張子決定
		if (bFileNameSpecified) {
			strExt = PathFindExtension(pathOrgFileName);
		} else {
			strExt = getArchiveFileExtension(format, pathOrgFileName, option).c_str();
		}
	} else {
		strErr = CString(MAKEINTRESOURCE(IDS_ERROR_ILLEGAL_FORMAT_TYPE));
		return E_INVALIDARG;
	}


	//出力先フォルダ名
	CPath pathOutputDir;
	if (0 != rCmdLineInfo.OutputDir.GetLength()) {
		//出力先フォルダが指定されている場合
		pathOutputDir = (LPCTSTR)rCmdLineInfo.OutputDir;
	} else {
		//設定を元に出力先を決める
		bool bUseForAll;
		HRESULT hr = GetOutputDirPathFromConfig(ConfCompress.OutputDirType, pathOrgFileName, ConfCompress.OutputDir, pathOutputDir, bUseForAll, strErr);
		if (FAILED(hr)) {
			return hr;
		}
		if (bUseForAll) {
			rCmdLineInfo.OutputDir = (LPCTSTR)pathOutputDir;
		}
	}
	pathOutputDir.AddBackslash();

	// 出力先がネットワークドライブ/リムーバブルディスクであるなら警告
	// 出力先が存在しないなら、作成確認
	HRESULT hRes = ConfirmOutputDir(ConfGeneral, pathOutputDir, strErr);
	if (FAILED(hRes)) {
		//キャンセルなど
		return hRes;
	}

	//ファイル名から拡張子とパス名を削除
	if (bDirectory || bDriveRoot) {	//ディレクトリ
		//ディレクトリからは拡張子(最後の'.'以降の文字列)を削除しない
		CPath pathDir = pathOrgFileName;
		pathDir.StripPath();

		r_pathArcFileName = pathOutputDir;
		r_pathArcFileName.Append(pathDir);
		r_pathArcFileName = (LPCTSTR)(((CString)r_pathArcFileName) + strExt);
	} else {	//ファイル
		CPath pathFile = pathOrgFileName;
		pathFile.StripPath();
		r_pathArcFileName = pathOutputDir;
		r_pathArcFileName.Append(pathFile);
		r_pathArcFileName = (LPCTSTR)(((CString)r_pathArcFileName) + strExt);
	}

	//ドライブルートを圧縮する場合、ファイル名は即決しない
	return ConfirmOutputFile(r_pathArcFileName, OrgFileNameList, strExt, ConfCompress.SpecifyOutputFilename || bDriveRoot);
}

//pathFileNameは最初のファイル名
//深さ優先探索
//圧縮対象のファイルを探す;戻り値は
//0:ファイルは含まれていない
//1:単一ファイルのみが含まれている
//2:複数ファイルが含まれている
int CheckIfMultipleFilesToCompress(LPCTSTR lpszPath, CPath &pathFileName)
{
	if (!PathIsDirectory(lpszPath)) {	//パスがファイルの場合
		pathFileName = lpszPath;
		return 1;
	} else {	//パスがフォルダの場合、フォルダ内部のファイルを探す
		int nFileCount = 0;

		CPath pathWild = lpszPath;
		pathWild.Append(_T("\\*"));
		CFindFile cFind;

		BOOL bFound = cFind.FindFile(pathWild);
		for (; bFound; bFound = cFind.FindNextFile()) {
			if (cFind.IsDots())continue;

			if (cFind.IsDirectory()) {	//サブディレクトリ検索
				nFileCount += CheckIfMultipleFilesToCompress(cFind.GetFilePath(), pathFileName);
				if (nFileCount >= 2) {
					return 2;
				}
			} else {
				pathFileName = (LPCTSTR)cFind.GetFilePath();
				nFileCount++;
				if (nFileCount >= 2) {
					return 2;
				}
			}
		}
		return min(2, nFileCount);
	}
}


bool Compress(const std::list<CString> &_sourcePathList,LF_ARCHIVE_FORMAT format, LF_WRITE_OPTIONS options, CConfigManager &ConfigManager,CMDLINEINFO& CmdLineInfo)
{
	CConfigCompress ConfCompress;
	ConfCompress.load(ConfigManager);
	CConfigGeneral ConfGeneral;
	ConfGeneral.load(ConfigManager);

	int Options=CmdLineInfo.Options;
	//---設定上書き
	//出力先上書き
	if(-1!=CmdLineInfo.OutputToOverride){
		ConfCompress.OutputDirType=CmdLineInfo.OutputToOverride;
	}
	if(-1!=CmdLineInfo.IgnoreTopDirOverride){
		ConfCompress.IgnoreTopDirectory=CmdLineInfo.IgnoreTopDirOverride;
	}
	if(-1!=CmdLineInfo.DeleteAfterProcess){
		ConfCompress.DeleteAfterCompress=CmdLineInfo.DeleteAfterProcess;
	}

	//ディレクトリ内にファイルが全て入っているかの判定
	auto sourcePathList = _sourcePathList;
	if(ConfCompress.IgnoreTopDirectory){
		//NOTE: this code equals to ParamList.size()==1
		auto ite= sourcePathList.begin();
		ite++;
		if(ite == sourcePathList.end()){
			//ファイルが一つしかない
			CPath path = sourcePathList.front();
			path.AddBackslash();
			if(path.IsDirectory()){
				//単体のディレクトリを圧縮
				UtilPathExpandWild(sourcePathList, path+L"*");
			}
			if(sourcePathList.empty()){
				sourcePathList = sourcePathList;
			}
		}
	}

	const auto& cap = get_archive_capability(format);
	if(!cap.multi_file_archive){
		int nFileCount=0;
		CPath pathFileName;
		for(const auto &path: sourcePathList){
			nFileCount += CheckIfMultipleFilesToCompress(path, pathFileName);
			if(nFileCount>=2){
				ErrorMessage(CString(MAKEINTRESOURCE(IDS_ERROR_SINGLE_FILE_ONLY)));
				return false;
			}
		}
		if(0==nFileCount){
			ErrorMessage(CString(MAKEINTRESOURCE(IDS_ERROR_FILE_NOT_SPECIFIED)));
			return false;
		}
		sourcePathList.clear();
		sourcePathList.push_back(pathFileName);
	}

	//=========================================================
	// get common path for base path
	//=========================================================
	CString pathBase;
	UtilGetBaseDirectory(pathBase, sourcePathList);
	TRACE((LPCTSTR)pathBase),TRACE(_T("\n"));

	//圧縮先ファイル名決定
	TRACE(_T("圧縮先ファイル名決定\n"));
	CPath pathArcFileName;
	CString strErr;
	HRESULT hr=GetArchiveName(pathArcFileName, sourcePathList, format, Options,ConfCompress,ConfGeneral,CmdLineInfo,strErr);
	if(FAILED(hr)){
		if(hr!=E_ABORT){
			ErrorMessage(strErr);
		}
		return false;
	}

	{
		if(PathFileExists(pathArcFileName)){
			if(!DeleteFile(pathArcFileName)){
				//削除できなかった
				CString strLastError;
				UtilGetLastErrorMessage(strLastError);
				CString msg;
				msg.Format(IDS_ERROR_FILE_REPLACE,(LPCTSTR)strLastError);

				ErrorMessage(msg);
				return false;
			}
		}

		//======================================
		// 圧縮対象ファイル名を修正する
		//======================================
		const int nBasePathLength=pathBase.GetLength();
		for(std::list<CString>::iterator ite=sourcePathList.begin();ite!= sourcePathList.end();++ite){
			//ベースパスを元に相対パス取得 : 共通である基底パスの文字数分だけカットする
			//(*ite)=(*ite).Right((*ite).GetLength()-BasePathLength);
			//(*ite).Delete(0,nBasePathLength);
			(*ite)=(LPCTSTR)(*ite)+nBasePathLength;

			//もしBasePathと同じになって空になってしまったパスがあれば、カレントディレクトリ指定を代わりに入れておく
			if((*ite).IsEmpty()){
				*ite=L".";
			}
		}
	}

	//セマフォによる排他処理
	CSemaphoreLocker SemaphoreLock;
	if(ConfCompress.LimitCompressFileCount){
		SemaphoreLock.Lock(LHAFORGE_COMPRESS_SEMAPHORE_NAME,ConfCompress.MaxCompressFileCount);
	}

	TRACE(_T("ArchiverHandler 呼び出し\n"));
	//------------
	// 圧縮を行う
	//------------
	CString strLog;
	bool bError = false;
	try {
		ARCHIVE_FILE_TO_WRITE archive;
		_wchdir(pathBase);
		archive.write_open(pathArcFileName, format, options);
		for (const auto &fname : sourcePathList) {
			strLog += Format(L"Compress %s\n", (LPCWSTR)fname).c_str();
			LF_ARCHIVE_ENTRY entry;
			entry.copy_file_stat(fname);
			FILE_READER provider;
			provider.open(fname);
			archive.add_entry(entry, provider);
		}
	} catch (const ARCHIVE_EXCEPTION& e) {
		strLog += e.what();
		bError = true;
	}
	//---ログ表示
	switch(ConfGeneral.LogViewEvent){
	case LOGVIEW_ON_ERROR:
		if(bError){
			//TODO
			CLogListDialog LogDlg(L"Log");
			std::vector<ARCLOG> logs;
			logs.resize(1);
			logs.back().logs.resize(1);
			logs.back().logs.back().entryPath = pathArcFileName;
			logs.back().logs.back().message = strLog;
			LogDlg.SetLogArray(logs);
			LogDlg.DoModal();
		}
		break;
	case LOGVIEW_ALWAYS:
	{
		//TODO
		CLogListDialog LogDlg(L"Log");
		std::vector<ARCLOG> logs;
		logs.resize(1);
		logs.back().logs.resize(1);
		logs.back().logs.back().entryPath = pathArcFileName;
		logs.back().logs.back().message = strLog;
		LogDlg.SetLogArray(logs);
		LogDlg.DoModal();
	}
		break;
	}

	if(ConfGeneral.NotifyShellAfterProcess){
		//圧縮完了を通知
		::SHChangeNotify(SHCNE_CREATE,SHCNF_PATH,pathArcFileName,NULL);
	}

	//出力先フォルダを開く
	if(!bError && ConfCompress.OpenDir){
		CPath pathOpenDir=pathArcFileName;
		pathOpenDir.RemoveFileSpec();
		pathOpenDir.AddBackslash();
		if(ConfGeneral.Filer.UseFiler){
			//パラメータ展開に必要な情報
			std::map<stdString,CString> _envInfo;
			MakeExpandInformationEx(_envInfo,pathOpenDir,pathArcFileName);
			std::map<std::wstring, std::wstring> envInfo;
			for (auto& item : _envInfo) {
				envInfo[item.first] = item.second;
			}

			//コマンド・パラメータ展開
			auto strCmd = UtilExpandTemplateString(ConfGeneral.Filer.FilerPath,envInfo);	//コマンド
			auto strParam = UtilExpandTemplateString(ConfGeneral.Filer.Param,envInfo);	//パラメータ
			ShellExecute(NULL, _T("open"), strCmd.c_str(), strParam.c_str(), NULL, SW_SHOWNORMAL);
		}else{
			//Explorerで開く
			UtilNavigateDirectory(pathOpenDir);
		}
	}

	//正常に圧縮できたファイルを削除orごみ箱に移動
	if(!bError && ConfCompress.DeleteAfterCompress){
		//カレントディレクトリは削除できないので別の場所へ移動
		::SetCurrentDirectory(UtilGetModuleDirectoryPath());
		//削除:オリジナルの指定で消す
		DeleteOriginalFiles(ConfCompress,_sourcePathList);
	}


	TRACE(_T("Exit Compress()\n"));
	return bError ? E_FAIL : S_OK;
}



bool DeleteOriginalFiles(const CConfigCompress &ConfCompress,const std::list<CString>& fileList)
{
	CString strFiles;	//ファイル一覧
	size_t idx=0;
	for(std::list<CString>::const_iterator ite=fileList.begin();ite!=fileList.end();++ite){
		strFiles+=_T("\n");
		strFiles+=*ite;
		idx++;
		if(idx>=10 && idx<fileList.size()){
			strFiles+=_T("\n");
			strFiles.AppendFormat(IDS_NUM_EXTRA_FILES,fileList.size()-idx);
			break;
		}
	}

	//削除する
	if(ConfCompress.MoveToRecycleBin){
		//--------------
		// ごみ箱に移動
		//--------------
		if(!ConfCompress.DeleteNoConfirm){	//削除確認する場合
			CString Message;
			Message.Format(IDS_ASK_MOVE_ORIGINALFILE_TO_RECYCLE_BIN);
			Message+=strFiles;

			//確認後ゴミ箱に移動
			if(IDYES!=MessageBox(NULL,Message,UtilGetMessageCaption(),MB_YESNO|MB_ICONQUESTION|MB_DEFBUTTON2)){
				return false;
			}
		}

		//削除実行
		UtilMoveFileToRecycleBin(fileList);
		return true;
	}else{
		//------
		// 削除
		//------
		if(!ConfCompress.DeleteNoConfirm){	//確認する場合
			CString Message;
			Message.Format(IDS_ASK_DELETE_ORIGINALFILE);
			Message+=strFiles;

			if(IDYES!=MessageBox(NULL,Message,UtilGetMessageCaption(),MB_YESNO|MB_ICONQUESTION|MB_DEFBUTTON2)){
				return false;
			}
		}
		//削除実行
		for(std::list<CString>::const_iterator ite=fileList.begin();ite!=fileList.end();++ite){
			UtilDeletePath(*ite);
		}
		return true;
	}
}


const std::vector<COMPRESS_COMMANDLINE_PARAMETER> g_CompressionCmdParams = {
	{L"/c:zip",		LF_FMT_ZIP,		LF_WOPT_STANDARD					,IDS_FORMAT_NAME_ZIP},
	{L"/c:zippass",	LF_FMT_ZIP,		LF_WOPT_DATA_ENCRYPTION	,IDS_FORMAT_NAME_ZIP_PASS},
//	{L"/c:zipsfx",	LF_FMT_ZIP,		LF_WOPT_SFX		,IDS_FORMAT_NAME_ZIP_SFX},
//	{L"/c:zippasssfx",LF_FMT_ZIP,		LF_WOPT_DATA_ENCRYPTION | COMPRESS_SFX,IDS_FORMAT_NAME_ZIP_PASS_SFX},
//	{L"/c:zipsplit",	LF_FMT_ZIP,		COMPRESS_SPLIT		,IDS_FORMAT_NAME_ZIP_SPLIT},
//	{L"/c:zippasssplit",LF_FMT_ZIP,		LF_WOPT_DATA_ENCRYPTION | COMPRESS_SPLIT,IDS_FORMAT_NAME_ZIP_PASS_SPLIT},
	{L"/c:7z",		LF_FMT_7Z,		LF_WOPT_STANDARD					,IDS_FORMAT_NAME_7Z},
//	{L"/c:7zpass",	LF_FMT_7Z,		LF_WOPT_DATA_ENCRYPTION	,IDS_FORMAT_NAME_7Z_PASS},
//	{L"/c:7zsfx",	LF_FMT_7Z,		LF_WOPT_SFX		,IDS_FORMAT_NAME_7Z_SFX},
//	{L"/c:7zsplit",	LF_FMT_7Z,		COMPRESS_SPLIT		,IDS_FORMAT_NAME_7Z_SPLIT},
//	{L"/c:7zpasssplit",LF_FMT_7Z,		LF_WOPT_DATA_ENCRYPTION | COMPRESS_SPLIT,IDS_FORMAT_NAME_7Z_PASS_SPLIT},
	{L"/c:gz",		LF_FMT_GZ,		LF_WOPT_STANDARD					,IDS_FORMAT_NAME_GZ},
	{L"/c:bz2",		LF_FMT_BZ2,		LF_WOPT_STANDARD					,IDS_FORMAT_NAME_BZ2},
	{L"/c:lzma",	LF_FMT_LZMA,		LF_WOPT_STANDARD					,IDS_FORMAT_NAME_LZMA},
	{L"/c:xz",		LF_FMT_XZ,		LF_WOPT_STANDARD					,IDS_FORMAT_NAME_XZ},
//	{L"/c:z",		LF_FMT_Z,		LF_WOPT_STANDARD					,IDS_FORMAT_NAME_Z},
	{L"/c:tar",		LF_FMT_TAR,		LF_WOPT_STANDARD					,IDS_FORMAT_NAME_TAR},
	{L"/c:tgz",		LF_FMT_TAR_GZ,	LF_WOPT_STANDARD					,IDS_FORMAT_NAME_TGZ},
	{L"/c:tbz",		LF_FMT_TAR_BZ2,	LF_WOPT_STANDARD					,IDS_FORMAT_NAME_TBZ},
	{L"/c:tlz",		LF_FMT_TAR_LZMA,	LF_WOPT_STANDARD					,IDS_FORMAT_NAME_TAR_LZMA},
	{L"/c:txz",		LF_FMT_TAR_XZ,	LF_WOPT_STANDARD					,IDS_FORMAT_NAME_TAR_XZ},
//	{L"/c:taz",		LF_FMT_TAR_Z,	LF_WOPT_STANDARD					,IDS_FORMAT_NAME_TAR_Z},
	{L"/c:uue",		LF_FMT_UUE,		LF_WOPT_STANDARD					,IDS_FORMAT_NAME_UUE},
};


const COMPRESS_COMMANDLINE_PARAMETER& get_archive_format_args(LF_ARCHIVE_FORMAT fmt, int opts)
{
	for (const auto &item : g_CompressionCmdParams) {
		if (item.Type == fmt && item.Options == opts) {
			return item;
		}
	}
	throw ARCHIVE_EXCEPTION(EINVAL);
}
