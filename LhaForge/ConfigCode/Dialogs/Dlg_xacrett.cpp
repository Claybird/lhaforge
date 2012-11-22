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
#include "Dlg_xacrett.h"

//=================
// XACRETT�ݒ���
//=================
LRESULT CConfigDlgXACRETT::OnInitDialog(HWND hWnd, LPARAM lParam)
{
	// ���b�Z�[�W���[�v�Ƀ��b�Z�[�W�t�B���^�ƃA�C�h���n���h����ǉ�
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	pLoop->AddMessageFilter(this);

	//---------------------------
	// XacRett.DLL���g�����ǂ���
	//---------------------------
	Check_EnableXacrett=GetDlgItem(IDC_CHECK_ENABLE_XACRETT);
	Check_EnableXacrett.SetCheck(m_Config.EnableDLL);

	//DDX���A�b�v�f�[�g
	DoDataExchange(FALSE);

	//�L��/����
	::EnableWindow(GetDlgItem(IDC_EDIT_XACRETT_PRIORITY_EXTENSION),Check_EnableXacrett.GetCheck());
	return TRUE;
}

LRESULT CConfigDlgXACRETT::OnApply()
{
//===============================
// �ݒ��ConfigManager�ɏ����߂�
//===============================
	//---------------------------
	// XacRett.DLL���g�����ǂ���
	//---------------------------
	m_Config.EnableDLL=Check_EnableXacrett.GetCheck();

	//---------------
	// DDX�f�[�^�X�V
	//---------------
	if(!DoDataExchange(TRUE)){
		return FALSE;
	}

	return TRUE;
}

LRESULT CConfigDlgXACRETT::OnCheckEnableXacrett(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if(BN_CLICKED==wNotifyCode){
		//�L��/����
		::EnableWindow(GetDlgItem(IDC_EDIT_XACRETT_PRIORITY_EXTENSION),Check_EnableXacrett.GetCheck());
	}
	return 0;
}

