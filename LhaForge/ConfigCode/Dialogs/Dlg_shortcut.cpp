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
#include "Dlg_shortcut.h"
#include "../../Compress.h"
#include "../../Dialogs/SelectDlg.h"
#include "../../ArchiverManager.h"
#include "../../Utilities/OSUtil.h"

//========================
// �V���[�g�J�b�g�쐬���
//========================
LRESULT CConfigDlgShortcut::OnInitDialog(HWND hWnd, LPARAM lParam)
{
	// ���b�Z�[�W���[�v�Ƀ��b�Z�[�W�t�B���^�ƃA�C�h���n���h����ǉ�
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	pLoop->AddMessageFilter(this);

	//Windows2000�ȑO�Ȃ�V���[�g�J�b�g�Ɉ����������Ȃ�
	//�����̃V���[�g�J�b�g�ȊO�͖�����
	if(AtlIsOldWindows()){
		//�𓀂̃V���[�g�J�b�g
		::EnableWindow(GetDlgItem(IDC_BUTTON_CREATE_EXTRACT_SHORTCUT_DESKTOP),false);
		::EnableWindow(GetDlgItem(IDC_BUTTON_CREATE_EXTRACT_SHORTCUT_SENDTO),false);

		//���k�̃V���[�g�J�b�g
		::EnableWindow(GetDlgItem(IDC_BUTTON_CREATE_COMPRESS_SHORTCUT_DESKTOP),false);
		::EnableWindow(GetDlgItem(IDC_BUTTON_CREATE_COMPRESS_SHORTCUT_SENDTO),false);

		//�{���̃V���[�g�J�b�g
		::EnableWindow(GetDlgItem(IDC_BUTTON_CREATE_LIST_SHORTCUT_DESKTOP),false);
		::EnableWindow(GetDlgItem(IDC_BUTTON_CREATE_LIST_SHORTCUT_SENDTO),false);

		//�����̃V���[�g�J�b�g
		::EnableWindow(GetDlgItem(IDC_BUTTON_CREATE_TESTARCHIVE_SHORTCUT_DESKTOP),false);
		::EnableWindow(GetDlgItem(IDC_BUTTON_CREATE_TESTARCHIVE_SHORTCUT_SENDTO),false);
	}
	return TRUE;
}

