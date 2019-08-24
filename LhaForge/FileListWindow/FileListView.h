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
#include "FileListModel.h"
#include "ShellDataManager.h"
#include "OLE/DnDSource.h"
#include "OLE/DropTarget.h"	//�h���b�v�󂯓���,IDropCommunicator
#include "../Utilities/TemporaryDirMgr.h"
#include "../resource.h"
#include "FileListMessages.h"
#include "../Utilities/OSUtil.h"

struct CConfigFileListWindow;
class CFileListView:public CWindowImpl<CFileListView,CListViewCtrl>,public CCustomDraw<CFileListView>,public IDropCommunicator//���O�̃C���^�[�t�F�C�X
{
public:
	DECLARE_WND_SUPERCLASS(NULL, CListViewCtrl::GetWndClassName())
	BOOL PreTranslateMessage(MSG* pMsg){return FALSE;}

	//----------------------
	//IDropCommunicator�̎���
	HRESULT DragEnter(IDataObject*,POINTL&,DWORD&);
	HRESULT DragLeave();
	HRESULT DragOver(IDataObject*,POINTL&,DWORD&);
	HRESULT Drop(IDataObject*,POINTL&,DWORD&);
protected:
	BEGIN_MSG_MAP_EX(CFileListView)
		MSG_WM_CREATE(OnCreate)
		MSG_WM_DESTROY(OnDestroy)
		MSG_WM_CONTEXTMENU(OnContextMenu)	//�E�N���b�N���j���[
		COMMAND_ID_HANDLER_EX(ID_MENUITEM_SELECT_ALL,OnSelectAll)
		MESSAGE_HANDLER(WM_FILELIST_ARCHIVE_LOADED, OnFileListNewContent)
		MESSAGE_HANDLER(WM_FILELIST_NEWCONTENT, OnFileListNewContent)
		MESSAGE_HANDLER(WM_FILELIST_UPDATED, OnFileListUpdated)
		NOTIFY_CODE_HANDLER(NM_RCLICK, OnColumnRClick)	//�J�����w�b�_���E�N���b�N
		COMMAND_ID_HANDLER_EX(ID_MENUITEM_DELETE_SELECTED,OnDelete)
		COMMAND_ID_HANDLER_EX(ID_MENUITEM_EXTRACT_SELECTED,OnExtractItem)
		COMMAND_ID_HANDLER_EX(ID_MENUITEM_EXTRACT_SELECTED_SAMEDIR,OnExtractItem)
		COMMAND_ID_HANDLER_EX(ID_MENUITEM_FINDITEM,OnFindItem)
		COMMAND_ID_HANDLER_EX(ID_MENUITEM_FINDITEM_END,OnFindItem)
		COMMAND_ID_HANDLER_EX(ID_MENUITEM_ADD_FILE,OnAddItems)
		COMMAND_ID_HANDLER_EX(ID_MENUITEM_ADD_DIRECTORY,OnAddItems)
		COMMAND_ID_HANDLER_EX(ID_MENUITEM_SHOW_COLUMNHEADER_MENU,OnShowCustomizeColumn)
		COMMAND_ID_HANDLER_EX(ID_MENUITEM_OPEN_ASSOCIATION,OnOpenAssociation)
		COMMAND_ID_HANDLER_EX(ID_MENUITEM_OPEN_ASSOCIATION_OVERWRITE,OnOpenAssociation)
		COMMAND_ID_HANDLER_EX(ID_MENUITEM_EXTRACT_TEMPORARY,OnExtractTemporary)
		COMMAND_ID_HANDLER_EX(ID_MENUITEM_CLEAR_TEMPORARY,OnClearTemporary)
		COMMAND_RANGE_HANDLER_EX(ID_MENUITEM_COPY_FILENAME,ID_MENUITEM_COPY_ALL,OnCopyInfo)
		COMMAND_RANGE_HANDLER_EX(ID_MENUITEM_USERAPP_BEGIN,ID_MENUITEM_USERAPP_END+MenuCommand_GetNumSendToCmd(),OnOpenWithUserApp)
		REFLECTED_NOTIFY_CODE_HANDLER_EX(NM_DBLCLK, OnDblClick)
		REFLECTED_NOTIFY_CODE_HANDLER_EX(NM_RETURN, OnDblClick)
		REFLECTED_NOTIFY_CODE_HANDLER_EX(LVN_BEGINDRAG, OnBeginDrag)
		REFLECTED_NOTIFY_CODE_HANDLER_EX(LVN_GETINFOTIP, OnGetInfoTip)
		REFLECTED_NOTIFY_CODE_HANDLER_EX(LVN_GETDISPINFO, OnGetDispInfo)
		REFLECTED_NOTIFY_CODE_HANDLER_EX(LVN_ODFINDITEM, OnFindItem)	//�L�[�^�C�v�ł̌���
		REFLECTED_NOTIFY_CODE_HANDLER_EX(LVN_COLUMNCLICK, OnSortItem)
		CHAIN_MSG_MAP_ALT(CCustomDraw<CFileListView>, 1)	// CCustomDraw�N���X�փ`�F�[��
		DEFAULT_REFLECTION_HANDLER()
	END_MSG_MAP()
protected:
	int m_ColumnIndexArray[FILEINFO_ITEM_COUNT];	//�J�����C���f�b�N�X�ƃT�u�A�C�e���ԍ��̑Ή�
	/*
		Config.FileListWindow.ColumnOrderArray?�ɂ́A�J���������я���FILEINFO_TYPE�̒l�œ����Ă���(��\�����ڂ�-1)�B
		m_ColumnIndexArray[colIdx] = FILEINFO_TYPE
	*/


