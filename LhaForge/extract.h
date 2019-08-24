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

enum DLL_ID;
class CConfigManager;
struct CConfigGeneral;
struct CConfigExtract;
enum OUTPUT_TO;
class CMDLINEINFO;
//解凍を行う
bool Extract(std::list<CString>&,CConfigManager&,DLL_ID,LPCTSTR lpSpecificOutputDir=NULL,const CMDLINEINFO* lpCmdLineInfo=NULL);
//出力先のディレクトリを取得し、カレントディレクトリにセットする。
HRESULT GetExtractDestDir(LPCTSTR,const CConfigGeneral&,const CConfigExtract&,LPCTSTR,bool,CPath&,const int,LPCTSTR,CPath&,bool &r_bUseForAll,CString &strErr);

const LPCTSTR LHAFORGE_EXTRACT_SEMAPHORE_NAME=_T("LhaForgeExtractLimitSemaphore");
