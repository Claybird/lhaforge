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
#include "CommonUtil.h"
#include "ConfigCode/ConfigManager.h"
#include "ConfigCode/ConfigGeneral.h"
#include "ArchiverCode/arc_interface.h"
#include "resource.h"
#include "Dialogs/LFFolderDialog.h"
#include "Utilities/FileOperation.h"
#include "Utilities/OSUtil.h"

//�ݒ肩��o�͐�t�H���_��ǂݍ���
//r_bUseForAll:����������t�H���_�ݒ���g���Ȃ�true
HRESULT GetOutputDirPathFromConfig(OUTPUT_TO outputDirType,LPCTSTR lpszOrgFile,LPCTSTR lpszSpecific,CPath &r_pathOutputDir,bool &r_bUseForAll,CString &strErr)
{
	TCHAR szBuffer[_MAX_PATH+1];
	FILL_ZERO(szBuffer);

	switch(outputDirType){
	case OUTPUT_TO_SPECIFIC_DIR:	//Specific Directory
		//TRACE(_T("Specific Dir:%s\n"),Config.Common.Extract.OutputDir);
		r_pathOutputDir=lpszSpecific;
		if(_tcslen(r_pathOutputDir)>0){
			return S_OK;
		}else{
			//�o�͐悪������Ă��Ȃ���΁A�f�X�N�g�b�v�ɏo�͂���
		}
		//FALLTHROUGH
	case OUTPUT_TO_DESKTOP:	//Desktop
		if(SHGetSpecialFolderPath(NULL,szBuffer,CSIDL_DESKTOPDIRECTORY,FALSE)){
			r_pathOutputDir=szBuffer;
		}else{	//�f�X�N�g�b�v���Ȃ��H
			strErr=CString(MAKEINTRESOURCE(IDS_ERROR_GET_DESKTOP));
			return E_FAIL;
		}
		return S_OK;
	case OUTPUT_TO_SAME_DIR:	//Same Directory
		_tcsncpy_s(szBuffer,lpszOrgFile,_MAX_PATH);
		PathRemoveFileSpec(szBuffer);
		r_pathOutputDir=szBuffer;
		return S_OK;
	case OUTPUT_TO_ALWAYS_ASK_WHERE:	//�o�͐�𖈉񕷂�
		TRACE(_T("Always ask\n"));
		{
			//���̃t�@�C���Ɠ����ꏊ�ɂ���;2��ڈȍ~�͑O��o�͏ꏊ���g�p����
			static CString s_strLastOutput;
			CPath pathTmp;
			if(s_strLastOutput.IsEmpty()){
				pathTmp=lpszOrgFile;
				pathTmp.RemoveFileSpec();
			}else{
				pathTmp=(LPCTSTR)s_strLastOutput;
			}

			CString title(MAKEINTRESOURCE(IDS_INPUT_TARGET_FOLDER_WITH_SHIFT));
			CLFFolderDialog dlg(NULL,title,BIF_RETURNONLYFSDIRS|BIF_NEWDIALOGSTYLE);
			dlg.SetInitialFolder(pathTmp);
			if(IDOK==dlg.DoModal()){
				r_bUseForAll=(GetKeyState(VK_SHIFT)<0);	//TODO
				r_pathOutputDir=dlg.GetFolderPath();
				s_strLastOutput=(LPCTSTR)r_pathOutputDir;
				return S_OK;
			}else{
				return E_ABORT;	//�L�����Z�����ꂽ
			}
		}
		break;
	default:
		ASSERT(!"This code cannot be run");
		return E_NOTIMPL;
	}
}


//S_FALSE���Ԃ����Ƃ��ɂ́A�u���O�����ĕۑ��v�_�C�A���O���J��
HRESULT ConfirmOutputDir(const CConfigGeneral &Conf,LPCTSTR lpszOutputDir,CString &strErr)
{
	//---
	// �o�͐悪�l�b�g���[�N�h���C�u/�����[�o�u���f�B�X�N�ł���Ȃ�A�o�͐��I��������
	if(Conf.WarnNetwork || Conf.WarnRemovable){
		//���[�g�h���C�u���擾
		CPath pathDrive=lpszOutputDir;
		pathDrive.StripToRoot();

		switch(GetDriveType(pathDrive)){
		case DRIVE_REMOVABLE://�h���C�u����f�B�X�N�𔲂����Ƃ��ł��܂��B
		case DRIVE_CDROM://CD-ROM
			if(Conf.WarnRemovable){
				if(IDNO==MessageBox(NULL,CString(MAKEINTRESOURCE(IDS_ASK_ISOK_REMOVABLE)),UtilGetMessageCaption(),MB_YESNO|MB_ICONQUESTION)){
					return S_FALSE;
				}
			}
			break;
		case DRIVE_REMOTE://�����[�g (�l�b�g���[�N) �h���C�u�ł��B
		case DRIVE_NO_ROOT_DIR:
			if(Conf.WarnNetwork){
				if(IDNO==MessageBox(NULL,CString(MAKEINTRESOURCE(IDS_ASK_ISOK_NETWORK)),UtilGetMessageCaption(),MB_YESNO|MB_ICONQUESTION)){
					return S_FALSE;
				}
			}
			break;
		}
	}

	//---
	//�o�͐�̃`�F�b�N
	if(!PathIsDirectory(lpszOutputDir)){
		//�p�X�����݂��Ȃ��ꍇ
		CString strMsg;
		switch(Conf.OnDirNotFound){
		case LOSTDIR_ASK_TO_CREATE:	//�쐬���邩�ǂ�������
			strMsg.Format(IDS_ASK_CREATE_DIR,lpszOutputDir);
			if(IDNO==MessageBox(NULL,strMsg,UtilGetMessageCaption(),MB_YESNO|MB_ICONQUESTION)){
				return E_ABORT;
			}
			//FALLTHROUGH
		case LOSTDIR_FORCE_CREATE:	//�f�B���N�g���쐬
			if(!UtilMakeSureDirectoryPathExists(lpszOutputDir)){
				strErr.Format(IDS_ERROR_CANNOT_MAKE_DIR,lpszOutputDir);
				//ErrorMessage(strMsg);
				return E_FAIL;
			}
			break;
		default://�G���[�ƌ��Ȃ�
			strErr.Format(IDS_ERROR_DIR_NOTFOUND,lpszOutputDir);
			return E_FAIL;
		}
	}

	return S_OK;
}

//UtilExpandTemplateString()�̃p�����[�^�W�J�ɕK�v�ȏ����\�z����
void MakeExpandInformationEx(std::map<stdString,CString> &envInfo,LPCTSTR lpOpenDir,LPCTSTR lpOutputFile)
{
	//���ϐ��ō\�z
	UtilMakeExpandInformation(envInfo);

	//�ϐ��o�^
	if(lpOpenDir){
		envInfo[_T("dir")]=lpOpenDir;
		envInfo[_T("OutputDir")]=lpOpenDir;

		//�o�̓h���C�u��
		TCHAR szBuffer[MAX_PATH+1];
		_tcsncpy_s(szBuffer,lpOpenDir,COUNTOF(szBuffer)-1);
		PathStripToRoot(szBuffer);
		if(szBuffer[_tcslen(szBuffer)-1]==L'\\')szBuffer[_tcslen(szBuffer)-1]=L'\0';
		envInfo[_T("OutputDrive")]=(LPCTSTR)szBuffer;
	}

	if(lpOutputFile){
		envInfo[_T("OutputFile")]=lpOutputFile;
		envInfo[_T("OutputFileName")]=PathFindFileName(lpOutputFile);
	}
}
