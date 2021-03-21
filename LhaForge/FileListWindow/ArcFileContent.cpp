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
	m_bModifySupported = (!(GetFileAttributesW(archiveName.c_str()) & FILE_ATTRIBUTE_READONLY)) && arc.is_modify_supported();
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


#ifdef UNIT_TEST

TEST(ArcFileContent, scanArchiveStruct)
{
	_wsetlocale(LC_ALL, L"");	//default locale
	CLFPassphraseNULL no_passphrase;
	CArchiveFileContent content(no_passphrase);

	auto arcpath = std::filesystem::path(__FILEW__).parent_path() / L"test/test_content.zip";
	content.scanArchiveStruct(arcpath, CLFScanProgressHandlerNULL());
	EXPECT_TRUE(content.isOK());
	EXPECT_EQ(arcpath, content.getArchivePath());

	const auto* root = content.getRootNode();
	EXPECT_EQ(3, root->getNumChildren());
	EXPECT_EQ(L"dirA", root->getChild(0)->_entryName);
	EXPECT_EQ(L"dirA", root->getChild(L"dirA")->_entryName);
	EXPECT_TRUE(root->getChild(L"dirA")->_entry.path.empty());
	EXPECT_EQ(L"dirB", root->getChild(L"dirA")->getChild(L"dirB")->_entryName);
	EXPECT_EQ(L"dirA/dirB/", root->getChild(L"dirA")->getChild(L"dirB")->_entry.path);
	EXPECT_EQ(8, root->enumChildren().size());
	EXPECT_EQ(L"file3.txt", root->getChild(L"かきくけこ")->getChild(0)->_entryName);
	EXPECT_EQ(L"かきくけこ/file3.txt", root->getChild(L"かきくけこ")->getChild(0)->_entry.path);
	EXPECT_EQ(L"あいうえお.txt", root->getChild(L"あいうえお.txt")->_entryName);
	EXPECT_EQ(L"あいうえお.txt", root->getChild(L"あいうえお.txt")->_entry.path);

	content.clear();
	EXPECT_FALSE(content.isOK());
}
#endif


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
		if (child->is_directory()) {
			auto subFound = findSubItem(pattern, child.get());
			found.insert(found.end(), subFound.begin(), subFound.end());
		}
	}
	return found;
}

#ifdef UNIT_TEST
TEST(ArcFileContent, findItem)
{
	_wsetlocale(LC_ALL, L"");	//default locale
	CLFPassphraseNULL no_passphrase;
	CArchiveFileContent content(no_passphrase);

	content.scanArchiveStruct(std::filesystem::path(__FILEW__).parent_path() / L"test/test_content.zip", CLFScanProgressHandlerNULL());
	EXPECT_TRUE(content.isOK());
	auto result = content.findItem(L"*");
	EXPECT_EQ(content.getRootNode()->enumChildren().size(), result.size());
	result = content.findItem(L"*.*");
	EXPECT_EQ(content.getRootNode()->enumChildren().size(), result.size());
	result = content.findItem(L".txt");
	EXPECT_EQ(4, result.size());
	result = content.findItem(L"*.txt");
	EXPECT_EQ(4, result.size());
	result = content.findItem(L"*.TXT");
	EXPECT_EQ(4, result.size());
	result = content.findItem(L"dirB");
	EXPECT_EQ(1, result.size());
}
#endif

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

