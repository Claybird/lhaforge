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

#pragma once
#include "stdafx.h"
#include "ArchiverCode/arc_interface.h"
#include "ArchiverCode/ArchiverUNLHA.h"
#include "ArchiverCode/ArchiverCAB.h"
#include "ArchiverCode/Archiver7ZIP.h"
#include "ArchiverCode/ArchiverTAR.h"
#include "ArchiverCode/ArchiverUNARJ.h"
#include "ArchiverCode/ArchiverUNRAR.h"
#include "ArchiverCode/ArchiverUNACE.h"
#include "ArchiverCode/ArchiverUNIMP.h"
#include "ArchiverCode/ArchiverJACK.h"
#include "ArchiverCode/ArchiverUNBEL.h"
#include "ArchiverCode/ArchiverUNHKI.h"
#include "ArchiverCode/ArchiverBGA.h"
#include "ArchiverCode/ArchiverUNISO.h"
#include "ArchiverCode/ArchiverAISH.h"

//DLL種類
enum DLL_ID{
	DLL_ID_UNKNOWN=-1,
	DLL_ID_UNLHA,
	DLL_ID_7ZIP,
	DLL_ID_CAB,
	DLL_ID_TAR,
	DLL_ID_JACK,
	DLL_ID_UNARJ,
	DLL_ID_UNRAR,
	DLL_ID_UNACE,
	DLL_ID_UNIMP,
	DLL_ID_UNBEL,
	DLL_ID_UNHKI,
	DLL_ID_BGA,
	DLL_ID_AISH,
	DLL_ID_UNISO,

	ENUM_COUNT_AND_LASTITEM(DLL_ID),
};

class CConfigManager;
#include "ConfigCode/ConfigDLL.h"

DLL_ID GuessDllIDFromFileName(LPCTSTR,CConfigManager&);
DLL_ID GetDllIDFromParameterType(PARAMETER_TYPE);

class CArchiverDLLManager{
protected:
	CArchiverTAR	ArcTAR;
	CArchiverJACK	ArcJACK;
	CArchiverUNLHA	ArcUNLHA;
	CArchiver7ZIP	Arc7ZIP;
	CArchiverCAB	ArcCAB;
	CArchiverUNARJ	ArcUNARJ;
	CArchiverUNRAR	ArcUNRAR;
	CArchiverUNACE	ArcUNACE;
	CArchiverUNIMP	ArcUNIMP;
	CArchiverUNBEL	ArcUNBEL;
	CArchiverUNHKI	ArcUNHKI;
	CArchiverBGA	ArcBGA;
	CArchiverAISH	ArcAISH;
	CArchiverUNISO	ArcUNISO;
	std::list<std::pair<CArchiverDLL*,DLL_ID> > ArchiverList;	//listもしくはvectorにしておくこと;pushの順番がアーカイブ形式推定の順番になる

	CConfigDLL m_ConfDLL;
	CConfigManager *m_lpConfig;
protected:
	CArchiverDLLManager();
public:
	virtual ~CArchiverDLLManager();

	static CArchiverDLLManager& GetInstance(){
		static CArchiverDLLManager Singleton;
		return Singleton;
	}

	CArchiverDLL* GetArchiver(LPCTSTR,LPCTSTR lpDenyExt,DLL_ID ForceDLL=DLL_ID_UNKNOWN);	//ForceDLLにDLL_IDを指定するとそのDLLを返す
	CArchiverDLL* GetArchiver(DLL_ID,bool bSilent=false,bool bIgnoreError=false);
	void Final();
	void Free();

	void SetConfigManager(CConfigManager &Config){m_lpConfig=&Config;UpdateDLLConfig();}
	void UpdateDLLConfig();
};

//---------------------
// 参照用のテーブル
//---------------------

