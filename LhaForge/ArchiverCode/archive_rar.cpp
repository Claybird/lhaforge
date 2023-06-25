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
#undef WINVER	//to avoid compiler warning
#undef _WIN32_WINNT	//to avoid compiler warning
#define RARDLL
#include <unrar/rar.hpp>
#include <unrar/dll.hpp>
#include "Utilities/Utility.h"

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

std::wstring rarMethod(int method)
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

struct CLFArchiveRAR::INTERNAL
{
protected:
	std::filesystem::path path;
	std::shared_ptr<ILFPassphrase> passphrase_callback;
	std::function<void(const void*, size_t/*data size*/, const offset_info*)> data_receiver;
	LF_ENTRY_STAT _entry;
	CommandData Cmd;
	Archive Arc;
	CmdExtract Extract;
	int HeaderSize;

	static void null_receiver(const void*, size_t/*data size*/, const offset_info*) {}
	static int RarErrorToDll(RAR_EXIT ErrCode) {
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
	void seekNextHeader() {	//original: ProcessFile
		try {
			Cmd.DllError = 0;
			int Operation = RAR_SKIP;
			if (Arc.Solid) {
				Cmd.DllOpMode = Operation;

				*Cmd.ExtrPath = 0;
				*Cmd.DllDestName = 0;

				Cmd.Test = false;
				bool Repeat = false;
				Extract.ExtractCurrentFile(Arc, HeaderSize, Repeat);

				// Now we process extra file information if any.
				//
				// Archive can be closed if we process volumes, next volume is missing
				// and current one is already removed or deleted. So we need to check
				// if archive is still open to avoid calling file operations on
				// the invalid file handle. Some of our file operations like Seek()
				// process such invalid handle correctly, some not.
				while (Arc.IsOpened() && Arc.ReadHeader() != 0 &&
					Arc.GetHeaderType() == HEAD_SERVICE) {
					Extract.ExtractCurrentFile(Arc, HeaderSize, Repeat);
					Arc.SeekToNext();
				}
				Arc.Seek(Arc.CurBlockPos, SEEK_SET);
			} else {
				if (Arc.Volume && Arc.GetHeaderType() == HEAD_FILE && Arc.FileHead.SplitAfter) {
					if (MergeArchive(Arc, NULL, false, 'L')) {
						Arc.Seek(Arc.CurBlockPos, SEEK_SET);
						//return ERAR_SUCCESS;
					} else {
						throw LF_EXCEPTION(rarErrMsg(ERAR_EOPEN));
					}
				} else {
					Arc.SeekToNext();
				}
			}
		} catch (std::bad_alloc&) {
			throw LF_EXCEPTION(rarErrMsg(ERAR_NO_MEMORY));
		} catch (RAR_EXIT ErrCode) {
			throw LF_EXCEPTION(rarErrMsg(RarErrorToDll(ErrCode)));
		}
	}
	LF_ENTRY_STAT* readHeader() {	//original: RARReadHeaderEx
		try {
			if ((HeaderSize = (int)Arc.SearchBlock(HEAD_FILE)) <= 0) {
				if (Arc.Volume && Arc.GetHeaderType() == HEAD_ENDARC &&
					Arc.EndArcHead.NextVolume) {
					if (MergeArchive(Arc, NULL, false, 'L')) {
						Arc.Seek(Arc.CurBlockPos, SEEK_SET);
						return readHeader();
					} else {
						throw LF_EXCEPTION(rarErrMsg(ERAR_EOPEN));
					}
				}

				if (Arc.BrokenHeader) {
					throw LF_EXCEPTION(rarErrMsg(ERAR_BAD_DATA));
				}

				// Might be necessary if RARSetPassword is still called instead of
				// open callback for RAR5 archives and if password is invalid.
				if (Arc.FailedHeaderDecryption) {
					throw LF_EXCEPTION(rarErrMsg(ERAR_BAD_PASSWORD));
				}
				return nullptr;
			}
			FileHeader* hd = &Arc.FileHead;

			_entry.path = hd->FileName;
			_entry.compressed_size = hd->PackSize;
			_entry.method_name = rarMethod(hd->Method + 0x30);

			_entry.stat.st_mtime = hd->mtime.GetUnix();
			_entry.stat.st_atime = hd->atime.GetUnix();
			_entry.stat.st_ctime = hd->ctime.GetUnix();
			_entry.stat.st_size = hd->UnpSize;

			_entry.stat.st_mode = S_IFREG;
			if (hd->Dir) _entry.stat.st_mode = S_IFDIR;
			_entry.is_encrypted = hd->Encrypted;
			return &_entry;
		} catch (RAR_EXIT ErrCode) {
			throw LF_EXCEPTION(rarErrMsg(RarErrorToDll(ErrCode)));
		}
	}


public:
	INTERNAL():Arc(&Cmd), Extract(&Cmd) {}
	virtual ~INTERNAL() {
		close();
	}

	bool isOpened() {
		return Arc.IsOpened();
	}
	void close() {
			data_receiver = null_receiver;
		Arc.Close();
	}
	void open(const std::filesystem::path& file, std::shared_ptr<ILFPassphrase> passphrase) {
		passphrase_callback = passphrase;
		data_receiver = null_receiver;
		path = file;

		rewind();
	}
	void rewind() {
		close();
		HeaderSize = 0;

		//original: RAROpenArchiveEx
		ErrHandler.Clean();
		try {
			Cmd.DllError = 0;
			Cmd.FileArgs.AddString(L"*");
			Cmd.KeepBroken = false;

			Cmd.AddArcName(path.c_str());
			Cmd.Overwrite = OVERWRITE_ALL;
			Cmd.VersionControl = 1;

			Cmd.Callback = rar_event_handler;
			Cmd.UserData = (LPARAM)this;

			// Open shared mode is added by request of dll users, who need to
			// browse and unpack archives while downloading.
			Cmd.OpenShared = true;
			if (!Arc.Open(path.c_str(), FMF_OPENSHARED)) {
				throw LF_EXCEPTION(rarErrMsg(ERAR_EOPEN));
			}
			if (!Arc.IsArchive(true)) {
				RAR_EXIT ErrCode = ErrHandler.GetErrorCode();
				if (ErrCode != RARX_SUCCESS && ErrCode != RARX_WARNING) {
					throw LF_EXCEPTION(rarErrMsg(RarErrorToDll(ErrCode)));
				} else {
					throw LF_EXCEPTION(rarErrMsg(ERAR_BAD_ARCHIVE));
				}
			}

			Extract.ExtractArchiveInit(Arc);
		} catch (RAR_EXIT ErrCode) {
			throw LF_EXCEPTION(rarErrMsg(RarErrorToDll(ErrCode)));
		} catch (std::bad_alloc&) {
			throw LF_EXCEPTION(rarErrMsg(ERAR_NO_MEMORY));
		}
	}
	LF_ENTRY_STAT* scanNext() {
		seekNextHeader();
		return readHeader();
	}
	void readEntryContent(std::function<void(const void*, size_t/*data size*/, const offset_info*)> receiver) {	//original::ProcessFile
		data_receiver = receiver;
		try {
			bool Repeat = false;
			Extract.ExtractCurrentFile(Arc, HeaderSize, Repeat);

			// Now we process extra file information if any.
			//
			// Archive can be closed if we process volumes, next volume is missing
			// and current one is already removed or deleted. So we need to check
			// if archive is still open to avoid calling file operations on
			// the invalid file handle. Some of our file operations like Seek()
			// process such invalid handle correctly, some not.
			while (Arc.IsOpened() && Arc.ReadHeader() != 0 &&
				Arc.GetHeaderType() == HEAD_SERVICE) {
				Extract.ExtractCurrentFile(Arc, HeaderSize, Repeat);
				Arc.SeekToNext();
			}
			Arc.Seek(Arc.CurBlockPos, SEEK_SET);
			data_receiver = null_receiver;
		} catch (std::bad_alloc&) {
			data_receiver = null_receiver;
			throw LF_EXCEPTION(rarErrMsg(ERAR_NO_MEMORY));
		} catch (RAR_EXIT ErrCode) {
			data_receiver = null_receiver;
			throw LF_EXCEPTION(rarErrMsg(RarErrorToDll(ErrCode)));
		}
	}

	//---
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

void CLFArchiveRAR::read_open(const std::filesystem::path& file, std::shared_ptr<ILFPassphrase> passphrase)
{
	_internal->open(file, passphrase);
}

void CLFArchiveRAR::close()
{
	_internal->close();
}

//archive property
std::wstring CLFArchiveRAR::get_format_name()
{
	return L"RAR";
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
	_internal->readEntryContent(data_receiver);
}

#include "CommonUtil.h"
bool CLFArchiveRAR::is_known_format(const std::filesystem::path& arcname)
{
	try {
		CLFArchiveRAR a;
		a.read_open(arcname, std::make_shared<CLFPassphraseNULL>());
		return true;
	} catch (...) {
		return false;
	}
}



#ifdef UNIT_TEST
TEST(CLFArchiveRAR, is_known_format)
{
	{
		const auto dir = LF_PROJECT_DIR() / L"ArchiverCode/test";
		EXPECT_FALSE(CLFArchiveRAR::is_known_format(dir / L"empty.gz"));
		EXPECT_FALSE(CLFArchiveRAR::is_known_format(dir / L"empty.bz2"));
		EXPECT_FALSE(CLFArchiveRAR::is_known_format(dir / L"empty.xz"));
		EXPECT_FALSE(CLFArchiveRAR::is_known_format(dir / L"empty.lzma"));
		EXPECT_FALSE(CLFArchiveRAR::is_known_format(dir / L"empty.zst"));

		EXPECT_FALSE(CLFArchiveRAR::is_known_format(dir / L"abcde.gz"));
		EXPECT_FALSE(CLFArchiveRAR::is_known_format(dir / L"abcde.bz2"));
		EXPECT_FALSE(CLFArchiveRAR::is_known_format(dir / L"abcde.xz"));
		EXPECT_FALSE(CLFArchiveRAR::is_known_format(dir / L"abcde.lzma"));
		EXPECT_FALSE(CLFArchiveRAR::is_known_format(dir / L"abcde.zst"));

		EXPECT_FALSE(CLFArchiveRAR::is_known_format(__FILEW__));
		EXPECT_FALSE(CLFArchiveRAR::is_known_format(L"some_non_existing_file"));
	}
	{
		const auto dir = LF_PROJECT_DIR() / L"test";
		EXPECT_FALSE(CLFArchiveRAR::is_known_format(dir / L"test_broken_file.zip"));
		EXPECT_FALSE(CLFArchiveRAR::is_known_format(dir / L"test_broken_crc.zip"));
		EXPECT_FALSE(CLFArchiveRAR::is_known_format(dir / L"test_extract.zip"));
		EXPECT_FALSE(CLFArchiveRAR::is_known_format(dir / L"test_extract.zipx"));
		EXPECT_FALSE(CLFArchiveRAR::is_known_format(dir / L"test_password_abcde.zip"));
		EXPECT_FALSE(CLFArchiveRAR::is_known_format(dir / L"test_unicode_control.zip"));
		EXPECT_FALSE(CLFArchiveRAR::is_known_format(dir / L"test_zip_sfx.dat"));

		EXPECT_TRUE(CLFArchiveRAR::is_known_format(dir / L"smile_encrypted.rar"));
		EXPECT_FALSE(CLFArchiveRAR::is_known_format(dir / L"smile_header_encrypted.rar"));	//need password while checking
		EXPECT_TRUE(CLFArchiveRAR::is_known_format(dir / L"smile_locked.rar"));
		EXPECT_TRUE(CLFArchiveRAR::is_known_format(dir / L"smile_solid.rar"));
		EXPECT_TRUE(CLFArchiveRAR::is_known_format(dir / L"smile.part0001.rar"));
		EXPECT_TRUE(CLFArchiveRAR::is_known_format(dir / L"smile.part0002.rar"));
	}
}

#endif
