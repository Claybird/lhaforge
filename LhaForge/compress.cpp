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
#include "compress.h"
#include "Dialogs/LogDialog.h"

#include "resource.h"

#include "archivermanager.h"
#include "Utilities/Semaphore.h"
#include "Utilities/StringUtil.h"
#include "Utilities/FileOperation.h"
#include "Utilities/OSUtil.h"
#include "ConfigCode/ConfigManager.h"
#include "ConfigCode/ConfigCompress.h"
#include "ConfigCode/ConfigGeneral.h"
#include "CommonUtil.h"
#include "CmdLineInfo.h"

bool DeleteOriginalFiles(const CConfigCompress &ConfCompress,const std::list<CString>& fileList);

bool Compress(const std::list<CString> &_ParamList,const PARAMETER_TYPE Type,CConfigManager &ConfigManager,LPCTSTR lpszFormat,LPCTSTR lpszMethod,LPCTSTR lpszLevel,CMDLINEINFO& CmdLineInfo)
{
	TRACE(_T("Function ::Compress() started.\n"));

	std::list<CString> ParamList=_ParamList;

	CArchiverDLLManager &ArchiverManager=CArchiverDLLManager::GetInstance();
	CConfigCompress ConfCompress;
	ConfCompress.load(ConfigManager);
	CConfigGeneral ConfGeneral;
	ConfGeneral.load(ConfigManager);

	int Options=CmdLineInfo.Options;
	//---�ݒ�㏑��
	//�o�͐�㏑��
	if(-1!=CmdLineInfo.OutputToOverride){
		ConfCompress.OutputDirType=CmdLineInfo.OutputToOverride;
	}
	if(-1!=CmdLineInfo.IgnoreTopDirOverride){
		ConfCompress.IgnoreTopDirectory=CmdLineInfo.IgnoreTopDirOverride;
	}
	if(-1!=CmdLineInfo.DeleteAfterProcess){
		ConfCompress.DeleteAfterCompress=CmdLineInfo.DeleteAfterProcess;
	}

	CArchiverDLL *lpArchiver=ArchiverManager.GetArchiver(GetDllIDFromParameterType(Type));
	if(!lpArchiver || !lpArchiver->IsOK()){
		return false;
	}

	//�f�B���N�g�����Ƀt�@�C�����S�ē����Ă��邩�̔���
	if(ConfCompress.IgnoreTopDirectory){
		//NOTE: this code equals to ParamList.size()==1
		std::list<CString>::const_iterator ite=ParamList.begin();
		ite++;
		if(ite==ParamList.end()){
			//�t�@�C����������Ȃ�
			CPath path=*ParamList.begin();
			path.AddBackslash();
			if(path.IsDirectory()){
				//�P�̂̃f�B���N�g�������k
				UtilPathExpandWild(ParamList,path+_T("*"));
			}
			if(ParamList.empty()){
				ParamList=_ParamList;
			}
		}
	}

	//UNICODE�Ή��̃`�F�b�N
	if(!lpArchiver->IsUnicodeCapable()){		//UNICODE�ɑΉ����Ă��Ȃ�
		if(!UtilCheckT2AList(ParamList)){
			//�t�@�C������UNICODE���������t�@�C�������k���悤�Ƃ���
			ErrorMessage(CString(MAKEINTRESOURCE(IDS_ERROR_UNICODEPATH)));
			return false;
		}
	}

	//----------------------------------------------
	// GZ/BZ2/JACK�ň��k�\(�P�t�@�C��)���ǂ����`�F�b�N
	//----------------------------------------------
	if(PARAMETER_GZ==Type||PARAMETER_BZ2==Type||PARAMETER_XZ==Type||PARAMETER_LZMA==Type||PARAMETER_JACK==Type){
		int nFileCount=0;
		CPath pathFileName;
		for(std::list<CString>::iterator ite=ParamList.begin();ite!=ParamList.end();++ite){
			nFileCount+=FindFileToCompress(*ite,pathFileName);
			if(nFileCount>=2){
				ErrorMessage(CString(MAKEINTRESOURCE(IDS_ERROR_SINGLE_FILE_ONLY)));
				return false;
			}
		}
		if(0==nFileCount){
			ErrorMessage(CString(MAKEINTRESOURCE(IDS_ERROR_FILE_NOT_SPECIFIED)));
			return false;
		}
		ParamList.clear();
		ParamList.push_back(pathFileName);
	}else if(PARAMETER_ISH==Type||PARAMETER_UUE==Type){	// ISH/UUE�ŕϊ��\��(�t�@�C�����ǂ���)�`�F�b�N
		for(std::list<CString>::iterator ite=ParamList.begin();ite!=ParamList.end();++ite){
			if(PathIsDirectory(*ite)){
				ErrorMessage(CString(MAKEINTRESOURCE(IDS_ERROR_CANT_COMPRESS_FOLDER)));
				return false;
			}
		}
	}

	//=========================================================
	// �J�����g�f�B���N�g���ݒ�̂��߂�\�ŏI����_�p�X���擾
	//=========================================================
	CString pathBase;
	UtilGetBaseDirectory(pathBase,ParamList);
	TRACE(pathBase),TRACE(_T("\n"));

	//�J�����g�f�B���N�g���ݒ�
	SetCurrentDirectory(pathBase);

	//���k��t�@�C��������
	TRACE(_T("���k��t�@�C��������\n"));
	CPath pathArcFileName;
	CString strErr;
	HRESULT hr=GetArchiveName(pathArcFileName,_ParamList,Type,Options,ConfCompress,ConfGeneral,CmdLineInfo,lpArchiver->IsUnicodeCapable(),strErr);
	if(FAILED(hr)){
		if(hr!=E_ABORT){
			ErrorMessage(strErr);
		}
		return false;
	}

	//====================================================================
	// ���Ή�DLL�Ńt�@�C������UNICODE���������t�@�C�������k���悤�Ƃ���
	//====================================================================
	if(!lpArchiver->IsUnicodeCapable()&&!UtilCheckT2A(pathArcFileName)){
		//GetArchiveName�Œe���؂�Ȃ���������(B2E/JACK�Ȃ�)�������Œe��
		//B2E�͕ʌn���ŏ�������Ă��邪�A�O�̂��߁B
		ErrorMessage(CString(MAKEINTRESOURCE(IDS_ERROR_UNICODEPATH)));
		return false;
	}

	if(PARAMETER_JACK!=Type){
		if(PathFileExists(pathArcFileName)){
			if(!DeleteFile(pathArcFileName)){
				//�폜�ł��Ȃ�����
				CString strLastError;
				UtilGetLastErrorMessage(strLastError);
				CString msg;
				msg.Format(IDS_ERROR_FILE_REPLACE,(LPCTSTR)strLastError);

				ErrorMessage(msg);
				return false;
			}
		}

		//======================================
		// ���X�|���X�t�@�C�����ɋL�����邽�߂�
		// ���k�Ώۃt�@�C�������C������
		//======================================
		const int nBasePathLength=pathBase.GetLength();
		for(std::list<CString>::iterator ite=ParamList.begin();ite!=ParamList.end();++ite){
			//�x�[�X�p�X�����ɑ��΃p�X�擾 : ���ʂł�����p�X�̕������������J�b�g����
			//(*ite)=(*ite).Right((*ite).GetLength()-BasePathLength);
			//(*ite).Delete(0,nBasePathLength);
			(*ite)=(LPCTSTR)(*ite)+nBasePathLength;

			//����BasePath�Ɠ����ɂȂ��ċ�ɂȂ��Ă��܂����p�X������΁A�J�����g�f�B���N�g���w������ɓ���Ă���
			if((*ite).IsEmpty()){
				*ite=_T(".");
			}
		}
	}

	//�Z�}�t�H�ɂ��r������
	CSemaphoreLocker SemaphoreLock;
	if(ConfCompress.LimitCompressFileCount){
		SemaphoreLock.Lock(LHAFORGE_COMPRESS_SEMAPHORE_NAME,ConfCompress.MaxCompressFileCount);
	}


	TRACE(_T("ArchiverHandler �Ăяo��\n"));
	//------------
	// ���k���s��
	//------------
	CString strLog;
	/*
	format�̎w��́AB2E32.dll�ł̂ݗL��
	level�̎w��́AB2E32.dll�ȊO�ŗL��
	*/
	bool bRet=lpArchiver->Compress(pathArcFileName,ParamList,ConfigManager,Type,Options,lpszFormat,lpszMethod,lpszLevel,strLog);
	//---���O�\��
	switch(ConfGeneral.LogViewEvent){
	case LOGVIEW_ON_ERROR:
		if(!bRet){
			CLogDialog LogDlg;
			LogDlg.SetData(strLog);
			LogDlg.DoModal();
		}
		break;
	case LOGVIEW_ALWAYS:
		CLogDialog LogDlg;
		LogDlg.SetData(strLog);
		LogDlg.DoModal();
		break;
	}

	if(ConfGeneral.NotifyShellAfterProcess){
		//���k������ʒm
		::SHChangeNotify(SHCNE_CREATE,SHCNF_PATH,pathArcFileName,NULL);
	}

	//�o�͐�t�H���_���J��
	if(bRet && ConfCompress.OpenDir){
		CPath pathOpenDir=pathArcFileName;
		pathOpenDir.RemoveFileSpec();
		pathOpenDir.AddBackslash();
		if(ConfGeneral.Filer.UseFiler){
			//�p�����[�^�W�J�ɕK�v�ȏ��
			std::map<stdString,CString> envInfo;
			MakeExpandInformationEx(envInfo,pathOpenDir,pathArcFileName);

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

	//����Ɉ��k�ł����t�@�C�����폜or���ݔ��Ɉړ�
	if(bRet && ConfCompress.DeleteAfterCompress){
		if(!ConfCompress.ForceDelete && lpArchiver->IsWeakErrorCheck()){
			//�G���[�`�F�b�N�@�\���n��Ȃ��߁A���k���s���ɂ�����Ɣ��f���Ă��܂��悤��
			//DLL���g�����Ƃ��ɂ͖����I�Ɏw�肵�Ȃ�����폜�����Ȃ�
			MessageBox(NULL,CString(MAKEINTRESOURCE(IDS_MESSAGE_COMPRESS_DELETE_SKIPPED)),UtilGetMessageCaption(),MB_OK|MB_ICONINFORMATION);
		}else{
			//�J�����g�f�B���N�g���͍폜�ł��Ȃ��̂ŕʂ̏ꏊ�ֈړ�
			::SetCurrentDirectory(UtilGetModuleDirectoryPath());
			//�폜:�I���W�i���̎w��ŏ���
			DeleteOriginalFiles(ConfCompress,_ParamList);
		}
	}


	TRACE(_T("Exit Compress()\n"));
	return bRet;
}

void GetArchiveFileExtension(PARAMETER_TYPE type,LPCTSTR lpszOrgFile,CString &rExt,LPCTSTR lpszDefaultExt)
{
	switch(type){
	case PARAMETER_GZ:
	case PARAMETER_BZ2:
	case PARAMETER_XZ:
	case PARAMETER_LZMA:
		rExt=PathFindExtension(lpszOrgFile);
		rExt+=lpszDefaultExt;
		break;
	case PARAMETER_JACK:
		//JACK�̏ꍇ�ɂ̓A�[�J�C�u�t�@�C�������o�͐�t�H���_���ɂȂ�
		//:�t�@�C������DLL����������
		rExt.Empty();
		break;
	default:
		rExt=lpszDefaultExt;
		break;
	}
}

HRESULT CheckArchiveName(LPCTSTR lpszArcFile,const std::list<CString> &rOrgFileList,bool bOverwrite,bool bUnicodeCapable,CString &strErr)
{
	// �t�@�C���������������Ƃ�
	if(_tcslen(lpszArcFile)>=_MAX_PATH){
		strErr=CString(MAKEINTRESOURCE(IDS_ERROR_MAX_PATH));
		return E_LF_CANNOT_DECIDE_FILENAME;
	}

	// �ł����������t�@�C���������͌��̃t�@�C�����Ɠ�����
	std::list<CString>::const_iterator ite,end;
	end=rOrgFileList.end();
	for(ite=rOrgFileList.begin();ite!=end;++ite){
		if(0==(*ite).CompareNoCase(lpszArcFile)){
			strErr.Format(IDS_ERROR_SAME_INPUT_AND_OUTPUT,lpszArcFile);
			return E_LF_OVERWRITE_SOURCE;
		}
	}

	// ���Ή�DLL��UNICODE�t�@�C������n���Ă��Ȃ���
	if(!bUnicodeCapable && !UtilCheckT2A(lpszArcFile)){
		strErr=CString(MAKEINTRESOURCE(IDS_ERROR_UNICODEPATH));
		return E_LF_CANNOT_DECIDE_FILENAME;
	}

	// �t�@�C�������ɑ��݂��邩�ǂ���
	if(!bOverwrite){
		if(PathFileExists(lpszArcFile)){
			// �t�@�C�������݂������肠��
			// ���̏ꍇ�̓G���[���b�Z�[�W���o�����Ƀt�@�C��������͂�����
			return S_LF_ARCHIVE_EXISTS;
		}
	}

	return S_OK;
}

int FindCompressionParamIndex(PARAMETER_TYPE type,int Options)
{
	//���k�`���̏�������
	for(int nIndex=0;nIndex<COMPRESS_PARAM_COUNT;nIndex++){
		if(type==CompressParameterArray[nIndex].Type){
			if(Options==CompressParameterArray[nIndex].Options){
				return nIndex;
			}
		}
	}
	return -1;
}

//�㏑���m�F�Ȃ�
HRESULT ConfirmOutputFile(CPath &r_pathArcFileName,const std::list<CString> &rOrgFileNameList,LPCTSTR lpExt,BOOL bAskName,bool bUnicodeCapable)
{
	//----------------------------
	// �t�@�C�����w�肪�K�؂��m�F
	//----------------------------
	bool bForceOverwrite=false;
	BOOL bInputFilenameFirst=bAskName;//Config.Common.Compress.SpecifyOutputFilename;

	HRESULT hStatus=E_UNEXPECTED;
	while(S_OK!=hStatus){
		if(!bInputFilenameFirst){
			//�t�@�C�����̒����ȂǊm�F
			CString strLocalErr;
			hStatus=CheckArchiveName(r_pathArcFileName,rOrgFileNameList,bForceOverwrite,bUnicodeCapable,strLocalErr);
			if(FAILED(hStatus)){
				ErrorMessage(strLocalErr);
			}
			if(S_OK==hStatus){
				break;	//����
			}
		}

		//�ŏ��̃t�@�C�����͉������������̂��g��
		//�I�����ꂽ�t�@�C�������Ԃ��Ă���
		CPath pathFile;
		if(E_LF_CANNOT_DECIDE_FILENAME==hStatus){
			//�t�@�C����������ł��Ȃ��ꍇ�A�J�����g�f�B���N�g�������s�p�X�Ƃ���
			TCHAR szBuffer[_MAX_PATH+1]={0};
			GetCurrentDirectory(_MAX_PATH,szBuffer);
			pathFile=szBuffer;
			pathFile.Append(CString(MAKEINTRESOURCE(IDS_UNTITLED)) + lpExt);
		}else{
			pathFile=r_pathArcFileName;
		}

		//==================
		// ���O��t���ĕۑ�
		//==================
		//�t�B���^�����񐶐�
		CString strFilter(MAKEINTRESOURCE(IDS_COMPRESS_FILE));
		strFilter.AppendFormat(_T(" (*%s)|*%s"),lpExt,lpExt);
		TCHAR filter[_MAX_PATH+2]={0};
		UtilMakeFilterString(strFilter,filter,COUNTOF(filter));

		CFileDialog dlg(FALSE, lpExt, pathFile, OFN_OVERWRITEPROMPT|OFN_NOCHANGEDIR,filter);
		if(IDCANCEL==dlg.DoModal()){	//�L�����Z��
			return E_ABORT;
		}

		r_pathArcFileName=dlg.m_szFileName;
		//DELETED:�t�@�C�����̃����O�p�X���擾

		bForceOverwrite=true;
		bInputFilenameFirst=false;
	}
	return S_OK;
}

/*************************************************************
�A�[�J�C�u�t�@�C���������肷��B

 ��{�I�ɂ́A�R�}���h���C�������ŗ^������t�@�C�����̂����A
�ŏ��̃t�@�C���������ɂ��āA�g���q������ς����t�@�C�������B
*************************************************************/
HRESULT GetArchiveName(CPath &r_pathArcFileName,const std::list<CString> &OrgFileNameList,const PARAMETER_TYPE Type,const int Options,const CConfigCompress &ConfCompress,const CConfigGeneral &ConfGeneral,CMDLINEINFO &rCmdLineInfo,bool bUnicodeCapable,CString &strErr)
{
	ASSERT(!OrgFileNameList.empty());

	//==================
	// �t�@�C�����̐���
	//==================
	BOOL bDirectory=FALSE;	//���k�ΏۂɎw�肳�ꂽ�����t�H���_�Ȃ�true;�t�H���_�ƃt�@�C���Ŗ�����'.'�̈�����ς���
	BOOL bDriveRoot=FALSE;	//���k�Ώۂ��h���C�u�̃��[�g(X:\ etc.)�Ȃ�true
	CPath pathOrgFileName;


	bool bFileNameSpecified=(0<rCmdLineInfo.OutputFileName.GetLength());

	//---���̃t�@�C����
	if(bFileNameSpecified){
		//TODO:�����ŏ�������̂͂�������
		pathOrgFileName=(LPCTSTR)rCmdLineInfo.OutputFileName;
		pathOrgFileName.StripPath();	//�p�X������菜��
	}else{
		//�擪�̃t�@�C���������Ɏb��I�Ƀt�@�C���������o��
		pathOrgFileName=(LPCTSTR)*OrgFileNameList.begin();

		//---����
		//�f�B���N�g�����ǂ���
		bDirectory=pathOrgFileName.IsDirectory();
		//�h���C�u�̃��[�g���ǂ���
		CPath pathRoot=pathOrgFileName;
		pathRoot.AddBackslash();
		bDriveRoot=pathRoot.IsRoot();
		if(bDriveRoot){
			//�{�����[�����擾
			TCHAR szVolume[MAX_PATH];
			GetVolumeInformation(pathRoot,szVolume,MAX_PATH,NULL,NULL,NULL,NULL,0);
			//�h���C�u���ۂ��ƈ��k����ꍇ�ɂ́A�{�����[�������t�@�C�����Ƃ���
			UtilFixFileName(pathOrgFileName,szVolume,_T('_'));
		}
	}

	//���k�`���̏�������
	int nIndex=FindCompressionParamIndex(Type,Options);
	if(-1==nIndex){
		strErr=CString(MAKEINTRESOURCE(IDS_ERROR_ILLEGAL_FORMAT_TYPE));
		return E_FAIL;
	}

	//�g���q����
	CString strExt;
	if(bFileNameSpecified){
		strExt=PathFindExtension(pathOrgFileName);
	}else{
		GetArchiveFileExtension(Type,pathOrgFileName,strExt,CompressParameterArray[nIndex].Ext);
	}

	//�o�͐�t�H���_��
	CPath pathOutputDir;
	if(0!=rCmdLineInfo.OutputDir.GetLength()){
		//�o�͐�t�H���_���w�肳��Ă���ꍇ
		pathOutputDir=(LPCTSTR)rCmdLineInfo.OutputDir;
	}else{
		//�ݒ�����ɏo�͐�����߂�
		bool bUseForAll;
		HRESULT hr=GetOutputDirPathFromConfig(ConfCompress.OutputDirType,pathOrgFileName,ConfCompress.OutputDir,pathOutputDir,bUseForAll,strErr);
		if(FAILED(hr)){
			return hr;
		}
		if(bUseForAll){
			rCmdLineInfo.OutputDir=(LPCTSTR)pathOutputDir;
		}
	}
	pathOutputDir.AddBackslash();

	// �o�͐悪�l�b�g���[�N�h���C�u/�����[�o�u���f�B�X�N�ł���Ȃ�x��
	// �o�͐悪���݂��Ȃ��Ȃ�A�쐬�m�F
	HRESULT hRes=ConfirmOutputDir(ConfGeneral,pathOutputDir,strErr);
	if(FAILED(hRes)){
		//�L�����Z���Ȃ�
		return hRes;
	}

	if(PARAMETER_JACK==Type){
		//JACK�`���̏ꍇ�͏o�̓t�H���_�����w��
		r_pathArcFileName=pathOutputDir;
		return S_OK;
	}

	//�t�@�C��������g���q�ƃp�X�����폜
	if(bDirectory || bDriveRoot){	//�f�B���N�g��
		//�f�B���N�g������͊g���q(�Ō��'.'�ȍ~�̕�����)���폜���Ȃ�
		CPath pathDir=pathOrgFileName;
		pathDir.StripPath();

		r_pathArcFileName=pathOutputDir;
		r_pathArcFileName.Append(pathDir);
		r_pathArcFileName=(LPCTSTR)(((CString)r_pathArcFileName)+strExt);
	}else{	//�t�@�C��
		CPath pathFile=pathOrgFileName;
		pathFile.StripPath();
		r_pathArcFileName=pathOutputDir;
		r_pathArcFileName.Append(pathFile);
		if(PARAMETER_B2E!=Type){
			//B2E�̎��͊g���q�̈�����B2E32.dll�ɔC����
			r_pathArcFileName.RemoveExtension();
		}
		r_pathArcFileName=(LPCTSTR)(((CString)r_pathArcFileName)+strExt);
	}

	//---B2E����ʈ���
	if(PARAMETER_B2E==Type){
		return S_OK;
	}

	//�h���C�u���[�g�����k����ꍇ�A�t�@�C�����͑������Ȃ�
	return ConfirmOutputFile(r_pathArcFileName,OrgFileNameList,strExt,ConfCompress.SpecifyOutputFilename || bDriveRoot,bUnicodeCapable);
}


//pathFileName�͍ŏ��̃t�@�C����
//�[���D��T��
int FindFileToCompress(LPCTSTR lpszPath,CPath &pathFileName)
{
	if(!PathIsDirectory(lpszPath)){	//�p�X���t�@�C���̏ꍇ
		pathFileName=lpszPath;
		return 1;
	}else{	//�p�X���t�H���_�̏ꍇ�A�t�H���_�����̃t�@�C����T��
		int nFileCount=0;

		CPath pathWild=lpszPath;
		pathWild.Append(_T("\\*"));
		CFindFile cFind;

		BOOL bFound=cFind.FindFile(pathWild);
		for(;bFound;bFound=cFind.FindNextFile()){
			if(cFind.IsDots())continue;

			if(cFind.IsDirectory()){	//�T�u�f�B���N�g������
				nFileCount+=FindFileToCompress(cFind.GetFilePath(),pathFileName);
				if(nFileCount>=2){
					return 2;
				}
			}else{
				pathFileName=(LPCTSTR)cFind.GetFilePath();
				nFileCount++;
				if(nFileCount>=2){
					return 2;
				}
			}
		}
		return nFileCount;
	}
}

bool DeleteOriginalFiles(const CConfigCompress &ConfCompress,const std::list<CString>& fileList)
{
	CString strFiles;	//�t�@�C���ꗗ
	size_t idx=0;
	for(std::list<CString>::const_iterator ite=fileList.begin();ite!=fileList.end();++ite){
		strFiles+=_T("\n");
		strFiles+=*ite;
		idx++;
		if(idx>=10 && idx<fileList.size()){
			strFiles+=_T("\n");
			strFiles.AppendFormat(IDS_NUM_EXTRA_FILES,fileList.size()-idx);
			break;
		}
	}

	//�폜����
	if(ConfCompress.MoveToRecycleBin){
		//--------------
		// ���ݔ��Ɉړ�
		//--------------
		if(!ConfCompress.DeleteNoConfirm){	//�폜�m�F����ꍇ
			CString Message;
			Message.Format(IDS_ASK_MOVE_ORIGINALFILE_TO_RECYCLE_BIN);
			Message+=strFiles;

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
		if(!ConfCompress.DeleteNoConfirm){	//�m�F����ꍇ
			CString Message;
			Message.Format(IDS_ASK_DELETE_ORIGINALFILE);
			Message+=strFiles;

			if(IDYES!=MessageBox(NULL,Message,UtilGetMessageCaption(),MB_YESNO|MB_ICONQUESTION|MB_DEFBUTTON2)){
				return false;
			}
		}
		//�폜���s
		for(std::list<CString>::const_iterator ite=fileList.begin();ite!=fileList.end();++ite){
			UtilDeletePath(*ite);
		}
		return true;
	}
}
