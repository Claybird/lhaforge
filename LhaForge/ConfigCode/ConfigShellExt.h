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
#include "ConfigFile.h"

struct CConfigShellExt:public IConfigIO{
public:
	bool ShellMenuCompress;
	bool ShellMenuExtract;
	bool ShellMenuList;
	bool ShellMenuTest;
	bool ShellMenuUnderSubMenu;
	bool DragMenuCompress;
	bool DragMenuExtract;
	bool DragMenuUnderSubMenu;
	bool ForceExtraMenu;
protected:
	void loadShellMenu(const CConfigFile&);
	void storeShellMenu(CConfigFile&)const;
	void loadDragMenu(const CConfigFile&);
	void storeDragMenu(CConfigFile&)const;
	void loadExtraMenu(const CConfigFile&);
	void storeExtraMenu(CConfigFile&)const;
public:
	virtual ~CConfigShellExt(){}
	virtual void load(const CConfigFile& Config) {
		loadShellMenu(Config);
		loadDragMenu(Config);
		loadExtraMenu(Config);
	}
	virtual void store(CConfigFile& Config)const {
		storeShellMenu(Config);
		storeDragMenu(Config);
		storeExtraMenu(Config);
	}
};
