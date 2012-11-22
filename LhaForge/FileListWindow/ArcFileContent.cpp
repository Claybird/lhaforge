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
#include "ArcFileContent.h"
#include "../Utilities/StringUtil.h"
#include "../Utilities/FileOperation.h"
#include "../ArchiverCode/arc_interface.h"
#include "../ArchiverManager.h"

CArchiveFileContent::CArchiveFileContent():m_bReadOnly(false)
{
	Clear();
}

CArchiveFileContent::~CArchiveFileContent()
{
	Clear();
}

void CArchiveFileContent::Clear()
{
	m_Root.Clear();
	m_Root.lpParent=NULL;

	m_bReadOnly=false;
	m_GC.DeleteAll();

	m_lpArchiver=NULL;
	m_pathArcFileName.Empty();
	m_bExtractEachSupported=false;
}

ARCHIVE_ENTRY_INFO_TREE* CArchiveFileContent::ForceFindEntry(ARCHIVE_ENTRY_INFO_TREE* lpParent,LPCTSTR lpName)
{
	ASSERT(lpParent);
	ARCHIVE_ENTRY_INFO_TREE::DICT::iterator ite=lpParent->childrenDict.find(lpName);
	if(lpParent->childrenDict.end()==ite){
		ARCHIVE_ENTRY_INFO_TREE* lpTree=new ARCHIVE_ENTRY_INFO_TREE;
		m_GC.Add(lpTree);
		lpTree->Clear();

		lpTree->lpParent=lpParent;
		lpParent->childrenDict.insert(ARCHIVE_ENTRY_INFO_TREE::DICT::value_type(lpName,lpTree));
		lpParent->childrenArray.push_back(lpTree);
		return lpTree;
	}else{
		return (*ite).second;
	}
}

HRESULT CArchiveFileContent::InspectArchiveStruct(LPCTSTR lpFile,CConfigManager &ConfMan,CArchiverDLL *lpArchiver,std::vector<ARCHIVE_ENTRY_INFO> &entries,IArchiveContentUpdateHandler* lpHandler)
{
	//��͊J�n
	TRACE(_T("�f�B���N�g���\����͊J�n\n"));
	if(!lpArchiver->InspectArchiveBegin(lpFile,ConfMan)){
		//�X�L�����ł��Ȃ�
		return E_LF_FILELIST_NOT_SUPPORTED;
	}

	HRESULT hr=S_OK;
	//�ꗗ�擾
	while(lpArchiver->InspectArchiveNext()){
		ARCHIVE_ENTRY_INFO item;

		//�i�[�t�@�C�����擾
		lpArchiver->InspectArchiveGetFileName(item.strFullPath);
		//�����擾
		item.nAttribute=lpArchiver->InspectArchiveGetAttribute();
		//�t�@�C���T�C�Y(���k�O)
		if(!lpArchiver->InspectArchiveGetOriginalFileSize(item.llOriginalSize)){
			item.llOriginalSize.LowPart=-1;
			item.llOriginalSize.HighPart=-1;
		}
		//�t�@�C���T�C�Y(���k��)
		if(!lpArchiver->InspectArchiveGetCompressedFileSize(item.llCompressedSize)){
			item.llCompressedSize.LowPart=-1;
			item.llCompressedSize.HighPart=-1;
		}
		//�����擾
		if(!lpArchiver->InspectArchiveGetWriteTime(item.cFileTime)){
			item.cFileTime.dwLowDateTime=-1;
			item.cFileTime.dwHighDateTime=-1;
		}
		//���k��
		item.wRatio=lpArchiver->InspectArchiveGetRatio();
		//CRC
		item.dwCRC=lpArchiver->InspectArchiveGetCRC();
		//���\�b�h
		lpArchiver->InspectArchiveGetMethodString(item.strMethod);

		//�o�^		
		entries.push_back(item);

		//�X�V
		if(lpHandler){
			while(UtilDoMessageLoop())continue;
			lpHandler->onUpdated(item);
			if(lpHandler->isAborted()){
				hr=E_ABORT;
				break;
			}
		}
	}
	//��͏I��
	lpArchiver->InspectArchiveEnd();

	return hr;
}


