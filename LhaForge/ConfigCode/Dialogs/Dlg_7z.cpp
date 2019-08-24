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
#include "Dlg_7z.h"
#include "../../Dialogs/SevenZipVolumeSizeDlg.h"

//=================
// 7Z��ʐݒ���
//=================
LRESULT CConfigDlg7Z::OnInitDialog(HWND hWnd, LPARAM lParam)
{
	// ���b�Z�[�W���[�v�Ƀ��b�Z�[�W�t�B���^�ƃA�C�h���n���h����ǉ�
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	pLoop->AddMessageFilter(this);

	//----------------
	// ���k�`���̑I��
	//----------------
	//�v���Z�b�g���g����
	Check_UsePreset=GetDlgItem(IDC_CHECK_7Z_USE_PRESET);
	Check_UsePreset.SetCheck(m_Config.UsePreset);
	//�蓮�ݒ肷�邩
	Radio_CompressType[SEVEN_ZIP_COMPRESS_LZMA]=GetDlgItem(IDC_RADIO_7Z_METHOD_LZMA);
	Radio_CompressType[SEVEN_ZIP_COMPRESS_PPMD]=GetDlgItem(IDC_RADIO_7Z_METHOD_PPMD);
	Radio_CompressType[SEVEN_ZIP_COMPRESS_BZIP2]=GetDlgItem(IDC_RADIO_7Z_METHOD_BZIP2);
	Radio_CompressType[SEVEN_ZIP_COMPRESS_DEFLATE]=GetDlgItem(IDC_RADIO_7Z_METHOD_DEFLATE);
	Radio_CompressType[SEVEN_ZIP_COMPRESS_COPY]=GetDlgItem(IDC_RADIO_7Z_METHOD_COPY);
	Radio_CompressType[SEVEN_ZIP_COMPRESS_LZMA2]=GetDlgItem(IDC_RADIO_7Z_METHOD_LZMA2);

	Radio_CompressType[m_Config.CompressType].SetCheck(1);

	//�v���Z�b�g���g���ꍇ�ɂ͖�����
	for(int Type=0;Type<COUNTOF(Radio_CompressType);Type++){
		Radio_CompressType[Type].EnableWindow(!m_Config.UsePreset);
	}

	//------------------------------
	// ���k���x���̑I��(�v���Z�b�g)
	//------------------------------
	Radio_CompressLevel[SEVEN_ZIP_COMPRESS_LEVEL0]=GetDlgItem(IDC_RADIO_7Z_COMPRESS_LEVEL_0);
	Radio_CompressLevel[SEVEN_ZIP_COMPRESS_LEVEL1]=GetDlgItem(IDC_RADIO_7Z_COMPRESS_LEVEL_1);
	Radio_CompressLevel[SEVEN_ZIP_COMPRESS_LEVEL5]=GetDlgItem(IDC_RADIO_7Z_COMPRESS_LEVEL_5);
	Radio_CompressLevel[SEVEN_ZIP_COMPRESS_LEVEL7]=GetDlgItem(IDC_RADIO_7Z_COMPRESS_LEVEL_7);
	Radio_CompressLevel[SEVEN_ZIP_COMPRESS_LEVEL9]=GetDlgItem(IDC_RADIO_7Z_COMPRESS_LEVEL_9);

	Radio_CompressLevel[m_Config.CompressLevel].SetCheck(1);
	//�v���Z�b�g���g��Ȃ��ꍇ�ɂ͖�����
	for(int Type=0;Type<COUNTOF(Radio_CompressLevel);Type++){
		Radio_CompressLevel[Type].EnableWindow(m_Config.UsePreset);
	}

	//----------------------
	// LZMA���k���[�h�̑I��
	//----------------------
	Radio_LZMA_Mode[SEVEN_ZIP_LZMA_MODE0]=GetDlgItem(IDC_RADIO_7Z_LZMA_MODE0);
	Radio_LZMA_Mode[SEVEN_ZIP_LZMA_MODE1]=GetDlgItem(IDC_RADIO_7Z_LZMA_MODE1);

	Radio_LZMA_Mode[m_Config.LZMA_Mode].SetCheck(1);
	//�v���Z�b�g���g���ꍇ�ɂ͖�����
	for(int Type=0;Type<COUNTOF(Radio_LZMA_Mode);Type++){
		Radio_LZMA_Mode[Type].EnableWindow((SEVEN_ZIP_COMPRESS_LZMA==m_Config.CompressType || SEVEN_ZIP_COMPRESS_LZMA2==m_Config.CompressType)&&(!m_Config.UsePreset));
	}

	//------------
	// �w�b�_���k
	//------------
	Check_HeaderCompression=GetDlgItem(IDC_CHECK_7Z_HEADER_COMPRESSION);

	//------------
	// PPMd�̐ݒ�
	//------------
	//���f���T�C�Y
	::SendMessage(GetDlgItem(IDC_EDIT_7Z_PPMD_MODEL_SIZE),EM_LIMITTEXT,2,NULL);
	Check_SpecifyPPMdModelSize=GetDlgItem(IDC_CHECK_7Z_SPECIFY_PPMD_MODEL_SIZE);
	Check_SpecifyPPMdModelSize.SetCheck(m_Config.SpecifyPPMdModelSize);
	//�v���Z�b�g���g���ꍇ�ɂ͖�����
	Check_SpecifyPPMdModelSize.EnableWindow((SEVEN_ZIP_COMPRESS_PPMD==m_Config.CompressType)&&!m_Config.UsePreset);
	::EnableWindow(GetDlgItem(IDC_EDIT_7Z_PPMD_MODEL_SIZE),(SEVEN_ZIP_COMPRESS_PPMD==m_Config.CompressType)&&m_Config.SpecifyPPMdModelSize&&!m_Config.UsePreset);

	//------------------
	// �����T�C�Y�̐ݒ�
	//------------------
	::EnableWindow(GetDlgItem(IDC_EDIT_7Z_SPLIT_SIZE), m_Config.SpecifySplitSize);
	::EnableWindow(GetDlgItem(IDC_COMBO_7Z_SPLIT_SIZE_UNIT), m_Config.SpecifySplitSize);

	Combo_SizeUnit=GetDlgItem(IDC_COMBO_7Z_SPLIT_SIZE_UNIT);
	for(int i=0;i<ZIP_VOLUME_UNIT_MAX_NUM;i++){
		Combo_SizeUnit.InsertString(-1,ZIP_VOLUME_UNIT[i].DispName);
	}
	Combo_SizeUnit.SetCurSel(m_Config.SplitSizeUnit);

	//DDX���A�b�v�f�[�g
	DoDataExchange(FALSE);

	//�w�b�_���S���k�̐ݒ�
	::EnableWindow(GetDlgItem(IDC_CHECK_7Z_FULL_HEADER_COMPRESSION),Check_HeaderCompression.GetCheck());

	return TRUE;
}

