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
#include "ConfigCode/ConfigManager.h"
#include "CmdLineInfo.h"
#include "Utilities/FileOperation.h"
#include "Utilities/OSUtil.h"
#include "Utilities/StringUtil.h"
#include "compress.h"
#include "extract.h"
#include "ConfigCode/ConfigOpenAction.h"
#include "ConfigCode/ConfigGeneral.h"



class COpenActionDialog : public CDialogImpl<COpenActionDialog>
{
public:
	enum {IDD = IDD_DIALOG_OPENACTION_SELECT};
	// メッセージマップ
	BEGIN_MSG_MAP_EX(COpenActionDialog)
		COMMAND_ID_HANDLER_EX(IDC_BUTTON_OPENACTION_EXTRACT, OnButton)
		COMMAND_ID_HANDLER_EX(IDC_BUTTON_OPENACTION_LIST, OnButton)
		COMMAND_ID_HANDLER_EX(IDC_BUTTON_OPENACTION_TEST, OnButton)
		COMMAND_ID_HANDLER_EX(IDCANCEL, OnButton)
	END_MSG_MAP()

	void OnButton(UINT uNotifyCode, int nID, HWND hWndCtl){
		EndDialog(nID);
	}
};


//開く動作を選択
PROCESS_MODE SelectOpenAction()
{
	COpenActionDialog Dialog;
	switch(Dialog.DoModal()){
	case IDC_BUTTON_OPENACTION_EXTRACT:
		return PROCESS_EXTRACT;
	case IDC_BUTTON_OPENACTION_LIST:
		return PROCESS_LIST;
	case IDC_BUTTON_OPENACTION_TEST:
		return PROCESS_TEST;
	default:
		return PROCESS_INVALID;
	}
}

//-----------

std::vector<std::wstring> GetCommandLineArgs()
{
	std::vector<std::wstring> args;
	int nArgc = 0;
	LPWSTR *lplpArgs = CommandLineToArgvW(GetCommandLine(), &nArgc);
	args.resize(nArgc);
	for (int i = 1; i < nArgc; i++) {	//lplpArgs[0] is executable name
		args[i] = lplpArgs[i];
	}
	LocalFree(lplpArgs);
	return args;
}

