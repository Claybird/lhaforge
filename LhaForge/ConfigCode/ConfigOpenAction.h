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
#include "ConfigManager.h"

enum OPENACTION{
	OPENACTION_EXTRACT,
	OPENACTION_LIST,
	OPENACTION_TEST,
	OPENACTION_ASK,

	ENUM_COUNT_AND_LASTITEM(OPENACTION),
};

struct CConfigOpenAction:public IConfigIO{	//関連付けで開いた場合(/m)のデフォルト動作
public:
	OPENACTION OpenAction;
	OPENACTION OpenAction_Shift;
	OPENACTION OpenAction_Ctrl;
public:
	virtual ~CConfigOpenAction(){}
	virtual void load(const CConfigManager&);
	virtual void store(CConfigManager&)const;
};
