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

struct SHORTCUTINFO{
	CString strTitle;
	CString strCmd;
	CString strParam;
	CString strWorkingDir;
	CBitmap cIconBmpSmall;
};

//�V���[�g�J�b�g�쐬
HRESULT UtilCreateShortcut(LPCTSTR lpszPathLink,LPCTSTR lpszPathTarget,LPCTSTR lpszArgs,LPCTSTR lpszIconPath,int iIcon,LPCTSTR lpszDescription);

//�V���[�g�J�b�g�̏����擾
HRESULT UtilGetShortcutInfo(LPCTSTR lpPath,CString &strTargetPath,CString &strParam,CString &strWorkingDir);
void UtilGetShortcutInfo(const std::vector<CString> &files,std::vector<SHORTCUTINFO> &info);

//WindowsNT���ǂ���
bool UtilIsWindowsNT();
//�E�B���h�E���m���Ƀt�H�A�O���E���h�ɂ���
void UtilSetAbsoluteForegroundWindow(HWND);

//���݂�OS��WindowsVista�������͂���ȏ�Ȃ�true��Ԃ�
bool UtilIsOSVistaOrHigher();

//WoW64(64bit OS�ł�32bit�G�~�����[�V����)�œ����Ă����TRUE��Ԃ��֐�
BOOL UtilIsWow64();

//�R�}���h���C���������擾(����Ԃ�)
int UtilGetCommandLineParams(std::vector<CString>&);

//����̃t�H���_��Explorer�ŊJ��
void UtilNavigateDirectory(LPCTSTR lpszDir);

//���ϐ����Q�Ƃ��A�����`���Ŏ擾����
void UtilGetEnvInfo(std::map<stdString,stdString> &envInfo);

//UtilExpandTemplateString()�̃p�����[�^�W�J�ɕK�v�ȏ����\�z����
void UtilMakeExpandInformation(std::map<stdString,CString> &envInfo);

//�A�C�R���𓧖��x�t���r�b�g�}�b�v�ɕϊ�����
void UtilMakeDIBFromIcon(CBitmap&,HICON);

//�v���Z�X�D��x�̐ݒ�
void UtilSetPriorityClass(DWORD dwPriorityClass);

class CCurrentDirManager
{
	DISALLOW_COPY_AND_ASSIGN(CCurrentDirManager);
protected:
	TCHAR _prevDir[_MAX_PATH+1];
public:
	CCurrentDirManager(LPCTSTR lpPath){
		::GetCurrentDirectory(COUNTOF(_prevDir),_prevDir);
		::SetCurrentDirectory(lpPath);
	}
	virtual ~CCurrentDirManager(){
		::SetCurrentDirectory(_prevDir);
	}
};
