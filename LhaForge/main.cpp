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
#include "resource.h"
#include "main.h"
#include "ConfigCode/configwnd.h"
#include "compress.h"
#include "extract.h"
#include "TestArchive.h"
#include "ArchiverCode/arc_interface.h"
#include "FileListWindow/FileListFrame.h"
#include "ArchiverManager.h"
#include "Dialogs/SelectDlg.h"
#include "Dialogs/ProgressDlg.h"
#include "Utilities/OSUtil.h"
#include "Utilities/StringUtil.h"
#include "Update.h"
#include "CmdLineInfo.h"

CAppModule _Module;


//---------------------------------------------

//エントリーポイント
int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE, LPTSTR lpCmdLine, int nCmdShow)
{
#if defined(_DEBUG)
	// メモリリーク検出用
	_CrtSetDbgFlag(
		_CRTDBG_ALLOC_MEM_DF
		| _CRTDBG_LEAK_CHECK_DF
		);
#endif
	_tsetlocale(LC_ALL,_T(""));	//ロケールを環境変数から取得

	HRESULT hRes = ::CoInitialize(NULL);
	ATLASSERT(SUCCEEDED(hRes));
	OleInitialize(NULL);
	// これはMicrosoft Layer for Unicode (MSLU) が使用された時の
	// ATLウインドウ thunking 問題を解決する
	::DefWindowProc(NULL, 0, 0, 0L);

	// 他のコントロールをサポートするためのフラグを追加
	AtlInitCommonControls(ICC_WIN95_CLASSES|ICC_COOL_CLASSES | ICC_BAR_CLASSES);
	_Module.Init(NULL,hInstance);
	CMessageLoop theLoop;
	_Module.AddMessageLoop(&theLoop);

	//*********************************
	// コマンドラインパラメータの解析
	//*********************************
	/*
	オプション指定形式:

	/b2e		B2E32.dllを使用
	/c:???		???形式で指定されたファイルを圧縮(???=lzh,zip,etc...)
	/method:???	???メソッドを使って圧縮(B2Eのみ)
	/b2esfx		自己解凍形式で圧縮(B2Eのみ)
	/e			ファイルを解凍
	/o:[dir]	出力先ディレクトリ指定
	/l			アーカイブファイルのリスト表示(単一)
	/f:[file]	圧縮時出力ファイル名の指定(パス指定は無視される)
	/xacrett	XacRett.DLLを強制的に使用(解凍/閲覧)
	/!			/xacrettと同等
	/m			ファイルを解凍もしくはリスト表示(設定により変更)
	/s			ファイル・フォルダを一つずつ圧縮
	/t			アーカイブファイルの完全性をテスト
	/@[file]	レスポンスファイル指定
	/$[file]	レスポンスファイル指定:レスポンスファイルは読み取り完了後削除

	*/

	bool bCheckUpdate=true;	//LFCaldixを起動するならtrue

	CMDLINEINFO cli;	//CommandLineInfo

	CConfigManager ConfigManager;
	PROCESS_MODE ProcessMode=ParseCommandLine(ConfigManager,cli);

	{
		//優先度設定
		CConfigGeneral ConfGeneral;
		ConfGeneral.load(ConfigManager);
		LFPROCESS_PRIORITY priority=(LFPROCESS_PRIORITY)ConfGeneral.ProcessPriority;
		//コマンドラインオプションで上書き?
		if(cli.PriorityOverride!=LFPRIOTITY_DEFAULT){
			priority=cli.PriorityOverride;
		}
		switch(priority){
		case LFPRIOTITY_LOW:
			UtilSetPriorityClass(IDLE_PRIORITY_CLASS);break;
		case LFPRIOTITY_LOWER:
			UtilSetPriorityClass(BELOW_NORMAL_PRIORITY_CLASS);break;
		case LFPRIOTITY_NORMAL:
			UtilSetPriorityClass(NORMAL_PRIORITY_CLASS);break;
		case LFPRIOTITY_HIGHER:
			UtilSetPriorityClass(ABOVE_NORMAL_PRIORITY_CLASS);break;
		case LFPRIOTITY_HIGH:
			UtilSetPriorityClass(HIGH_PRIORITY_CLASS);break;
		case LFPRIOTITY_DEFAULT:
		default:
			//nothing to do
			break;
		}

		//一時ディレクトリの変更
		CString strPath=ConfGeneral.TempPath;
		if(!strPath.IsEmpty()){
			//パラメータ展開に必要な情報
			std::map<stdString,CString> envInfo;
			UtilMakeExpandInformation(envInfo);
			//環境変数展開
			UtilExpandTemplateString(strPath, strPath, envInfo);

			//絶対パスに変換
			if(PathIsRelative(strPath)){
				CPath tmp=UtilGetModuleDirectoryPath();
				tmp.AddBackslash();
				tmp+=strPath;
				strPath=(LPCTSTR)tmp;
			}
			UtilGetCompletePathName(strPath,strPath);

			//環境変数設定
			SetEnvironmentVariable(_T("TEMP"),strPath);
			SetEnvironmentVariable(_T("TMP"),strPath);
		}
	}

	CString strErr;
	if(PROCESS_LIST!=ProcessMode && cli.FileList.empty()){
		//ファイル一覧ウィンドウを出す場合以外は、ファイル指定が無いときは設定ダイアログを出す
		ProcessMode=PROCESS_CONFIGURE;
	}
	CArchiverDLLManager::GetInstance().SetConfigManager(ConfigManager);

	//DLLインストール先を取得して%PATH%に追加
	CConfigUpdate ConfUpdate;
	ConfUpdate.load(ConfigManager);
	{
		//コマンド・パラメータ展開
		//---新しい環境変数
		CString strPath=ConfUpdate.strDLLPath;
		if(!strPath.IsEmpty()){
			//---実行情報取得
			//パラメータ展開に必要な情報
			std::map<stdString,CString> envInfo;
			UtilMakeExpandInformation(envInfo);
			//環境変数展開
			UtilExpandTemplateString(strPath, strPath, envInfo);

			//絶対パスに変換
			if(PathIsRelative(strPath)){
				CPath tmp=UtilGetModuleDirectoryPath();
				tmp.AddBackslash();
				tmp+=strPath;
				strPath=(LPCTSTR)tmp;
			}
			UtilGetCompletePathName(strPath,strPath);
			strPath+=_T(";");
			//---環境変数取得
			int len=GetEnvironmentVariable(_T("PATH"),NULL,0);
			CString strEnv;
			GetEnvironmentVariable(_T("PATH"),strEnv.GetBuffer(len+1),len);
			strEnv.ReleaseBuffer();

			strPath+=strEnv;
			SetEnvironmentVariable(_T("PATH"),strPath);
		}
	}

	switch(ProcessMode){
	case PROCESS_COMPRESS://圧縮
		DoCompress(ConfigManager,cli);
		break;
	case PROCESS_EXTRACT://解凍
		DoExtract(ConfigManager,cli);
		break;
	case PROCESS_AUTOMATIC://お任せ判定
		if(PathIsDirectory(*cli.FileList.begin())){
			DoCompress(ConfigManager,cli);
		}else{
			CConfigExtract ConfExtract;
			ConfExtract.load(ConfigManager);
			if(CArchiverDLLManager::GetInstance().GetArchiver(*cli.FileList.begin(),ConfExtract.DenyExt,cli.idForceDLL)){	//解凍可能な形式かどうか
				DoExtract(ConfigManager,cli);
			}else{
				DoCompress(ConfigManager,cli);
			}
		}
		break;
	case PROCESS_LIST://リスト表示
		DoList(ConfigManager,cli);
		break;
	case PROCESS_TEST://アーカイブファイルのテスト
		DoTest(ConfigManager,cli);
		break;
	case PROCESS_CONFIGURE://設定画面表示
		{
			//ここで確認を行うので終了時の確認は不要
			bCheckUpdate=false;
			//DLL更新確認
			if(CheckUpdateArchiverDLLRequired(ConfUpdate)){
				DoUpdateArchiverDLL(ConfigManager);
			}
			//ダイアログ表示
			CConfigDialog confdlg(ConfigManager);
			if(IDOK==confdlg.DoModal()){
				if(!ConfigManager.SaveConfig(strErr)){
					ErrorMessage(strErr);
				}
			}
		}
		break;
	case PROCESS_INVALID:
		TRACE(_T("Process Mode Undefined\n"));
		break;
	default:
		ASSERT(!"Unexpected Process Mode");
	}
	//アップデートチェック
	if(bCheckUpdate){
		//DLL更新確認
		if(CheckUpdateArchiverDLLRequired(ConfUpdate)){
			DoUpdateArchiverDLL(ConfigManager);
		}
	}

	TRACE(_T("Terminating...\n"));
	CArchiverDLLManager::GetInstance().Final();
//	Sleep(1);	//対症療法 for 0xC0000005: Access Violation
	_Module.RemoveMessageLoop();
	_Module.Term();
	OleUninitialize();
	::CoUninitialize();
	TRACE(_T("Exit main()\n"));
	return 0;
}

