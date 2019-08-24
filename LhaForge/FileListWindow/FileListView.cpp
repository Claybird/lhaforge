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
#include "FileListView.h"
#include "../resource.h"
#include "../Dialogs/LogDialog.h"
#include "../Dialogs/LFFolderDialog.h"
#include "../ConfigCode/ConfigFileListWindow.h"
#include "../Utilities/StringUtil.h"
#include <sstream>

CFileListView::CFileListView(CConfigManager& rConfig,CFileListModel& rModel):
	mr_Config(rConfig),
	mr_Model(rModel),
	m_bDisplayFileSizeInByte(false),
	m_bPathOnly(false),
	//m_DnDSource(m_TempDirManager),
	m_DropTarget(this),
	m_nDropHilight(-1),
	m_hFrameWnd(NULL),
	m_TempDirMgr(_T("lhaf"))
{
}

LRESULT CFileListView::OnCreate(LPCREATESTRUCT lpcs)
{
	LRESULT lRes=DefWindowProc();
	//SetFont(AtlGetDefaultGuiFont());

	// �C���[�W���X�g�쐬
	m_ShellDataManager.Init();
	SetImageList(m_ShellDataManager.GetImageList(true),LVSIL_NORMAL);
	SetImageList(m_ShellDataManager.GetImageList(false),LVSIL_SMALL);

	//�J�����w�b�_�̃\�[�g�A�C�R��
	m_SortImageList.CreateFromImage(MAKEINTRESOURCE(IDB_BITMAP_SORTMARK),16,1,CLR_DEFAULT,IMAGE_BITMAP,LR_CREATEDIBSECTION);

	mr_Model.addEventListener(m_hWnd);
	return lRes;
}

LRESULT CFileListView::OnDestroy()
{
/*	CConfigFileListWindow ConfFLW;
	ConfFLW.load(mr_Config);

	//�E�B���h�E�ݒ�̕ۑ�
	if( ConfFLW.StoreSetting ) {
		//���X�g�r���[�̃X�^�C��
		ConfFLW.ListStyle = GetWindowLong(GWL_STYLE) % ( 0x0004 );

		ConfFLW.FileListMode = mr_Model.GetListMode();
		//�J�����̕��я��E�J�����̕�
		GetColumnState(ConfFLW.ColumnOrderArray, ConfFLW.ColumnWidthArray);

		ConfFLW.store(mr_Config);
		CString strErr;
		if( !mr_Config.SaveConfig(strErr) ) {
			ErrorMessage(strErr);
		}
	}
	*/

	mr_Model.removeEventListener(m_hWnd);
	return 0;
}



