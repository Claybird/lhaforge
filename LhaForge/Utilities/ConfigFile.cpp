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
		strErr = UtilGetLastErrorMessage().c_str();
		return false;
	}

	//BOM書き込み
	/*const BYTE BOM[]={0xEF, 0xBB, 0xBF};	UTF-8*/
	const WCHAR BOM[]={0xFEFF};	//UTF-16
	DWORD dwWritten;
	DWORD toWrite=sizeof(BOM);
	if(!WriteFile(hFile,&BOM[0],toWrite,&dwWritten,NULL) || (toWrite!=dwWritten)){
		CloseHandle(hFile);
		strErr = UtilGetLastErrorMessage().c_str();
		return false;
	}

	bool bRet=true;
	for(std::list<CONFIG_SECTION>::const_iterator ite=r_Sections.begin();ite!=r_Sections.end();++ite){
		if(!UtilWriteSection(hFile,*ite)){
			strErr = UtilGetLastErrorMessage().c_str();
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


//INIに数字を文字列として書き込む
BOOL UtilWritePrivateProfileInt(LPCTSTR lpAppName, LPCTSTR lpKeyName, LONG nData, LPCTSTR lpFileName)
{
	TCHAR Buffer[32] = { 0 };
	wsprintf(Buffer, _T("%ld"), nData);
	return ::WritePrivateProfileString(lpAppName, lpKeyName, Buffer, lpFileName);
}