	CConfigManager&				mr_Config;
	CFileListModel&				mr_Model;
	CTemporaryDirectoryManager	m_TempDirMgr;
	CShellDataManager			m_ShellDataManager;
	CImageList					m_SortImageList;
	bool	m_bDisplayFileSizeInByte;
	bool	m_bPathOnly;
	HWND	m_hFrameWnd;

	COLEDnDSource	m_DnDSource;	//DnD�n���h��
	CDropTarget		m_DropTarget;	//�h���b�v�󂯓���Ɏg��
	int				m_nDropHilight;		//�h���b�v�n�C���C�g��Ԃɂ���A�C�e���̃C���f�b�N�X

protected:
	//---�C�x���g�n���h��
	LRESULT OnCreate(LPCREATESTRUCT lpcs);
	LRESULT OnDestroy();
	LRESULT OnFileListNewContent(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnFileListUpdated(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	LRESULT OnGetDispInfo(LPNMHDR pnmh);
	LRESULT OnGetInfoTip(LPNMHDR pnmh);
	LRESULT OnFindItem(LPNMHDR pnmh);
	LRESULT OnSortItem(LPNMHDR pnmh);
	LRESULT OnBeginDrag(LPNMHDR pnmh);
	LRESULT OnDblClick(LPNMHDR pnmh);
	void OnSelectAll(UINT,int,HWND);
	void OnDelete(UINT,int,HWND);
	void OnContextMenu(HWND,CPoint&);
	void OnExtractItem(UINT,int,HWND);	//�I���t�@�C������
	void OnFindItem(UINT,int,HWND);	//�t�@�C������
	void OnShowCustomizeColumn(UINT,int,HWND);	//�J�����w�b�_�ҏW���j���[��\��
	void OnOpenAssociation(UINT,int,HWND);	//�ꎞ�t�H���_�ɓW�J�����t�@�C�����֘A�t���ŊJ��
	void OnExtractTemporary(UINT,int,HWND);	//�ꎞ�t�H���_�ɓW�J
	void OnClearTemporary(UINT,int,HWND);	//�ꎞ�t�H���_����ɂ���
	void OnAddItems(UINT,int,HWND);

	//�R�}���h�n���h��
	void OnCopyInfo(UINT,int,HWND);
	void OnOpenWithUserApp(UINT,int,HWND);
public:
	LRESULT OnColumnRClick(int, LPNMHDR pnmh, BOOL& bHandled);//�J�����w�b�_���E�N���b�N
	void SortItem(int iCol);
	//�J�X�^���h���[
	DWORD OnPrePaint(int nID, LPNMCUSTOMDRAW lpnmcd);
	DWORD OnItemPrePaint(int nID, LPNMCUSTOMDRAW lpnmcd);
protected:
	//---internal functions
	//�t�@�C�����擾
	static void FormatFileSizeInBytes(CString&,const LARGE_INTEGER&);
	void FormatFileSize(CString&,const LARGE_INTEGER&);
	static void FormatFileTime(CString&,const FILETIME&);
	static void FormatAttribute(CString&,int);
	static void FormatCRC(CString &strBuffer,DWORD dwCRC);
	static void FormatRatio(CString &strBuffer,WORD wRatio);
	bool OpenAssociation(bool bOverwrite,bool bOpen);	//bOverwrite:true�Ȃ瑶�݂���e���|�����t�@�C�����폜���Ă���𓀂���
	void OpenAssociation(const std::list<CString> &filesList);
	bool OnUserApp(const std::vector<CMenuCommandItem> &menuCommandArray,UINT nID);	//�u�v���O�����ŊJ���v�̃n���h��(LhaForge�ݒ�)
	bool OnSendToApp(UINT nID);	//�u�v���O�����ŊJ���v�̃n���h��(SendTo�R�}���h)
	HRESULT AddItems(const std::list<CString> &fileList,LPCTSTR strDest);
	void UpdateSortIcon();
public:
	CFileListView(CConfigManager&,CFileListModel& rModel);
	virtual ~CFileListView(){}
	bool SetColumnState(const int* pColumnOrderArray, const int* pFileInfoWidthArray);	//���X�g�r���[�̃J�������Z�b�g����
	void GetColumnState(int* pColumnOrderArray, int* pFileInfoWidthArray);

	void SetDisplayFileSizeInByte(bool b){m_bDisplayFileSizeInByte=b;}
	void SetDisplayPathOnly(bool b){m_bPathOnly=b;}

	void GetSelectedItems(std::list<ARCHIVE_ENTRY_INFO_TREE*>&);
	void EnableDropTarget(bool bEnable);
	void SetFrameWnd(HWND hWnd){m_hFrameWnd=hWnd;}
};