bool CFileListView::SetColumnState(const int* pColumnOrderArray, const int *pFileInfoWidthArray)
{
	//�����̃J�������폜
	if(GetHeader().IsWindow()){
		int nCount=GetHeader().GetItemCount();
		for(;nCount>0;nCount--){
			DeleteColumn(nCount-1);
		}
	}

//========================================
//      ���X�g�r���[�ɃJ�����ǉ�
//========================================
//���X�g�r���[�ɃJ������ǉ����邽�߂̃}�N��
#define ADD_COLUMNITEM(x,width,pos) \
{if(-1!=UtilCheckNumberArray(pColumnOrderArray,FILEINFO_ITEM_COUNT,FILEINFO_##x)){\
	int nIndex=InsertColumn(FILEINFO_##x, CString(MAKEINTRESOURCE(IDS_FILELIST_COLUMN_##x)), pos, width,-1);\
	if(nIndex<0||nIndex>=FILEINFO_ITEM_COUNT)return false;\
	m_ColumnIndexArray[nIndex]=FILEINFO_##x;\
}}

	memset(m_ColumnIndexArray,-1,sizeof(m_ColumnIndexArray));

	//�t�@�C����
	ADD_COLUMNITEM(FILENAME,100,LVCFMT_LEFT);
	//�t���p�X��
	ADD_COLUMNITEM(FULLPATH,200,LVCFMT_LEFT);
	//�t�@�C���T�C�Y
	ADD_COLUMNITEM(ORIGINALSIZE,90,LVCFMT_RIGHT);
	//�t�@�C�����
	ADD_COLUMNITEM(TYPENAME,120,LVCFMT_LEFT);
	//�X�V����
	ADD_COLUMNITEM(FILETIME,120,LVCFMT_LEFT);
	//����
	ADD_COLUMNITEM(ATTRIBUTE,60,LVCFMT_LEFT);
	//���k��T�C�Y
	ADD_COLUMNITEM(COMPRESSEDSIZE,90,LVCFMT_RIGHT);
	//���k���\�b�h
	ADD_COLUMNITEM(METHOD,60,LVCFMT_LEFT);
	//���k��
	ADD_COLUMNITEM(RATIO,60,LVCFMT_RIGHT);
	//CRC
	ADD_COLUMNITEM(CRC,60,LVCFMT_LEFT);


	//----------------------
	// �J�����̕��я���ݒ�
	//----------------------
	int Count=0;
	for(;Count<FILEINFO_ITEM_COUNT;Count++){
		//�L���ȃA�C�e���������߂�
		if(-1==pColumnOrderArray[Count])break;
	}
	//���я���ϊ�
	int TemporaryArray[FILEINFO_ITEM_COUNT];

	for(int i=0;i<FILEINFO_ITEM_COUNT;i++){
		TemporaryArray[i]=pColumnOrderArray[i];
	}
	for(int i=0;i<Count;i++){
		int nIndex=UtilCheckNumberArray(m_ColumnIndexArray,FILEINFO_ITEM_COUNT,TemporaryArray[i]);
		ASSERT(-1!=nIndex);
		if(-1!=nIndex){
			TemporaryArray[i]=nIndex;
		}
	}
	SetColumnOrderArray(Count,TemporaryArray);
	for( int i = 0; i < Count; i++ ) {
		SetColumnWidth(TemporaryArray[i], pFileInfoWidthArray[i]);
	}

	//�J�����w�b�_�̃\�[�g�A�C�R��
	if(GetHeader().IsWindow()){
		GetHeader().SetImageList(m_SortImageList);
	}
	UpdateSortIcon();
	return true;
}

void CFileListView::GetColumnState(int* pColumnOrderArray, int *pFileInfoWidthArray)
{
	//�J�����̕��я��擾
	const int nCount=GetHeader().GetItemCount();
	ASSERT(nCount<=FILEINFO_ITEM_COUNT);

	int TemporaryArray[FILEINFO_ITEM_COUNT];
	memset(TemporaryArray,-1,sizeof(TemporaryArray));
	GetColumnOrderArray(nCount,TemporaryArray);
	//���я���ϊ�
	memset(pColumnOrderArray,-1,FILEINFO_ITEM_COUNT*sizeof(int));
	for(int i=0;i<nCount;i++){
		pColumnOrderArray[i]=m_ColumnIndexArray[TemporaryArray[i]];
	}

	for( int i = 0; i < nCount; i++ ) {
		pFileInfoWidthArray[i] = GetColumnWidth(TemporaryArray[i]);
	}
}

void CFileListView::GetSelectedItems(std::list<ARCHIVE_ENTRY_INFO_TREE*> &items)
{
	items.clear();
	int nIndex=-1;
	for(;;){
		nIndex = GetNextItem(nIndex, LVNI_ALL | LVNI_SELECTED);
		if(-1==nIndex)break;
		ARCHIVE_ENTRY_INFO_TREE* lpNode=mr_Model.GetFileListItemByIndex(nIndex);

		ASSERT(lpNode);

		items.push_back(lpNode);
	}
}

LRESULT CFileListView::OnFileListNewContent(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	TRACE(__FUNCTIONW__ _T("\n"));

	DeleteAllItems();
	SetItemCount(0);
	if(mr_Model.IsOK()){
		ARCHIVE_ENTRY_INFO_TREE* lpCurrent=mr_Model.GetCurrentNode();
		ASSERT(lpCurrent);
		if(lpCurrent){
			SetItemCount(lpCurrent->GetNumChildren());
			SetItemState(0,LVIS_FOCUSED,LVIS_FOCUSED);
		}
	}

	//�t�@�C���h���b�v�̎󂯓���:�󂯓��ꋑ�ۂ�DragOver���s��
	EnableDropTarget(true);
	return 0;
}

LRESULT CFileListView::OnFileListUpdated(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	TRACE(__FUNCTIONW__ _T("\n"));

	Invalidate();

	return 0;
}

LRESULT CFileListView::OnDblClick(LPNMHDR pnmh)
{
	TRACE(__FUNCTIONW__ _T("\n"));
	//----------------------------------------------------------------------
	//�I���A�C�e���������̎�:
	// �I�����֘A�t���ŊJ��
	//Shift��������Ă�����:
	// �I�����֘A�t���ŊJ��
	//�I���A�C�e����������̎�:
	// �t�H���_���_�u���N���b�N����/Enter���������ꍇ�ɂ͂��̃t�H���_���J��
	// �I�����t�@�C���������ꍇ�ɂ͊֘A�t���ŊJ��
	//----------------------------------------------------------------------

	//�I���A�C�e���������̎�
	//����Shift��������Ă�����A�֘A�t���ŊJ��
	if(GetKeyState(VK_SHIFT)<0||GetSelectedCount()>=2){
		OpenAssociation(false,true);
		return 0;
	}

	//�I�����ꂽ�A�C�e�����擾
	std::list<ARCHIVE_ENTRY_INFO_TREE*> items;
	GetSelectedItems(items);
	if(items.empty())return 0;
	ARCHIVE_ENTRY_INFO_TREE* lpNode=*(items.begin());

	if(lpNode->bDir){
		//�K�w�\���𖳎�����ꍇ�ɂ́A���̓���͖��������
		if(mr_Model.GetListMode()==FILELIST_TREE){
			mr_Model.MoveDownDir(lpNode);
		}
	}else{
		OpenAssociation(false,true);
	}
	return 0;
}


//�J�����\����On/Off��؂�ւ���
//�\����:�Y���J�������\���ɂ��A�z����l�߂�
//��\��:�g���Ă��Ȃ������Ɏw��J������ǉ�
void _ToggleColumn(int *lpArray,size_t size,FILEINFO_TYPE type)
{
	ASSERT(lpArray);
	if(!lpArray)return;

	for(size_t i=0;i<size;i++){
		if(type==lpArray[i]){
			//�z����l�߂�
			for(size_t j=i;j<size-1;j++){
				lpArray[j]=lpArray[j+1];
			}
			lpArray[size-1]=-1;
			return;
		}
		else if(-1==lpArray[i]){
			lpArray[i]=type;
			return;
		}
	}
}


//�J�����w�b�_����/�E�N���b�N
LRESULT CFileListView::OnColumnRClick(int /*idCtrl*/, LPNMHDR pnmh, BOOL& bHandled)
{
	if(pnmh->hwndFrom!=GetHeader()){
		//���b�Z�[�W�͏������Ȃ��������Ƃɂ���
		bHandled = FALSE;
		return 0;
	}

	//�E�N���b�N���j���[�\��
	POINT point;
	GetCursorPos(&point);
	CMenu cMenu;
	cMenu.LoadMenu(IDR_LISTVIEW_HEADER_MENU);
	CMenuHandle cSubMenu(cMenu.GetSubMenu(0));

	//--------------------------------
	// �e���j���[�A�C�e���̗L���E����
	//--------------------------------

	int columnOrderArray[FILEINFO_ITEM_COUNT];
	int columnWidthArray[FILEINFO_ITEM_COUNT];
	GetColumnState(columnOrderArray, columnWidthArray);

	struct{
		FILEINFO_TYPE idx;
		UINT nMenuID;
	}menuTable[]={
		{FILEINFO_FULLPATH,			ID_MENUITEM_LISTVIEW_COLUMN_FULLPATH},
		{FILEINFO_ORIGINALSIZE,		ID_MENUITEM_LISTVIEW_COLUMN_ORIGINALSIZE},
		{FILEINFO_TYPENAME,			ID_MENUITEM_LISTVIEW_COLUMN_TYPENAME},
		{FILEINFO_FILETIME,			ID_MENUITEM_LISTVIEW_COLUMN_FILETIME},
		{FILEINFO_ATTRIBUTE,		ID_MENUITEM_LISTVIEW_COLUMN_ATTRIBUTE},
		{FILEINFO_COMPRESSEDSIZE,	ID_MENUITEM_LISTVIEW_COLUMN_COMPRESSEDSIZE},
		{FILEINFO_METHOD,			ID_MENUITEM_LISTVIEW_COLUMN_METHOD},
		{FILEINFO_RATIO,			ID_MENUITEM_LISTVIEW_COLUMN_RATIO},
		{FILEINFO_CRC,				ID_MENUITEM_LISTVIEW_COLUMN_CRC},
	};

	for(size_t i=0;i<COUNTOF(menuTable);i++){
		bool bEnabled=(-1!=UtilCheckNumberArray(columnOrderArray,COUNTOF(columnOrderArray),menuTable[i].idx));
		cSubMenu.CheckMenuItem(menuTable[i].nMenuID,MF_BYCOMMAND|(bEnabled?MF_CHECKED:MF_UNCHECKED));
	}

	//���j���[�\��:�I�������R�}���h���Ԃ��Ă���
	int nRet=cSubMenu.TrackPopupMenu(TPM_NONOTIFY|TPM_RETURNCMD|TPM_LEFTALIGN|TPM_RIGHTBUTTON,point.x, point.y, m_hWnd,NULL);
	if(0==nRet){
		//Not Selected
		return 0;
	}else if(ID_MENUITEM_LISTVIEW_COLUMN_RESET==nRet){
		//������
		for(size_t i=0;i<COUNTOF(columnOrderArray);i++){
			columnOrderArray[i]=i;
		}
	}else{
		for(size_t i=0;i<COUNTOF(menuTable);i++){
			if(menuTable[i].nMenuID==nRet){
				_ToggleColumn(columnOrderArray,COUNTOF(columnOrderArray),menuTable[i].idx);
			}
		}
	}

	SetColumnState(columnOrderArray, columnWidthArray);

	return 0;
}


//�L�[���͂ɂ��t�@�C������
LRESULT CFileListView::OnFindItem(LPNMHDR pnmh)
{
	ARCHIVE_ENTRY_INFO_TREE* lpCurrent=mr_Model.GetCurrentNode();
	ASSERT(lpCurrent);
	if(!lpCurrent)return -1;

	int iCount=lpCurrent->GetNumChildren();
	if(iCount<=0)return -1;

	LPNMLVFINDITEM lpFindInfo = (LPNMLVFINDITEM)pnmh;
	int iStart=lpFindInfo->iStart;
	if(iStart<0)iStart=0;
	if(lpFindInfo->lvfi.flags & LVFI_STRING || lpFindInfo->lvfi.flags & LVFI_PARTIAL){	//�t�@�C�����Ō���
		LPCTSTR lpFindString=lpFindInfo->lvfi.psz;
		size_t nLength=_tcslen(lpFindString);
		//�O����v�Ō���
		for(int i=iStart;i<iCount;i++){
			ARCHIVE_ENTRY_INFO_TREE* lpNode=mr_Model.GetFileListItemByIndex(i);
			ASSERT(lpNode);
			if(0==_tcsnicmp(lpFindString,lpNode->strTitle,nLength)){
				return i;
			}
		}
		if(lpFindInfo->lvfi.flags & LVFI_WRAP){
			for(int i=0;i<iStart;i++){
				ARCHIVE_ENTRY_INFO_TREE* lpNode=mr_Model.GetFileListItemByIndex(i);
				ASSERT(lpNode);
				if(0==_tcsnicmp(lpFindString,lpNode->strTitle,nLength)){
					return i;
				}
			}
		}
		return -1;
	}else{
		return -1;
	}
}



//�\�[�g
LRESULT CFileListView::OnSortItem(LPNMHDR pnmh)
{
	LPNMLISTVIEW lpNMLV=(LPNMLISTVIEW)pnmh;
	int iCol=lpNMLV->iSubItem;
	SortItem(iCol);
	return 0;
}


void CFileListView::SortItem(int iCol)
{
	if(!(iCol >= 0 && iCol < FILEINFO_ITEM_COUNT))return;

	if(iCol==mr_Model.GetSortKeyType()){
		if(mr_Model.GetSortMode()){
			mr_Model.SetSortMode(false);
		}else{	//�\�[�g����
			mr_Model.SetSortKeyType(FILEINFO_INVALID);
			mr_Model.SetSortMode(true);
		}
	}else{
		mr_Model.SetSortKeyType(iCol);
		mr_Model.SetSortMode(true);
	}

	UpdateSortIcon();
}

void CFileListView::UpdateSortIcon()
{
	if(!IsWindow())return;
	//�\�[�g��Ԃ����ɃJ������ύX����
	CHeaderCtrl hc=GetHeader();
	ASSERT(hc.IsWindow());
	if(hc.IsWindow()){
		int count=hc.GetItemCount();
		//�A�C�R������
		for(int i=0;i<count;i++){
			HDITEM hdi={0};
			hdi.mask=HDI_FORMAT;
			hc.GetItem(i,&hdi);
			if((hdi.fmt & HDF_IMAGE)){
				hdi.fmt&=~HDF_IMAGE;
				hc.SetItem(i,&hdi);
			}
		}

		//�A�C�R���Z�b�g
		int iCol=mr_Model.GetSortKeyType();
		if(iCol!=FILEINFO_INVALID && iCol<count){
			HDITEM hdi={0};
			hdi.mask=HDI_FORMAT;

			hc.GetItem(iCol,&hdi);
			hdi.mask|=HDI_FORMAT|HDI_IMAGE;
			hdi.fmt|=HDF_IMAGE|HDF_BITMAP_ON_RIGHT;
			hdi.iImage=mr_Model.GetSortMode() ? 0 : 1;
			hc.SetItem(iCol,&hdi);
		}
	}
}

//�J�X�^���h���[
DWORD CFileListView::OnPrePaint(int nID, LPNMCUSTOMDRAW lpnmcd)
{
	if(lpnmcd->hdr.hwndFrom == m_hWnd)
		return CDRF_NOTIFYITEMDRAW;
	else
		return CDRF_DODEFAULT;
}

DWORD CFileListView::OnItemPrePaint(int nID, LPNMCUSTOMDRAW lpnmcd)
{
	if(lpnmcd->hdr.hwndFrom == m_hWnd){
		LPNMLVCUSTOMDRAW lpnmlv = (LPNMLVCUSTOMDRAW)lpnmcd;

		ARCHIVE_ENTRY_INFO_TREE* lpNode=mr_Model.GetFileListItemByIndex(lpnmcd->dwItemSpec);
		if(lpNode){
			if(!lpNode->bSafe){
				//�댯�ȃA�[�J�C�u�Ȃ̂ŐF��t����
				lpnmlv->clrText = RGB(255, 255, 255);
				lpnmlv->clrTextBk = RGB(255, 0, 0);
			}
		}
	}
	return CDRF_DODEFAULT;
}


//���z���X�g�r���[�̃A�C�e���擾�ɔ���
LRESULT CFileListView::OnGetDispInfo(LPNMHDR pnmh)
{
	LV_DISPINFO* pstLVDInfo=(LV_DISPINFO*)pnmh;

	ARCHIVE_ENTRY_INFO_TREE* lpNode=mr_Model.GetFileListItemByIndex(pstLVDInfo->item.iItem);
	//ASSERT(lpNode);
	if(!lpNode)return 0;

	//�Y�����`�F�b�N
	ASSERT(pstLVDInfo->item.iSubItem>=0 && pstLVDInfo->item.iSubItem<FILEINFO_ITEM_COUNT);
	if(pstLVDInfo->item.iSubItem<0||pstLVDInfo->item.iSubItem>=FILEINFO_ITEM_COUNT)return 0;

	CString strBuffer;
	LPCTSTR lpText=NULL;
	switch(m_ColumnIndexArray[pstLVDInfo->item.iSubItem]){
	case FILEINFO_FILENAME:	//�t�@�C����
		if(pstLVDInfo->item.mask & LVIF_TEXT)lpText=lpNode->strTitle;
		if(pstLVDInfo->item.mask & LVIF_IMAGE)pstLVDInfo->item.iImage=m_ShellDataManager.GetIconIndex(lpNode->strExt);
		break;
	case FILEINFO_FULLPATH:	//�i�[�p�X
		if(pstLVDInfo->item.mask & LVIF_TEXT){
			if(m_bPathOnly){
				//'\\'�ł͂Ȃ�'/'�Ńp�X����؂��Ă����true
				bool bSlash = (-1!=lpNode->strFullPath.Find(_T('/')));
				//��x�u������
				strBuffer = lpNode->strFullPath;
				strBuffer.Replace(_T('/'),_T('\\'));

				//�t�@�C��������
				TCHAR buf[_MAX_PATH+1];
				_tcsncpy_s(buf, strBuffer, COUNTOF(buf));
				PathRemoveBackslash(buf);
				PathRemoveFileSpec(buf);
				if(_tcslen(buf)==0){
					//�����Ȃ��Ȃ����A�܂胋�[�g�f�B���N�g���ɂ���
					strBuffer = _T("\\");
				}else{
					PathAddBackslash(buf);
					strBuffer = buf;
				}

				if(bSlash){
					//���ɖ߂��u������
					strBuffer.Replace(_T('\\'),_T('/'));
				}
				lpText = strBuffer;
			}else{
				lpText=lpNode->strFullPath;
			}
		}
		break;
	case FILEINFO_ORIGINALSIZE:	//�T�C�Y(���k�O)
		if(pstLVDInfo->item.mask & LVIF_TEXT){
			FormatFileSize(strBuffer,lpNode->llOriginalSize);
			lpText=strBuffer;
		}
		break;
	case FILEINFO_COMPRESSEDSIZE:	//�T�C�Y(���k��)
		if(pstLVDInfo->item.mask & LVIF_TEXT){
			FormatFileSize(strBuffer,lpNode->llCompressedSize);
			lpText=strBuffer;
		}
		break;
	case FILEINFO_TYPENAME:	//�t�@�C���^�C�v
		if(pstLVDInfo->item.mask & LVIF_TEXT)lpText=m_ShellDataManager.GetTypeName(lpNode->strExt);
		break;
	case FILEINFO_FILETIME:	//�t�@�C������
		if(pstLVDInfo->item.mask & LVIF_TEXT){
			FormatFileTime(strBuffer,lpNode->cFileTime);
			lpText=strBuffer;
		}
		break;
	case FILEINFO_ATTRIBUTE:	//����
		if(pstLVDInfo->item.mask & LVIF_TEXT){
			FormatAttribute(strBuffer,lpNode->nAttribute);
			lpText=strBuffer;
		}
		break;
	case FILEINFO_METHOD:	//���k���\�b�h
		if(pstLVDInfo->item.mask & LVIF_TEXT)lpText=lpNode->strMethod;
		break;
	case FILEINFO_RATIO:	//���k��
		if(pstLVDInfo->item.mask & LVIF_TEXT){
			FormatRatio(strBuffer,lpNode->wRatio);
			lpText=strBuffer;
		}
		break;
	case FILEINFO_CRC:	//CRC
		if(pstLVDInfo->item.mask & LVIF_TEXT){
			FormatCRC(strBuffer,lpNode->dwCRC);
			lpText=strBuffer;
		}
		break;
	}

	if(lpText){
		_tcsncpy_s(pstLVDInfo->item.pszText,pstLVDInfo->item.cchTextMax, lpText,pstLVDInfo->item.cchTextMax);
	}
	return 0;
}

//���z���X�g�r���[�̃c�[���`�b�v�擾�ɔ���
LRESULT CFileListView::OnGetInfoTip(LPNMHDR pnmh)
{
	LPNMLVGETINFOTIP pGetInfoTip=(LPNMLVGETINFOTIP)pnmh;

	ARCHIVE_ENTRY_INFO_TREE* lpNode=mr_Model.GetFileListItemByIndex(pGetInfoTip->iItem);
	ASSERT(lpNode);
	if(!lpNode)return 0;
	CString strInfo;
	CString strBuffer;

	//�t�@�C����
	strInfo+=CString(MAKEINTRESOURCE(IDS_FILELIST_COLUMN_FILENAME));
	strInfo+=_T(" : ");	strInfo+=lpNode->strTitle;		strInfo+=_T("\n");
	//�i�[�p�X
	strInfo+=CString(MAKEINTRESOURCE(IDS_FILELIST_COLUMN_FULLPATH));
	strInfo+=_T(" : ");	strInfo+=lpNode->strFullPath;	strInfo+=_T("\n");
	//���k�O�T�C�Y
	FormatFileSize(strBuffer,lpNode->llOriginalSize);
	strInfo+=CString(MAKEINTRESOURCE(IDS_FILELIST_COLUMN_ORIGINALSIZE));
	strInfo+=_T(" : ");	strInfo+=strBuffer;		strInfo+=_T("\n");
	//�t�@�C���^�C�v
	strInfo+=CString(MAKEINTRESOURCE(IDS_FILELIST_COLUMN_TYPENAME));
	strInfo+=_T(" : ");	strInfo+=m_ShellDataManager.GetTypeName(lpNode->strExt);	strInfo+=_T("\n");
	//�t�@�C������
	FormatFileTime(strBuffer,lpNode->cFileTime);
	strInfo+=CString(MAKEINTRESOURCE(IDS_FILELIST_COLUMN_FILETIME));
	strInfo+=_T(" : ");	strInfo+=strBuffer;		strInfo+=_T("\n");
	//����
	FormatAttribute(strBuffer,lpNode->nAttribute);
	strInfo+=CString(MAKEINTRESOURCE(IDS_FILELIST_COLUMN_ATTRIBUTE));
	strInfo+=_T(" : ");	strInfo+=strBuffer;		strInfo+=_T("\n");
	//���k��T�C�Y
	FormatFileSize(strBuffer,lpNode->llCompressedSize);
	strInfo+=CString(MAKEINTRESOURCE(IDS_FILELIST_COLUMN_COMPRESSEDSIZE));
	strInfo+=_T(" : ");	strInfo+=strBuffer;		strInfo+=_T("\n");
	//���k���\�b�h
	strInfo+=CString(MAKEINTRESOURCE(IDS_FILELIST_COLUMN_METHOD));
	strInfo+=_T(" : ");	strInfo+=lpNode->strMethod;	strInfo+=_T("\n");
	//���k��
	FormatRatio(strBuffer,lpNode->wRatio);
	strInfo+=CString(MAKEINTRESOURCE(IDS_FILELIST_COLUMN_RATIO));
	strInfo+=_T(" : ");	strInfo+=strBuffer;		strInfo+=_T("\n");
	//CRC
	FormatCRC(strBuffer,lpNode->dwCRC);
	strInfo+=CString(MAKEINTRESOURCE(IDS_FILELIST_COLUMN_CRC));
	strInfo+=_T(" : ");	strInfo+=strBuffer;		strInfo+=_T("\n");

	if(lpNode->nAttribute&FA_DIREC){
		//--------------
		// �f�B���N�g��
		//--------------
		//�q�m�[�h�̐���\��
		strInfo+=_T("\n");
		strInfo+=CString(MAKEINTRESOURCE(IDS_FILELIST_SUBITEM));
		strInfo+=_T(" : ");
		strInfo.AppendFormat(IDS_FILELIST_ITEMCOUNT,lpNode->GetNumChildren());
	}

	//InfoTip�ɐݒ�
	_tcsncpy_s(pGetInfoTip->pszText,pGetInfoTip->cchTextMax,strInfo,pGetInfoTip->cchTextMax);
	return 0;
}

void CFileListView::FormatFileSizeInBytes(CString &Info,const LARGE_INTEGER &_Size)
{
	CString format(MAKEINTRESOURCE(IDS_ORDERUNIT_BYTE));

	std::wstringstream ss;
	ss.imbue(std::locale(""));
	ss << (unsigned __int64)_Size.QuadPart;

	Info.Format(format,ss.str().c_str());
}

void CFileListView::FormatFileSize(CString &Info,const LARGE_INTEGER &_Size)
{
	LARGE_INTEGER Size=_Size;
	bool bInByte=m_bDisplayFileSizeInByte;
	Info.Empty();
	if(-1==Size.LowPart&&-1==Size.HighPart){
		Info=_T("---");
		return;
	}

	static CString OrderUnit[]={
		MAKEINTRESOURCE(IDS_ORDERUNIT_BYTE),
		MAKEINTRESOURCE(IDS_ORDERUNIT_KILOBYTE),
		MAKEINTRESOURCE(IDS_ORDERUNIT_MEGABYTE),
		MAKEINTRESOURCE(IDS_ORDERUNIT_GIGABYTE),
		MAKEINTRESOURCE(IDS_ORDERUNIT_TERABYTE),
	};	//�T�C�Y�̒P��
	static const int MAX_ORDERUNIT=COUNTOF(OrderUnit);

	if(bInByte){	//�t�@�C���T�C�Y���o�C�g�P�ʂŕ\�L����
		FormatFileSizeInBytes(Info,_Size);
		return;
	}

	int Order=0;
	for(;Order<MAX_ORDERUNIT;Order++){
		if(Size.QuadPart<1024*1024){
			break;
		}
		Size.QuadPart=Int64ShrlMod32(Size.QuadPart,10);	//1024�Ŋ���
	}
	if(0==Order && Size.QuadPart<1024){
		//1KB�ɖ����Ȃ��̂Ńo�C�g�P�ʂł��̂܂ܕ\�L
		FormatFileSizeInBytes(Info,_Size);
	}else{
		TCHAR Buffer[64]={0};
		if(Order<MAX_ORDERUNIT-1){
			double SizeToDisplay=Size.QuadPart/1024.0;
			Order++;
			Info.Format(OrderUnit[Order],SizeToDisplay);
		}else{
			//�ߑ�T�C�Y
			Info.Format(OrderUnit[Order],Size.QuadPart);
		}
	}
}

void CFileListView::FormatFileTime(CString &Info,const FILETIME &rFileTime)
{
	Info.Empty();
	if(-1==rFileTime.dwHighDateTime && -1==rFileTime.dwLowDateTime){
		Info=_T("------");
	}else{
		FILETIME LocalFileTime;
		SYSTEMTIME SystemTime;

		FileTimeToLocalFileTime(&rFileTime,&LocalFileTime);
		FileTimeToSystemTime(&LocalFileTime, &SystemTime);
		TCHAR Buffer[64];

		wsprintf(Buffer,CString(MAKEINTRESOURCE(IDS_FILELIST_FILETIME_FORMAT)),
				SystemTime.wYear, SystemTime.wMonth, SystemTime.wDay,
				SystemTime.wHour, SystemTime.wMinute,SystemTime.wSecond);
		Info+=Buffer;
	}
}

void CFileListView::FormatAttribute(CString &strBuffer,int nAttribute)
{
	if(nAttribute&FA_UNKNOWN){
		strBuffer=_T("?????");
	}else{
		strBuffer="";
		strBuffer+=(nAttribute&FA_RDONLY)	? _T("R") : _T("-");
		strBuffer+=(nAttribute&FA_HIDDEN)	? _T("H") : _T("-");
		strBuffer+=(nAttribute&FA_SYSTEM)	? _T("S") : _T("-");
		strBuffer+=(nAttribute&FA_DIREC)	? _T("D") : _T("-");
		strBuffer+=(nAttribute&FA_ARCH)		? _T("A") : _T("-");
		strBuffer+=(nAttribute&FA_ENCRYPTED)? _T("P") : _T("-");
	}
}

void CFileListView::FormatRatio(CString &strBuffer,WORD wRatio)
{
	if(0xFFFF==wRatio)strBuffer=_T("?????");	//�擾���s
	else strBuffer.Format(_T("%.1f%%"),(double)wRatio/10.0);
}

void CFileListView::FormatCRC(CString &strBuffer,DWORD dwCRC)
{
	if(-1==dwCRC)strBuffer=_T("?????");	//�擾���s
	else strBuffer.Format(_T("%08x"),dwCRC);
}

//-------

void CFileListView::OnClearTemporary(UINT uNotifyCode,int nID,HWND hWndCtrl)
{
	mr_Model.ClearTempDir();
}


void CFileListView::OnOpenAssociation(UINT uNotifyCode,int nID,HWND hWndCtrl)
{
	//true�Ȃ瑶�݂���e���|�����t�@�C�����폜���Ă���𓀂���
	const bool bOverwrite=(nID==ID_MENUITEM_OPEN_ASSOCIATION_OVERWRITE);
	OpenAssociation(bOverwrite,true);
}


void CFileListView::OnExtractTemporary(UINT uNotifyCode,int nID,HWND hWndCtrl)
{
	//�㏑���͂��邪�A�J���Ȃ�
	OpenAssociation(true,false);
}

//bOverwrite:true�Ȃ瑶�݂���e���|�����t�@�C�����폜���Ă���𓀂���
bool CFileListView::OpenAssociation(bool bOverwrite,bool bOpen)
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
		if(mr_Model.MakeSureItemsExtracted(NULL,mr_Model.GetRootNode(),items,filesList,bOverwrite,strLog)){
			if(bOpen)OpenAssociation(filesList);
		}else{
			CLogDialog LogDialog;
			LogDialog.SetData(strLog);
			LogDialog.DoModal();
		}
	}

	return true;
}

