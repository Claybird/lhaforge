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
#include "../ArchiverCode/Archiver7ZIP.h"
#include "ConfigManager.h"
#include "Config7Z.h"
#include "../Dialogs/SevenZipVolumeSizeDlg.h"

void CConfig7Z::load(CONFIG_SECTION &Config)
{
	//�v���Z�b�g���g�����ǂ���
	UsePreset=Config.Data[_T("UsePreset")].GetNParam(TRUE);
	//���k�`��
	CompressType=(SEVEN_ZIP_COMPRESS_TYPE)Config.Data[_T("CompressType")].GetNParam(0,SEVEN_ZIP_COMPRESS_TYPE_LAST_ITEM,0);
	//���k���x��
	CompressLevel=(SEVEN_ZIP_COMPRESS_LEVEL)Config.Data[_T("CompressLevel")].GetNParam(0,SEVEN_ZIP_COMPRESS_LEVEL_LAST_ITEM,0);
	//LZMA���k���[�h
	LZMA_Mode=(SEVEN_ZIP_LZMA_MODE)Config.Data[_T("LZMAMode")].GetNParam(0,SEVEN_ZIP_LZMA_MODE_LAST_ITEM,0);
	//�\���b�h���k
	SolidMode=Config.Data[_T("SolidMode")].GetNParam(TRUE);
	//�w�b�_���k
	HeaderCompression=Config.Data[_T("HeaderCompression")].GetNParam(TRUE);
	//�w�b�_���S���k
	//FullHeaderCompression=Config.Data[_T("HeaderFullCompression")].GetNParam(TRUE);
	//�w�b�_�Í���
	HeaderEncryption=Config.Data[_T("HeaderEncryption")].GetNParam(FALSE);

	//�㋉�ݒ�
	//PPMd�̃��f���T�C�Y���w�肷�邩�ǂ���
	SpecifyPPMdModelSize=Config.Data[_T("SpecifyPPMdModelSize")].GetNParam(FALSE);
	//PPMd�̃��f���T�C�Y
	PPMdModelSize=Config.Data[_T("PPMdModelSize")].GetNParam(SEVEN_ZIP_PPMD_MODEL_SIZE_LOWEST,SEVEN_ZIP_PPMD_MODEL_SIZE_HIGHEST,6);

	//�����T�C�Y�����炩���ߎw��
	SpecifySplitSize = Config.Data[_T("SpecifySplitSize")].GetNParam(FALSE);
	SplitSize = Config.Data[_T("SplitSize")].GetNParam(1,INT_MAX,10);
	SplitSizeUnit = Config.Data[_T("SplitSizeUnit")].GetNParam(0,ZIP_VOLUME_UNIT_MAX_NUM,0);
}

void CConfig7Z::store(CONFIG_SECTION &Config)const
{
	//�v���Z�b�g���g�����ǂ���
	Config.Data[_T("UsePreset")]=UsePreset;
	//���k�`��
	Config.Data[_T("CompressType")]=CompressType;
	//���k���x��
	Config.Data[_T("CompressLevel")]=CompressLevel;
	//LZMA���k���[�h
	Config.Data[_T("LZMAMode")]=LZMA_Mode;
	//�\���b�h���k
	Config.Data[_T("SolidMode")]=SolidMode;
	//�w�b�_���k
	Config.Data[_T("HeaderCompression")]=HeaderCompression;
	//�w�b�_���S���k
	//Config.Data[_T("HeaderFullCompression")]=FullHeaderCompression;
	//�w�b�_�Í���
	Config.Data[_T("HeaderEncryption")]=HeaderEncryption;

	//�㋉�ݒ�
	//PPMd�̃��f���T�C�Y���w�肷�邩�ǂ���
	Config.Data[_T("SpecifyPPMdModelSize")]=SpecifyPPMdModelSize;
	//PPMd�̃��f���T�C�Y
	Config.Data[_T("PPMdModelSize")]=PPMdModelSize;

	//�����T�C�Y�����炩���ߎw��
	Config.Data[_T("SpecifySplitSize")] = SpecifySplitSize;
	Config.Data[_T("SplitSize")] = SplitSize;
	Config.Data[_T("SplitSizeUnit")] = SplitSizeUnit;

	// �p�X���[�h�֘A;���݂̓p�X���[�h�����p�R�[�h�̂�
	//�w�肳�ꂽ�p�X���[�h�������I�ɍ폜
	Config.Data.erase(_T("UseFixedPassword"));
	Config.Data.erase(_T("PasswordLength"));
	Config.Data.erase(_T("Password"));
}


void CConfig7Z::load(CConfigManager &ConfMan)
{
	load(ConfMan.GetSection(_T("7Z")));
}

void CConfig7Z::store(CConfigManager &ConfMan)const
{
	store(ConfMan.GetSection(_T("7Z")));
	UtilDumpFlatConfig(ConfMan.GetSection(_T("7Z")).Data);
}