#ifdef UNIT_TEST
TEST(ArcFileContent, extractEntries)
{
	_wsetlocale(LC_ALL, L"");	//default locale
	auto tempDir = UtilGetTempPath() / L"arcfilecontent_extractEntries";
	tempDir.make_preferred();
	{
		CLFPassphraseNULL no_passphrase;
		CArchiveFileContent content(no_passphrase);
		ARCLOG arcLog;
		std::vector<const ARCHIVE_ENTRY_INFO*> entriesSub;

		EXPECT_FALSE(content.checkArchiveExists());

		content.scanArchiveStruct(LF_PROJECT_DIR() / L"test/test_extract.zip", CLFScanProgressHandlerNULL());
		EXPECT_EQ(8, content.getRootNode()->enumChildren().size());
		auto entries = content.findItem(L"*");
		EXPECT_EQ(8, entries.size());
		entriesSub.clear();
		for (auto entry : entries) {
			entriesSub.push_back(entry.get());
		}
		EXPECT_EQ(entries.size(), entriesSub.size());
		auto extracted = content.extractEntries(entriesSub, tempDir, content.getRootNode(), CLFProgressHandlerNULL(), arcLog);
		EXPECT_EQ(6, extracted.size());

		for (auto f : extracted) {
			EXPECT_TRUE(std::filesystem::exists(f));
			//extracted file should be in tempDir
			EXPECT_NE(std::wstring::npos, f.make_preferred().wstring().find(tempDir));
		}

		const std::vector<std::filesystem::path> expectedPath = {
			tempDir / L"あいうえお.txt",
			//tempDir / L"dirA",	implicit entry
			tempDir / L"dirA/dirB/",
			tempDir / L"dirA/dirB/file2.txt",
			tempDir / L"dirA/dirB/dirC/",
			tempDir / L"dirA/dirB/dirC/file1.txt",
			//tempDir / L"かきくけこ",	implicit entry
			tempDir / L"かきくけこ/file3.txt",
		};
		//all entries are returned as extracted
		for (const auto p : expectedPath) {
			EXPECT_TRUE(isIn(extracted, p));
		}
	}
	UtilDeleteDir(tempDir, true);
	EXPECT_FALSE(std::filesystem::exists(tempDir));

	{
		CLFPassphraseConst passphrase(L"abcde");
		CArchiveFileContent content(passphrase);
		ARCLOG arcLog;
		std::vector<const ARCHIVE_ENTRY_INFO*> entriesSub;

		EXPECT_FALSE(content.checkArchiveExists());

		content.scanArchiveStruct(LF_PROJECT_DIR() / L"test/test_password_abcde.zip", CLFScanProgressHandlerNULL());
		EXPECT_EQ(1, content.getRootNode()->enumChildren().size());
		auto entries = content.findItem(L"*.txt");
		EXPECT_EQ(1, entries.size());
		entriesSub.clear();
		for (auto entry : entries) {
			entriesSub.push_back(entry.get());
		}
		EXPECT_EQ(entries.size(), entriesSub.size());
		auto extracted = content.extractEntries(entriesSub, tempDir, content.getRootNode(), CLFProgressHandlerNULL(), arcLog);
		EXPECT_EQ(entries.size(), extracted.size());

		for (auto f : extracted) {
			EXPECT_TRUE(std::filesystem::exists(f));
			//extracted file should be in tempDir
			EXPECT_NE(std::wstring::npos, f.make_preferred().wstring().find(tempDir));
		}

		const std::vector<std::filesystem::path> expectedPath = {
			tempDir / L"test.txt",
		};
		//all entries are returned as extracted, even if the directory is not stored explicitly
		for (const auto p : expectedPath) {
			EXPECT_TRUE(isIn(extracted, p));
		}
	}
	UtilDeleteDir(tempDir, true);
	EXPECT_FALSE(std::filesystem::exists(tempDir));
}
#endif

std::tuple<std::filesystem::path, std::unique_ptr<ILFArchiveFile>>
CArchiveFileContent::subDeleteEntries(
	LF_COMPRESS_ARGS& args,
	const std::unordered_set<std::wstring> &items_to_delete,
	ILFProgressHandler& progressHandler,
	ARCLOG &arcLog)
{
	CLFArchive src;
	src.read_open(m_pathArchive, m_passphrase);

	auto tempFile = UtilGetTemporaryFileName();
	auto dest = src.make_copy_archive(tempFile, args,
		[&](const LF_ENTRY_STAT& entry) {
		progressHandler.onNextEntry(entry.path, entry.stat.st_size);
		progressHandler.onEntryIO(0);	//TODO
		if (isIn(items_to_delete, toLower(entry.path))) {
			arcLog(entry.path, L"Removed");
			return false;
		} else {
			return true;
		}
	});
	return { tempFile, std::move(dest) };
}

void CArchiveFileContent::addEntries(
	LF_COMPRESS_ARGS& args,
	const std::vector<std::filesystem::path> &files,
	const ARCHIVE_ENTRY_INFO* lpParent,
	ILFProgressHandler& progressHandler,
	ARCLOG &arcLog)
{
	std::filesystem::path destDir;
	if(lpParent)destDir = lpParent->calcFullpath();
	std::unordered_set<std::wstring> items_to_delete;
	for (const auto &file : files) {
		auto entryPath = destDir / file.filename();
		items_to_delete.insert(toLower(entryPath));
	}
	arcLog.setArchivePath(m_pathArchive);
	progressHandler.setArchive(m_pathArchive);
	progressHandler.setNumEntries(files.size() + m_numFiles);

	//read from source
	auto [tempFile,dest] = subDeleteEntries(args, items_to_delete, progressHandler, arcLog);

	//add
	for (const auto &file : files) {
		try {
			LF_ENTRY_STAT entry;
			auto entryPath = destDir / std::filesystem::path(file).filename();
			entry.read_file_stat(file, entryPath);
			progressHandler.onNextEntry(entry.path, entry.stat.st_size);

			if (std::filesystem::is_regular_file(file)) {
				RAW_FILE_READER provider;
				provider.open(file);
				dest->add_file_entry(entry, [&]() {
					auto data = provider();
					progressHandler.onEntryIO(data.offset);
					return data;
				});
				progressHandler.onEntryIO(entry.stat.st_size);
			} else {
				//directory
				dest->add_directory_entry(entry);
				progressHandler.onEntryIO(entry.stat.st_size);
			}
			arcLog(file, L"OK");
		} catch (const LF_USER_CANCEL_EXCEPTION& e) {	//need this to know that user cancel
			arcLog(file, e.what());
			UtilDeletePath(m_pathArchive);
			throw e;
		} catch (const LF_EXCEPTION& e) {
			arcLog(file, e.what());
			UtilDeletePath(m_pathArchive);
			throw e;
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
	LF_COMPRESS_ARGS& args,
	const std::vector<const ARCHIVE_ENTRY_INFO*> &items,
	ILFProgressHandler& progressHandler,
	ARCLOG &arcLog)
{
	std::unordered_set<std::wstring> items_to_delete;
	for (const auto &item : items) {
		items_to_delete.insert(
			toLower(std::filesystem::path(item->calcFullpath()).lexically_normal())
		);
	}
	arcLog.setArchivePath(m_pathArchive);
	progressHandler.setArchive(m_pathArchive);
	progressHandler.setNumEntries(m_numFiles);

	//read from source
	auto[tempFile, dest] = subDeleteEntries(args, items_to_delete, progressHandler, arcLog);
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
			throw e;
		}
	}
}

