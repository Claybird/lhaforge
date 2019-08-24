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

struct CConfigFileListWindow:public IConfigConverter{
public:
	int Width;					//�E�B���h�E�̕�
	int Height;					//�E�B���h�E�̍���
	int TreeWidth;				//�c���[�r���[�̕�

	int SortColumn;				//�ǂ̃J�����Ń\�[�g���邩
	BOOL SortDescending;		//�\�[�g�̏����E�~��
	int ListStyle;				//���X�g�r���[�̌`��

	BOOL StoreSetting;			//�E�B���h�E�̐ݒ��ۑ�

	int WindowPos_x;			//�E�B���h�E�̍��W(x)
	int WindowPos_y;			//�E�B���h�E�̍��W(y)
	BOOL StoreWindowPosition;	//�E�B���h�E�̈ʒu��ۑ�����

	BOOL IgnoreMeaninglessPath;	//�󔒂�.�݂̂̃p�X�w��͖�������
	FILELISTMODE FileListMode;	//�t�@�C���\���̊K�w���[�h
	BOOL ExpandTree;			//�N�����Ƀc���[�r���[��W�J���Ă���
	BOOL DisplayFileSizeInByte;	//�o�C�g�P�ʂŃt�@�C���T�C�Y��\�L����
	BOOL DisplayPathOnly;		//�t���p�X�̗��Ƀt�@�C������\�����Ȃ�
	int ColumnOrderArray[FILEINFO_ITEM_COUNT];	//���X�g�r���[�J�����̕��я�
	int ColumnWidthArray[FILEINFO_ITEM_COUNT];	//���X�g�r���[�J�����̕�
	BOOL ExitWithEscape;		//[ESC]�L�[�ŏI��
	BOOL DisableTab;			//�^�u�\�����g��Ȃ��Ȃ�TRUE
	BOOL KeepSingleInstance;	//�E�B���h�E����ɕۂȂ�TRUE
	BOOL DenyPathExt;			//%PATHEXT%�Ŏw�肳�ꂽ�t�@�C�����J���Ȃ��Ȃ�TRUE

	CString strCustomToolbarImage;	//�J�X�^���c�[���o�[�摜
	BOOL ShowToolbar;			//�c�[���o�[��\������Ȃ�TRUE
	BOOL ShowTreeView;			//�c���[�r���[��\������Ȃ�TRUE

	struct tagOpenAssoc{
		virtual ~tagOpenAssoc(){}
		CString Accept;
		CString Deny;
	}OpenAssoc;

	std::vector<CMenuCommandItem> MenuCommandArray;	//�u�v���O�����ŊJ���v�̃R�}���h
protected:
	virtual void load(CONFIG_SECTION&);	//�ݒ��CONFIG_SECTION����ǂݍ���
	virtual void store(CONFIG_SECTION&)const;	//�ݒ��CONFIG_SECTION�ɏ�������
	void loadMenuCommand(CONFIG_SECTION&,CMenuCommandItem&);
	void storeMenuCommand(CONFIG_SECTION&,const CMenuCommandItem&)const;
public:
	virtual ~CConfigFileListWindow(){}
	virtual void load(CConfigManager&);
	virtual void store(CConfigManager&)const;
};

