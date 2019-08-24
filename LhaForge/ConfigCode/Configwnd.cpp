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
#include "../ArchiverCode/arc_interface.h"
#include "configwnd.h"
#include "../Utilities/OSUtil.h"


CConfigDialog::CConfigDialog(CConfigManager &cfg)
	:mr_Config(cfg),
	hActiveDialogWnd(NULL),
	PageShellExt(*this),
	PageAssociation(*this),
	m_nAssistRequireCount(0)
{
	TRACE(_T("CConfigDialog()\n"));

	//�e���|����INI�t�@�C�����擾
	TCHAR szIniName[_MAX_PATH+1]={0};
	UtilGetTemporaryFileName(szIniName,_T("lhf"));
	m_strAssistINI=szIniName;

	//�ݒ�ǂݍ���
	CString strErr;
	if(!mr_Config.LoadConfig(strErr))ErrorMessage(strErr);
}

CConfigDialog::~CConfigDialog()
{
}


LRESULT CConfigDialog::OnInitDialog(HWND hWnd, LPARAM lParam)
{
	// �傫���A�C�R���ݒ�
	HICON hIcon = AtlLoadIconImage(IDI_APP, LR_DEFAULTCOLOR,::GetSystemMetrics(SM_CXICON),::GetSystemMetrics(SM_CYICON));
	SetIcon(hIcon, TRUE);

	// �������A�C�R���ݒ�
	HICON hIconSmall = AtlLoadIconImage(IDI_APP, LR_DEFAULTCOLOR,::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON));
	SetIcon(hIconSmall, FALSE);

	//�v���p�e�B�V�[�g��\��t���邽�߂̃X�N���[���R���e�i�̔z�u�ꏊ���擾
	CStatic StaticFrame;
	StaticFrame=GetDlgItem(IDC_STATIC_FRAME);
	RECT rect;
	StaticFrame.GetWindowRect(&rect);
//	StaticFrame.ShowWindow(SW_HIDE);
	ScreenToClient(&rect);

	//�X�N���[���R���e�i��z�u
	ScrollWindow.Create(m_hWnd,rect,NULL,/*WS_TABSTOP|*/WS_CHILD|WS_VISIBLE , WS_EX_CLIENTEDGE|WS_EX_CONTROLPARENT);

	//--------------------
	// �c���[�r���[�̐ݒ�
	//--------------------
	SelectTreeView=GetDlgItem(IDC_TREE_SELECT_PROPPAGE);

#define ADD_PAGE(_DIALOG,_ROOTITEM) {\
	m_ConfigDlgList.insert(&_DIALOG);\
	_DIALOG.LoadConfig(mr_Config);\
	_DIALOG.Create(ScrollWindow);\
	CString strTitle;\
	_DIALOG.GetWindowText(strTitle);\
	HTREEITEM hItem=SelectTreeView.InsertItem(strTitle, _ROOTITEM, TVI_LAST);\
	SelectTreeView.SetItemData(hItem,(DWORD_PTR)_DIALOG.m_hWnd);\
}

	//--------------------------------
	// �_�C�A���O�̏���ǉ����Ă���
	//--------------------------------
	ADD_PAGE(PageGeneral,TVI_ROOT);
	ADD_PAGE(PageShellExt,TVI_ROOT);
	ADD_PAGE(PageShortcut,TVI_ROOT);
	ADD_PAGE(PageFileListWindow,TVI_ROOT);
	ADD_PAGE(PageCompressGeneral,TVI_ROOT);
	ADD_PAGE(PageExtractGeneral,TVI_ROOT);
	ADD_PAGE(PageAssociation,TVI_ROOT);
	ADD_PAGE(PageOpenAction,TVI_ROOT);