LRESULT CConfigDlgShortcut::OnCreateShortcut(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if(BN_CLICKED==wNotifyCode){
		//------------------------
		// LhaForge�{�̂̃p�X�擾
		//------------------------
		TCHAR ExePath[_MAX_PATH+1];
		FILL_ZERO(ExePath);
		GetModuleFileName(GetModuleHandle(NULL), ExePath, _MAX_PATH);

		//�V���[�g�J�b�g �t�@�C����
		TCHAR ShortcutFileName[_MAX_PATH+1];
		FILL_ZERO(ShortcutFileName);

		//----------------------
		// �쐬��t�H���_�̎擾
		//----------------------
		switch(wID){
		case IDC_BUTTON_CREATE_COMPRESS_SHORTCUT_DESKTOP://FALLTHROUGH
		case IDC_BUTTON_CREATE_EXTRACT_SHORTCUT_DESKTOP://FALLTHROUGH
		case IDC_BUTTON_CREATE_AUTOMATIC_SHORTCUT_DESKTOP://FALLTHROUGH
		case IDC_BUTTON_CREATE_LIST_SHORTCUT_DESKTOP://FALLTHROUGH
		case IDC_BUTTON_CREATE_TESTARCHIVE_SHORTCUT_DESKTOP:
			// �f�X�N�g�b�v�ɍ쐬
			if(!SHGetSpecialFolderPath(NULL,ShortcutFileName,CSIDL_DESKTOPDIRECTORY,FALSE)){
				ErrorMessage(CString(MAKEINTRESOURCE(IDS_ERROR_GET_DESKTOP)));
				return 0;
			}
			break;
		case IDC_BUTTON_CREATE_COMPRESS_SHORTCUT_SENDTO://FALLTHROUGH
		case IDC_BUTTON_CREATE_EXTRACT_SHORTCUT_SENDTO://FALLTHROUGH
		case IDC_BUTTON_CREATE_AUTOMATIC_SHORTCUT_SENDTO://FALLTHROUGH
		case IDC_BUTTON_CREATE_LIST_SHORTCUT_SENDTO://FALLTHROUGH
		case IDC_BUTTON_CREATE_TESTARCHIVE_SHORTCUT_SENDTO:
			// �u����v�t�H���_�ɍ쐬
			if(!SHGetSpecialFolderPath(NULL,ShortcutFileName,CSIDL_SENDTO,FALSE)){
				ErrorMessage(CString(MAKEINTRESOURCE(IDS_ERROR_GET_SENDTO)));
				return 0;
			}
			break;
		default:ASSERT(!"OnCreateShortcut:this code must not be run.");return 0;
		}

		//----------------------
		// �V���[�g�J�b�g�̐ݒ�
		//----------------------
		CString Param;	//�R�}���h���C������
		int IconIndex=-1;	//�V���[�g�J�b�g�A�C�R��
		WORD DescriptionID;	//�V���[�g�J�b�g�̐����̃��\�[�XID
		switch(wID){
		case IDC_BUTTON_CREATE_COMPRESS_SHORTCUT_DESKTOP://FALLTHROUGH
		case IDC_BUTTON_CREATE_COMPRESS_SHORTCUT_SENDTO:
			//LhaForge�ň��k
			if(!GetCompressShortcutInfo(ShortcutFileName,Param))return 0;
			DescriptionID=IDS_SHORTCUT_DESCRIPTION_COMPRESS;
			IconIndex=1;
			break;

		case IDC_BUTTON_CREATE_EXTRACT_SHORTCUT_DESKTOP://FALLTHROUGH
		case IDC_BUTTON_CREATE_EXTRACT_SHORTCUT_SENDTO:
			//LhaForge�ŉ�
			PathAppend(ShortcutFileName,CString(MAKEINTRESOURCE(IDS_SHORTCUT_NAME_EXTRACT)));
			Param=_T("/e");
			DescriptionID=IDS_SHORTCUT_DESCRIPTION_EXTRACT;
			IconIndex=2;
			break;

		case IDC_BUTTON_CREATE_AUTOMATIC_SHORTCUT_DESKTOP://FALLTHROUGH
		case IDC_BUTTON_CREATE_AUTOMATIC_SHORTCUT_SENDTO:
			//LhaForge�ŏ���
			PathAppend(ShortcutFileName,CString(MAKEINTRESOURCE(IDS_SHORTCUT_NAME_AUTOMATIC)));
			Param.Empty();
			DescriptionID=IDS_SHORTCUT_DESCRIPTION_AUTOMATIC;
			IconIndex=0;
			break;

		case IDC_BUTTON_CREATE_LIST_SHORTCUT_DESKTOP://FALLTHROUGH
		case IDC_BUTTON_CREATE_LIST_SHORTCUT_SENDTO:
			//LhaForge�ŉ{��
			PathAppend(ShortcutFileName,CString(MAKEINTRESOURCE(IDS_SHORTCUT_NAME_LIST)));
			Param=_T("/l");
			DescriptionID=IDS_SHORTCUT_DESCRIPTION_LIST;
			IconIndex=3;
			break;

		case IDC_BUTTON_CREATE_TESTARCHIVE_SHORTCUT_DESKTOP://FALLTHROUGH
		case IDC_BUTTON_CREATE_TESTARCHIVE_SHORTCUT_SENDTO:
			//LhaForge�Ō���
			PathAppend(ShortcutFileName,CString(MAKEINTRESOURCE(IDS_SHORTCUT_NAME_TESTARCHIVE)));
			Param=_T("/t");
			DescriptionID=IDS_SHORTCUT_DESCRIPTION_TESTARCHIVE;
			IconIndex=4;
			break;

		default:ASSERT(!"OnCreateShortcut:this code must not be run.");return 0;
		}
		//�g���q
		_tcsncat_s(ShortcutFileName,_T(".lnk"),_MAX_PATH);

		if(FAILED(UtilCreateShortcut(ShortcutFileName,ExePath,Param,ExePath,IconIndex,CString(MAKEINTRESOURCE(DescriptionID))))){
			ErrorMessage(CString(MAKEINTRESOURCE(IDS_ERROR_CREATE_SHORTCUT)));
		}else{
			//�쐬�����ŉ���炷
			MessageBeep(MB_ICONASTERISK);
		}
	}
	return 0;
}

