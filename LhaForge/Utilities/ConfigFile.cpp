/*
 * Copyright (c) 2005-, Claybird
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
#include "ConfigFile.h"
#include "FileOperation.h"
#include "Utility.h"
#include "StringUtil.h"

//マップファイルなど設定ファイルの読み込み:Pythonのように辞書のリストでデータを返す
bool UtilReadSectionedConfig(LPCTSTR lpFile,std::list<CONFIG_SECTION> &r_Sections,CString &strErr)
{
	//セクションリストをクリア
	r_Sections.clear();

	FILELINECONTAINER flc;
	if(!UtilReadFileSplitted(lpFile,flc)){
		strErr=_T("Failed to read file \'");
		strErr+=lpFile;
		strErr+=_T("\'");
		return false;
	}

	//行のイテレータ
	CONFIG_SECTION tmpConf;		//バッファ
	for(UINT i=0;i<flc.lines.size();i++){
		if(flc.lines[i][0]==_T(';')){	//コメント
			continue;
		}
		const CString &str(flc.lines[i]);
		ASSERT(str.GetLength()!=0);
		if(str[0]==_T('[')){	//セクション開始
			if(!tmpConf.SectionName.empty()){	//名前が空の時は保存する必要なし:データなしと見なす
				//古いセクションを保存
				r_Sections.push_back(tmpConf);
			}
			//---前のデータを破棄
			tmpConf.SectionName.clear();
			tmpConf.Data.clear();

			//---セクション名取得
			int idx=str.Find(_T(']'));
			if(-1==idx){	//セクション名の記述が不完全
				strErr=_T("Incomplete section tag:");
				strErr+=str;
				return false;
			}else if(1==idx){	//セクション名が空
				strErr=_T("Empty section name");
				return false;
			}
			tmpConf.SectionName=stdString((LPCTSTR)str.Left(idx)+1);
		}else{	//要素
			int idx=str.Find(_T('='));	//区切りを探す
			if(-1==idx){
				strErr=_T("Invalid data item:");
				strErr+=str;
				return false;
			}
			//空白除去は行わない
			CString strKey=str.Left(idx);	//キー
			//データセット
			tmpConf.Data[(LPCTSTR)strKey]=(LPCTSTR)str+idx+1;
		}
	}
	//---後始末
	if(!tmpConf.SectionName.empty()){	//名前が空の時は保存する必要なし:データなしと見なす
		//古いセクションを保存
		r_Sections.push_back(tmpConf);
	}


	return true;
}

bool UtilWriteSection(HANDLE hFile,const CONFIG_SECTION& section)
{
	CString strSection;
	strSection.AppendFormat(_T("[%s]\r\n"),section.SectionName.c_str());
	for(FLATCONFIG::const_iterator ite=section.Data.begin();ite!=section.Data.end();++ite){
		strSection.AppendFormat(_T("%s=%s\r\n"),(*ite).first.c_str(),(*ite).second.getRawValue());
	}
	strSection+=_T("\r\n");

	DWORD dwWritten;
	DWORD toWrite=strSection.GetLength()*sizeof(TCHAR);
	if(!WriteFile(hFile,(LPCTSTR)strSection,toWrite,&dwWritten,NULL))return false;
	return (toWrite==dwWritten);
}

//設定ファイルの書き込み
bool UtilWriteSectionedConfig(LPCTSTR lpFile,const std::list<CONFIG_SECTION> &r_Sections,CString &strErr)
{
	HANDLE hFile=CreateFile(lpFile,GENERIC_WRITE,FILE_SHARE_READ,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
	if(INVALID_HANDLE_VALUE==hFile){
		UtilGetLastErrorMessage(strErr);
		return false;
	}

	//BOM書き込み
	/*const BYTE BOM[]={0xEF, 0xBB, 0xBF};	UTF-8*/
	const WCHAR BOM[]={0xFEFF};	//UTF-16
	DWORD dwWritten;
	DWORD toWrite=sizeof(BOM);
	if(!WriteFile(hFile,&BOM[0],toWrite,&dwWritten,NULL) || (toWrite!=dwWritten)){
		CloseHandle(hFile);
		UtilGetLastErrorMessage(strErr);
		return false;
	}

	bool bRet=true;
	for(std::list<CONFIG_SECTION>::const_iterator ite=r_Sections.begin();ite!=r_Sections.end();++ite){
		if(!UtilWriteSection(hFile,*ite)){
			UtilGetLastErrorMessage(strErr);
			bRet=false;
			break;
		}
	}

	CloseHandle(hFile);

	return bRet;
}


void UtilDumpFlatConfig(const FLATCONFIG &conf)
{
	for(FLATCONFIG::const_iterator ite=conf.begin();ite!=conf.end();++ite){
		OutputDebugString((*ite).first.c_str());
		OutputDebugString(_T("="));
		OutputDebugString((*ite).second.getRawValue());
		OutputDebugString(_T("\n"));
	}
}
