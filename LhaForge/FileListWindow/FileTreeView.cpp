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
#include "FileTreeView.h"
#include "../resource.h"
#include "../Dialogs/LogDialog.h"
#include "../Utilities/StringUtil.h"
#include "../Utilities/OSUtil.h"


CFileTreeView::CFileTreeView(CFileListModel &rModel):
	m_bSelfAction(false),
	m_hFrameWnd(NULL),
	m_DropTarget(this),
	m_hDropHilight(NULL),
	mr_Model(rModel)
{
}

LRESULT CFileTreeView::OnCreate(LPCREATESTRUCT lpcs)
{
	LRESULT lRes=DefWindowProc();
	SetFont(AtlGetDefaultGuiFont());

	//���f���ɃC�x���g�n���h���o�^
	mr_Model.addEventListener(m_hWnd);

	//�C���[�W���X�g�쐬
	m_ImageList.Create(::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CXSMICON), ILC_COLOR32 | ILC_MASK, 8, 1);
	SetImageList(m_ImageList,TVSIL_NORMAL);
	//�t�H���_�A�C�R���擾
	SHFILEINFO shfi;
	SHGetFileInfo(_T("dummy"),FILE_ATTRIBUTE_DIRECTORY,&shfi,sizeof(shfi),SHGFI_USEFILEATTRIBUTES|SHGFI_ICON|SHGFI_SMALLICON);
	m_ImageList.AddIcon(shfi.hIcon);
	DestroyIcon(shfi.hIcon);
	SHGetFileInfo(_T("dummy"),FILE_ATTRIBUTE_DIRECTORY,&shfi,sizeof(shfi),SHGFI_USEFILEATTRIBUTES|SHGFI_ICON|SHGFI_SMALLICON|SHGFI_OPENICON);
	m_ImageList.AddIcon(shfi.hIcon);
	DestroyIcon(shfi.hIcon);

	return lRes;
}

LRESULT CFileTreeView::OnDestroy()
{
	mr_Model.removeEventListener(m_hWnd);
	m_ImageList.Destroy();
	return 0;
}

LRESULT CFileTreeView::OnFileListArchiveLoaded(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	Clear();
	ConstructTree();

	//�t�@�C���h���b�v�̎󂯓���
	EnableDropTarget(true);
	return 0;
}


LRESULT CFileTreeView::OnFileListNewContent(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	UpdateCurrentNode();
	return 0;
}

LRESULT CFileTreeView::OnFileListUpdated(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	//nothing to do
	return 0;
}

bool CFileTreeView::UpdateCurrentNode()
{
	ARCHIVE_ENTRY_INFO_TREE* lpNode=mr_Model.GetCurrentNode();
	ITEMDICT::iterator ite=m_TreeItemMap.find(lpNode);
	if(m_TreeItemMap.end()==ite){
		return false;
	}
	EnsureVisible((*ite).second);
	SelectItem((*ite).second);
	return true;
}


bool CFileTreeView::ConstructTree(HTREEITEM hParentItem,ARCHIVE_ENTRY_INFO_TREE* lpNode)
{
	BeginSelfAction();
	HTREEITEM hItem;
	if(NULL==hParentItem){
		lpNode=mr_Model.GetRootNode();
		hItem=InsertItem(PathFindFileName(mr_Model.GetArchiveFileName()), TVI_ROOT, TVI_LAST);
		SetItemImage(hItem,2,2);

		//�A�[�J�C�u�A�C�R���̐ݒ�
		m_ImageList.Remove(2);	//�Â��A�C�R�����폜

		//�֘A�t���̃A���A�C�R�����擾
		SHFILEINFO shfi;
		::SHGetFileInfo(mr_Model.GetArchiveFileName(), 0, &shfi, sizeof(shfi),SHGFI_ICON | SHGFI_SMALLICON);
		m_ImageList.AddIcon(shfi.hIcon);
	}else{
		//�q��ǉ�
		hItem=InsertItem(lpNode->strTitle,hParentItem,TVI_LAST);
		SetItemImage(hItem,0,1);
	}
	m_TreeItemMap.insert(ITEMDICT::value_type(lpNode,hItem));
	//�A�C�e���̃f�[�^�Ƀm�[�h�̃|�C���^��ݒ�
	SetItemData(hItem,(DWORD_PTR)lpNode);

	//Node�z���̃t�H���_�ɑ΂��ď���
	UINT numItems=lpNode->GetNumChildren();
	for(UINT i=0;i<numItems;i++){
		ARCHIVE_ENTRY_INFO_TREE* lpChild=lpNode->GetChild(i);
		if(lpChild->bDir){
			//�f�B���N�g���Ȃ�ǉ�
			ConstructTree(hItem,lpChild);
		}
	}
	return true;
}

