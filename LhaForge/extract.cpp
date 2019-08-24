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
#include "extract.h"
#include "ArchiverManager.h"
#include "resource.h"
#include "ConfigCode/ConfigManager.h"
#include "ConfigCode/ConfigGeneral.h"
#include "ConfigCode/ConfigExtract.h"
#include "Utilities/Semaphore.h"
#include "Dialogs/LogDialog.h"
#include "Dialogs/LogListDialog.h"
#include "Dialogs/ProgressDlg.h"
#include "Dialogs/LFFolderDialog.h"
#include "Utilities/StringUtil.h"
#include "Utilities/FileOperation.h"
#include "Utilities/OSUtil.h"
#include "CommonUtil.h"
#include "CmdLineInfo.h"

//�폜�Ώۂ̋L�����X�g
const LPCWSTR g_szTable=L"0123456789./*-+{}[]@`:;!\"#$%&\'()_><=~^|,\\ �@";	//�Ō�͔��p��,�S�p��

//strOpenDir:�𓀐���J���Ƃ��A���ۂɊJ���p�X
bool ExtractOneArchive(CConfigManager &ConfMan,const CConfigGeneral &ConfGeneral,const CConfigExtract &ConfExtract,CArchiverDLL *lpArchiver,LPCTSTR lpArcFile,CPath &r_pathSpecificOutputDir,ARCLOG &r_ArcLog,CPath &r_pathOpenDir)
{
	//�t�@�C�������L�^
	r_ArcLog.strFile=lpArcFile;

	//���S�ȃA�[�J�C�u���ǂ����A�����
	//��d�Ƀt�H���_�����Ȃ��悤�A��Ƀt�H���_���K�v���ǂ����𔻒肷��
	bool bInFolder,bSafeArchive;
	bool bSkipDirCheck=(CREATE_OUTPUT_DIR_SINGLE!=ConfExtract.CreateDir);

	CString strBaseDir;	//��d�t�H���_�`�F�b�N�̂Ƃ��ɓ�����A�S�Ẵt�@�C��������t�H���_�̖��O(���������)
	CString strExamErr;
	if(!lpArchiver->ExamineArchive(lpArcFile,ConfMan,bSkipDirCheck,bInFolder,bSafeArchive,strBaseDir,strExamErr)){
		//NOTE:B2E32.dll�̂��߂ɁA�G���[�`�F�b�N���キ���Ă���
		ErrorMessage(strExamErr);
		bInFolder=false;
		bSafeArchive=false;
	}

	//�A�[�J�C�u���̃t�@�C���E�t�H���_�̐��𒲂ׂ�
	int nItemCount=-1;
	if(ConfExtract.CreateNoFolderIfSingleFileOnly){
		//�u�t�@�C���E�t�H���_��������̎��t�H���_���쐬���Ȃ��v�ݒ�̎��ɂ̂ݒ�������
		nItemCount=lpArchiver->GetFileCount(lpArcFile);
	}

	bool bRet=false;
	CPath pathOutputDir;
	bool bUseForAll=false;	//����������o�̓t�H���_���g���Ȃ�true
	CString strErr;
	HRESULT hr=GetExtractDestDir(lpArcFile,ConfGeneral,ConfExtract,r_pathSpecificOutputDir,bInFolder,pathOutputDir,nItemCount,strBaseDir,r_pathOpenDir,bUseForAll,strErr);
	if(FAILED(hr)){
		if(E_ABORT == hr){
			r_ArcLog.Result=EXTRACT_CANCELED;
			r_ArcLog.strMsg.Format(IDS_ERROR_USERCANCEL);
		}else{
			r_ArcLog.Result=EXTRACT_NG;
			r_ArcLog.strMsg=strErr;
		}
		return false;
	}
	if(bUseForAll){	//����̐ݒ���㏑��
		r_pathSpecificOutputDir=pathOutputDir;
	}

	//�o�͐�f�B���N�g�����J�����g�f�B���N�g���ɐݒ�
	if(!SetCurrentDirectory(pathOutputDir)){
		r_ArcLog.Result=EXTRACT_NG;
		r_ArcLog.strMsg.Format(IDS_ERROR_CANNOT_SET_CURRENT_DIR,(LPCTSTR)pathOutputDir);
		return false;
	}

	TRACE(_T("Archive Handler �Ăяo��\n"));
	//------------
	// �𓀂��s��
	//------------
	if(!lpArchiver->IsUnicodeCapable() && !UtilCheckT2A(pathOutputDir)){
		//UNICODE�ɑΉ����Ă��Ȃ��̂�UNICODE�t�@�C�����̃t�H���_�ɓW�J���悤�Ƃ���
		bRet=false;
		r_ArcLog.Result=EXTRACT_NG;
		r_ArcLog.strMsg=CString(MAKEINTRESOURCE(IDS_ERROR_UNICODEPATH));
	}else{
		CString strLog;
		bRet=lpArchiver->Extract(lpArcFile,ConfMan,ConfExtract,bSafeArchive,pathOutputDir,strLog);

		//---���O�f�[�^
		r_ArcLog.Result=(bRet?EXTRACT_OK:EXTRACT_NG);
		r_ArcLog.strMsg=strLog;
	}
	if(ConfGeneral.NotifyShellAfterProcess){
		//�𓀊�����ʒm
		::SHChangeNotify(SHCNE_UPDATEDIR,SHCNF_PATH,pathOutputDir,NULL);
	}
	return bRet;
}

