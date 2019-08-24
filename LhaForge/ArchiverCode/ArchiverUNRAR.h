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

#pragma once
#include "arc_interface.h"

class CArchiverUNRAR:public CArchiverDLL{
protected:
	COMMON_ARCHIVER_SETUNICODEMODE ArchiverSetUnicodeMode;
	bool m_bUTF8;
	CString m_strDllDisplayName;
protected:
	bool IsHeaderEncrypted(LPCTSTR);
	virtual void WriteResponceFile(HANDLE,LPCTSTR);;
public:
	CArchiverUNRAR();
	virtual ~CArchiverUNRAR();
	virtual LOAD_RESULT LoadDLL(CConfigManager&,CString &strErr)override;
	virtual void FreeDLL()override;
	virtual bool Compress(LPCTSTR,std::list<CString>&,CConfigManager&,const PARAMETER_TYPE,int,LPCTSTR,LPCTSTR,LPCTSTR,CString &)override;
	virtual bool Extract(LPCTSTR,CConfigManager&,const CConfigExtract&,bool,LPCTSTR,CString &)override;
	virtual bool ExtractSpecifiedOnly(LPCTSTR ArcFileName,CConfigManager&,LPCTSTR OutputDir,std::list<CString>&,CString &,bool bUsePath=false)override;
	virtual bool ExamineArchive(LPCTSTR,CConfigManager&,bool,bool&,bool&,CString&,CString &strErr)override;
	virtual ARCRESULT TestArchive(LPCTSTR,CString &)override;	//�A�[�J�C�u�����������ǂ����`�F�b�N����
	virtual LPCTSTR GetName()const override{return m_strDllDisplayName;}	//DLL����Ԃ�
	virtual bool IsUnicodeCapable()const{return true;}	//UNICODE�Ή�DLL�Ȃ�true��Ԃ�

	virtual BOOL CheckArchive(LPCTSTR)override;
	virtual int GetFileCount(LPCTSTR)override;	//�A�[�J�C�u���̃t�@�C������Ԃ�
	// ���ɓ������p���\�b�h
	virtual bool InspectArchiveBegin(LPCTSTR,CConfigManager&)override;				//���ɓ������J�n
	virtual bool InspectArchiveGetFileName(CString&)override;		//���ɓ��t�@�C�����擾
	virtual bool InspectArchiveGetMethodString(CString&)override;			//���ɓ��t�@�C���i�[���[�h�擾
};