void CFileTreeView::Clear()
{
	DeleteAllItems();
	m_TreeItemMap.clear();
}

void CFileTreeView::ExpandTree()
{
	ITEMDICT::iterator ite=m_TreeItemMap.begin();
	ITEMDICT::iterator end=m_TreeItemMap.end();
	for(;ite!=end;++ite){
		Expand((*ite).second);
	}
}

LRESULT CFileTreeView::OnTreeSelect(LPNMHDR)
{
	TRACE(__FUNCTIONW__ _T("\n"));
	if(IsSelfAction()){
		//ConstructTree()�ɂ�莩�����I�񂾎��A�����Ŕ������Ă��܂�Ȃ����߂̃t���O
		EndSelfAction();
	}else{
		//�I�����ꂽ�A�C�e���Ɋ֘A�t����ꂽ�m�[�h���擾���A
		//���̔z���̃t�@�C�������X�g�r���[�ɕ\������
		ARCHIVE_ENTRY_INFO_TREE* lpCurrent=mr_Model.GetCurrentNode();

		HTREEITEM hItem=GetSelectedItem();
		if(!hItem)return 0;

		ARCHIVE_ENTRY_INFO_TREE* lpNode=(ARCHIVE_ENTRY_INFO_TREE*)GetItemData(hItem);
		ASSERT(lpNode);

		//TODO
		//if(!m_FileListModel.IsRoot())m_FileListModel.GetRootNode().EndFindItem();
		if(lpNode && lpNode!=lpCurrent){
			mr_Model.SetCurrentNode(lpNode);
		}
	}
	return 0;
}


void CFileTreeView::EnableDropTarget(bool bEnable)
{
	if(bEnable){
		//�h���b�v�󂯓���ݒ�
		::RegisterDragDrop(m_hWnd,&m_DropTarget);
	}else{
		//�h���b�v���󂯓���Ȃ�
		::RevokeDragDrop(m_hWnd);
	}
}

//---------------------------------------------------------
//    IDropCommunicator�̎���:�h���b�O&�h���b�v�ɂ�鈳�k
//---------------------------------------------------------
HRESULT CFileTreeView::DragEnter(IDataObject *lpDataObject,POINTL &pt,DWORD &dwEffect)
{
	return DragOver(lpDataObject,pt,dwEffect);
}

HRESULT CFileTreeView::DragLeave()
{
	//�S�Ẵn�C���C�g�𖳌���
	if(m_hDropHilight){
		SetItemState( m_hDropHilight, ~LVIS_DROPHILITED, LVIS_DROPHILITED);
	}
	m_hDropHilight=NULL;
	return S_OK;
}

HRESULT CFileTreeView::DragOver(IDataObject *,POINTL &pt,DWORD &dwEffect)
{
	//�t�H�[�}�b�g�ɑΉ���������������
	if(!m_DropTarget.QueryFormat(CF_HDROP) || !mr_Model.IsAddItemsSupported()){	//�t�@�C����p
		//�t�@�C���ł͂Ȃ��̂ŋ���
		dwEffect = DROPEFFECT_NONE;
	}else{
		//---�h���b�v��A�C�e�����擾
		CPoint ptTemp(pt.x,pt.y);
		ScreenToClient(&ptTemp);
		HTREEITEM hItem=HitTest(ptTemp,NULL);

		//---��������������邽�߁A�O�Ɠ����A�C�e�����n�C���C�g�����Ȃ�n�C���C�g���N���A���Ȃ�
		if(hItem!=m_hDropHilight){
			//�O�̃n�C���C�g�𖳌���
			if(m_hDropHilight){
				SetItemState( m_hDropHilight, ~LVIS_DROPHILITED, LVIS_DROPHILITED);
			}
			//�V�����n�C���C�g
			if(hItem){
				SelectDropTarget(hItem);
				m_hDropHilight=hItem;
			}
		}
		dwEffect = hItem ? DROPEFFECT_COPY : DROPEFFECT_NONE;
	}
	return S_OK;
}