//コマンドラインを解釈しファイルの処理方法を決定する
PROCESS_MODE ParseCommandLine(CConfigManager &ConfigManager,CMDLINEINFO &cli)
{
	std::vector<std::wstring> args = GetCommandLineArgs();

	const bool bPressedShift=GetKeyState(VK_SHIFT)<0;	//SHIFTが押されているかどうか
	const bool bPressedControl=GetKeyState(VK_CONTROL)<0;	//CONTROLが押されているかどうか
	PROCESS_MODE ProcessMode=PROCESS_AUTOMATIC;

	//デフォルト値読み込み
	CString strErr;
	if(!ConfigManager.LoadConfig(strErr))ErrorMessage((const wchar_t*)strErr);

	UTIL_CODEPAGE uCodePage= UTIL_CODEPAGE::CP932;	//レスポンスファイルのコードページ指定

	for(const auto& arg: args){
		if (arg.empty())continue;
		if (L'/' != arg[0]) {//オプションではない
			//ファイルとみなし、処理対象ファイルのリストに詰め込む
			cli.FileList.push_back(arg);
		}else{
			//------------------
			// オプションの解析
			//------------------
			CString Parameter(arg.c_str());
			//小文字に変換
			Parameter.MakeLower();
			if(0==_tcsncmp(_T("/cfg"),Parameter,4)){//設定ファイル名指定
				if(0==_tcsncmp(_T("/cfg:"),Parameter,5)){
					//出力ファイル名の切り出し;この時点で""は外れている
					cli.ConfigPath= arg.c_str() +5;

					//---環境変数(LhaForge独自定義変数)展開
					//パラメータ展開に必要な情報
					auto envInfo = LF_make_expand_information(nullptr, nullptr);

					//コマンド・パラメータ展開
					cli.ConfigPath = UtilExpandTemplateString((const wchar_t*)cli.ConfigPath, envInfo).c_str();
				}else if(_T("/cfg")==Parameter){
					cli.ConfigPath.Empty();
				}else{
					CString msg;
					msg.Format(IDS_ERROR_INVALID_PARAMETER, arg.c_str());
					ErrorMessage((const wchar_t*)msg);
					return PROCESS_INVALID;
				}
				//設定ファイルの指定が有ればセットする;無ければデフォルトに戻す
				if(cli.ConfigPath.IsEmpty()){
					ConfigManager.SetConfigFile(NULL);
				}else{
					ConfigManager.SetConfigFile(cli.ConfigPath);
				}
				//変更後の値読み込み
				if(!ConfigManager.LoadConfig(strErr))ErrorMessage((const wchar_t*)strErr);
				TRACE(_T("ConfigPath=%s\n"),cli.ConfigPath);
			}else if(0==_tcsncmp(_T("/cp"),Parameter,3)){//レスポンスファイルのコードページ指定
				if(0==_tcsncmp(_T("/cp:"),Parameter,4)){
					CString cp((LPCTSTR)Parameter+4);
					cp.MakeLower();
					if(cp==_T("utf8")||cp==_T("utf-8")){
						uCodePage= UTIL_CODEPAGE::UTF8;
					}else if(cp==_T("utf16")||cp==_T("utf-16")||cp==_T("unicode")){
						uCodePage= UTIL_CODEPAGE::UTF16;
					}else if(cp==_T("sjis")||cp==_T("shiftjis")||cp==_T("s-jis")||cp==_T("s_jis")){
						uCodePage= UTIL_CODEPAGE::CP932;
					}else{
						CString msg;
						msg.Format(IDS_ERROR_INVALID_PARAMETER, arg.c_str() +4);
						ErrorMessage((const wchar_t*)msg);
						return PROCESS_INVALID;
					}
				}else if(_T("/cp")==Parameter){
					uCodePage= UTIL_CODEPAGE::CP932;	//デフォルトに戻す
				}else{
					CString msg;
					msg.Format(IDS_ERROR_INVALID_PARAMETER, arg.c_str());
					ErrorMessage((const wchar_t*)msg);
					return PROCESS_INVALID;
				}
			}else if(0==_tcsncmp(_T("/c"),Parameter,2)){//圧縮を指示されている
				ProcessMode=PROCESS_COMPRESS;
				//----------------
				// 圧縮形式の解読
				//----------------
				if(_T("/c")==Parameter){	//形式が指定されていない場合
					cli.CompressType= LF_FMT_INVALID;
				}else if(0!=_tcsncmp(_T("/c:"),Parameter,3)){
					CString msg;
					msg.Format(IDS_ERROR_INVALID_PARAMETER, arg.c_str());
					ErrorMessage((const wchar_t*)msg);
					return PROCESS_INVALID;
				}else{
					cli.CompressType= LF_FMT_INVALID;
					//コマンドラインパラメータと形式の対応表から探す
					for(const auto &p: g_CompressionCmdParams){
						if(p.Options==Parameter){
							cli.CompressType=p.Type;
							cli.Options=p.Options;
							break;
						}
					}
					if(-1 ==cli.CompressType){
						CString msg;
						msg.Format(IDS_ERROR_INVALID_COMPRESS_PARAMETER,Parameter);
						ErrorMessage((const wchar_t*)msg);
						return PROCESS_INVALID;
					}

					//CONTROLキーが押されているなら、個別圧縮
					if(bPressedControl){
						cli.bSingleCompression=true;
					}
				}
			}else if(_T("/e")==Parameter){//解凍を指示されている
				if(bPressedShift){
					ProcessMode=PROCESS_LIST;	//SHIFTキーが押されていたら閲覧モード
				}else if(bPressedControl){
					ProcessMode=PROCESS_TEST;	//CTRLキーが押されていたら検査モード
				}else{
					ProcessMode=PROCESS_EXTRACT;
				}
			}else if(_T("/l")==Parameter){//ファイル一覧表示
				ProcessMode=PROCESS_LIST;
			}else if(_T("/t")==Parameter){//アーカイブテスト
				ProcessMode=PROCESS_TEST;
			}else if(_T("/m")==Parameter){//処理方法選択
				CConfigOpenAction ConfOpenAction;
				ConfOpenAction.load(ConfigManager);
				OPENACTION OpenAction;
				if(bPressedShift){	//---Shift押下時
					OpenAction=ConfOpenAction.OpenAction_Shift;
				}else if(bPressedControl){	//---Ctrl押下時
					OpenAction=ConfOpenAction.OpenAction_Ctrl;
				}else{	//---通常時
					OpenAction=ConfOpenAction.OpenAction;
				}
				switch(OpenAction){
				case OPENACTION_EXTRACT://解凍
					ProcessMode=PROCESS_EXTRACT;
					break;
				case OPENACTION_LIST:	//閲覧
					ProcessMode=PROCESS_LIST;
					break;
				case OPENACTION_TEST:	//検査
					ProcessMode=PROCESS_TEST;
					break;
				case OPENACTION_ASK:	//毎回確認
					ProcessMode=SelectOpenAction();
					if(ProcessMode==PROCESS_INVALID){
						return PROCESS_INVALID;
					}
					break;
				default:
					ASSERT(!"This code must not be run");
					return PROCESS_INVALID;
				}
			}else if(_T("/s")==Parameter){//ファイルを一つずつ圧縮
				cli.bSingleCompression=true;
			}else if(0==_tcsncmp(_T("/o"),Parameter,2)){//出力先フォルダ指定
				if(0==_tcsncmp(_T("/o:"),Parameter,3)){
					//出力先フォルダの切り出し;この時点で""は外れている
					cli.OutputDir=(LPCTSTR)Parameter+3;
					cli.OutputToOverride=(OUTPUT_TO)-1;
				}else if(_T("/o")==Parameter){
					cli.OutputDir.Empty();
					cli.OutputToOverride=(OUTPUT_TO)-1;
				}else if(_T("/od")==Parameter){
					//デスクトップに出力
					cli.OutputDir.Empty();
					cli.OutputToOverride=OUTPUT_TO_DESKTOP;
				}else if(_T("/os")==Parameter){
					//同一ディレクトリに出力
					cli.OutputDir.Empty();
					cli.OutputToOverride=OUTPUT_TO_SAME_DIR;
				}else if(_T("/oa")==Parameter){
					//毎回聞く
					cli.OutputDir.Empty();
					cli.OutputToOverride=OUTPUT_TO_ALWAYS_ASK_WHERE;
				}else{
					CString msg;
					msg.Format(IDS_ERROR_INVALID_PARAMETER, arg.c_str());
					ErrorMessage((const wchar_t*)msg);
					return PROCESS_INVALID;
				}
				TRACE(_T("OutputDir=%s\n"),cli.OutputDir);
			}else if(0==_tcsncmp(_T("/@"),Parameter,2)&&Parameter.GetLength()>2){//レスポンスファイル指定
				try{
					auto strFile = UtilGetCompletePathName((LPCTSTR)Parameter + 2);
					cli.FileList = UtilReadFromResponseFile(strFile.c_str(), uCodePage);
				}catch(LF_EXCEPTION){
					//読み込み失敗
					ErrorMessage((const wchar_t*)CString(MAKEINTRESOURCE(IDS_ERROR_READ_RESPONCEFILE)));
					return PROCESS_INVALID;
				}
			}else if(0==_tcsncmp(_T("/$"),Parameter,2)&&Parameter.GetLength()>2){//レスポンスファイル指定(読み取り後削除)
				try {
					auto strFile = UtilGetCompletePathName((LPCTSTR)Parameter + 2);
					cli.FileList = UtilReadFromResponseFile(strFile.c_str(), uCodePage);
					DeleteFileW(strFile.c_str());	//削除
				} catch (LF_EXCEPTION) {
					//読み込み失敗
					ErrorMessage((const wchar_t*)CString(MAKEINTRESOURCE(IDS_ERROR_READ_RESPONCEFILE)));
					return PROCESS_INVALID;
				}
			}else if(0==_tcsncmp(_T("/f"),Parameter,2)){//出力ファイル名指定
				if(0==_tcsncmp(_T("/f:"),Parameter,3)){
					//出力ファイル名の切り出し;この時点で""は外れている
					cli.OutputFileName=(LPCTSTR)Parameter+3;
				}else if(_T("/f")==Parameter){
					cli.OutputFileName.Empty();
				}else{
					CString msg;
					msg.Format(IDS_ERROR_INVALID_PARAMETER, arg.c_str());
					ErrorMessage((const wchar_t*)msg);
					return PROCESS_INVALID;
				}
				TRACE(_T("OutputFileName=%s\n"),cli.OutputFileName);
			}else if(0==_tcsncmp(_T("/mkdir:"),Parameter,7)){//解凍時の出力ディレクトリ制御
				CString mode=(LPCTSTR)Parameter+7;
				if(_T("no")==mode){
					cli.CreateDirOverride=CREATE_OUTPUT_DIR_NEVER;
				}else if(_T("single")==mode){
					cli.CreateDirOverride=CREATE_OUTPUT_DIR_SINGLE;
				}else if(_T("always")==mode){
					cli.CreateDirOverride=CREATE_OUTPUT_DIR_ALWAYS;
				}else{
					CString msg;
					msg.Format(IDS_ERROR_INVALID_PARAMETER, arg.c_str());
					ErrorMessage((const wchar_t*)msg);
					return PROCESS_INVALID;
				}
			}else if(0==_tcsncmp(_T("/popdir:"),Parameter,8)){//解凍時の出力ディレクトリ制御
				CString mode=(LPCTSTR)Parameter+8;
				if(_T("no")==mode){
					cli.IgnoreTopDirOverride=0;
				}else if(_T("yes")==mode){
					cli.IgnoreTopDirOverride=1;
				}else{
					CString msg;
					msg.Format(IDS_ERROR_INVALID_PARAMETER, arg.c_str());
					ErrorMessage((const wchar_t*)msg);
					return PROCESS_INVALID;
				}
			}else if(_T("/popdir")==Parameter){//解凍時の出力ディレクトリ制御
				cli.IgnoreTopDirOverride=1;
			}else if(0==_tcsncmp(_T("/delete:"),Parameter,8)){//処理後にソースを削除するか
				CString mode=(LPCTSTR)Parameter+8;
				if(_T("no")==mode){
					cli.DeleteAfterProcess=0;
				}else if(_T("yes")==mode){
					cli.DeleteAfterProcess=1;
				}else{
					CString msg;
					msg.Format(IDS_ERROR_INVALID_PARAMETER, arg.c_str());
					ErrorMessage((const wchar_t*)msg);
					return PROCESS_INVALID;
				}
			}else if(_T("/delete")==Parameter){//処理後に削除
				cli.DeleteAfterProcess=1;
			}else if(0==_tcsncmp(_T("/priority:"),Parameter,10)){	//プロセス優先度
				CString mode=(LPCTSTR)Parameter+10;
					 if(_T("low")==mode)	cli.PriorityOverride=LFPRIOTITY_LOW;
				else if(_T("lower")==mode)	cli.PriorityOverride=LFPRIOTITY_LOWER;
				else if(_T("normal")==mode)	cli.PriorityOverride=LFPRIOTITY_NORMAL;
				else if(_T("higher")==mode)	cli.PriorityOverride=LFPRIOTITY_HIGHER;
				else if(_T("high")==mode)	cli.PriorityOverride=LFPRIOTITY_HIGH;
				else{
					CString msg;
					msg.Format(IDS_ERROR_INVALID_PARAMETER, arg.c_str());
					ErrorMessage((const wchar_t*)msg);
					return PROCESS_INVALID;
				}
			}else{	//未知のオプション
				CString msg;
				msg.Format(IDS_ERROR_INVALID_PARAMETER, arg.c_str());
				ErrorMessage((const wchar_t*)msg);
				return PROCESS_INVALID;
			}
		}
	}
	if(cli.FileList.empty()){
		//スイッチのみが指定されていた場合には設定画面を表示させる
		//------
		// 存在しないファイルが指定されていた場合にはエラーが返っているので、
		// ここでファイルリストが空であればファイルが指定されていないと判断できる。
		//return PROCESS_CONFIGURE;
	}else{
		//expand filename pattern
		std::vector<std::wstring> tmp;
		for (const auto& item : cli.FileList) {
			auto out = UtilPathExpandWild(item.c_str());
			tmp.insert(tmp.end(), out.begin(), out.end());
		}
		cli.FileList = tmp;
	}
	//---ファイル名のフルパスなどチェック
	for(auto &item: cli.FileList){
		CPath strAbsPath;
		try {
			strAbsPath = UtilGetCompletePathName(item.c_str()).c_str();
		} catch (LF_EXCEPTION) {
			//絶対パスの取得に失敗
			ErrorMessage((const wchar_t*)CString(MAKEINTRESOURCE(IDS_ERROR_FAIL_GET_ABSPATH)));
			return PROCESS_INVALID;
		}

		//パス名の最後が\で終わっていたら\を取り除く
		strAbsPath.RemoveBackslash();
		TRACE(L"%s\n", (const wchar_t*)strAbsPath);

		//値更新
		item = (const wchar_t*)strAbsPath;
	}
	//出力フォルダが指定されていたら、それを絶対パスに変換
	if(!cli.OutputDir.IsEmpty()){
		try{
			cli.OutputDir = UtilGetCompletePathName((const wchar_t*)cli.OutputDir).c_str();
		} catch (LF_EXCEPTION) {
			//絶対パスの取得に失敗
			ErrorMessage((const wchar_t*)CString(MAKEINTRESOURCE(IDS_ERROR_FAIL_GET_ABSPATH)));
			return PROCESS_INVALID;
		}
	}

	return ProcessMode;
}