//�쐬����V���[�g�J�b�g�̏����擾
//Path:�V���[�g�J�b�g�t�@�C����,Param:�R�}���h���C������
bool CConfigDlgShortcut::GetCompressShortcutInfo(LPTSTR Path,CString &Param)
{
/*	int Options=-1;
	bool bSingleCompression=false;
	bool bB2ESFX=false;
	CString strB2EFormat,strB2EMethod;*/

	//���k�`���������߂Ă������A��Ō��߂邩��I�΂���
	if(IDYES==MessageBox(CString(MAKEINTRESOURCE(IDS_ASK_SHORTCUT_COMPRESS_TYPE_ALWAYS_ASK)),UtilGetMessageCaption(),MB_YESNO|MB_ICONQUESTION)){
		int Options=-1;
		bool bSingleCompression=false;
		bool bB2ESFX=false;
		CString strB2EFormat,strB2EMethod;

		//�`���I���_�C�A���O
		PARAMETER_TYPE CompressType=SelectCompressType(Options,bSingleCompression,strB2EFormat,strB2EMethod,bB2ESFX);
		if(CompressType==PARAMETER_UNDEFINED)return false;	//�L�����Z��

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
				return false;
			}
			//�V���[�g�J�b�g���擾
			CString Buf;
			if(bSingleCompression){
				//������k
				Buf.Format(IDS_SHORTCUT_NAME_COMPRESS_EX_SINGLE,CString(MAKEINTRESOURCE(CompressParameterArray[Index].FormatName)));
			}
			else{
				//�ʏ�
				Buf.Format(IDS_SHORTCUT_NAME_COMPRESS_EX,CString(MAKEINTRESOURCE(CompressParameterArray[Index].FormatName)));
			}
			PathAppend(Path,Buf);
			//�p�����[�^
			Param=CompressParameterArray[Index].Param;
			if(bSingleCompression){
				//������k
				Param+=_T(" /s");
			}
		}else{	//B2E32.dll���g�p
			//�p�����[�^
			Param.Format(_T("/b2e \"/c:%s\" \"/method:%s\""),strB2EFormat,strB2EMethod);
			if(bB2ESFX){
				Param+=_T(" /b2esfx");
			}

			//�V���[�g�J�b�g���擾
			CString strInfo;
			if(bB2ESFX){
				strInfo.Format(IDS_FORMAT_NAME_B2E_SFX,strB2EFormat,strB2EMethod);
			}
			else{
				strInfo.Format(IDS_FORMAT_NAME_B2E,strB2EFormat,strB2EMethod);
			}
			CString Buf;
			if(bSingleCompression){
				//������k
				Buf.Format(IDS_SHORTCUT_NAME_COMPRESS_EX_SINGLE,strInfo);
				Param+=_T(" /s");
			}
			else{
				//�ʏ�
				Buf.Format(IDS_SHORTCUT_NAME_COMPRESS_EX,strInfo);
			}
			PathAppend(Path,Buf);
		}
	}
	else{
		//���k�`�������̓s�x���߂�
		PathAppend(Path,CString(MAKEINTRESOURCE(IDS_SHORTCUT_NAME_COMPRESS)));
		Param=_T("/c");
	}
	return true;
}

