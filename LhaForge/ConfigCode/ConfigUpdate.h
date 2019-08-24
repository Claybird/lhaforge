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

#pragma once

struct CConfigUpdate:public IConfigConverter{
public:
	BOOL SilentUpdate;		//確認無くDLLインストールを行うならtrue
	BOOL AskUpdate;			//自動的にアップデートを確認するか
	int Interval;			//アップデートの周期
	time_t LastTime;		//最終アップデート確認日時
	CString strDLLPath;		//DLLがインストールされた場所
protected:
	virtual void load(CONFIG_SECTION&);	//設定をCONFIG_SECTIONから読み込む
	virtual void store(CONFIG_SECTION&)const;	//設定をCONFIG_SECTIONに書き込む
	void loadUpdate(CONFIG_SECTION&,CONFIG_SECTION&);
	void storeUpdate(CONFIG_SECTION&)const;
	void loadCaldixConf(CONFIG_SECTION&);
public:
	virtual ~CConfigUpdate(){}
	virtual void load(CConfigManager&);
	virtual void store(CConfigManager&)const;
};