//	ADD_PAGE(PageAssociation2,TVI_ROOT);
	ADD_PAGE(PageDLLUpdate,TVI_ROOT);
	ADD_PAGE(PageVersion,TVI_ROOT);

	//----------------
	// �ȉ��͏ڍאݒ�
	//----------------
	//�c���[�̐e
	m_ConfigDlgList.insert(&PageDetail);
	PageDetail.LoadConfig(mr_Config);
	PageDetail.Create(ScrollWindow);
	HTREEITEM hItemDetail;
	{
		TCHAR Buffer[_MAX_PATH+1]={0};
		PageDetail.GetWindowText(Buffer,_MAX_PATH);
		hItemDetail=SelectTreeView.InsertItem(Buffer, TVI_ROOT, TVI_LAST);
	}
	SelectTreeView.SetItemData(hItemDetail,(DWORD_PTR)PageDetail.m_hWnd);

	ADD_PAGE(PageLZH,hItemDetail);
	ADD_PAGE(PageZIP,hItemDetail);
	ADD_PAGE(PageCAB,hItemDetail);
	ADD_PAGE(Page7Z,hItemDetail);
	ADD_PAGE(PageTAR,hItemDetail);
	ADD_PAGE(PageJACK,hItemDetail);
//	ADD_PAGE(PageBH,hItemDetail);
//	ADD_PAGE(PageYZ1,TVI_ROOT);
	ADD_PAGE(PageHKI,hItemDetail);
	ADD_PAGE(PageBGA,hItemDetail);
	ADD_PAGE(PageAISH,hItemDetail);
	ADD_PAGE(PageXACRETT,hItemDetail);
	ADD_PAGE(PageB2E,hItemDetail);
	ADD_PAGE(PageDLL,hItemDetail);

	//------------------------
	// �͂��߂ɕ\������y�[�W
	//------------------------
	PageGeneral.ShowWindow(SW_SHOW);
	ScrollWindow.SetClient(PageGeneral);
	hActiveDialogWnd=PageGeneral;
	SelectTreeView.SetFocus();

	// �_�C�A���O���T�C�Y������
	DlgResize_Init(true, true, WS_THICKFRAME | WS_CLIPCHILDREN);

	//---���[�U�[�ԋ��ʐݒ�?
	if(mr_Config.IsUserCommon()){
		//�E�B���h�E�^�C�g����ݒ�
		SetWindowText(CString(MAKEINTRESOURCE(IDS_CAPTION_CONFIG_USERCOMMON)));
	}

	//�E�B���h�E�𒆐S��
	CenterWindow();

	return TRUE;
}

void CConfigDialog::OnOK(UINT uNotifyCode, int nID, HWND hWndCtl)
{
	//���O�Ƀ��j���[�G�f�B�^�ȂǂŐݒ肪�ύX����Ă����ꍇ�A�����ŒP���ɏ㏑������ƁA�ύX��̏�񂪏����Ă��܂��̂�
	//�ēǂݍ��݂��s���A��������ɏ㏑������
	CString tmp;
	mr_Config.LoadConfig(tmp);

	//�e�_�C�A���O��OnApply���Ă�
	bool bRet=true;
	for(std::set<IConfigDlgBase*>::iterator ite=m_ConfigDlgList.begin();ite!=m_ConfigDlgList.end();++ite){
		bRet=bRet && (*ite)->OnApply();
		(*ite)->StoreConfig(mr_Config);
	}

	//UAC����̃A�V�X�^���g���v������Ă���
	if(m_nAssistRequireCount>0){
		//64bit�Ȃ�64bit��p�������ɑ��点��
		if(UtilIsWow64()){
			//���LFAssist(64bit)�ɏ�����n���B������INI�폜�͍s��Ȃ��B
			//---�A�V�X�^���g(64bit)�̃p�X���擾
			CPath strExePath(UtilGetModuleDirectoryPath());
			strExePath+=_T("LFAssist64.exe");
			if(strExePath.FileExists()){	//�t�@�C�������݂���Ƃ��̂�
				strExePath.QuoteSpaces();

				//---���s:CreateProcess�ł�UAC���`�F�b�N���Ď��s���Ă��炦�Ȃ�
				SHELLEXECUTEINFO shei={0};
				shei.fMask=SEE_MASK_FLAG_DDEWAIT;	//�u���ɏI������\��������̂ł�����w�肷��
				shei.cbSize=sizeof(shei);
				shei.lpFile=strExePath;
				shei.lpParameters=m_strAssistINI;
				shei.nShow=SW_SHOW;
				if(!ShellExecuteEx(&shei)){
					//���s�G���[
					CString strLastError;
					UtilGetLastErrorMessage(strLastError);

					CString msg;
					msg.Format(IDS_ERROR_CANNOT_EXECUTE,strExePath,(LPCTSTR)strLastError);

					ErrorMessage(msg);
				}
			}
		}

		//LFAssist(32bit)��INI�폜��v��
		WritePrivateProfileString(_T("PostProcess"),_T("DeleteMe"),_T("Please_Delete_Me"),m_strAssistINI);

		//�ύX�����s
		//---�A�V�X�^���g�̃p�X���擾
		CPath strExePath(UtilGetModuleDirectoryPath());
		strExePath+=_T("LFAssist.exe");
		strExePath.QuoteSpaces();

		//---���s:CreateProcess�ł�UAC���`�F�b�N���Ď��s���Ă��炦�Ȃ�
		SHELLEXECUTEINFO shei={0};
		shei.fMask=SEE_MASK_FLAG_DDEWAIT;	//�u���ɏI������\��������̂ł�����w�肷��
		shei.cbSize=sizeof(shei);
		shei.lpFile=strExePath;
		shei.lpParameters=m_strAssistINI;
		shei.nShow=SW_SHOW;
		if(!ShellExecuteEx(&shei)){
			//���s�G���[
			CString strLastError;
			UtilGetLastErrorMessage(strLastError);

			CString msg;
			msg.Format(IDS_ERROR_CANNOT_EXECUTE,strExePath,(LPCTSTR)strLastError);

			ErrorMessage(msg);
		}else{
			Sleep(100);
			::SHChangeNotify(SHCNE_ASSOCCHANGED,SHCNF_FLUSH,NULL,NULL);	//�֘A�t���̕ύX���V�F���ɒʒm(�ύX����Ă��Ȃ���������Ȃ����ȕւ̂���)
		}
	}else{
		//�e���|����INI���폜
		if(!m_strAssistINI.IsEmpty()){
			::DeleteFile(m_strAssistINI);
		}
	}

	if(bRet)EndDialog(nID);
}

