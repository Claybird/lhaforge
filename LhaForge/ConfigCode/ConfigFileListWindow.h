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
#include "FileListWindow/FileListModel.h"
#include "FileListWindow/MenuCommand.h"
#include "ConfigFile.h"

struct CConfigFileListWindow:public IConfigIO{
public:
	struct GENERAL {
		bool StoreSetting;
		bool ExitWithEscape;
		bool KeepSingleInstance;
		bool DisableTab;	//trie if disable tab
	}general;

	struct DIMENSIONS{
		int Width;
		int Height;
		int TreeWidth;

		int WindowPos_x;
		int WindowPos_y;
		bool StoreWindowPosition;
	}dimensions;

	struct VIEW {
		int SortColumnIndex;
		bool SortAtoZ;	//true if sort descending
		int ListStyle;

		bool DisplayFileSizeInByte;
		bool DisplayPathOnly;
		struct LISTVIEW_COLUMN {
			std::array<int, (int)FILEINFO_TYPE::ItemCount> order;
			std::array<int, (int)FILEINFO_TYPE::ItemCount> width;
		}column;

		bool ExpandTree;	//true to expand treeview on startup
		bool ShowTreeView;
		bool ShowToolbar;
		std::wstring strCustomToolbarImage;

		struct tagOpenAssoc {
			virtual ~tagOpenAssoc() {}
			std::wstring Accept;
			std::wstring Deny;
			bool DenyExecutables;	//true to deny opening files that match %PATHEXT%
		}OpenAssoc;

		std::vector<CLFMenuCommandItem> MenuCommandArray;	//Open with app
	}view;

protected:
	void loadMenuCommand(const CConfigFile&);
	void storeMenuCommand(CConfigFile&)const;

	void load_sub(const CConfigFile&);
	void store_sub(CConfigFile&)const;
public:
	virtual ~CConfigFileListWindow(){}
	virtual void load(const CConfigFile &Config) {
		load_sub(Config);
		loadMenuCommand(Config);
	}
	virtual void store(CConfigFile& Config)const {
		store_sub(Config);
		storeMenuCommand(Config);
	}

	//checks file extension whether file is allowed to be opened.
	bool isPathAcceptableToOpenAssoc(const std::filesystem::path &path, bool bDenyOnly)const;
};

