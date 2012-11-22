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
#include "Dlg_extract_general.h"
#include "../../Dialogs/LFFolderDialog.h"


//==================
// �𓀈�ʐݒ���
//==================
LRESULT CConfigDlgExtractGeneral::OnInitDialog(HWND hWnd, LPARAM lParam)
{
	// ���b�Z�[�W���[�v�Ƀ��b�Z�[�W�t�B���^�ƃA�C�h���n���h����ǉ�
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	pLoop->AddMessageFilter(this);

	//------------------
	// �𓀏o�͐�^�C�v
	//------------------
	Radio_ExtractTo[OUTPUT_TO_DESKTOP]=GetDlgItem(IDC_RADIO_EXTRACT_TO_DESKTOP);
	Radio_ExtractTo[OUTPUT_TO_SAME_DIR]=GetDlgItem(IDC_RADIO_EXTRACT_TO_SAME_DIR);
	Radio_ExtractTo[OUTPUT_TO_SPECIFIC_DIR]=GetDlgItem(IDC_RADIO_EXTRACT_TO_SPECIFIC_DIR);
	Radio_ExtractTo[OUTPUT_TO_ALWAYS_ASK_WHERE]=GetDlgItem(IDC_RADIO_EXTRACT_TO_ALWAYS_ASK_WHERE);

	Radio_ExtractTo[m_Config.OutputDirType].SetCheck(1);

	//----------------------------------------------------
	// �o�͐�t�H���_�̃p�X���G�f�B�b�g�R���g���[���ɐݒ�
	//----------------------------------------------------
	Edit_ExtractOutputDirPath=GetDlgItem(IDC_EDIT_EXTRACT_TO_SPECIFIC_DIR);
	Edit_ExtractOutputDirPath.SetLimitText(_MAX_PATH);
	Edit_ExtractOutputDirPath.SetWindowText(m_Config.OutputDir);

	Button_ExtractToFolder=GetDlgItem(IDC_BUTTON_EXTRACT_BROWSE_FOLDER);

	//�o�͐���w�肷�邽�߂̃{�^���ƃG�f�B�b�g�R���g���[���̗L��������؂�ւ�
	bool bActive=(OUTPUT_TO_SPECIFIC_DIR==m_Config.OutputDirType);
	Edit_ExtractOutputDirPath.EnableWindow(bActive);
	Button_ExtractToFolder.EnableWindow(bActive);

	//--------------------------------------
	// �o�͐�̃t�H���_���d�ɂ��邩�ǂ���
	//--------------------------------------
	Radio_CreateDir[CREATE_OUTPUT_DIR_ALWAYS]=GetDlgItem(IDC_RADIO_CREATE_FOLDER);
	Radio_CreateDir[CREATE_OUTPUT_DIR_SINGLE]=GetDlgItem(IDC_RADIO_CREATE_SINGLE_FOLDER);
	Radio_CreateDir[CREATE_OUTPUT_DIR_NEVER]=GetDlgItem(IDC_RADIO_CREATE_NO_FOLDER);

	Radio_CreateDir[m_Config.CreateDir].SetCheck(1);

	//�f�B���N�g�����쐬���Ȃ��ꍇ�ɂ́u�L������菜���v�Ɓu�t�@�C����̎��ɂ̓t�H���_�����Ȃ��v�𖳌��ɂ���
	bActive=(CREATE_OUTPUT_DIR_NEVER!=m_Config.CreateDir);
	::EnableWindow(GetDlgItem(IDC_CHECK_CREATE_NO_FOLDER_IF_SINGLE_FILE_ONLY),bActive);
	::EnableWindow(GetDlgItem(IDC_CHECK_REMOVE_SYMBOL_AND_NUMBER),bActive);


	//--------------------------------
	// �����ɉ𓀂���t�@�C�����̏��
	//--------------------------------
	UpDown_MaxExtractFileCount=GetDlgItem(IDC_SPIN_MAX_EXTRACT_FILECOUNT);

	UpDown_MaxExtractFileCount.SetPos(m_Config.MaxExtractFileCount);
	UpDown_MaxExtractFileCount.SetRange(1,32767);

	UpDown_MaxExtractFileCount.EnableWindow(m_Config.LimitExtractFileCount);
	::EnableWindow(GetDlgItem(IDC_EDIT_MAX_EXTRACT_FILECOUNT),m_Config.LimitExtractFileCount);

	//�u�����ɉ𓀂���t�@�C�����𐧌�����v�`�F�b�N�{�b�N�X
	Check_LimitExtractFileCount=GetDlgItem(IDC_CHECK_LIMIT_EXTRACT_FILECOUNT);

	//----------------------------------
	// �𓀌㈳�k�t�@�C�����폜����@�\
	//----------------------------------
	//�u�𓀌㈳�k�t�@�C�����폜����v�`�F�b�N�{�b�N�X
	Check_DeleteFileAfterExtract=GetDlgItem(IDC_CHECK_DELETE_ARCHIVE_AFTER_EXTRACT);
	//�u���ݔ��ֈړ�����v�`�F�b�N�{�b�N�X�̗L������
	::EnableWindow(GetDlgItem(IDC_CHECK_MOVETO_RECYCLE_BIN),m_Config.DeleteArchiveAfterExtract);
	//�u�m�F���Ȃ��v�̗L������
	::EnableWindow(GetDlgItem(IDC_CHECK_DELETE_NOCONFIRM),m_Config.DeleteArchiveAfterExtract);
	//�����폜�̗L������
	::EnableWindow(GetDlgItem(IDC_CHECK_FORCE_DELETE),m_Config.DeleteArchiveAfterExtract);
	//�}���`�{�����[���폜�̗L������
	::EnableWindow(GetDlgItem(IDC_CHECK_DELETE_MULTIVOLUME),m_Config.DeleteArchiveAfterExtract);

	//DDX���ݒ�
	DoDataExchange(FALSE);
	return TRUE;
}

