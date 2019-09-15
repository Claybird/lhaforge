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
	LOAD_ENTRY(UNLHA);
	LOAD_ENTRY(7ZIP);
	LOAD_ENTRY(CAB);
	LOAD_ENTRY(TAR);
	LOAD_ENTRY(JACK);
	LOAD_ENTRY(UNARJ);
	LOAD_ENTRY(UNRAR);
	LOAD_ENTRY(UNACE);
	LOAD_ENTRY(UNIMP);
	LOAD_ENTRY(UNBEL);
	LOAD_ENTRY(UNHKI);
	LOAD_ENTRY(BGA);
	LOAD_ENTRY(AISH);
	LOAD_ENTRY(UNISO);
}

void CConfigDLL::store(CONFIG_SECTION &Config)const
{
#define SAVE_ENTRY(x) Config.Data[_T(#x)]=EnableDLL[DLL_ID_##x]
	SAVE_ENTRY(UNLHA);
	SAVE_ENTRY(7ZIP);
	SAVE_ENTRY(CAB);
	SAVE_ENTRY(TAR);
	SAVE_ENTRY(JACK);
	SAVE_ENTRY(UNARJ);
	SAVE_ENTRY(UNRAR);
	SAVE_ENTRY(UNACE);
	SAVE_ENTRY(UNIMP);
	SAVE_ENTRY(UNBEL);
	SAVE_ENTRY(UNHKI);
	SAVE_ENTRY(BGA);
	SAVE_ENTRY(AISH);
	SAVE_ENTRY(UNISO);
}

void CConfigDLL::load(CConfigManager &ConfMan)
{
	load(ConfMan.GetSection(_T("Enable")));
}

void CConfigDLL::store(CConfigManager &ConfMan)const
{
	store(ConfMan.GetSection(_T("Enable")));
}
