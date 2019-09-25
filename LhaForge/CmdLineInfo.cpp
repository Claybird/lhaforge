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
#include "ArchiverManager.h"
#include "Utilities/FileOperation.h"
#include "Utilities/OSUtil.h"
#include "Utilities/StringUtil.h"
#include "compress.h"
#include "extract.h"
#include "ConfigCode/ConfigOpenAction.h"
#include "ConfigCode/ConfigGeneral.h"


CMDLINEINFO::CMDLINEINFO():
	CompressType(PARAMETER_UNDEFINED),
	Options(0),
	bSingleCompression(false),
	OutputToOverride((OUTPUT_TO)-1),
	CreateDirOverride((CREATE_OUTPUT_DIR)-1),
	IgnoreTopDirOverride(-1),
	DeleteAfterProcess(-1),
	PriorityOverride(LFPRIOTITY_DEFAULT)
{}


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

//コマンドラインを解釈しファイルの処理方法を決定する
PROCESS_MODE ParseCommandLine(CConfigManager &ConfigManager,CMDLINEINFO &cli)
{
	std::vector<CString> ParamsArray;
	int nArgc=UtilGetCommandLineParams(ParamsArray);
#if defined(_DEBUG)
	//For Debug
//	MessageBox(NULL,GetCommandLine(),_T("CommandLine"),MB_OK|MB_ICONINFORMATION);
	TRACE(_T("---Command Parameter Dump---\n"));
	for(int i=0;i<nArgc;i++)
	{
		TRACE(_T("ParamsArray[%d]=%s\n"),i,ParamsArray[i]);
	}
	TRACE(_T("---End Dump---\n\n"));
#endif

	const bool bPressedShift=GetKeyState(VK_SHIFT)<0;	//SHIFTが押されているかどうか
	const bool bPressedControl=GetKeyState(VK_CONTROL)<0;	//CONTROLが押されているかどうか
	PROCESS_MODE ProcessMode=PROCESS_AUTOMATIC;

	//デフォルト値読み込み
	CString strErr;
	if(!ConfigManager.LoadConfig(strErr))ErrorMessage(strErr);

	UTIL_CODEPAGE uCodePage=UTILCP_SJIS;	//レスポンスファイルのコードページ指定

	for(int iIndex=1;iIndex<nArgc;iIndex++){
		if(0!=_tcsncmp(_T("/"),ParamsArray[iIndex],1)){//オプションではない
			//ファイルとみなし、処理対象ファイルのリストに詰め込む
			//以下はファイル名の処理
			if(0>=_tcslen(ParamsArray[iIndex])){	//引数がNULLなら無視
				continue;
			}
			cli.FileList.push_back(ParamsArray[iIndex]);
		}else{
			//------------------
			// オプションの解析
			//------------------
			CString Parameter(ParamsArray[iIndex]);
			//小文字に変換
			Parameter.MakeLower();
			if(0==_tcsncmp(_T("/cfg"),Parameter,4)){//設定ファイル名指定
				if(0==_tcsncmp(_T("/cfg:"),Parameter,5)){
					//出力ファイル名の切り出し;この時点で""は外れている
					cli.ConfigPath=(LPCTSTR)ParamsArray[iIndex]+5;

					//---環境変数(LhaForge独自定義変数)展開
					//パラメータ展開に必要な情報
					std::map<stdString,CString> envInfo;
					UtilMakeExpandInformation(envInfo);

					//コマンド・パラメータ展開
					UtilExpandTemplateString(cli.ConfigPath, cli.ConfigPath, envInfo);
				}else if(_T("/cfg")==Parameter){
					cli.ConfigPath.Empty();
				}else{
					CString msg;
					msg.Format(IDS_ERROR_INVALID_PARAMETER,ParamsArray[iIndex]);
					ErrorMessage(msg);
					return PROCESS_INVALID;
				}
				//設定ファイルの指定が有ればセットする;無ければデフォルトに戻す
				if(cli.ConfigPath.IsEmpty()){
					ConfigManager.SetConfigFile(NULL);
				}else{
					ConfigManager.SetConfigFile(cli.ConfigPath);
				}
				//変更後の値読み込み
				if(!ConfigManager.LoadConfig(strErr))ErrorMessage(strErr);
				TRACE(_T("ConfigPath=%s\n"),cli.ConfigPath);
			}else if(0==_tcsncmp(_T("/cp"),Parameter,3)){//レスポンスファイルのコードページ指定
				if(0==_tcsncmp(_T("/cp:"),Parameter,4)){
					CString cp((LPCTSTR)Parameter+4);
					cp.MakeLower();
					if(cp==_T("utf8")||cp==_T("utf-8")){
						uCodePage=UTILCP_UTF8;
					}else if(cp==_T("utf16")||cp==_T("utf-16")||cp==_T("unicode")){
						uCodePage=UTILCP_UTF16;
					}else if(cp==_T("sjis")||cp==_T("shiftjis")||cp==_T("s-jis")||cp==_T("s_jis")){
						uCodePage=UTILCP_SJIS;
					}else{
						CString msg;
						msg.Format(IDS_ERROR_INVALID_PARAMETER,(LPCTSTR)ParamsArray[iIndex]+4);
						ErrorMessage(msg);
						return PROCESS_INVALID;
					}
				}else if(_T("/cp")==Parameter){
					uCodePage=UTILCP_SJIS;	//デフォルトに戻す
				}else{
					CString msg;
					msg.Format(IDS_ERROR_INVALID_PARAMETER,ParamsArray[iIndex]);
					ErrorMessage(msg);
					return PROCESS_INVALID;
				}
			}else if(0==_tcsncmp(_T("/c"),Parameter,2)){//圧縮を指示されている
				ProcessMode=PROCESS_COMPRESS;
				//----------------
				// 圧縮形式の解読
				//----------------
				if(_T("/c")==Parameter){	//形式が指定されていない場合
					cli.CompressType=PARAMETER_UNDEFINED;
				}else if(0!=_tcsncmp(_T("/c:"),Parameter,3)){
					CString msg;
					msg.Format(IDS_ERROR_INVALID_PARAMETER,ParamsArray[iIndex]);
					ErrorMessage(msg);
					return PROCESS_INVALID;
				}else{
					cli.CompressType=PARAMETER_UNDEFINED;
					//コマンドラインパラメータと形式の対応表から探す
					for(int i=0;i<COMPRESS_PARAM_COUNT;i++){
						if(CompressParameterArray[i].Param==Parameter){
							cli.CompressType=CompressParameterArray[i].Type;
							cli.Options=CompressParameterArray[i].Options;
							break;
						}
					}
					if(PARAMETER_UNDEFINED==cli.CompressType){
						CString msg;
						msg.Format(IDS_ERROR_INVALID_COMPRESS_PARAMETER,Parameter);
						ErrorMessage(msg);
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
					msg.Format(IDS_ERROR_INVALID_PARAMETER,ParamsArray[iIndex]);
					ErrorMessage(msg);
					return PROCESS_INVALID;
				}
				TRACE(_T("OutputDir=%s\n"),cli.OutputDir);
			}else if(0==_tcsncmp(_T("/@"),Parameter,2)&&Parameter.GetLength()>2){//レスポンスファイル指定
				CString strFile;
				if(PATHERROR_NONE!=UtilGetCompletePathName(strFile,(LPCTSTR)Parameter+2)||
					!UtilReadFromResponceFile(strFile,uCodePage,cli.FileList)){
					//読み込み失敗
					ErrorMessage(CString(MAKEINTRESOURCE(IDS_ERROR_READ_RESPONCEFILE)));
					return PROCESS_INVALID;
				}
			}else if(0==_tcsncmp(_T("/$"),Parameter,2)&&Parameter.GetLength()>2){//レスポンスファイル指定(読み取り後削除)
				CString strFile;
				if(PATHERROR_NONE!=UtilGetCompletePathName(strFile,(LPCTSTR)Parameter+2)||
					!UtilReadFromResponceFile(strFile,uCodePage,cli.FileList)){
					//読み込み失敗
					ErrorMessage(CString(MAKEINTRESOURCE(IDS_ERROR_READ_RESPONCEFILE)));
					return PROCESS_INVALID;
				}
				else DeleteFile(strFile);	//削除
			}else if(0==_tcsncmp(_T("/f"),Parameter,2)){//出力ファイル名指定
				if(0==_tcsncmp(_T("/f:"),Parameter,3)){
					//出力ファイル名の切り出し;この時点で""は外れている
					cli.OutputFileName=(LPCTSTR)Parameter+3;
				}else if(_T("/f")==Parameter){
					cli.OutputFileName.Empty();
				}else{
					CString msg;
					msg.Format(IDS_ERROR_INVALID_PARAMETER,ParamsArray[iIndex]);
					ErrorMessage(msg);
					return PROCESS_INVALID;
				}
				TRACE(_T("OutputFileName=%s\n"),cli.OutputFileName);
			}else if(0==_tcsncmp(_T("/method:"),Parameter,8)){//圧縮メソッド指定
				cli.strMethod=(LPCTSTR)Parameter+8;
			}else if(0==_tcsncmp(_T("/level:"),Parameter,7)){//圧縮メソッド指定
				cli.strLevel=(LPCTSTR)Parameter+7;
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
					msg.Format(IDS_ERROR_INVALID_PARAMETER,ParamsArray[iIndex]);
					ErrorMessage(msg);
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
					msg.Format(IDS_ERROR_INVALID_PARAMETER,ParamsArray[iIndex]);
					ErrorMessage(msg);
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
					msg.Format(IDS_ERROR_INVALID_PARAMETER,ParamsArray[iIndex]);
					ErrorMessage(msg);
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
					msg.Format(IDS_ERROR_INVALID_PARAMETER,ParamsArray[iIndex]);
					ErrorMessage(msg);
					return PROCESS_INVALID;
				}
			}else if(0==_tcsncmp(_T("/volume:"),Parameter,8)){	//分割サイズ
				cli.strSplitSize=((LPCTSTR)ParamsArray[iIndex])+8;
			}else{	//未知のオプション
				CString msg;
				msg.Format(IDS_ERROR_INVALID_PARAMETER,ParamsArray[iIndex]);
				ErrorMessage(msg);
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
		//ワイルドカードの展開
		UtilPathExpandWild(cli.FileList,cli.FileList);
	}
	//---ファイル名のフルパスなどチェック
	for(std::list<CString>::iterator ite=cli.FileList.begin();ite!=cli.FileList.end();ite++){
		CPath strAbsPath;
		switch(UtilGetCompletePathName(strAbsPath,*ite)){
		case PATHERROR_NONE:
			//成功
			break;
		case PATHERROR_INVALID:
			//パラメータ指定が不正
			ASSERT(!"パラメータ指定が不正");
			return PROCESS_INVALID;
		case PATHERROR_ABSPATH:
			//絶対パスの取得に失敗
			ErrorMessage(CString(MAKEINTRESOURCE(IDS_ERROR_FAIL_GET_ABSPATH)));
			return PROCESS_INVALID;
		case PATHERROR_NOTFOUND:
			//ファイルもしくはフォルダが存在しない
			{
				CString msg;
				msg.Format(IDS_ERROR_FILE_NOT_FOUND,*ite);
				ErrorMessage(msg);
			}
			return PROCESS_INVALID;
		case PATHERROR_LONGNAME:
			//ロングファイル名取得失敗
			ErrorMessage(CString(MAKEINTRESOURCE(IDS_ERROR_FAIL_GET_LONGNAME)));
			return PROCESS_INVALID;
		}

		//パス名の最後が\で終わっていたら\を取り除く
		strAbsPath.RemoveBackslash();
		TRACE(strAbsPath),TRACE(_T("\n"));

		//値更新
		*ite=(CString)strAbsPath;
	}
	//出力フォルダが指定されていたら、それを絶対パスに変換
	if(!cli.OutputDir.IsEmpty()){
		if(!UtilGetAbsPathName(cli.OutputDir,cli.OutputDir)){
			//絶対パスの取得に失敗
			ErrorMessage(CString(MAKEINTRESOURCE(IDS_ERROR_FAIL_GET_ABSPATH)));
			return PROCESS_INVALID;
		}
	}

	return ProcessMode;
}

