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
#include "resource.h"
#include "main.h"
#include "ConfigCode/configwnd.h"
#include "compress.h"
#include "extract.h"
#include "TestArchive.h"
#include "ArchiverCode/arc_interface.h"
#include "FileListWindow/FileListFrame.h"
#include "ArchiverManager.h"
#include "Dialogs/SelectDlg.h"
#include "Dialogs/ProgressDlg.h"
#include "Utilities/OSUtil.h"
#include "Utilities/StringUtil.h"
#include "Update.h"
#include "CmdLineInfo.h"

CAppModule _Module;


//---------------------------------------------

//�G���g���[�|�C���g
int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE, LPTSTR lpCmdLine, int nCmdShow)
{
#if defined(_DEBUG)
	// ���������[�N���o�p
	_CrtSetDbgFlag(
		_CRTDBG_ALLOC_MEM_DF
		| _CRTDBG_LEAK_CHECK_DF
		);
#endif
	_tsetlocale(LC_ALL,_T(""));	//���P�[�������ϐ�����擾

	HRESULT hRes = ::CoInitialize(NULL);
	ATLASSERT(SUCCEEDED(hRes));
	OleInitialize(NULL);
	// �����Microsoft Layer for Unicode (MSLU) ���g�p���ꂽ����
	// ATL�E�C���h�E thunking ������������
	::DefWindowProc(NULL, 0, 0, 0L);

	// ���̃R���g���[�����T�|�[�g���邽�߂̃t���O��ǉ�
	AtlInitCommonControls(ICC_WIN95_CLASSES|ICC_COOL_CLASSES | ICC_BAR_CLASSES);
	_Module.Init(NULL,hInstance);
	CMessageLoop theLoop;
	_Module.AddMessageLoop(&theLoop);

	//*********************************
	// �R�}���h���C���p�����[�^�̉��
	//*********************************
	/*
	�I�v�V�����w��`��:

	/b2e		B2E32.dll���g�p
	/c:???		???�`���Ŏw�肳�ꂽ�t�@�C�������k(???=lzh,zip,etc...)
	/method:???	???���\�b�h���g���Ĉ��k(B2E�̂�)
	/b2esfx		���ȉ𓀌`���ň��k(B2E�̂�)
	/e			�t�@�C������
	/o:[dir]	�o�͐�f�B���N�g���w��
	/l			�A�[�J�C�u�t�@�C���̃��X�g�\��(�P��)
	/f:[file]	���k���o�̓t�@�C�����̎w��(�p�X�w��͖��������)
	/xacrett	XacRett.DLL�������I�Ɏg�p(��/�{��)
	/!			/xacrett�Ɠ���
	/m			�t�@�C�����𓀂������̓��X�g�\��(�ݒ�ɂ��ύX)
	/s			�t�@�C���E�t�H���_��������k
	/t			�A�[�J�C�u�t�@�C���̊��S�����e�X�g
	/@[file]	���X�|���X�t�@�C���w��
	/$[file]	���X�|���X�t�@�C���w��:���X�|���X�t�@�C���͓ǂݎ�芮����폜

	*/

	bool bCheckUpdate=true;	//LFCaldix���N������Ȃ�true

	CMDLINEINFO cli;	//CommandLineInfo

	CConfigManager ConfigManager;
	PROCESS_MODE ProcessMode=ParseCommandLine(ConfigManager,cli);

	{
		//�D��x�ݒ�
		CConfigGeneral ConfGeneral;
		ConfGeneral.load(ConfigManager);
		LFPROCESS_PRIORITY priority=(LFPROCESS_PRIORITY)ConfGeneral.ProcessPriority;
		//�R�}���h���C���I�v�V�����ŏ㏑��?
		if(cli.PriorityOverride!=LFPRIOTITY_DEFAULT){
			priority=cli.PriorityOverride;
		}
		switch(priority){
		case LFPRIOTITY_LOW:
			UtilSetPriorityClass(IDLE_PRIORITY_CLASS);break;
		case LFPRIOTITY_LOWER:
			UtilSetPriorityClass(BELOW_NORMAL_PRIORITY_CLASS);break;
		case LFPRIOTITY_NORMAL:
			UtilSetPriorityClass(NORMAL_PRIORITY_CLASS);break;
		case LFPRIOTITY_HIGHER:
			UtilSetPriorityClass(ABOVE_NORMAL_PRIORITY_CLASS);break;
		case LFPRIOTITY_HIGH:
			UtilSetPriorityClass(HIGH_PRIORITY_CLASS);break;
		case LFPRIOTITY_DEFAULT:
		default:
			//nothing to do
			break;
		}

		//�ꎞ�f�B���N�g���̕ύX
		CString strPath=ConfGeneral.TempPath;
		if(!strPath.IsEmpty()){
			//�p�����[�^�W�J�ɕK�v�ȏ��
			std::map<stdString,CString> envInfo;
			UtilMakeExpandInformation(envInfo);
			//���ϐ��W�J
			UtilExpandTemplateString(strPath, strPath, envInfo);

			//��΃p�X�ɕϊ�
			if(PathIsRelative(strPath)){
				CPath tmp=UtilGetModuleDirectoryPath();
				tmp.AddBackslash();
				tmp+=strPath;
				strPath=(LPCTSTR)tmp;
			}
			UtilGetCompletePathName(strPath,strPath);

			//���ϐ��ݒ�
			SetEnvironmentVariable(_T("TEMP"),strPath);
			SetEnvironmentVariable(_T("TMP"),strPath);
		}
	}

	CString strErr;
	if(PROCESS_LIST!=ProcessMode && cli.FileList.empty()){
		//�t�@�C���ꗗ�E�B���h�E���o���ꍇ�ȊO�́A�t�@�C���w�肪�����Ƃ��͐ݒ�_�C�A���O���o��
		ProcessMode=PROCESS_CONFIGURE;
	}
	CArchiverDLLManager::GetInstance().SetConfigManager(ConfigManager);

	//DLL�C���X�g�[������擾����%PATH%�ɒǉ�
	CConfigUpdate ConfUpdate;
	ConfUpdate.load(ConfigManager);
	{
		//�R�}���h�E�p�����[�^�W�J
		//---�V�������ϐ�
		CString strPath=ConfUpdate.strDLLPath;
		if(!strPath.IsEmpty()){
			//---���s���擾
			//�p�����[�^�W�J�ɕK�v�ȏ��
			std::map<stdString,CString> envInfo;
			UtilMakeExpandInformation(envInfo);
			//���ϐ��W�J
			UtilExpandTemplateString(strPath, strPath, envInfo);

			//��΃p�X�ɕϊ�
			if(PathIsRelative(strPath)){
				CPath tmp=UtilGetModuleDirectoryPath();
				tmp.AddBackslash();
				tmp+=strPath;
				strPath=(LPCTSTR)tmp;
			}
			UtilGetCompletePathName(strPath,strPath);
			strPath+=_T(";");
			//---���ϐ��擾
			int len=GetEnvironmentVariable(_T("PATH"),NULL,0);
			CString strEnv;
			GetEnvironmentVariable(_T("PATH"),strEnv.GetBuffer(len+1),len);
			strEnv.ReleaseBuffer();

			strPath+=strEnv;
			SetEnvironmentVariable(_T("PATH"),strPath);
		}
	}

	switch(ProcessMode){
	case PROCESS_COMPRESS://���k
		DoCompress(ConfigManager,cli);
		break;
	case PROCESS_EXTRACT://��
		DoExtract(ConfigManager,cli);
		break;
	case PROCESS_AUTOMATIC://���C������
		if(PathIsDirectory(*cli.FileList.begin())){
			DoCompress(ConfigManager,cli);
		}else{
			CConfigExtract ConfExtract;
			ConfExtract.load(ConfigManager);
			if(CArchiverDLLManager::GetInstance().GetArchiver(*cli.FileList.begin(),ConfExtract.DenyExt,cli.idForceDLL)){	//�𓀉\�Ȍ`�����ǂ���
				DoExtract(ConfigManager,cli);
			}else{
				DoCompress(ConfigManager,cli);
			}
		}
		break;
	case PROCESS_LIST://���X�g�\��
		DoList(ConfigManager,cli);
		break;
	case PROCESS_TEST://�A�[�J�C�u�t�@�C���̃e�X�g
		DoTest(ConfigManager,cli);
		break;
	case PROCESS_CONFIGURE://�ݒ��ʕ\��
		{
			//�����Ŋm�F���s���̂ŏI�����̊m�F�͕s�v
			bCheckUpdate=false;
			//DLL�X�V�m�F
			if(CheckUpdateArchiverDLLRequired(ConfUpdate)){
				DoUpdateArchiverDLL(ConfigManager);
			}
			//�_�C�A���O�\��
			CConfigDialog confdlg(ConfigManager);
			if(IDOK==confdlg.DoModal()){
				if(!ConfigManager.SaveConfig(strErr)){
					ErrorMessage(strErr);
				}
			}
		}
		break;
	case PROCESS_INVALID:
		TRACE(_T("Process Mode Undefined\n"));
		break;
	default:
		ASSERT(!"Unexpected Process Mode");
	}
	//�A�b�v�f�[�g�`�F�b�N
	if(bCheckUpdate){
		//DLL�X�V�m�F
		if(CheckUpdateArchiverDLLRequired(ConfUpdate)){
			DoUpdateArchiverDLL(ConfigManager);
		}
	}

	TRACE(_T("Terminating...\n"));
	CArchiverDLLManager::GetInstance().Final();
//	Sleep(1);	//�ΏǗÖ@ for 0xC0000005: Access Violation
	_Module.RemoveMessageLoop();
	_Module.Term();
	OleUninitialize();
	::CoUninitialize();
	TRACE(_T("Exit main()\n"));
	return 0;
}

