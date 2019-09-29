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
#include "TestArchive.h"
#include "ConfigCode/ConfigManager.h"
#include "ConfigCode/ConfigGeneral.h"
#include "Dialogs/LogListDialog.h"
#include "Dialogs/ProgressDlg.h"
#include "Utilities/StringUtil.h"

bool TestArchive(const std::list<CString> &rArcList,CConfigManager &ConfigManager)
{
	CArchiverDLLManager &ArchiverManager=CArchiverDLLManager::GetInstance();

	//XacRettはチェックが甘いため使わない

	//プログレスバー
	CProgressDialog dlg;
	int nFiles=rArcList.size();
	dlg.Create(NULL);
	dlg.SetTotalFileCount(nFiles);
	dlg.ShowWindow(SW_SHOW);


	std::list<CString>::const_iterator ite=rArcList.begin();
	const std::list<CString>::const_iterator end=rArcList.end();

	//テスト結果を格納する
	std::vector<ARCLOG> LogArray;
	for(;ite!=end;++ite){
		//プログレスバーを進める
		if(dlg.IsWindow())dlg.SetNextState(*ite);

		ARCLOG arcLog;
		arcLog.strFile=*ite;
		//アーカイバハンドラ取得
		//NOTE:DenyExtによる対象絞り込みは既に行われているので2回は行わない
		CArchiverDLL *Archiver=ArchiverManager.GetArchiver(*ite,NULL);

		if(!Archiver){
			//---対応するハンドラがなかった:原因を調査
			//通常のエラー
			arcLog.strMsg.Format(IDS_ERROR_ILLEGAL_HANDLER,*ite);
			arcLog.Result=TEST_NOTARCHIVE;

			//UNICODE関係のチェック
			if(!UtilCheckT2A(*ite)){
				//UNICODEに対応していないのにUNICODEファイル名のファイルを扱おうとした
				arcLog.strMsg+=_T("\r\n\r\n");
				arcLog.strMsg.AppendFormat(IDS_ERROR_UNICODEPATH);
			}
		}else{
			CString strTemp;
			arcLog.Result=Archiver->TestArchive(*ite,strTemp);
			arcLog.strMsg=Archiver->GetName();
			arcLog.strMsg+=_T("\r\n\r\n");
			arcLog.strMsg+=strTemp;
		}
		LogArray.push_back(arcLog);
	}
	//プログレスバーを閉じる
	if(dlg.IsWindow())dlg.DestroyWindow();

	//ログに表示
	CLogListDialog LogDlg(CString(MAKEINTRESOURCE(IDS_LOGINFO_OPERATION_TESTARCHIVE)));
	LogDlg.SetLogArray(LogArray);
	LogDlg.DoModal(::GetDesktopWindow());

	return true;
}