/*
formatの指定は、B2E32.dllでのみ有効
levelの指定は、B2E32.dll以外で有効
*/
bool DoCompress(CConfigManager &ConfigManager,CMDLINEINFO &cli)
{
	//圧縮オプション指定
	CString strFormat=cli.strFormat;
	CString strMethod=cli.strMethod;
	CString strLevel=cli.strLevel;

	if(cli.idForceDLL==DLL_ID_B2E){
		cli.CompressType=PARAMETER_B2E;
	}else{
		strFormat = cli.strSplitSize;
	}

	CConfigCompress ConfCompress;
	CConfigGeneral ConfGeneral;
	ConfCompress.load(ConfigManager);
	ConfGeneral.load(ConfigManager);

	while(PARAMETER_UNDEFINED==cli.CompressType || PARAMETER_B2E==cli.CompressType){	//---使用DLLを決定
		if(PARAMETER_UNDEFINED==cli.CompressType){	//形式が指定されていない場合
			if(ConfCompress.UseDefaultParameter){	//デフォルトパラメータを使うならデータ取得
				cli.CompressType=ConfCompress.DefaultType;
				cli.Options=ConfCompress.DefaultOptions;

				if(cli.CompressType==PARAMETER_B2E){	//B2Eを使用する場合
					cli.idForceDLL=DLL_ID_B2E;	//B2E32.dllの使用を明示
					//パラメータ指定
					if(cli.Options)cli.Options=COMPRESS_SFX;	//自己解凍
					cli.strFormat=ConfCompress.DefaultB2EFormat;	//形式
					cli.strMethod=ConfCompress.DefaultB2EMethod;	//メソッド
				}
			}else{	//入力を促す
				CSelectDialog SelDlg;
				SelDlg.SetDeleteAfterCompress(BOOL2bool(ConfCompress.DeleteAfterCompress));
				cli.CompressType=(PARAMETER_TYPE)SelDlg.DoModal();
				if(PARAMETER_UNDEFINED==cli.CompressType){	//キャンセルの場合
					return false;
				}else if(cli.CompressType!=PARAMETER_B2E){
					cli.idForceDLL=DLL_ID_UNKNOWN;

					cli.Options=SelDlg.GetOptions();
					cli.bSingleCompression=SelDlg.IsSingleCompression();
					cli.DeleteAfterProcess=SelDlg.GetDeleteAfterCompress() ? 1 : 0;
					break;
				}
			}
		}
		if(cli.CompressType==PARAMETER_B2E){	//B2Eを使用する場合
			//---B2E32.dllのチェック
			CArchiverB2E &B2EHandler=CArchiverDLLManager::GetInstance().GetB2EHandler();
			if(!B2EHandler.IsOK()){
				cli.CompressType=PARAMETER_UNDEFINED;
				CString msg;
				msg.Format(IDS_ERROR_DLL_LOAD,B2EHandler.GetName());
				ErrorMessage(msg);
				continue;
//				return false;
			}

			//---形式選択
			if(cli.strFormat.IsEmpty()){
				CB2ESelectDialog SelDlg;
				INT_PTR Ret=SelDlg.DoModal();
				if(IDCANCEL==Ret){	//キャンセルの場合
					return false;
				}else if(IDC_COMPRESS_USENORMAL==Ret){	//通常のDLLを使う
					cli.CompressType=PARAMETER_UNDEFINED;
				}else{
					cli.idForceDLL=DLL_ID_B2E;

					cli.Options=SelDlg.IsSFX() ? COMPRESS_SFX : 0;
					cli.bSingleCompression=SelDlg.IsSingleCompression();
					strFormat=SelDlg.GetFormat();
					strMethod=SelDlg.GetMethod();
					break;
				}
			}else{
				strFormat=cli.strFormat;
				strMethod=cli.strMethod;
				break;
			}
		}
	}

	//--------------------
	// 圧縮作業

	if(cli.bSingleCompression){	//ファイルを一つずつ圧縮
		//メッセージループを回すためのタイマー
		int timer=SetTimer(NULL,NULL,1000,UtilMessageLoopTimerProc);
		//プログレスバー
		CProgressDialog dlg;
		int nFiles=cli.FileList.size();
		if(nFiles>=2){	//ファイルが複数ある時に限定
			dlg.Create(NULL);
			dlg.SetTotalFileCount(nFiles);
			dlg.ShowWindow(SW_SHOW);
		}
		bool bRet=true;
		for(std::list<CString>::iterator ite=cli.FileList.begin();ite!=cli.FileList.end();ite++){
			//プログレスバーを進める
			if(dlg.IsWindow())dlg.SetNextState(*ite);
			while(UtilDoMessageLoop())continue;

			//圧縮作業
			std::list<CString> TempList;
			TempList.push_back(*ite);

			bRet=bRet && Compress(TempList,cli.CompressType,ConfigManager,strFormat,strMethod,strLevel,cli);
		}
		//プログレスバーを閉じる
		if(dlg.IsWindow())dlg.DestroyWindow();

		//タイマーを閉じる
		KillTimer(NULL,timer);
		return bRet;
	}else{	//通常圧縮
		return Compress(cli.FileList,cli.CompressType,ConfigManager,strFormat,strMethod,strLevel,cli);
	}
}

