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
#include "../ArchiverCode/ArchiverAISH.h"
#include "ConfigManager.h"
#include "ConfigAISH.h"

void CConfigAISH::load(CONFIG_SECTION &Config)
{
	//ISH�G���R�[�h�^�C�v
	EncodeType=(ISH_ENCODE_TYPE)Config.Data[_T("EncodeType")].GetNParam(0,ISH_ENCODE_TYPE_LAST_ITEM,0);
	//uuencode�`�F�b�N�T��
	UUEncodeChecksum=Config.Data[_T("UUEChecksum")].GetNParam(FALSE);
}

void CConfigAISH::store(CONFIG_SECTION &Config)const
{
	//ISH�G���R�[�h�^�C�v
	Config.Data[_T("EncodeType")]=EncodeType;
	//uuencode�`�F�b�N�T��
	Config.Data[_T("UUEChecksum")]=UUEncodeChecksum;
}

void CConfigAISH::load(CConfigManager &ConfMan)
{
	load(ConfMan.GetSection(_T("AISH")));
}

void CConfigAISH::store(CConfigManager &ConfMan)const
{
	store(ConfMan.GetSection(_T("AISH")));
}
