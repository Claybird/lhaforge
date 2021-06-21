#include "stdafx.h"
#include "FindDlg.h"


#ifdef UNIT_TEST

TEST(CLFFindDialog, getCondition)
{
	CLFFindDialog dlg;
	{
		EXPECT_EQ(FIND_CONDITION::filename_match, (FIND_CONDITION)dlg._condition);
		dlg._path = L"abc";

		auto cond = dlg.getCondition();
		EXPECT_EQ(ARCHIVE_FIND_CONDITION::KEY::filename, cond.key);
		EXPECT_EQ(dlg._path, cond.patternStr);
	}

	//----
	{
		dlg._condition = (int)FIND_CONDITION::filepath_match;
		dlg._path = L"DEF";

		auto cond = dlg.getCondition();
		EXPECT_EQ(ARCHIVE_FIND_CONDITION::KEY::fullpath, cond.key);
		EXPECT_EQ(dlg._path, cond.patternStr);
	}

	//----
	{
		dlg._condition = (int)FIND_CONDITION::original_size_equal;
		dlg._sizeStr = L"10KB";

		auto cond = dlg.getCondition();
		EXPECT_EQ(ARCHIVE_FIND_CONDITION::KEY::originalSize, cond.key);
		EXPECT_EQ(ARCHIVE_FIND_CONDITION::COMPARE::equal, cond.compare);
		EXPECT_EQ(10 * 1024, cond.st_size);
	}

	{
		dlg._condition = (int)FIND_CONDITION::original_size_equal_or_less;
		dlg._sizeStr = L"10KB";

		auto cond = dlg.getCondition();
		EXPECT_EQ(ARCHIVE_FIND_CONDITION::KEY::originalSize, cond.key);
		EXPECT_EQ(ARCHIVE_FIND_CONDITION::COMPARE::equalOrLess, cond.compare);
		EXPECT_EQ(10 * 1024, cond.st_size);
	}

	{
		dlg._condition = (int)FIND_CONDITION::original_size_equal_or_greater;
		dlg._sizeStr = L"10KB";

		auto cond = dlg.getCondition();
		EXPECT_EQ(ARCHIVE_FIND_CONDITION::KEY::originalSize, cond.key);
		EXPECT_EQ(ARCHIVE_FIND_CONDITION::COMPARE::equalOrGreater, cond.compare);
		EXPECT_EQ(10 * 1024, cond.st_size);
	}

	//----
	{
		dlg._condition = (int)FIND_CONDITION::mdate_equal;

		auto cond = dlg.getCondition();
		EXPECT_EQ(ARCHIVE_FIND_CONDITION::KEY::mdate, cond.key);
		EXPECT_EQ(ARCHIVE_FIND_CONDITION::COMPARE::equal, cond.compare);
	}

	{
		dlg._condition = (int)FIND_CONDITION::mdate_equal_or_newer;

		auto cond = dlg.getCondition();
		EXPECT_EQ(ARCHIVE_FIND_CONDITION::KEY::mdate, cond.key);
		EXPECT_EQ(ARCHIVE_FIND_CONDITION::COMPARE::equalOrGreater, cond.compare);
	}

	{
		dlg._condition = (int)FIND_CONDITION::mdate_equal_or_older;

		auto cond = dlg.getCondition();
		EXPECT_EQ(ARCHIVE_FIND_CONDITION::KEY::mdate, cond.key);
		EXPECT_EQ(ARCHIVE_FIND_CONDITION::COMPARE::equalOrLess, cond.compare);
	}

	//----
	{
		dlg._condition = (int)FIND_CONDITION::all_files;

		auto cond = dlg.getCondition();
		EXPECT_EQ(ARCHIVE_FIND_CONDITION::KEY::mode, cond.key);
		EXPECT_EQ(S_IFREG, cond.st_mode_mask);
	}
	{
		dlg._condition = (int)FIND_CONDITION::all_directories;

		auto cond = dlg.getCondition();
		EXPECT_EQ(ARCHIVE_FIND_CONDITION::KEY::mode, cond.key);
		EXPECT_EQ(S_IFDIR, cond.st_mode_mask);
	}

	//----
	{
		dlg._condition = (int)FIND_CONDITION::everything;

		auto cond = dlg.getCondition();
		EXPECT_EQ(ARCHIVE_FIND_CONDITION::KEY::filename, cond.key);
		EXPECT_EQ(L"*", cond.patternStr);
	}
}

