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
#include "TestArchive.h"
#include "ArchiverManager.h"
#include "ConfigCode/ConfigManager.h"
#include "ConfigCode/ConfigGeneral.h"
#include "Dialogs/LogListDialog.h"
#include "Dialogs/ProgressDlg.h"
#include "Utilities/StringUtil.h"

bool TestArchive(const std::list<CString> &rArcList,CConfigManager &ConfigManager)
{
	CArchiverDLLManager &ArchiverManager=CArchiverDLLManager::GetInstance();

	//XacRett�̓`�F�b�N���Â����ߎg��Ȃ�

	//�v���O���X�o�[
	CProgressDialog dlg;
	int nFiles=rArcList.size();
	dlg.Create(NULL);
	dlg.SetTotalFileCount(nFiles);
	dlg.ShowWindow(SW_SHOW);


	std::list<CString>::const_iterator ite=rArcList.begin();
	const std::list<CString>::const_iterator end=rArcList.end();

	//�e�X�g���ʂ��i�[����
	std::vector<ARCLOG> LogArray;
	for(;ite!=end;++ite){
		//�v���O���X�o�[��i�߂�
		if(dlg.IsWindow())dlg.SetNextState(*ite);

		ARCLOG arcLog;
		arcLog.strFile=*ite;
		//�A�[�J�C�o�n���h���擾
		//NOTE:DenyExt�ɂ��Ώۍi�荞�݂͊��ɍs���Ă���̂�2��͍s��Ȃ�
		CArchiverDLL *Archiver=ArchiverManager.GetArchiver(*ite,NULL);

		if(!Archiver){
			//---�Ή�����n���h�����Ȃ�����:�����𒲍�
			//�ʏ�̃G���[
			arcLog.strMsg.Format(IDS_ERROR_ILLEGAL_HANDLER,*ite);
			arcLog.Result=TEST_NOTARCHIVE;

			//UNICODE�֌W�̃`�F�b�N
			if(!UtilCheckT2A(*ite)){
				//UNICODE�ɑΉ����Ă��Ȃ��̂�UNICODE�t�@�C�����̃t�@�C�����������Ƃ���
				arcLog.strMsg+=_T("\r\n\r\n");
				arcLog.strMsg.AppendFormat(IDS_ERROR_UNICODEPATH);
			}
		}else{
			CString strTemp;
			arcLog.Result=Archiver->TestArchive(*ite,strTemp);
			arcLog.strMsg=Archiver->GetName();
			arcLog.strMsg+=_T("\r\n\r\n");
			arcLog.strMsg+=strTemp;
		}
		LogArray.push_back(arcLog);
	}
	//�v���O���X�o�[�����
	if(dlg.IsWindow())dlg.DestroyWindow();

	//���O�ɕ\��
	CLogListDialog LogDlg(CString(MAKEINTRESOURCE(IDS_LOGINFO_OPERATION_TESTARCHIVE)));
	LogDlg.SetLogArray(LogArray);
	LogDlg.DoModal(::GetDesktopWindow());

	return true;
}