//�t�@�C���̃h���b�v
HRESULT CFileTreeView::Drop(IDataObject *lpDataObject,POINTL &pt,DWORD &dwEffect)
{
	//�O�̃n�C���C�g�𖳌���
	if(m_hDropHilight){
		SetItemState( m_hDropHilight, ~LVIS_DROPHILITED, LVIS_DROPHILITED);
	}else{
		return E_HANDLE;
	}

	//�t�@�C���擾
	std::list<CString> fileList;
	if(S_OK==m_DropTarget.GetDroppedFiles(lpDataObject,fileList)){
		//�t�@�C�����h���b�v���ꂽ
		dwEffect = DROPEFFECT_COPY;

		//---�h���b�v������
		CPoint ptTemp(pt.x,pt.y);
		ScreenToClient(&ptTemp);
		HTREEITEM hItem=HitTest(ptTemp,NULL);

		CString strDest;	//���荞�ސ�
		ARCHIVE_ENTRY_INFO_TREE* lpNode=(ARCHIVE_ENTRY_INFO_TREE*)GetItemData(hItem);
		if(lpNode){		//�A�C�e�����DnD
			//�A�C�e�����t�H���_�������炻�̃t�H���_�ɒǉ�
			ASSERT(lpNode->bDir);
			ArcEntryInfoTree_GetNodePathRelative(lpNode,mr_Model.GetRootNode(),strDest);
		}else{
			return E_HANDLE;
		}
		TRACE(_T("Target:%s\n"),(LPCTSTR)strDest);

		//�ǉ��J�n
		::EnableWindow(m_hFrameWnd,FALSE);
		CString strLog;

		HRESULT hr=mr_Model.AddItem(fileList,strDest,strLog);

		::EnableWindow(m_hFrameWnd,TRUE);
		::SetForegroundWindow(m_hFrameWnd);

		if(FAILED(hr) || S_FALSE==hr){
			CString msg;
			switch(hr){
			case E_LF_SAME_INPUT_AND_OUTPUT:	//�A�[�J�C�u���g��ǉ����悤�Ƃ���
				msg.Format(IDS_ERROR_SAME_INPUT_AND_OUTPUT,mr_Model.GetArchiveFileName());
				ErrorMessage(msg);
				break;
			case E_LF_UNICODE_NOT_SUPPORTED:	//�t�@�C������UNICODE���������t�@�C�������k���悤�Ƃ���
				ErrorMessage(CString(MAKEINTRESOURCE(IDS_ERROR_UNICODEPATH)));
				break;
			case S_FALSE:	//�ǉ������ɖ��
				{
					CLogDialog LogDialog;
					LogDialog.SetData(strLog);
					LogDialog.DoModal();
				}
				break;
			default:
				ASSERT(!"This code cannot be run");
			}
			return E_INVALIDARG;
		}
		//�A�[�J�C�u�X�V
		::PostMessage(m_hFrameWnd,WM_FILELIST_REFRESH,0,0);
		return S_OK;
	}else{
		//�󂯓���ł��Ȃ��`��
		dwEffect = DROPEFFECT_NONE;
		return S_FALSE;	//S_OK
	}
}

LRESULT CFileTreeView::OnRClick(LPNMHDR lpNM)
{
	CPoint pt;
	GetCursorPos(&pt);
	CPoint ptClient=pt;
	ScreenToClient(&ptClient);
	SelectItem(HitTest(ptClient,NULL));
	OnContextMenu(m_hWnd,pt);
	return 0;
}