#ifdef UNIT_TEST

TEST(ArcFileContent, makeSureItemsExtracted)
{
	_wsetlocale(LC_ALL, L"");	//default locale
	auto tempDir = UtilGetTempPath() / L"arcfilecontent_makeSureItemsExtracted";
	tempDir.make_preferred();
	EXPECT_FALSE(std::filesystem::exists(tempDir));

	{
		CLFPassphraseNULL no_passphrase;
		CArchiveFileContent content(no_passphrase);
		ARCLOG arcLog;
		content.scanArchiveStruct(LF_PROJECT_DIR() / L"test/test_extract.zip", CLFScanProgressHandlerNULL());

		EXPECT_EQ(8, content.getRootNode()->enumChildren().size());
		auto entries = content.findItem(L"*");
		EXPECT_EQ(8, entries.size());

		std::vector<const ARCHIVE_ENTRY_INFO*> entriesSub;
		for (auto entry : entries) {
			entriesSub.push_back(entry.get());
		}

		EXPECT_EQ(entries.size(), entriesSub.size());
		auto extracted = content.makeSureItemsExtracted(entriesSub, tempDir, nullptr, CLFProgressHandlerNULL(), overwrite_options::abort, arcLog);
		EXPECT_EQ(6, extracted.size());

		for (auto f : extracted) {
			EXPECT_TRUE(std::filesystem::exists(f));
			//extracted file should be in tempDir
			EXPECT_NE(std::wstring::npos, f.make_preferred().wstring().find(tempDir));
		}

		const std::vector<std::filesystem::path> expectedPath = {
			tempDir / L"あいうえお.txt",
			//tempDir / L"dirA",	implicit entry
			tempDir / L"dirA/dirB/",
			tempDir / L"dirA/dirB/file2.txt",
			tempDir / L"dirA/dirB/dirC/",
			tempDir / L"dirA/dirB/dirC/file1.txt",
			//tempDir / L"かきくけこ",	implicit entry
			tempDir / L"かきくけこ/file3.txt",
		};
		//all entries are returned as extracted, even if the directory is not stored explicitly
		for (const auto p : expectedPath) {
			EXPECT_TRUE(isIn(extracted, p));
		}
	}
	UtilDeleteDir(tempDir, true);
	EXPECT_FALSE(std::filesystem::exists(tempDir));
}
#endif


#ifdef UNIT_TEST

