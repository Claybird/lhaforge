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

#pragma once

//�����A�[�J�C�o�v���W�F�N�g�d�l�̃t�@�C������
#define FA_RDONLY		0x01			// �������ݕی쑮��
#define FA_HIDDEN		0x02			// �B������
#define FA_SYSTEM		0x04			// �V�X�e������
#define FA_LABEL		0x08			// �{�����[���E���x��
#define FA_DIREC		0x10			// �f�B���N�g��
#define FA_ARCH			0x20			// �A�[�J�C�u����
#define FA_UNKNOWN		0x40			// �s���ȑ���(LhaForge�̓Ǝ��g��)


//�t�H���_�̎��ʕ�����(�g���q)
const LPCTSTR FOLDER_EXTENSION_STRING=_T("***");

//�A�[�J�C�u���̃t�@�C��/�t�H���_�̃G���g����ێ�
struct ARCHIVE_ENTRY_INFO{	//�t�@�C���A�C�e�����ێ�
	virtual ~ARCHIVE_ENTRY_INFO(){}

	CString			strFullPath;	//�i�[���ꂽ�Ƃ��̖��O
	CString			strExt;			//�t�@�C���g���q
	int				nAttribute;		//����;�������t�H���_���ǂ����Ȃǂ̏��
	CString			strMethod;		//���k���\�b�h
	WORD			wRatio;			//���k��
	DWORD			dwCRC;			//CRC
	LARGE_INTEGER	llOriginalSize;		//�i�[�t�@�C���̈��k�O�̃T�C�Y(�f�B���N�g���Ȃ�A���ɓ����Ă���t�@�C���T�C�Y�̍��v)
	LARGE_INTEGER	llCompressedSize;	//�i�[�t�@�C���̈��k��̃T�C�Y(�f�B���N�g���Ȃ�A���ɓ����Ă���t�@�C���T�C�Y�̍��v)
	FILETIME		cFileTime;		//�i�[�t�@�C���ŏI�X�V����

	bool bSafe;
};

struct ARCHIVE_ENTRY_INFO_TREE:public ARCHIVE_ENTRY_INFO{
	virtual ~ARCHIVE_ENTRY_INFO_TREE(){}
	typedef std::hash_map<stdString,ARCHIVE_ENTRY_INFO_TREE*> DICT;
	std::vector<ARCHIVE_ENTRY_INFO_TREE*> childrenArray;
	DICT					childrenDict;
	ARCHIVE_ENTRY_INFO_TREE	*lpParent;
	CString					strTitle;
	bool					bDir;			//�f�B���N�g�����ǂ���

	size_t GetNumChildren()const{return childrenArray.size();}
	ARCHIVE_ENTRY_INFO_TREE* GetChild(size_t idx)const;
	ARCHIVE_ENTRY_INFO_TREE* GetChild(LPCTSTR lpName)const;
	void Clear(){
		lpParent=NULL;
		childrenArray.clear();
		childrenDict.clear();

		nAttribute=-1;
		wRatio=0xFFFF;
		dwCRC=-1;
		llOriginalSize.HighPart=-1;
		llOriginalSize.LowPart=-1;
		llCompressedSize.HighPart=-1;
		llCompressedSize.LowPart=-1;
		cFileTime.dwLowDateTime=-1;
		cFileTime.dwHighDateTime=-1;
		bSafe=true;
	}

	//�����ȉ��̃t�@�C�����
	void EnumFiles(std::list<CString> &rFileList)const;
};

//���[�g����݂Ď����܂ł̃p�X���擾
void ArcEntryInfoTree_GetNodePathRelative(const ARCHIVE_ENTRY_INFO_TREE* lpDir,const ARCHIVE_ENTRY_INFO_TREE* lpBase,CString &strPath);
