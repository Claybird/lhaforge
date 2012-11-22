#pragma once
//�ݒ�t�@�C������n

#include "variant.h"

//from http://ml.tietew.jp/cppll/cppll/article/12168
struct less_ignorecase {
	bool operator()(const stdString& a, const stdString& b) const {
		return _tcsicmp(a.c_str(), b.c_str()) < 0;
	}
};

//---�Z�N�V�����\���̂Ȃ��ݒ�;�f�[�^�́uKey=Value�v
typedef std::map<stdString,CVariant,less_ignorecase> FLATCONFIG;
//---�Z�N�V�������Ƃɕ����ꂽ�f�[�^
struct CONFIG_SECTION{	//�ݒ�t�@�C���̃Z�N�V�����̃f�[�^
	stdString SectionName;	//�Z�N�V�����̖��O
	FLATCONFIG Data;
};

//�ݒ�t�@�C���̓ǂݍ���
bool UtilReadSectionedConfig(LPCTSTR,std::list<CONFIG_SECTION>&,CString &strErr);

