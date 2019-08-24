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
#include "../Association.h"

class ASSOC_SETTINGS{
protected:
	CIcon		Icon;
public:
	ASSOC_SETTINGS();
	virtual ~ASSOC_SETTINGS();

	int DefaultIconIndex;	//標準アイコンのインデックス
	int DefaultIconIndex_Ex;	//標準アイコンのインデックス(TAR関係のアイコンも全て区別する場合)
	CStatic		Picture_Icon;
	CButton		Button_SetIcon;
	CButton		Check_SetAssoc;
	void SetIconFromAssoc(CIcon &IconSystemDefault);
	void SetIcon(CIcon &icon);
	void SetIcon(LPCTSTR path,int idx);
	ASSOCINFO	AssocInfo;	//関連付け情報
	bool		bChanged;	//関連付けレジストリに変更があったならtrue
	bool CheckAssociation(LPCTSTR);
};