HRESULT CArchiveFileContent::ConstructFlat(LPCTSTR lpFile,CConfigManager &ConfMan,DLL_ID idForce,LPCTSTR lpDenyExt,bool bFilesOnly,CString &strErr,IArchiveContentUpdateHandler* lpHandler)
{
	Clear();

	m_bReadOnly = GetFileAttributes(lpFile) & FILE_ATTRIBUTE_READONLY;

	CArchiverDLL* lpArchiver=CArchiverDLLManager::GetInstance().GetArchiver(lpFile,lpDenyExt,idForce);
	if(!lpArchiver){
		//�s���Ȍ`�� or ��Ή�DLL��UNICODE�t�@�C�������������Ƃ���
		strErr.Format(IDS_FILELIST_FORMAT_UNKNOWN,lpFile);
		return E_LF_UNKNOWN_FORMAT;
	}
	if(!lpArchiver->QueryInspectSupported()){
		//�{���ł��Ȃ��`��
		strErr.Format(IDS_FILELIST_FORMAT_NOTSUPPORTED,lpFile);
		return E_LF_FILELIST_NOT_SUPPORTED;
	}

	//--�\���擾
	std::vector<ARCHIVE_ENTRY_INFO> entries;
	HRESULT hr=InspectArchiveStruct(lpFile,ConfMan,lpArchiver,entries,lpHandler);
	if(FAILED(hr)){
		ASSERT(hr==E_ABORT);
		if(hr==E_ABORT){
			strErr.Format(IDS_ERROR_USERCANCEL);
		}
		return hr;
	}
	//�L�^
	m_lpArchiver=lpArchiver;
	m_pathArcFileName=lpFile;
	m_bExtractEachSupported=m_lpArchiver->QueryExtractSpecifiedOnlySupported(m_pathArcFileName);

	//���
	size_t numEntries=entries.size();
	for(size_t i=0;i<numEntries;i++){
		if(bFilesOnly && entries[i].nAttribute&FA_DIREC)continue;	//�t�@�C���݂̂̏ꍇ�̓f�B���N�g���͖���

		ARCHIVE_ENTRY_INFO_TREE* lpEntry=new ARCHIVE_ENTRY_INFO_TREE;
		m_GC.Add(lpEntry);
		lpEntry->Clear();

		//�G���g���ݒ�
		*((ARCHIVE_ENTRY_INFO*)lpEntry)=entries[i];
		lpEntry->lpParent=&m_Root;
		UtilPathGetLastSection(lpEntry->strTitle,entries[i].strFullPath);

		m_Root.childrenDict.insert(ARCHIVE_ENTRY_INFO_TREE::DICT::value_type((LPCTSTR)lpEntry->strTitle,lpEntry));
		m_Root.childrenArray.push_back(lpEntry);

		//��Őݒ肷��
		lpEntry->bDir=false;
		lpEntry->bSafe=true;
	}

	PostProcess(m_lpArchiver->IsUnicodeCapable(),NULL);
	return S_OK;
}