//�R���e�L�X�g���j���[���J��
void CFileTreeView::OnContextMenu(HWND hWndCtrl,CPoint &Point)
{
	//�I�����ꂽ�A�C�e�����
	std::list<ARCHIVE_ENTRY_INFO_TREE*> items;
	GetSelectedItems(items);

	//�����I�����Ă��Ȃ��A�������̓��[�g��I�������ꍇ�͕\�����Ȃ�
	if(items.empty())return;

	if(-1==Point.x&&-1==Point.y){
		//�L�[�{�[�h����̓��͂ł���ꍇ
		//���X�g�r���[�̍���ɕ\������
		Point.x=Point.y=0;
		ClientToScreen(&Point);
	}

	CMenu cMenu;
	CMenuHandle cSubMenu;
	HWND hWndSendTo=NULL;
	if((*items.begin())->lpParent==NULL){
		hWndSendTo=m_hFrameWnd;
		//���[�g���j���[
		cMenu.LoadMenu(IDR_ARCHIVE_POPUP);
		cSubMenu=cMenu.GetSubMenu(0);
	}else{
		hWndSendTo=m_hWnd;
		//�A�C�e�����j���[
		//�����𓀂��g�p�ł��Ȃ��Ȃ�A���j���[��\������Ӗ����Ȃ�
		//TODO:�폜���j���[�͂ǂ�����?�����𓀂ł����ɍ폜�\�͂قڂ��蓾�Ȃ�
		if(!mr_Model.IsExtractEachSupported()){
			MessageBeep(MB_ICONASTERISK);
			return;
		}

		//---�E�N���b�N���j���[�\��
		cMenu.LoadMenu(IDR_FILELIST_POPUP);
		cSubMenu=cMenu.GetSubMenu(0);

		//�R�}���h��ǉ����邽�߂̃T�u���j���[��T��
		int MenuCount=cSubMenu.GetMenuItemCount();
		int iIndex=-1;
		for(int i=0;i<=MenuCount;i++){
			if(-1==cSubMenu.GetMenuItemID(i)){	//�|�b�v�A�b�v�̐e
				iIndex=i;
				break;
			}
		}
		ASSERT(-1!=iIndex);
		if(-1!=iIndex){
			MenuCommand_MakeUserAppMenu(cSubMenu.GetSubMenu(iIndex));
			MenuCommand_MakeSendToMenu(cSubMenu.GetSubMenu(iIndex+1));
		}
	}
	cSubMenu.TrackPopupMenu(TPM_LEFTALIGN|TPM_RIGHTBUTTON,Point.x, Point.y, hWndSendTo,NULL);
}

void CFileTreeView::OnDelete(UINT uNotifyCode,int nID,HWND hWndCtrl)
{
	if(!mr_Model.IsDeleteItemsSupported()){
		//�I���t�@�C���̍폜�̓T�|�[�g����Ă��Ȃ�
		if(1==uNotifyCode){	//�A�N�Z�����[�^���瑀��
			MessageBeep(MB_OK);
		}else{
			ErrorMessage(CString(MAKEINTRESOURCE(IDS_ERROR_FILELIST_DELETE_SELECTED_NOT_SUPPORTED)));
		}
		return;// false;
	}

	//�I�����ꂽ�t�@�C�����
	std::list<ARCHIVE_ENTRY_INFO_TREE*> items;
	GetSelectedItems(items);

	//�t�@�C�����I������Ă��Ȃ���΃G���[
	ASSERT(!items.empty());
	if(items.empty()){
		return;// false;
	}

	//�����m�F
	if(IDYES!=MessageBox(CString(MAKEINTRESOURCE(IDS_ASK_FILELIST_DELETE_SELECTED)),UtilGetMessageCaption(),MB_YESNO|MB_DEFBUTTON2|MB_ICONEXCLAMATION)){
		return;
	}

	//----------------
	// �t�@�C��������
	//----------------
	//�E�B���h�E���g�p�s��
	::EnableWindow(m_hFrameWnd,FALSE);
	CString strLog;
	bool bRet=mr_Model.DeleteItems(items,strLog);
	//�E�B���h�E���g�p�\��
	::EnableWindow(m_hFrameWnd,TRUE);
	SetForegroundWindow(m_hFrameWnd);
	if(!bRet){
		CLogDialog LogDlg;
		LogDlg.SetData(strLog);
		LogDlg.DoModal(m_hFrameWnd);
	}

	//�A�[�J�C�u���e�X�V
	::PostMessage(m_hFrameWnd,WM_FILELIST_REFRESH,NULL,NULL);
}