//拡張子とDLL_IDの対応表
const struct ARRAYITEM_EXT_DLLID{
	LPCTSTR	lpszExt;
	DLL_ID	DllID;
}Array_ExtDLLID[]={
	{_T(".lzh"),	DLL_ID_UNLHA},
	{_T(".lha"),	DLL_ID_UNLHA},
	{_T(".lzs"),	DLL_ID_UNLHA},
	{_T(".cab"),	DLL_ID_CAB},
	{_T(".zip"),	DLL_ID_7ZIP},
	{_T(".7z"),		DLL_ID_7ZIP},
	{_T(".jar"),	DLL_ID_7ZIP},
	{_T(".tar"),	DLL_ID_TAR},
	{_T(".tgz"),	DLL_ID_TAR},
	{_T(".gz"),		DLL_ID_TAR},
	{_T(".tbz"),	DLL_ID_TAR},
	{_T(".bz2"),	DLL_ID_TAR},
	{_T(".xz"),		DLL_ID_TAR},
	{_T(".txz"),	DLL_ID_TAR},
	{_T(".lzma"),	DLL_ID_TAR},
	{_T(".tlz"),	DLL_ID_TAR},
	{_T(".tlzma"),	DLL_ID_TAR},

	{_T(".jak"),	DLL_ID_JACK},
	{_T(".hki"),	DLL_ID_UNHKI},
	{_T(".bza"),	DLL_ID_BGA},
	{_T(".gza"),	DLL_ID_BGA},

	{_T(".ish"),	DLL_ID_AISH},
	{_T(".uue"),	DLL_ID_AISH},
	//ISH関係で怪しそうなもの
	{_T(".doc"),	DLL_ID_AISH},
	{_T(".txt"),	DLL_ID_AISH},
	{_T(".log"),	DLL_ID_AISH},

//以下、TARによる解凍のみ可能な形式
	{_T(".z"),		DLL_ID_TAR},
	{_T(".taz"),	DLL_ID_TAR},
	{_T(".cpio"),	DLL_ID_TAR},
	{_T(".a"),		DLL_ID_TAR},
	{_T(".lib"),	DLL_ID_TAR},
	{_T(".rpm"),	DLL_ID_TAR},
	{_T(".deb"),	DLL_ID_TAR},
//ここからは解凍のみ対応の形式
	{_T(".arj"),	DLL_ID_UNARJ},
	{_T(".rar"),	DLL_ID_UNRAR},
	{_T(".ace"),	DLL_ID_UNACE},
	{_T(".imp"),	DLL_ID_UNIMP},
	{_T(".bel"),	DLL_ID_UNBEL},
	{_T(".iso"),	DLL_ID_UNISO},
};
const int ARRAY_EXT_DLLID_COUNT=COUNTOF(Array_ExtDLLID);


//PARAMETER_TYPEとDLL_IDの対応表
const struct ARRAYITEM_PARAMETERTYPE_DLLID{
	PARAMETER_TYPE	ParameterType;
	DLL_ID			DllID;
}Array_ParameterTypeDLLID[]={
	{PARAMETER_LZH,		DLL_ID_UNLHA},

	{PARAMETER_ZIP,		DLL_ID_7ZIP},
	{PARAMETER_7Z,		DLL_ID_7ZIP},

	{PARAMETER_CAB,		DLL_ID_CAB},

	{PARAMETER_JACK,	DLL_ID_JACK},

	{PARAMETER_HKI,		DLL_ID_UNHKI},

	{PARAMETER_BZA,		DLL_ID_BGA},
	{PARAMETER_GZA,		DLL_ID_BGA},

	{PARAMETER_ISH,		DLL_ID_AISH},
	{PARAMETER_UUE,		DLL_ID_AISH},

	{PARAMETER_TAR,		DLL_ID_TAR},
	{PARAMETER_BZ2,		DLL_ID_TAR},
	{PARAMETER_GZ,		DLL_ID_TAR},
	{PARAMETER_XZ,		DLL_ID_TAR},
	{PARAMETER_LZMA,	DLL_ID_TAR},
	{PARAMETER_TAR_GZ,	DLL_ID_TAR},
	{PARAMETER_TAR_BZ2,	DLL_ID_TAR},
	{PARAMETER_TAR_XZ,	DLL_ID_TAR},
	{PARAMETER_TAR_LZMA,DLL_ID_TAR},
};
const int ARRAY_PARAMETERTYPE_DLLID_COUNT=COUNTOF(Array_ParameterTypeDLLID);


