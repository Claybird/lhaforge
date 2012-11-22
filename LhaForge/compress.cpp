/*
 * Copyright (c) 2005-2012, Claybird
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
#include "compress.h"
#include "Dialogs/LogDialog.h"

#include "resource.h"

#include "archivermanager.h"
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

bool Compress(const std::list<CString> &_ParamList,const PARAMETER_TYPE Type,CConfigManager &ConfigManager,LPCTSTR lpszFormat,LPCTSTR lpszMethod,LPCTSTR lpszLevel,CMDLINEINFO& CmdLineInfo)
{
	TRACE(_T("Function ::Compress() started.\n"));

	std::list<CString> ParamList=_ParamList;

	CArchiverDLLManager &ArchiverManager=CArchiverDLLManager::GetInstance();
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

	CArchiverDLL *lpArchiver=ArchiverManager.GetArchiver(GetDllIDFromParameterType(Type));
	if(!lpArchiver || !lpArchiver->IsOK()){
		return false;
	}

	//ディレクトリ内にファイルが全て入っているかの判定
	if(ConfCompress.IgnoreTopDirectory){
		//NOTE: this code equals to ParamList.size()==1
		std::list<CString>::const_iterator ite=ParamList.begin();
		ite++;
		if(ite==ParamList.end()){
			//ファイルが一つしかない
			CPath path=*ParamList.begin();
			path.AddBackslash();
			if(path.IsDirectory()){
				//単体のディレクトリを圧縮
				UtilPathExpandWild(ParamList,path+_T("*"));
			}
			if(ParamList.empty()){
				ParamList=_ParamList;
			}
		}
	}

	//UNICODE対応のチェック
	if(!lpArchiver->IsUnicodeCapable()){		//UNICODEに対応していない
		if(!UtilCheckT2AList(ParamList)){
			//ファイル名にUNICODE文字を持つファイルを圧縮しようとした
			ErrorMessage(CString(MAKEINTRESOURCE(IDS_ERROR_UNICODEPATH)));
			return false;
		}
	}

	//----------------------------------------------
	// GZ/BZ2/JACKで圧縮可能(単ファイル)かどうかチェック
	//----------------------------------------------
	if(PARAMETER_GZ==Type||PARAMETER_BZ2==Type||PARAMETER_XZ==Type||PARAMETER_LZMA==Type||PARAMETER_JACK==Type){
		int nFileCount=0;
		CPath pathFileName;
		for(std::list<CString>::iterator ite=ParamList.begin();ite!=ParamList.end();++ite){
			nFileCount+=FindFileToCompress(*ite,pathFileName);
			if(nFileCount>=2){
				ErrorMessage(CString(MAKEINTRESOURCE(IDS_ERROR_SINGLE_FILE_ONLY)));
				return false;
			}
		}
		if(0==nFileCount){
			ErrorMessage(CString(MAKEINTRESOURCE(IDS_ERROR_FILE_NOT_SPECIFIED)));
			return false;
		}
		ParamList.clear();
		ParamList.push_back(pathFileName);
	}else if(PARAMETER_ISH==Type||PARAMETER_UUE==Type){	// ISH/UUEで変換可能か(ファイルかどうか)チェック
		for(std::list<CString>::iterator ite=ParamList.begin();ite!=ParamList.end();++ite){
			if(PathIsDirectory(*ite)){
				ErrorMessage(CString(MAKEINTRESOURCE(IDS_ERROR_CANT_COMPRESS_FOLDER)));
				return false;
			}
		}
	}

	//=========================================================
	// カレントディレクトリ設定のために\で終わる基点パスを取得
	//=========================================================
	CString pathBase;
	UtilGetBaseDirectory(pathBase,ParamList);
	TRACE(pathBase),TRACE(_T("\n"));

	//カレントディレクトリ設定
	SetCurrentDirectory(pathBase);

	//圧縮先ファイル名決定
	TRACE(_T("圧縮先ファイル名決定\n"));
	CPath pathArcFileName;
	CString strErr;
	HRESULT hr=GetArchiveName(pathArcFileName,_ParamList,Type,Options,ConfCompress,ConfGeneral,CmdLineInfo,lpArchiver->IsUnicodeCapable(),strErr);
	if(FAILED(hr)){
		if(hr!=E_ABORT){
			ErrorMessage(strErr);
		}
		return false;
	}

	//====================================================================
	// 未対応DLLでファイル名にUNICODE文字を持つファイルを圧縮しようとした
	//====================================================================
	if(!lpArchiver->IsUnicodeCapable()&&!UtilCheckT2A(pathArcFileName)){
		//GetArchiveNameで弾き切れなかったもの(B2E/JACKなど)をここで弾く
		//B2Eは別系統で処理されているが、念のため。
		ErrorMessage(CString(MAKEINTRESOURCE(IDS_ERROR_UNICODEPATH)));
		return false;
	}

	if(PARAMETER_JACK!=Type){
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
		// レスポンスファイル内に記入するために
		// 圧縮対象ファイル名を修正する
		//======================================
		const int nBasePathLength=pathBase.GetLength();
		for(std::list<CString>::iterator ite=ParamList.begin();ite!=ParamList.end();++ite){
			//ベースパスを元に相対パス取得 : 共通である基底パスの文字数分だけカットする
			//(*ite)=(*ite).Right((*ite).GetLength()-BasePathLength);
			//(*ite).Delete(0,nBasePathLength);
			(*ite)=(LPCTSTR)(*ite)+nBasePathLength;

			//もしBasePathと同じになって空になってしまったパスがあれば、カレントディレクトリ指定を代わりに入れておく
			if((*ite).IsEmpty()){
				*ite=_T(".");
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
	/*
	formatの指定は、B2E32.dllでのみ有効
	levelの指定は、B2E32.dll以外で有効
	*/
	bool bRet=lpArchiver->Compress(pathArcFileName,ParamList,ConfigManager,Type,Options,lpszFormat,lpszMethod,lpszLevel,strLog);
	//---ログ表示
	switch(ConfGeneral.LogViewEvent){
	case LOGVIEW_ON_ERROR:
		if(!bRet){
			CLogDialog LogDlg;
			LogDlg.SetData(strLog);
			LogDlg.DoModal();
		}
		break;
	case LOGVIEW_ALWAYS:
		CLogDialog LogDlg;
		LogDlg.SetData(strLog);
		LogDlg.DoModal();
		break;
	}

	if(ConfGeneral.NotifyShellAfterProcess){
		//圧縮完了を通知
		::SHChangeNotify(SHCNE_CREATE,SHCNF_PATH,pathArcFileName,NULL);
	}

	//出力先フォルダを開く
	if(bRet && ConfCompress.OpenDir){
		CPath pathOpenDir=pathArcFileName;
		pathOpenDir.RemoveFileSpec();
		pathOpenDir.AddBackslash();
		if(ConfGeneral.Filer.UseFiler){
			//パラメータ展開に必要な情報
			std::map<stdString,CString> envInfo;
			MakeExpandInformationEx(envInfo,pathOpenDir,pathArcFileName);

			//コマンド・パラメータ展開
			CString strCmd,strParam;
			UtilExpandTemplateString(strCmd,ConfGeneral.Filer.FilerPath,envInfo);	//コマンド
			UtilExpandTemplateString(strParam,ConfGeneral.Filer.Param,envInfo);	//パラメータ
			ShellExecute(NULL, _T("open"), strCmd,strParam, NULL, SW_SHOWNORMAL);
		}else{
			//Explorerで開く
			UtilNavigateDirectory(pathOpenDir);
		}
	}

	//正常に圧縮できたファイルを削除orごみ箱に移動
	if(bRet && ConfCompress.DeleteAfterCompress){
		if(!ConfCompress.ForceDelete && lpArchiver->IsWeakErrorCheck()){
			//エラーチェック機構が貧弱なため、圧縮失敗時にも正常と判断してしまうような
			//DLLを使ったときには明示的に指定しない限り削除させない
			MessageBox(NULL,CString(MAKEINTRESOURCE(IDS_MESSAGE_COMPRESS_DELETE_SKIPPED)),UtilGetMessageCaption(),MB_OK|MB_ICONINFORMATION);
		}else{
			//カレントディレクトリは削除できないので別の場所へ移動
			::SetCurrentDirectory(UtilGetModuleDirectoryPath());
			//削除:オリジナルの指定で消す
			DeleteOriginalFiles(ConfCompress,_ParamList);
		}
	}


	TRACE(_T("Exit Compress()\n"));
	return bRet;
}