bool DeleteOriginalArchives(const CConfigExtract &ConfExtract,LPCTSTR lpszArcFile)
{
	std::list<CString> fileList;	//�폜�Ώۂ̃t�@�C���ꗗ

	//---�}���`�{�����[���Ȃ�܂Ƃ߂č폜
	CString strFindParam;
	bool bMultiVolume=false;
	if(ConfExtract.DeleteMultiVolume){	//�}���`�{�����[���ł̍폜���L�����H
		bMultiVolume=UtilIsMultiVolume(lpszArcFile,strFindParam);
	}

	CString strFiles;	//�t�@�C���ꗗ

	if(bMultiVolume){
		UtilPathExpandWild(fileList,strFindParam);
		for(std::list<CString>::iterator ite=fileList.begin();ite!=fileList.end();++ite){
			strFiles+=_T("\n");
			strFiles+=*ite;
		}
	}else{
		fileList.push_back(lpszArcFile);
	}

	//�폜����
	if(ConfExtract.MoveToRecycleBin){
		//--------------
		// ���ݔ��Ɉړ�
		//--------------
		if(!ConfExtract.DeleteNoConfirm){	//�폜�m�F����ꍇ
			CString Message;
			if(bMultiVolume){
				//�}���`�{�����[��
				Message.Format(IDS_ASK_MOVE_ARCHIVE_TO_RECYCLE_BIN_MANY);
				Message+=strFiles;
			}else{
				//�P��t�@�C��
				Message.Format(IDS_ASK_MOVE_ARCHIVE_TO_RECYCLE_BIN,lpszArcFile);
			}

			//�m�F��S�~���Ɉړ�
			if(IDYES!=MessageBox(NULL,Message,UtilGetMessageCaption(),MB_YESNO|MB_ICONQUESTION|MB_DEFBUTTON2)){
				return false;
			}
		}

		//�폜���s
		UtilMoveFileToRecycleBin(fileList);
		return true;
	}else{
		//------
		// �폜
		//------
		if(!ConfExtract.DeleteNoConfirm){	//�m�F����ꍇ
			CString Message;
			if(bMultiVolume){
				//�}���`�{�����[��
				Message.Format(IDS_ASK_DELETE_ARCHIVE_MANY);
				Message+=strFiles;
			}else{
				//�P��t�@�C��
				Message.Format(IDS_ASK_DELETE_ARCHIVE,lpszArcFile);
			}
			if(IDYES!=MessageBox(NULL,Message,UtilGetMessageCaption(),MB_YESNO|MB_ICONQUESTION|MB_DEFBUTTON2)){
				return false;
			}
		}
		//�폜���s
		for(std::list<CString>::iterator ite=fileList.begin();ite!=fileList.end();++ite){
			DeleteFile(*ite);
		}
		return true;
	}
}


// �A�[�J�C�u������t�H���_�p�X���쐬����
void GetArchiveDirectoryPath(const CConfigExtract &ConfExtract,LPCTSTR lpszArcName,CPath &pathDir)
{
	pathDir=lpszArcName;
	pathDir.StripPath();	//�p�X������t�@�C�������擾
	pathDir.RemoveExtension();//�A�[�J�C�u�t�@�C��������g���q���폜

	//�t�H���_�������̐����ƋL������菜��
	if(ConfExtract.RemoveSymbolAndNumber){
		CPath pathOrg=pathDir;
		UtilTrimString(pathDir,g_szTable);
		//�����ƋL������菜�������ʁA�����񂪋�ɂȂ��Ă��܂��Ă����猳�ɂ��ǂ�
		if(_tcslen(pathDir)==0){
			pathDir=pathOrg;
		}
	}

	//�󔒂���菜��
	UtilTrimString(pathDir,_T(".\\ �@"));

	if(_tcslen(pathDir)>0){
		pathDir.AddBackslash();
	}
}

