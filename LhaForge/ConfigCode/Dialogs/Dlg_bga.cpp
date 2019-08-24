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
#include "Dlg_bga.h"

//=================
// BGA��ʐݒ���
//=================
LRESULT CConfigDlgBGA::OnInitDialog(HWND hWnd, LPARAM lParam)
{
	// ���b�Z�[�W���[�v�Ƀ��b�Z�[�W�t�B���^�ƃA�C�h���n���h����ǉ�
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	pLoop->AddMessageFilter(this);

	//--------------------------------
	// GZA���k���x���̐ݒ�p�X���C�_
	//--------------------------------
	Track_GZA_Level=GetDlgItem(IDC_SLIDER_GZA_COMPRESS_LEVEL);
	Track_GZA_Level.SetRange(GZA_COMPRESS_LEVEL_LOWEST,GZA_COMPRESS_LEVEL_HIGHEST);
	Track_GZA_Level.SetTicFreq(1);
	Track_GZA_Level.SetPageSize(1);
	Track_GZA_Level.SetLineSize(1);
	Track_GZA_Level.SetPos(m_Config.GZALevel);

	//----------------------------------
	// GZA���k���x���̊m�F�p�G�f�B�b�g
	//----------------------------------
	Edit_GZA_Level=GetDlgItem(IDC_EDIT_GZA_COMPRESS_LEVEL);
	CString Buffer;
	Buffer.Format(_T("%d"),m_Config.GZALevel);
	Edit_GZA_Level.SetWindowText(Buffer);

	//---------------------------------
	// BZA���k���x���̐ݒ�p�X���C�_
	//---------------------------------
	Track_BZA_Level=GetDlgItem(IDC_SLIDER_BZA_COMPRESS_LEVEL);
	Track_BZA_Level.SetRange(BZA_COMPRESS_LEVEL_LOWEST,BZA_COMPRESS_LEVEL_HIGHEST);
	Track_BZA_Level.SetTicFreq(1);
	Track_BZA_Level.SetPageSize(1);
	Track_BZA_Level.SetLineSize(1);
	Track_BZA_Level.SetPos(m_Config.BZALevel);

	//-----------------------------------
	// BZA���k���x���̊m�F�p�G�f�B�b�g
	//-----------------------------------
	Edit_BZA_Level=GetDlgItem(IDC_EDIT_BZA_COMPRESS_LEVEL);
	Buffer.Format(_T("%d"),m_Config.BZALevel);
	Edit_BZA_Level.SetWindowText(Buffer);

	return TRUE;
}

LRESULT CConfigDlgBGA::OnApply()
{
//===============================
// �ݒ��ConfigManager�ɏ����߂�
//===============================
	//----------------
	// GZA���k���x��
	//----------------
	m_Config.GZALevel=Track_GZA_Level.GetPos();

	//-----------------
	// BZA���k���x��
	//-----------------
	m_Config.BZALevel=Track_BZA_Level.GetPos();
	return TRUE;
}


void CConfigDlgBGA::OnHScroll(int, short, HWND)
{
	CString Buffer;
	Buffer.Format(_T("%d"),Track_GZA_Level.GetPos());
	Edit_GZA_Level.SetWindowText(Buffer);
	Buffer.Format(_T("%d"),Track_BZA_Level.GetPos());
	Edit_BZA_Level.SetWindowText(Buffer);
}