void CFileListView::OpenAssociation(const std::list<CString> &filesList)
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

HRESULT CFileListView::AddItems(const std::list<CString> &fileList,LPCTSTR strDest)
{
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
}

void CFileListView::OnAddItems(UINT uNotifyCode,int nID,HWND hWndCtrl)
{
	ASSERT(mr_Model.IsAddItemsSupported());
	if(!mr_Model.IsAddItemsSupported())return;

	CString strDest;	//���荞�ސ�
	//�J�����g�t�H���_�ɒǉ�
	ArcEntryInfoTree_GetNodePathRelative(mr_Model.GetCurrentNode(),mr_Model.GetRootNode(),strDest);

	std::list<CString> fileList;
	if(nID==ID_MENUITEM_ADD_FILE){		//�t�@�C���ǉ�
		//�u�S�Ẵt�@�C���v�̃t�B���^���������
		CString strAnyFile(MAKEINTRESOURCE(IDS_FILTER_ANYFILE));
		std::vector<TCHAR> filter(strAnyFile.GetLength()+1);
		UtilMakeFilterString(strAnyFile,&filter[0],filter.size());

		CMultiFileDialog dlg(NULL, NULL, OFN_NOCHANGEDIR|OFN_DONTADDTORECENT|OFN_PATHMUSTEXIST|OFN_HIDEREADONLY|OFN_ALLOWMULTISELECT,&filter[0]);
		if(IDOK==dlg.DoModal()){
			//�t�@�C�������o��
			CString tmp;
			if(dlg.GetFirstPathName(tmp)){
				do{
					fileList.push_back(tmp);
				}while(dlg.GetNextPathName(tmp));
			}
		}
	}else{		//�t�H���_�ǉ�
		CLFFolderDialog dlg(m_hFrameWnd,NULL,BIF_RETURNONLYFSDIRS|BIF_NEWDIALOGSTYLE);
		if(IDOK==dlg.DoModal()){
			fileList.push_back(dlg.GetFolderPath());
		}
	}

	if(!fileList.empty()){
		//�ǉ��J�n
		AddItems(fileList,strDest);
	}
}


