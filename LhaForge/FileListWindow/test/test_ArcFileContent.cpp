#include "stdafx.h"
#ifdef UNIT_TEST
#include <gtest/gtest.h>
#include "FileListWindow/ArcFileContent.h"
#include "extract.h"

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

		item._fullpath = pathname;
		item._nAttribute = S_IFREG;	//fake info
		item._originalSize = 10;
		item._st_mtime = time(nullptr);
	}
	EXPECT_EQ(1, root.getNumChildren());
	EXPECT_EQ(L"dirA", root.getChild(0)->_entryName);
	EXPECT_EQ(L"dirA", root.getChild(L"dirA")->_entryName);
	EXPECT_EQ(L"dirA", root.getChild(L"DIRA")->_entryName);
	EXPECT_EQ(nullptr, root.getChild(1));
	EXPECT_EQ(nullptr, root.getChild(L"dirB"));
	EXPECT_EQ(nullptr, root.getChild(L"DIRC"));

	EXPECT_EQ(L".txt", root.getChild(L"dirA")->getChild(L"dirB")->getChild(L"file2.txt")->getExt());
	EXPECT_EQ(L"dirA", root.getChild(L"dirA")->getFullpath());
	EXPECT_EQ(L"dirA/dirB/dirC", root.getChild(L"dirA")->getChild(L"dirB")->getChild(L"dirC")->getFullpath());
	EXPECT_EQ(6, root.enumFiles().size());

	auto file1 = root.getChild(L"dirA")->getChild(L"dirB")->getChild(L"dirC")->getChild(L"file1.txt");
	EXPECT_EQ(L"dirB/dirC/file1.txt", file1->getRelativePath(root.getChild(L"dirA")));
	EXPECT_EQ(L"dirA/dirB/dirC/file1.txt", file1->getRelativePath(&root));

	auto aiueo = root.getChild(L"dirA")->getChild(L"dirB")->getChild(L"あいうえお.txt");
	EXPECT_EQ(L"あいうえお.txt", aiueo->_entryName);
	EXPECT_EQ(L"dirB/あいうえお.txt", aiueo->getRelativePath(root.getChild(L"dirA")));
	EXPECT_EQ(L"dirA/dirB/あいうえお.txt", aiueo->getRelativePath(&root));
}

TEST(CArchiveFileContent, inspectArchiveStruct)
{
	_wsetlocale(LC_ALL, L"");	//default locale
	CArchiveFileContent content;

	content.inspectArchiveStruct(std::filesystem::path(__FILEW__).parent_path() / L"test_content.zip", nullptr);

	const auto& root = content.m_Root;
	EXPECT_EQ(3, root.getNumChildren());
	EXPECT_EQ(L"dirA", root.getChild(0)->_entryName);
	EXPECT_EQ(L"dirA", root.getChild(L"dirA")->_entryName);
	EXPECT_EQ(L"dirB", root.getChild(L"dirA")->getChild(L"dirB")->_entryName);
	EXPECT_EQ(8, root.enumFiles().size());
	EXPECT_EQ(L"file3.txt", root.getChild(L"かきくけこ")->getChild(0)->_entryName);
	EXPECT_EQ(L"あいうえお.txt", root.getChild(L"あいうえお.txt")->_entryName);
}

#endif

