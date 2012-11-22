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

//LhaForge�p

//�f�o�b�O�p�֐�����ѕ֗��Ȋ֐��Q
#pragma once
//#pragma warning(disable:4786)

#if defined(_DEBUG) || defined(DEBUG)

void TraceLastError();

#else
// Release�̂Ƃ�
#define TraceLastError()

#endif	//_DEBUG

//=============================================
// ���ʕ֗��֐�
//=============================================

//�G���[���b�Z�[�W��\��
int ErrorMessage(LPCTSTR);
//���b�Z�[�W�L���v�V�������擾
LPCTSTR UtilGetMessageCaption();
void UtilGetLastErrorMessage(CString &strMsg);

#define BOOL2bool(x)	(FALSE!=x)

//�z��̒��Ɏw�肳�ꂽ�������L��΂��̈ʒu��Ԃ�;������Ȃ����-1��Ԃ�
int UtilCheckNumberArray(const int *lpcArray,int size,int c);

enum UTIL_CODEPAGE;

//���X�|���X�t�@�C����ǂݎ��
bool UtilReadFromResponceFile(LPCTSTR lpszRespFile,UTIL_CODEPAGE,std::list<CString> &FileList);

//INI�ɐ����𕶎���Ƃ��ď�������
BOOL UtilWritePrivateProfileInt(LPCTSTR lpAppName,LPCTSTR lpKeyName,LONG nData,LPCTSTR lpFileName);

//INI�Ɏw�肳�ꂽ�Z�N�V����������Ȃ�true��Ԃ�
bool UtilCheckINISectionExists(LPCTSTR lpAppName,LPCTSTR lpFileName);

//���������͂�����
bool UtilInputText(LPCTSTR lpszMessage,CString &strInput);

//�^����ꂽ�t�@�C�������}���`�{�����[�����ɂƌ��Ȃ���Ȃ猟����������쐬���Atrue��Ԃ�
bool UtilIsMultiVolume(LPCTSTR lpszPath,CString &r_strFindParam);

//�W���̐ݒ�t�@�C���̃p�X���擾
void UtilGetDefaultFilePath(CString &strPath,LPCTSTR lpszDir,LPCTSTR lpszFile,bool &bUserCommon);

//�t�@�C�������w�肵���p�^�[���ɓ��Ă͂܂��true
bool UtilExtMatchSpec(LPCTSTR lpszPath,LPCTSTR lpPattern);

//�t�@�C�������w�肵��2�̏�����[����]����邩�ǂ���;���ۂ��D��;bDenyOnly=true�Ȃ�ADeny�̃`�F�b�N�̂ݍs��
bool UtilPathAcceptSpec(LPCTSTR,LPCTSTR lpDeny,LPCTSTR lpAccept,bool bDenyOnly);

//�����I�Ƀ��b�Z�[�W���[�v����
bool UtilDoMessageLoop();
VOID CALLBACK UtilMessageLoopTimerProc(HWND,UINT,UINT,DWORD);

//�w�肳�ꂽmap���L�[�������Ă��邩�ǂ���
template <typename mapclass,typename keyclass>
bool has_key(const mapclass &theMap,keyclass theKey){
	return theMap.find(theKey)!=theMap.end();
}

//�w�肳�ꂽ�l���z�񒆂ɂ���΂��̃C���f�b�N�X��T��;�������-1
template <typename arrayclass,typename valueclass>
int index_of(const arrayclass &theArray,valueclass theValue){
	for(unsigned int i=0;i<theArray.size();++i){
		if(theArray[i]==theValue){
			return (signed)i;
		}
	}
	return -1;
}
//�R���e�i�̗v�f���폜����
template <typename arrayclass,typename valueclass>
void remove_item(arrayclass &theArray,const valueclass &theValue){
	theArray.erase(std::remove(theArray.begin(), theArray.end(), theValue), theArray.end());
}
