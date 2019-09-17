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
#include "../ArchiverManager.h"
#include "ConfigDLL.h"

void CConfigDLL::load(CONFIG_SECTION &Config)
{
#define LOAD_ENTRY(x) EnableDLL[DLL_ID_##x]=Config.Data[_T(#x)].GetNParam(TRUE)
	LOAD_ENTRY(7ZIP);
}

void CConfigDLL::store(CONFIG_SECTION &Config)const
{
#define SAVE_ENTRY(x) Config.Data[_T(#x)]=EnableDLL[DLL_ID_##x]
	SAVE_ENTRY(7ZIP);
}

void CConfigDLL::load(CConfigManager &ConfMan)
{
	load(ConfMan.GetSection(_T("Enable")));
}

void CConfigDLL::store(CConfigManager &ConfMan)const
{
	store(ConfMan.GetSection(_T("Enable")));
}
