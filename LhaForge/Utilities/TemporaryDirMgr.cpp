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

//�ꎞ�o�̓t�H���_���m��
void CTemporaryDirectoryManager::Prepare()
{
	Finish();
	//%TEMP%\%prefix%0000\filename...
	int Count=0;
	while(true){	//�ꎞ�𓀐�m��
		CPath Buffer=UtilGetTempPath();
		CString Name;
		Name.Format(_T("%s%d"),m_strPrefix,Count);
		Count++;

		//���݃`�F�b�N
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
	//�e���|�����f�B���N�g�����폜(�e�f�B���N�g�����܂߂�)

	if(m_strDirPath.IsEmpty())return false;
	
	//�t�H���_���
	if(!UtilDeleteDir(m_strDirPath,true))return false;

	//�t�H���_���N���A
	m_strDirPath.Empty();
	return true;
}

bool CTemporaryDirectoryManager::ClearSubDir()
{
	//�e���|�����f�B���N�g���̓��e���폜(�T�u�f�B���N�g�����e�̂�)
	if(m_strDirPath.IsEmpty())return false;

	//���g�̍폜�Ɏ��s
	if(!UtilDeleteDir(m_strDirPath,false)){
		if(!PathIsDirectory(m_strDirPath)){
			//�Ώۃf�B���N�g�������݂��Ă��Ȃ���΁A�m�ۂ��Ă���
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

	//����
	const TCHAR DigitArray[]=_T("0123456789");
	const int DigitArrayLength=COUNTOF(DigitArray);

	//�e���|�����f�B���N�g����T��
	CFindFile cFindFile;

	BOOL bContinue=cFindFile.FindFile(TempPath);

	bool bRet=true;
	while(bContinue){
		if(cFindFile.IsDirectory()){
			//prefix�̌オ�����݂̂Ȃ�폜
			CString strDirName(cFindFile.GetFileName());
			strDirName.Delete(0,_tcslen(lpszPrefix));
			if(!strDirName.IsEmpty()){
				for(int i=0;i<DigitArrayLength;i++){
					strDirName.Remove(DigitArray[i]);
				}

				//�폜
				if(strDirName.IsEmpty()){
					bRet=bRet&&UtilDeleteDir(cFindFile.GetFilePath(),true);
				}
			}
		}
		bContinue=cFindFile.FindNextFile();
	}
	return bRet;
}
