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
#include "ConfigManager.h"
#include "../FileListWindow/FileListModel.h"
#include "../FileListWindow/FileListFrame.h"
#include "../FileListWindow/FileListTabClient.h"
#include "ConfigFileListWindow.h"
#include "../resource.h"

void CConfigFileListWindow::load(CONFIG_SECTION &Config)
{
	//�E�B���h�E�̐ݒ��ۑ�
	StoreSetting=Config.Data[_T("StoreSetting")].GetNParam(TRUE);
	//�E�B���h�E�̕��ƍ���
	Width=Config.Data[_T("Width")].GetNParam(FILELISTWINDOW_DEFAULT_WIDTH);
	if(Width<0){
		Width=FILELISTWINDOW_DEFAULT_WIDTH;
	}

	Height=Config.Data[_T("Height")].GetNParam(FILELISTWINDOW_DEFAULT_HEIGHT);
	if(Height<0){
		Height=FILELISTWINDOW_DEFAULT_HEIGHT;
	}

	//�c���[�r���[�̕�
	TreeWidth=Config.Data[_T("TreeWidth")].GetNParam(FILELISTWINDOW_DEFAULT_TREE_WIDTH);
	if(TreeWidth<0){
		TreeWidth=175;
	}
	//���X�g�r���[�̃X�^�C��(Default:LVS_ICON)
	ListStyle=Config.Data[_T("ListStyle")].GetNParam(0);
	if(
		(LVS_LIST!=ListStyle)&&
		(LVS_REPORT!=ListStyle)&&
		(LVS_SMALLICON!=ListStyle)&&
		(LVS_ICON!=ListStyle)
	)ListStyle=LVS_ICON;

	//---------
	//�\�[�g�̐ݒ�
	//�J����
	SortColumn=Config.Data[_T("SortColumn")].GetNParam(FILEINFO_INVALID,FILEINFO_LAST_ITEM,FILEINFO_FILENAME);
	//����/�~��
	SortDescending=Config.Data[_T("Descending")].GetNParam(TRUE);
	//----------
	//�E�B���h�E�̈ʒu��ۑ�
	StoreWindowPosition=Config.Data[_T("StoreWindowPosition")].GetNParam(FALSE);
	//�E�B���h�E�̍��W
	WindowPos_x=Config.Data[_T("x")].GetNParam(0);
	WindowPos_y=Config.Data[_T("y")].GetNParam(0);
	//---------
	//���Ӗ��ȃp�X�𖳎����邩
	IgnoreMeaninglessPath=Config.Data[_T("IgnoreMeaninglessPath")].GetNParam(TRUE);
	//�K�w�\���𖳎����邩
	FileListMode=(FILELISTMODE)Config.Data[_T("FileListMode")].GetNParam(0,FILELISTMODE_LAST_ITEM,0);
	//---------
	//���߂���c���[��W�J
	ExpandTree=Config.Data[_T("ExpandTree")].GetNParam(FALSE);
	//�o�C�g�P�ʂŃt�@�C���T�C�Y��\�L
	DisplayFileSizeInByte=Config.Data[_T("DisplayFileSizeInByte")].GetNParam(FALSE);
	//[ESC]�L�[�ŏI��
	ExitWithEscape=Config.Data[_T("ExitWithEscape")].GetNParam(FALSE);
	//�^�u�\�����g��Ȃ��Ȃ�TRUE
	DisableTab=Config.Data[_T("DisableTab")].GetNParam(FALSE);
	//�E�B���h�E����ɕۂȂ�TRUE
	KeepSingleInstance=Config.Data[_T("KeepSingleInstance")].GetNParam(FALSE);

	//���X�g�r���[�J�����̕��я�
	{
		CString Buffer=Config.Data[_T("ColumnOrder")];
		if(Buffer.IsEmpty()){
			for(int i=0;i<FILEINFO_ITEM_COUNT;i++){
				//�z�񏉊���
				ColumnOrderArray[i]=i;
			}
		}else{
			//�J�����̕��я����擾
			int Index=0;
			LPCTSTR lpArrayString=Buffer;
			for(;_T('\0')!=*lpArrayString&&Index<FILEINFO_ITEM_COUNT;Index++){
				CString Temp;
				for(;;){
					if(_T(',')==*lpArrayString||_T('\0')==*lpArrayString){
						lpArrayString++;
						break;
					}else{
						Temp+=*lpArrayString;
						lpArrayString++;
					}
				}
				ColumnOrderArray[Index]=_ttoi(Temp);
				if(ColumnOrderArray[Index]<-1||ColumnOrderArray[Index]>=FILEINFO_ITEM_COUNT){
					ColumnOrderArray[Index]=Index;
				}
			}
		}
	}
	//�֘A�t���ŊJ��������/���ۂ���g���q
	if(has_key(Config.Data,_T("OpenAssocAccept"))){
		OpenAssoc.Accept=Config.Data[_T("OpenAssocAccept")];
	}else{
		OpenAssoc.Accept=CString(MAKEINTRESOURCE(IDS_FILELIST_OPENASSOC_DEFAULT_ACCEPT));
	}

	//����
	if(has_key(Config.Data,_T("OpenAssocDeny"))){
		OpenAssoc.Deny=Config.Data[_T("OpenAssocDeny")];
	}else{
		OpenAssoc.Deny=CString(MAKEINTRESOURCE(IDS_FILELIST_OPENASSOC_DEFAULT_DENY));
	}

	//%PATHEXT%�Ŏw�肳�ꂽ�t�@�C�����J���Ȃ��Ȃ�TRUE
	DenyPathExt=Config.Data[_T("DenyPathExt")].GetNParam(TRUE);

	//�J�X�^���c�[���o�[�摜
	strCustomToolbarImage=Config.Data[_T("CustomToolbarImage")];
	//�c�[���o�[�\��/��\��
	ShowToolbar=Config.Data[_T("ShowToolbar")].GetNParam(TRUE);

	//�c���[�r���[�\��/��\��
	ShowTreeView=Config.Data[_T("ShowTreeView")].GetNParam(TRUE);
}

