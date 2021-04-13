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

#include "stdafx.h"
#include "Dlg_version.h"

//URLにリンクを設定
LRESULT CConfigDlgVersion::OnInitDialog(HWND hWnd, LPARAM lParam)
{
	// メッセージループにメッセージフィルタとアイドルハンドラを追加
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	pLoop->AddMessageFilter(this);

	const TCHAR URL_OF_CLAYBIRD[]=_T("https://claybird.sakura.ne.jp/");
	const TCHAR MAIL_OF_CLAYBIRD[]=_T("mailto:claybird.without.wing@gmail.com");

	link_url.SetHyperLinkExtendedStyle(HLINK_USETAGS);
	link_url.SubclassWindow(GetDlgItem(IDC_URL));
	link_url.SetHyperLink(URL_OF_CLAYBIRD);

	link_mailto.SetHyperLinkExtendedStyle(HLINK_USETAGS);
	link_mailto.SubclassWindow(GetDlgItem(IDC_MAILTO));
	link_mailto.SetHyperLink(MAIL_OF_CLAYBIRD);
	return TRUE;
}