/*
format�̎w��́AB2E32.dll�ł̂ݗL��
level�̎w��́AB2E32.dll�ȊO�ŗL��
*/
bool DoCompress(CConfigManager &ConfigManager,CMDLINEINFO &cli)
{
	//���k�I�v�V�����w��
	CString strFormat=cli.strFormat;
	CString strMethod=cli.strMethod;
	CString strLevel=cli.strLevel;

	if(cli.idForceDLL==DLL_ID_B2E){
		cli.CompressType=PARAMETER_B2E;
	}else{
		strFormat = cli.strSplitSize;
	}

	CConfigCompress ConfCompress;
	CConfigGeneral ConfGeneral;
	ConfCompress.load(ConfigManager);
	ConfGeneral.load(ConfigManager);

	while(PARAMETER_UNDEFINED==cli.CompressType || PARAMETER_B2E==cli.CompressType){	//---�g�pDLL������
		if(PARAMETER_UNDEFINED==cli.CompressType){	//�`�����w�肳��Ă��Ȃ��ꍇ
			if(ConfCompress.UseDefaultParameter){	//�f�t�H���g�p�����[�^���g���Ȃ�f�[�^�擾
				cli.CompressType=ConfCompress.DefaultType;
				cli.Options=ConfCompress.DefaultOptions;

				if(cli.CompressType==PARAMETER_B2E){	//B2E���g�p����ꍇ
					cli.idForceDLL=DLL_ID_B2E;	//B2E32.dll�̎g�p�𖾎�
					//�p�����[�^�w��
					if(cli.Options)cli.Options=COMPRESS_SFX;	//���ȉ�
					cli.strFormat=ConfCompress.DefaultB2EFormat;	//�`��
					cli.strMethod=ConfCompress.DefaultB2EMethod;	//���\�b�h
				}
			}else{	//���͂𑣂�
				CSelectDialog SelDlg;
				SelDlg.SetDeleteAfterCompress(BOOL2bool(ConfCompress.DeleteAfterCompress));
				cli.CompressType=(PARAMETER_TYPE)SelDlg.DoModal();
				if(PARAMETER_UNDEFINED==cli.CompressType){	//�L�����Z���̏ꍇ
					return false;
				}else if(cli.CompressType!=PARAMETER_B2E){
					cli.idForceDLL=DLL_ID_UNKNOWN;

					cli.Options=SelDlg.GetOptions();
					cli.bSingleCompression=SelDlg.IsSingleCompression();
					cli.DeleteAfterProcess=SelDlg.GetDeleteAfterCompress() ? 1 : 0;
					break;
				}
			}
		}
		if(cli.CompressType==PARAMETER_B2E){	//B2E���g�p����ꍇ
			//---B2E32.dll�̃`�F�b�N
			CArchiverB2E &B2EHandler=CArchiverDLLManager::GetInstance().GetB2EHandler();
			if(!B2EHandler.IsOK()){
				cli.CompressType=PARAMETER_UNDEFINED;
				CString msg;
				msg.Format(IDS_ERROR_DLL_LOAD,B2EHandler.GetName());
				ErrorMessage(msg);
				continue;
//				return false;
			}

			//---�`���I��
			if(cli.strFormat.IsEmpty()){
				CB2ESelectDialog SelDlg;
				INT_PTR Ret=SelDlg.DoModal();
				if(IDCANCEL==Ret){	//�L�����Z���̏ꍇ
					return false;
				}else if(IDC_COMPRESS_USENORMAL==Ret){	//�ʏ��DLL���g��
					cli.CompressType=PARAMETER_UNDEFINED;
				}else{
					cli.idForceDLL=DLL_ID_B2E;

					cli.Options=SelDlg.IsSFX() ? COMPRESS_SFX : 0;
					cli.bSingleCompression=SelDlg.IsSingleCompression();
					strFormat=SelDlg.GetFormat();
					strMethod=SelDlg.GetMethod();
					break;
				}
			}else{
				strFormat=cli.strFormat;
				strMethod=cli.strMethod;
				break;
			}
		}
	}

	//--------------------
	// ���k���

	if(cli.bSingleCompression){	//�t�@�C����������k
		//���b�Z�[�W���[�v���񂷂��߂̃^�C�}�[
		int timer=SetTimer(NULL,NULL,1000,UtilMessageLoopTimerProc);
		//�v���O���X�o�[
		CProgressDialog dlg;
		int nFiles=cli.FileList.size();
		if(nFiles>=2){	//�t�@�C�����������鎞�Ɍ���
			dlg.Create(NULL);
			dlg.SetTotalFileCount(nFiles);
			dlg.ShowWindow(SW_SHOW);
		}
		bool bRet=true;
		for(std::list<CString>::iterator ite=cli.FileList.begin();ite!=cli.FileList.end();ite++){
			//�v���O���X�o�[��i�߂�
			if(dlg.IsWindow())dlg.SetNextState(*ite);
			while(UtilDoMessageLoop())continue;

			//���k���
			std::list<CString> TempList;
			TempList.push_back(*ite);

			bRet=bRet && Compress(TempList,cli.CompressType,ConfigManager,strFormat,strMethod,strLevel,cli);
		}
		//�v���O���X�o�[�����
		if(dlg.IsWindow())dlg.DestroyWindow();

		//�^�C�}�[�����
		KillTimer(NULL,timer);
		return bRet;
	}else{	//�ʏ툳�k
		return Compress(cli.FileList,cli.CompressType,ConfigManager,strFormat,strMethod,strLevel,cli);
	}
}