HRESULT CArchiveFileContent::ConstructTree(LPCTSTR lpFile,CConfigManager &ConfMan,DLL_ID idForce,LPCTSTR lpDenyExt,bool bSkipMeaningless,CString &strErr,IArchiveContentUpdateHandler* lpHandler)
{
	Clear();

	m_bReadOnly = GetFileAttributes(lpFile) & FILE_ATTRIBUTE_READONLY;

	CArchiverDLL* lpArchiver=CArchiverDLLManager::GetInstance().GetArchiver(lpFile,lpDenyExt,idForce);
	if(!lpArchiver){
		//�s���Ȍ`�� or ��Ή�DLL��UNICODE�t�@�C�������������Ƃ���
		strErr.Format(IDS_FILELIST_FORMAT_UNKNOWN,lpFile);
		//UNICODE�֌W�̃`�F�b�N
		if(!UtilCheckT2A(lpFile)){
			//UNICODE�ɑΉ����Ă��Ȃ��̂�UNICODE�t�@�C�����̃t�@�C�����������Ƃ����\��������
			strErr+=_T("\r\n\r\n");
			strErr.AppendFormat(IDS_ERROR_UNICODEPATH);
		}
		return E_LF_UNKNOWN_FORMAT;
	}
	if(!lpArchiver->QueryInspectSupported()){
		//�{���ł��Ȃ��`��
		strErr.Format(IDS_FILELIST_FORMAT_NOTSUPPORTED,lpFile);
		return E_LF_FILELIST_NOT_SUPPORTED;
	}

	//--�\���擾
	std::vector<ARCHIVE_ENTRY_INFO> entries;
	HRESULT hr=InspectArchiveStruct(lpFile,ConfMan,lpArchiver,entries,lpHandler);
	if(FAILED(hr)){
		ASSERT(hr==E_ABORT && "Not Implemented");
		if(hr==E_ABORT){
			strErr.Format(IDS_ERROR_USERCANCEL);
		}
		return hr;
	}

	//�L�^
	m_lpArchiver=lpArchiver;
	m_pathArcFileName=lpFile;
	m_bExtractEachSupported=m_lpArchiver->QueryExtractSpecifiedOnlySupported(m_pathArcFileName);

	//���
	//TODO:���ɏo�������f�B���N�g���݂̂�Ώۂɔ�r���s��
	size_t numEntries=entries.size();
	for(size_t i=0;i<numEntries;i++){
		ARCHIVE_ENTRY_INFO_TREE* lpEntry=&m_Root;
		LPCTSTR lpPath=entries[i].strFullPath;

		CString strEntry;
		CString strTitle;
		bool bBreak=false;
		for(;*lpPath!=L'\0';){
			LPCTSTR lpStart,lpEnd;
			if(UtilPathNextSection(lpPath,lpStart,lpEnd,bSkipMeaningless)){
				UtilAssignSubString(strEntry,lpStart,lpEnd);
				lpPath=lpEnd;
				if(*lpPath!=L'\0')lpPath++;
			}else{
				//�����s�\
				strEntry=lpPath;
				bBreak=true;
			}
			strTitle=strEntry;
			//��������
			strEntry.MakeLower();

			//�q�G���g������
			lpEntry=ForceFindEntry(lpEntry,strEntry);
			if(bBreak){
				break;
			}else{
				if(lpEntry->strTitle.IsEmpty()){
					//���z�f�B���N�g���̐ݒ�
					lpEntry->strTitle=strTitle;
				}
			}
		}
		ASSERT(lpEntry);
		//�G���g���ݒ�
		*((ARCHIVE_ENTRY_INFO*)lpEntry)=entries[i];

		lpEntry->strTitle=strTitle;

		//��Őݒ肷��
		lpEntry->bDir=false;
		lpEntry->bSafe=true;
	}

	PostProcess(m_lpArchiver->IsUnicodeCapable(),NULL);
	return S_OK;
}

/*
 * �t�@�C���ꗗ�ǉ���̏���
 * �f�B���N�g�����t�@�C���̃T�C�Y���擾����Ȃ�
 * bUnicode�͎g�p����DLL��UNICODE�ɑΉ����Ă���ꍇ�ɂ�true
 */
