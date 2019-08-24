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

#pragma once
#include "../resource.h"
#include "../TestArchive.h"

class CLogListDialog:public CDialogImpl<CLogListDialog>,public CDialogResize<CLogListDialog>, public CCustomDraw<CLogListDialog>
{
protected:
	LRESULT OnInitDialog(HWND hWnd, LPARAM lParam);

	//CSplitterWindow	m_SplitterWindow;	// �X�v���b�^�E�B���h�E
	CListViewCtrl	m_ItemListView;		// �t�@�C���ꗗ
	CEdit			m_MsgEdit;			// �������ʂ̃��b�Z�[�W�\���p
	CEdit			m_PathEdit;			// �������ʂ̃t�@�C�����\���p

	std::vector<ARCLOG>	m_LogArray;		//�������ʂ̔z��
	bool				m_bAllOK;		//���ׂĂ�OK�Ȃ�true
	CString				m_strCaption;	//�_�C�A���O�̃L���v�V����

	//--------------
	// ���X�g�r���[
	//--------------
	LRESULT OnGetDispInfo(LPNMHDR pnmh);	//���z���X�g�r���[�̃f�[�^�擾
	LRESULT OnItemChanged(LPNMHDR pnmh);	//�I���̕ύX
	LRESULT OnSortItem(LPNMHDR pnmh);		//�\�[�g
	int m_nSortColumn;						//�\�[�g�Ɏg������
	bool m_bSortDescending;					//�\�[�g�������Ȃ�false


	// ���b�Z�[�W�}�b�v
	BEGIN_MSG_MAP_EX(CLogListDialog)
		NOTIFY_CODE_HANDLER_EX(LVN_GETDISPINFO, OnGetDispInfo)
		MSG_WM_INITDIALOG(OnInitDialog)
//		COMMAND_ID_HANDLER_EX(IDOK, OnEnd)
		COMMAND_ID_HANDLER_EX(IDCANCEL, OnEnd)
		NOTIFY_CODE_HANDLER_EX(LVN_ITEMCHANGED, OnItemChanged)
		MSG_WM_CTLCOLORSTATIC(OnCtrlColorEdit)
		NOTIFY_CODE_HANDLER_EX(LVN_COLUMNCLICK, OnSortItem)
		CHAIN_MSG_MAP(CCustomDraw<CLogListDialog>)    // CCustomDraw�N���X�փ`�F�[��
		CHAIN_MSG_MAP(CDialogResize<CLogListDialog>)    // CDialogResize�N���X�ւ̃`�F�[��
//		REFLECT_NOTIFICATIONS()
	END_MSG_MAP()

	void OnEnd(UINT uNotifyCode, int nID, HWND hWndCtl){
		EndDialog(nID);
	}

	// �_�C�A���O���T�C�Y�}�b�v
	BEGIN_DLGRESIZE_MAP(CLogListDialog)
		DLGRESIZE_CONTROL(IDC_LIST_LOGINFO_ITEMS,DLSZ_SIZE_X | DLSZ_SIZE_Y)
		DLGRESIZE_CONTROL(IDC_STATIC_MOVABLE1,DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_STATIC_MOVABLE2,DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_EDIT_LOGINFO_FILE,DLSZ_SIZE_X|DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_STATIC_LOGINFO,DLSZ_SIZE_X|DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_EDIT_LOGINFO_MSG,DLSZ_MOVE_X|DLSZ_SIZE_X|DLSZ_SIZE_Y)
	END_DLGRESIZE_MAP()

	//�J�X�^���h���[
	DWORD OnPrePaint(int nID, LPNMCUSTOMDRAW);
	DWORD OnItemPrePaint(int nID, LPNMCUSTOMDRAW);

	//�R���g���[���̐F:�ǂݎ���p�G�f�B�b�g
	HBRUSH OnCtrlColorEdit(HDC,HWND);
public:
	CLogListDialog(LPCTSTR lpszCaption):m_strCaption(lpszCaption),m_nSortColumn(-1),m_bSortDescending(false){}
	enum {IDD = IDD_DIALOG_LOGLIST};

	//���O�����Z�b�g
	void SetLogArray(const std::vector<ARCLOG>&);
};