void CFileListView::EnableDropTarget(bool bEnable)
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
HRESULT CFileListView::DragEnter(IDataObject *lpDataObject,POINTL &pt,DWORD &dwEffect)
{
	return DragOver(lpDataObject,pt,dwEffect);
}

HRESULT CFileListView::DragLeave()
{
	//�S�Ẵn�C���C�g�𖳌���
	SetItemState( -1, ~LVIS_DROPHILITED, LVIS_DROPHILITED);
	m_nDropHilight=-1;
	return S_OK;
}

HRESULT CFileListView::DragOver(IDataObject *,POINTL &pt,DWORD &dwEffect)
{
	//�t�H�[�}�b�g�ɑΉ���������������
	if(!m_DropTarget.QueryFormat(CF_HDROP) || !mr_Model.IsAddItemsSupported()){	//�t�@�C����p
		//�t�@�C���ł͂Ȃ��̂ŋ���
		dwEffect = DROPEFFECT_NONE;
	}else{
		//---�h���b�v��A�C�e�����擾
		CPoint ptTemp(pt.x,pt.y);
		ScreenToClient(&ptTemp);
		int nIndex=HitTest(ptTemp,NULL);

		//---��������������邽�߁A�O�Ɠ����A�C�e�����n�C���C�g�����Ȃ�n�C���C�g���N���A���Ȃ�
		if(nIndex!=m_nDropHilight){
			//�S�Ẵn�C���C�g�𖳌���
			SetItemState( -1, ~LVIS_DROPHILITED, LVIS_DROPHILITED);
			m_nDropHilight=-1;
			ARCHIVE_ENTRY_INFO_TREE* lpNode=mr_Model.GetFileListItemByIndex(nIndex);
			if(lpNode){		//�A�C�e�����DnD
				//�A�C�e�����t�H���_��������n�C���C�g
				if(lpNode->bDir){
					SetItemState( nIndex, LVIS_DROPHILITED, LVIS_DROPHILITED);
					m_nDropHilight=nIndex;
				}
			}
		}
		dwEffect = DROPEFFECT_COPY;
	}
	return S_OK;
}

