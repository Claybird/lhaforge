/*
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

#include "stdafx.h"
#include "ConfigCode/ConfigManager.h"
#include "Utilities/StringUtil.h"
#include "Utilities/FileOperation.h"
#include "Utilities/Utility.h"
#include "Dialogs/ProgressDlg.h"
#include "extract.h"
#include "compress.h"
#include "CommonUtil.h"
#include "ArcFileContent.h"


void CArchiveFileContent::inspectArchiveStruct(
	const std::wstring& archiveName,
	IArchiveContentUpdateHandler* lpHandler)
{
	clear();
	m_pRoot = std::make_shared<ARCHIVE_ENTRY_INFO>();

	ARCHIVE_FILE_TO_READ arc;
	arc.read_open(archiveName, LF_passphrase_input);

	bool bEncrypted = false;
	for (LF_ARCHIVE_ENTRY* entry = arc.begin(); entry; entry = arc.next()) {
		auto pathname = UtilPathRemoveLastSeparator(LF_sanitize_pathname(entry->get_pathname()));
		auto elements = UtilSplitString(pathname, L"/");

		if (elements.empty() || elements[0].empty())continue;

		auto &item = m_pRoot->addEntry(elements);
		item._entryName = elements.back();
		item._fullpath = pathname;
		item._nAttribute = entry->get_file_mode();
		item._originalSize = entry->get_original_filesize();
		item._st_mtime = entry->get_mtime();

		bEncrypted = bEncrypted || entry->is_encrypted();

		//notifier
		if (lpHandler) {
			while (UtilDoMessageLoop())continue;
			lpHandler->onUpdated(item);
			if (lpHandler->isAborted()) {
				CANCEL_EXCEPTION();
			}
		}
	}
	m_bEncrypted = bEncrypted;
	m_bReadOnly = GetFileAttributesW(archiveName.c_str()) & FILE_ATTRIBUTE_READONLY;
	m_pathArchive = archiveName;
	postInspectArchive(nullptr);
}


void CArchiveFileContent::postInspectArchive(ARCHIVE_ENTRY_INFO* pNode)
{
	if (!pNode)pNode = m_pRoot.get();

	if (pNode != m_pRoot.get()) {
		//in case of directories that does not have a fullpath
		pNode->_fullpath = pNode->getFullpath();
	}

	if (pNode->isDirectory()) {
		pNode->_originalSize = 0;
	}

	//children
	for (auto& child : pNode->_children) {
		postInspectArchive(child.get());

		if (pNode->_originalSize >= 0) {
			if (child->_originalSize >= 0) {
				pNode->_originalSize += child->_originalSize;
			} else {
				//file size unknown
				pNode->_originalSize = -1;
			}
		}
	}
}


std::vector<std::shared_ptr<ARCHIVE_ENTRY_INFO> > CArchiveFileContent::findSubItem(
	const std::wstring& pattern,
	const ARCHIVE_ENTRY_INFO* parent)const
{
	//---breadth first search

	std::vector<std::shared_ptr<ARCHIVE_ENTRY_INFO> > found;
	for (auto& child : parent->_children) {
		if (UtilPathMatchSpec(child->_entryName, pattern)) {
			found.push_back(child);
		}
	}
	for (auto& child : parent->_children) {
		if (child->isDirectory()) {
			auto subFound = findSubItem(pattern, child.get());
			found.insert(found.end(), subFound.begin(), subFound.end());
		}
	}
	return found;
}

//extracts one entry; for directories, caller should expand and add children to items
void CArchiveFileContent::extractItems(
	CConfigManager &Config,
	const std::vector<ARCHIVE_ENTRY_INFO*> &items,
	const std::wstring& outputDir,
	const ARCHIVE_ENTRY_INFO* lpBase,
	bool bCollapseDir,
	ARCLOG &arcLog)
{
	ARCHIVE_FILE_TO_READ arc;
	//progress bar
	CProgressDialog dlg;
	dlg.Create(nullptr);
	dlg.ShowWindow(SW_SHOW);

	arc.read_open(m_pathArchive, LF_passphrase_input);

	std::unordered_map<std::wstring, ARCHIVE_ENTRY_INFO*> unextracted;
	for (const auto &item : items) {
		unextracted[item->getFullpath()] = item;
	}

	std::function<overwrite_options(const std::wstring&, const LF_ARCHIVE_ENTRY*)> preExtractHandler =
		[&](const std::wstring& fullpath, const LF_ARCHIVE_ENTRY* entry)->overwrite_options {
			return overwrite_options::overwrite;
	};
	std::function<void(const std::wstring&, UINT64, UINT64)> progressHandler =
		[&](const std::wstring& originalPath, UINT64 currentSize, UINT64 totalSize)->void {
		dlg.SetProgress(
			m_pathArchive,
			1,
			1,
			originalPath,
			currentSize,
			totalSize);
		while (UtilDoMessageLoop())continue;
		if (dlg.isAborted()) {
			dlg.DestroyWindow();
			CANCEL_EXCEPTION();
		}
	};

	for (LF_ARCHIVE_ENTRY* entry = arc.begin(); entry; entry = arc.next()) {
		auto pathname = UtilPathRemoveLastSeparator(LF_sanitize_pathname(entry->get_pathname()));
		auto iter = unextracted.find(pathname);
		if (iter != unextracted.end()) {
			auto defaultDecision = overwrite_options::overwrite_all;
			if (bCollapseDir) {
				entry->_pathname = std::filesystem::path(entry->get_pathname()).filename();
			}
			extractOneEntry(arc, entry, outputDir, defaultDecision, arcLog, preExtractHandler, progressHandler);
			unextracted.erase(iter);
		}
	}
	arc.close();
	dlg.DestroyWindow();
}

void CArchiveFileContent::collectUnextractedFiles(const std::wstring& outputDir,const ARCHIVE_ENTRY_INFO* lpBase,const ARCHIVE_ENTRY_INFO* lpParent,std::map<const ARCHIVE_ENTRY_INFO*,std::vector<ARCHIVE_ENTRY_INFO*> > &toExtractList)
{
	size_t numChildren=lpParent->getNumChildren();
	for(size_t i=0;i<numChildren;i++){
		ARCHIVE_ENTRY_INFO* lpNode=lpParent->getChild(i);
		std::filesystem::path path=outputDir;

		auto subPath = lpNode->getRelativePath(lpBase);
		path /= subPath;

		if(std::filesystem::is_directory(path)){
			// フォルダが存在するが中身はそろっているか?
			collectUnextractedFiles(outputDir,lpBase,lpNode,toExtractList);
		}else if(!std::filesystem::is_regular_file(path)){
			// キャッシュが存在しないので、解凍要請リストに加える
			toExtractList[lpParent].push_back(lpNode);
			if (lpNode->isDirectory()) {
				//is a directory, but does not exist
				collectUnextractedFiles(outputDir, lpBase, lpNode, toExtractList);
			}
		}
	}
}


//bOverwrite:trueなら存在するテンポラリファイルを削除してから解凍する
bool CArchiveFileContent::MakeSureItemsExtracted(
	CConfigManager& Config,
	const std::wstring &outputDir,
	bool bOverwrite,
	const ARCHIVE_ENTRY_INFO* lpBase,
	const std::vector<ARCHIVE_ENTRY_INFO*> &items,
	std::vector<std::wstring> &r_extractedFiles,
	std::wstring &strLog)
{
	//選択されたアイテムを列挙
	std::map<const ARCHIVE_ENTRY_INFO*,std::vector<ARCHIVE_ENTRY_INFO*> > toExtractList;

	for(auto &lpNode: items){
		// 存在をチェックし、もし解凍済みであればそれを開く
		std::filesystem::path path = outputDir;

		auto subPath = lpNode->getRelativePath(lpBase);
		path /= subPath;

		if(bOverwrite){
			// 上書き解凍するので、存在するファイルは削除
			if(lpNode->isDirectory()){
				if (std::filesystem::is_directory(path))UtilDeleteDir(path, true);
				collectUnextractedFiles(outputDir, lpBase, lpNode, toExtractList);
			}else{
				if (std::filesystem::is_regular_file(path))UtilDeletePath(path);
			}
			//解凍要請リストに加える
			toExtractList[lpBase].push_back(lpNode);
		}else{	//上書きはしない
			if(std::filesystem::is_directory(path)){
				// フォルダが存在するが中身はそろっているか?
				collectUnextractedFiles(outputDir,lpBase,lpNode,toExtractList);
			}else if(!std::filesystem::is_regular_file(path)){
				// キャッシュが存在しないので、解凍要請リストに加える
				toExtractList[lpBase].push_back(lpNode);
			}
		}

		//extracted files
		r_extractedFiles.push_back(path.make_preferred());
	}
	if(toExtractList.empty()){
		return true;
	}

	//未解凍の物のみ一時フォルダに解凍
	for(const auto &pair: toExtractList){
		const std::vector<ARCHIVE_ENTRY_INFO*> &filesList = pair.second;
		ARCLOG arcLog;
		arcLog.setArchivePath(m_pathArchive);
		try {
			extractItems(Config, filesList, outputDir, lpBase, false, arcLog);
		} catch (const LF_EXCEPTION& e) {
			arcLog.logException(e);
			for(const auto &toDelete : filesList){
				//失敗したので削除
				std::filesystem::path path=outputDir;
				auto subPath = toDelete->getRelativePath(lpBase);
				path /= subPath;
				UtilDeletePath(path);
			}
			return false;
		}
	}
	return true;
}


HRESULT CArchiveFileContent::AddItem(const std::vector<std::wstring> &fileList,LPCTSTR lpDestDir,CConfigManager& rConfig,CString &strLog)
{
	for (const auto &file : fileList) {
		if (0 == _wcsicmp(file.c_str(), m_pathArchive.c_str())) {
			//tried to compress archive itself
			//TODO:this could be acceptable
			return E_LF_SAME_INPUT_AND_OUTPUT;
		}
	}

	std::unordered_set<std::wstring> fileList_lower_case;
	for (const auto &file : fileList) {
		auto entryPath = lpDestDir / std::filesystem::path(file).filename();
		fileList_lower_case.insert(toLower(entryPath));
	}

	ARCLOG arcLog;	//TODO

	//read from source, write to new
	//this would need overhead of extract on read and compress on write
	//there seems no way to get raw data

	auto tempFile = UtilGetTemporaryFileName();
	ARCHIVE_FILE_TO_WRITE dest;
	copyArchive(
		rConfig,
		tempFile,
		dest,
		m_pathArchive,
		[&](LF_ARCHIVE_ENTRY* entry) {
			std::wstring path = entry->get_pathname();
			if (isIn(fileList_lower_case, toLower(path))) {
				return false;
			} else {
				return true;
			}
		});

	//TODO: move to sub-function
	for (const auto &file : fileList) {
		try {
			LF_ARCHIVE_ENTRY entry;
			auto entryPath = lpDestDir / std::filesystem::path(file).filename();
			entry.copy_file_stat(file, entryPath);
			//progressHandler(tempFile, file, processed, total_filesize);

			if (std::filesystem::is_regular_file(file)) {
				RAW_FILE_READER provider;
				provider.open(file);
				dest.add_entry(entry, [&]() {
					auto data = provider();
					/*if (!data.is_eof()) {
						progressHandler(
							output_archive,
							source.originalFullPath,
							processed + data.offset,
							source_files.total_filesize);
					}*/
					while (UtilDoMessageLoop())continue;
					return data;
				});

				/*processed += std::filesystem::file_size(source.originalFullPath);
				progressHandler(
					output_archive,
					source.originalFullPath,
					processed,
					source_files.total_filesize);*/
			} else {
				//directory
				dest.add_directory(entry);
			}
			arcLog(tempFile, L"OK");
		} catch (const LF_USER_CANCEL_EXCEPTION& e) {	//need this to know that user cancel
			arcLog(tempFile, e.what());
			dest.close();
			UtilDeletePath(m_pathArchive);
			//throw e;
			return E_ABORT;	//TODO
		} catch (const LF_EXCEPTION& e) {
			arcLog(tempFile, e.what());
			dest.close();
			UtilDeletePath(m_pathArchive);
			//throw e;
			return E_FAIL;	//TODO
		} catch (const std::filesystem::filesystem_error& e) {
			auto msg = UtilUTF8toUNICODE(e.what(), strlen(e.what()));
			arcLog(tempFile, msg);
			dest.close();
			UtilDeletePath(m_pathArchive);
			//throw LF_EXCEPTION(msg);
			return E_FAIL;	//TODO
		}
	}
	dest.close();
	UtilDeletePath(m_pathArchive);
	std::filesystem::rename(tempFile, m_pathArchive);

	return S_OK;
}

bool CArchiveFileContent::DeleteItems(CConfigManager &Config,const std::list<ARCHIVE_ENTRY_INFO*> &fileList,CString &strLog)
{
	//TODO
	RAISE_EXCEPTION(L"NOT INMPELEMTED");
	return false;
/*	//削除対象を列挙
	std::list<CString> filesToDel;
	for(std::list<ARCHIVE_ENTRY_INFO*>::const_iterator ite=fileList.begin();ite!=fileList.end();++ite){
		(*ite)->EnumFiles(filesToDel);
	}
	return m_lpArchiver->DeleteItemFromArchive(m_pathArchive,Config,filesToDel,strLog);*/
}

