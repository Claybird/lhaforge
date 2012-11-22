#include "stdafx.h"
#include "config.h"
#include "FileOperation.h"

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


	return bRet;
}

