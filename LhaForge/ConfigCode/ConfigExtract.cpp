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
#include "ConfigExtract.h"
#include "../Utilities/FileOperation.h"

void CConfigExtract::load(CONFIG_SECTION &Config)
{
	OutputDirType=(OUTPUT_TO)Config.Data[_T("OutputDirType")].GetNParam(0,OUTPUT_TO_LAST_ITEM,OUTPUT_TO_DESKTOP);

	CString Buffer=Config.Data[_T("OutputDir")];
	if(!Buffer.IsEmpty()){
		UtilGetCompletePathName(OutputDir,Buffer);
	}else{
		OutputDir=_T("");
	}

	//�𓀌�t�H���_���J�����ǂ���
	OpenDir=Config.Data[_T("OpenFolder")].GetNParam(TRUE);

	//�𓀎��t�H���_���d�ɂ��邩�ǂ���
	CreateDir=(CREATE_OUTPUT_DIR)Config.Data[_T("CreateDir")].GetNParam(0,CREATE_OUTPUT_DIR_LAST_ITEM,CREATE_OUTPUT_DIR_ALWAYS);

	//�𓀎��t�@�C�����m�F�����ɏ㏑�����邩�ǂ���
	ForceOverwrite=Config.Data[_T("ForceOverwrite")].GetNParam(FALSE);

	//�𓀎��t�H���_�����琔���ƋL�����폜����
	RemoveSymbolAndNumber=Config.Data[_T("RemoveSymbolAndNumber")].GetNParam(FALSE);

	//�𓀎��t�@�C���E�t�H���_��������̎��t�H���_�����Ȃ�
	CreateNoFolderIfSingleFileOnly=Config.Data[_T("CreateNoFolderIfSingleFileOnly")].GetNParam(FALSE);

	//�����ɉ𓀂���t�@�C�����𐧌�����
	LimitExtractFileCount=Config.Data[_T("LimitExtractFileCount")].GetNParam(FALSE);

	//�����ɉ𓀂���t�@�C�����̏��
	MaxExtractFileCount=max(1,Config.Data[_T("MaxExtractFileCount")].GetNParam(1));

	//����ɉ𓀂ł������k�t�@�C�����폜
	DeleteArchiveAfterExtract=Config.Data[_T("DeleteArchiveAfterExtract")].GetNParam(FALSE);

	//�𓀌�t�@�C�������ݔ��Ɉړ�
	MoveToRecycleBin=Config.Data[_T("MoveToRecycleBin")].GetNParam(TRUE);

	//�m�F�����ɃA�[�J�C�u���폜/���ݔ��Ɉړ�
	DeleteNoConfirm=Config.Data[_T("DeleteNoConfirm")].GetNParam(FALSE);

	//�𓀃G���[�����m�ł��Ȃ��ꍇ���폜
	ForceDelete=Config.Data[_T("ForceDelete")].GetNParam(FALSE);

	//�}���`�{�����[�����폜
	DeleteMultiVolume=Config.Data[_T("DeleteMultiVolume")].GetNParam(FALSE);

	//�p�X���[�h���͉񐔂��ŏ��ɂ���Ȃ�TRUE
	MinimumPasswordRequest=Config.Data[_T("MinimumPasswordRequest")].GetNParam(FALSE);

	//�𓀑Ώۂ���O���g���q
	if(has_key(Config.Data,_T("DenyExt"))){
		DenyExt=Config.Data[_T("DenyExt")];
	}else{
		DenyExt.LoadString(IDS_DENYEXT_DEFAULT);
	}
}

void CConfigExtract::store(CONFIG_SECTION &Config)const
{
	Config.Data[_T("OutputDirType")]=OutputDirType;

	Config.Data[_T("OutputDir")]=OutputDir;

	//�𓀌�t�H���_���J�����ǂ���
	Config.Data[_T("OpenFolder")]=OpenDir;

	//�𓀎��t�H���_���d�ɂ��邩�ǂ���
	Config.Data[_T("CreateDir")]=CreateDir;

	//�𓀎��t�@�C�����m�F�����ɏ㏑�����邩�ǂ���
	Config.Data[_T("ForceOverwrite")]=ForceOverwrite;

	//�𓀎��t�H���_�����琔���ƋL�����폜����
	Config.Data[_T("RemoveSymbolAndNumber")]=RemoveSymbolAndNumber;

	//�𓀎��t�@�C���E�t�H���_��������̎��t�H���_�����Ȃ�
	Config.Data[_T("CreateNoFolderIfSingleFileOnly")]=CreateNoFolderIfSingleFileOnly;

	//�����ɉ𓀂���t�@�C�����𐧌�����
	Config.Data[_T("LimitExtractFileCount")]=LimitExtractFileCount;

	//�����ɉ𓀂���t�@�C�����̏��
	Config.Data[_T("MaxExtractFileCount")]=MaxExtractFileCount;

	//����ɉ𓀂ł������k�t�@�C�����폜
	Config.Data[_T("DeleteArchiveAfterExtract")]=DeleteArchiveAfterExtract;

	//�𓀌�t�@�C�������ݔ��Ɉړ�
	Config.Data[_T("MoveToRecycleBin")]=MoveToRecycleBin;

	//�m�F�����ɃA�[�J�C�u���폜/���ݔ��Ɉړ�
	Config.Data[_T("DeleteNoConfirm")]=DeleteNoConfirm;

	//�𓀃G���[�����m�ł��Ȃ��ꍇ���폜
	Config.Data[_T("ForceDelete")]=ForceDelete;

	//�}���`�{�����[�����폜
	Config.Data[_T("DeleteMultiVolume")]=DeleteMultiVolume;

	//�p�X���[�h���͉񐔂��ŏ��ɂ���Ȃ�TRUE
	Config.Data[_T("MinimumPasswordRequest")]=MinimumPasswordRequest;

	//�𓀑Ώۂ���O���g���q
	Config.Data[_T("DenyExt")]=DenyExt;
}

void CConfigExtract::load(CConfigManager &ConfMan)
{
	load(ConfMan.GetSection(_T("Extract")));
}

void CConfigExtract::store(CConfigManager &ConfMan)const
{
	store(ConfMan.GetSection(_T("Extract")));
}
