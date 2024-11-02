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
#include "archive_rar.h"
#include "Utilities/Utility.h"
#undef WINVER	//to avoid compiler warning
#undef _WIN32_WINNT	//to avoid compiler warning
#define RARDLL
#include <unrar/rar.hpp>
#include <unrar/dll.hpp>

std::wstring rarErrMsg(int code)
{
	switch (code) {
	case ERAR_SUCCESS:
		return L"Success";
	case ERAR_END_ARCHIVE:
		return L"End archive";
	case ERAR_NO_MEMORY:
		return L"No memory";
	case ERAR_BAD_DATA:
		return L"Bad data";
	case ERAR_BAD_ARCHIVE:
		return L"Bad archive";
	case ERAR_UNKNOWN_FORMAT:
		return L"Unknown format";
	case ERAR_EOPEN:
		return L"Error opening file";
	case ERAR_ECREATE:
		return L"Error creating file";
	case ERAR_ECLOSE:
		return L"Error closing file";
	case ERAR_EREAD:
		return L"Error reading file";
	case ERAR_EWRITE:
		return L"Error writing file";
	case ERAR_SMALL_BUF:
		return L"Buffer too small";
	case ERAR_MISSING_PASSWORD:
		return L"Missing password";
	case ERAR_EREFERENCE:
		return L"Error reference";
	case ERAR_BAD_PASSWORD:
		return L"Bad password";
	case ERAR_UNKNOWN:
	default:
		return L"Unknown error";
	}
}

static std::wstring rarMethod(int method)
{
	switch (method) {
	case 0x30:
		return L"storing";
	case 0x31:
		return L"fastest";
	case 0x32:
		return L"fast";
	case 0x33:
		return L"normal";
	case 0x34:
		return L"good";
	case 0x35:
		return L"best";
	default:
		return L"unknown";
	}
}

std::filesystem::path rar_first_file(const std::filesystem::path& file)
{
	std::wregex multipart(L"\\.part\\d+\\.rar$", std::wregex::icase);
	std::wcmatch results;
	auto s = file.wstring();
	if (std::regex_search(s.c_str(), results, multipart)) {
		int64_t digits = results.length() - 9;	//partX/partXX/partXXX etc.
		auto part = Format(Format(L".part%%0%dd.rar", digits), 1);
		auto f = std::regex_replace(s.c_str(), multipart, part);
		return std::filesystem::path(f);
	}
	//fallback
	return file;
}

struct CLFArchiveRAR::INTERNAL
{
	std::filesystem::path path;
	std::shared_ptr<ILFPassphrase> passphrase_callback;
	std::function<void(const void*, size_t/*data size*/, const offset_info*)> data_receiver;

	LF_ENTRY_STAT _entry;
	HANDLE arc;

	bool bEntryRead;

	INTERNAL():arc(NULL), bEntryRead(false){}
	virtual ~INTERNAL() {
		close();
	}

	static int RarErrorToDll(RAR_EXIT ErrCode){
		switch (ErrCode) {
		case RARX_FATAL:
		case RARX_READ:
			return ERAR_EREAD;
		case RARX_CRC:
			return ERAR_BAD_DATA;
		case RARX_WRITE:
			return ERAR_EWRITE;
		case RARX_OPEN:
			return ERAR_EOPEN;
		case RARX_CREATE:
			return ERAR_ECREATE;
		case RARX_MEMORY:
			return ERAR_NO_MEMORY;
		case RARX_BADPWD:
			return ERAR_BAD_PASSWORD;
		case RARX_SUCCESS:
			return ERAR_SUCCESS; // 0.
		default:
			return ERAR_UNKNOWN;
		}
	}