//�t�@�C���̃h���b�v
HRESULT CFileListView::Drop(IDataObject *lpDataObject,POINTL &pt,DWORD &dwEffect)
{
	//�S�Ẵn�C���C�g�𖳌���
	SetItemState( -1, ~LVIS_DROPHILITED, LVIS_DROPHILITED);
	m_nDropHilight=-1;

	//�t�@�C���擾
	std::list<CString> fileList;
	if(S_OK==m_DropTarget.GetDroppedFiles(lpDataObject,fileList)){
		dwEffect = DROPEFFECT_COPY;

		//---�h���b�v������
		CPoint ptTemp(pt.x,pt.y);
		ScreenToClient(&ptTemp);
		int nIndex=HitTest(ptTemp,NULL);

		CString strDest;	//���荞�ސ�
		ARCHIVE_ENTRY_INFO_TREE* lpNode=mr_Model.GetFileListItemByIndex(nIndex);
		if(lpNode){		//�A�C�e�����DnD
			//�A�C�e�����t�H���_�������炻�̃t�H���_�ɒǉ�
			if(lpNode->bDir){
				ArcEntryInfoTree_GetNodePathRelative(lpNode,mr_Model.GetRootNode(),strDest);
			}else{
				//�J�����g�t�H���_�ɒǉ�
				ArcEntryInfoTree_GetNodePathRelative(mr_Model.GetCurrentNode(),mr_Model.GetRootNode(),strDest);
			}
		}else{	//�A�C�e���O��DnD->�J�����g�t�H���_�ɒǉ�
			ArcEntryInfoTree_GetNodePathRelative(mr_Model.GetCurrentNode(),mr_Model.GetRootNode(),strDest);
		}
		TRACE(_T("Target:%s\n"),(LPCTSTR)strDest);

		//�ǉ��J�n
		return AddItems(fileList,strDest);
	}else{
		//�󂯓���ł��Ȃ��`��
		dwEffect = DROPEFFECT_NONE;
		return S_FALSE;	//S_OK
	}
}