bool DoExtract(CConfigManager &ConfigManager,CMDLINEINFO &cli)
{
	CConfigExtract ConfExtract;
	ConfExtract.load(ConfigManager);
	MakeListFilesOnly(cli.FileList,cli.idForceDLL,ConfExtract.DenyExt,true);
	if(cli.FileList.empty()){
		ErrorMessage(CString(MAKEINTRESOURCE(IDS_ERROR_FILE_NOT_SPECIFIED)));
		return false;
	}
	return Extract(cli.FileList,ConfigManager,cli.idForceDLL,cli.OutputDir,&cli);
}

bool DoList(CConfigManager &ConfigManager,CMDLINEINFO &cli)
{
	CConfigExtract ConfExtract;
	ConfExtract.load(ConfigManager);
	bool bSpecified=!cli.FileList.empty();
	MakeListFilesOnly(cli.FileList,cli.idForceDLL,ConfExtract.DenyExt,true);
	//�t�@�C�����X�g�ɉ����c��Ȃ�������G���[���b�Z�[�W�\��
	if(bSpecified && cli.FileList.empty()){
		ErrorMessage(CString(MAKEINTRESOURCE(IDS_ERROR_FILE_NOT_SPECIFIED)));
	//	return false;
	}

//==========
// �{���J�n
//==========
	CFileListFrame ListWindow(ConfigManager);
	ListWindow.CreateEx();
	ListWindow.ShowWindow(SW_SHOW);
	ListWindow.UpdateWindow();
	//ListWindow.AddArchiveFile(*cli.FileList.begin());
	if(!cli.FileList.empty()){
		bool bAllFailed=true;
		std::list<CString>::iterator ite=cli.FileList.begin();
		const std::list<CString>::iterator End=cli.FileList.end();
		for(;ite!=cli.FileList.end();++ite){
			HRESULT hr=ListWindow.OpenArchiveFile(*ite,cli.idForceDLL);
			if(SUCCEEDED(hr)){
				if(hr!=S_FALSE)bAllFailed=false;
			}else if(hr==E_ABORT){
				break;
			}
		}
		if(bAllFailed)ListWindow.DestroyWindow();
	}

	CMessageLoop* pLoop = _Module.GetMessageLoop();
	pLoop->Run();
	TRACE(_T("Loop End\n"));
	return true;
}

