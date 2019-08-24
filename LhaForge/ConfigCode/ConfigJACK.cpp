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
#include "../ArchiverCode/ArchiverJACK.h"
#include "ConfigManager.h"
#include "ConfigJACK.h"

void CConfigJACK::load(CONFIG_SECTION &Config)
{
	// JACK圧縮設定
	SpecifyVolumeSizeAtCompress=Config.Data[_T("SpecifyAtCompress")].GetNParam(TRUE);
	VolumeSize=Config.Data[_T("VolumeSize")].GetNParam(1423);
	if(VolumeSize<=0){
		VolumeSize=1423;
	}
}

void CConfigJACK::store(CONFIG_SECTION &Config)const
{
	// JACK圧縮設定
	Config.Data[_T("SpecifyAtCompress")]=SpecifyVolumeSizeAtCompress;
	Config.Data[_T("VolumeSize")]=VolumeSize;
}

void CConfigJACK::load(CConfigManager &ConfMan)
{
	load(ConfMan.GetSection(_T("JACK")));
}

void CConfigJACK::store(CConfigManager &ConfMan)const
{
	store(ConfMan.GetSection(_T("JACK")));
}
