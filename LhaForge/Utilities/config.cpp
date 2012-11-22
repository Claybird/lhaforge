#include "stdafx.h"
#include "config.h"
#include "FileOperation.h"

//�}�b�v�t�@�C���Ȃǐݒ�t�@�C���̓ǂݍ���:Python�̂悤�Ɏ����̃��X�g�Ńf�[�^��Ԃ�
bool UtilReadSectionedConfig(LPCTSTR lpFile,std::list<CONFIG_SECTION> &r_Sections,CString &strErr)
{
	//�Z�N�V�������X�g���N���A
	r_Sections.clear();

	FILELINECONTAINER flc;
	if(!UtilReadFileSplitted(lpFile,flc)){
		strErr=_T("Failed to read file \'");
		strErr+=lpFile;
		strErr+=_T("\'");
		return false;
	}

	//�s�̃C�e���[�^
	CONFIG_SECTION tmpConf;		//�o�b�t�@
	for(UINT i=0;i<flc.lines.size();i++){
		if(flc.lines[i][0]==_T(';')){	//�R�����g
			continue;
		}
		const CString &str(flc.lines[i]);
		ASSERT(str.GetLength()!=0);
		if(str[0]==_T('[')){	//�Z�N�V�����J�n
			if(!tmpConf.SectionName.empty()){	//���O����̎��͕ۑ�����K�v�Ȃ�:�f�[�^�Ȃ��ƌ��Ȃ�
				//�Â��Z�N�V������ۑ�
				r_Sections.push_back(tmpConf);
			}
			//---�O�̃f�[�^��j��
			tmpConf.SectionName.clear();
			tmpConf.Data.clear();

			//---�Z�N�V�������擾
			int idx=str.Find(_T(']'));
			if(-1==idx){	//�Z�N�V�������̋L�q���s���S
				strErr=_T("Incomplete section tag:");
				strErr+=str;
				return false;
			}else if(1==idx){	//�Z�N�V����������
				strErr=_T("Empty section name");
				return false;
			}
			tmpConf.SectionName=stdString((LPCTSTR)str.Left(idx)+1);
		}else{	//�v�f
			int idx=str.Find(_T('='));	//��؂��T��
			if(-1==idx){
				strErr=_T("Invalid data item:");
				strErr+=str;
				return false;
			}
			//�󔒏����͍s��Ȃ�
			CString strKey=str.Left(idx);	//�L�[
			//�f�[�^�Z�b�g
			tmpConf.Data[(LPCTSTR)strKey]=(LPCTSTR)str+idx+1;
		}
	}
	//---��n��
	if(!tmpConf.SectionName.empty()){	//���O����̎��͕ۑ�����K�v�Ȃ�:�f�[�^�Ȃ��ƌ��Ȃ�
		//�Â��Z�N�V������ۑ�
		r_Sections.push_back(tmpConf);
	}


	return bRet;
}

