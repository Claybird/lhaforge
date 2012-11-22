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
#include "Dlg_compress_general.h"
#include "../../ArchiverCode/arc_interface.h"
#include "../../Dialogs/selectdlg.h"
#include "../../Dialogs/LFFolderDialog.h"
#include "../../compress.h"

//==================
// ���k��ʐݒ���
//==================
LRESULT CConfigDlgCompressGeneral::OnInitDialog(HWND hWnd, LPARAM lParam)
{
	// ���b�Z�[�W���[�v�Ƀ��b�Z�[�W�t�B���^�ƃA�C�h���n���h����ǉ�
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	pLoop->AddMessageFilter(this);

	//------------------
	// ���k�o�͐�^�C�v
	//------------------
	Radio_CompressTo[OUTPUT_TO_DESKTOP]=GetDlgItem(IDC_RADIO_COMPRESS_TO_DESKTOP);
	Radio_CompressTo[OUTPUT_TO_SAME_DIR]=GetDlgItem(IDC_RADIO_COMPRESS_TO_SAME_DIR);
	Radio_CompressTo[OUTPUT_TO_SPECIFIC_DIR]=GetDlgItem(IDC_RADIO_COMPRESS_TO_SPECIFIC_DIR);
	Radio_CompressTo[OUTPUT_TO_ALWAYS_ASK_WHERE]=GetDlgItem(IDC_RADIO_COMPRESS_TO_ALWAYS_ASK_WHERE);

	Radio_CompressTo[m_Config.OutputDirType].SetCheck(1);

	//----------------------------------------------------
	// �o�͐�t�H���_�̃p�X���G�f�B�b�g�R���g���[���ɐݒ�
	//----------------------------------------------------
	Edit_CompressOutputDirPath=GetDlgItem(IDC_EDIT_COMPRESS_TO_SPECIFIC_DIR);
	Edit_CompressOutputDirPath.SetLimitText(_MAX_PATH);
	Edit_CompressOutputDirPath.SetWindowText(m_Config.OutputDir);

	Button_CompressToFolder=GetDlgItem(IDC_BUTTON_COMPRESS_BROWSE_FOLDER);

	//�o�͐���w�肷�邽�߂̃{�^���ƃG�f�B�b�g�R���g���[���̗L��������؂�ւ�
	bool bActive=(OUTPUT_TO_SPECIFIC_DIR==m_Config.OutputDirType);
	Edit_CompressOutputDirPath.EnableWindow(bActive);
	Button_CompressToFolder.EnableWindow(bActive);

	//--------------------------------
	// �����Ɉ��k����t�@�C�����̏��
	//--------------------------------
	UpDown_MaxCompressFileCount=GetDlgItem(IDC_SPIN_MAX_COMPRESS_FILECOUNT);

	UpDown_MaxCompressFileCount.SetPos(m_Config.MaxCompressFileCount);
	UpDown_MaxCompressFileCount.SetRange(1,32767);

	UpDown_MaxCompressFileCount.EnableWindow(m_Config.LimitCompressFileCount);
	::EnableWindow(GetDlgItem(IDC_EDIT_MAX_COMPRESS_FILECOUNT),m_Config.LimitCompressFileCount);

	//�u�����Ɉ��k����t�@�C�����𐧌�����v�`�F�b�N�{�b�N�X
	Check_LimitCompressFileCount=GetDlgItem(IDC_CHECK_LIMIT_COMPRESS_FILECOUNT);

	//--------------------------
	// �f�t�H���g���k�p�����[�^
	//--------------------------
	Check_UseDefaultParameter=GetDlgItem(IDC_CHECK_USE_DEFAULTPARAMETER);
	Edit_DefaultParameterInfo=GetDlgItem(IDC_EDIT_DEFAULTPARAMETER);

	::EnableWindow(GetDlgItem(IDC_BUTTON_SELECT_DEFAULTPARAMETER),m_Config.UseDefaultParameter);

	SetParameterInfo();

	//----------------------------------
	// ���k�㌳�t�@�C�����폜����@�\
	//----------------------------------
	//�u�𓀌㈳�k�t�@�C�����폜����v�`�F�b�N�{�b�N�X
	Check_DeleteAfterCompress=GetDlgItem(IDC_CHECK_DELETE_AFTER_COMPRESS);
	//�u���ݔ��ֈړ�����v�`�F�b�N�{�b�N�X�̗L������
	::EnableWindow(GetDlgItem(IDC_CHECK_MOVETO_RECYCLE_BIN),m_Config.DeleteAfterCompress);
	//�u�m�F���Ȃ��v�̗L������
	::EnableWindow(GetDlgItem(IDC_CHECK_DELETE_NOCONFIRM),m_Config.DeleteAfterCompress);
	//�����폜�̗L������
	::EnableWindow(GetDlgItem(IDC_CHECK_FORCE_DELETE),m_Config.DeleteAfterCompress);

	//DDX���ݒ�
	DoDataExchange(FALSE);

	return TRUE;
}

