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

//WTL7.5�̃o�O��������邽�߂̂���
//�o�O:CFolderDialog��SetInitialFoder()�Ŏw�肳�ꂽ�t�H���_��BIF_NEWDIALOGSTYLE���w�肳��Ă���Ƃ��J����Ȃ�
//(Windows2000�����̖��̂悤�ł���)
//�Q��:http://sourceforge.net/tracker/index.php?func=detail&aid=1364046&group_id=109071&atid=652372
class CLFFolderDialog:public CFolderDialogImpl<CLFFolderDialog>
{
public:
	// �R���X�g���N�^
	CLFFolderDialog(HWND hWndParent = NULL, LPCTSTR lpstrTitle = NULL,UINT uFlags = BIF_RETURNONLYFSDIRS)
		:CFolderDialogImpl<CLFFolderDialog>(hWndParent, lpstrTitle, uFlags)
	{}

	void OnInitialized(){
		SetSelection(m_lpstrInitialFolder);
	}
};