void CConfigDialog::OnCancel(UINT uNotifyCode, int nID, HWND hWndCtl)
{
	EndDialog(nID);
}

LRESULT CConfigDialog::OnTreeSelect(LPNMHDR pnmh)
{
	if(pnmh->hwndFrom==SelectTreeView){
		HTREEITEM hItem=SelectTreeView.GetSelectedItem();
		if(!hItem)return 0;

		HWND hSelectedDialogWnd = (HWND)SelectTreeView.GetItemData(hItem);
		if (hSelectedDialogWnd && hSelectedDialogWnd != hActiveDialogWnd) {
			SetRedraw(FALSE);
			::ShowWindow(hActiveDialogWnd,SW_HIDE);
			//ScrollWindow.SetClient(NULL);
			ScrollWindow.SetClient(hSelectedDialogWnd);
			::ShowWindow(hSelectedDialogWnd,SW_SHOW);
			hActiveDialogWnd = hSelectedDialogWnd;
			SetRedraw(TRUE);
			RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_FRAME | RDW_UPDATENOW | RDW_ALLCHILDREN);
		}
	}
	return 0;
}

void CConfigDialog::OnSize(UINT, CSize&)
{
	SetMsgHandled(false);
	PostMessage(WM_USER_WM_SIZE);
}

LRESULT CConfigDialog::OnUserSize(UINT, WPARAM, LPARAM, BOOL& bHandled)
{
	//�v���p�e�B�V�[�g��\��t���邽�߂̃X�N���[���R���e�i�̔z�u�ꏊ���擾
	CStatic StaticFrame;
	StaticFrame=GetDlgItem(IDC_STATIC_FRAME);
	RECT rect;
	StaticFrame.GetWindowRect(&rect);
	ScreenToClient(&rect);

	//�X�N���[���R���e�i���ړ�
	ScrollWindow.MoveWindow(&rect);
	return 0;
}

//LFAssist.exe�̗v���J�E���g�𑀍�
void CConfigDialog::RequireAssistant()
{
	m_nAssistRequireCount++;
	Button_SetElevationRequiredState(GetDlgItem(IDOK),m_nAssistRequireCount);
}

void CConfigDialog::UnrequireAssistant()
{
	ASSERT(m_nAssistRequireCount>0);
	if(m_nAssistRequireCount>0){
		m_nAssistRequireCount--;
		Button_SetElevationRequiredState(GetDlgItem(IDOK),m_nAssistRequireCount);
	}
}
