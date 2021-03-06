﻿/*
* MIT License

* Copyright (c) 2005- Claybird

* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:

* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.

* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
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
 * アーカイブ内のファイル構造を保持
 */
class CArchiverDLL;
enum DLL_ID;
class CArchiveFileContent{
protected:
	//ファイル情報
	ARCHIVE_ENTRY_INFO_TREE m_Root;

	//Semi-Auto Garbage Collector
	CSmartPtrCollection<ARCHIVE_ENTRY_INFO> m_GC;

	CArchiverDLL*	m_lpArchiver;
	CString			m_pathArcFileName;
	bool			m_bExtractEachSupported;
	bool			m_bReadOnly;

	bool			m_bEncrypted;	//少なくとも一つのファイルが暗号化されているならtrue

protected:
	//---internal functions
	ARCHIVE_ENTRY_INFO_TREE* ForceFindEntry(ARCHIVE_ENTRY_INFO_TREE* lpParent,LPCTSTR lpName);

	//bMatchPath:trueならパスも含め検索、falseならファイル名のみ検索
	void FindSubItem(LPCTSTR lpszMask,bool bMatchPath,const ARCHIVE_ENTRY_INFO_TREE *lpTop,std::vector<ARCHIVE_ENTRY_INFO_TREE*> &founds)const;

	void PostProcess(bool bUnicode,ARCHIVE_ENTRY_INFO_TREE*);
	HRESULT InspectArchiveStruct(LPCTSTR lpFile,CConfigManager&,CArchiverDLL*,std::vector<ARCHIVE_ENTRY_INFO>&,IArchiveContentUpdateHandler*);
	void CollectUnextractedFiles(LPCTSTR lpOutputDir,const ARCHIVE_ENTRY_INFO_TREE* lpBase,const ARCHIVE_ENTRY_INFO_TREE* lpParent,std::map<const ARCHIVE_ENTRY_INFO_TREE*,std::list<ARCHIVE_ENTRY_INFO_TREE*> > &toExtractList);

public:
	CArchiveFileContent();
	virtual ~CArchiveFileContent();

	//処理対象アーカイブ名を取得
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
	bool IsArchiveEncrypted()const{return m_bEncrypted;}
	BOOL CheckArchiveExists()const{return PathFileExists(m_pathArcFileName);}

	//lpTop以下のファイルを検索
	void FindItem(LPCTSTR lpszMask,const ARCHIVE_ENTRY_INFO_TREE *lpTop,std::vector<ARCHIVE_ENTRY_INFO_TREE*> &founds)const;

	HRESULT AddItem(const std::list<CString>&,LPCTSTR lpDestDir,CConfigManager& rConfig,CString&);	//ファイルを追加圧縮
	bool ExtractItems(CConfigManager&,const std::list<ARCHIVE_ENTRY_INFO_TREE*> &items,LPCTSTR lpszDir,const ARCHIVE_ENTRY_INFO_TREE* lpBase,bool bCollapseDir,CString &strLog);
	//bOverwrite:trueなら存在するテンポラリファイルを削除してから解凍する
	bool MakeSureItemsExtracted(CConfigManager&,LPCTSTR lpOutputDir,const ARCHIVE_ENTRY_INFO_TREE* lpBase,const std::list<ARCHIVE_ENTRY_INFO_TREE*> &items,std::list<CString> &r_filesList,bool bOverwrite,CString &strLog);
	bool DeleteItems(CConfigManager&,const std::list<ARCHIVE_ENTRY_INFO_TREE*>&,CString&);
};
