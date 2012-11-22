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
#include "ConfigCode/ConfigManager.h"
#include "CmdLineInfo.h"
#include "ArchiverManager.h"
#include "Utilities/FileOperation.h"
#include "Utilities/OSUtil.h"
#include "Utilities/StringUtil.h"
#include "compress.h"
#include "extract.h"
#include "ConfigCode/ConfigOpenAction.h"
#include "ConfigCode/ConfigGeneral.h"


CMDLINEINFO::CMDLINEINFO():
	CompressType(PARAMETER_UNDEFINED),
	Options(0),
	bSingleCompression(false),
	idForceDLL(DLL_ID_UNKNOWN),
	OutputToOverride((OUTPUT_TO)-1),
	CreateDirOverride((CREATE_OUTPUT_DIR)-1),
	IgnoreTopDirOverride(-1),
	DeleteAfterProcess(-1),
	PriorityOverride(LFPRIOTITY_DEFAULT)
{}


class COpenActionDialog : public CDialogImpl<COpenActionDialog>
{
public:
	enum {IDD = IDD_DIALOG_OPENACTION_SELECT};
	// ���b�Z�[�W�}�b�v
	BEGIN_MSG_MAP_EX(COpenActionDialog)
		COMMAND_ID_HANDLER_EX(IDC_BUTTON_OPENACTION_EXTRACT, OnButton)
		COMMAND_ID_HANDLER_EX(IDC_BUTTON_OPENACTION_LIST, OnButton)
		COMMAND_ID_HANDLER_EX(IDC_BUTTON_OPENACTION_TEST, OnButton)
		COMMAND_ID_HANDLER_EX(IDCANCEL, OnButton)
	END_MSG_MAP()

	void OnButton(UINT uNotifyCode, int nID, HWND hWndCtl){
		EndDialog(nID);
	}
};


//�J�������I��
PROCESS_MODE SelectOpenAction()
{
	COpenActionDialog Dialog;
	switch(Dialog.DoModal()){
	case IDC_BUTTON_OPENACTION_EXTRACT:
		return PROCESS_EXTRACT;
	case IDC_BUTTON_OPENACTION_LIST:
		return PROCESS_LIST;
	case IDC_BUTTON_OPENACTION_TEST:
		return PROCESS_TEST;
	default:
		return PROCESS_INVALID;
	}
}

//-----------

