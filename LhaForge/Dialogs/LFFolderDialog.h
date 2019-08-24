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

//WTL7.5のバグを回避するためのもの
//バグ:CFolderDialogのSetInitialFoder()で指定されたフォルダがBIF_NEWDIALOGSTYLEが指定されているとき開かれない
//(Windows2000だけの問題のようである)
//参照:http://sourceforge.net/tracker/index.php?func=detail&aid=1364046&group_id=109071&atid=652372
class CLFFolderDialog:public CFolderDialogImpl<CLFFolderDialog>
{
public:
	// コンストラクタ
	CLFFolderDialog(HWND hWndParent = NULL, LPCTSTR lpstrTitle = NULL,UINT uFlags = BIF_RETURNONLYFSDIRS)
		:CFolderDialogImpl<CLFFolderDialog>(hWndParent, lpstrTitle, uFlags)
	{}

	void OnInitialized(){
		SetSelection(m_lpstrInitialFolder);
	}
};