void CFileTreeView::OnOpenWithUserApp(UINT uNotifyCode,int nID,HWND hWndCtrl)
{
	if(nID<ID_MENUITEM_USERAPP_END){
		//LhaForge�ݒ�̃R�}���h
		OnUserApp(MenuCommand_GetCmdArray(),nID-ID_MENUITEM_USERAPP_BEGIN);
	}else{
		//SendTo�̃R�}���h
		OnSendToApp(nID-ID_MENUITEM_USERAPP_END);
	}
}


bool CFileTreeView::OnUserApp(const std::vector<CMenuCommandItem> &menuCommandArray,UINT nID)	//�u�v���O�����ŊJ���v�̃n���h��
{
	ASSERT(!menuCommandArray.empty());
	ASSERT(nID<menuCommandArray.size());
	if(nID>=menuCommandArray.size())return false;

	if(!mr_Model.IsOK())return false;

	//---�I���𓀊J�n
	//�I�����ꂽ�A�C�e�����
	std::list<ARCHIVE_ENTRY_INFO_TREE*> items;
	GetSelectedItems(items);

	std::list<CString> filesList;
	if(!items.empty()){
		CString strLog;
		if(!mr_Model.MakeSureItemsExtracted(NULL,mr_Model.GetRootNode(),items,filesList,false,strLog)){
			CLogDialog LogDialog;
			LogDialog.SetData(strLog);
			LogDialog.DoModal();
			return false;
		}
	}

	//---���s���擾
	//�p�����[�^�W�J�ɕK�v�ȏ��
	std::map<stdString,CString> envInfo;
	UtilMakeExpandInformation(envInfo);

	//�R�}���h�E�p�����[�^�W�J
	CString strCmd,strParam,strDir;
	UtilExpandTemplateString(strCmd,  menuCommandArray[nID].Path, envInfo);	//�R�}���h
	UtilExpandTemplateString(strParam,menuCommandArray[nID].Param,envInfo);	//�p�����[�^
	UtilExpandTemplateString(strDir,  menuCommandArray[nID].Dir,  envInfo);	//�f�B���N�g��

	//�����u��
	if(-1!=strParam.Find(_T("%F"))){
		//�t�@�C���ꗗ��A�����č쐬
		CString strFileList;
		for(std::list<CString>::iterator ite=filesList.begin();ite!=filesList.end();++ite){
			CPath path=*ite;
			path.QuoteSpaces();
			strFileList+=(LPCTSTR)path;
			strFileList+=_T(" ");
		}
		strParam.Replace(_T("%F"),strFileList);
		//---���s
		::ShellExecute(GetDesktopWindow(),NULL,strCmd,strParam,strDir,SW_SHOW);
	}else if(-1!=strParam.Find(_T("%S"))){
		for(std::list<CString>::iterator ite=filesList.begin();ite!=filesList.end();++ite){
			CPath path=*ite;
			path.QuoteSpaces();

			CString strParamTmp=strParam;
			strParamTmp.Replace(_T("%S"),(LPCTSTR)path);
			//---���s
			::ShellExecute(GetDesktopWindow(),NULL,strCmd,strParamTmp,strDir,SW_SHOW);
		}
	}else{
		::ShellExecute(GetDesktopWindow(),NULL,strCmd,strParam,strDir,SW_SHOW);
	}

	return true;
}