bool DoExtract(CConfigManager &ConfigManager,CMDLINEINFO &cli)
{
	CConfigExtract ConfExtract;
	ConfExtract.load(ConfigManager);
	MakeListFilesOnly(cli.FileList,cli.idForceDLL,ConfExtract.DenyExt,true);
	if(cli.FileList.empty()){
		ErrorMessage(CString(MAKEINTRESOURCE(IDS_ERROR_FILE_NOT_SPECIFIED)));
		return false;
	}
	return Extract(cli.FileList,ConfigManager,cli.idForceDLL,cli.OutputDir,&cli);
}

bool DoList(CConfigManager &ConfigManager,CMDLINEINFO &cli)
{
	CConfigExtract ConfExtract;
	ConfExtract.load(ConfigManager);
	bool bSpecified=!cli.FileList.empty();
	MakeListFilesOnly(cli.FileList,cli.idForceDLL,ConfExtract.DenyExt,true);
	//ファイルリストに何も残らなかったらエラーメッセージ表示
	if(bSpecified && cli.FileList.empty()){
		ErrorMessage(CString(MAKEINTRESOURCE(IDS_ERROR_FILE_NOT_SPECIFIED)));
	//	return false;
	}

//==========
// 閲覧開始
//==========
	CFileListFrame ListWindow(ConfigManager);
	ListWindow.CreateEx();
	ListWindow.ShowWindow(SW_SHOW);
	ListWindow.UpdateWindow();
	//ListWindow.AddArchiveFile(*cli.FileList.begin());
	if(!cli.FileList.empty()){
		bool bAllFailed=true;
		std::list<CString>::iterator ite=cli.FileList.begin();
		const std::list<CString>::iterator End=cli.FileList.end();
		for(;ite!=cli.FileList.end();++ite){
			HRESULT hr=ListWindow.OpenArchiveFile(*ite,cli.idForceDLL);
			if(SUCCEEDED(hr)){
				if(hr!=S_FALSE)bAllFailed=false;
			}else if(hr==E_ABORT){
				break;
			}
		}
		if(bAllFailed)ListWindow.DestroyWindow();
	}

	CMessageLoop* pLoop = _Module.GetMessageLoop();
	pLoop->Run();
	TRACE(_T("Loop End\n"));
	return true;
}

