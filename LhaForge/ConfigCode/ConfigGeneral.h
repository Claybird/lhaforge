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

struct CConfigGeneral:public IConfigIO{
public:
	struct tagFiler{
		virtual ~tagFiler(){}
		std::wstring FilerPath;
		std::wstring Param;
		bool UseFiler;
	}Filer;

	bool WarnNetwork;
	bool WarnRemovable;
	bool NotifyShellAfterProcess;	//SHChangeNotifyを処理後に呼ぶならtrue
	LOSTDIR OnDirNotFound;
	LOGVIEW LogViewEvent;
	int/*LFPROCESS_PRIORITY*/ ProcessPriority;

	std::wstring TempPath;
protected:
	void loadOutput(const CConfigManager&);
	void storeOutput(CConfigManager&)const;
	void loadFiler(const CConfigManager&);
	void storeFiler(CConfigManager&)const;
	void loadLogView(const CConfigManager&);
	void storeLogView(CConfigManager&)const;
	void loadGeneral(const CConfigManager&);
	void storeGeneral(CConfigManager&)const;
public:
	virtual ~CConfigGeneral(){}
	virtual void load(const CConfigManager& Config) {
		loadOutput(Config);
		loadLogView(Config);
		loadFiler(Config);
		loadGeneral(Config);
	}
	virtual void store(CConfigManager& Config)const {
		storeOutput(Config);
		storeLogView(Config);
		storeFiler(Config);
		storeGeneral(Config);
	}
};

