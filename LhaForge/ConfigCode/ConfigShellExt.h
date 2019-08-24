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

struct CConfigShellExt:public IConfigConverter{
public:
	BOOL ShellMenuCompress;
	BOOL ShellMenuExtract;
	BOOL ShellMenuList;
	BOOL ShellMenuTest;
	BOOL ShellMenuUnderSubMenu;
	BOOL DragMenuCompress;
	BOOL DragMenuExtract;
	BOOL DragMenuUnderSubMenu;
	BOOL ForceExtraMenu;
	BOOL UseCustomMenu;
protected:
	virtual void load(CONFIG_SECTION&);	//設定をCONFIG_SECTIONから読み込む
	virtual void store(CONFIG_SECTION&)const;	//設定をCONFIG_SECTIONに書き込む
	void loadShellMenu(CONFIG_SECTION&);
	void storeShellMenu(CONFIG_SECTION&)const;
	void loadDragMenu(CONFIG_SECTION&);
	void storeDragMenu(CONFIG_SECTION&)const;
	void loadExtraMenu(CONFIG_SECTION&);
	void storeExtraMenu(CONFIG_SECTION&)const;
	void loadCustomMenu(CONFIG_SECTION&);
	void storeCustomMenu(CONFIG_SECTION&)const;
public:
	virtual ~CConfigShellExt(){}
	virtual void load(CConfigManager&);
	virtual void store(CConfigManager&)const;
};