void CArchiveFileContent::PostProcess(bool bUnicode,ARCHIVE_ENTRY_INFO_TREE* pNode)
{
	if(!pNode)pNode=&m_Root;
	//����
	if(-1==pNode->nAttribute){	//�܂������������Ă��Ȃ��ꍇ
		if(!pNode->childrenDict.empty()||UtilPathEndWithSeparator(pNode->strFullPath)){
			//�ȉ��̏����̂����ꂩ�ɍ��v����΃f�B���N�g��
			//�E�m�[�h�������Ƀp�X��؂蕶�����t���Ă���(bDir)
			//�E�q�m�[�h����łȂ�
			pNode->nAttribute=FA_DIREC;
			pNode->bDir=true;
		}else{
			pNode->nAttribute=FA_UNKNOWN;
		}
	}else{
		if(!pNode->childrenDict.empty()||UtilPathEndWithSeparator(pNode->strFullPath)){
			pNode->nAttribute|=FA_DIREC;
			pNode->bDir=true;
		}
	}
	if(pNode->nAttribute&FA_DIREC){
		//�t�H���_�ɂ͊g���q�͂Ȃ�
		pNode->strExt=FOLDER_EXTENSION_STRING;

		pNode->llOriginalSize.QuadPart=0;
		pNode->llCompressedSize.QuadPart=0;
	}else{
		pNode->strExt=PathFindExtension(pNode->strFullPath);
	}

	//�댯����
	if(bUnicode){
		pNode->bSafe=UtilIsSafeUnicode(pNode->strFullPath);
	}

	//�q�m�[�h�ɂ��K�p
	size_t numChildren=pNode->childrenArray.size();
	for(size_t i=0;i<numChildren;i++){
		ARCHIVE_ENTRY_INFO_TREE* pChild=pNode->childrenArray[i];
		PostProcess(bUnicode,pChild);

		//---�f�B���N�g���Ȃ�A���̃f�[�^�̃T�C�Y���W�v����
		if(pNode->bDir){
			//---���k�O�T�C�Y
			if(pNode->llOriginalSize.QuadPart>=0){	//�t�@�C���T�C�Y�擾�Ɏ��s���Ă��Ȃ�
				if(pChild->llOriginalSize.QuadPart>=0){
					pNode->llOriginalSize.QuadPart+=pChild->llOriginalSize.QuadPart;
				}else{
					pNode->llOriginalSize.LowPart=-1;
					pNode->llOriginalSize.HighPart=-1;
				}
			}
			//---���k��T�C�Y
			if(pNode->llCompressedSize.QuadPart>=0){	//�t�@�C���T�C�Y�擾�Ɏ��s���Ă��Ȃ�
				if(pChild->llCompressedSize.QuadPart>=0){
					pNode->llCompressedSize.QuadPart+=pChild->llCompressedSize.QuadPart;
				}else{
					pNode->llCompressedSize.LowPart=-1;
					pNode->llCompressedSize.HighPart=-1;
				}
			}
		}
	}
}



void CArchiveFileContent::FindSubItem(LPCTSTR lpszMask,bool bMatchPath,const ARCHIVE_ENTRY_INFO_TREE *lpTop,std::vector<ARCHIVE_ENTRY_INFO_TREE*> &founds)const
{
	//���D��T��
	std::vector<ARCHIVE_ENTRY_INFO_TREE*> dirs;
	for(size_t i=0;i<lpTop->childrenArray.size();i++){
		ARCHIVE_ENTRY_INFO_TREE* lpNode=lpTop->childrenArray[i];
		ASSERT(lpNode);
		CString strKey;
		if(bMatchPath){	//�p�X������v
			strKey=lpNode->strFullPath;
			strKey.Replace(_T("/"),_T("\\"));
		}else{
			strKey=lpNode->strTitle;
		}
		if(::PathMatchSpec(strKey,lpszMask)){
			founds.push_back(lpNode);
		}
		if(lpNode->bDir){	//�f�B���N�g���͍ċA����
			dirs.push_back(lpNode);
		}
	}
	//�f�B���N�g��
	for(size_t i=0;i<dirs.size();i++){
		FindSubItem(lpszMask,bMatchPath,dirs[i],founds);
	}
}