void CConfigFileListWindow::loadMenuCommand(CONFIG_SECTION &Config,CMenuCommandItem &mci)
{
	//�u�v���O�����ŊJ���v���j���[�̃R�}���h
	//�v���O�����̃p�X
	mci.Path=Config.Data[_T("Path")];
	//�p�����[�^
	mci.Param=Config.Data[_T("Param")];
	//�f�B���N�g��
	mci.Dir=Config.Data[_T("Dir")];
	//�L���v�V����
	mci.Caption=Config.Data[_T("Caption")];
}


void CConfigFileListWindow::store(CONFIG_SECTION &Config)const
{
	//�E�B���h�E�̐ݒ��ۑ�
	Config.Data[_T("StoreSetting")]=StoreSetting;
	if(StoreSetting){
		//�E�B���h�E�̕��ƍ���
		Config.Data[_T("Width")]=Width;
		Config.Data[_T("Height")]=Height;
		//�c���[�r���[�̕�
		Config.Data[_T("TreeWidth")]=TreeWidth;
		//���X�g�r���[�̃X�^�C��
		Config.Data[_T("ListStyle")]=ListStyle;
		//---------
		//�\�[�g�̐ݒ�
		//�J����
		Config.Data[_T("SortColumn")]=SortColumn;
		//����/�~��
		Config.Data[_T("Descending")]=SortDescending;
	}
	//----------
	//�E�B���h�E�̈ʒu��ۑ�
	Config.Data[_T("StoreWindowPosition")]=StoreWindowPosition;
	if(StoreWindowPosition){
		//�E�B���h�E�̍��W
		Config.Data[_T("x")]=WindowPos_x;
		Config.Data[_T("y")]=WindowPos_y;
	}
	//---------
	//���Ӗ��ȃp�X�𖳎����邩
	Config.Data[_T("IgnoreMeaninglessPath")]=IgnoreMeaninglessPath;
	//�K�w�\���𖳎����邩
	Config.Data[_T("FileListMode")]=FileListMode;
	//---------
	//���߂���c���[��W�J
	Config.Data[_T("ExpandTree")]=ExpandTree;
	//�o�C�g�P�ʂŃt�@�C���T�C�Y��\�L
	Config.Data[_T("DisplayFileSizeInByte")]=DisplayFileSizeInByte;
	//[ESC]�L�[�ŏI��
	Config.Data[_T("ExitWithEscape")]=ExitWithEscape;
	//�^�u�\�����g��Ȃ��Ȃ�TRUE
	Config.Data[_T("DisableTab")]=DisableTab;
	//�E�B���h�E����ɕۂȂ�TRUE
	Config.Data[_T("KeepSingleInstance")]=KeepSingleInstance;

	//���X�g�r���[�J�����̕��я�
	{
		CString Buffer;
		for(int i=0;i<FILEINFO_ITEM_COUNT;i++){
			Buffer.AppendFormat(_T("%d"),ColumnOrderArray[i]);
			if(i!=FILEINFO_ITEM_COUNT-1){
				Buffer+=_T(",");
			}
		}
		Config.Data[_T("ColumnOrder")]=Buffer;
	}
	//�֘A�t���ŊJ��������/���ۂ���g���q
	//����
	Config.Data[_T("OpenAssocAccept")]=OpenAssoc.Accept;
	//����
	Config.Data[_T("OpenAssocDeny")]=OpenAssoc.Deny;

	//%PATHEXT%�Ŏw�肳�ꂽ�t�@�C�����J���Ȃ��Ȃ�TRUE
	Config.Data[_T("DenyPathExt")]=DenyPathExt;

	//�J�X�^���c�[���o�[�摜
	Config.Data[_T("CustomToolbarImage")]=strCustomToolbarImage;
	//�c�[���o�[�\��/��\��
	Config.Data[_T("ShowToolbar")]=ShowToolbar;
	//�c���[�r���[�\��/��\��
	Config.Data[_T("ShowTreeView")]=ShowTreeView;
}

