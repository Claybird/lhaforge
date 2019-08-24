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
#include "ConfigManager.h"
#include "ConfigUpdate.h"

void CConfigUpdate::load(CONFIG_SECTION &Config){}
void CConfigUpdate::store(CONFIG_SECTION &Config)const{}

void CConfigUpdate::loadUpdate(CONFIG_SECTION &Config,CONFIG_SECTION &CaldixConfig)
{
	SilentUpdate=Config.Data[_T("SilentUpdate")].GetNParam(FALSE);

	AskUpdate=Config.Data[_T("AskUpdate")].GetNParam(TRUE);

	Interval=max(0,Config.Data[_T("Interval")].GetNParam(21));

	//最終更新日時の更新はLFCaldixに任せる
	LastTime=(time_t)max(0,(int)CaldixConfig.Data[_T("LastTime")]);
	//ユーザー用ファイルに書き込まれた最終更新日時をチェック
	LastTime=max(LastTime,(time_t)(int)Config.Data[_T("LastTime")]);
}

void CConfigUpdate::storeUpdate(CONFIG_SECTION &Config)const
{
	//更新日時はCaldix側で記録
	//更新キャンセル時はLFCaldix.iniの日時を進めるが、
	//書き込みに失敗する場合を想定してユーザー別設定ファイルにも設定を書き込む
	Config.Data[_T("SilentUpdate")]=SilentUpdate;
	Config.Data[_T("AskUpdate")]=AskUpdate;
	Config.Data[_T("Interval")]=Interval;
}

void CConfigUpdate::loadCaldixConf(CONFIG_SECTION &CaldixConfig)
{
	//DLLインストール先の取得
	strDLLPath=CaldixConfig.Data[_T("dll")];
}

void CConfigUpdate::load(CConfigManager &ConfMan)
{
	loadUpdate(ConfMan.GetSection(_T("Update")),ConfMan.GetCaldixSection(_T("Update")));
	loadCaldixConf(ConfMan.GetCaldixSection(_T("conf")));
}

void CConfigUpdate::store(CConfigManager &ConfMan)const
{
	storeUpdate(ConfMan.GetSection(_T("Update")));
}