//�R�}���h���C�������߂��t�@�C���̏������@�����肷��
PROCESS_MODE ParseCommandLine(CConfigManager &ConfigManager,CMDLINEINFO &cli)
{
	std::vector<CString> ParamsArray;
	int nArgc=UtilGetCommandLineParams(ParamsArray);
#if defined(_DEBUG)
	//For Debug
//	MessageBox(NULL,GetCommandLine(),_T("CommandLine"),MB_OK|MB_ICONINFORMATION);
	TRACE(_T("---Command Parameter Dump---\n"));
	for(int i=0;i<nArgc;i++)
	{
		TRACE(_T("ParamsArray[%d]=%s\n"),i,ParamsArray[i]);
	}
	TRACE(_T("---End Dump---\n\n"));
#endif

	const bool bPressedShift=GetKeyState(VK_SHIFT)<0;	//SHIFT��������Ă��邩�ǂ���
	const bool bPressedControl=GetKeyState(VK_CONTROL)<0;	//CONTROL��������Ă��邩�ǂ���
	PROCESS_MODE ProcessMode=PROCESS_AUTOMATIC;

	//�f�t�H���g�l�ǂݍ���
	CString strErr;
	if(!ConfigManager.LoadConfig(strErr))ErrorMessage(strErr);

	UTIL_CODEPAGE uCodePage=UTILCP_SJIS;	//���X�|���X�t�@�C���̃R�[�h�y�[�W�w��

	for(int iIndex=1;iIndex<nArgc;iIndex++){
		if(0!=_tcsncmp(_T("/"),ParamsArray[iIndex],1)){//�I�v�V�����ł͂Ȃ�
			//�t�@�C���Ƃ݂Ȃ��A�����Ώۃt�@�C���̃��X�g�ɋl�ߍ���
			//�ȉ��̓t�@�C�����̏���
			if(0>=_tcslen(ParamsArray[iIndex])){	//������NULL�Ȃ疳��
				continue;
			}
			cli.FileList.push_back(ParamsArray[iIndex]);
		}else{
			//------------------
			// �I�v�V�����̉��
			//------------------
			CString Parameter(ParamsArray[iIndex]);
			//�������ɕϊ�
			Parameter.MakeLower();
			if(0==_tcsncmp(_T("/cfg"),Parameter,4)){//�ݒ�t�@�C�����w��
				if(0==_tcsncmp(_T("/cfg:"),Parameter,5)){
					//�o�̓t�@�C�����̐؂�o��;���̎��_��""�͊O��Ă���
					cli.ConfigPath=(LPCTSTR)ParamsArray[iIndex]+5;

					//---���ϐ�(LhaForge�Ǝ���`�ϐ�)�W�J
					//�p�����[�^�W�J�ɕK�v�ȏ��
					std::map<stdString,CString> envInfo;
					UtilMakeExpandInformation(envInfo);

					//�R�}���h�E�p�����[�^�W�J
					UtilExpandTemplateString(cli.ConfigPath, cli.ConfigPath, envInfo);
				}else if(_T("/cfg")==Parameter){
					cli.ConfigPath.Empty();
				}else{
					CString msg;
					msg.Format(IDS_ERROR_INVALID_PARAMETER,ParamsArray[iIndex]);
					ErrorMessage(msg);
					return PROCESS_INVALID;
				}
				//�ݒ�t�@�C���̎w�肪�L��΃Z�b�g����;������΃f�t�H���g�ɖ߂�
				if(cli.ConfigPath.IsEmpty()){
					ConfigManager.SetConfigFile(NULL);
				}else{
					ConfigManager.SetConfigFile(cli.ConfigPath);
				}
				//�ύX��̒l�ǂݍ���
				if(!ConfigManager.LoadConfig(strErr))ErrorMessage(strErr);
				TRACE(_T("ConfigPath=%s\n"),cli.ConfigPath);
			}else if(0==_tcsncmp(_T("/cp"),Parameter,3)){//���X�|���X�t�@�C���̃R�[�h�y�[�W�w��
				if(0==_tcsncmp(_T("/cp:"),Parameter,4)){
					CString cp((LPCTSTR)Parameter+4);
					cp.MakeLower();
					if(cp==_T("utf8")||cp==_T("utf-8")){
						uCodePage=UTILCP_UTF8;
					}else if(cp==_T("utf16")||cp==_T("utf-16")||cp==_T("unicode")){
						uCodePage=UTILCP_UTF16;
					}else if(cp==_T("sjis")||cp==_T("shiftjis")||cp==_T("s-jis")||cp==_T("s_jis")){
						uCodePage=UTILCP_SJIS;
					}else{
						CString msg;
						msg.Format(IDS_ERROR_INVALID_PARAMETER,(LPCTSTR)ParamsArray[iIndex]+4);
						ErrorMessage(msg);
						return PROCESS_INVALID;
					}
				}else if(_T("/cp")==Parameter){
					uCodePage=UTILCP_SJIS;	//�f�t�H���g�ɖ߂�
				}else{
					CString msg;
					msg.Format(IDS_ERROR_INVALID_PARAMETER,ParamsArray[iIndex]);
					ErrorMessage(msg);
					return PROCESS_INVALID;
				}
			}else if(0==_tcsncmp(_T("/c"),Parameter,2)){//���k���w������Ă���
				ProcessMode=PROCESS_COMPRESS;
				//----------------
				// ���k�`���̉��
				//----------------
				if(_T("/c")==Parameter){	//�`�����w�肳��Ă��Ȃ��ꍇ
					cli.CompressType=PARAMETER_UNDEFINED;
				}else if(0!=_tcsncmp(_T("/c:"),Parameter,3)){
					CString msg;
					msg.Format(IDS_ERROR_INVALID_PARAMETER,ParamsArray[iIndex]);
					ErrorMessage(msg);
					return PROCESS_INVALID;
				}else if(DLL_ID_B2E==cli.idForceDLL){	//B2E�ň��k����ꍇ
					//���k�������̂ݐ؂肾��
					cli.strFormat=(LPCTSTR)Parameter+3;
				}else{
					cli.CompressType=PARAMETER_UNDEFINED;
					//�R�}���h���C���p�����[�^�ƌ`���̑Ή��\����T��
					for(int i=0;i<COMPRESS_PARAM_COUNT;i++){
						if(CompressParameterArray[i].Param==Parameter){
							cli.CompressType=CompressParameterArray[i].Type;
							cli.Options=CompressParameterArray[i].Options;
							break;
						}
					}
					if(PARAMETER_UNDEFINED==cli.CompressType){
						CString msg;
						msg.Format(IDS_ERROR_INVALID_COMPRESS_PARAMETER,Parameter);
						ErrorMessage(msg);
						return PROCESS_INVALID;
					}

					//CONTROL�L�[��������Ă���Ȃ�A�ʈ��k
					if(bPressedControl){
						cli.bSingleCompression=true;
					}
				}
			}else if(_T("/e")==Parameter){//�𓀂��w������Ă���
				if(bPressedShift){
					ProcessMode=PROCESS_LIST;	//SHIFT�L�[��������Ă�����{�����[�h
				}else if(bPressedControl){
					ProcessMode=PROCESS_TEST;	//CTRL�L�[��������Ă����猟�����[�h
				}else{
					ProcessMode=PROCESS_EXTRACT;
				}
			}else if(_T("/l")==Parameter){//�t�@�C���ꗗ�\��
				ProcessMode=PROCESS_LIST;
			}else if(_T("/t")==Parameter){//�A�[�J�C�u�e�X�g
				ProcessMode=PROCESS_TEST;
			}else if(_T("/m")==Parameter){//�������@�I��
				CConfigOpenAction ConfOpenAction;
				ConfOpenAction.load(ConfigManager);
				OPENACTION OpenAction;
				if(bPressedShift){	//---Shift������
					OpenAction=ConfOpenAction.OpenAction_Shift;
				}else if(bPressedControl){	//---Ctrl������
					OpenAction=ConfOpenAction.OpenAction_Ctrl;
				}else{	//---�ʏ펞
					OpenAction=ConfOpenAction.OpenAction;
				}
				switch(OpenAction){
				case OPENACTION_EXTRACT://��
					ProcessMode=PROCESS_EXTRACT;
					break;
				case OPENACTION_LIST:	//�{��
					ProcessMode=PROCESS_LIST;
					break;
				case OPENACTION_TEST:	//����
					ProcessMode=PROCESS_TEST;
					break;
				case OPENACTION_ASK:	//����m�F
					ProcessMode=SelectOpenAction();
					if(ProcessMode==PROCESS_INVALID){
						return PROCESS_INVALID;
					}
					break;
				default:
					ASSERT(!"This code must not be run");
					return PROCESS_INVALID;
				}
			}else if(_T("/!")==Parameter||_T("/xacrett")==Parameter){//XacRett.DLL���g�p
				cli.idForceDLL=DLL_ID_XACRETT;
			}else if(_T("/b2e")==Parameter){//B2E32.dll���g�p
				cli.idForceDLL=DLL_ID_B2E;
			}else if(_T("/b2esfx")==Parameter){//B2E32.dll���g�p���Ď��ȉ𓀂Ɉ��k
				cli.Options|=COMPRESS_SFX;
			}else if(_T("/s")==Parameter){//�t�@�C����������k
				cli.bSingleCompression=true;
			}else if(0==_tcsncmp(_T("/o"),Parameter,2)){//�o�͐�t�H���_�w��
				if(0==_tcsncmp(_T("/o:"),Parameter,3)){
					//�o�͐�t�H���_�̐؂�o��;���̎��_��""�͊O��Ă���
					cli.OutputDir=(LPCTSTR)Parameter+3;
					cli.OutputToOverride=(OUTPUT_TO)-1;
				}else if(_T("/o")==Parameter){
					cli.OutputDir.Empty();
					cli.OutputToOverride=(OUTPUT_TO)-1;
				}else if(_T("/od")==Parameter){
					//�f�X�N�g�b�v�ɏo��
					cli.OutputDir.Empty();
					cli.OutputToOverride=OUTPUT_TO_DESKTOP;
				}else if(_T("/os")==Parameter){
					//����f�B���N�g���ɏo��
					cli.OutputDir.Empty();
					cli.OutputToOverride=OUTPUT_TO_SAME_DIR;
				}else if(_T("/oa")==Parameter){
					//���񕷂�
					cli.OutputDir.Empty();
					cli.OutputToOverride=OUTPUT_TO_ALWAYS_ASK_WHERE;
				}else{
					CString msg;
					msg.Format(IDS_ERROR_INVALID_PARAMETER,ParamsArray[iIndex]);
					ErrorMessage(msg);
					return PROCESS_INVALID;
				}
				TRACE(_T("OutputDir=%s\n"),cli.OutputDir);
			}else if(0==_tcsncmp(_T("/@"),Parameter,2)&&Parameter.GetLength()>2){//���X�|���X�t�@�C���w��
				CString strFile;
				if(PATHERROR_NONE!=UtilGetCompletePathName(strFile,(LPCTSTR)Parameter+2)||
					!UtilReadFromResponceFile(strFile,uCodePage,cli.FileList)){
					//�ǂݍ��ݎ��s
					ErrorMessage(CString(MAKEINTRESOURCE(IDS_ERROR_READ_RESPONCEFILE)));
					return PROCESS_INVALID;
				}
			}else if(0==_tcsncmp(_T("/$"),Parameter,2)&&Parameter.GetLength()>2){//���X�|���X�t�@�C���w��(�ǂݎ���폜)
				CString strFile;
				if(PATHERROR_NONE!=UtilGetCompletePathName(strFile,(LPCTSTR)Parameter+2)||
					!UtilReadFromResponceFile(strFile,uCodePage,cli.FileList)){
					//�ǂݍ��ݎ��s
					ErrorMessage(CString(MAKEINTRESOURCE(IDS_ERROR_READ_RESPONCEFILE)));
					return PROCESS_INVALID;
				}
				else DeleteFile(strFile);	//�폜
			}else if(0==_tcsncmp(_T("/f"),Parameter,2)){//�o�̓t�@�C�����w��
				if(0==_tcsncmp(_T("/f:"),Parameter,3)){
					//�o�̓t�@�C�����̐؂�o��;���̎��_��""�͊O��Ă���
					cli.OutputFileName=(LPCTSTR)Parameter+3;
				}else if(_T("/f")==Parameter){
					cli.OutputFileName.Empty();
				}else{
					CString msg;
					msg.Format(IDS_ERROR_INVALID_PARAMETER,ParamsArray[iIndex]);
					ErrorMessage(msg);
					return PROCESS_INVALID;
				}
				TRACE(_T("OutputFileName=%s\n"),cli.OutputFileName);
			}else if(0==_tcsncmp(_T("/method:"),Parameter,8)){//���k���\�b�h�w��
				cli.strMethod=(LPCTSTR)Parameter+8;
			}else if(0==_tcsncmp(_T("/level:"),Parameter,7)){//���k���\�b�h�w��
				cli.strLevel=(LPCTSTR)Parameter+7;
			}else if(0==_tcsncmp(_T("/mkdir:"),Parameter,7)){//�𓀎��̏o�̓f�B���N�g������
				CString mode=(LPCTSTR)Parameter+7;
				if(_T("no")==mode){
					cli.CreateDirOverride=CREATE_OUTPUT_DIR_NEVER;
				}else if(_T("single")==mode){
					cli.CreateDirOverride=CREATE_OUTPUT_DIR_SINGLE;
				}else if(_T("always")==mode){
					cli.CreateDirOverride=CREATE_OUTPUT_DIR_ALWAYS;
				}else{
					CString msg;
					msg.Format(IDS_ERROR_INVALID_PARAMETER,ParamsArray[iIndex]);
					ErrorMessage(msg);
					return PROCESS_INVALID;
				}
			}else if(0==_tcsncmp(_T("/popdir:"),Parameter,8)){//�𓀎��̏o�̓f�B���N�g������
				CString mode=(LPCTSTR)Parameter+8;
				if(_T("no")==mode){
					cli.IgnoreTopDirOverride=0;
				}else if(_T("yes")==mode){
					cli.IgnoreTopDirOverride=1;
				}else{
					CString msg;
					msg.Format(IDS_ERROR_INVALID_PARAMETER,ParamsArray[iIndex]);
					ErrorMessage(msg);
					return PROCESS_INVALID;
				}
			}else if(_T("/popdir")==Parameter){//�𓀎��̏o�̓f�B���N�g������
				cli.IgnoreTopDirOverride=1;
			}else if(0==_tcsncmp(_T("/delete:"),Parameter,8)){//������Ƀ\�[�X���폜���邩
				CString mode=(LPCTSTR)Parameter+8;
				if(_T("no")==mode){
					cli.DeleteAfterProcess=0;
				}else if(_T("yes")==mode){
					cli.DeleteAfterProcess=1;
				}else{
					CString msg;
					msg.Format(IDS_ERROR_INVALID_PARAMETER,ParamsArray[iIndex]);
					ErrorMessage(msg);
					return PROCESS_INVALID;
				}
			}else if(_T("/delete")==Parameter){//������ɍ폜
				cli.DeleteAfterProcess=1;
			}else if(0==_tcsncmp(_T("/priority:"),Parameter,10)){	//�v���Z�X�D��x
				CString mode=(LPCTSTR)Parameter+10;
					 if(_T("low")==mode)	cli.PriorityOverride=LFPRIOTITY_LOW;
				else if(_T("lower")==mode)	cli.PriorityOverride=LFPRIOTITY_LOWER;
				else if(_T("normal")==mode)	cli.PriorityOverride=LFPRIOTITY_NORMAL;
				else if(_T("higher")==mode)	cli.PriorityOverride=LFPRIOTITY_HIGHER;
				else if(_T("high")==mode)	cli.PriorityOverride=LFPRIOTITY_HIGH;
				else{
					CString msg;
					msg.Format(IDS_ERROR_INVALID_PARAMETER,ParamsArray[iIndex]);
					ErrorMessage(msg);
					return PROCESS_INVALID;
				}
			}else{	//���m�̃I�v�V����
				CString msg;
				msg.Format(IDS_ERROR_INVALID_PARAMETER,ParamsArray[iIndex]);
				ErrorMessage(msg);
				return PROCESS_INVALID;
			}
		}
	}
	if(cli.FileList.empty()){
		//�X�C�b�`�݂̂��w�肳��Ă����ꍇ�ɂ͐ݒ��ʂ�\��������
		//------
		// ���݂��Ȃ��t�@�C�����w�肳��Ă����ꍇ�ɂ̓G���[���Ԃ��Ă���̂ŁA
		// �����Ńt�@�C�����X�g����ł���΃t�@�C�����w�肳��Ă��Ȃ��Ɣ��f�ł���B
		//return PROCESS_CONFIGURE;
	}else{
		//���C���h�J�[�h�̓W�J
		UtilPathExpandWild(cli.FileList,cli.FileList);
	}
	//---�t�@�C�����̃t���p�X�Ȃǃ`�F�b�N
	for(std::list<CString>::iterator ite=cli.FileList.begin();ite!=cli.FileList.end();ite++){
		CPath strAbsPath;
		switch(UtilGetCompletePathName(strAbsPath,*ite)){
		case PATHERROR_NONE:
			//����
			break;
		case PATHERROR_INVALID:
			//�p�����[�^�w�肪�s��
			ASSERT(!"�p�����[�^�w�肪�s��");
			return PROCESS_INVALID;
		case PATHERROR_ABSPATH:
			//��΃p�X�̎擾�Ɏ��s
			ErrorMessage(CString(MAKEINTRESOURCE(IDS_ERROR_FAIL_GET_ABSPATH)));
			return PROCESS_INVALID;
		case PATHERROR_NOTFOUND:
			//�t�@�C���������̓t�H���_�����݂��Ȃ�
			{
				CString msg;
				msg.Format(IDS_ERROR_FILE_NOT_FOUND,*ite);
				ErrorMessage(msg);
			}
			return PROCESS_INVALID;
		case PATHERROR_LONGNAME:
			//�����O�t�@�C�����擾���s
			ErrorMessage(CString(MAKEINTRESOURCE(IDS_ERROR_FAIL_GET_LONGNAME)));
			return PROCESS_INVALID;
		}

		//�p�X���̍Ōオ\�ŏI����Ă�����\����菜��
		strAbsPath.RemoveBackslash();
		TRACE(strAbsPath),TRACE(_T("\n"));

		//�l�X�V
		*ite=(CString)strAbsPath;
	}
	//�o�̓t�H���_���w�肳��Ă�����A������΃p�X�ɕϊ�
	if(!cli.OutputDir.IsEmpty()){
		if(!UtilGetAbsPathName(cli.OutputDir,cli.OutputDir)){
			//��΃p�X�̎擾�Ɏ��s
			ErrorMessage(CString(MAKEINTRESOURCE(IDS_ERROR_FAIL_GET_ABSPATH)));
			return PROCESS_INVALID;
		}
	}

	return ProcessMode;
}