void GetArchiveFileExtension(PARAMETER_TYPE type,LPCTSTR lpszOrgFile,CString &rExt,LPCTSTR lpszDefaultExt)
{
	switch(type){
	case PARAMETER_GZ:
	case PARAMETER_BZ2:
	case PARAMETER_XZ:
	case PARAMETER_LZMA:
		rExt=PathFindExtension(lpszOrgFile);
		rExt+=lpszDefaultExt;
		break;
	case PARAMETER_JACK:
		//JACKの場合にはアーカイブファイル名が出力先フォルダ名になる
		//:ファイル名はDLLが生成する
		rExt.Empty();
		break;
	default:
		rExt=lpszDefaultExt;
		break;
	}
}

HRESULT CheckArchiveName(LPCTSTR lpszArcFile,const std::list<CString> &rOrgFileList,bool bOverwrite,bool bUnicodeCapable,CString &strErr)
{
	// ファイル名が長すぎたとき
	if(_tcslen(lpszArcFile)>=_MAX_PATH){
		strErr=CString(MAKEINTRESOURCE(IDS_ERROR_MAX_PATH));
		return E_LF_CANNOT_DECIDE_FILENAME;
	}

	// できあがったファイル名が入力元のファイル名と同じか
	std::list<CString>::const_iterator ite,end;
	end=rOrgFileList.end();
	for(ite=rOrgFileList.begin();ite!=end;++ite){
		if(0==(*ite).CompareNoCase(lpszArcFile)){
			strErr.Format(IDS_ERROR_SAME_INPUT_AND_OUTPUT,lpszArcFile);
			return E_LF_OVERWRITE_SOURCE;
		}
	}

	// 未対応DLLにUNICODEファイル名を渡していないか
	if(!bUnicodeCapable && !UtilCheckT2A(lpszArcFile)){
		strErr=CString(MAKEINTRESOURCE(IDS_ERROR_UNICODEPATH));
		return E_LF_CANNOT_DECIDE_FILENAME;
	}

	// ファイルが既に存在するかどうか
	if(!bOverwrite){
		if(PathFileExists(lpszArcFile)){
			// ファイルが存在したら問題あり
			// この場合はエラーメッセージを出さずにファイル名を入力させる
			return S_LF_ARCHIVE_EXISTS;
		}
	}

	return S_OK;
}