bool CFileTreeView::OnSendToApp(UINT nID)	//�u�v���O�����ŊJ���v�̃n���h��
{
	ASSERT(MenuCommand_GetNumSendToCmd());
	ASSERT(nID<MenuCommand_GetNumSendToCmd());
	if(nID>=MenuCommand_GetNumSendToCmd())return false;

	if(!mr_Model.IsOK())return false;

	//---�I���𓀊J�n
	//�I�����ꂽ�A�C�e�����
	std::list<ARCHIVE_ENTRY_INFO_TREE*> items;
	GetSelectedItems(items);

	std::list<CString> filesList;
	if(!items.empty()){
		CString strLog;
		if(!mr_Model.MakeSureItemsExtracted(NULL,mr_Model.GetRootNode(),items,filesList,false,strLog)){
			CLogDialog LogDialog;
			LogDialog.SetData(strLog);
			LogDialog.DoModal();
			return false;
		}
	}

	//�����u��
	const std::vector<SHORTCUTINFO>& sendToCmd=MenuCommand_GetSendToCmdArray();
	if(PathIsDirectory(sendToCmd[nID].strCmd)){
		//�Ώۂ̓f�B���N�g���Ȃ̂ŁA�R�s�[
		CString strFiles;
		for(std::list<CString>::const_iterator ite=filesList.begin();ite!=filesList.end();++ite){
			CPath file=*ite;
			file.RemoveBackslash();
			strFiles+=file;
			strFiles+=_T('|');
		}
		strFiles+=_T('|');
		//TRACE(strFiles);
		std::vector<TCHAR> srcBuf(strFiles.GetLength()+1);
		UtilMakeFilterString(strFiles,&srcBuf[0],srcBuf.size());

		CPath destDir=sendToCmd[nID].strCmd;
		destDir.AddBackslash();
		//Windows�W���̃R�s�[����
		SHFILEOPSTRUCT fileOp={0};
		fileOp.wFunc=FO_COPY;
		fileOp.fFlags=FOF_NOCONFIRMMKDIR|FOF_NOCOPYSECURITYATTRIBS|FOF_NO_CONNECTED_ELEMENTS;
		fileOp.pFrom=&srcBuf[0];
		fileOp.pTo=destDir;

		//�R�s�[���s
		if(::SHFileOperation(&fileOp)){
			//�G���[
			ErrorMessage(CString(MAKEINTRESOURCE(IDS_ERROR_FILE_COPY)));
			return false;
		}else if(fileOp.fAnyOperationsAborted){
			//�L�����Z��
			ErrorMessage(CString(MAKEINTRESOURCE(IDS_ERROR_USERCANCEL)));
			return false;
		}
		return true;
	}else{
		//�����̓v���O����
		//�t�@�C���ꗗ��A�����č쐬
		CString strFileList;
		for(std::list<CString>::iterator ite=filesList.begin();ite!=filesList.end();++ite){
			CPath path=*ite;
			path.QuoteSpaces();
			strFileList+=(LPCTSTR)path;
			strFileList+=_T(" ");
		}
		CString strParam=sendToCmd[nID].strParam+_T(" ")+strFileList;
		//---���s
		CPath cmd=sendToCmd[nID].strCmd;
		cmd.QuoteSpaces();
		CPath workDir=sendToCmd[nID].strWorkingDir;
		workDir.QuoteSpaces();
		::ShellExecute(GetDesktopWindow(),NULL,cmd,strParam,workDir,SW_SHOW);
	}

	return true;
}

void CFileTreeView::OnExtractItem(UINT,int nID,HWND)
{
	//�A�[�J�C�u�Ɠ����t�H���_�ɉ𓀂���ꍇ��true
	const bool bSameDir=(ID_MENUITEM_EXTRACT_SELECTED_SAMEDIR==nID);

	if(!mr_Model.IsOK()){
		return;// false;
	}
	if(!mr_Model.IsExtractEachSupported()){
		//�I���t�@�C���̉𓀂̓T�|�[�g����Ă��Ȃ�
		ErrorMessage(CString(MAKEINTRESOURCE(IDS_ERROR_FILELIST_EXTRACT_SELECTED_NOT_SUPPORTED)));
		return;// false;
	}

	//�I�����ꂽ�A�C�e�����
	std::list<ARCHIVE_ENTRY_INFO_TREE*> items;
	GetSelectedItems(items);
	if(items.empty()){
		//�I�����ꂽ�t�@�C�����Ȃ�
		ErrorMessage(CString(MAKEINTRESOURCE(IDS_ERROR_FILELIST_NOT_SELECTED)));
		return;// false;
	}

	//��
	CString strLog;
	HRESULT hr=mr_Model.ExtractItems(m_hFrameWnd,bSameDir,items,(*items.begin())->lpParent,strLog);

	SetForegroundWindow(m_hFrameWnd);

	if(FAILED(hr)){
		CLogDialog LogDialog;
		LogDialog.SetData(strLog);
		LogDialog.DoModal();
	}
}