LRESULT CConfigDlg7Z::OnApply()
{
//===============================
// �ݒ��ConfigManager�ɏ����߂�
//===============================
	//----------------
	// ���k�`���̑I��
	//----------------
	//�v���Z�b�g���g�����ǂ���
	m_Config.UsePreset=Check_UsePreset.GetCheck();

	//�蓮�ݒ�̏ꍇ
	for(int Type=0;Type<COUNTOF(Radio_CompressType);Type++){
		if(Radio_CompressType[Type].GetCheck()){
			m_Config.CompressType=(SEVEN_ZIP_COMPRESS_TYPE)Type;
			break;
		}
	}

	//------------------
	// ���k���x���̑I��
	//------------------
	for(int Type=0;Type<COUNTOF(Radio_CompressLevel);Type++){
		if(Radio_CompressLevel[Type].GetCheck()){
			m_Config.CompressLevel=(SEVEN_ZIP_COMPRESS_LEVEL)Type;
			break;
		}
	}

	//----------------------
	// LZMA���k���[�h�̑I��
	//----------------------
	for(int Type=0;Type<COUNTOF(Radio_LZMA_Mode);Type++){
		if(Radio_LZMA_Mode[Type].GetCheck()){
			m_Config.LZMA_Mode=(SEVEN_ZIP_LZMA_MODE)Type;
			break;
		}
	}

	//------------------
	// �����T�C�Y�̐ݒ�
	//------------------
	m_Config.SplitSizeUnit = Combo_SizeUnit.GetCurSel();

	//---------------
	// DDX�f�[�^�X�V
	//---------------
	if(!DoDataExchange(TRUE)){
		return FALSE;
	}
	//---------------
	// PPMd�̐ݒ�
	//---------------
	//���f���T�C�Y
	m_Config.SpecifyPPMdModelSize=Check_SpecifyPPMdModelSize.GetCheck();
//	m_Config.PPMdModelSize=PPMdModelSize;
	return TRUE;
}

