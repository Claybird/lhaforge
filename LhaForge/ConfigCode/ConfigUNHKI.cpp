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
#include "../ArchiverCode/ArchiverUNHKI.h"
#include "ConfigManager.h"
#include "ConfigUNHKI.h"

// HKI圧縮設定
void CConfigHKI::load(CONFIG_SECTION &Config)
{
	//圧縮レベル
	CompressLevel=(HKI_COMPRESS_LEVEL)Config.Data[_T("CompressLevel")].GetNParam(0,HKI_COMPRESS_LEVEL_LAST_ITEM,0);
	//暗号化アルゴリズム
	EncryptAlgorithm=(HKI_ENCRYPT_ALGORITHM)Config.Data[_T("EncryptAlgorithm")].GetNParam(0,HKI_ENCRYPT_ALGORITHM_LAST_ITEM,0);
}

void CConfigHKI::store(CONFIG_SECTION &Config)const
{
	//圧縮レベル
	Config.Data[_T("CompressLevel")]=CompressLevel;
	//暗号化アルゴリズム
	Config.Data[_T("EncryptAlgorithm")]=EncryptAlgorithm;
}

void CConfigHKI::load(CConfigManager &ConfMan)
{
	load(ConfMan.GetSection(_T("HKI")));
}

void CConfigHKI::store(CConfigManager &ConfMan)const
{
	store(ConfMan.GetSection(_T("HKI")));
}