void CFileTreeView::GetSelectedItems(std::list<ARCHIVE_ENTRY_INFO_TREE*> &items)
{
	items.clear();
	HTREEITEM hItem=GetSelectedItem();
	if(hItem){
		ARCHIVE_ENTRY_INFO_TREE* lpNode=(ARCHIVE_ENTRY_INFO_TREE*)GetItemData(hItem);
		items.push_back(lpNode);
	}
	ASSERT(items.size()<=1);
}

void CFileTreeView::OnOpenAssociation(UINT uNotifyCode,int nID,HWND hWndCtrl)
{
	//true�Ȃ瑶�݂���e���|�����t�@�C�����폜���Ă���𓀂���
	const bool bOverwrite=(nID==ID_MENUITEM_OPEN_ASSOCIATION_OVERWRITE);
	OpenAssociation(bOverwrite,true);
}


void CFileTreeView::OnExtractTemporary(UINT uNotifyCode,int nID,HWND hWndCtrl)
{
	//�㏑���͂��邪�A�J���Ȃ�
	OpenAssociation(true,false);
}

//bOverwrite:true�Ȃ瑶�݂���e���|�����t�@�C�����폜���Ă���𓀂���
bool CFileTreeView::OpenAssociation(bool bOverwrite,bool bOpen)
{
	if(!mr_Model.IsExtractEachSupported()){
		//�I���t�@�C���̉𓀂̓T�|�[�g����Ă��Ȃ�
		ErrorMessage(CString(MAKEINTRESOURCE(IDS_ERROR_FILELIST_EXTRACT_SELECTED_NOT_SUPPORTED)));
		return false;
	}

	if(!mr_Model.CheckArchiveExists()){	//���݂��Ȃ��Ȃ�G���[
		CString msg;
		msg.Format(IDS_ERROR_FILE_NOT_FOUND,mr_Model.GetArchiveFileName());
		ErrorMessage(msg);
		return false;
	}

	//�I�����ꂽ�A�C�e�����
	std::list<ARCHIVE_ENTRY_INFO_TREE*> items;
	GetSelectedItems(items);

	if(!items.empty()){
		std::list<CString> filesList;
		CString strLog;
		if(!mr_Model.MakeSureItemsExtracted(NULL,mr_Model.GetRootNode(),items,filesList,bOverwrite,strLog)){
			CLogDialog LogDialog;
			LogDialog.SetData(strLog);
			LogDialog.DoModal();
		}
		if(bOpen)OpenAssociation(filesList);
	}

	return true;
}

void CFileTreeView::OpenAssociation(const std::list<CString> &filesList)
{
	for(std::list<CString>::const_iterator ite=filesList.begin();ite!=filesList.end();++ite){
		//���ۂ��ꂽ��㏑�����ǉ��𓀂����Ȃ�;�f�B���N�g���Ȃ狑�ۂ̂݃`�F�b�N
		bool bDenyOnly=BOOL2bool(::PathIsDirectory(*ite));//lpNode->bDir;
		if(UtilPathAcceptSpec(*ite,mr_Model.GetOpenAssocExtDeny(),mr_Model.GetOpenAssocExtAccept(),bDenyOnly)){
			::ShellExecute(GetDesktopWindow(),NULL,*ite,NULL,NULL,SW_SHOW);
			TRACE(_T("%s\n"),(LPCTSTR)*ite);
			//::ShellExecute(GetDesktopWindow(),_T("explore"),*ite,NULL,NULL,SW_SHOW);
		}else{
			::MessageBeep(MB_ICONEXCLAMATION);
		}
	}
}