void CArchiveFileContent::FindItem(LPCTSTR lpszMask,const ARCHIVE_ENTRY_INFO_TREE *lpTop,std::vector<ARCHIVE_ENTRY_INFO_TREE*> &founds)const
{
	founds.clear();
	ASSERT(lpTop);
	if(!lpTop)return;

	CString strMask(lpszMask);
	strMask.Replace(_T("/"),_T("\\"));
	bool bMatchPath=(-1!=strMask.Find(_T('\\')));

	//*��?���t���Ă��Ȃ��ꍇ��*�����������ɒǉ�
	if(-1==strMask.FindOneOf(_T("*?"))){
		strMask.Insert(0,_T("*"));
		strMask+=_T("*");
	}

	FindSubItem(strMask,bMatchPath,lpTop,founds);
}


bool CArchiveFileContent::ExtractItems(CConfigManager &ConfMan,const std::list<ARCHIVE_ENTRY_INFO_TREE*> &items,LPCTSTR lpszDir,const ARCHIVE_ENTRY_INFO_TREE* lpBase,bool bCollapseDir,CString &strLog)
{
	if(!IsExtractEachSupported()){
		//�I���t�@�C���̉𓀂̓T�|�[�g����Ă��Ȃ�
		return false;
	}

	return m_lpArchiver->ExtractItems(m_pathArcFileName,ConfMan,lpBase,items,lpszDir,bCollapseDir,strLog);
}

void CArchiveFileContent::CollectUnextractedFiles(LPCTSTR lpOutputDir,const ARCHIVE_ENTRY_INFO_TREE* lpBase,const ARCHIVE_ENTRY_INFO_TREE* lpParent,std::map<const ARCHIVE_ENTRY_INFO_TREE*,std::list<ARCHIVE_ENTRY_INFO_TREE*> > &toExtractList)
{
	size_t numChildren=lpParent->GetNumChildren();
	for(size_t i=0;i<numChildren;i++){
		ARCHIVE_ENTRY_INFO_TREE* lpNode=lpParent->GetChild(i);
		CPath path=lpOutputDir;

		CString strItem;
		ArcEntryInfoTree_GetNodePathRelative(lpNode,lpBase,strItem);
		path.Append(strItem);

		if(::PathIsDirectory(path)){
			// �t�H���_�����݂��邪���g�͂�����Ă��邩?
			CollectUnextractedFiles(lpOutputDir,lpBase,lpNode,toExtractList);
		}else if(!::PathFileExists(path)){
			// �L���b�V�������݂��Ȃ��̂ŁA�𓀗v�����X�g�ɉ�����
			toExtractList[lpParent].push_back(lpNode);
		}
	}
}


//bOverwrite:true�Ȃ瑶�݂���e���|�����t�@�C�����폜���Ă���𓀂���
bool CArchiveFileContent::MakeSureItemsExtracted(CConfigManager &ConfMan,LPCTSTR lpOutputDir,const ARCHIVE_ENTRY_INFO_TREE* lpBase,const std::list<ARCHIVE_ENTRY_INFO_TREE*> &items,std::list<CString> &r_filesList,bool bOverwrite,CString &strLog)
{
	//�I�����ꂽ�A�C�e�����
	std::map<const ARCHIVE_ENTRY_INFO_TREE*,std::list<ARCHIVE_ENTRY_INFO_TREE*> > toExtractList;

	for(std::list<ARCHIVE_ENTRY_INFO_TREE*>::const_iterator ite=items.begin();ite!=items.end();++ite){
		// ���݂��`�F�b�N���A�����𓀍ς݂ł���΂�����J��
		ARCHIVE_ENTRY_INFO_TREE* lpNode=*ite;
		CPath path=lpOutputDir;

		CString strItem;
		ArcEntryInfoTree_GetNodePathRelative(lpNode,lpBase,strItem);
		path.Append(strItem);

		if(bOverwrite){
			// �㏑���𓀂���̂ŁA���݂���t�@�C���͍폜
			if(lpNode->bDir){
				if(::PathIsDirectory(path))UtilDeleteDir(path,true);
			}else{
				if(::PathFileExists(path))UtilDeletePath(path);
			}
			//�𓀗v�����X�g�ɉ�����
			toExtractList[lpBase].push_back(lpNode);
		}else{	//�㏑���͂��Ȃ�
			if(::PathIsDirectory(path)){
				// �t�H���_�����݂��邪���g�͂�����Ă��邩?
				CollectUnextractedFiles(lpOutputDir,lpBase,lpNode,toExtractList);
			}else if(!::PathFileExists(path)){
				// �L���b�V�������݂��Ȃ��̂ŁA�𓀗v�����X�g�ɉ�����
				toExtractList[lpBase].push_back(lpNode);
			}
		}
		path.RemoveBackslash();
		//�J���\�胊�X�g�ɒǉ�
		r_filesList.push_back(path);
	}
	if(toExtractList.empty()){
		return true;
	}

	//���𓀂̕��݈̂ꎞ�t�H���_�ɉ�
	for(std::map<const ARCHIVE_ENTRY_INFO_TREE*,std::list<ARCHIVE_ENTRY_INFO_TREE*> >::iterator ite=toExtractList.begin();ite!=toExtractList.end();++ite){
		ExtractItems(ConfMan,(*ite).second,lpOutputDir,lpBase,false,strLog);
	}
	return true;
}