LRESULT CConfigDlgCompressGeneral::OnApply()
{
//===============================
// �ݒ��ConfigManager�ɏ����߂�
//===============================
	//------------------
	// ���k�o�͐�^�C�v
	//------------------
	for(int Type=0;Type<COUNTOF(Radio_CompressTo);Type++){
		if(Radio_CompressTo[Type].GetCheck()){
			m_Config.OutputDirType=(OUTPUT_TO)Type;
			break;
		}
	}
	//----------------------
	// �o�͐�t�H���_�̃p�X
	//----------------------
	Edit_CompressOutputDirPath.GetWindowText(m_Config.OutputDir);

	//--------------------------------
	// �����Ɉ��k����t�@�C�����̏��
	//--------------------------------
	m_Config.MaxCompressFileCount=UpDown_MaxCompressFileCount.GetPos();

	//---------------
	// DDX�f�[�^�擾
	//---------------
	if(!DoDataExchange(TRUE)){
		return FALSE;
	}
	return TRUE;
}

LRESULT CConfigDlgCompressGeneral::OnRadioCompressTo(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if(BN_CLICKED==wNotifyCode){
		bool bActive=(0!=Radio_CompressTo[OUTPUT_TO_SPECIFIC_DIR].GetCheck());
		Edit_CompressOutputDirPath.EnableWindow(bActive);
		Button_CompressToFolder.EnableWindow(bActive);
	}
	return 0;
}

LRESULT CConfigDlgCompressGeneral::OnBrowseFolder(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if(BN_CLICKED==wNotifyCode){
		TCHAR FolderPath[_MAX_PATH+1];
		FILL_ZERO(FolderPath);

		Edit_CompressOutputDirPath.GetWindowText(FolderPath,_MAX_PATH);

		CString title(MAKEINTRESOURCE(IDS_INPUT_TARGET_FOLDER));
		CLFFolderDialog dlg(m_hWnd,title,BIF_RETURNONLYFSDIRS|BIF_NEWDIALOGSTYLE);
		dlg.SetInitialFolder(FolderPath);
		if(IDOK==dlg.DoModal()){
			_tcsncpy_s(FolderPath,dlg.GetFolderPath(),_MAX_PATH);
			Edit_CompressOutputDirPath.SetWindowText(FolderPath);
		}
	}
	return 0;
}

LRESULT CConfigDlgCompressGeneral::OnCheckLimitCompressFileCount(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if(BN_CLICKED==wNotifyCode){
		BOOL State=Check_LimitCompressFileCount.GetCheck();
		UpDown_MaxCompressFileCount.EnableWindow(State);
		::EnableWindow(GetDlgItem(IDC_EDIT_MAX_COMPRESS_FILECOUNT),State);
	}
	return 0;
}

//�f�t�H���g���k�p�����[�^�̗L������
LRESULT CConfigDlgCompressGeneral::OnCheckUseDefaultParameter(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if(BN_CLICKED==wNotifyCode){
		BOOL State=Check_UseDefaultParameter.GetCheck();
		::EnableWindow(GetDlgItem(IDC_BUTTON_SELECT_DEFAULTPARAMETER),State);
	}
	return 0;
}

