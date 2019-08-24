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
#include "ConfigManager.h"
#include "ConfigUpdate.h"

void CConfigUpdate::load(CONFIG_SECTION &Config){}
void CConfigUpdate::store(CONFIG_SECTION &Config)const{}

void CConfigUpdate::loadUpdate(CONFIG_SECTION &Config,CONFIG_SECTION &CaldixConfig)
{
	SilentUpdate=Config.Data[_T("SilentUpdate")].GetNParam(FALSE);

	AskUpdate=Config.Data[_T("AskUpdate")].GetNParam(TRUE);

	Interval=max(0,Config.Data[_T("Interval")].GetNParam(21));

	//�ŏI�X�V�����̍X�V��LFCaldix�ɔC����
	LastTime=(time_t)max(0,(int)CaldixConfig.Data[_T("LastTime")]);
	//���[�U�[�p�t�@�C���ɏ������܂ꂽ�ŏI�X�V�������`�F�b�N
	LastTime=max(LastTime,(time_t)(int)Config.Data[_T("LastTime")]);
}

void CConfigUpdate::storeUpdate(CONFIG_SECTION &Config)const
{
	//�X�V������Caldix���ŋL�^
	//�X�V�L�����Z������LFCaldix.ini�̓�����i�߂邪�A
	//�������݂Ɏ��s����ꍇ��z�肵�ă��[�U�[�ʐݒ�t�@�C���ɂ��ݒ����������
	Config.Data[_T("SilentUpdate")]=SilentUpdate;
	Config.Data[_T("AskUpdate")]=AskUpdate;
	Config.Data[_T("Interval")]=Interval;
}

void CConfigUpdate::loadCaldixConf(CONFIG_SECTION &CaldixConfig)
{
	//DLL�C���X�g�[����̎擾
	strDLLPath=CaldixConfig.Data[_T("dll")];
}

void CConfigUpdate::load(CConfigManager &ConfMan)
{
	loadUpdate(ConfMan.GetSection(_T("Update")),ConfMan.GetCaldixSection(_T("Update")));
	loadCaldixConf(ConfMan.GetCaldixSection(_T("conf")));
}

void CConfigUpdate::store(CConfigManager &ConfMan)const
{
	storeUpdate(ConfMan.GetSection(_T("Update")));
}
