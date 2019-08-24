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
#include "extract.h"
#include "ArchiverManager.h"
#include "resource.h"
#include "ConfigCode/ConfigManager.h"
#include "ConfigCode/ConfigGeneral.h"
#include "ConfigCode/ConfigExtract.h"
#include "Utilities/Semaphore.h"
#include "Dialogs/LogDialog.h"
#include "Dialogs/LogListDialog.h"
#include "Dialogs/ProgressDlg.h"
#include "Dialogs/LFFolderDialog.h"
#include "Utilities/StringUtil.h"
#include "Utilities/FileOperation.h"
#include "Utilities/OSUtil.h"
#include "CommonUtil.h"
#include "CmdLineInfo.h"

//削除対象の記号リスト
const LPCWSTR g_szTable=L"0123456789./*-+{}[]@`:;!\"#$%&\'()_><=~^|,\\ 　";	//最後は半角空白,全角空白

//strOpenDir:解凍先を開くとき、実際に開くパス
bool ExtractOneArchive(CConfigManager &ConfMan,const CConfigGeneral &ConfGeneral,const CConfigExtract &ConfExtract,CArchiverDLL *lpArchiver,LPCTSTR lpArcFile,CPath &r_pathSpecificOutputDir,ARCLOG &r_ArcLog,CPath &r_pathOpenDir)
{
	//ファイル名を記録
	r_ArcLog.strFile=lpArcFile;

	//安全なアーカイブかどうか、および
	//二重にフォルダを作らないよう、先にフォルダが必要かどうかを判定する
	bool bInFolder,bSafeArchive;
	bool bSkipDirCheck=(CREATE_OUTPUT_DIR_SINGLE!=ConfExtract.CreateDir);

	CString strBaseDir;	//二重フォルダチェックのときに得られる、全てのファイルを内包するフォルダの名前(もしあれば)
	CString strExamErr;
	if(!lpArchiver->ExamineArchive(lpArcFile,ConfMan,bSkipDirCheck,bInFolder,bSafeArchive,strBaseDir,strExamErr)){
		//NOTE:B2E32.dllのために、エラーチェックを弱くしてある
		ErrorMessage(strExamErr);
		bInFolder=false;
		bSafeArchive=false;
	}

	//アーカイブ中のファイル・フォルダの数を調べる
	int nItemCount=-1;
	if(ConfExtract.CreateNoFolderIfSingleFileOnly){
		//「ファイル・フォルダが一つだけの時フォルダを作成しない」設定の時にのみ調査する
		nItemCount=lpArchiver->GetFileCount(lpArcFile);
	}

	bool bRet=false;
	CPath pathOutputDir;
	bool bUseForAll=false;	//今後も同じ出力フォルダを使うならtrue
	CString strErr;
	HRESULT hr=GetExtractDestDir(lpArcFile,ConfGeneral,ConfExtract,r_pathSpecificOutputDir,bInFolder,pathOutputDir,nItemCount,strBaseDir,r_pathOpenDir,bUseForAll,strErr);
	if(FAILED(hr)){
		if(E_ABORT == hr){
			r_ArcLog.Result=EXTRACT_CANCELED;
			r_ArcLog.strMsg.Format(IDS_ERROR_USERCANCEL);
		}else{
			r_ArcLog.Result=EXTRACT_NG;
			r_ArcLog.strMsg=strErr;
		}
		return false;
	}
	if(bUseForAll){	//今後の設定を上書き
		r_pathSpecificOutputDir=pathOutputDir;
	}

	//出力先ディレクトリをカレントディレクトリに設定
	if(!SetCurrentDirectory(pathOutputDir)){
		r_ArcLog.Result=EXTRACT_NG;
		r_ArcLog.strMsg.Format(IDS_ERROR_CANNOT_SET_CURRENT_DIR,(LPCTSTR)pathOutputDir);
		return false;
	}

	TRACE(_T("Archive Handler 呼び出し\n"));
	//------------
	// 解凍を行う
	//------------
	if(!lpArchiver->IsUnicodeCapable() && !UtilCheckT2A(pathOutputDir)){
		//UNICODEに対応していないのにUNICODEファイル名のフォルダに展開しようとした
		bRet=false;
		r_ArcLog.Result=EXTRACT_NG;
		r_ArcLog.strMsg=CString(MAKEINTRESOURCE(IDS_ERROR_UNICODEPATH));
	}else{
		CString strLog;
		bRet=lpArchiver->Extract(lpArcFile,ConfMan,ConfExtract,bSafeArchive,pathOutputDir,strLog);

		//---ログデータ
		r_ArcLog.Result=(bRet?EXTRACT_OK:EXTRACT_NG);
		r_ArcLog.strMsg=strLog;
	}
	if(ConfGeneral.NotifyShellAfterProcess){
		//解凍完了を通知
		::SHChangeNotify(SHCNE_UPDATEDIR,SHCNF_PATH,pathOutputDir,NULL);
	}
	return bRet;
}

