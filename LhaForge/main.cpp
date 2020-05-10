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
#include "resource.h"
#include "main.h"
#include "ConfigCode/configwnd.h"
#include "compress.h"
#include "extract.h"
#include "TestArchive.h"
#include "ArchiverCode/arc_interface.h"
#include "FileListWindow/FileListFrame.h"
#include "Dialogs/SelectDlg.h"
#include "Dialogs/ProgressDlg.h"
#include "Utilities/OSUtil.h"
#include "Utilities/StringUtil.h"
#include "CmdLineInfo.h"

CAppModule _Module;


bool LF_isExtractable(const wchar_t* fname)
{
	ARCHIVE_FILE_TO_READ f;
	try {
		f.read_open(fname);
		return true;
	} catch (const ARCHIVE_EXCEPTION& ) {
		return false;
	}
}


//---------------------------------------------

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR lpCmdLine, int nCmdShow)
{
#if defined(_DEBUG)
	// detect memory leaks
	_CrtSetDbgFlag(
		_CRTDBG_ALLOC_MEM_DF
		| _CRTDBG_LEAK_CHECK_DF
		);
#endif
	_tsetlocale(LC_ALL,L"");	//default locale

	HRESULT hRes = ::CoInitialize(NULL);
	ATLASSERT(SUCCEEDED(hRes));
	OleInitialize(NULL);

	// support control flags
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

		//To use custom temporary directory, if necessary
		CString strPath=ConfGeneral.TempPath;
		if(!strPath.IsEmpty()){
			//パラメータ展開に必要な情報
			auto envInfo = LF_make_expand_information(nullptr, nullptr);
			//環境変数展開
			strPath = UtilExpandTemplateString((const wchar_t*)strPath, envInfo).c_str();

			//絶対パスに変換
			if(PathIsRelative(strPath)){
				CPath tmp=UtilGetModuleDirectoryPath().c_str();
				tmp.AddBackslash();
				tmp+=strPath;
				strPath=(LPCTSTR)tmp;
			}
			try {
				auto buf = UtilGetCompletePathName((const wchar_t*)strPath);
				strPath = buf.c_str();
			} catch (LF_EXCEPTION) {
				//do nothing
			}

			//環境変数設定
			SetEnvironmentVariableW(L"TEMP",strPath);
			SetEnvironmentVariableW(L"TMP",strPath);
		}
	}

	CString strErr;
	if(PROCESS_LIST!=ProcessMode && cli.FileList.empty()){
		//ファイル一覧ウィンドウを出す場合以外は、ファイル指定が無いときは設定ダイアログを出す
		ProcessMode=PROCESS_CONFIGURE;
	}

	switch(ProcessMode){
	case PROCESS_COMPRESS://圧縮
		DoCompress(ConfigManager,cli);
		break;
	case PROCESS_EXTRACT://解凍
		DoExtract(ConfigManager,cli);
		break;
	case PROCESS_AUTOMATIC://お任せ判定
		if(PathIsDirectory(cli.FileList.front().c_str())){
			DoCompress(ConfigManager,cli);
		}else{
			CConfigExtract ConfExtract;
			ConfExtract.load(ConfigManager);
			bool isDenied = ConfExtract.DenyExt.MakeLower().Find(CString(PathFindExtension(cli.FileList.front().c_str())).MakeLower()) == -1;
			if(!isDenied && LF_isExtractable(cli.FileList.front().c_str())){	//解凍可能な形式かどうか
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
			//ダイアログ表示
			CConfigDialog confdlg(ConfigManager);
			if(IDOK==confdlg.DoModal()){
				if(!ConfigManager.SaveConfig(strErr)){
					ErrorMessage((const wchar_t*)strErr);
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

	TRACE(_T("Terminating...\n"));
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
	CConfigCompress ConfCompress;
	CConfigGeneral ConfGeneral;
	ConfCompress.load(ConfigManager);
	ConfGeneral.load(ConfigManager);

	while(LF_FMT_INVALID == cli.CompressType){	//---使用DLLを決定
		if(ConfCompress.UseDefaultParameter){	//デフォルトパラメータを使うならデータ取得
			cli.CompressType = ConfCompress.DefaultType;
			cli.Options = ConfCompress.DefaultOptions;
		}else{	//入力を促す
			CSelectDialog SelDlg;
			SelDlg.SetDeleteAfterCompress(BOOL2bool(ConfCompress.DeleteAfterCompress));
			cli.CompressType=(LF_ARCHIVE_FORMAT)SelDlg.DoModal();
			if(LF_FMT_INVALID ==cli.CompressType){	//キャンセルの場合
				return false;
			}else{
				cli.Options=SelDlg.GetOptions();
				cli.bSingleCompression=SelDlg.IsSingleCompression();
				cli.DeleteAfterProcess=SelDlg.GetDeleteAfterCompress() ? 1 : 0;
				break;
			}
		}
	}

	//--------------------
	// 圧縮作業

	if(cli.bSingleCompression){	//ファイルを一つずつ圧縮
		//プログレスバー
		CProgressDialog dlg;
		int nFiles=cli.FileList.size();
		dlg.Create(NULL);
		dlg.ShowWindow(SW_SHOW);
		bool bRet=true;
		int count = 0;
		for(const auto &filename: cli.FileList){
			//プログレスバーを進める
			if (dlg.IsWindow())dlg.SetProgress(filename.c_str(), count, nFiles, L"*prepare*", 0, 0);
			while(UtilDoMessageLoop())continue;

			//圧縮作業
			std::list<CString> TempList;
			TempList.push_back(filename.c_str());

#pragma message("FIXME!")
			//bRet=bRet && Compress(TempList,cli.CompressType,ConfigManager,cli);
			count += 1;
		}
		//プログレスバーを閉じる
		if(dlg.IsWindow())dlg.DestroyWindow();

		return bRet;
	}else{	//通常圧縮
#pragma message("FIXME!")
		//return Compress(cli.FileList,cli.CompressType,ConfigManager,cli);
		return FALSE;
	}
}

bool DoExtract(CConfigManager &ConfigManager,CMDLINEINFO &cli)
{
	CConfigExtract ConfExtract;
	ConfExtract.load(ConfigManager);

	std::list<CString> tmp;
	for (const auto& item : cli.FileList) {
		tmp.push_back(item.c_str());
	}

	MakeListFilesOnly(tmp,ConfExtract.DenyExt,true);

	cli.FileList.clear();
	for (const auto& item : tmp) {
		cli.FileList.push_back((const wchar_t*)item);
	}

	if(cli.FileList.empty()){
		ErrorMessage((const wchar_t*)CString(MAKEINTRESOURCE(IDS_ERROR_FILE_NOT_SPECIFIED)));
		return false;
	}
	return GUI_extract_multiple_files(cli.FileList, &cli);
}

bool DoList(CConfigManager &ConfigManager,CMDLINEINFO &cli)
{
	CConfigExtract ConfExtract;
	ConfExtract.load(ConfigManager);
	bool bSpecified=!cli.FileList.empty();

	std::list<CString> tmp;
	for (const auto& item : cli.FileList) {
		tmp.push_back(item.c_str());
	}

	MakeListFilesOnly(tmp, ConfExtract.DenyExt, true);

	cli.FileList.clear();
	for (const auto& item : tmp) {
		cli.FileList.push_back((const wchar_t*)item);
	}

	//ファイルリストに何も残らなかったらエラーメッセージ表示
	if(bSpecified && cli.FileList.empty()){
		ErrorMessage((const wchar_t*)CString(MAKEINTRESOURCE(IDS_ERROR_FILE_NOT_SPECIFIED)));
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
		for(const auto& item: cli.FileList){
			HRESULT hr = ListWindow.OpenArchiveFile(item.c_str());
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
	std::list<CString> tmp;
	for (const auto& item : cli.FileList) {
		tmp.push_back(item.c_str());
	}

	MakeListFilesOnly(tmp, ConfExtract.DenyExt, false);

	cli.FileList.clear();
	for (const auto& item : tmp) {
		cli.FileList.push_back((const wchar_t*)item);
	}
	//ファイルリストに何も残らなかったらエラーメッセージ表示
	if(cli.FileList.empty()){
		ErrorMessage((const wchar_t*)CString(MAKEINTRESOURCE(IDS_ERROR_FILE_NOT_SPECIFIED)));
		return false;
	}

	//テスト
	return TestArchive(tmp,ConfigManager);
}

//リストからフォルダを削除し、サブフォルダのファイルを追加
void MakeListFilesOnly(std::list<CString> &FileList,LPCTSTR lpDenyExt,bool bArchivesOnly)
{
	for(auto ite=FileList.begin();ite!=FileList.end();){
		if(PathIsDirectory(*ite)){
			//---解凍対象がフォルダなら再帰解凍する
			auto subFileList = UtilRecursiveEnumFile((const wchar_t*)*ite);

			for(const auto& subFile: subFileList){
				bool isDenied = CString(lpDenyExt).MakeLower().Find(CString(PathFindExtension(subFile.c_str())).MakeLower()) == -1;

				if(!bArchivesOnly|| (!isDenied && LF_isExtractable(subFile.c_str()))){
					//対応している形式のみ追加する必要がある時は、解凍可能な形式かどうか判定する
					FileList.push_back(subFile.c_str());	//TODO
				}
			}
			//自分は削除
			ite=FileList.erase(ite);
		}
		else{
			++ite;
		}
	}
}


#ifdef UNIT_TEST
#include <gtest/gtest.h>
int wmain(int argc, wchar_t *argv[], wchar_t *envp[]){
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
#endif
