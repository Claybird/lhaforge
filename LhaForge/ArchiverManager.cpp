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

//拡張子から解凍に使うDLLを推定
DLL_ID GuessDllIDFromFileName(LPCTSTR ArcFileName,CConfigManager &Config)
{
	TRACE(_T("GuessDllIDFromFileName()\n"));
	TRACE(_T("ArcFileName=%s\n"),ArcFileName);

	//拡張子抜きだし
	CString strExt=PathFindExtension(ArcFileName);
	strExt.MakeLower();

	//参照表から拡張子比較
	for(int i=0;i<ARRAY_EXT_DLLID_COUNT;i++){
		if(Array_ExtDLLID[i].lpszExt==strExt){
			return Array_ExtDLLID[i].DllID;
		}
	}
	return DLL_ID_UNKNOWN;
}

//圧縮タイプからDLLのIDを取得
DLL_ID GetDllIDFromParameterType(PARAMETER_TYPE Type)
{
	//参照表から合致するPARAMETER_TYPE取得
	for(int i=0;i<ARRAY_PARAMETERTYPE_DLLID_COUNT;i++){
		if(Array_ParameterTypeDLLID[i].ParameterType==Type){
			return Array_ParameterTypeDLLID[i].DllID;
		}
	}
	ASSERT(!"Out of range");
	return DLL_ID_UNKNOWN;
}


//---------------------
#define PAIR_DLL_ITEM(name)		std::pair<CArchiverDLL*,DLL_ID>(&Arc##name,DLL_ID_##name)	
CArchiverDLLManager::CArchiverDLLManager()
	:m_lpConfig(NULL)
{
	TRACE(_T("ArchiverDLLManager()\n"));
	ArchiverList.push_back(PAIR_DLL_ITEM(7ZIP));

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
	ArchiverList.clear();
}

void CArchiverDLLManager::Free()
{
	//全開放
	if(ArchiverList.empty())return;
	for(std::list<std::pair<CArchiverDLL*,DLL_ID> >::iterator ite=ArchiverList.begin();ite!=ArchiverList.end();ite++){
		(*ite).first->FreeDLL();
	}
}

CArchiverDLL* CArchiverDLLManager::GetArchiver(LPCTSTR ArcFileName,LPCTSTR lpDenyExt,DLL_ID ForceDLL)
{
	ASSERT(m_lpConfig);
	if(!m_lpConfig)return NULL;
	TRACE(_T("CArchiverDLLManager::GetArchiver(LPCTSTR)\n"));

	if(lpDenyExt && UtilExtMatchSpec(ArcFileName,lpDenyExt))return NULL;

	//-----使用するDLLを指定されている
	if(DLL_ID_UNKNOWN!=ForceDLL){
		CArchiverDLL* Archiver=GetArchiver(ForceDLL);	//ロードできないときにはエラーメッセージ
		if(Archiver){
			if(!Archiver->IsOK()){
				CString strDummy;
				Archiver->LoadDLL(*m_lpConfig,strDummy);
			}
			if(Archiver->IsOK()){
				if(!Archiver->IsUnicodeCapable()){		//UNICODE未対応DLLなら、文字をチェックする
					if(!UtilCheckT2A(ArcFileName)){
						//現在のロケールで表現できない文字を使った
						return NULL;
					}
				}
				if(Archiver->CheckArchive(ArcFileName)){
					return Archiver;
				}
			}
		}
		return NULL;
	}

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

		if(!m_ConfDLL.EnableDLL[(*ite).second]){
			continue;
		}

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
	return NULL;
}

CArchiverDLL* CArchiverDLLManager::GetArchiver(DLL_ID Type,bool bSilent,bool bIgnoreError)
{
	TRACE(_T("CArchiverDLLManager::GetArchiver(DLL_ID)\n"));
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
		if(!m_ConfDLL.EnableDLL[Type]){
			lpArchiver->FreeDLL();
			if(!bSilent){
				ErrorMessage(CString(MAKEINTRESOURCE(IDS_ERROR_DISABLED_DLL)));
			}
			return lpArchiver;
		}

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
	return NULL;
}

void CArchiverDLLManager::UpdateDLLConfig()
{
	if(m_lpConfig)m_ConfDLL.load(*m_lpConfig);
}

