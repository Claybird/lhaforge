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
#include "ArchiverManager.h"
#include "ConfigCode/ConfigManager.h"
#include "Utilities/FileOperation.h"
#include "Utilities/StringUtil.h"


//---------------------
CArchiverDLLManager::CArchiverDLLManager()
	:m_lpConfig(NULL)
{
	TRACE(_T("ArchiverDLLManager()\n"));
}

CArchiverDLLManager::~CArchiverDLLManager()
{
	TRACE(_T("~CArchiverDLLManager()\n"));
	Final();
}

void CArchiverDLLManager::Final()
{
	//全クリア
	TRACE(_T("Freeing dlls...\n"));

	Free();
}

void CArchiverDLLManager::Free()
{
}

CArchiverDLL* CArchiverDLLManager::GetArchiver(LPCTSTR ArcFileName,LPCTSTR lpDenyExt)
{
	return &Arc7ZIP;
/*	ASSERT(m_lpConfig);
	if(!m_lpConfig)return NULL;
	TRACE(_T("CArchiverDLLManager::GetArchiver(LPCTSTR)\n"));

	if(lpDenyExt && UtilExtMatchSpec(ArcFileName,lpDenyExt))return NULL;

	//拡張子から目星をつける
	DLL_ID Type=GuessDllIDFromFileName(ArcFileName,*m_lpConfig);
	CArchiverDLL* lpGuessedArchiver=GetArchiver(Type,true);
	TRACE(_T("推測済みアーカイバが正しいかどうかチェック\n"));
	if(lpGuessedArchiver){
		if(lpGuessedArchiver->IsOK()){
			bool bUnicodeOK=lpGuessedArchiver->IsUnicodeCapable();		//UNICODE未対応DLLなら、文字をチェックする
			if(bUnicodeOK||UtilCheckT2A(ArcFileName)){		//文字が現在のロケールで表現できるかどうか
				if(lpGuessedArchiver->CheckArchive(ArcFileName)){
					TRACE(_T("推定は当たっていた\n"));
					return lpGuessedArchiver;
				}
			}
		}
	}
	TRACE(_T("総当りによる推測を開始\n"));

	std::list<std::pair<CArchiverDLL*,DLL_ID> >::iterator ite;
	for(ite=ArchiverList.begin();ite!=ArchiverList.end();ite++){
		CArchiverDLL* lpArchiver=(*ite).first;

		ASSERT(lpArchiver);

		if(lpGuessedArchiver==lpArchiver)continue;	//既に適合しないことが分かっている
		if(!lpArchiver->IsOK()){
			CString strDummy;
			lpArchiver->LoadDLL(*m_lpConfig,strDummy);
		}
		if(lpArchiver->IsWeakCheckArchive()){
			continue;	//フォーマットチェックが甘いのであてにならない
		}
		if(!lpArchiver->IsUnicodeCapable()){
			//UNICODE未対応DLLなら、文字をチェックする
			if(!UtilCheckT2A(ArcFileName)){
				//現在のロケールで表現できない文字を使った
				continue;
			}
		}
		if(lpArchiver->IsOK()&&lpArchiver->CheckArchive(ArcFileName)){
			return lpArchiver;	//アーカイブハンドラ決定
		}
	}

	TRACE(_T("DLL決定に失敗\n"));
	return NULL;*/
}

CArchiverDLL* CArchiverDLLManager::GetArchiver(bool bSilent,bool bIgnoreError)
{
	return &Arc7ZIP;
/*	TRACE(_T("CArchiverDLLManager::GetArchiver(DLL_ID)\n"));
	ASSERT(m_lpConfig);
	if(!m_lpConfig)return NULL;

	CArchiverDLL* lpArchiver=NULL;
	std::list<std::pair<CArchiverDLL*,DLL_ID> >::iterator ite;
	for(ite=ArchiverList.begin();ite!=ArchiverList.end();ite++){
		if((*ite).second==Type){
			lpArchiver=(*ite).first;
			break;
		}
	}

	if(lpArchiver){
		//DLL有効/無効の判定
		if(!lpArchiver->IsOK()){
			CString strErr;
			if(LOAD_RESULT_OK!=lpArchiver->LoadDLL(*m_lpConfig,strErr) && !bSilent){
				ErrorMessage(strErr);
			}
		}
		if(bIgnoreError||lpArchiver->IsOK()){
			return lpArchiver;
		}
	}
	TRACE(_T("適切なDLLの取得に失敗\n"));
	return NULL;*/
}


