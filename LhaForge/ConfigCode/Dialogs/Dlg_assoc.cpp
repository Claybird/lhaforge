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
#include "Dlg_assoc.h"
#include "../configwnd.h"
#include "../../Utilities/StringUtil.h"

#define ICONINDEX_EXTERNAL_SINGLE	21

//ASTYPE=TAR,LZH,etc.
//EXT=.lzh,.tar,etc.
#define LOAD_ASSOC_AND_SET_ICON(TYPE,EXT,INDEX,INDEX2)	{\
	AssocSettings[ASSOC_##TYPE].DefaultIconIndex=INDEX;\
	AssocSettings[ASSOC_##TYPE].DefaultIconIndex_Ex=INDEX2;\
	AssocSettings[ASSOC_##TYPE].Picture_Icon=GetDlgItem(IDC_STATIC_ASSOCIATION_##TYPE);\
	AssocSettings[ASSOC_##TYPE].Button_SetIcon=GetDlgItem(IDC_BUTTON_CHANGE_ICON_##TYPE);\
	AssocSettings[ASSOC_##TYPE].Check_SetAssoc=GetDlgItem(IDC_CHECK_ASSOCIATION_##TYPE);\
	AssocSettings[ASSOC_##TYPE].AssocInfo.Ext=EXT;\
	AssocSettings[ASSOC_##TYPE].Check_SetAssoc.SetCheck(FALSE);\
	if(AssocGetAssociation(AssocSettings[ASSOC_##TYPE].AssocInfo)){\
		AssocSettings[ASSOC_##TYPE].SetIconFromAssoc(Icon_SystemDefault);/*�֘A�t����񂩂�A�C�R�����擾����*/\
		AssocSettings[ASSOC_##TYPE].CheckAssociation(m_strAssocDesired);	/*�֘A�t����񂪐��������ǂ����`�F�b�N*/\
		if(AssocSettings[ASSOC_##TYPE].bChanged)mr_ConfigDlg.RequireAssistant();	/*�֘A�t����񂪌���Ă���̂�LFAssist��v��*/\
		if(!AssocSettings[ASSOC_##TYPE].AssocInfo.bOrgStatus){\
			AssocSettings[ASSOC_##TYPE].Button_SetIcon.EnableWindow(false);\
			AssocSettings[ASSOC_##TYPE].Check_SetAssoc.SetCheck(FALSE);\
		}\
		else{\
			AssocSettings[ASSOC_##TYPE].Check_SetAssoc.SetCheck(TRUE);\
		}\
	}\
	else{\
		AssocSettings[ASSOC_##TYPE].Picture_Icon.SetIcon(Icon_SystemDefault);\
		AssocSettings[ASSOC_##TYPE].Button_SetIcon.EnableWindow(false);\
	}\
}

//--------------------------------------------

//�s�N�`���{�b�N�X�ɃA�C�R�����w��
LRESULT CConfigDlgAssociation::OnInitDialog(HWND hWnd, LPARAM lParam)
{
	// ���b�Z�[�W���[�v�Ƀ��b�Z�[�W�t�B���^�ƃA�C�h���n���h����ǉ�
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	pLoop->AddMessageFilter(this);

	// �֘A�t����ShellOpenCommand���Z�o
	{
		TCHAR szModule[_MAX_PATH+1];
		GetModuleFileName(GetModuleHandle(NULL), szModule, _MAX_PATH);	//�{�̂̃p�X�擾
		CPath fullPath;
		UtilGetCompletePathName(fullPath,szModule);	//�p�X�𐳋K��
		fullPath.QuoteSpaces();
		m_strAssocDesired=(LPCTSTR)fullPath;
		//m_strAssocDesired.MakeLower();	//�������ɐ��K��
		m_strAssocDesired+=_T(" /m \"%1\"");	//�p�����[�^
		TRACE(_T("ShellOpenCommand_Desired=%s\n"),m_strAssocDesired);
	}

	// �V�X�e���f�t�H���g�A�C�R�����擾
	if(Icon_SystemDefault.IsNull()){
		TCHAR Path[_MAX_PATH+1];
		FILL_ZERO(Path);
		GetSystemDirectory(Path,_MAX_PATH);
		PathAppend(Path,_T("shell32.dll"));
		Icon_SystemDefault.ExtractIcon(Path,0);
	}

	// �֘A�t�������`�F�b�N
	LOAD_ASSOC_AND_SET_ICON(LZH,	_T(".lzh"),	0,	0);
	LOAD_ASSOC_AND_SET_ICON(LZS,	_T(".lzs"),	0,	22);
	LOAD_ASSOC_AND_SET_ICON(LHA,	_T(".lha"),	0,	23);
	LOAD_ASSOC_AND_SET_ICON(ZIP,	_T(".zip"),	1,	1);
	LOAD_ASSOC_AND_SET_ICON(JAR,	_T(".jar"),	2,	2);
	LOAD_ASSOC_AND_SET_ICON(CAB,	_T(".cab"),	3,	3);
	LOAD_ASSOC_AND_SET_ICON(7Z,		_T(".7z"),	4,	4);
	LOAD_ASSOC_AND_SET_ICON(ARJ,	_T(".arj"),	14,	14);
	LOAD_ASSOC_AND_SET_ICON(RAR,	_T(".rar"),	5,	5);
	LOAD_ASSOC_AND_SET_ICON(JAK,	_T(".jak"),	10,	10);
	LOAD_ASSOC_AND_SET_ICON(GCA,	_T(".gca"),	6,	6);
	LOAD_ASSOC_AND_SET_ICON(IMP,	_T(".imp"),	17,	17);
	LOAD_ASSOC_AND_SET_ICON(ACE,	_T(".ace"),	13,	13);
	LOAD_ASSOC_AND_SET_ICON(YZ1,	_T(".yz1"),	12,	12);
	LOAD_ASSOC_AND_SET_ICON(HKI,	_T(".hki"),	11,	11);
	LOAD_ASSOC_AND_SET_ICON(BZA,	_T(".bza"),	15,	15);
	LOAD_ASSOC_AND_SET_ICON(GZA,	_T(".gza"),	16,	16);
	LOAD_ASSOC_AND_SET_ICON(ISH,	_T(".ish"),	18,	18);
	LOAD_ASSOC_AND_SET_ICON(UUE,	_T(".uue"),	19,	19);
	LOAD_ASSOC_AND_SET_ICON(BEL,	_T(".bel"),	20,	20);

	LOAD_ASSOC_AND_SET_ICON(TAR,	_T(".tar"),	7,	24);
	LOAD_ASSOC_AND_SET_ICON(GZ,		_T(".gz"),	7,	25);
	LOAD_ASSOC_AND_SET_ICON(TGZ,	_T(".tgz"),	7,	26);
	LOAD_ASSOC_AND_SET_ICON(BZ2,	_T(".bz2"),	7,	27);
	LOAD_ASSOC_AND_SET_ICON(TBZ,	_T(".tbz"),	7,	28);
	LOAD_ASSOC_AND_SET_ICON(XZ,		_T(".xz"),	7,	36);
	LOAD_ASSOC_AND_SET_ICON(TAR_XZ,	_T(".txz"),	7,	37);
	LOAD_ASSOC_AND_SET_ICON(LZMA,	_T(".lzma"),7,	38);
	LOAD_ASSOC_AND_SET_ICON(TAR_LZMA,_T(".tlz"),7,	39);
	LOAD_ASSOC_AND_SET_ICON(Z,		_T(".z"),	7,	29);
	LOAD_ASSOC_AND_SET_ICON(TAZ,	_T(".taz"),	7,	30);
	LOAD_ASSOC_AND_SET_ICON(CPIO,	_T(".cpio"),7,	31);
	LOAD_ASSOC_AND_SET_ICON(A,		_T(".a"),	7,	32);
	LOAD_ASSOC_AND_SET_ICON(LIB,	_T(".lib"),	9,	9);
	LOAD_ASSOC_AND_SET_ICON(RPM,	_T(".rpm"),	8,	33);
	LOAD_ASSOC_AND_SET_ICON(DEB,	_T(".deb"),	8,	34);
	LOAD_ASSOC_AND_SET_ICON(ISO,	_T(".iso"),	8,	35);
	return TRUE;
}


LRESULT CConfigDlgAssociation::OnCheckAssoc(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if(BN_CLICKED==wNotifyCode){
		for(int i=0;i<COUNTOF(AssocSettings);i++){
			if(hWndCtl==AssocSettings[i].Check_SetAssoc){
				BOOL bEnabled=AssocSettings[i].Check_SetAssoc.GetCheck();
				AssocSettings[i].Button_SetIcon.EnableWindow(bEnabled);
				if(!bEnabled){
					AssocSettings[i].SetIconFromAssoc(Icon_SystemDefault);
				}
				//LFAssist.exe�̎��s��v��
				mr_ConfigDlg.RequireAssistant();
				break;
			}
		}
	}
	return 0;
}


LRESULT CConfigDlgAssociation::OnChangeIcon(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if(BN_CLICKED==wNotifyCode){
		for(int i=0;i<COUNTOF(AssocSettings);i++){
			if(hWndCtl==AssocSettings[i].Button_SetIcon){
				CIconSelectDialog isd(AssocSettings[i].AssocInfo);
				if(IDOK==isd.DoModal()){
					TRACE(_T("IconFile=%s\n"),AssocSettings[i].AssocInfo.IconFile);
					AssocSettings[i].SetIcon(AssocSettings[i].AssocInfo.IconFile,AssocSettings[i].AssocInfo.IconIndex);
					AssocSettings[i].bChanged=true;
					//LFAssist.exe�̎��s��v��
					mr_ConfigDlg.RequireAssistant();
				}
				break;
			}
		}
	}
	return 0;
}

LRESULT CConfigDlgAssociation::OnSetAssoc(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if(BN_CLICKED!=wNotifyCode){
		return 0;
	}

	TCHAR ResourcePath[_MAX_PATH+1];	//�A�C�R���t�@�C����
	int IconIndex=-1;
	if(IDC_BUTTON_ASSOC_SET_DEFAULT_ICON==wID||IDC_BUTTON_ASSOC_SET_DEFAULT_ICON_SINGLE==wID){
		//--------------
		// �W���A�C�R��
		//--------------
		FILL_ZERO(ResourcePath);
		GetModuleFileName(GetModuleHandle(NULL), ResourcePath, _MAX_PATH);	//�{�̂̃p�X�擾
		//EXE�̃p�X������DLL�̃t�@�C������g�ݗ��Ă�
		PathRemoveFileSpec(ResourcePath);
		PathAppend(ResourcePath,CString(MAKEINTRESOURCE(IDS_ICON_FILE_NAME_DEFAULT)));
	}else if(IDC_BUTTON_ASSOC_SET_EXTERNAL_ICON==wID||IDC_BUTTON_ASSOC_SET_EXTERNAL_ICON_SINGLE==wID){
		//------------------------------------
		// �O���̃A�C�R���t�@�C�����Z�b�g����
		//------------------------------------

		//�A�C�R����I��������(�f�t�H���g�͕W���A�C�R��)
		ASSOCINFO ac;
		CIconSelectDialog isd(ac);
		if(IDOK!=isd.DoModal()){
			return 0;
		}
		TRACE(_T("IconFile=%s\n"),ac.IconFile);
		_tcsncpy_s(ResourcePath,ac.IconFile,_MAX_PATH);
		IconIndex=ac.IconIndex;
		if(IDC_BUTTON_ASSOC_SET_EXTERNAL_ICON_SINGLE==wID&&-1==IconIndex){
			//�A�C�R�����I������Ă��Ȃ�
			return 0;
		}
	}
	for(int i=0;i<COUNTOF(AssocSettings);i++){
		switch(wID){
		case IDC_BUTTON_ASSOC_CHECK_TO_DEFAULT:	//�W���̊֘A�t��
			if(-1==UtilCheckNumberArray(NO_DEFAULT_ASSOCS,COUNTOF(NO_DEFAULT_ASSOCS),i)){
				AssocSettings[i].Button_SetIcon.EnableWindow(TRUE);
				AssocSettings[i].Check_SetAssoc.SetCheck(TRUE);
			}
			break;
		case IDC_BUTTON_ASSOC_CHECK_ALL:
			AssocSettings[i].Button_SetIcon.EnableWindow(TRUE);
			AssocSettings[i].Check_SetAssoc.SetCheck(TRUE);
			break;
		case IDC_BUTTON_ASSOC_UNCHECK_ALL:
			AssocSettings[i].Button_SetIcon.EnableWindow(FALSE);
			AssocSettings[i].Check_SetAssoc.SetCheck(FALSE);
			AssocSettings[i].SetIconFromAssoc(Icon_SystemDefault);
			break;
		case IDC_BUTTON_ASSOC_SET_DEFAULT_ICON:	//FALLTHROUGH
		case IDC_BUTTON_ASSOC_SET_EXTERNAL_ICON:
			//�S�ĕW���A�C�R����/�S�ĊO���A�C�R����
			{
				//�A�C�R�����̎擾
				long IconCount=(long)ExtractIcon(GetModuleHandle(NULL),ResourcePath,-1);
				const bool bExtraIcon=(IconCount>=35);	//�A�C�R���̐���35��葽����ΑS�Ď��ʂ���^�C�v�̃A�C�R�����Ƃ������ƂɂȂ�
				if(AssocSettings[i].Check_SetAssoc.GetCheck()){
					AssocSettings[i].AssocInfo.IconIndex=bExtraIcon?AssocSettings[i].DefaultIconIndex_Ex:AssocSettings[i].DefaultIconIndex;
					AssocSettings[i].AssocInfo.IconFile=ResourcePath;
					AssocSettings[i].SetIcon(AssocSettings[i].AssocInfo.IconFile,AssocSettings[i].AssocInfo.IconIndex);
					AssocSettings[i].bChanged=true;
				}
			}
			break;
		case IDC_BUTTON_ASSOC_SET_DEFAULT_ICON_SINGLE:
			//�S�ĕW���P��A�C�R����
			if(AssocSettings[i].Check_SetAssoc.GetCheck()){
				AssocSettings[i].AssocInfo.IconIndex=ICONINDEX_EXTERNAL_SINGLE;
				AssocSettings[i].AssocInfo.IconFile=ResourcePath;
				AssocSettings[i].SetIcon(AssocSettings[i].AssocInfo.IconFile,AssocSettings[i].AssocInfo.IconIndex);
				AssocSettings[i].bChanged=true;
			}
			break;
		case IDC_BUTTON_ASSOC_SET_EXTERNAL_ICON_SINGLE:
			//�S�ĊO���P��A�C�R����
			if(AssocSettings[i].Check_SetAssoc.GetCheck()){
				AssocSettings[i].AssocInfo.IconIndex=IconIndex;
				AssocSettings[i].AssocInfo.IconFile=ResourcePath;
				AssocSettings[i].SetIcon(AssocSettings[i].AssocInfo.IconFile,AssocSettings[i].AssocInfo.IconIndex);
				AssocSettings[i].bChanged=true;
			}
			break;
		}
	}
	//LFAssist.exe�̎��s��v��
	mr_ConfigDlg.RequireAssistant();
	return 0;
}

LRESULT CConfigDlgAssociation::OnApply()
{
	CString strIniName(mr_ConfigDlg.GetAssistantFile());
	for(int i=0;i<COUNTOF(AssocSettings);i++){
		//���W�X�g���ύX�̕K�v������ꍇ
		bool Checked=(0!=AssocSettings[i].Check_SetAssoc.GetCheck());
		if((AssocSettings[i].AssocInfo.bOrgStatus^Checked)||AssocSettings[i].bChanged){
			if(AssocSettings[i].AssocInfo.bOrgStatus&&!Checked){
				//�֘A�Â������v��
				WritePrivateProfileString(AssocSettings[i].AssocInfo.Ext,_T("set"),_T("0"),strIniName);
			}
			else{
				//�֘A�Â��v��
				WritePrivateProfileString(AssocSettings[i].AssocInfo.Ext,_T("set"),_T("1"),strIniName);
				WritePrivateProfileString(AssocSettings[i].AssocInfo.Ext,_T("iconfile"),AssocSettings[i].AssocInfo.IconFile,strIniName);
				UtilWritePrivateProfileInt(AssocSettings[i].AssocInfo.Ext,_T("iconindex"),AssocSettings[i].AssocInfo.IconIndex,strIniName);
			}
		}
	}

	return TRUE;
}


//==================================================================================

CIconSelectDialog::CIconSelectDialog(ASSOCINFO &ai)
{
	AssocInfo=&ai;
	if(AssocInfo->IconFile.IsEmpty()){
		//EXE�̃p�X������DLL�̃t�@�C������g�ݗ��Ă�
		CPath strResourcePath(UtilGetModuleDirectoryPath());
		strResourcePath+=CString(MAKEINTRESOURCE(IDS_ICON_FILE_NAME_DEFAULT));
		IconPath=(CString)strResourcePath;
		TRACE(_T("Set Default Icon Path\n"));
	}
	else{
		IconPath=AssocInfo->IconFile;
	}
}

LRESULT CIconSelectDialog::OnInitDialog(HWND hWnd, LPARAM lParam)
{
	ASSERT(AssocInfo);
	CenterWindow();
	ListView=GetDlgItem(IDC_LIST_ICON);
	//DDX���A�b�v�f�[�g
	DoDataExchange(FALSE);

	UpdateIcon();
	ListView.SetItemState(AssocInfo->IconIndex,LVIS_SELECTED,1);

	// �_�C�A���O���T�C�Y������
	DlgResize_Init(true, true, WS_THICKFRAME | WS_CLIPCHILDREN);
	return TRUE;
}

void CIconSelectDialog::OnOK(UINT uNotifyCode, int nID, HWND hWndCtl)
{
	if(!DoDataExchange(TRUE))return;
	AssocInfo->IconFile=IconPath;
	const int ItemCount=ListView.GetItemCount();
	for(int i=0;i<ItemCount;i++){
		if(ListView.GetItemState(i,LVIS_SELECTED)){
			AssocInfo->IconIndex=i;
			break;
		}
	}
	EndDialog(nID);
}

void CIconSelectDialog::OnBrowse(UINT uNotifyCode, int nID, HWND hWndCtl)
{
	TCHAR filter[_MAX_PATH+2]={0};
	UtilMakeFilterString(
		_T("Icon File|*.dll;*.exe;*.ico;*.ocx;*.cpl;*.vbx;*.scr;*.icl|")
		_T("All Files|*.*||")
		,filter,_MAX_PATH+2);

	if(!DoDataExchange(TRUE))return;
	CFileDialog dlg(TRUE, NULL, IconPath, OFN_HIDEREADONLY|OFN_NOCHANGEDIR,filter);
	if(IDCANCEL==dlg.DoModal()){	//�L�����Z��
		return;
	}

	IconPath=dlg.m_szFileName;
	DoDataExchange(FALSE);
	UpdateIcon();
}

void CIconSelectDialog::OnBrowseDefault(UINT uNotifyCode, int nID, HWND hWndCtl)
{
	TCHAR ResourcePath[_MAX_PATH+1];
	FILL_ZERO(ResourcePath);
	GetModuleFileName(GetModuleHandle(NULL), ResourcePath, _MAX_PATH);	//�{�̂̃p�X�擾
	//EXE�̃p�X������DLL�̃t�@�C������g�ݗ��Ă�
	PathRemoveFileSpec(ResourcePath);
	PathAppend(ResourcePath,CString(MAKEINTRESOURCE(IDS_ICON_FILE_NAME_DEFAULT)));
	IconPath=ResourcePath;

	DoDataExchange(FALSE);
	UpdateIcon();
}

bool CIconSelectDialog::UpdateIcon()
{
	ListView.DeleteAllItems();
	IconList.Destroy();
	//�A�C�R�����̎擾
	long IconCount=(long)ExtractIcon(GetModuleHandle(NULL),IconPath,-1);
	if(0==IconCount){
		ListView.EnableWindow(false);
		return false;
	}
	ListView.EnableWindow(true);
	IconList.Create(32,32,ILC_COLOR32 | ILC_MASK,IconCount,1);
	for(long i=0;i<IconCount;i++){
		CIcon Icon;
		Icon.ExtractIcon(IconPath,i);
		IconList.AddIcon(Icon);
	}
	ListView.SetImageList(IconList,LVSIL_NORMAL);
	for(long i=0;i<IconCount;i++){
		ListView.AddItem(i,0,_T(""),i);
	}
	return true;
}