int FindCompressionParamIndex(PARAMETER_TYPE type,int Options)
{
	//圧縮形式の情報を検索
	for(int nIndex=0;nIndex<COMPRESS_PARAM_COUNT;nIndex++){
		if(type==CompressParameterArray[nIndex].Type){
			if(Options==CompressParameterArray[nIndex].Options){
				return nIndex;
			}
		}
	}
	return -1;
}

//上書き確認など
HRESULT ConfirmOutputFile(CPath &r_pathArcFileName,const std::list<CString> &rOrgFileNameList,LPCTSTR lpExt,BOOL bAskName,bool bUnicodeCapable)
{
	//----------------------------
	// ファイル名指定が適切か確認
	//----------------------------
	bool bForceOverwrite=false;
	BOOL bInputFilenameFirst=bAskName;//Config.Common.Compress.SpecifyOutputFilename;

	HRESULT hStatus=E_UNEXPECTED;
	while(S_OK!=hStatus){
		if(!bInputFilenameFirst){
			//ファイル名の長さなど確認
			CString strLocalErr;
			hStatus=CheckArchiveName(r_pathArcFileName,rOrgFileNameList,bForceOverwrite,bUnicodeCapable,strLocalErr);
			if(FAILED(hStatus)){
				ErrorMessage(strLocalErr);
			}
			if(S_OK==hStatus){
				break;	//完了
			}
		}

		//最初のファイル名は仮生成したものを使う
		//選択されたファイル名が返ってくる
		CPath pathFile;
		if(E_LF_CANNOT_DECIDE_FILENAME==hStatus){
			//ファイル名が決定できない場合、カレントディレクトリを現行パスとする
			TCHAR szBuffer[_MAX_PATH+1]={0};
			GetCurrentDirectory(_MAX_PATH,szBuffer);
			pathFile=szBuffer;
			pathFile.Append(CString(MAKEINTRESOURCE(IDS_UNTITLED)) + lpExt);
		}else{
			pathFile=r_pathArcFileName;
		}

		//==================
		// 名前を付けて保存
		//==================
		//フィルタ文字列生成
		CString strFilter(MAKEINTRESOURCE(IDS_COMPRESS_FILE));
		strFilter.AppendFormat(_T(" (*%s)|*%s"),lpExt,lpExt);
		TCHAR filter[_MAX_PATH+2]={0};
		UtilMakeFilterString(strFilter,filter,COUNTOF(filter));

		CFileDialog dlg(FALSE, lpExt, pathFile, OFN_OVERWRITEPROMPT|OFN_NOCHANGEDIR,filter);
		if(IDCANCEL==dlg.DoModal()){	//キャンセル
			return E_ABORT;
		}

		r_pathArcFileName=dlg.m_szFileName;
		//DELETED:ファイル名のロングパスを取得

		bForceOverwrite=true;
		bInputFilenameFirst=false;
	}
	return S_OK;
}