/*************************************************************
�o�͐�t�H���_�����肷��
�o�͐�t�H���_���d�ɍ쐬���Ȃ��悤�AbInFolder�����ł��ׂ�
�t�H���_�ɓ����Ă��邩�ǂ����m�F����
*************************************************************/
HRESULT GetExtractDestDir(LPCTSTR ArcFileName,const CConfigGeneral &ConfGeneral,const CConfigExtract &ConfExtract,LPCTSTR lpSpecificOutputDir,bool bInFolder,CPath &r_pathOutputDir,const int nItemCount,LPCTSTR lpszBaseDir,CPath &r_pathOpenDir,bool &r_bUseForAll/*�ȍ~�����̃t�H���_�ɉ𓀂���Ȃ�true���Ԃ�*/,CString &strErr)
{
	CPath pathOutputDir;

	if(lpSpecificOutputDir && 0!=_tcslen(lpSpecificOutputDir)){
		//����f�B���N�g�����o�͐�Ƃ��Ďw�肳��Ă����ꍇ
		pathOutputDir=lpSpecificOutputDir;
	}else{
		//�ݒ�����ɏo�͐�����߂�
		HRESULT hr=GetOutputDirPathFromConfig(ConfExtract.OutputDirType,ArcFileName,ConfExtract.OutputDir,pathOutputDir,r_bUseForAll,strErr);
		if(FAILED(hr)){
			return hr;
		}
	}
	pathOutputDir.AddBackslash();

	// �o�͐悪�l�b�g���[�N�h���C�u/�����[�o�u���f�B�X�N�ł���Ȃ�x��
	// �o�͐悪���݂��Ȃ��Ȃ�A�쐬�m�F
	HRESULT hStatus=ConfirmOutputDir(ConfGeneral,pathOutputDir,strErr);
	if(FAILED(hStatus)){
		//�L�����Z���Ȃ�
		return hStatus;
	}

	//�A�[�J�C�u������t�H���_�����
	CPath pathArchiveNamedDir;	//�A�[�J�C�u���̃t�H���_
	bool bCreateArchiveDir=false;	//�A�[�J�C�u���̃t�H���_���쐬����ꍇ�ɂ�true
	if(
		(!ConfExtract.CreateNoFolderIfSingleFileOnly || nItemCount!=1)&&
			(
				((CREATE_OUTPUT_DIR_SINGLE==ConfExtract.CreateDir)&&!bInFolder)||
				 (CREATE_OUTPUT_DIR_ALWAYS==ConfExtract.CreateDir)
			)
	){
		GetArchiveDirectoryPath(ConfExtract,ArcFileName,pathArchiveNamedDir);
		if(_tcslen(pathArchiveNamedDir)==0){
			hStatus=S_FALSE;	//NG
		}else{
			pathOutputDir+=pathArchiveNamedDir;
			bCreateArchiveDir=true;
		}
	}

	//�p�X���̒����`�F�b�N
	if(S_OK==hStatus){
		if(_tcslen(pathOutputDir)>=_MAX_PATH){
			// �p�X�������������Ƃ�
			//TODO
			ErrorMessage(CString(MAKEINTRESOURCE(IDS_ERROR_MAX_PATH)));
			hStatus=S_FALSE;
		}
	}

	//���̂܂܂ł̓t�H���_�����g���Ȃ��ꍇ
	if(S_FALSE==hStatus){
		// ���O��t���ĕۑ�
		CString title(MAKEINTRESOURCE(IDS_INPUT_TARGET_FOLDER_WITH_SHIFT));
		CLFFolderDialog dlg(NULL,title,BIF_RETURNONLYFSDIRS|BIF_NEWDIALOGSTYLE);
		if(IDOK==dlg.DoModal()){
			r_bUseForAll=(GetKeyState(VK_SHIFT)<0);	//TODO
			pathOutputDir=dlg.GetFolderPath();
			pathOutputDir.AddBackslash();
			if(bCreateArchiveDir){
				pathOutputDir+=pathArchiveNamedDir;
			}
		}else{
			//�L�����Z��
			return E_ABORT;
		}
	}

	//�o�͐��(�A�[�J�C�u���Ɠ����̃t�H���_�����܂߂�)�t�H���_�����݂��邱�Ƃ�ۏ؂�����
	if(!UtilMakeSureDirectoryPathExists(pathOutputDir)){
		strErr.Format(IDS_ERROR_CANNOT_MAKE_DIR,(LPCTSTR)pathOutputDir);
		return E_FAIL;
	}

	//�o�͐�t�H���_����Ԃ�
	r_pathOutputDir=pathOutputDir;

	//�J���p�X�̑g�ݗ���
	CPath pathToOpen=pathOutputDir;
	if(!bCreateArchiveDir){	//�A�[�J�C�u���t�H���_���쐬���Ȃ��ꍇ
		if(ConfExtract.CreateNoFolderIfSingleFileOnly && nItemCount==1){
			//-�t�@�C���E�t�H���_��������ł��邽�߃t�H���_���쐬���Ȃ�����
			//Nothing to do
		}else if(CREATE_OUTPUT_DIR_SINGLE==ConfExtract.CreateDir && bInFolder){
			//-��d�Ƀt�H���_�����Ȃ��ݒ�ɏ]���A�t�H���_���쐬���Ȃ�����
			//�A�[�J�C�u���t�H���_���J��
			pathToOpen.Append(lpszBaseDir);
			pathToOpen.AddBackslash();
		}
	}
	pathToOpen.QuoteSpaces();
	//�J���p�X��Ԃ�
	r_pathOpenDir=pathToOpen;
	return true;
}