	void close() {
		if (arc) {
			RARCloseArchive(arc);
		}
		arc = NULL;
	}
	void rewind() {
		close();
		RAROpenArchiveDataEx d = {};
		d.Callback = rar_event_handler;
		d.UserData = (LPARAM)this;

		d.ArcNameW = (wchar_t*)path.c_str();
		d.OpenMode = RAR_OM_EXTRACT;
		arc = RAROpenArchiveEx(&d);
		if (!arc) {
			throw LF_EXCEPTION(rarErrMsg(ERAR_EOPEN));
		}
		bEntryRead = false;
	}
	bool isOpened()const {
		return arc != NULL;
	}
	void open(const std::filesystem::path& file, std::shared_ptr<ILFPassphrase> passphrase) {
		passphrase_callback = passphrase;
		path = rar_first_file(file);
		rewind();
	}
	LF_ENTRY_STAT* scanNext() {
		if (!bEntryRead) {
			int ret = skipEntryData();
			if (ret != ERAR_SUCCESS) {
				throw LF_EXCEPTION(rarErrMsg(ret));
			}
		}
		bEntryRead = false;

		RARHeaderDataEx data = {};
		int ret = RARReadHeaderEx(arc, &data);
		if (ERAR_END_ARCHIVE == ret) {
			return nullptr;
		}else if (ret != ERAR_SUCCESS) {
			throw LF_EXCEPTION(rarErrMsg(ret));
		}
		_entry.compressed_size = data.PackSize + (((unsigned __int64)data.PackSizeHigh) << 32);
		_entry.stat.st_size = data.UnpSize + (((unsigned __int64)data.UnpSizeHigh) << 32);

		_entry.path = data.FileNameW;
		_entry.method_name = rarMethod(data.Method);

		_entry.stat.st_mtime = UtilFileTimeToUnixTime({ data.MtimeLow, data.MtimeHigh });
		_entry.stat.st_atime = UtilFileTimeToUnixTime({ data.AtimeLow, data.AtimeHigh });
		_entry.stat.st_ctime = UtilFileTimeToUnixTime({ data.CtimeLow, data.CtimeHigh });

		_entry.stat.st_mode = S_IFREG;
		if (data.Flags & RHDF_DIRECTORY) _entry.stat.st_mode = S_IFDIR;

		_entry.is_encrypted = data.Flags & RHDF_ENCRYPTED;

		return &_entry;
	}
	int skipEntryData() {
		if (bEntryRead) {
			RAISE_EXCEPTION(L"Entry is already read. Need to Proceed to next entry.");
		}
		bEntryRead = true;
		return RARProcessFileW(arc, RAR_SKIP, nullptr, nullptr);
	}
	int readEntryContent(std::function<void(const void*, size_t/*data size*/, const offset_info*)> receiver) {
		if (bEntryRead) {
			RAISE_EXCEPTION(L"Entry is already read. Need to Proceed to next entry.");
		}
		data_receiver = receiver;
		bEntryRead = true;
		return RARProcessFileW(arc, RAR_TEST, nullptr, nullptr);	//RAR_EXTRACT will generate actual file
	}

	static int CALLBACK rar_event_handler(UINT msg, LPARAM UserData, LPARAM P1, LPARAM P2) {
		INTERNAL* p = (INTERNAL*)UserData;
		switch (msg) {
		case UCM_CHANGEVOLUMEW:
			if (P2 == RAR_VOL_ASK) {
				//Need to ask user for new file path
				//TODO: not implemented
				return -1;
			}
			return 0;
		case UCM_NEEDPASSWORDW:
		if(p->passphrase_callback){
			auto cb = p->passphrase_callback.get();
			auto pwdA = cb->operator()();
			if (pwdA) {
				//got some password input
				wcsncpy_s((wchar_t*)P1, P2, cb->raw.c_str(), P2);
				return 0;
			} else {
				return -1;
			}
		} else {
			return -1;
		}
		case UCM_PROCESSDATA:
			p->data_receiver((const void*)P1, (size_t)P2, nullptr);
			return 0;

		//---implement wide version only
		case UCM_NEEDPASSWORD:
		case UCM_CHANGEVOLUME:
			return 0;
		}
		return 0;
	}
};

CLFArchiveRAR::CLFArchiveRAR()
{
	_internal = new INTERNAL;
}

CLFArchiveRAR::~CLFArchiveRAR()
{
	delete _internal;
}

std::filesystem::path CLFArchiveRAR::get_archive_path()const
{
	return _internal->path;
}

void CLFArchiveRAR::read_open(const std::filesystem::path& file, std::shared_ptr<ILFPassphrase> passphrase)
{
	_internal->open(file, passphrase);
}

void CLFArchiveRAR::close()
{
	_internal->close();
}

//entry seek; returns null if it reached EOF
LF_ENTRY_STAT* CLFArchiveRAR::read_entry_begin()
{
	_internal->rewind();
	return read_entry_next();
}

LF_ENTRY_STAT* CLFArchiveRAR::read_entry_next()
{
	return _internal->scanNext();
}

void CLFArchiveRAR::read_entry_end()
{
	//do nothing
}

//read entry
void CLFArchiveRAR::read_file_entry_block(std::function<void(const void*, size_t/*data size*/, const offset_info*)> data_receiver)
{
	int ret = _internal->readEntryContent(data_receiver);
	if (ret != ERAR_SUCCESS) {
		throw LF_EXCEPTION(rarErrMsg(ret));
	}
	data_receiver(nullptr, 0, nullptr);	//tell end of archive
}

#include "CommonUtil.h"
bool CLFArchiveRAR::is_known_format(const std::filesystem::path& arcname)
{
	//passphrase callback shoud be called if RAR header is encrypted.
	struct CLFPassphraseRunCheck :public ILFPassphrase {
		bool called;
		CLFPassphraseRunCheck() :called(false) {}
		virtual ~CLFPassphraseRunCheck() {}
		const char* operator()()override { called = true; return nullptr; }
	};

	auto passphrase = std::make_shared<CLFPassphraseRunCheck>();
	try {
		CLFArchiveRAR a;

		a.read_open(arcname, passphrase);
		return true;
	} catch (...) {
		return passphrase.get()->called;
	}
}