//�A�[�J�C�u�̃e�X�g
bool DoTest(CConfigManager &ConfigManager,CMDLINEINFO &cli)
{
	CConfigExtract ConfExtract;
	ConfExtract.load(ConfigManager);
	//�S�Ẵt�@�C���������Ώۂɂ���
	MakeListFilesOnly(cli.FileList,cli.idForceDLL,ConfExtract.DenyExt,false);
	//�t�@�C�����X�g�ɉ����c��Ȃ�������G���[���b�Z�[�W�\��
	if(cli.FileList.empty()){
		ErrorMessage(CString(MAKEINTRESOURCE(IDS_ERROR_FILE_NOT_SPECIFIED)));
		return false;
	}

	//�e�X�g
	return TestArchive(cli.FileList,ConfigManager);
}

//���X�g����t�H���_���폜���A�T�u�t�H���_�̃t�@�C����ǉ�
void MakeListFilesOnly(std::list<CString> &FileList,DLL_ID idForceDLL,LPCTSTR lpDenyExt,bool bArchivesOnly)
{
	std::list<CString>::iterator ite;
	for(ite=FileList.begin();ite!=FileList.end();){
		if(PathIsDirectory(*ite)){
			//---�𓀑Ώۂ��t�H���_�Ȃ�ċA�𓀂���
			std::list<CString> subFileList;
			UtilRecursiveEnumFile(*ite,subFileList);

			for(std::list<CString>::iterator ite2=subFileList.begin();ite2!=subFileList.end();ite2++){
				if(!bArchivesOnly||CArchiverDLLManager::GetInstance().GetArchiver(*ite2,lpDenyExt,idForceDLL)){
					//�Ή����Ă���`���̂ݒǉ�����K�v�����鎞�́A�𓀉\�Ȍ`�����ǂ������肷��
					FileList.push_back(*ite2);
				}
			}
			//�����͍폜
			ite=FileList.erase(ite);
		}
		else{
			ite++;
		}
	}
}
