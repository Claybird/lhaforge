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

#pragma once
#include "../ArchiverCode/ArcEntryInfo.h"
#include "../ConfigCode/ConfigManager.h"
#include "../Utilities/PtrCollection.h"

struct IArchiveContentUpdateHandler{
	virtual ~IArchiveContentUpdateHandler(){}
	virtual void onUpdated(ARCHIVE_ENTRY_INFO&)=0;
	virtual bool isAborted()=0;
};


/*
 * �A�[�J�C�u���̃t�@�C���\����ێ�
 */
class CArchiverDLL;
enum DLL_ID;
class CArchiveFileContent{
protected:
	//�t�@�C�����
	ARCHIVE_ENTRY_INFO_TREE m_Root;

	//Semi-Auto Garbage Collector
	CSmartPtrCollection<ARCHIVE_ENTRY_INFO> m_GC;

	CArchiverDLL*	m_lpArchiver;
	CString			m_pathArcFileName;
	bool			m_bExtractEachSupported;
	bool			m_bReadOnly;
protected:
	//---internal functions
	ARCHIVE_ENTRY_INFO_TREE* ForceFindEntry(ARCHIVE_ENTRY_INFO_TREE* lpParent,LPCTSTR lpName);

	//bMatchPath:true�Ȃ�p�X���܂ߌ����Afalse�Ȃ�t�@�C�����̂݌���
	void FindSubItem(LPCTSTR lpszMask,bool bMatchPath,const ARCHIVE_ENTRY_INFO_TREE *lpTop,std::vector<ARCHIVE_ENTRY_INFO_TREE*> &founds)const;

	void PostProcess(bool bUnicode,ARCHIVE_ENTRY_INFO_TREE*);
	HRESULT InspectArchiveStruct(LPCTSTR lpFile,CConfigManager&,CArchiverDLL*,std::vector<ARCHIVE_ENTRY_INFO>&,IArchiveContentUpdateHandler*);
	void CollectUnextractedFiles(LPCTSTR lpOutputDir,const ARCHIVE_ENTRY_INFO_TREE* lpBase,const ARCHIVE_ENTRY_INFO_TREE* lpParent,std::map<const ARCHIVE_ENTRY_INFO_TREE*,std::list<ARCHIVE_ENTRY_INFO_TREE*> > &toExtractList);

public:
	CArchiveFileContent();
	virtual ~CArchiveFileContent();

	//�����ΏۃA�[�J�C�u�����擾
	void SetArchiveFileName(LPCTSTR lpFile){m_pathArcFileName=lpFile;}
	LPCTSTR GetArchiveFileName()const{return m_pathArcFileName;}
	const CArchiverDLL* GetArchiver()const{return m_lpArchiver;}

	ARCHIVE_ENTRY_INFO_TREE* GetRootNode(){return &m_Root;}
	const ARCHIVE_ENTRY_INFO_TREE* GetRootNode()const{return &m_Root;}

	HRESULT ConstructFlat(LPCTSTR lpFile,CConfigManager&,DLL_ID idForce,LPCTSTR lpDenyExt,bool bFilesOnly,CString &strErr,IArchiveContentUpdateHandler*);
	HRESULT ConstructTree(LPCTSTR lpFile,CConfigManager&,DLL_ID idForce,LPCTSTR lpDenyExt,bool bSkipMeaningless,CString &strErr,IArchiveContentUpdateHandler*);
	void Clear();

	bool ReloadArchiverIfLost(CConfigManager &ConfigManager,CString &strErr);

	bool IsExtractEachSupported()const{return m_bExtractEachSupported;}
	bool IsDeleteItemsSupported()const;
	bool IsAddItemsSupported()const;
	bool IsUnicodeCapable()const;
	BOOL CheckArchiveExists()const{return PathFileExists(m_pathArcFileName);}

	//lpTop�ȉ��̃t�@�C��������
	void FindItem(LPCTSTR lpszMask,const ARCHIVE_ENTRY_INFO_TREE *lpTop,std::vector<ARCHIVE_ENTRY_INFO_TREE*> &founds)const;

	HRESULT AddItem(const std::list<CString>&,LPCTSTR lpDestDir,CConfigManager& rConfig,CString&);	//�t�@�C����ǉ����k
	bool ExtractItems(CConfigManager&,const std::list<ARCHIVE_ENTRY_INFO_TREE*> &items,LPCTSTR lpszDir,const ARCHIVE_ENTRY_INFO_TREE* lpBase,bool bCollapseDir,CString &strLog);
	//bOverwrite:true�Ȃ瑶�݂���e���|�����t�@�C�����폜���Ă���𓀂���
	bool MakeSureItemsExtracted(CConfigManager&,LPCTSTR lpOutputDir,const ARCHIVE_ENTRY_INFO_TREE* lpBase,const std::list<ARCHIVE_ENTRY_INFO_TREE*> &items,std::list<CString> &r_filesList,bool bOverwrite,CString &strLog);
	bool DeleteItems(CConfigManager&,const std::list<ARCHIVE_ENTRY_INFO_TREE*>&,CString&);
};
