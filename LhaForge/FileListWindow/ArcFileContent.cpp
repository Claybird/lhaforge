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
#include "ConfigCode/ConfigFile.h"
#include "Utilities/StringUtil.h"
#include "Utilities/FileOperation.h"
#include "Utilities/Utility.h"
#include "Dialogs/ProgressDlg.h"
#include "extract.h"
#include "compress.h"
#include "CommonUtil.h"
#include "ArcFileContent.h"

bool ARCHIVE_FIND_CONDITION::matchItem(const ARCHIVE_ENTRY_INFO& p)const
{
	switch (key) {
	case KEY::filename:
		return UtilPathMatchSpec(p._entryName, patternStr);
	case KEY::fullpath:
		return UtilPathMatchSpec(p._entry.path, patternStr);
	case KEY::originalSize:
		switch (compare) {
		case COMPARE::equal:
			return (st_size == p._entry.stat.st_size);
		case COMPARE::equalOrGreater:
			return (st_size <= p._entry.stat.st_size);
		case COMPARE::equalOrLess:
			return (st_size >= p._entry.stat.st_size);
		}
	case KEY::mdate:
		//by day
	{
		if (p._entry.stat.st_mtime == 0)return false;	//no date provided
		auto ft_gmt = UtilUnixTimeToFileTime(p._entry.stat.st_mtime);
		FILETIME ft_local;
		FileTimeToLocalFileTime(&ft_gmt, &ft_local);
		SYSTEMTIME systime = {};
		FileTimeToSystemTime(&ft_local, &systime);

		switch (compare) {
		case COMPARE::equal:
			return (mdate.wYear == systime.wYear)
				&& (mdate.wMonth == systime.wMonth)
				&& (mdate.wDay == systime.wDay);
		case COMPARE::equalOrGreater:
			return (mdate.wYear < systime.wYear)
				|| (mdate.wYear == systime.wYear && mdate.wMonth < systime.wMonth)
				|| (mdate.wYear == systime.wYear && mdate.wMonth == systime.wMonth && mdate.wDay <= systime.wDay);
		case COMPARE::equalOrLess:
			return (mdate.wYear > systime.wYear)
				|| (mdate.wYear == systime.wYear && mdate.wMonth > systime.wMonth)
				|| (mdate.wYear == systime.wYear && mdate.wMonth == systime.wMonth && mdate.wDay >= systime.wDay);
		}
	}
	case KEY::mode:
		if (st_mode_mask & S_IFDIR) {
			return p.is_directory();
		} else {
			return (p._entry.stat.st_mode & st_mode_mask) != 0;
		}
	default:
		ASSERT(!"This code cannot be run");
		return false;
	}
}

std::wstring ARCHIVE_FIND_CONDITION::toString()const
{
	std::wstring desc;
	switch (key) {
	case KEY::filename:
		if (patternStr==L"*" || patternStr == L"*.*") {
			desc = UtilLoadString(IDS_SEARCH_EVERYTHING);
		} else {
			desc = Format(UtilLoadString(IDS_SEARCH_BY_FILENAME), patternStr.c_str());
		}
		break;
	case KEY::fullpath:
		if (patternStr == L"*" || patternStr == L"*.*") {
			desc = UtilLoadString(IDS_SEARCH_EVERYTHING);
		} else {
			desc = Format(UtilLoadString(IDS_SEARCH_BY_FILEPATH), patternStr.c_str());
		}
		break;
	case KEY::originalSize:
	{
		std::wstring cond;
		auto size = UtilFormatSizeStrict(st_size);
		switch (compare) {
		case COMPARE::equal:
			cond = Format(UtilLoadString(IDS_COND_FILESIZE_EQUAL), size.c_str());
			break;
		case COMPARE::equalOrGreater:
			cond = Format(UtilLoadString(IDS_COND_FILESIZE_EQUAL_OR_GREATER), size.c_str());
			break;
		case COMPARE::equalOrLess:
			cond = Format(UtilLoadString(IDS_COND_FILESIZE_EQUAL_OR_LESS), size.c_str());
			break;
		}
		desc = Format(UtilLoadString(IDS_SEARCH_BY_ORIGINAL_SIZE), cond.c_str());
		break;
	}
	case KEY::mdate:
	{
		std::wstring cond;
		switch (compare) {
		case COMPARE::equal:
			cond = Format(UtilLoadString(IDS_COND_MDATE_EQUAL), mdate.wYear, mdate.wMonth, mdate.wDay);
			break;
		case COMPARE::equalOrGreater:
			cond = Format(UtilLoadString(IDS_COND_MDATE_EQUAL_OR_GREATER), mdate.wYear, mdate.wMonth, mdate.wDay);
			break;
		case COMPARE::equalOrLess:
			cond = Format(UtilLoadString(IDS_COND_MDATE_EQUAL_OR_LESS), mdate.wYear, mdate.wMonth, mdate.wDay);
			break;
		}
		desc = Format(UtilLoadString(IDS_SEARCH_BY_MDATE), cond.c_str());
		break;
	}
	case KEY::mode:
	{
		std::wstring cond;
		if (st_mode_mask & S_IFDIR) {
			cond = UtilLoadString(IDS_COND_FOLDER);
		} else {
			cond = UtilLoadString(IDS_COND_FILE);
		}
		desc = Format(UtilLoadString(IDS_SEARCH_BY_MODE), cond.c_str());
		break;
	}
	}
	return desc;
}