HRESULT CArchiveFileContent::AddItem(const std::list<CString> &fileList,LPCTSTR lpDestDir,CConfigManager& rConfig,CString &strLog)
{
	ASSERT(IsAddItemsSupported());
	if(!IsAddItemsSupported())return false;

	//---�t�@�C�����`�F�b�N
	bool bUnicode=IsUnicodeCapable();
	for(std::list<CString>::const_iterator ite=fileList.begin();ite!=fileList.end();++ite){
		if(0==m_pathArcFileName.CompareNoCase(*ite)){
			//�A�[�J�C�u���g��ǉ����悤�Ƃ���
			return E_LF_SAME_INPUT_AND_OUTPUT;
		}
		//---UNICODE�`�F�b�N
		if(!bUnicode){
			if(!UtilCheckT2A(*ite)){
				//�t�@�C������UNICODE���������t�@�C�������k���悤�Ƃ���
				return E_LF_UNICODE_NOT_SUPPORTED;
			}
		}
	}

	//---�ǉ�
	//���f�B���N�g���擾�Ȃǂ�CArchiverDLL���ɔC����
	if(m_lpArchiver->AddItemToArchive(m_pathArcFileName,fileList,rConfig,lpDestDir,strLog)){
		return S_OK;
	}else{
		return S_FALSE;
	}
}

bool CArchiveFileContent::DeleteItems(CConfigManager &ConfMan,const std::list<ARCHIVE_ENTRY_INFO_TREE*> &fileList,CString &strLog)
{
	ASSERT(IsDeleteItemsSupported());
	if(!IsDeleteItemsSupported())return false;
	//�폜�Ώۂ��
	std::list<CString> filesToDel;
	for(std::list<ARCHIVE_ENTRY_INFO_TREE*>::const_iterator ite=fileList.begin();ite!=fileList.end();++ite){
		(*ite)->EnumFiles(filesToDel);
	}
	return m_lpArchiver->DeleteItemFromArchive(m_pathArcFileName,ConfMan,filesToDel,strLog);
}

bool CArchiveFileContent::IsDeleteItemsSupported()const
{
	return (!m_bReadOnly) && m_lpArchiver && m_lpArchiver->QueryDeleteItemFromArchiveSupported(m_pathArcFileName);
}

bool CArchiveFileContent::IsAddItemsSupported()const
{
	return (!m_bReadOnly) && m_lpArchiver && m_lpArchiver->QueryAddItemToArchiveSupported(m_pathArcFileName);
}

bool CArchiveFileContent::IsUnicodeCapable()const
{
	return m_lpArchiver && m_lpArchiver->IsUnicodeCapable();
}

bool CArchiveFileContent::ReloadArchiverIfLost(CConfigManager &ConfigManager,CString &strErr)
{
	if(m_lpArchiver && !m_lpArchiver->IsOK()){
		return (LOAD_RESULT_OK==m_lpArchiver->LoadDLL(ConfigManager,strErr));
	}
	return true;
}