//-----------------------------
// �h���b�O&�h���b�v�ɂ���
//-----------------------------

LRESULT CFileListView::OnBeginDrag(LPNMHDR pnmh)
{
	if(!mr_Model.IsExtractEachSupported()){
		//�I���t�@�C���̉𓀂̓T�|�[�g����Ă��Ȃ�
		//ErrorMessage(CString(MAKEINTRESOURCE(IDS_ERROR_FILELIST_EXTRACT_SELECTED_NOT_SUPPORTED)));
		MessageBeep(MB_ICONASTERISK);
		return 0;
	}

	if(!mr_Model.CheckArchiveExists()){	//���݂��Ȃ��Ȃ�G���[
		return 0;
	}
	//�I�����ꂽ�A�C�e�����
	std::list<ARCHIVE_ENTRY_INFO_TREE*> items;
	GetSelectedItems(items);
	if(items.empty()){	//�{�����蓾�Ȃ�
		ASSERT(!"This code cannot be run");
		return 0;
	}

	if(!m_TempDirMgr.ClearSubDir()){
		//�e���|�����f�B���N�g������ɏo���Ȃ�
		ErrorMessage(CString(MAKEINTRESOURCE(IDS_ERROR_CANT_CLEAR_TEMPDIR)));
		return 0;
	}else{
		::EnableWindow(m_hFrameWnd,FALSE);

		//�h���b�O&�h���b�v�ŉ�
		CString strLog;
		HRESULT hr=m_DnDSource.DragDrop(mr_Model,items,mr_Model.GetCurrentNode(),m_TempDirMgr.GetDirPath(),strLog);
		if(FAILED(hr)){
			if(hr==E_ABORT){
				CLogDialog LogDialog;
				LogDialog.SetData(strLog);
				LogDialog.DoModal();
			}else{
				ErrorMessage(strLog);
			}
		}

		::EnableWindow(m_hFrameWnd,TRUE);
		::SetForegroundWindow(m_hFrameWnd);
	}
	return 0;
}

