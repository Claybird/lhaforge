/*
 * Copyright (c) 2005-2012, Claybird
 * All rights reserved.

 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:

 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Claybird nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.

 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
 * THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 */

#include "stdafx.h"
#include "../ArchiverCode/ArchiverJACK.h"
#include "ConfigManager.h"
#include "ConfigJACK.h"

void CConfigJACK::load(CONFIG_SECTION &Config)
{
	// JACKà≥èkê›íË
	SpecifyVolumeSizeAtCompress=Config.Data[_T("SpecifyAtCompress")].GetNParam(TRUE);
	VolumeSize=Config.Data[_T("VolumeSize")].GetNParam(1423);
	if(VolumeSize<=0){
		VolumeSize=1423;
	}
}

void CConfigJACK::store(CONFIG_SECTION &Config)const
{
	// JACKà≥èkê›íË
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
