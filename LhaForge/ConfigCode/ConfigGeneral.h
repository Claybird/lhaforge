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

#pragma once

enum LOSTDIR;
enum LOGVIEW;
enum LFPROCESS_PRIORITY{
	LFPRIOTITY_DEFAULT=0,
	LFPRIOTITY_LOW=1,
	LFPRIOTITY_LOWER=2,
	LFPRIOTITY_NORMAL=3,
	LFPRIOTITY_HIGHER=4,
	LFPRIOTITY_HIGH=5,
	LFPRIOTITY_MAX_NUM=LFPRIOTITY_HIGH,
};

struct CConfigGeneral:public IConfigConverter{
public:
	struct tagFiler{
		virtual ~tagFiler(){}
		CString FilerPath;
		CString Param;
		BOOL UseFiler;
	}Filer;

	BOOL WarnNetwork;
	BOOL WarnRemovable;
	BOOL NotifyShellAfterProcess;	//SHChangeNotifyÇèàóùå„Ç…åƒÇ‘Ç»ÇÁtrue
	LOSTDIR OnDirNotFound;
	LOGVIEW LogViewEvent;
	int/*LFPROCESS_PRIORITY*/ ProcessPriority;
protected:
	virtual void load(CONFIG_SECTION&){ASSERT(!"This code cannot be run");}	//ê›íËÇCONFIG_SECTIONÇ©ÇÁì«Ç›çûÇﬁ
	virtual void store(CONFIG_SECTION&)const{ASSERT(!"This code cannot be run");}	//ê›íËÇCONFIG_SECTIONÇ…èëÇ´çûÇﬁ

	void loadOutput(CONFIG_SECTION&);
	void storeOutput(CONFIG_SECTION&)const;
	void loadFiler(CONFIG_SECTION&);
	void storeFiler(CONFIG_SECTION&)const;
	void loadLogView(CONFIG_SECTION&);
	void storeLogView(CONFIG_SECTION&)const;
	void loadGeneral(CONFIG_SECTION&);
	void storeGeneral(CONFIG_SECTION&)const;
public:
	virtual ~CConfigGeneral(){}
	virtual void load(CConfigManager&);
	virtual void store(CConfigManager&)const;
};

