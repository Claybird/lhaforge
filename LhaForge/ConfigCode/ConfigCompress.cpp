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
#include "../ArchiverCode/arc_interface.h"
#include "ConfigManager.h"
#include "ConfigCompress.h"
#include "../Utilities/FileOperation.h"

void CConfigCompress::load(CONFIG_SECTION &Config)
{
	//�o�͐�̎��
	OutputDirType=(OUTPUT_TO)Config.Data[_T("OutputDirType")].GetNParam(0,OUTPUT_TO_LAST_ITEM,OUTPUT_TO_DESKTOP);
	//�o�͐�̃p�X
	CString Buffer=Config.Data[_T("OutputDir")];
	if(!Buffer.IsEmpty()){
		UtilGetCompletePathName(OutputDir,Buffer);
	}else{
		OutputDir=_T("");
	}
	//���k��t�H���_���J�����ǂ���
	OpenDir=Config.Data[_T("OpenFolder")].GetNParam(TRUE);

	//�o�̓t�@�C�������w�肷�邩�ǂ���
	SpecifyOutputFilename=Config.Data[_T("SpecifyName")].GetNParam(FALSE);

	//�����Ɉ��k����t�@�C�����𐧌�����
	LimitCompressFileCount=Config.Data[_T("LimitCompressFileCount")].GetNParam(FALSE);

	//�����Ɉ��k����t�@�C�����̏��
	MaxCompressFileCount=max(1,(int)Config.Data[_T("MaxCompressFileCount")]);

	//�f�t�H���g���k�p�����[�^���g�p����Ȃ�true
	UseDefaultParameter=Config.Data[_T("UseDefaultParameter")].GetNParam(FALSE);

	//�f�t�H���g���k�p�����[�^(�`���w��)
	DefaultType=(PARAMETER_TYPE)Config.Data[_T("DefaultType")].GetNParam(0,PARAMETER_LAST_ITEM,PARAMETER_UNDEFINED);

	//�f�t�H���g���k�p�����[�^�̃I�v�V����
	DefaultOptions=Config.Data[_T("DefaultOptions")].GetNParam(0);

	//B2E���k�̏��
	DefaultB2EFormat=Config.Data[_T("DefaultB2EFormat")];
	DefaultB2EMethod=Config.Data[_T("DefaultB2EMethod")];

	//����Ɉ��k�ł����t�@�C�����폜
	DeleteAfterCompress=Config.Data[_T("DeleteAfterCompress")].GetNParam(FALSE);
	//���k��t�@�C�������ݔ��Ɉړ�
	MoveToRecycleBin=Config.Data[_T("MoveToRecycleBin")].GetNParam(TRUE);
	//�m�F�����ɍ폜/���ݔ��Ɉړ�
	DeleteNoConfirm=Config.Data[_T("DeleteNoConfirm")].GetNParam(FALSE);
	//���폈�����m�F�ł��Ȃ��`���ł��폜
	ForceDelete=Config.Data[_T("ForceDelete")].GetNParam(FALSE);

	//�u�t�H���_��艺�̃t�@�C�������k�v
	IgnoreTopDirectory=Config.Data[_T("IgnoreTopDirectory")].GetNParam(FALSE);
}

void CConfigCompress::store(CONFIG_SECTION &Config)const
{
	//�o�͐�̎��
	Config.Data[_T("OutputDirType")]=OutputDirType;
	//�o�͐�̃p�X
	Config.Data[_T("OutputDir")]=OutputDir;
	//���k��t�H���_���J�����ǂ���
	Config.Data[_T("OpenFolder")]=OpenDir;

	//�o�̓t�@�C�������w�肷�邩�ǂ���
	Config.Data[_T("SpecifyName")]=SpecifyOutputFilename;

	//�����Ɉ��k����t�@�C�����𐧌�����
	Config.Data[_T("LimitCompressFileCount")]=LimitCompressFileCount;

	//�����Ɉ��k����t�@�C�����̏��
	Config.Data[_T("MaxCompressFileCount")]=MaxCompressFileCount;

	//�f�t�H���g���k�p�����[�^���g�p����Ȃ�true
	Config.Data[_T("UseDefaultParameter")]=UseDefaultParameter;

	//�f�t�H���g���k�p�����[�^(�`���w��)
	Config.Data[_T("DefaultType")]=DefaultType;

	//�f�t�H���g���k�p�����[�^�̃I�v�V����
	Config.Data[_T("DefaultOptions")]=DefaultOptions;

	//B2E���k�̏��
	Config.Data[_T("DefaultB2EFormat")]=DefaultB2EFormat;
	Config.Data[_T("DefaultB2EMethod")]=DefaultB2EMethod;

	//����Ɉ��k�ł����t�@�C�����폜
	Config.Data[_T("DeleteAfterCompress")]=DeleteAfterCompress;
	//���k��t�@�C�������ݔ��Ɉړ�
	Config.Data[_T("MoveToRecycleBin")]=MoveToRecycleBin;
	//�m�F�����ɍ폜/���ݔ��Ɉړ�
	Config.Data[_T("DeleteNoConfirm")]=DeleteNoConfirm;
	//���폈�����m�F�ł��Ȃ��`���ł��폜
	Config.Data[_T("ForceDelete")]=ForceDelete;

	//�u�t�H���_��艺�̃t�@�C�������k�v
	Config.Data[_T("IgnoreTopDirectory")]=IgnoreTopDirectory;
}

void CConfigCompress::load(CConfigManager &ConfMan)
{
	load(ConfMan.GetSection(_T("Compress")));
}

void CConfigCompress::store(CConfigManager &ConfMan)const
{
	store(ConfMan.GetSection(_T("Compress")));
}