void CFileListView::OnSelectAll(UINT,int,HWND)
{
	SetItemState(-1, LVIS_SELECTED, LVIS_SELECTED);
	SetFocus();
}


void CFileListView::OnDelete(UINT uNotifyCode,int nID,HWND hWndCtrl)
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

//�R���e�L�X�g���j���[���J��
void CFileListView::OnContextMenu(HWND hWndCtrl,CPoint &Point)
{
	//�����I�����Ă��Ȃ���Ε\�����Ȃ�
	if(GetSelectedCount()==0)return;

	if(-1==Point.x&&-1==Point.y){
		//�L�[�{�[�h����̓��͂ł���ꍇ
		//���X�g�r���[�̍���ɕ\������
		Point.x=Point.y=0;
		ClientToScreen(&Point);
	}else{
		//�}�E�X�E�{�^�����t�@�C���ꗗ�E�B���h�E��*�J����*�ŉ����ꂽ�̂ł���ΕԂ�
		HDHITTESTINFO hdHitTestInfo={0};
		hdHitTestInfo.pt=Point;
		GetHeader().ScreenToClient(&hdHitTestInfo.pt);
		GetHeader().HitTest(&hdHitTestInfo);
		if(hdHitTestInfo.flags&HHT_ONHEADER){
			return;
		}
	}

	//�����𓀂��g�p�ł��Ȃ��Ȃ�A���j���[��\������Ӗ����Ȃ�
	//TODO:�폜���j���[�͂ǂ�����?�����𓀂ł����ɍ폜�\�͂قڂ��蓾�Ȃ�
	if(!mr_Model.IsExtractEachSupported()){
		MessageBeep(MB_ICONASTERISK);
		return;
	}

	//---�E�N���b�N���j���[�\��
	CMenu cMenu;
	cMenu.LoadMenu(IDR_FILELIST_POPUP);
	CMenuHandle cSubMenu(cMenu.GetSubMenu(0));

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

	cSubMenu.TrackPopupMenu(TPM_LEFTALIGN|TPM_RIGHTBUTTON,Point.x, Point.y, m_hWnd,NULL);
}

