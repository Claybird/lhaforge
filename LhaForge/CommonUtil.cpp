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
#include "CommonUtil.h"
#include "ConfigCode/ConfigManager.h"
#include "ConfigCode/ConfigGeneral.h"
#include "ArchiverCode/arc_interface.h"
#include "resource.h"
#include "Dialogs/LFFolderDialog.h"
#include "Utilities/FileOperation.h"
#include "Utilities/OSUtil.h"

//設定から出力先フォルダを読み込む
//r_bUseForAll:今後も同じフォルダ設定を使うならtrue
HRESULT GetOutputDirPathFromConfig(OUTPUT_TO outputDirType,LPCTSTR lpszOrgFile,LPCTSTR lpszSpecific,CPath &r_pathOutputDir,bool &r_bUseForAll,CString &strErr)
{
	TCHAR szBuffer[_MAX_PATH+1];
	FILL_ZERO(szBuffer);

	switch(outputDirType){
	case OUTPUT_TO_SPECIFIC_DIR:	//Specific Directory
		//TRACE(_T("Specific Dir:%s\n"),Config.Common.Extract.OutputDir);
		r_pathOutputDir=lpszSpecific;
		if(_tcslen(r_pathOutputDir)>0){
			return S_OK;
		}else{
			//出力先がかかれていなければ、デスクトップに出力する
		}
		//FALLTHROUGH
	case OUTPUT_TO_DESKTOP:	//Desktop
		if(SHGetSpecialFolderPath(NULL,szBuffer,CSIDL_DESKTOPDIRECTORY,FALSE)){
			r_pathOutputDir=szBuffer;
		}else{	//デスクトップがない？
			strErr=CString(MAKEINTRESOURCE(IDS_ERROR_GET_DESKTOP));
			return E_FAIL;
		}
		return S_OK;
	case OUTPUT_TO_SAME_DIR:	//Same Directory
		_tcsncpy_s(szBuffer,lpszOrgFile,_MAX_PATH);
		PathRemoveFileSpec(szBuffer);
		r_pathOutputDir=szBuffer;
		return S_OK;
	case OUTPUT_TO_ALWAYS_ASK_WHERE:	//出力先を毎回聞く
		TRACE(_T("Always ask\n"));
		{
			//元のファイルと同じ場所にする;2回目以降は前回出力場所を使用する
			static CString s_strLastOutput;
			CPath pathTmp;
			if(s_strLastOutput.IsEmpty()){
				pathTmp=lpszOrgFile;
				pathTmp.RemoveFileSpec();
			}else{
				pathTmp=(LPCTSTR)s_strLastOutput;
			}

			CString title(MAKEINTRESOURCE(IDS_INPUT_TARGET_FOLDER_WITH_SHIFT));
			CLFFolderDialog dlg(NULL,title,BIF_RETURNONLYFSDIRS|BIF_NEWDIALOGSTYLE);
			dlg.SetInitialFolder(pathTmp);
			if(IDOK==dlg.DoModal()){
				r_bUseForAll=(GetKeyState(VK_SHIFT)<0);	//TODO
				r_pathOutputDir=dlg.GetFolderPath();
				s_strLastOutput=(LPCTSTR)r_pathOutputDir;
				return S_OK;
			}else{
				return E_ABORT;	//キャンセルされた
			}
		}
		break;
	default:
		ASSERT(!"This code cannot be run");
		return E_NOTIMPL;
	}
}


//S_FALSEが返ったときには、「名前をつけて保存」ダイアログを開く
HRESULT ConfirmOutputDir(const CConfigGeneral &Conf,LPCTSTR lpszOutputDir,CString &strErr)
{
	//---
	// 出力先がネットワークドライブ/リムーバブルディスクであるなら、出力先を選択させる
	if(Conf.WarnNetwork || Conf.WarnRemovable){
		//ルートドライブ名取得
		CPath pathDrive=lpszOutputDir;
		pathDrive.StripToRoot();

		switch(GetDriveType(pathDrive)){
		case DRIVE_REMOVABLE://ドライブからディスクを抜くことができます。
		case DRIVE_CDROM://CD-ROM
			if(Conf.WarnRemovable){
				if(IDNO==MessageBox(NULL,CString(MAKEINTRESOURCE(IDS_ASK_ISOK_REMOVABLE)),UtilGetMessageCaption(),MB_YESNO|MB_ICONQUESTION)){
					return S_FALSE;
				}
			}
			break;
		case DRIVE_REMOTE://リモート (ネットワーク) ドライブです。
		case DRIVE_NO_ROOT_DIR:
			if(Conf.WarnNetwork){
				if(IDNO==MessageBox(NULL,CString(MAKEINTRESOURCE(IDS_ASK_ISOK_NETWORK)),UtilGetMessageCaption(),MB_YESNO|MB_ICONQUESTION)){
					return S_FALSE;
				}
			}
			break;
		}
	}

	//---
	//出力先のチェック
	if(!PathIsDirectory(lpszOutputDir)){
		//パスが存在しない場合
		CString strMsg;
		switch(Conf.OnDirNotFound){
		case LOSTDIR_ASK_TO_CREATE:	//作成するかどうか聞く
			strMsg.Format(IDS_ASK_CREATE_DIR,lpszOutputDir);
			if(IDNO==MessageBox(NULL,strMsg,UtilGetMessageCaption(),MB_YESNO|MB_ICONQUESTION)){
				return E_ABORT;
			}
			//FALLTHROUGH
		case LOSTDIR_FORCE_CREATE:	//ディレクトリ作成
			if(!UtilMakeSureDirectoryPathExists(lpszOutputDir)){
				strErr.Format(IDS_ERROR_CANNOT_MAKE_DIR,lpszOutputDir);
				//ErrorMessage(strMsg);
				return E_FAIL;
			}
			break;
		default://エラーと見なす
			strErr.Format(IDS_ERROR_DIR_NOTFOUND,lpszOutputDir);
			return E_FAIL;
		}
	}

	return S_OK;
}

//UtilExpandTemplateString()のパラメータ展開に必要な情報を構築する
void MakeExpandInformationEx(std::map<stdString,CString> &envInfo,LPCTSTR lpOpenDir,LPCTSTR lpOutputFile)
{
	//環境変数で構築
	UtilMakeExpandInformation(envInfo);

	//変数登録
	if(lpOpenDir){
		envInfo[_T("dir")]=lpOpenDir;
		envInfo[_T("OutputDir")]=lpOpenDir;

		//出力ドライブ名
		TCHAR szBuffer[MAX_PATH+1];
		_tcsncpy_s(szBuffer,lpOpenDir,COUNTOF(szBuffer)-1);
		PathStripToRoot(szBuffer);
		if(szBuffer[_tcslen(szBuffer)-1]==L'\\')szBuffer[_tcslen(szBuffer)-1]=L'\0';
		envInfo[_T("OutputDrive")]=(LPCTSTR)szBuffer;
	}

	if(lpOutputFile){
		envInfo[_T("OutputFile")]=lpOutputFile;
		envInfo[_T("OutputFileName")]=PathFindFileName(lpOutputFile);
	}
}