//-----
void CArchiveFileContent::scanArchiveStruct(
	const std::filesystem::path& archiveName,
	ILFScanProgressHandler& progressHandler)
{
	clear();
	m_pRoot = std::make_shared<ARCHIVE_ENTRY_INFO>();

	progressHandler.setArchive(archiveName);
	CLFArchive arc;
	arc.read_open(archiveName, m_passphrase);
	m_numFiles = 0;

	bool bEncrypted = false;
	for (auto* entry = arc.read_entry_begin(); entry; entry = arc.read_entry_next()) {
		m_numFiles++;
		auto pathname = UtilPathRemoveLastSeparator(LF_sanitize_pathname(entry->path));
		auto elements = UtilSplitString(pathname, L"/");

		if (elements.empty() || elements[0].empty())continue;

		auto &item = m_pRoot->addEntry(elements);
		item._entry = *entry;
		item._entryName = elements.back();
		item._originalSize = entry->stat.st_size;

		bEncrypted = bEncrypted || entry->is_encrypted;

		//notifier
		progressHandler.onNextEntry(entry->path);
	}
	m_bEncrypted = bEncrypted;
	m_bModifySupported = (
		!(GetFileAttributesW(archiveName.c_str()) & FILE_ATTRIBUTE_READONLY)) &&
		arc.is_modify_supported() &&
		toLower(archiveName.extension()) != L".exe";	//Self Extracting Archive
	m_pathArchive = archiveName;
	postScanArchive(nullptr);
}