/*************************************************************
アーカイブファイル名を決定する。

 基本的には、コマンドライン引数で与えられるファイル名のうち、
最初のファイル名を元にして、拡張子だけを変えたファイルを作る。
*************************************************************/
HRESULT GetArchiveName(CPath &r_pathArcFileName,const std::list<CString> &OrgFileNameList,const PARAMETER_TYPE Type,const int Options,const CConfigCompress &ConfCompress,const CConfigGeneral &ConfGeneral,CMDLINEINFO &rCmdLineInfo,bool bUnicodeCapable,CString &strErr)
{
	ASSERT(!OrgFileNameList.empty());

	//==================
	// ファイル名の生成
	//==================
	BOOL bDirectory=FALSE;	//圧縮対象に指定された物がフォルダならtrue;フォルダとファイルで末尾の'.'の扱いを変える
	BOOL bDriveRoot=FALSE;	//圧縮対象がドライブのルート(X:\ etc.)ならtrue
	CPath pathOrgFileName;


	bool bFileNameSpecified=(0<rCmdLineInfo.OutputFileName.GetLength());

	//---元のファイル名
	if(bFileNameSpecified){
		//TODO:ここで処理するのはおかしい
		pathOrgFileName=(LPCTSTR)rCmdLineInfo.OutputFileName;
		pathOrgFileName.StripPath();	//パス名を取り除く
	}else{
		//先頭のファイル名を元に暫定的にファイル名を作り出す
		pathOrgFileName=(LPCTSTR)*OrgFileNameList.begin();

		//---判定
		//ディレクトリかどうか
		bDirectory=pathOrgFileName.IsDirectory();
		//ドライブのルートかどうか
		CPath pathRoot=pathOrgFileName;
		pathRoot.AddBackslash();
		bDriveRoot=pathRoot.IsRoot();
		if(bDriveRoot){
			//ボリューム名取得
			TCHAR szVolume[MAX_PATH];
			GetVolumeInformation(pathRoot,szVolume,MAX_PATH,NULL,NULL,NULL,NULL,0);
			//ドライブを丸ごと圧縮する場合には、ボリューム名をファイル名とする
			UtilFixFileName(pathOrgFileName,szVolume,_T('_'));
		}
	}

	//圧縮形式の情報を検索
	int nIndex=FindCompressionParamIndex(Type,Options);
	if(-1==nIndex){
		strErr=CString(MAKEINTRESOURCE(IDS_ERROR_ILLEGAL_FORMAT_TYPE));
		return E_FAIL;
	}

	//拡張子決定
	CString strExt;
	if(bFileNameSpecified){
		strExt=PathFindExtension(pathOrgFileName);
	}else{
		GetArchiveFileExtension(Type,pathOrgFileName,strExt,CompressParameterArray[nIndex].Ext);
	}

	//出力先フォルダ名
	CPath pathOutputDir;
	if(0!=rCmdLineInfo.OutputDir.GetLength()){
		//出力先フォルダが指定されている場合
		pathOutputDir=(LPCTSTR)rCmdLineInfo.OutputDir;
	}else{
		//設定を元に出力先を決める
		bool bUseForAll;
		HRESULT hr=GetOutputDirPathFromConfig(ConfCompress.OutputDirType,pathOrgFileName,ConfCompress.OutputDir,pathOutputDir,bUseForAll,strErr);
		if(FAILED(hr)){
			return hr;
		}
		if(bUseForAll){
			rCmdLineInfo.OutputDir=(LPCTSTR)pathOutputDir;
		}
	}
	pathOutputDir.AddBackslash();

	// 出力先がネットワークドライブ/リムーバブルディスクであるなら警告
	// 出力先が存在しないなら、作成確認
	HRESULT hRes=ConfirmOutputDir(ConfGeneral,pathOutputDir,strErr);
	if(FAILED(hRes)){
		//キャンセルなど
		return hRes;
	}

	if(PARAMETER_JACK==Type){
		//JACK形式の場合は出力フォルダだけ指定
		r_pathArcFileName=pathOutputDir;
		return S_OK;
	}

	//ファイル名から拡張子とパス名を削除
	if(bDirectory || bDriveRoot){	//ディレクトリ
		//ディレクトリからは拡張子(最後の'.'以降の文字列)を削除しない
		CPath pathDir=pathOrgFileName;
		pathDir.StripPath();

		r_pathArcFileName=pathOutputDir;
		r_pathArcFileName.Append(pathDir);
		r_pathArcFileName=(LPCTSTR)(((CString)r_pathArcFileName)+strExt);
	}else{	//ファイル
		CPath pathFile=pathOrgFileName;
		pathFile.StripPath();
		r_pathArcFileName=pathOutputDir;
		r_pathArcFileName.Append(pathFile);
		if(PARAMETER_B2E!=Type){
			//B2Eの時は拡張子の扱いをB2E32.dllに任せる
			r_pathArcFileName.RemoveExtension();
		}
		r_pathArcFileName=(LPCTSTR)(((CString)r_pathArcFileName)+strExt);
	}

	//---B2Eを特別扱い
	if(PARAMETER_B2E==Type){
		return S_OK;
	}

	//ドライブルートを圧縮する場合、ファイル名は即決しない
	return ConfirmOutputFile(r_pathArcFileName,OrgFileNameList,strExt,ConfCompress.SpecifyOutputFilename || bDriveRoot,bUnicodeCapable);
}


//pathFileNameは最初のファイル名
//深さ優先探索
int FindFileToCompress(LPCTSTR lpszPath,CPath &pathFileName)
{
	if(!PathIsDirectory(lpszPath)){	//パスがファイルの場合
		pathFileName=lpszPath;
		return 1;
	}else{	//パスがフォルダの場合、フォルダ内部のファイルを探す
		int nFileCount=0;

		CPath pathWild=lpszPath;
		pathWild.Append(_T("\\*"));
		CFindFile cFind;

		BOOL bFound=cFind.FindFile(pathWild);
		for(;bFound;bFound=cFind.FindNextFile()){
			if(cFind.IsDots())continue;

			if(cFind.IsDirectory()){	//サブディレクトリ検索
				nFileCount+=FindFileToCompress(cFind.GetFilePath(),pathFileName);
				if(nFileCount>=2){
					return 2;
				}
			}else{
				pathFileName=(LPCTSTR)cFind.GetFilePath();
				nFileCount++;
				if(nFileCount>=2){
					return 2;
				}
			}
		}
		return nFileCount;
	}
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
