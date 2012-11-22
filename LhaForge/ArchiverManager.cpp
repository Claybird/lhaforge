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
#include "ArchiverManager.h"
#include "ConfigCode/ConfigManager.h"
#include "Utilities/FileOperation.h"
#include "Utilities/StringUtil.h"
#include "ConfigCode/ConfigB2E.h"
#include "ConfigCode/ConfigXACRETT.h"

//拡張子から解凍に使うDLLを推定
DLL_ID GuessDllIDFromFileName(LPCTSTR ArcFileName,CConfigManager &Config)
{
	TRACE(_T("GuessDllIDFromFileName()\n"));
	TRACE(_T("ArcFileName=%s\n"),ArcFileName);

	//---B2EをXacRettより優先する場合
	CConfigB2E ConfB2E;
	CConfigXACRETT ConfXACRETT;
	ConfB2E.load(Config);
	ConfXACRETT.load(Config);
	if(ConfB2E.EnableDLL && ConfB2E.Priority){
		//B2Eに優先する拡張子の確認
		if(UtilPathAcceptSpec(ArcFileName,CString(),ConfB2E.Extensions,false))return DLL_ID_B2E;

		//XacRettに優先する拡張子の確認
		if(ConfXACRETT.EnableDLL){
			if(UtilPathAcceptSpec(ArcFileName,CString(),ConfXACRETT.Extensions,false))return DLL_ID_XACRETT;
		}
	}else{
		//---XacRettがB2Eより優先
		//XacRettに優先する拡張子の確認
		if(ConfXACRETT.EnableDLL){
			if(UtilPathAcceptSpec(ArcFileName,CString(),ConfXACRETT.Extensions,false))return DLL_ID_XACRETT;
		}
		//B2Eに優先する拡張子の確認
		if(ConfB2E.EnableDLL){
			if(UtilPathAcceptSpec(ArcFileName,CString(),ConfB2E.Extensions,false))return DLL_ID_B2E;
		}
	}

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
	ArchiverList.push_back(PAIR_DLL_ITEM(TAR));
	ArchiverList.push_back(PAIR_DLL_ITEM(JACK));
	ArchiverList.push_back(PAIR_DLL_ITEM(UNLHA));
	ArchiverList.push_back(PAIR_DLL_ITEM(7ZIP));
	ArchiverList.push_back(PAIR_DLL_ITEM(CAB));
	ArchiverList.push_back(PAIR_DLL_ITEM(UNARJ));
	ArchiverList.push_back(PAIR_DLL_ITEM(UNGCA));
	ArchiverList.push_back(PAIR_DLL_ITEM(UNRAR));
	ArchiverList.push_back(PAIR_DLL_ITEM(UNACE));
	ArchiverList.push_back(PAIR_DLL_ITEM(UNIMP));
	ArchiverList.push_back(PAIR_DLL_ITEM(YZ1));
	ArchiverList.push_back(PAIR_DLL_ITEM(UNHKI));
	ArchiverList.push_back(PAIR_DLL_ITEM(BGA));
	ArchiverList.push_back(PAIR_DLL_ITEM(AISH));
	ArchiverList.push_back(PAIR_DLL_ITEM(UNBEL));
	ArchiverList.push_back(PAIR_DLL_ITEM(XACRETT));
	ArchiverList.push_back(PAIR_DLL_ITEM(B2E));
	ArchiverList.push_back(PAIR_DLL_ITEM(UNISO));

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

	CConfigB2E ConfB2E;
	CConfigXACRETT ConfXACRETT;
	ConfB2E.load(*m_lpConfig);
	ConfXACRETT.load(*m_lpConfig);

	std::list<std::pair<CArchiverDLL*,DLL_ID> >::iterator ite;
	for(ite=ArchiverList.begin();ite!=ArchiverList.end();ite++){
		CArchiverDLL* lpArchiver=(*ite).first;

		ASSERT(lpArchiver);

		if(&ArcXACRETT==lpArchiver){	//XacRettの使用が許可されている場合のみロード
			if(!ConfXACRETT.EnableDLL)continue;
		}else if(&ArcB2E==lpArchiver){	//B2Eの使用が許可されている場合のみロード
			if(!ConfB2E.EnableDLL)continue;
		}else if(!m_ConfDLL.EnableDLL[(*ite).second]){
			continue;
		}

		if(ConfB2E.Priority){
			//B2E優先の時は、このループではXacRettを処理させない
			if(&ArcXACRETT==lpArchiver)continue;
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

	if(ConfB2E.Priority){
		//B2E優先の時は、後からXacRettを処理
		if(ConfXACRETT.EnableDLL){
			CArchiverDLL* lpArchiver=&ArcXACRETT;
			if(!lpArchiver->IsOK()){
				CString strDummy;
				lpArchiver->LoadDLL(*m_lpConfig,strDummy);
			}
			if(lpArchiver->IsUnicodeCapable() || UtilCheckT2A(ArcFileName)){
				if(lpArchiver->IsOK()&&lpArchiver->CheckArchive(ArcFileName)){
					return lpArchiver;	//アーカイブハンドラ決定
				}
			}
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
		if(Type!=DLL_ID_XACRETT && Type!=DLL_ID_B2E && !m_ConfDLL.EnableDLL[Type]){
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

CArchiverB2E& CArchiverDLLManager::GetB2EHandler()
{
	if(!ArcB2E.IsOK()){
		CString strDummy;
		ArcB2E.LoadDLL(*m_lpConfig,strDummy);
	}
	return ArcB2E;
}

void CArchiverDLLManager::UpdateDLLConfig()
{
	if(m_lpConfig)m_ConfDLL.load(*m_lpConfig);
}

