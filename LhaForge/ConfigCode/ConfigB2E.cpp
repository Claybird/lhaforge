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
#include "../ArchiverCode/ArchiverB2E.h"
#include "ConfigManager.h"
#include "ConfigB2E.h"

void CConfigB2E::load(CONFIG_SECTION &Config)
{
	//B2E32.dll��L���ɂ��邩�ǂ���
	EnableDLL=Config.Data[_T("EnableDLL")].GetNParam(FALSE);
	//B2E�֌W�̃��j���[��L���ɂ��邩�ǂ���
	EnableShellMenu=Config.Data[_T("EnableShellMenu")].GetNParam(FALSE);
	//�X�N���v�g�̃f�B���N�g��
	ScriptDirectory=Config.Data[_T("ScriptDirectory")];
	//B2E��XacRett���D��
	Priority=Config.Data[_T("Priority")].GetNParam(FALSE);
	//B2E��D�悷��g���q
	Extensions=Config.Data[_T("Extension")];
}

void CConfigB2E::store(CONFIG_SECTION &Config)const
{
	//B2E32.dll��L���ɂ��邩�ǂ���
	Config.Data[_T("EnableDLL")]=EnableDLL;
	//B2E�֌W�̃��j���[��L���ɂ��邩�ǂ���
	Config.Data[_T("EnableShellMenu")]=EnableShellMenu;
	//�X�N���v�g�̃f�B���N�g��
	Config.Data[_T("ScriptDirectory")]=ScriptDirectory;
	//B2E��XacRett���D��
	Config.Data[_T("Priority")]=Priority;
	//B2E��D�悷��g���q
	Config.Data[_T("Extension")]=Extensions;
}

void CConfigB2E::load(CConfigManager &ConfMan)
{
	load(ConfMan.GetSection(_T("B2E")));
}

void CConfigB2E::store(CConfigManager &ConfMan)const
{
	store(ConfMan.GetSection(_T("B2E")));
}
