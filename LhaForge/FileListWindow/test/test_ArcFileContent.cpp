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
#include "../ArcFileContent.h"

TEST(ArcFileContent, scanArchiveStruct)
{
	_wsetlocale(LC_ALL, L"");	//default locale
	auto pp = std::make_shared<CLFPassphraseNULL>();
	CArchiveFileContent content(pp);

	auto arcpath = std::filesystem::path(__FILEW__).parent_path() / L"test_content.zip";
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

TEST(ArcFileContent, findItem)
{
	_wsetlocale(LC_ALL, L"");	//default locale
	auto pp = std::make_shared<CLFPassphraseNULL>();
	CArchiveFileContent content(pp);

	ARCHIVE_FIND_CONDITION afc;

	content.scanArchiveStruct(std::filesystem::path(__FILEW__).parent_path() / L"test_content.zip", CLFScanProgressHandlerNULL());
	EXPECT_TRUE(content.isOK());

	//---by filename
	afc.setFindByFilename(L"*");
	auto result = content.findItem(afc);
	EXPECT_EQ(content.getRootNode()->enumChildren().size(), result.size());

	afc.setFindByFilename(L"*.*");
	result = content.findItem(afc);
	EXPECT_EQ(content.getRootNode()->enumChildren().size(), result.size());

	afc.setFindByFilename(L".txt");
	result = content.findItem(afc);
	EXPECT_EQ(4, result.size());

	afc.setFindByFilename(L"*.txt");
	result = content.findItem(afc);
	EXPECT_EQ(4, result.size());

	afc.setFindByFilename(L".TXT");
	result = content.findItem(afc);
	EXPECT_EQ(4, result.size());

	afc.setFindByFilename(L"dirB");
	result = content.findItem(afc);
	EXPECT_EQ(1, result.size());

	//---by fullpath
	afc.setFindByFullpath(L"かきくけこ/*.txt");
	result = content.findItem(afc);
	EXPECT_EQ(1, result.size());

	afc.setFindByFullpath(L"かきくけこ\\*.txt");
	result = content.findItem(afc);
	EXPECT_EQ(1, result.size());

	//---by original size
	afc.setFindByOriginalSize(5, ARCHIVE_FIND_CONDITION::COMPARE::equal);
	result = content.findItem(afc);
	EXPECT_EQ(3, result.size());

	afc.setFindByOriginalSize(5, ARCHIVE_FIND_CONDITION::COMPARE::equalOrGreater);
	result = content.findItem(afc);
	EXPECT_EQ(3, result.size());

	afc.setFindByOriginalSize(5, ARCHIVE_FIND_CONDITION::COMPARE::equalOrLess);
	result = content.findItem(afc);
	EXPECT_EQ(8, result.size());

	//---by mode
	afc.setFindByMode(S_IFDIR);
	result = content.findItem(afc);
	EXPECT_EQ(4, result.size());

	afc.setFindByMode(S_IFREG);
	result = content.findItem(afc);
	EXPECT_EQ(4, result.size());

	//------------------
	content.scanArchiveStruct(std::filesystem::path(__FILEW__).parent_path() / L"test_mtime.zip", CLFScanProgressHandlerNULL());
	EXPECT_TRUE(content.isOK());

	//---by st_mtime, in date unit
	SYSTEMTIME st = {};
	st.wYear = 2021;
	st.wMonth = 6;
	st.wDay = 4;
	afc.setFindByMDate(st, ARCHIVE_FIND_CONDITION::COMPARE::equal);
	result = content.findItem(afc);
	EXPECT_EQ(1, result.size());

	afc.setFindByMDate(st, ARCHIVE_FIND_CONDITION::COMPARE::equalOrGreater);
	result = content.findItem(afc);
	EXPECT_EQ(2, result.size());

	afc.setFindByMDate(st, ARCHIVE_FIND_CONDITION::COMPARE::equalOrLess);
	result = content.findItem(afc);
	EXPECT_EQ(4, result.size());
}


TEST(ArcFileContent, extractEntries)
{
	_wsetlocale(LC_ALL, L"");	//default locale
	auto tempDir = UtilGetTempPath() / L"arcfilecontent_extractEntries";
	tempDir.make_preferred();
	{
		auto pp = std::make_shared<CLFPassphraseNULL>();
		CArchiveFileContent content(pp);
		ARCLOG arcLog;
		std::vector<const ARCHIVE_ENTRY_INFO*> entriesSub;

		EXPECT_FALSE(content.checkArchiveExists());

		content.scanArchiveStruct(LF_PROJECT_DIR() / L"test/test_extract.zip", CLFScanProgressHandlerNULL());
		EXPECT_EQ(8, content.getRootNode()->enumChildren().size());

		ARCHIVE_FIND_CONDITION afc;
		afc.setFindByFullpath(L"*");
		auto entries = content.findItem(afc);
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
		auto pp = std::make_shared<CLFPassphraseConst>(L"abcde");
		CArchiveFileContent content(pp);
		ARCLOG arcLog;
		std::vector<const ARCHIVE_ENTRY_INFO*> entriesSub;

		EXPECT_FALSE(content.checkArchiveExists());

		content.scanArchiveStruct(LF_PROJECT_DIR() / L"test/test_password_abcde.zip", CLFScanProgressHandlerNULL());
		EXPECT_EQ(1, content.getRootNode()->enumChildren().size());
		ARCHIVE_FIND_CONDITION afc;
		afc.setFindByFullpath(L"*.txt");
		auto entries = content.findItem(afc);
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

TEST(ArcFileContent, addEntries_file_in_directory)
{
	_wsetlocale(LC_ALL, L"");	//default locale
	auto temp = UtilGetTemporaryFileName();
	LF_COMPRESS_ARGS args;
	args.load(CConfigFile());
	//copy
	{
		CAutoFile fout, fin;
		fout.open(temp, L"wb");
		fin.open(LF_PROJECT_DIR() / L"test/test_extract.zip", L"rb");

		const int bufsize = 256;
		std::vector<char> buf(bufsize);
		for (;;) {
			auto size = fread(&buf[0], 1, bufsize, fin);
			fwrite(&buf[0], 1, bufsize, fout);
			if (size < bufsize)break;
		}
	}
	auto src0 = UtilGetTempPath() / "arcfilecontent/added_file3.txt";
	{
		std::filesystem::create_directories(src0.parent_path());
		CAutoFile f;
		f.open(src0, L"w");
		fputs("abcde12345", f);
	}
	auto src1 = UtilGetTempPath() / "added_file4.txt";
	{
		CAutoFile f;
		f.open(src1, L"w");
		fputs("ABCDE1234567890", f);
	}
	{
		//keep previous
		auto pp = std::make_shared<CLFPassphraseNULL>();
		CArchiveFileContent content(pp);
		ARCLOG arcLog;

		content.scanArchiveStruct(temp, CLFScanProgressHandlerNULL());
		content.addEntries(
			args,
			{ src1, src0.parent_path(), src1 },	//duplicated entry contained
			content.getRootNode()->getChild(L"かきくけこ"),
			CLFProgressHandlerNULL(),
			CLFOverwriteInArchiveConfirmFORCED(overwrite_options::skip),
			arcLog);

		CLFArchive a;
		a.read_open(temp, pp);
		auto e = a.read_entry_begin();
		EXPECT_NE(nullptr, e);
		EXPECT_EQ(L"dirA/dirB/", e->path.wstring());

		e = a.read_entry_next();
		EXPECT_NE(nullptr, e);
		EXPECT_EQ(L"dirA/dirB/dirC/", e->path.wstring());

		e = a.read_entry_next();
		EXPECT_NE(nullptr, e);
		EXPECT_EQ(L"dirA/dirB/dirC/file1.txt", e->path.wstring());

		e = a.read_entry_next();
		EXPECT_NE(nullptr, e);
		EXPECT_EQ(L"dirA/dirB/file2.txt", e->path.wstring());

		e = a.read_entry_next();
		EXPECT_NE(nullptr, e);
		EXPECT_EQ(L"あいうえお.txt", e->path.wstring());

		e = a.read_entry_next();
		EXPECT_NE(nullptr, e);
		EXPECT_EQ(L"かきくけこ/file3.txt", e->path);
		EXPECT_EQ(5, e->stat.st_size);

		e = a.read_entry_next();
		EXPECT_NE(nullptr, e);
		EXPECT_EQ(L"かきくけこ/added_file4.txt", e->path.wstring());
		EXPECT_EQ(15, e->stat.st_size);

		e = a.read_entry_next();
		EXPECT_NE(nullptr, e);
		EXPECT_EQ(L"かきくけこ/arcfilecontent/", e->path.wstring());
		EXPECT_TRUE(e->is_directory());

		e = a.read_entry_next();
		EXPECT_NE(nullptr, e);
		EXPECT_EQ(L"かきくけこ/arcfilecontent/added_file3.txt", e->path.wstring());
		EXPECT_EQ(10, e->stat.st_size);

		e = a.read_entry_next();
		EXPECT_EQ(nullptr, e);
	}
	UtilDeletePath(temp);
	UtilDeletePath(src0);
	UtilDeletePath(src1);
	EXPECT_FALSE(std::filesystem::exists(src0));
	EXPECT_FALSE(std::filesystem::exists(src1));
	EXPECT_FALSE(std::filesystem::exists(temp));
}

TEST(ArcFileContent, addEntries_keep)
{
	_wsetlocale(LC_ALL, L"");	//default locale
	auto temp = UtilGetTemporaryFileName();
	LF_COMPRESS_ARGS args;
	args.load(CConfigFile());
	//copy
	{
		CAutoFile fout, fin;
		fout.open(temp, L"wb");
		fin.open(LF_PROJECT_DIR() / L"test/test_extract.zip", L"rb");

		const int bufsize = 256;
		std::vector<char> buf(bufsize);
		for (;;) {
			auto size = fread(&buf[0], 1, bufsize, fin);
			fwrite(&buf[0], 1, bufsize, fout);
			if (size < bufsize)break;
		}
	}
	auto src = UtilGetTempPath() / "file3.txt";
	{
		CAutoFile f;
		f.open(src, L"w");
		fputs("abcde12345aaaaaaa", f);
	}
	{
		//keep previous
		auto pp = std::make_shared<CLFPassphraseNULL>();
		CArchiveFileContent content(pp);
		ARCLOG arcLog;

		content.scanArchiveStruct(temp, CLFScanProgressHandlerNULL());
		content.addEntries(
			args,
			{ src },
			content.getRootNode()->getChild(L"かきくけこ"),
			CLFProgressHandlerNULL(),
			CLFOverwriteInArchiveConfirmFORCED(overwrite_options::skip),
			arcLog);

		CLFArchive a;
		a.read_open(temp, pp);
		auto e = a.read_entry_begin();
		EXPECT_NE(nullptr, e);
		EXPECT_EQ(L"dirA/dirB/", e->path);

		e = a.read_entry_next();
		EXPECT_NE(nullptr, e);
		EXPECT_EQ(L"dirA/dirB/dirC/", e->path);

		e = a.read_entry_next();
		EXPECT_NE(nullptr, e);
		EXPECT_EQ(L"dirA/dirB/dirC/file1.txt", e->path);

		e = a.read_entry_next();
		EXPECT_NE(nullptr, e);
		EXPECT_EQ(L"dirA/dirB/file2.txt", e->path);

		e = a.read_entry_next();
		EXPECT_NE(nullptr, e);
		EXPECT_EQ(L"あいうえお.txt", e->path);

		e = a.read_entry_next();
		EXPECT_NE(nullptr, e);
		EXPECT_EQ(L"かきくけこ/file3.txt", e->path);
		EXPECT_NE(17, e->stat.st_size);

		e = a.read_entry_next();
		EXPECT_EQ(nullptr, e);
	}
	UtilDeletePath(temp);
	UtilDeletePath(src);
	EXPECT_FALSE(std::filesystem::exists(src));
	EXPECT_FALSE(std::filesystem::exists(temp));
}

TEST(ArcFileContent, addEntries_abort)
{
	_wsetlocale(LC_ALL, L"");	//default locale
	auto temp = UtilGetTemporaryFileName();
	LF_COMPRESS_ARGS args;
	args.load(CConfigFile());
	//copy
	{
		CAutoFile fout, fin;
		fout.open(temp, L"wb");
		fin.open(LF_PROJECT_DIR() / L"test/test_extract.zip", L"rb");

		const int bufsize = 256;
		std::vector<char> buf(bufsize);
		for (;;) {
			auto size = fread(&buf[0], 1, bufsize, fin);
			fwrite(&buf[0], 1, bufsize, fout);
			if (size < bufsize)break;
		}
	}
	auto src = UtilGetTempPath() / "file3.txt";
	{
		CAutoFile f;
		f.open(src, L"w");
		fputs("abcde12345", f);
	}
	{
		//abort
		auto pp = std::make_shared<CLFPassphraseNULL>();
		CArchiveFileContent content(pp);
		ARCLOG arcLog;

		content.scanArchiveStruct(temp, CLFScanProgressHandlerNULL());
		EXPECT_THROW(
			content.addEntries(
				args,
				{ src },
				content.getRootNode()->getChild(L"かきくけこ"),
				CLFProgressHandlerNULL(),
				CLFOverwriteInArchiveConfirmFORCED(overwrite_options::abort),
				arcLog), LF_USER_CANCEL_EXCEPTION);

		CLFArchive a;
		a.read_open(temp, pp);
		auto e = a.read_entry_begin();
		EXPECT_NE(nullptr, e);
		EXPECT_EQ(L"dirA/dirB/", e->path);

		e = a.read_entry_next();
		EXPECT_NE(nullptr, e);
		EXPECT_EQ(L"dirA/dirB/dirC/", e->path);

		e = a.read_entry_next();
		EXPECT_NE(nullptr, e);
		EXPECT_EQ(L"dirA/dirB/dirC/file1.txt", e->path);

		e = a.read_entry_next();
		EXPECT_NE(nullptr, e);
		EXPECT_EQ(L"dirA/dirB/file2.txt", e->path);

		e = a.read_entry_next();
		EXPECT_NE(nullptr, e);
		EXPECT_EQ(L"あいうえお.txt", e->path);

		e = a.read_entry_next();
		EXPECT_NE(nullptr, e);
		EXPECT_EQ(L"かきくけこ/file3.txt", e->path);
		EXPECT_NE(10, e->stat.st_size);

		e = a.read_entry_next();
		EXPECT_EQ(nullptr, e);
	}

	UtilDeletePath(temp);
	UtilDeletePath(src);
	EXPECT_FALSE(std::filesystem::exists(src));
	EXPECT_FALSE(std::filesystem::exists(temp));
}

TEST(ArcFileContent, addEntries_replace)
{
	_wsetlocale(LC_ALL, L"");	//default locale
	auto temp = UtilGetTemporaryFileName();
	LF_COMPRESS_ARGS args;
	args.load(CConfigFile());
	//copy
	{
		CAutoFile fout, fin;
		fout.open(temp, L"wb");
		fin.open(LF_PROJECT_DIR() / L"test/test_extract.zip", L"rb");

		const int bufsize = 256;
		std::vector<char> buf(bufsize);
		for (;;) {
			auto size = fread(&buf[0], 1, bufsize, fin);
			fwrite(&buf[0], 1, bufsize, fout);
			if (size < bufsize)break;
		}
	}
	auto src = UtilGetTempPath() / "file3.txt";
	{
		CAutoFile f;
		f.open(src, L"w");
		fputs("abcde12345aaaaaaa", f);
	}

	{
		//overwrite
		auto pp = std::make_shared<CLFPassphraseNULL>();
		CArchiveFileContent content(pp);
		ARCLOG arcLog;

		content.scanArchiveStruct(temp, CLFScanProgressHandlerNULL());
		content.addEntries(
			args,
			{ src },
			content.getRootNode()->getChild(L"かきくけこ"),
			CLFProgressHandlerNULL(),
			CLFOverwriteInArchiveConfirmFORCED(overwrite_options::overwrite),
			arcLog);

		CLFArchive a;
		a.read_open(temp, pp);
		auto e = a.read_entry_begin();
		EXPECT_NE(nullptr, e);
		EXPECT_EQ(L"dirA/dirB/", e->path);

		e = a.read_entry_next();
		EXPECT_NE(nullptr, e);
		EXPECT_EQ(L"dirA/dirB/dirC/", e->path);

		e = a.read_entry_next();
		EXPECT_NE(nullptr, e);
		EXPECT_EQ(L"dirA/dirB/dirC/file1.txt", e->path);

		e = a.read_entry_next();
		EXPECT_NE(nullptr, e);
		EXPECT_EQ(L"dirA/dirB/file2.txt", e->path);

		e = a.read_entry_next();
		EXPECT_NE(nullptr, e);
		EXPECT_EQ(L"あいうえお.txt", e->path);

		e = a.read_entry_next();
		EXPECT_NE(nullptr, e);
		EXPECT_EQ(L"かきくけこ/file3.txt", e->path);
		EXPECT_EQ(17, e->stat.st_size);

		e = a.read_entry_next();
		EXPECT_EQ(nullptr, e);
	}
	{
		//test archive
		auto pp = std::make_shared<CLFPassphraseNULL>();
		CLFArchive a;
		a.read_open(temp, pp);
		for (auto* entry = a.read_entry_begin(); entry; entry = a.read_entry_next()) {
			if (!entry->is_directory()) {
				//go
				int64_t global_offset = 0;
				for (bool bEOF = false; !bEOF;) {
					a.read_file_entry_block([&](const void* buf, int64_t data_size, const offset_info* offset) {
						if (!buf || data_size == 0) {
							bEOF = true;
						} else {
							global_offset += data_size;
							if (offset && offset->offset != global_offset) {
								global_offset = offset->offset;
							}
						}
					});
				}
			}
		}
		//end
		a.close();
	}
	UtilDeletePath(temp);
	UtilDeletePath(src);
	EXPECT_FALSE(std::filesystem::exists(src));
	EXPECT_FALSE(std::filesystem::exists(temp));
}

TEST(ArcFileContent, addEntries_replace_gz)
{
	_wsetlocale(LC_ALL, L"");	//default locale
	auto temp = UtilGetTemporaryFileName();
	LF_COMPRESS_ARGS args;
	args.load(CConfigFile());
	//copy
	{
		CAutoFile fout, fin;
		fout.open(temp, L"wb");
		fin.open(LF_PROJECT_DIR() / L"test/test_gzip.gz", L"rb");

		const int bufsize = 256;
		std::vector<char> buf(bufsize);
		for (;;) {
			auto size = fread(&buf[0], 1, bufsize, fin);
			fwrite(&buf[0], 1, bufsize, fout);
			if (size < bufsize)break;
		}
	}
	auto src = UtilGetTempPath() / "file3.txt";
	{
		CAutoFile f;
		f.open(src, L"w");
		fputs("abcde12345", f);
	}

	{
		//overwrite
		auto pp = std::make_shared<CLFPassphraseNULL>();
		CArchiveFileContent content(pp);
		ARCLOG arcLog;

		content.scanArchiveStruct(temp, CLFScanProgressHandlerNULL());
		EXPECT_THROW(
			content.addEntries(
			args,
			{ src },
			content.getRootNode(),
			CLFProgressHandlerNULL(),
			CLFOverwriteInArchiveConfirmFORCED(overwrite_options::overwrite),
			arcLog), LF_EXCEPTION);
	}
	{
		//test archive
		auto pp = std::make_shared<CLFPassphraseNULL>();
		CLFArchive a;
		a.read_open(temp, pp);
		for (auto* entry = a.read_entry_begin(); entry; entry = a.read_entry_next()) {
			if (!entry->is_directory()) {
				//go
				int64_t global_offset = 0;
				for (bool bEOF = false; !bEOF;) {
					a.read_file_entry_block([&](const void* buf, int64_t data_size, const offset_info* offset) {
						if (!buf || data_size == 0) {
							bEOF = true;
						} else {
							global_offset += data_size;
							if (offset && offset->offset != global_offset) {
								global_offset = offset->offset;
							}
						}
					});
				}
			}
		}
		//end
		a.close();
	}
	UtilDeletePath(temp);
	UtilDeletePath(src);
	EXPECT_FALSE(std::filesystem::exists(src));
	EXPECT_FALSE(std::filesystem::exists(temp));
}


TEST(ArcFileContent, deleteEntries_file)
{
	_wsetlocale(LC_ALL, L"");	//default locale
	auto temp = UtilGetTemporaryFileName();
	LF_COMPRESS_ARGS args;
	args.load(CConfigFile());
	//copy
	{
		CAutoFile fout, fin;
		fout.open(temp, L"wb");
		fin.open(LF_PROJECT_DIR() / L"test/test_extract.zip", L"rb");

		const int bufsize = 256;
		std::vector<char> buf(bufsize);
		for (;;) {
			auto size = fread(&buf[0], 1, bufsize, fin);
			fwrite(&buf[0], 1, bufsize, fout);
			if (size < bufsize)break;
		}
	}
	{
		auto pp = std::make_shared<CLFPassphraseNULL>();
		CArchiveFileContent content(pp);
		ARCLOG arcLog;

		content.scanArchiveStruct(temp, CLFScanProgressHandlerNULL());
		content.deleteEntries(args,
			{ content.getRootNode()->getChild(L"かきくけこ")->getChild(L"file3.txt") },
			CLFProgressHandlerNULL(), arcLog);

		CLFArchive a;
		a.read_open(temp, pp);
		auto e = a.read_entry_begin();
		EXPECT_NE(nullptr, e);
		EXPECT_EQ(L"dirA/dirB/", e->path);

		e = a.read_entry_next();
		EXPECT_NE(nullptr, e);
		EXPECT_EQ(L"dirA/dirB/dirC/", e->path);

		e = a.read_entry_next();
		EXPECT_NE(nullptr, e);
		EXPECT_EQ(L"dirA/dirB/dirC/file1.txt", e->path);

		e = a.read_entry_next();
		EXPECT_NE(nullptr, e);
		EXPECT_EQ(L"dirA/dirB/file2.txt", e->path);

		e = a.read_entry_next();
		EXPECT_NE(nullptr, e);
		EXPECT_EQ(L"あいうえお.txt", e->path);

		e = a.read_entry_next();
		EXPECT_EQ(nullptr, e);
	}
	UtilDeletePath(temp);
	EXPECT_FALSE(std::filesystem::exists(temp));
}

TEST(ArcFileContent, deleteEntries_dir)
{
	_wsetlocale(LC_ALL, L"");	//default locale
	auto temp = UtilGetTemporaryFileName();
	LF_COMPRESS_ARGS args;
	args.load(CConfigFile());
	//copy
	{
		CAutoFile fout, fin;
		fout.open(temp, L"wb");
		fin.open(LF_PROJECT_DIR() / L"test/test_extract.zip", L"rb");

		const int bufsize = 256;
		std::vector<char> buf(bufsize);
		for (;;) {
			auto size = fread(&buf[0], 1, bufsize, fin);
			fwrite(&buf[0], 1, bufsize, fout);
			if (size < bufsize)break;
		}
	}
	{
		auto pp = std::make_shared<CLFPassphraseNULL>();
		CArchiveFileContent content(pp);
		ARCLOG arcLog;

		content.scanArchiveStruct(temp, CLFScanProgressHandlerNULL());
		content.deleteEntries(args,
			{ content.getRootNode()->getChild(L"かきくけこ") },
			CLFProgressHandlerNULL(), arcLog);

		CLFArchive a;
		a.read_open(temp, pp);
		auto e = a.read_entry_begin();
		EXPECT_NE(nullptr, e);
		EXPECT_EQ(L"dirA/dirB/", e->path);

		e = a.read_entry_next();
		EXPECT_NE(nullptr, e);
		EXPECT_EQ(L"dirA/dirB/dirC/", e->path);

		e = a.read_entry_next();
		EXPECT_NE(nullptr, e);
		EXPECT_EQ(L"dirA/dirB/dirC/file1.txt", e->path);

		e = a.read_entry_next();
		EXPECT_NE(nullptr, e);
		EXPECT_EQ(L"dirA/dirB/file2.txt", e->path);

		e = a.read_entry_next();
		EXPECT_NE(nullptr, e);
		EXPECT_EQ(L"あいうえお.txt", e->path);

		e = a.read_entry_next();
		EXPECT_EQ(nullptr, e);
	}
	UtilDeletePath(temp);
	EXPECT_FALSE(std::filesystem::exists(temp));
}

TEST(ArcFileContent, deleteEntries_gz)
{
	_wsetlocale(LC_ALL, L"");	//default locale
	auto temp = UtilGetTemporaryFileName();
	LF_COMPRESS_ARGS args;
	args.load(CConfigFile());
	//copy
	{
		CAutoFile fout, fin;
		fout.open(temp, L"wb");
		fin.open(LF_PROJECT_DIR() / L"test/test_gzip.gz", L"rb");

		const int bufsize = 256;
		std::vector<char> buf(bufsize);
		for (;;) {
			auto size = fread(&buf[0], 1, bufsize, fin);
			fwrite(&buf[0], 1, bufsize, fout);
			if (size < bufsize)break;
		}
	}
	{
		auto pp = std::make_shared<CLFPassphraseNULL>();
		CArchiveFileContent content(pp);
		ARCLOG arcLog;

		content.scanArchiveStruct(temp, CLFScanProgressHandlerNULL());
		EXPECT_THROW(content.deleteEntries(args,
			{ content.getRootNode()->getChild(0) },
			CLFProgressHandlerNULL(), arcLog), LF_EXCEPTION);
	}
	UtilDeletePath(temp);
	EXPECT_FALSE(std::filesystem::exists(temp));
}


TEST(ArcFileContent, makeSureItemsExtracted)
{
	_wsetlocale(LC_ALL, L"");	//default locale
	auto tempDir = UtilGetTempPath() / L"arcfilecontent_makeSureItemsExtracted";
	tempDir.make_preferred();
	EXPECT_FALSE(std::filesystem::exists(tempDir));

	{
		auto pp = std::make_shared<CLFPassphraseNULL>();
		CArchiveFileContent content(pp);
		ARCLOG arcLog;
		content.scanArchiveStruct(LF_PROJECT_DIR() / L"test/test_extract.zip", CLFScanProgressHandlerNULL());

		EXPECT_EQ(8, content.getRootNode()->enumChildren().size());

		ARCHIVE_FIND_CONDITION afc;
		afc.setFindByFullpath(L"*");
		auto entries = content.findItem(afc);
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
	auto pp = std::make_shared<CLFPassphraseNULL>();
	CArchiveFileContent content(pp);

	content.scanArchiveStruct(LF_PROJECT_DIR() / L"test/test_password_abcde.zip", CLFScanProgressHandlerNULL());
	EXPECT_EQ(1, content.getRootNode()->enumChildren().size());
	EXPECT_TRUE(content.isArchiveEncrypted());
	content.scanArchiveStruct(LF_PROJECT_DIR() / L"test/test_extract.zip", CLFScanProgressHandlerNULL());
	EXPECT_EQ(8, content.getRootNode()->enumChildren().size());
	EXPECT_FALSE(content.isArchiveEncrypted());
}

TEST(ArcFileContent, isModifySupported_checkArchiveExists_isMultipleContentAllowed)
{
	auto pp = std::make_shared<CLFPassphraseNULL>();
	CArchiveFileContent content(pp);

	EXPECT_FALSE(content.checkArchiveExists());

	content.scanArchiveStruct(LF_PROJECT_DIR() / L"test/test_password_abcde.zip", CLFScanProgressHandlerNULL());
	EXPECT_EQ(1, content.getRootNode()->enumChildren().size());
	EXPECT_TRUE(content.isModifySupported());
	EXPECT_TRUE(content.checkArchiveExists());
	EXPECT_TRUE(content.isMultipleContentAllowed());

	content.scanArchiveStruct(LF_PROJECT_DIR() / L"test/test_extract.zip", CLFScanProgressHandlerNULL());
	EXPECT_EQ(8, content.getRootNode()->enumChildren().size());
	EXPECT_TRUE(content.isModifySupported());
	EXPECT_TRUE(content.checkArchiveExists());
	EXPECT_TRUE(content.isMultipleContentAllowed());

	content.scanArchiveStruct(LF_PROJECT_DIR() / L"test/test.tar.gz", CLFScanProgressHandlerNULL());
	EXPECT_EQ(2, content.getRootNode()->enumChildren().size());
	EXPECT_TRUE(content.isModifySupported());
	EXPECT_TRUE(content.checkArchiveExists());
	EXPECT_TRUE(content.isMultipleContentAllowed());

	content.scanArchiveStruct(LF_PROJECT_DIR() / L"test/test_gzip.gz", CLFScanProgressHandlerNULL());
	EXPECT_EQ(1, content.getRootNode()->enumChildren().size());
	EXPECT_FALSE(content.isModifySupported());
	EXPECT_TRUE(content.checkArchiveExists());
	EXPECT_FALSE(content.isMultipleContentAllowed());
}

TEST(ArcFileContent, safe_unicode_path)
{
	auto pp = std::make_shared<CLFPassphraseNULL>();
	CArchiveFileContent content(pp);

	EXPECT_FALSE(content.checkArchiveExists());

	content.scanArchiveStruct(LF_PROJECT_DIR() / L"test/test_unicode_control.zip", CLFScanProgressHandlerNULL());
	EXPECT_EQ(3, content.getRootNode()->enumChildren().size());
	auto dir = content.getRootNode()->getChild(0);
	auto numEntries = dir->getNumChildren();
	EXPECT_EQ(2, numEntries);

	EXPECT_TRUE(UtilIsSafeUnicode(dir->getChild(0)->calcFullpath()));
	EXPECT_TRUE(UtilIsSafeUnicode(dir->getChild(0)->_entryName));
	EXPECT_FALSE(UtilIsSafeUnicode(dir->getChild(0)->_entry.path));

	EXPECT_TRUE(UtilIsSafeUnicode(dir->getChild(1)->calcFullpath()));
	EXPECT_TRUE(UtilIsSafeUnicode(dir->getChild(1)->_entryName));
	EXPECT_TRUE(UtilIsSafeUnicode(dir->getChild(1)->_entry.path));
}