LRESULT CConfigDlg7Z::OnUsePreset(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if(BN_CLICKED==wNotifyCode){
		for(int Type=0;Type<COUNTOF(Radio_CompressType);Type++){
			Radio_CompressType[Type].EnableWindow(!Check_UsePreset.GetCheck());
		}
		for(int Type=0;Type<COUNTOF(Radio_CompressLevel);Type++){
			Radio_CompressLevel[Type].EnableWindow(Check_UsePreset.GetCheck());
		}
		for(int Type=0;Type<COUNTOF(Radio_LZMA_Mode);Type++){
			BOOL bCheck=Radio_CompressType[SEVEN_ZIP_COMPRESS_LZMA].GetCheck() || Radio_CompressType[SEVEN_ZIP_COMPRESS_LZMA2].GetCheck();
			Radio_LZMA_Mode[Type].EnableWindow(bCheck&&!Check_UsePreset.GetCheck());
		}
		Check_SpecifyPPMdModelSize.EnableWindow(Radio_CompressType[SEVEN_ZIP_COMPRESS_PPMD].GetCheck()&&!Check_UsePreset.GetCheck());
		::EnableWindow(GetDlgItem(IDC_EDIT_7Z_PPMD_MODEL_SIZE),Radio_CompressType[SEVEN_ZIP_COMPRESS_PPMD].GetCheck()&&Check_SpecifyPPMdModelSize.GetCheck()&&!Check_UsePreset.GetCheck());
	}
	return 0;
}

LRESULT CConfigDlg7Z::OnSpecifyPPMdModelSize(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if(BN_CLICKED==wNotifyCode){
		::EnableWindow(GetDlgItem(IDC_EDIT_7Z_PPMD_MODEL_SIZE),(Check_SpecifyPPMdModelSize.GetCheck())&&(!Check_UsePreset.GetCheck()));
	}
	return 0;
}

LRESULT CConfigDlg7Z::OnSelectCompressType(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if(BN_CLICKED==wNotifyCode){
		//LZMA
		for(int Type=0;Type<COUNTOF(Radio_LZMA_Mode);Type++){
			BOOL bCheck=Radio_CompressType[SEVEN_ZIP_COMPRESS_LZMA].GetCheck() || Radio_CompressType[SEVEN_ZIP_COMPRESS_LZMA2].GetCheck();
			Radio_LZMA_Mode[Type].EnableWindow(bCheck&&!Check_UsePreset.GetCheck());
		}
		//PPMd
		Check_SpecifyPPMdModelSize.EnableWindow(Radio_CompressType[SEVEN_ZIP_COMPRESS_PPMD].GetCheck()&&!Check_UsePreset.GetCheck());
		::EnableWindow(GetDlgItem(IDC_EDIT_7Z_PPMD_MODEL_SIZE),Radio_CompressType[SEVEN_ZIP_COMPRESS_PPMD].GetCheck()&&Check_SpecifyPPMdModelSize.GetCheck()&&!Check_UsePreset.GetCheck());
	}
	return 0;
}

LRESULT CConfigDlg7Z::OnHeaderCompression(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if(BN_CLICKED==wNotifyCode){
		::EnableWindow(GetDlgItem(IDC_CHECK_7Z_FULL_HEADER_COMPRESSION),Check_HeaderCompression.GetCheck());
	}
	return 0;
}

LRESULT CConfigDlg7Z::OnSpecifySplitSize(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if(BN_CLICKED==wNotifyCode){
		::EnableWindow(GetDlgItem(IDC_EDIT_7Z_SPLIT_SIZE), Button_GetState(GetDlgItem(IDC_CHECK_7Z_SPECIFY_SPLIT_SIZE)));
		::EnableWindow(GetDlgItem(IDC_COMBO_7Z_SPLIT_SIZE_UNIT), Button_GetState(GetDlgItem(IDC_CHECK_7Z_SPECIFY_SPLIT_SIZE)));
	}
	return 0;
}