void CConfigFileListWindow::storeMenuCommand(CONFIG_SECTION &Config,const CMenuCommandItem &mci)const
{
	//�v���O�����̃p�X
	Config.Data[_T("Path")]=mci.Path;
	//�p�����[�^
	Config.Data[_T("Param")]=mci.Param;
	//�f�B���N�g��
	Config.Data[_T("Dir")]=mci.Dir;
	//�L���v�V����
	Config.Data[_T("Caption")]=mci.Caption;
}

void CConfigFileListWindow::load(CConfigManager &ConfMan)
{
	load(ConfMan.GetSection(_T("FileListWindow")));

	//�u�v���O�����ŊJ���v���j���[�̃R�}���h
	//�Â����̔j��
	MenuCommandArray.clear();
	for(UINT iIndex=0;iIndex<USERAPP_MAX_NUM;iIndex++){
		CString strSectionName;
		strSectionName.Format(_T("UserApp%d"),iIndex);
		if(!ConfMan.HasSection(strSectionName)){
			break;
		}else{
			CMenuCommandItem mci;
			loadMenuCommand(ConfMan.GetSection(strSectionName),mci);
			MenuCommandArray.push_back(mci);
		}
	}
}

void CConfigFileListWindow::store(CConfigManager &ConfMan)const
{
	store(ConfMan.GetSection(_T("FileListWindow")));

	//�u�v���O�����ŊJ���v���j���[�̃R�}���h
	//---�Â��Z�N�V�����̔j��
	for(UINT iIndex=0;iIndex<USERAPP_MAX_NUM;iIndex++){
		CString strSectionName;
		strSectionName.Format(_T("UserApp%d"),iIndex);
		if(!ConfMan.HasSection(strSectionName)){
			break;
		}else{
			ConfMan.DeleteSection(strSectionName);
		}
	}
	//---�f�[�^�̏㏑��
	for(UINT iIndex=0;iIndex<MenuCommandArray.size();iIndex++){
		CString strSectionName;
		strSectionName.Format(_T("UserApp%d"),iIndex);
		storeMenuCommand(ConfMan.GetSection(strSectionName),MenuCommandArray[iIndex]);
	}
}

