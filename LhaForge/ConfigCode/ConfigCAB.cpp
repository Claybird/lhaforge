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
#include "../ArchiverCode/ArchiverCAB.h"
#include "ConfigManager.h"
#include "ConfigCAB.h"

void CConfigCAB::load(CONFIG_SECTION &Config)
{
	//圧縮形式
	CompressType=(CAB_COMPRESS_TYPE)Config.Data[_T("CompressType")].GetNParam(0,CAB_COMPRESS_TYPE_LAST_ITEM,0);
	//LZX圧縮のレベル
	LZX_Level=Config.Data[_T("LZXLevel")].GetNParam(CAB_LZX_LOWEST,CAB_LZX_HIGHEST,CAB_LZX_LOWEST);
}

void CConfigCAB::store(CONFIG_SECTION &Config)const
{
	//圧縮形式
	Config.Data[_T("CompressType")]=CompressType;
	//LZX圧縮のレベル
	Config.Data[_T("LZXLevel")]=LZX_Level;
}

void CConfigCAB::load(CConfigManager &ConfMan)
{
	load(ConfMan.GetSection(_T("CAB")));
}

void CConfigCAB::store(CConfigManager &ConfMan)const
{
	store(ConfMan.GetSection(_T("CAB")));
}
