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
#include "Dlg_zip.h"

//=================
// ZIP��ʐݒ���
//=================
LRESULT CConfigDlgZIP::OnInitDialog(HWND hWnd, LPARAM lParam)
{
	// ���b�Z�[�W���[�v�Ƀ��b�Z�[�W�t�B���^�ƃA�C�h���n���h����ǉ�
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	pLoop->AddMessageFilter(this);
	//----------------
	// ���k�`���̑I��
	//----------------
	Radio_CompressType[ZIP_COMPRESS_DEFLATE]=GetDlgItem(IDC_RADIO_ZIP_DEFLATE);
	Radio_CompressType[ZIP_COMPRESS_DEFLATE64]=GetDlgItem(IDC_RADIO_ZIP_DEFLATE64);
	Radio_CompressType[ZIP_COMPRESS_BZIP2]=GetDlgItem(IDC_RADIO_ZIP_BZIP2);
	Radio_CompressType[ZIP_COMPRESS_COPY]=GetDlgItem(IDC_RADIO_ZIP_COPY);
	Radio_CompressType[ZIP_COMPRESS_LZMA]=GetDlgItem(IDC_RADIO_ZIP_LZMA);
	Radio_CompressType[ZIP_COMPRESS_PPMD]=GetDlgItem(IDC_RADIO_ZIP_PPMD);

	Radio_CompressType[m_Config.CompressType].SetCheck(1);

	//------------------
	// ���k���x���̑I��
	//------------------
	Radio_CompressLevel[ZIP_COMPRESS_LEVEL0]=GetDlgItem(IDC_RADIO_ZIP_COMPRESS_LEVEL_0);
	Radio_CompressLevel[ZIP_COMPRESS_LEVEL5]=GetDlgItem(IDC_RADIO_ZIP_COMPRESS_LEVEL_5);
	Radio_CompressLevel[ZIP_COMPRESS_LEVEL9]=GetDlgItem(IDC_RADIO_ZIP_COMPRESS_LEVEL_9);

	Radio_CompressLevel[m_Config.CompressLevel].SetCheck(1);


	//---------------
	// Deflate�̐ݒ�
	//---------------
	//�������T�C�Y
	::SendMessage(GetDlgItem(IDC_EDIT_ZIP_DEFLATE_MEMORY_SIZE),EM_LIMITTEXT,3,NULL);
	Check_SpecifyDeflateMemorySize=GetDlgItem(IDC_CHECK_ZIP_SPECIFY_MEMORY_SIZE);
	Check_SpecifyDeflateMemorySize.SetCheck(m_Config.SpecifyDeflateMemorySize);
	::EnableWindow(GetDlgItem(IDC_EDIT_ZIP_DEFLATE_MEMORY_SIZE),m_Config.SpecifyDeflateMemorySize);
	//�p�X��
	::SendMessage(GetDlgItem(IDC_EDIT_ZIP_DEFLATE_PASS_NUMBER),EM_LIMITTEXT,1,NULL);
	Check_SpecifyDeflatePassNumber=GetDlgItem(IDC_CHECK_ZIP_SPECIFY_PASS_NUMBER);
	Check_SpecifyDeflatePassNumber.SetCheck(m_Config.SpecifyDeflatePassNumber);
	::EnableWindow(GetDlgItem(IDC_EDIT_ZIP_DEFLATE_PASS_NUMBER),m_Config.SpecifyDeflatePassNumber);


	//DDX���A�b�v�f�[�g
	DoDataExchange(FALSE);

	return TRUE;
}

LRESULT CConfigDlgZIP::OnApply()
{
//===============================
// �ݒ��ConfigManager�ɏ����߂�
//===============================
	//----------------
	// ���k�`���̑I��
	//----------------
	for(int Type=0;Type<COUNTOF(Radio_CompressType);Type++){
		if(Radio_CompressType[Type].GetCheck()){
			m_Config.CompressType=(ZIP_COMPRESS_TYPE)Type;
			break;
		}
	}

	//------------------
	// ���k���x���̑I��
	//------------------
	for(int Type=0;Type<COUNTOF(Radio_CompressLevel);Type++){
		if(Radio_CompressLevel[Type].GetCheck()){
			m_Config.CompressLevel=(ZIP_COMPRESS_LEVEL)Type;
			break;
		}
	}

	//---------------
	// DDX�f�[�^�X�V
	//---------------
	if(!DoDataExchange(TRUE)){
		return FALSE;
	}
	//---------------
	// Deflate�̐ݒ�
	//---------------
	//�������T�C�Y
	m_Config.SpecifyDeflateMemorySize=Check_SpecifyDeflateMemorySize.GetCheck();
	//�p�X��
	m_Config.SpecifyDeflatePassNumber=Check_SpecifyDeflatePassNumber.GetCheck();
	return TRUE;
}

LRESULT CConfigDlgZIP::OnSpecifyDeflateMemorySize(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if(BN_CLICKED==wNotifyCode){
		::EnableWindow(GetDlgItem(IDC_EDIT_ZIP_DEFLATE_MEMORY_SIZE),Check_SpecifyDeflateMemorySize.GetCheck());
	}
	return 0;
}

LRESULT CConfigDlgZIP::OnSpecifyDeflatePassNumber(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if(BN_CLICKED==wNotifyCode){
		::EnableWindow(GetDlgItem(IDC_EDIT_ZIP_DEFLATE_PASS_NUMBER),Check_SpecifyDeflatePassNumber.GetCheck());
	}
	return 0;
}