void CArchiveFileContent::postScanArchive(ARCHIVE_ENTRY_INFO* pNode)
{
	if (!pNode)pNode = m_pRoot.get();

	if (pNode->is_directory()) {
		pNode->_originalSize = 0;
		//children
		for (auto& child : pNode->_children) {
			postScanArchive(child.get());

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
}

std::vector<std::shared_ptr<ARCHIVE_ENTRY_INFO> > CArchiveFileContent::findItem(
	const ARCHIVE_FIND_CONDITION& condition,
	const ARCHIVE_ENTRY_INFO* parent)const 
{
	if (!parent)parent = m_pRoot.get();
	//---breadth first search
	std::vector<std::shared_ptr<ARCHIVE_ENTRY_INFO> > found;
	for (auto& child : parent->_children) {
		if (condition.matchItem(*(child.get()))) {
			found.push_back(child);
		}
	}
	for (auto& child : parent->_children) {
		if (child->is_directory()) {
			auto subFound = findItem(condition, child.get());
			found.insert(found.end(), subFound.begin(), subFound.end());
		}
	}
	return found;
}


//extracts one entry; for directories, caller should expand and add children to items
std::vector<std::filesystem::path> CArchiveFileContent::extractEntries(
	const std::vector<const ARCHIVE_ENTRY_INFO*> &entries,
	const std::filesystem::path &outputDir,
	const ARCHIVE_ENTRY_INFO* lpBase,
	ILFProgressHandler& progressHandler,
	ARCLOG &arcLog)
{
	std::vector<std::filesystem::path> extracted;
	CLFArchive arc;
	progressHandler.setArchive(m_pathArchive);
	progressHandler.setNumEntries(entries.size());
	arc.read_open(m_pathArchive, m_passphrase);

	std::unordered_map<std::wstring, const ARCHIVE_ENTRY_INFO*> unextracted;
	for (const auto &item : entries) {
		unextracted[item->calcFullpath()] = item;
	}

	CLFOverwriteConfirmFORCED preExtractHandler(overwrite_options::overwrite);

	for (auto entry = arc.read_entry_begin(); entry && !unextracted.empty(); entry = arc.read_entry_next()) {
		auto pathname = UtilPathRemoveLastSeparator(LF_sanitize_pathname(entry->path));
		auto iter = unextracted.find(pathname);
		if (iter != unextracted.end()) {
			auto out = extractCurrentEntry(arc, entry, outputDir, arcLog, preExtractHandler, progressHandler);
			extracted.push_back(out);
			unextracted.erase(iter);
		}
	}
	arc.close();
	return extracted;
}


std::tuple<std::filesystem::path,	//output file name
	std::unique_ptr<ILFArchiveFile>,	//output file handle
	std::vector<std::filesystem::path>>	//files not removed
CArchiveFileContent::subDeleteEntries(
	const LF_COMPRESS_ARGS& args,
	const std::vector<std::pair<std::filesystem::path/*path in archive*/, std::filesystem::path/*path on disk*/>> &items_to_delete,
	ILFProgressHandler& progressHandler,
	ILFOverwriteInArchiveConfirm& confirmHandler,
	ARCLOG &arcLog)
{
	//check for single-file-compressor
	if (!isMultipleContentAllowed()) {
		throw LF_EXCEPTION(L"This format cannot contain more than one file");
	}

	CLFArchive src;
	src.read_open(m_pathArchive, m_passphrase);

	std::vector<std::filesystem::path> not_removed;

	auto judger = [&](const LF_ENTRY_STAT& entry) {
		progressHandler.onNextEntry(entry.path, entry.stat.st_size);
		progressHandler.onEntryIO(0);	//TODO
		auto subject = std::filesystem::path(entry.path);

		for (const auto& item : items_to_delete) {
			if (subject == item.first || UtilPathIsInSubDirectory(subject, item.first)) {
				//check overwrite, or just delete. depends on confirmHandler
				auto decision = confirmHandler(item.second, entry);
				switch (decision) {
				case overwrite_options::overwrite:
					arcLog(entry.path, UtilLoadString(IDS_ARCLOG_REMOVED));
					return false;
				case overwrite_options::skip:
					not_removed.push_back(subject);
					arcLog(entry.path, UtilLoadString(IDS_ARCLOG_KEEP));
					return true;
				case overwrite_options::abort:
				default:
					arcLog(entry.path, UtilLoadString(IDS_ARCLOG_ABORT));
					CANCEL_EXCEPTION();
				}
				return false;
			}
		}
		return true;
	};

	auto tempFile = UtilGetTemporaryFileName();
	auto dest = src.make_copy_archive(tempFile, args, judger);
	return { tempFile, std::move(dest), not_removed};
}

bool CArchiveFileContent::isMultipleContentAllowed()const
{
	//check for single-file-compressor
	CLFArchive arc;
	arc.read_open(m_pathArchive, std::make_shared<CLFPassphraseNULL>());
	auto caps = CLFArchive::get_compression_capability(arc.get_format());
	if (caps.contains_multiple_files) {
		return true;
	}
	return false;
}

void CArchiveFileContent::addEntries(
	const LF_COMPRESS_ARGS& args,
	const std::vector<std::filesystem::path> &files,
	const ARCHIVE_ENTRY_INFO* lpParent,
	ILFProgressHandler& progressHandler,
	ILFOverwriteInArchiveConfirm& confirmHandler,
	ARCLOG &arcLog)
{
	//check for single-file-compressor
	if (!isMultipleContentAllowed()) {
		throw LF_EXCEPTION(L"This format cannot contain more than one file");
	}

	//---
	// check for existing file
	// ask user to remove or keep existing files
	// then add new files
	//---
	std::filesystem::path destDir;
	if(lpParent)destDir = lpParent->calcFullpath();

	auto get_path_in_archive = [&](const std::filesystem::path& file) {
		return destDir / file.filename();
	};

	std::vector<std::pair<std::filesystem::path, std::filesystem::path>> items_to_delete;
	for (const auto &file : files) {
		auto entryPath = get_path_in_archive(file);
		items_to_delete.push_back({ entryPath, file });
	}

	arcLog.setArchivePath(m_pathArchive);
	progressHandler.setArchive(m_pathArchive);
	progressHandler.setNumEntries(files.size() + m_numFiles);

	//read from source
	auto [tempFile,dest,not_removed] = subDeleteEntries(args, items_to_delete, progressHandler, confirmHandler, arcLog);

	//add
	for (const auto &file : files) {
		//keep existing files if user wants to
		if(isIn(not_removed, get_path_in_archive(file)))continue;

		try {
			LF_ENTRY_STAT entry;
			auto entryPath = destDir / std::filesystem::path(file).filename();
			entry.read_stat(file, entryPath);
			progressHandler.onNextEntry(entry.path, entry.stat.st_size);

			if (std::filesystem::is_regular_file(file)) {
				RAW_FILE_READER provider;
				provider.open(file);
				uint64_t size = 0;
				dest->add_file_entry(entry, [&]() {
					auto data = provider();
					if (data.offset) {
						size = data.offset->offset + data.size;
					} else {
						size += data.size;
					}
					progressHandler.onEntryIO(size);
					return data;
				});
				progressHandler.onEntryIO(entry.stat.st_size);
			} else {
				//directory
				dest->add_directory_entry(entry);
				progressHandler.onEntryIO(entry.stat.st_size);
			}
			arcLog(file, UtilLoadString(IDS_ARCLOG_OK));
		} catch (const LF_USER_CANCEL_EXCEPTION& e) {	//need this to know that user cancel
			arcLog(file, e.what());
			UtilDeletePath(m_pathArchive);
			throw;
		} catch (const LF_EXCEPTION& e) {
			arcLog(file, e.what());
			UtilDeletePath(m_pathArchive);
			throw;
		} catch (const std::filesystem::filesystem_error& e) {
			auto msg = UtilUTF8toUNICODE(e.what(), strlen(e.what()));
			arcLog(file, msg);
			UtilDeletePath(m_pathArchive);
			throw LF_EXCEPTION(msg);
		}
	}
	dest->close();
	UtilDeletePath(m_pathArchive);
	std::filesystem::rename(tempFile, m_pathArchive);
}


void CArchiveFileContent::deleteEntries(
	const LF_COMPRESS_ARGS& args,
	const std::vector<const ARCHIVE_ENTRY_INFO*> &items,
	ILFProgressHandler& progressHandler,
	ARCLOG &arcLog)
{
	/*
	* To delete items from archive,
	* skip items while making a copy of existing archive
	*/
	std::vector<std::pair<std::filesystem::path, std::filesystem::path>> items_to_delete;
	for (const auto &item : items) {
		items_to_delete.push_back({
			std::filesystem::path(item->calcFullpath()).lexically_normal(),
			L""
		});
	}
	arcLog.setArchivePath(m_pathArchive);
	progressHandler.setArchive(m_pathArchive);
	progressHandler.setNumEntries(m_numFiles);

	//read from source
	auto[tempFile, dest, not_removed] = subDeleteEntries(
		args,
		items_to_delete,
		progressHandler,
		CLFOverwriteInArchiveConfirmFORCED(overwrite_options::overwrite),
		arcLog);
	dest->close();

	UtilDeletePath(m_pathArchive);
	std::filesystem::rename(tempFile, m_pathArchive);
}


std::vector<std::filesystem::path>
CArchiveFileContent::makeSureItemsExtracted(	//returns list of extracted files
	const std::vector<const ARCHIVE_ENTRY_INFO*> &items,
	const std::filesystem::path &outputDir,
	const ARCHIVE_ENTRY_INFO* lpBase,
	ILFProgressHandler& progressHandler,
	enum class overwrite_options options,
	ARCLOG &arcLog)
{
	std::vector<const ARCHIVE_ENTRY_INFO*> toExtract;

	for(auto &item: items){
		std::filesystem::path path = outputDir;

		auto subPath = item->getRelativePath(lpBase);
		path /= subPath;

		auto children = item->enumChildren();
		for (auto c : children) {
			if (!c->_entry.path.empty()) {
				toExtract.push_back(c);
			}
		}
	}
	arcLog.setArchivePath(m_pathArchive);
	if (toExtract.empty()) {
		return {};
	}else{
		try {
			arcLog.setArchivePath(m_pathArchive);
			auto extractedFiles = extractEntries(toExtract, outputDir, lpBase, progressHandler, arcLog);
			return extractedFiles;
		} catch (const LF_EXCEPTION& e) {
			arcLog.logException(e);
			throw;
		}
	}
}