//アーカイブのテスト
bool DoTest(CConfigManager &ConfigManager,CMDLINEINFO &cli)
{
	CConfigExtract ConfExtract;
	ConfExtract.load(ConfigManager);
	//全てのファイルを検査対象にする
	MakeListFilesOnly(cli.FileList,cli.idForceDLL,ConfExtract.DenyExt,false);
	//ファイルリストに何も残らなかったらエラーメッセージ表示
	if(cli.FileList.empty()){
		ErrorMessage(CString(MAKEINTRESOURCE(IDS_ERROR_FILE_NOT_SPECIFIED)));
		return false;
	}

	//テスト
	return TestArchive(cli.FileList,ConfigManager);
}

//リストからフォルダを削除し、サブフォルダのファイルを追加
void MakeListFilesOnly(std::list<CString> &FileList,DLL_ID idForceDLL,LPCTSTR lpDenyExt,bool bArchivesOnly)
{
	std::list<CString>::iterator ite;
	for(ite=FileList.begin();ite!=FileList.end();){
		if(PathIsDirectory(*ite)){
			//---解凍対象がフォルダなら再帰解凍する
			std::list<CString> subFileList;
			UtilRecursiveEnumFile(*ite,subFileList);

			for(std::list<CString>::iterator ite2=subFileList.begin();ite2!=subFileList.end();ite2++){
				if(!bArchivesOnly||CArchiverDLLManager::GetInstance().GetArchiver(*ite2,lpDenyExt,idForceDLL)){
					//対応している形式のみ追加する必要がある時は、解凍可能な形式かどうか判定する
					FileList.push_back(*ite2);
				}
			}
			//自分は削除
			ite=FileList.erase(ite);
		}
		else{
			ite++;
		}
	}
}
