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
#include "TemporaryDirMgr.h"
#include "../Utilities/FileOperation.h"

CTemporaryDirectoryManager::CTemporaryDirectoryManager(LPCTSTR lpszPrefix)
	:m_strPrefix(lpszPrefix)
{
	Prepare();
}

CTemporaryDirectoryManager::~CTemporaryDirectoryManager()
{
	Finish();
}

//一時出力フォルダを確保
void CTemporaryDirectoryManager::Prepare()
{
	Finish();
	//%TEMP%\%prefix%0000\filename...
	int Count=0;
	while(true){	//一時解凍先確保
		CPath Buffer=UtilGetTempPath();
		CString Name;
		Name.Format(_T("%s%d"),m_strPrefix,Count);
		Count++;

		//存在チェック
		Buffer.Append(Name);
//		TRACE(_T("%s\n"),Buffer);
		if(!PathFileExists(Buffer)){
			Buffer.AddBackslash();
			if(!PathIsDirectory(Buffer)){
				if(UtilMakeSureDirectoryPathExists(Buffer)){
					m_strDirPath=(LPCTSTR)Buffer;
					break;
				}
			}
		}
	}
}

bool CTemporaryDirectoryManager::Finish()
{
	//テンポラリディレクトリを削除(親ディレクトリも含めて)

	if(m_strDirPath.IsEmpty())return false;
	
	//フォルダ解放
	if(!UtilDeleteDir(m_strDirPath,true))return false;

	//フォルダ名クリア
	m_strDirPath.Empty();
	return true;
}

bool CTemporaryDirectoryManager::ClearSubDir()
{
	//テンポラリディレクトリの内容を削除(サブディレクトリ内容のみ)
	if(m_strDirPath.IsEmpty())return false;

	//中身の削除に失敗
	if(!UtilDeleteDir(m_strDirPath,false)){
		if(!PathIsDirectory(m_strDirPath)){
			//対象ディレクトリが存在していなければ、確保しておく
			if(UtilMakeSureDirectoryPathExists(m_strDirPath))return true;
		}
		return false;
	}
	return true;
}

LPCTSTR CTemporaryDirectoryManager::GetDirPath()
{
	if(m_strDirPath.IsEmpty())return NULL;
	return m_strDirPath;
}

bool CTemporaryDirectoryManager::DeleteAllTemporaryDir(LPCTSTR lpszPrefix)
{
	//%TEMP%\%prefix%0000\filename...
	CPath TempPath=UtilGetTempPath();
	{
		CString strTemp(lpszPrefix);
		strTemp+=_T("*");
		TempPath.Append(strTemp);
	}

	//数字
	const TCHAR DigitArray[]=_T("0123456789");
	const int DigitArrayLength=COUNTOF(DigitArray);

	//テンポラリディレクトリを探す
	CFindFile cFindFile;

	BOOL bContinue=cFindFile.FindFile(TempPath);

	bool bRet=true;
	while(bContinue){
		if(cFindFile.IsDirectory()){
			//prefixの後が数字のみなら削除
			CString strDirName(cFindFile.GetFileName());
			strDirName.Delete(0,_tcslen(lpszPrefix));
			if(!strDirName.IsEmpty()){
				for(int i=0;i<DigitArrayLength;i++){
					strDirName.Remove(DigitArray[i]);
				}

				//削除
				if(strDirName.IsEmpty()){
					bRet=bRet&&UtilDeleteDir(cFindFile.GetFilePath(),true);
				}
			}
		}
		bContinue=cFindFile.FindNextFile();
	}
	return bRet;
}
