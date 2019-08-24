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

#pragma once

struct CConfigUpdate:public IConfigConverter{
public:
	BOOL SilentUpdate;		//�m�F����DLL�C���X�g�[�����s���Ȃ�true
	BOOL AskUpdate;			//�����I�ɃA�b�v�f�[�g���m�F���邩
	int Interval;			//�A�b�v�f�[�g�̎���
	time_t LastTime;		//�ŏI�A�b�v�f�[�g�m�F����
	CString strDLLPath;		//DLL���C���X�g�[�����ꂽ�ꏊ
protected:
	virtual void load(CONFIG_SECTION&);	//�ݒ��CONFIG_SECTION����ǂݍ���
	virtual void store(CONFIG_SECTION&)const;	//�ݒ��CONFIG_SECTION�ɏ�������
	void loadUpdate(CONFIG_SECTION&,CONFIG_SECTION&);
	void storeUpdate(CONFIG_SECTION&)const;
	void loadCaldixConf(CONFIG_SECTION&);
public:
	virtual ~CConfigUpdate(){}
	virtual void load(CConfigManager&);
	virtual void store(CConfigManager&)const;
};