void CFileListView::OnCopyInfo(UINT uNotifyCode,int nID,HWND hWndCtrl)
{
	//�I�����ꂽ�A�C�e�����
	std::list<ARCHIVE_ENTRY_INFO_TREE*> items;
	GetSelectedItems(items);
	std::list<ARCHIVE_ENTRY_INFO_TREE*>::iterator ite=items.begin();
	std::list<ARCHIVE_ENTRY_INFO_TREE*>::iterator end=items.end();

	CString info;

	switch(nID){
	case ID_MENUITEM_COPY_FILENAME:
		for(;ite!=end;++ite){
			ARCHIVE_ENTRY_INFO_TREE* lpItem = *ite;
			info.AppendFormat(_T("%s\n"),(LPCTSTR)lpItem->strTitle);
		}
		break;
	case ID_MENUITEM_COPY_PATH:
		for(;ite!=end;++ite){
			ARCHIVE_ENTRY_INFO_TREE* lpItem = *ite;
			info.AppendFormat(_T("%s\n"),(LPCTSTR)lpItem->strFullPath);
		}
		break;
	case ID_MENUITEM_COPY_ORIGINAL_SIZE:
		for(;ite!=end;++ite){
			ARCHIVE_ENTRY_INFO_TREE* lpItem = *ite;
			info.AppendFormat(_T("%I64d\n"),lpItem->llOriginalSize.QuadPart);
		}
		break;
	case ID_MENUITEM_COPY_FILETYPE:
		for(;ite!=end;++ite){
			ARCHIVE_ENTRY_INFO_TREE* lpItem = *ite;
			info.AppendFormat(_T("%s\n"),m_ShellDataManager.GetTypeName(lpItem->strExt));
		}
		break;
	case ID_MENUITEM_COPY_FILETIME:
		for(;ite!=end;++ite){
			ARCHIVE_ENTRY_INFO_TREE* lpItem = *ite;
			CString strBuffer;
			FormatFileTime(strBuffer,lpItem->cFileTime);
			info.AppendFormat(_T("%s\n"),(LPCTSTR)strBuffer);
		}
		break;
	case ID_MENUITEM_COPY_ATTRIBUTE:
		for(;ite!=end;++ite){
			ARCHIVE_ENTRY_INFO_TREE* lpItem = *ite;
			CString strBuffer;
			FormatAttribute(strBuffer,lpItem->nAttribute);
			info.AppendFormat(_T("%s\n"),(LPCTSTR)strBuffer);
		}
		break;
	case ID_MENUITEM_COPY_COMPRESSED_SIZE:
		for(;ite!=end;++ite){
			ARCHIVE_ENTRY_INFO_TREE* lpItem = *ite;
			info.AppendFormat(_T("%I64d\n"),lpItem->llCompressedSize.QuadPart);
		}
		break;
	case ID_MENUITEM_COPY_METHOD:
		for(;ite!=end;++ite){
			ARCHIVE_ENTRY_INFO_TREE* lpItem = *ite;
			info.AppendFormat(_T("%s\n"),(LPCTSTR)lpItem->strMethod);
		}
		break;
	case ID_MENUITEM_COPY_COMPRESSION_RATIO:
		for(;ite!=end;++ite){
			ARCHIVE_ENTRY_INFO_TREE* lpItem = *ite;
			CString strBuffer;
			FormatRatio(strBuffer,lpItem->wRatio);
			info.AppendFormat(_T("%s\n"),(LPCTSTR)strBuffer);
		}
		break;
	case ID_MENUITEM_COPY_CRC:
		for(;ite!=end;++ite){
			ARCHIVE_ENTRY_INFO_TREE* lpItem = *ite;
			CString strBuffer;
			FormatCRC(strBuffer,lpItem->dwCRC);
			info.AppendFormat(_T("%s\n"),(LPCTSTR)strBuffer);
		}
		break;
	case ID_MENUITEM_COPY_ALL:
		info=_T("FileName\tFullPath\tOriginalSize\tFileType\tFileTime\tAttribute\tCompressedSize\tMethod\tCompressionRatio\tCRC\n");
		for(;ite!=end;++ite){
			ARCHIVE_ENTRY_INFO_TREE* lpItem = *ite;
			CString strFileTime, strAttrib, strRatio, strCRC;
			FormatFileTime(strFileTime,lpItem->cFileTime);
			FormatAttribute(strAttrib,lpItem->nAttribute);
			FormatRatio(strRatio,lpItem->wRatio);
			FormatCRC(strCRC,lpItem->dwCRC);

			info.AppendFormat(_T("%s\t%s\t%I64d\t%s\t%s\t%s\t%I64d\t%s\t%s\t%s\n"),
				(LPCTSTR)lpItem->strTitle,
				(LPCTSTR)lpItem->strFullPath,
				lpItem->llOriginalSize.QuadPart,
				m_ShellDataManager.GetTypeName(lpItem->strExt),
				(LPCTSTR)strFileTime,
				(LPCTSTR)strAttrib,
				lpItem->llCompressedSize.QuadPart,
				(LPCTSTR)lpItem->strMethod,
				(LPCTSTR)strRatio,
				(LPCTSTR)strCRC);
		}
		break;
	default:
		ASSERT(!"Unknown command");
	}
	//MessageBox(info);
	UtilSetTextOnClipboard(info);
}

void CFileListView::OnOpenWithUserApp(UINT uNotifyCode,int nID,HWND hWndCtrl)
{
	if(nID<ID_MENUITEM_USERAPP_END){
		//LhaForge�ݒ�̃R�}���h
		OnUserApp(MenuCommand_GetCmdArray(),nID-ID_MENUITEM_USERAPP_BEGIN);
	}else{
		//SendTo�̃R�}���h
		OnSendToApp(nID-ID_MENUITEM_USERAPP_END);
	}
}


bool CFileListView::OnUserApp(const std::vector<CMenuCommandItem> &menuCommandArray,UINT nID)	//�u�v���O�����ŊJ���v�̃n���h��
{
	ASSERT(!menuCommandArray.empty());
	ASSERT(nID<menuCommandArray.size());
	if(nID>=menuCommandArray.size())return false;

	if(!mr_Model.IsOK())return false;

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

bool CFileListView::OnSendToApp(UINT nID)	//�u�v���O�����ŊJ���v�̃n���h��
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

void CFileListView::OnExtractItem(UINT,int nID,HWND)
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
	HRESULT hr=mr_Model.ExtractItems(m_hFrameWnd,bSameDir,items,mr_Model.GetCurrentNode(),strLog);

	SetForegroundWindow(m_hFrameWnd);

	if(FAILED(hr)){
		CLogDialog LogDialog;
		LogDialog.SetData(strLog);
		LogDialog.DoModal();
	}
}

void CFileListView::OnFindItem(UINT uNotifyCode,int nID,HWND hWndCtrl)
{
	if(ID_MENUITEM_FINDITEM_END==nID){
		mr_Model.EndFindItem();
	}else{
		CString strSpec;
		if(UtilInputText(CString(MAKEINTRESOURCE(IDS_INPUT_FIND_PARAM)),strSpec)){
			mr_Model.EndFindItem();
			ARCHIVE_ENTRY_INFO_TREE* lpFound=mr_Model.FindItem(strSpec,mr_Model.GetCurrentNode());
			mr_Model.SetCurrentNode(lpFound);
		}
	}
}

void CFileListView::OnShowCustomizeColumn(UINT,int,HWND)
{
	//�J�����w�b�_�ҏW���j���[��\�����邽�߁A�J�����w�b�_�̉E�N���b�N���G�~�����[�g
	BOOL bTemp;
	NMHDR nmhdr;
	nmhdr.hwndFrom=GetHeader();
	OnColumnRClick(0, &nmhdr, bTemp);
}