LRESULT CConfigDlgExtractGeneral::OnApply()
{
//===============================
// �ݒ��ConfigManager�ɏ����߂�
//===============================
	//------------------
	// �𓀏o�͐�^�C�v
	//------------------
	for(int Type=0;Type<COUNTOF(Radio_ExtractTo);Type++){
		if(Radio_ExtractTo[Type].GetCheck()){
			m_Config.OutputDirType=(OUTPUT_TO)Type;
			break;
		}
	}
	//----------------------
	// �o�͐�t�H���_�̃p�X
	//----------------------
	Edit_ExtractOutputDirPath.GetWindowText(m_Config.OutputDir);

	//--------------------------------------
	// �o�͐�̃t�H���_���d�ɂ��邩�ǂ���
	//--------------------------------------
	for(int Type=0;Type<COUNTOF(Radio_CreateDir);Type++){
		if(Radio_CreateDir[Type].GetCheck()){
			m_Config.CreateDir=(CREATE_OUTPUT_DIR)Type;
			break;
		}
	}
	//--------------------------------
	// �����ɉ𓀂���t�@�C�����̏��
	//--------------------------------
	m_Config.MaxExtractFileCount=UpDown_MaxExtractFileCount.GetPos();

	//---------------
	// DDX�f�[�^�擾
	//---------------
	if(!DoDataExchange(TRUE)){
		return FALSE;
	}

	return TRUE;
}

LRESULT CConfigDlgExtractGeneral::OnRadioExtractTo(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if(BN_CLICKED==wNotifyCode){
		bool bActive=(0!=Radio_ExtractTo[OUTPUT_TO_SPECIFIC_DIR].GetCheck());
		Edit_ExtractOutputDirPath.EnableWindow(bActive);
		Button_ExtractToFolder.EnableWindow(bActive);
	}
	return 0;
}

LRESULT CConfigDlgExtractGeneral::OnBrowseFolder(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if(BN_CLICKED==wNotifyCode){
		TCHAR FolderPath[_MAX_PATH+1];
		FILL_ZERO(FolderPath);

		Edit_ExtractOutputDirPath.GetWindowText(FolderPath,_MAX_PATH);

		CString title(MAKEINTRESOURCE(IDS_INPUT_TARGET_FOLDER));
		CLFFolderDialog dlg(m_hWnd,title,BIF_RETURNONLYFSDIRS|BIF_NEWDIALOGSTYLE);
		dlg.SetInitialFolder(FolderPath);
		if(IDOK==dlg.DoModal()){
			_tcsncpy_s(FolderPath,dlg.GetFolderPath(),_MAX_PATH);
			Edit_ExtractOutputDirPath.SetWindowText(FolderPath);
		}
	}
	return 0;
}


LRESULT CConfigDlgExtractGeneral::OnCheckLimitExtractFileCount(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if(BN_CLICKED==wNotifyCode){
		BOOL State=Check_LimitExtractFileCount.GetCheck();
		UpDown_MaxExtractFileCount.EnableWindow(State);
		::EnableWindow(GetDlgItem(IDC_EDIT_MAX_EXTRACT_FILECOUNT),State);
	}
	return 0;
}

LRESULT CConfigDlgExtractGeneral::OnRadioCreateDirectory(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if(BN_CLICKED==wNotifyCode){
		bool bActive=!Radio_CreateDir[CREATE_OUTPUT_DIR_NEVER].GetCheck();
		::EnableWindow(GetDlgItem(IDC_CHECK_CREATE_NO_FOLDER_IF_SINGLE_FILE_ONLY),bActive);
		::EnableWindow(GetDlgItem(IDC_CHECK_REMOVE_SYMBOL_AND_NUMBER),bActive);
	}
	return 0;
}

//�𓀌�폜�̐ݒ�ɍ��킹�ă`�F�b�N�{�b�N�X�̗L�����������߂�
LRESULT CConfigDlgExtractGeneral::OnCheckDeleteArchive(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if(BN_CLICKED==wNotifyCode){
		BOOL State=Check_DeleteFileAfterExtract.GetCheck();
		::EnableWindow(GetDlgItem(IDC_CHECK_MOVETO_RECYCLE_BIN),State);
		::EnableWindow(GetDlgItem(IDC_CHECK_DELETE_NOCONFIRM),State);
		::EnableWindow(GetDlgItem(IDC_CHECK_FORCE_DELETE),State);
		::EnableWindow(GetDlgItem(IDC_CHECK_DELETE_MULTIVOLUME),State);
	}
	return 0;
}
