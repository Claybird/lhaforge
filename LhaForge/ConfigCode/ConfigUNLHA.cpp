/*
 * Copyright (c) 2005-, Claybird
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
#include "../ArchiverCode/ArchiverUNLHA.h"
#include "ConfigManager.h"
#include "ConfigUNLHA.h"

// LZH���k�ݒ�
void CConfigLZH::load(CONFIG_SECTION &Config)
{
	//���k�`��
	CompressType=(LZH_COMPRESS_TYPE)Config.Data[_T("CompressType")].GetNParam(0,LZH_COMPRESS_TYPE_LAST_ITEM,0);
	//SFX�̐ݒ���s�����ǂ���
	ConfigSFX=Config.Data[_T("ConfigSFX")].GetNParam(FALSE);
}

void CConfigLZH::store(CONFIG_SECTION &Config)const
{
	//���k�`��
	Config.Data[_T("CompressType")]=CompressType;
	//SFX�̐ݒ���s�����ǂ���
	Config.Data[_T("ConfigSFX")]=ConfigSFX;
}

void CConfigLZH::load(CConfigManager &ConfMan)
{
	load(ConfMan.GetSection(_T("LZH")));
}

void CConfigLZH::store(CConfigManager &ConfMan)const
{
	store(ConfMan.GetSection(_T("LZH")));
}
