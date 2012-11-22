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
	LOAD_ENTRY(YZ1);
	LOAD_ENTRY(UNARJ);
	LOAD_ENTRY(UNGCA);
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
	SAVE_ENTRY(YZ1);
	SAVE_ENTRY(UNARJ);
	SAVE_ENTRY(UNGCA);
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