//�f�t�H���g���k�p�����[�^�̑I��
LRESULT CConfigDlgCompressGeneral::OnSelectDefaultParameter(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if(BN_CLICKED==wNotifyCode){
		int Options=-1;
		bool bSingleCompression=false;	//���������
		bool bB2ESFX=false;
		CString strB2EFormat,strB2EMethod;

		//�`���I���_�C�A���O
		PARAMETER_TYPE CompressType=SelectCompressType(Options,bSingleCompression,strB2EFormat,strB2EMethod,bB2ESFX);
		if(CompressType==PARAMETER_UNDEFINED)return 1;	//�L�����Z��

		if(CompressType!=PARAMETER_B2E){	//�ʏ�DLL���g�p
			//�I���_�C�A���O�̏����Ɉ�v����p�����[�^������
			int Index=0;
			for(;Index<COMPRESS_PARAM_COUNT;Index++){
				if(CompressType==CompressParameterArray[Index].Type){
					if(Options==CompressParameterArray[Index].Options){
						break;
					}
				}
			}
			if(Index>=COMPRESS_PARAM_COUNT){
				//�ꗗ�Ɏw�肳�ꂽ���k�������Ȃ�
				//�܂�A�T�|�[�g���Ă��Ȃ����k�����������Ƃ�
				ErrorMessage(CString(MAKEINTRESOURCE(IDS_ERROR_ILLEGAL_FORMAT_TYPE)));
				return 1;
			}else{
				//�ݒ��ۑ�
				m_Config.DefaultType=CompressType;
				m_Config.DefaultOptions=Options;

				SetParameterInfo();//Edit�Ɍ��݂̃p�����[�^�̏���\������
				return 0;
			}
		}else{	//B2E32.dll���g�p
			//�p�����[�^
			if(bB2ESFX){
				m_Config.DefaultOptions=TRUE;
			}else{
				m_Config.DefaultOptions=0;
			}
			m_Config.DefaultType=CompressType;
			m_Config.DefaultB2EFormat=strB2EFormat;
			m_Config.DefaultB2EMethod=strB2EMethod;
			SetParameterInfo();//Edit�Ɍ��݂̃p�����[�^�̏���\������
			return 0;
		}
	}
	return 0;
}


void CConfigDlgCompressGeneral::SetParameterInfo()//Edit�Ɍ��݂̃p�����[�^�̏���\������
{
	if(m_Config.DefaultType==PARAMETER_UNDEFINED){
		//����`
		Edit_DefaultParameterInfo.SetWindowText(_T(""));
	}else if(m_Config.DefaultType!=PARAMETER_B2E){	//�ʏ�DLL���g�p
		//�I���_�C�A���O�̏����Ɉ�v����p�����[�^������
		int Index=0;
		for(;Index<COMPRESS_PARAM_COUNT;Index++){
			if(m_Config.DefaultType==CompressParameterArray[Index].Type){
				if(m_Config.DefaultOptions==CompressParameterArray[Index].Options){
					break;
				}
			}
		}
		if(Index>=COMPRESS_PARAM_COUNT){
			//�ꗗ�Ɏw�肳�ꂽ���k�������Ȃ�
			//�܂�A�T�|�[�g���Ă��Ȃ����k�����������Ƃ�
			Edit_DefaultParameterInfo.SetWindowText(CString(MAKEINTRESOURCE(IDS_ERROR_ILLEGAL_FORMAT_TYPE)));
		}
		else{
			//����Ȑݒ�
			Edit_DefaultParameterInfo.SetWindowText(CString(MAKEINTRESOURCE(CompressParameterArray[Index].FormatName)));
		}
	}else{	//B2E32.dll���g�p
		CString strInfo;
		BOOL bB2ESFX=m_Config.DefaultOptions;
		if(bB2ESFX){
			//���ȉ�
			strInfo.Format(IDS_FORMAT_NAME_B2E_SFX,m_Config.DefaultB2EFormat,m_Config.DefaultB2EMethod);
		}else{
			strInfo.Format(IDS_FORMAT_NAME_B2E,m_Config.DefaultB2EFormat,m_Config.DefaultB2EMethod);
		}
		Edit_DefaultParameterInfo.SetWindowText(strInfo);
	}
}


//�𓀌�폜�̐ݒ�ɍ��킹�ă`�F�b�N�{�b�N�X�̗L�����������߂�
LRESULT CConfigDlgCompressGeneral::OnCheckDelete(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if(BN_CLICKED==wNotifyCode){
		BOOL State=Check_DeleteAfterCompress.GetCheck();
		::EnableWindow(GetDlgItem(IDC_CHECK_MOVETO_RECYCLE_BIN),State);
		::EnableWindow(GetDlgItem(IDC_CHECK_DELETE_NOCONFIRM),State);
		::EnableWindow(GetDlgItem(IDC_CHECK_FORCE_DELETE),State);
	}
	return 0;
}