TEST(CLFFindDialog, setCondition)
{
	ARCHIVE_FIND_CONDITION cond;
	{
		CLFFindDialog dlg;
		cond.setFindByFilename(L"*");
		dlg.setCondition(cond);
		EXPECT_EQ((int)FIND_CONDITION::everything, dlg._condition);

		cond.setFindByFilename(L"*.*");
		dlg.setCondition(cond);
		EXPECT_EQ((int)FIND_CONDITION::everything, dlg._condition);

		cond.setFindByFilename(L"*.txt");
		dlg.setCondition(cond);
		EXPECT_EQ((int)FIND_CONDITION::filename_match, dlg._condition);
		EXPECT_EQ(L"*.txt", dlg._path);
	}

	{
		CLFFindDialog dlg;
		cond.setFindByFullpath(L"*");
		dlg.setCondition(cond);
		EXPECT_EQ((int)FIND_CONDITION::everything, dlg._condition);

		cond.setFindByFullpath(L"*.*");
		dlg.setCondition(cond);
		EXPECT_EQ((int)FIND_CONDITION::everything, dlg._condition);

		cond.setFindByFullpath(L"dir/*.txt");
		dlg.setCondition(cond);
		EXPECT_EQ((int)FIND_CONDITION::filepath_match, dlg._condition);
		EXPECT_EQ(L"dir/*.txt", dlg._path);
	}

	{
		CLFFindDialog dlg;
		cond.setFindByOriginalSize(10240, ARCHIVE_FIND_CONDITION::COMPARE::equal);
		dlg.setCondition(cond);
		EXPECT_EQ((int)FIND_CONDITION::original_size_equal, dlg._condition);
		EXPECT_EQ(L"10KB", dlg._sizeStr);

		cond.setFindByOriginalSize(10240, ARCHIVE_FIND_CONDITION::COMPARE::equalOrGreater);
		dlg.setCondition(cond);
		EXPECT_EQ((int)FIND_CONDITION::original_size_equal_or_greater, dlg._condition);
		EXPECT_EQ(L"10KB", dlg._sizeStr);

		cond.setFindByOriginalSize(10240, ARCHIVE_FIND_CONDITION::COMPARE::equalOrLess);
		dlg.setCondition(cond);
		EXPECT_EQ((int)FIND_CONDITION::original_size_equal_or_less, dlg._condition);
		EXPECT_EQ(L"10KB", dlg._sizeStr);
	}

	{
		SYSTEMTIME st = {};
		st.wYear = 2021;
		st.wMonth = 5;
		st.wDay = 21;
		CLFFindDialog dlg;
		cond.setFindByMDate(st, ARCHIVE_FIND_CONDITION::COMPARE::equal);
		dlg.setCondition(cond);
		EXPECT_EQ((int)FIND_CONDITION::mdate_equal, dlg._condition);
		EXPECT_EQ(st.wYear, dlg._date.wYear);
		EXPECT_EQ(st.wMonth, dlg._date.wMonth);
		EXPECT_EQ(st.wDay, dlg._date.wDay);

		cond.setFindByMDate(st, ARCHIVE_FIND_CONDITION::COMPARE::equalOrGreater);
		dlg.setCondition(cond);
		EXPECT_EQ((int)FIND_CONDITION::mdate_equal_or_newer, dlg._condition);
		EXPECT_EQ(st.wYear, dlg._date.wYear);
		EXPECT_EQ(st.wMonth, dlg._date.wMonth);
		EXPECT_EQ(st.wDay, dlg._date.wDay);

		cond.setFindByMDate(st, ARCHIVE_FIND_CONDITION::COMPARE::equalOrLess);
		dlg.setCondition(cond);
		EXPECT_EQ((int)FIND_CONDITION::mdate_equal_or_older, dlg._condition);
		EXPECT_EQ(st.wYear, dlg._date.wYear);
		EXPECT_EQ(st.wMonth, dlg._date.wMonth);
		EXPECT_EQ(st.wDay, dlg._date.wDay);
	}

	{
		CLFFindDialog dlg;
		cond.setFindByMode(S_IFDIR);
		dlg.setCondition(cond);
		EXPECT_EQ((int)FIND_CONDITION::all_directories, dlg._condition);

		cond.setFindByMode(S_IFREG);
		dlg.setCondition(cond);
		EXPECT_EQ((int)FIND_CONDITION::all_files, dlg._condition);
	}
}

#endif