TEST(ArcFileContent, ARCHIVE_ENTRY_INFO)
{
	_wsetlocale(LC_ALL, L"");	//default locale
	ARCHIVE_ENTRY_INFO root;
	std::vector<std::wstring> files = {
		L"/dirA/dirB/dirC/file1.txt",
		L"/dirA/dirB",
		L"/dirA/dirB/file2.txt",
		L"/dirA/dirB/あいうえお.txt",
		L"/",
	};
	for (const auto &file : files) {
		auto pathname = UtilPathRemoveLastSeparator(LF_sanitize_pathname(file));
		auto elements = UtilSplitString(pathname, L"/");
		if (elements.empty() || elements[0].empty())continue;

		auto &item = root.addEntry(elements);
		EXPECT_NE(&item, &root);
		EXPECT_NE(L"/", pathname);

		item._entry.path = pathname;
		item._entry.stat.st_mode = S_IFREG;	//fake info
		item._entry.stat.st_mtime = time(nullptr);
		item._originalSize = 10;
	}
	/*
		/dirA
		|-- dirB
			|-- dirC
			|   |-- file1.txt
			|-- file2.txt
			|-- あいうえお.txt
	*/

	EXPECT_EQ(1, root.getNumChildren());
	EXPECT_EQ(L"dirA", root.getChild(0)->_entryName);
	EXPECT_EQ(L"dirA", root.getChild(L"dirA")->_entryName);
	EXPECT_EQ(L"dirA", root.getChild(L"DIRA")->_entryName);
	EXPECT_EQ(nullptr, root.getChild(1));
	EXPECT_EQ(nullptr, root.getChild(L"dirB"));
	EXPECT_EQ(nullptr, root.getChild(L"DIRC"));

	EXPECT_EQ(L".txt", root.getChild(L"dirA")->getChild(L"dirB")->getChild(L"file2.txt")->getExt());
	EXPECT_EQ(L"dirA", root.getChild(L"dirA")->calcFullpath());
	EXPECT_EQ(L"dirA/dirB/dirC", root.getChild(L"dirA")->getChild(L"dirB")->getChild(L"dirC")->calcFullpath());
	EXPECT_EQ(6, root.enumChildren().size());

	auto file1 = root.getChild(L"dirA")->getChild(L"dirB")->getChild(L"dirC")->getChild(L"file1.txt");
	EXPECT_EQ(L"dirB/dirC/file1.txt", file1->getRelativePath(root.getChild(L"dirA")));
	EXPECT_EQ(L"dirA/dirB/dirC/file1.txt", file1->getRelativePath(&root));

	auto aiueo = root.getChild(L"dirA")->getChild(L"dirB")->getChild(L"あいうえお.txt");
	EXPECT_EQ(L"あいうえお.txt", aiueo->_entryName);
	EXPECT_EQ(L"dirB/あいうえお.txt", aiueo->getRelativePath(root.getChild(L"dirA")));
	EXPECT_EQ(L"dirA/dirB/あいうえお.txt", aiueo->getRelativePath(&root));
}


TEST(ArcFileContent, isArchiveEncrypted)
{
	CLFPassphraseNULL no_passphrase;
	CArchiveFileContent content(no_passphrase);

	content.scanArchiveStruct(LF_PROJECT_DIR() / L"test/test_password_abcde.zip", CLFScanProgressHandlerNULL());
	EXPECT_EQ(1, content.getRootNode()->enumChildren().size());
	EXPECT_TRUE(content.isArchiveEncrypted());
	content.scanArchiveStruct(LF_PROJECT_DIR() / L"test/test_extract.zip", CLFScanProgressHandlerNULL());
	EXPECT_EQ(8, content.getRootNode()->enumChildren().size());
	EXPECT_FALSE(content.isArchiveEncrypted());
}

TEST(ArcFileContent, isModifySupported_checkArchiveExists)
{
	CLFPassphraseNULL no_passphrase;
	CArchiveFileContent content(no_passphrase);

	EXPECT_FALSE(content.checkArchiveExists());

	content.scanArchiveStruct(LF_PROJECT_DIR() / L"test/test_password_abcde.zip", CLFScanProgressHandlerNULL());
	EXPECT_EQ(1, content.getRootNode()->enumChildren().size());
	EXPECT_TRUE(content.isModifySupported());
	EXPECT_TRUE(content.checkArchiveExists());

	content.scanArchiveStruct(LF_PROJECT_DIR() / L"test/test_extract.zip", CLFScanProgressHandlerNULL());
	EXPECT_EQ(8, content.getRootNode()->enumChildren().size());
	EXPECT_TRUE(content.isModifySupported());
	EXPECT_TRUE(content.checkArchiveExists());

	content.scanArchiveStruct(LF_PROJECT_DIR() / L"test/test.tar.gz", CLFScanProgressHandlerNULL());
	EXPECT_EQ(2, content.getRootNode()->enumChildren().size());
	EXPECT_TRUE(content.isModifySupported());
	EXPECT_TRUE(content.checkArchiveExists());

	content.scanArchiveStruct(LF_PROJECT_DIR() / L"test/test_gzip.gz", CLFScanProgressHandlerNULL());
	EXPECT_EQ(1, content.getRootNode()->enumChildren().size());
	EXPECT_FALSE(content.isModifySupported());
	EXPECT_TRUE(content.checkArchiveExists());
}

/*void addEntries(
	LF_COMPRESS_ARGS& args,
	const std::vector<std::filesystem::path> &files,
	const ARCHIVE_ENTRY_INFO* lpParent,
	ILFProgressHandler& progressHandler,
	ARCLOG &arcLog);
void deleteEntries(
	LF_COMPRESS_ARGS& args,
	const std::vector<const ARCHIVE_ENTRY_INFO*> &items,
	ILFProgressHandler& progressHandler,
	ARCLOG &arcLog);
*/

#endif

