﻿/*
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
#include "resource.h"
#include "ConfigCode/configwnd.h"
#include "compress.h"
#include "extract.h"
#include "ArchiverCode/arc_interface.h"
#include "FileListWindow/FileListFrame.h"
#include "Dialogs/SelectDlg.h"
#include "Dialogs/ProgressDlg.h"
#include "Utilities/OSUtil.h"
#include "Utilities/StringUtil.h"
#include "CmdLineInfo.h"

#include "ConfigCode/ConfigOpenAction.h"
#include "ConfigCode/ConfigGeneral.h"

CAppModule _Module;


bool isArchive(const std::wstring& fname)
{
	ARCHIVE_FILE_TO_READ arc;
	try {
		arc.read_open(fname, nullptr);
		for (LF_ARCHIVE_ENTRY* entry = arc.begin(); entry; entry = arc.next()) {
			continue;
		}
		return true;
	} catch (const ARCHIVE_EXCEPTION& ) {
		return false;
	}
}

//enumerates files, removes directory
std::vector<std::wstring> enumerateFiles(const std::vector<std::wstring>& input, const std::vector<std::wstring>& denyExts)
{
	std::vector<std::wstring> out;
	for (const auto &item: input) {
		std::vector<std::wstring> children;
		if (std::filesystem::is_directory(item)) {
			children = UtilRecursiveEnumFile(item);
		} else {
			children = { item };
		}
		for (const auto &subItem : children) {
			bool bDenied = false;
			for (const auto& deny : denyExts) {
				if (UtilExtMatchSpec(subItem, deny)) {
					bDenied = true;
					break;
				}
			}
			//finally
			if (!bDenied) {
				out.push_back(subItem);
			}
		}
	}
	return out;
}


PROCESS_MODE selectOpenAction()
{
	class COpenActionDialog : public CDialogImpl<COpenActionDialog> {
	public:
		enum { IDD = IDD_DIALOG_OPENACTION_SELECT };
		BEGIN_MSG_MAP_EX(COpenActionDialog)
			COMMAND_ID_HANDLER_EX(IDC_BUTTON_OPENACTION_EXTRACT, OnButton)
			COMMAND_ID_HANDLER_EX(IDC_BUTTON_OPENACTION_LIST, OnButton)
			COMMAND_ID_HANDLER_EX(IDC_BUTTON_OPENACTION_TEST, OnButton)
			COMMAND_ID_HANDLER_EX(IDCANCEL, OnButton)
			END_MSG_MAP()

		void OnButton(UINT uNotifyCode, int nID, HWND hWndCtl) {
			EndDialog(nID);
		}
	};

	COpenActionDialog Dialog;
	switch (Dialog.DoModal()) {
	case IDC_BUTTON_OPENACTION_EXTRACT:
		return PROCESS_EXTRACT;
	case IDC_BUTTON_OPENACTION_LIST:
		return PROCESS_LIST;
	case IDC_BUTTON_OPENACTION_TEST:
		return PROCESS_TEST;
	default:
		return PROCESS_INVALID;
	}
}

//---------------------------------------------

bool DoCompress(CConfigManager &ConfigManager, CMDLINEINFO &cli)
{
	CConfigCompress ConfCompress;
	CConfigGeneral ConfGeneral;
	ConfCompress.load(ConfigManager);
	ConfGeneral.load(ConfigManager);

	while(LF_FMT_INVALID == cli.CompressType){
		if(ConfCompress.UseDefaultParameter){
			cli.CompressType = ConfCompress.DefaultType;
			cli.Options = ConfCompress.DefaultOptions;
		}else{	//not default parameter
			CSelectDialog SelDlg;
			SelDlg.SetDeleteAfterCompress(BOOL2bool(ConfCompress.DeleteAfterCompress));
			cli.CompressType = (LF_ARCHIVE_FORMAT)SelDlg.DoModal();
			if(LF_FMT_INVALID ==cli.CompressType){	//cancel
				return false;
			}else{
				cli.Options=SelDlg.GetOptions();
				cli.bSingleCompression=SelDlg.IsSingleCompression();
				if (SelDlg.GetDeleteAfterCompress()) {
					cli.DeleteAfterProcess = CMDLINEINFO::ACTION::True;
				} else {
					cli.DeleteAfterProcess = CMDLINEINFO::ACTION::False;
				}
				break;
			}
		}
	}

	//--------------------
	if(cli.bSingleCompression){
		//TODO:progress dialog should be handled as callbacks
		CProgressDialog dlg;
		int nFiles=cli.FileList.size();
		dlg.Create(nullptr);
		dlg.ShowWindow(SW_SHOW);
		bool bRet=true;
		int count = 0;
		for(const auto &filename: cli.FileList){
			if (dlg.IsWindow())dlg.SetProgress(filename, count, nFiles, L"*prepare*", 0, 0);
			while(UtilDoMessageLoop())continue;

			bool result = GUI_compress_multiple_files({filename}, cli.CompressType, (LF_WRITE_OPTIONS)cli.Options, cli);
			bRet = bRet && result;
			count += 1;
		}
		if(dlg.IsWindow())dlg.DestroyWindow();

		return bRet;
	}else{
		return GUI_compress_multiple_files(
			cli.FileList,
			cli.CompressType,
			(LF_WRITE_OPTIONS)cli.Options,
			cli);
	}
}

bool DoExtract(CConfigManager &ConfigManager,CMDLINEINFO &cli)
{
	CConfigExtract ConfExtract;
	ConfExtract.load(ConfigManager);
	const auto denyList = UtilSplitString(ConfExtract.DenyExt.operator LPCWSTR(), L";");

	auto tmp = enumerateFiles(cli.FileList, denyList);
	remove_item_if(tmp, [](const std::wstring& file) {return !isArchive(file); });

	if(tmp.empty()){
		ErrorMessage(UtilLoadString(IDS_ERROR_FILE_NOT_SPECIFIED));
		return false;
	}
	return GUI_extract_multiple_files(tmp, &cli);
}

bool DoList(CConfigManager &ConfigManager,CMDLINEINFO &cli)
{
	CConfigExtract ConfExtract;
	ConfExtract.load(ConfigManager);
	const auto denyList = UtilSplitString(ConfExtract.DenyExt.operator LPCWSTR(), L";");

	auto tmp = enumerateFiles(cli.FileList, denyList);
	remove_item_if(tmp, [](const std::wstring& file) {return !isArchive(file); });

	if(!cli.FileList.empty() && tmp.empty()){
		ErrorMessage(UtilLoadString(IDS_ERROR_FILE_NOT_SPECIFIED));
		return false;
	}

	CFileListFrame ListWindow(ConfigManager);
	ListWindow.CreateEx();
	ListWindow.ShowWindow(SW_SHOW);
	ListWindow.UpdateWindow();
	bool bAllFailed = !tmp.empty();
	for (const auto& item : tmp) {
		HRESULT hr = ListWindow.OpenArchiveFile(item.c_str());
		if (SUCCEEDED(hr)) {
			if (hr != S_FALSE)bAllFailed = false;
		} else if (hr == E_ABORT) {
			break;
		}
	}
	if(bAllFailed)ListWindow.DestroyWindow();

	CMessageLoop* pLoop = _Module.GetMessageLoop();
	pLoop->Run();
	return true;
}

bool DoTest(CConfigManager &ConfigManager,CMDLINEINFO &cli)
{
	CConfigExtract ConfExtract;
	ConfExtract.load(ConfigManager);
	const auto denyList = UtilSplitString(ConfExtract.DenyExt.operator LPCWSTR(), L";");

	auto tmp = enumerateFiles(cli.FileList, denyList);

	if(tmp.empty()){
		ErrorMessage(UtilLoadString(IDS_ERROR_FILE_NOT_SPECIFIED));
		return false;
	}

	return GUI_test_multiple_files(tmp, &cli);
}

void procMain()
{
	auto[ProcessMode, cli] = ParseCommandLine(GetCommandLineW(), ErrorMessage);
	if (PROCESS_INVALID == ProcessMode) {
		return;
	}

	CConfigManager ConfigManager;
	if (cli.ConfigPath.empty()) {
		ConfigManager.SetConfigFile(nullptr);
	} else {
		ConfigManager.SetConfigFile(cli.ConfigPath.c_str());
	}
	{
		CString strErr;
		//user specified, show message if error
		if (!ConfigManager.LoadConfig(strErr))ErrorMessage((const wchar_t*)strErr);
	}

	//key modifier
	{
		bool shift = GetKeyState(VK_SHIFT) < 0;
		bool control = GetKeyState(VK_CONTROL) < 0;

		switch (ProcessMode) {
		case PROCESS_COMPRESS:
			if (control) {
				//single compression if ctrl is pressed
				cli.bSingleCompression = true;
			}
			break;
		case PROCESS_EXTRACT:
			if (shift) {
				ProcessMode = PROCESS_LIST;	//list mode if shift is pressed
			} else if (control) {
				ProcessMode = PROCESS_TEST;	//test mode if ctrl is pressed
			}
			break;
		case PROCESS_MANAGED:
		{
			CConfigOpenAction ConfOpenAction;
			ConfOpenAction.load(ConfigManager);
			OPENACTION OpenAction;
			if (shift) {	//---when shift is pressed
				OpenAction = ConfOpenAction.OpenAction_Shift;
			} else if (control) {	//---when ctrl is pressed
				OpenAction = ConfOpenAction.OpenAction_Ctrl;
			} else {	//---default
				OpenAction = ConfOpenAction.OpenAction;
			}
			switch (OpenAction) {
			case OPENACTION_EXTRACT:
				ProcessMode = PROCESS_EXTRACT;
				break;
			case OPENACTION_LIST:
				ProcessMode = PROCESS_LIST;
				break;
			case OPENACTION_TEST:
				ProcessMode = PROCESS_TEST;
				break;
			case OPENACTION_ASK:
			default:
				ProcessMode = selectOpenAction();
				if (ProcessMode == PROCESS_INVALID) {
					return;
				}
				break;
			}
		}
		break;
		}
	}

	CConfigGeneral ConfGeneral;
	ConfGeneral.load(ConfigManager);
	{
		//process priority
		LFPROCESS_PRIORITY priority = (LFPROCESS_PRIORITY)ConfGeneral.ProcessPriority;
		//override
		if (cli.PriorityOverride != LFPRIOTITY_DEFAULT) {
			priority = cli.PriorityOverride;
		}
		switch (priority) {
		case LFPRIOTITY_LOW:
			UtilSetPriorityClass(IDLE_PRIORITY_CLASS); break;
		case LFPRIOTITY_LOWER:
			UtilSetPriorityClass(BELOW_NORMAL_PRIORITY_CLASS); break;
		case LFPRIOTITY_NORMAL:
			UtilSetPriorityClass(NORMAL_PRIORITY_CLASS); break;
		case LFPRIOTITY_HIGHER:
			UtilSetPriorityClass(ABOVE_NORMAL_PRIORITY_CLASS); break;
		case LFPRIOTITY_HIGH:
			UtilSetPriorityClass(HIGH_PRIORITY_CLASS); break;
		case LFPRIOTITY_DEFAULT:
		default:
			//nothing to do
			break;
		}
	}

	{
		//To use custom temporary directory, if necessary
		std::wstring strPath = ConfGeneral.TempPath;
		if (!strPath.empty()) {
			auto envInfo = LF_make_expand_information(nullptr, nullptr);
			strPath = UtilExpandTemplateString(strPath, envInfo);

			//get absolute path
			if (std::filesystem::path(strPath).is_relative()) {
				auto tmp = std::filesystem::path(UtilGetModuleDirectoryPath()) / strPath;
				strPath = tmp.lexically_normal();
			}
			try {
				auto buf = UtilGetCompletePathName(strPath);
				strPath = buf;
			} catch (LF_EXCEPTION) {
				//do nothing
			}

			//set environment
			SetEnvironmentVariableW(L"TEMP", strPath.c_str());
			SetEnvironmentVariableW(L"TMP", strPath.c_str());
		}
	}

	switch (ProcessMode) {
	case PROCESS_COMPRESS:
		DoCompress(ConfigManager, cli);
		break;
	case PROCESS_EXTRACT:
		DoExtract(ConfigManager, cli);
		break;
	case PROCESS_AUTOMATIC:
		if (std::filesystem::is_directory(cli.FileList.front())) {
			DoCompress(ConfigManager, cli);
		} else {
			CConfigExtract ConfExtract;
			ConfExtract.load(ConfigManager);
			bool isDenied = ConfExtract.DenyExt.MakeLower().Find(toLower(std::filesystem::path(cli.FileList.front()).extension()).c_str()) == -1;
			if (!isDenied && isArchive(cli.FileList.front())) {
				DoExtract(ConfigManager, cli);
			} else {
				DoCompress(ConfigManager, cli);
			}
		}
		break;
	case PROCESS_LIST:
		DoList(ConfigManager, cli);
		break;
	case PROCESS_TEST:
		DoTest(ConfigManager, cli);
		break;
	case PROCESS_CONFIGURE:
	default:
	{
		CConfigDialog confdlg(ConfigManager);
		if (IDOK == confdlg.DoModal()) {
			CString strErr;
			if (!ConfigManager.SaveConfig(strErr)) {
				ErrorMessage(strErr.operator LPCWSTR());
			}
		}
		break;
	}
	}
}



int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR, int nCmdShow)
{
#if defined(_DEBUG)
	// detect memory leaks
	_CrtSetDbgFlag(
		_CRTDBG_ALLOC_MEM_DF
		| _CRTDBG_LEAK_CHECK_DF
	);
#endif
	_wsetlocale(LC_ALL, L"");	//default locale

	HRESULT hRes = ::CoInitialize(nullptr);
	ATLASSERT(SUCCEEDED(hRes));
	OleInitialize(nullptr);

	// support control flags
	AtlInitCommonControls(ICC_WIN95_CLASSES | ICC_COOL_CLASSES | ICC_BAR_CLASSES);
	_Module.Init(nullptr, hInstance);
	CMessageLoop theLoop;
	_Module.AddMessageLoop(&theLoop);

	procMain();

	_Module.RemoveMessageLoop();
	_Module.Term();
	OleUninitialize();
	::CoUninitialize();
	return 0;
}


#ifdef UNIT_TEST
#include <gtest/gtest.h>
int wmain(int argc, wchar_t *argv[], wchar_t *envp[]){
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
#endif
