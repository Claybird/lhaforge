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
#include "arc_interface.h"
#include "../Utilities/FileOperation.h"
#include "../Utilities/OSUtil.h"

CArchiverDLL::CArchiverDLL():
	m_bInspectFirstTime(false)
{
}

//TODO:remove
WORD CArchiverDLL::GetVersion()const
{
	return 0;
}

//TODO:remove
WORD CArchiverDLL::GetSubVersion()const
{
	return 0;
}

//TODO:remove
bool CArchiverDLL::GetVersionString(CString &String)const
{
	return false;
}

//TODO:update
BOOL CArchiverDLL::CheckArchive(LPCTSTR _szFileName)
{
	return true;
}


//TODO:remove
void CArchiverDLL::FreeDLL()
{
}

//TODO:remove
LOAD_RESULT CArchiverDLL::LoadDLL(CConfigManager&,CString &strErr)
{
	return LOAD_RESULT_OK;
}


//TODO:update: in-memory extract
ARCRESULT CArchiverDLL::TestArchive(LPCTSTR lpszFile,CString &strMsg)
{
	return TEST_NOTIMPL;
}

//TODO:update
bool CArchiverDLL::ExtractItems(LPCTSTR lpszArcFile,CConfigManager &ConfMan,const ARCHIVE_ENTRY_INFO_TREE *lpBase, const std::list<ARCHIVE_ENTRY_INFO_TREE *> &items, LPCTSTR lpszOutputBaseDir, bool bCollapseDir, CString &strLog)
{
	//アーカイブの存在は既に確認済みとする
	//文字コード関係は既に確認済みとする
	return false;
}