bool Extract(std::list<CString> &ParamList,CConfigManager &ConfigManager,DLL_ID ForceDLL,LPCTSTR lpSpecificOutputDir,const CMDLINEINFO* lpCmdLineInfo)
{
	TRACE(_T("Function ::Extract() started.\n"));
	CConfigGeneral ConfGeneral;
	CConfigExtract ConfExtract;

	ConfGeneral.load(ConfigManager);
	ConfExtract.load(ConfigManager);

	//�ݒ�㏑��
	if(lpCmdLineInfo){
		if(-1!=lpCmdLineInfo->OutputToOverride){
			ConfExtract.OutputDirType=lpCmdLineInfo->OutputToOverride;
		}
		if(-1!=lpCmdLineInfo->CreateDirOverride){
			ConfExtract.CreateDir=lpCmdLineInfo->CreateDirOverride;
		}
		if(-1!=lpCmdLineInfo->DeleteAfterProcess){
			ConfExtract.DeleteArchiveAfterExtract=lpCmdLineInfo->DeleteAfterProcess;
		}
	}

	//�Z�}�t�H�ɂ��r������
	CSemaphoreLocker SemaphoreLock;
	if(ConfExtract.LimitExtractFileCount){
		SemaphoreLock.Lock(LHAFORGE_EXTRACT_SEMAPHORE_NAME,ConfExtract.MaxExtractFileCount);
	}

	UINT uFiles=ParamList.size();	//�����ɂ���t�@�C���̐�

	//�v���O���X�o�[
	CProgressDialog dlg;
	//���b�Z�[�W���[�v���񂷂��߂̃^�C�}�[
	int timer=NULL;
	if(uFiles>=2){	//�t�@�C�����������鎞�Ɍ���
		dlg.Create(NULL);
		dlg.SetTotalFileCount(uFiles);
		dlg.ShowWindow(SW_SHOW);
		timer=SetTimer(NULL,NULL,1000,UtilMessageLoopTimerProc);
	}

	//�w��̏o�͐�
	CPath pathSpecificOutputDir(lpSpecificOutputDir ? lpSpecificOutputDir : _T(""));

	std::vector<ARCLOG> LogArray;	//�������ʂ�ێ�
	bool bAllOK=true;	//���ׂĖ��Ȃ��𓀂�����true

	//�𓀏���
	for(std::list<CString>::iterator ite_param=ParamList.begin();ite_param!=ParamList.end();++ite_param){
		//�v���O���X�o�[��i�߂�
		if(dlg.IsWindow())dlg.SetNextState(*ite_param);

		ARCLOG arcLog;

		//���b�Z�[�W���[�v
		while(UtilDoMessageLoop())continue;

		//�A�[�J�C�o�n���h���擾
		//������UNICODE��Ή�DLL�Ƀ��j�R�[�h�t�@�C������n�����Ƃ����ꍇ�͒e�����B�����āA�����ł͎��s�̌������𖾂ł��Ȃ�
		CArchiverDLL *lpArchiver=CArchiverDLLManager::GetInstance().GetArchiver(*ite_param,ConfExtract.DenyExt,ForceDLL);
		if(!lpArchiver){
			//�Ή�����n���h�����Ȃ�����
			arcLog.Result=EXTRACT_NOTARCHIVE;
			arcLog.strMsg.Format(IDS_ERROR_ILLEGAL_HANDLER,(LPCTSTR)*ite_param);
			arcLog.strFile=*ite_param;
			bAllOK=false;
			LogArray.push_back(arcLog);
			continue;
		}

		CPath pathOpenDir;		//�t�@�C�����J���ׂ��t�H���_
		//�𓀎��s
		bool bRet=ExtractOneArchive(ConfigManager,ConfGeneral,ConfExtract,lpArchiver,*ite_param,pathSpecificOutputDir,arcLog,pathOpenDir);
		//���O�ۑ�
		LogArray.push_back(arcLog);

		if(!bRet){
			bAllOK=false;
		}else{
			//�o�͐�t�H���_���J��
			if(ConfExtract.OpenDir){
				if(ConfGeneral.Filer.UseFiler){
					//�p�����[�^�W�J�ɕK�v�ȏ��
					std::map<stdString,CString> envInfo;
					MakeExpandInformationEx(envInfo,pathOpenDir,NULL);

					//�R�}���h�E�p�����[�^�W�J
					CString strCmd,strParam;
					UtilExpandTemplateString(strCmd,ConfGeneral.Filer.FilerPath,envInfo);	//�R�}���h
					UtilExpandTemplateString(strParam,ConfGeneral.Filer.Param,envInfo);	//�p�����[�^
					ShellExecute(NULL, _T("open"), strCmd,strParam, NULL, SW_SHOWNORMAL);
				}else{
					//Explorer�ŊJ��
					UtilNavigateDirectory(pathOpenDir);
				}
			}

			//����ɉ𓀂ł������k�t�@�C�����폜or���ݔ��Ɉړ�
			if(bRet && ConfExtract.DeleteArchiveAfterExtract){
				if(!ConfExtract.ForceDelete && lpArchiver->IsWeakErrorCheck()){
					//�G���[�`�F�b�N�@�\���n��Ȃ��߁A�𓀎��s���ɂ�����Ɣ��f���Ă��܂��悤��
					//DLL���g�����Ƃ��ɂ͖����I�Ɏw�肵�Ȃ�����폜�����Ȃ�
					MessageBox(NULL,CString(MAKEINTRESOURCE(IDS_MESSAGE_EXTRACT_DELETE_SKIPPED)),UtilGetMessageCaption(),MB_OK|MB_ICONINFORMATION);
				}else{
					//�폜
					DeleteOriginalArchives(ConfExtract,*ite_param);
				}
			}
		}
	}
	//�v���O���X�o�[�����
	if(dlg.IsWindow())dlg.DestroyWindow();
	//�^�C�}�[�����
	if(timer)KillTimer(NULL,timer);

	//---���O�\��
	switch(ConfGeneral.LogViewEvent){
	case LOGVIEW_ON_ERROR:
		if(!bAllOK){
			if(1==uFiles){
				//�t�@�C��������̎��̓_�C�A���O�{�b�N�X��
				if(EXTRACT_CANCELED!=LogArray[0].Result){
					ErrorMessage(LogArray[0].strMsg);
				}
			}else{
				//���O�ɕ\��
				CLogListDialog LogDlg(CString(MAKEINTRESOURCE(IDS_LOGINFO_OPERATION_EXTRACT)));
				LogDlg.SetLogArray(LogArray);
				LogDlg.DoModal(::GetDesktopWindow());
			}
		}
		break;
	case LOGVIEW_ALWAYS:
		//���O�ɕ\��
		CLogListDialog LogDlg(CString(MAKEINTRESOURCE(IDS_LOGINFO_OPERATION_EXTRACT)));
		LogDlg.SetLogArray(LogArray);
		LogDlg.DoModal(::GetDesktopWindow());
		break;
	}

	TRACE(_T("Exit Extract()\n"));
	return bAllOK;
}