bool DeleteOriginalArchives(const CConfigExtract &ConfExtract,LPCTSTR lpszArcFile)
{
	std::list<CString> fileList;	//削除対象のファイル一覧

	//---マルチボリュームならまとめて削除
	CString strFindParam;
	bool bMultiVolume=false;
	if(ConfExtract.DeleteMultiVolume){	//マルチボリュームでの削除が有効か？
		bMultiVolume=UtilIsMultiVolume(lpszArcFile,strFindParam);
	}

	CString strFiles;	//ファイル一覧

	if(bMultiVolume){
		UtilPathExpandWild(fileList,strFindParam);
		for(std::list<CString>::iterator ite=fileList.begin();ite!=fileList.end();++ite){
			strFiles+=_T("\n");
			strFiles+=*ite;
		}
	}else{
		fileList.push_back(lpszArcFile);
	}

	//削除する
	if(ConfExtract.MoveToRecycleBin){
		//--------------
		// ごみ箱に移動
		//--------------
		if(!ConfExtract.DeleteNoConfirm){	//削除確認する場合
			CString Message;
			if(bMultiVolume){
				//マルチボリューム
				Message.Format(IDS_ASK_MOVE_ARCHIVE_TO_RECYCLE_BIN_MANY);
				Message+=strFiles;
			}else{
				//単一ファイル
				Message.Format(IDS_ASK_MOVE_ARCHIVE_TO_RECYCLE_BIN,lpszArcFile);
			}

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
		if(!ConfExtract.DeleteNoConfirm){	//確認する場合
			CString Message;
			if(bMultiVolume){
				//マルチボリューム
				Message.Format(IDS_ASK_DELETE_ARCHIVE_MANY);
				Message+=strFiles;
			}else{
				//単一ファイル
				Message.Format(IDS_ASK_DELETE_ARCHIVE,lpszArcFile);
			}
			if(IDYES!=MessageBox(NULL,Message,UtilGetMessageCaption(),MB_YESNO|MB_ICONQUESTION|MB_DEFBUTTON2)){
				return false;
			}
		}
		//削除実行
		for(std::list<CString>::iterator ite=fileList.begin();ite!=fileList.end();++ite){
			DeleteFile(*ite);
		}
		return true;
	}
}


// アーカイブ名からフォルダパスを作成する
void GetArchiveDirectoryPath(const CConfigExtract &ConfExtract,LPCTSTR lpszArcName,CPath &pathDir)
{
	pathDir=lpszArcName;
	pathDir.StripPath();	//パス名からファイル名を取得
	pathDir.RemoveExtension();//アーカイブファイル名から拡張子を削除

	//フォルダ名末尾の数字と記号を取り除く
	if(ConfExtract.RemoveSymbolAndNumber){
		CPath pathOrg=pathDir;
		UtilTrimString(pathDir,g_szTable);
		//数字と記号を取り除いた結果、文字列が空になってしまっていたら元にもどす
		if(_tcslen(pathDir)==0){
			pathDir=pathOrg;
		}
	}

	//空白を取り除く
	UtilTrimString(pathDir,_T(".\\ 　"));

	if(_tcslen(pathDir)>0){
		pathDir.AddBackslash();
	}
}

/*************************************************************
出力先フォルダを決定する
出力先フォルダを二重に作成しないよう、bInFolder引数ですべて
フォルダに入っているかどうか確認する
*************************************************************/
HRESULT GetExtractDestDir(LPCTSTR ArcFileName,const CConfigGeneral &ConfGeneral,const CConfigExtract &ConfExtract,LPCTSTR lpSpecificOutputDir,bool bInFolder,CPath &r_pathOutputDir,const int nItemCount,LPCTSTR lpszBaseDir,CPath &r_pathOpenDir,bool &r_bUseForAll/*以降もこのフォルダに解凍するならtrueが返る*/,CString &strErr)
{
	CPath pathOutputDir;

	if(lpSpecificOutputDir && 0!=_tcslen(lpSpecificOutputDir)){
		//特定ディレクトリを出力先として指定されていた場合
		pathOutputDir=lpSpecificOutputDir;
	}else{
		//設定を元に出力先を決める
		HRESULT hr=GetOutputDirPathFromConfig(ConfExtract.OutputDirType,ArcFileName,ConfExtract.OutputDir,pathOutputDir,r_bUseForAll,strErr);
		if(FAILED(hr)){
			return hr;
		}
	}
	pathOutputDir.AddBackslash();

	// 出力先がネットワークドライブ/リムーバブルディスクであるなら警告
	// 出力先が存在しないなら、作成確認
	HRESULT hStatus=ConfirmOutputDir(ConfGeneral,pathOutputDir,strErr);
	if(FAILED(hStatus)){
		//キャンセルなど
		return hStatus;
	}

	//アーカイブ名からフォルダを作る
	CPath pathArchiveNamedDir;	//アーカイブ名のフォルダ
	bool bCreateArchiveDir=false;	//アーカイブ名のフォルダを作成する場合にはtrue
	if(
		(!ConfExtract.CreateNoFolderIfSingleFileOnly || nItemCount!=1)&&
			(
				((CREATE_OUTPUT_DIR_SINGLE==ConfExtract.CreateDir)&&!bInFolder)||
				 (CREATE_OUTPUT_DIR_ALWAYS==ConfExtract.CreateDir)
			)
	){
		GetArchiveDirectoryPath(ConfExtract,ArcFileName,pathArchiveNamedDir);
		if(_tcslen(pathArchiveNamedDir)==0){
			hStatus=S_FALSE;	//NG
		}else{
			pathOutputDir+=pathArchiveNamedDir;
			bCreateArchiveDir=true;
		}
	}

	//パス名の長さチェック
	if(S_OK==hStatus){
		if(_tcslen(pathOutputDir)>=_MAX_PATH){
			// パス名が長すぎたとき
			//TODO
			ErrorMessage(CString(MAKEINTRESOURCE(IDS_ERROR_MAX_PATH)));
			hStatus=S_FALSE;
		}
	}

	//そのままではフォルダ名を使えない場合
	if(S_FALSE==hStatus){
		// 名前を付けて保存
		CString title(MAKEINTRESOURCE(IDS_INPUT_TARGET_FOLDER_WITH_SHIFT));
		CLFFolderDialog dlg(NULL,title,BIF_RETURNONLYFSDIRS|BIF_NEWDIALOGSTYLE);
		if(IDOK==dlg.DoModal()){
			r_bUseForAll=(GetKeyState(VK_SHIFT)<0);	//TODO
			pathOutputDir=dlg.GetFolderPath();
			pathOutputDir.AddBackslash();
			if(bCreateArchiveDir){
				pathOutputDir+=pathArchiveNamedDir;
			}
		}else{
			//キャンセル
			return E_ABORT;
		}
	}

	//出力先の(アーカイブ名と同名のフォルダ名も含めた)フォルダが存在することを保証させる
	if(!UtilMakeSureDirectoryPathExists(pathOutputDir)){
		strErr.Format(IDS_ERROR_CANNOT_MAKE_DIR,(LPCTSTR)pathOutputDir);
		return E_FAIL;
	}

	//出力先フォルダ名を返す
	r_pathOutputDir=pathOutputDir;

	//開くパスの組み立て
	CPath pathToOpen=pathOutputDir;
	if(!bCreateArchiveDir){	//アーカイブ名フォルダを作成しない場合
		if(ConfExtract.CreateNoFolderIfSingleFileOnly && nItemCount==1){
			//-ファイル・フォルダが一つだけであるためフォルダを作成しなかった
			//Nothing to do
		}else if(CREATE_OUTPUT_DIR_SINGLE==ConfExtract.CreateDir && bInFolder){
			//-二重にフォルダを作らない設定に従い、フォルダを作成しなかった
			//アーカイブ内フォルダを開く
			pathToOpen.Append(lpszBaseDir);
			pathToOpen.AddBackslash();
		}
	}
	pathToOpen.QuoteSpaces();
	//開くパスを返す
	r_pathOpenDir=pathToOpen;
	return true;
}

bool Extract(std::list<CString> &ParamList,CConfigManager &ConfigManager,DLL_ID ForceDLL,LPCTSTR lpSpecificOutputDir,const CMDLINEINFO* lpCmdLineInfo)
{
	TRACE(_T("Function ::Extract() started.\n"));
	CConfigGeneral ConfGeneral;
	CConfigExtract ConfExtract;

	ConfGeneral.load(ConfigManager);
	ConfExtract.load(ConfigManager);

	//設定上書き
	if(lpCmdLineInfo){
		if(-1!=lpCmdLineInfo->OutputToOverride){
			ConfExtract.OutputDirType=lpCmdLineInfo->OutputToOverride;
		}
		if(-1!=lpCmdLineInfo->CreateDirOverride){
			ConfExtract.CreateDir=lpCmdLineInfo->CreateDirOverride;
		}
		if(-1!=lpCmdLineInfo->DeleteAfterProcess){
			ConfExtract.DeleteArchiveAfterExtract=lpCmdLineInfo->DeleteAfterProcess;
		}
	}

	//セマフォによる排他処理
	CSemaphoreLocker SemaphoreLock;
	if(ConfExtract.LimitExtractFileCount){
		SemaphoreLock.Lock(LHAFORGE_EXTRACT_SEMAPHORE_NAME,ConfExtract.MaxExtractFileCount);
	}

	UINT uFiles=ParamList.size();	//引数にあるファイルの数

	//プログレスバー
	CProgressDialog dlg;
	//メッセージループを回すためのタイマー
	int timer=NULL;
	if(uFiles>=2){	//ファイルが複数ある時に限定
		dlg.Create(NULL);
		dlg.SetTotalFileCount(uFiles);
		dlg.ShowWindow(SW_SHOW);
		timer=SetTimer(NULL,NULL,1000,UtilMessageLoopTimerProc);
	}

	//指定の出力先
	CPath pathSpecificOutputDir(lpSpecificOutputDir ? lpSpecificOutputDir : _T(""));

	std::vector<ARCLOG> LogArray;	//処理結果を保持
	bool bAllOK=true;	//すべて問題なく解凍されればtrue

	//解凍処理
	for(std::list<CString>::iterator ite_param=ParamList.begin();ite_param!=ParamList.end();++ite_param){
		//プログレスバーを進める
		if(dlg.IsWindow())dlg.SetNextState(*ite_param);

		ARCLOG arcLog;

		//メッセージループ
		while(UtilDoMessageLoop())continue;

		//アーカイバハンドラ取得
		//ここでUNICODE非対応DLLにユニコードファイル名を渡そうとした場合は弾かれる。そして、ここでは失敗の原因を解明できない
		CArchiverDLL *lpArchiver=CArchiverDLLManager::GetInstance().GetArchiver(*ite_param,ConfExtract.DenyExt,ForceDLL);
		if(!lpArchiver){
			//対応するハンドラがなかった
			arcLog.Result=EXTRACT_NOTARCHIVE;
			arcLog.strMsg.Format(IDS_ERROR_ILLEGAL_HANDLER,(LPCTSTR)*ite_param);
			arcLog.strFile=*ite_param;
			bAllOK=false;
			LogArray.push_back(arcLog);
			continue;
		}

		CPath pathOpenDir;		//ファイラが開くべきフォルダ
		//解凍実行
		bool bRet=ExtractOneArchive(ConfigManager,ConfGeneral,ConfExtract,lpArchiver,*ite_param,pathSpecificOutputDir,arcLog,pathOpenDir);
		//ログ保存
		LogArray.push_back(arcLog);

		if(!bRet){
			bAllOK=false;
		}else{
			//出力先フォルダを開く
			if(ConfExtract.OpenDir){
				if(ConfGeneral.Filer.UseFiler){
					//パラメータ展開に必要な情報
					std::map<stdString,CString> envInfo;
					MakeExpandInformationEx(envInfo,pathOpenDir,NULL);

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

			//正常に解凍できた圧縮ファイルを削除orごみ箱に移動
			if(bRet && ConfExtract.DeleteArchiveAfterExtract){
				if(!ConfExtract.ForceDelete && lpArchiver->IsWeakErrorCheck()){
					//エラーチェック機構が貧弱なため、解凍失敗時にも正常と判断してしまうような
					//DLLを使ったときには明示的に指定しない限り削除させない
					MessageBox(NULL,CString(MAKEINTRESOURCE(IDS_MESSAGE_EXTRACT_DELETE_SKIPPED)),UtilGetMessageCaption(),MB_OK|MB_ICONINFORMATION);
				}else{
					//削除
					DeleteOriginalArchives(ConfExtract,*ite_param);
				}
			}
		}
	}
	//プログレスバーを閉じる
	if(dlg.IsWindow())dlg.DestroyWindow();
	//タイマーを閉じる
	if(timer)KillTimer(NULL,timer);

	//---ログ表示
	switch(ConfGeneral.LogViewEvent){
	case LOGVIEW_ON_ERROR:
		if(!bAllOK){
			if(1==uFiles){
				//ファイル一つだけの時はダイアログボックスで
				if(EXTRACT_CANCELED!=LogArray[0].Result){
					ErrorMessage(LogArray[0].strMsg);
				}
			}else{
				//ログに表示
				CLogListDialog LogDlg(CString(MAKEINTRESOURCE(IDS_LOGINFO_OPERATION_EXTRACT)));
				LogDlg.SetLogArray(LogArray);
				LogDlg.DoModal(::GetDesktopWindow());
			}
		}
		break;
	case LOGVIEW_ALWAYS:
		//ログに表示
		CLogListDialog LogDlg(CString(MAKEINTRESOURCE(IDS_LOGINFO_OPERATION_EXTRACT)));
		LogDlg.SetLogArray(LogArray);
		LogDlg.DoModal(::GetDesktopWindow());
		break;
	}

	TRACE(_T("Exit Extract()\n"));
	return bAllOK;
}

