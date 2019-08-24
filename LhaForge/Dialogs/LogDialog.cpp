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
#include "LogDialog.h"
#include "../Utilities/OSUtil.h"

void CLogDialog::SetData(LPCTSTR log)
{
	LogStr=log;

	//���s�R�[�h�̕ϊ�
	// \n��\r\n�ƒu��������
	LogStr.Replace(_T("\n"),_T("\r\n"));
	LogStr.Replace(_T("\r\r\n"),_T("\r\n"));
}


LRESULT CLogDialog::OnInitDialog(HWND hWnd, LPARAM lParam)
{
	CenterWindow();
	LogView=GetDlgItem(IDC_EDIT_LOG);
	LogView.SetWindowText(LogStr);

	// �_�C�A���O���T�C�Y������
	DlgResize_Init(true, true, WS_THICKFRAME | WS_CLIPCHILDREN);

	//---�E�B���h�E���A�N�e�B�u�ɂ���
	UtilSetAbsoluteForegroundWindow(hWnd);
	return TRUE;
}


