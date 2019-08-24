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

enum LOSTDIR;
enum LOGVIEW;
enum LFPROCESS_PRIORITY{
	LFPRIOTITY_DEFAULT=0,
	LFPRIOTITY_LOW=1,
	LFPRIOTITY_LOWER=2,
	LFPRIOTITY_NORMAL=3,
	LFPRIOTITY_HIGHER=4,
	LFPRIOTITY_HIGH=5,
	LFPRIOTITY_MAX_NUM=LFPRIOTITY_HIGH,
};

struct CConfigGeneral:public IConfigConverter{
public:
	struct tagFiler{
		virtual ~tagFiler(){}
		CString FilerPath;
		CString Param;
		BOOL UseFiler;
	}Filer;

	BOOL WarnNetwork;
	BOOL WarnRemovable;
	BOOL NotifyShellAfterProcess;	//SHChangeNotifyを処理後に呼ぶならtrue
	LOSTDIR OnDirNotFound;
	LOGVIEW LogViewEvent;
	int/*LFPROCESS_PRIORITY*/ ProcessPriority;

	CString TempPath;
protected:
	virtual void load(CONFIG_SECTION&){ASSERT(!"This code cannot be run");}	//設定をCONFIG_SECTIONから読み込む
	virtual void store(CONFIG_SECTION&)const{ASSERT(!"This code cannot be run");}	//設定をCONFIG_SECTIONに書き込む

	void loadOutput(CONFIG_SECTION&);
	void storeOutput(CONFIG_SECTION&)const;
	void loadFiler(CONFIG_SECTION&);
	void storeFiler(CONFIG_SECTION&)const;
	void loadLogView(CONFIG_SECTION&);
	void storeLogView(CONFIG_SECTION&)const;
	void loadGeneral(CONFIG_SECTION&);
	void storeGeneral(CONFIG_SECTION&)const;
public:
	virtual ~CConfigGeneral(){}
	virtual void load(CConfigManager&);
	virtual void store(CConfigManager&)const;
};

