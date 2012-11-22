#pragma once
//設定ファイル操作系

#include "variant.h"

//from http://ml.tietew.jp/cppll/cppll/article/12168
struct less_ignorecase {
	bool operator()(const stdString& a, const stdString& b) const {
		return _tcsicmp(a.c_str(), b.c_str()) < 0;
	}
};

//---セクション構造のない設定;データは「Key=Value」
typedef std::map<stdString,CVariant,less_ignorecase> FLATCONFIG;
//---セクションごとに分かれたデータ
struct CONFIG_SECTION{	//設定ファイルのセクションのデータ
	stdString SectionName;	//セクションの名前
	FLATCONFIG Data;
};

//設定ファイルの読み込み
bool UtilReadSectionedConfig(LPCTSTR,std::list<CONFIG_SECTION>&,CString &strErr);

