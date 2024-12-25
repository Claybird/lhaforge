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

#include "archive.h"

#define LIBARCHIVE_STATIC
#include <libarchive/archive.h>
#include <libarchive/archive_entry.h>
#include "Utilities/FileOperation.h"

template <typename T, typename S>
void convert_stat(T& dest, const S& src)
{
	dest.st_dev = src.st_dev;
	dest.st_ino = src.st_ino;
	dest.st_mode = src.st_mode;
	dest.st_nlink = src.st_nlink;
	dest.st_uid = src.st_uid;
	dest.st_gid = src.st_gid;
	dest.st_rdev = src.st_rdev;
	dest.st_size = (decltype(dest.st_size))src.st_size;
	dest.st_atime = src.st_atime;
	dest.st_mtime = src.st_mtime;
	dest.st_ctime = src.st_ctime;
}

const char* LF_LA_passphrase(struct archive* arc, void* _client_data);

//#include <libarchive/archive_platform.h>
struct LA_EXCEPTION : public ARCHIVE_EXCEPTION
{
protected:
	std::wstring error_string(int la_errno)const {
		switch (la_errno) {
		case ARCHIVE_EOF:
			return L"End of archive";
		case ARCHIVE_OK:
			return L"Operation was successful";
		case ARCHIVE_RETRY:
			return L"Retry might succeed";
		case ARCHIVE_WARN:
			return L"Warning: Partial success";
		case ARCHIVE_FAILED:
			return L"Failed: Current operation cannot complete";
		case ARCHIVE_FATAL:
			return L"Fatal Error: No more operations are possible";
		case EILSEQ/*ARCHIVE_ERRNO_FILE_FORMAT*/:	//<libarchive/archive_platform.h>
			return L"Unrecognized or invalid file format";
		case EINVAL/*ARCHIVE_ERRNO_PROGRAMMER*/:	//<libarchive/archive_platform.h>
			return L"Illegal usage of the library";
		case -1/*ARCHIVE_ERRNO_MISC*/:	//<libarchive/archive_platform.h>
			return L"Unknown or unclassified error";
		default:
			return L"Unknown error";
		}
	}
public:
	LA_EXCEPTION(int la_errno) :ARCHIVE_EXCEPTION(L"") {
		_errno = la_errno;
		auto msg = error_string(la_errno);
	}
};

struct LA_COMPRESSION_CAPABILITY :public LF_COMPRESS_CAPABILITY {
	int mapped_libarchive_format;
};
const LA_COMPRESSION_CAPABILITY& la_get_compression_capability(LF_ARCHIVE_FORMAT fmt);


struct LF_LA_ENTRY {
	DISALLOW_COPY_AND_ASSIGN(LF_LA_ENTRY);
	struct archive* _arc;
	struct archive_entry* _entry;

	LF_ENTRY_STAT _lf_stat;

	LF_LA_ENTRY() :_arc(nullptr), _entry(nullptr) { renew(); }
	LF_LA_ENTRY(archive_entry* entry) = delete;
	virtual ~LF_LA_ENTRY() {
		if (_entry) {
			archive_entry_free(_entry);
			_entry = nullptr;
		}
	}
	archive_entry* la_entry() { return _entry; }
	void clear_entry_stat() {
		_lf_stat = LF_ENTRY_STAT();
	}
	void renew() {
		if (_entry) {
			archive_entry_free(_entry);
			_entry = nullptr;
		}
		_entry = archive_entry_new();
		clear_entry_stat();
	}
	void set_archive(archive* arc) {
		_arc = arc;
		renew();
	}

	bool read_next() {
		clear_entry_stat();
		int r = archive_read_next_header2(_arc, _entry);
		if (ARCHIVE_OK == r) {
			auto s = archive_entry_stat(_entry);
			if (s) {
				convert_stat(_lf_stat.stat, *s);
			} else {
				clear_entry_stat();
			}
			_lf_stat.compressed_size = -1;
			auto p = archive_entry_pathname_w(_entry);
			if (p) {
				_lf_stat.path = p;
			} else {
				auto mbs = archive_entry_pathname(_entry);
				_lf_stat.path = UtilToUNICODE(mbs, strlen(mbs), UtilGuessCodepage(mbs, strlen(mbs)));
			}
			_lf_stat.method_name = L"---";
			_lf_stat.is_encrypted = archive_entry_is_encrypted(_entry);
			return true;
		} else if (ARCHIVE_EOF == r) {
			return false;
		} else {
			throw LA_EXCEPTION(r);
		}
	}

	void set_stat(const LF_ENTRY_STAT& stat) {
		_lf_stat = stat;
		archive_entry_copy_pathname_w(_entry, _lf_stat.path.c_str());
		struct stat s;
		convert_stat(s, _lf_stat.stat);
		archive_entry_copy_stat(_entry, &s);
		archive_entry_set_size(_entry, _lf_stat.stat.st_size);	//make sure file size is in 64bit
	}
};

struct LA_FILE_TO_READ
{
	DISALLOW_COPY_AND_ASSIGN(LA_FILE_TO_READ);
	struct archive* _arc;
	LF_LA_ENTRY _entry;
	struct REWIND {
		bool need_rewind;
		std::filesystem::path arcpath;
		std::shared_ptr<ILFPassphrase> passphrase;
	}_rewind;

	LA_FILE_TO_READ() :_arc(nullptr) {}
	virtual ~LA_FILE_TO_READ() {
		close();
	}
	void open(const std::filesystem::path& arcpath, std::shared_ptr<ILFPassphrase> passphrase) {
		close();
		_rewind.arcpath = arcpath;
		_rewind.need_rewind = false;
		_rewind.passphrase = passphrase;
		_arc = archive_read_new();
		archive_read_support_filter_all(_arc);
		archive_read_support_format_all(_arc);

		int r = archive_read_set_passphrase_callback(_arc, passphrase.get(), LF_LA_passphrase);
		if (r < ARCHIVE_OK) {
			throw LA_EXCEPTION(r);
		}
		r = archive_read_open_filename_w(_arc, arcpath.c_str(), 10240);
		if (r < ARCHIVE_OK) {
			//retry enabling archive_read_support_format_raw
			close();
			if (check_format(arcpath) != LF_ARCHIVE_FORMAT::INVALID) {
				_arc = archive_read_new();
				r = archive_read_set_passphrase_callback(_arc, passphrase.get(), LF_LA_passphrase);
				if (r < ARCHIVE_OK) {
					throw LA_EXCEPTION(r);
				}
				archive_read_support_filter_all(_arc);
				archive_read_support_format_raw(_arc);
				r = archive_read_open_filename_w(_arc, arcpath.c_str(), 10240);
				if (r < ARCHIVE_OK) {
					throw LA_EXCEPTION(r);
				}
			} else {
				throw ARCHIVE_EXCEPTION(L"Invalid format");
			}
		}
	}
	void close() {
		if (_arc) {
			archive_read_close(_arc);
			archive_read_free(_arc);
		}
		_arc = nullptr;
	}
	void rewind() {
		if (_arc) {
			if (_rewind.need_rewind) {
				open(_rewind.arcpath, _rewind.passphrase);
			}
		} else {
			throw ARCHIVE_EXCEPTION(EFAULT);
		}
	}
	LF_LA_ENTRY* begin() {
		rewind();
		_entry.set_archive(_arc);
		return next();
	}
	LF_LA_ENTRY* next() {
		_rewind.need_rewind = true;
		if (_entry.read_next()) {
			if (L"data" == _entry._lf_stat.path && archive_format(_arc) == ARCHIVE_FORMAT_RAW) {
				_entry._lf_stat.path = _rewind.arcpath.stem();
			}

			return &_entry;
		} else {
			return nullptr;
		}
	}

	operator struct archive* () { return _arc; }

	void read_block(std::function<void(const void*, size_t/*data size*/, const offset_info*/*offset*/)> data_receiver) {
		const void* buf;
		size_t size;
		la_int64_t offset = 0;
		while (true) {
			int r = archive_read_data_block(_arc, &buf, &size, &offset);
			if (ARCHIVE_EOF == r) {
				data_receiver(nullptr, 0, 0);
				break;
			} else if (r < ARCHIVE_OK) {
				throw LA_EXCEPTION(r);
				break;
			} else if(buf){
				offset_info oi = { (uint64_t)offset };
				data_receiver(buf, size, &oi);
				break;
			}
		}
	}
	static LF_ARCHIVE_FORMAT check_format(const std::filesystem::path& arcname) {
		try {
			if (std::filesystem::file_size(arcname) == 0)return LF_ARCHIVE_FORMAT::INVALID;
		} catch (...) {
			return LF_ARCHIVE_FORMAT::INVALID;
		}

		CAutoFile fp;
		fp.open(arcname);
		if (!fp.is_opened())return LF_ARCHIVE_FORMAT::INVALID;
		const size_t bufSize = 14;
		std::vector<unsigned char> header(bufSize);
		size_t read = fread(&header[0], 1, bufSize, fp);
		if (read < 1) {
			return LF_ARCHIVE_FORMAT::INVALID;
		}

		//check header for known format; does not care if its tar or not
		//gzip: RFC 1952
		if (read > 2 &&
			header[0] == 0x1f && header[1] == 0x8b) {
			return LF_ARCHIVE_FORMAT::GZ;
		}
		//bz2: https://github.com/dsnet/compress/blob/master/doc/bzip2-format.pdf
		if (read >= 10 &&
			header[0] == 'B' && header[1] == 'Z' && header[2] == 'h' &&
			'0' <= header[3] && header[3] <= '9' &&
			(
				//compressed_magic
				(header[4] == 0x31 && header[5] == 0x41 && header[6] == 0x59 &&
					header[7] == 0x26 && header[8] == 0x53 && header[9] == 0x59) ||
				//eos_magic
				(header[4] == 0x17 && header[5] == 0x72 && header[6] == 0x45 &&
					header[7] == 0x38 && header[8] == 0x50 && header[9] == 0x90)
				)
			) {
			return LF_ARCHIVE_FORMAT::BZ2;
		}
		//zip, zipx: https://en.wikipedia.org/wiki/ZIP_(file_format)
		if (read > 4 && header[0] == 'P' && header[1] == 'K' && (
			(header[2] == 0x03 && header[3] == 0x04) ||
			(header[2] == 0x05 && header[3] == 0x06) ||
			(header[2] == 0x07 && header[3] == 0x08)
			)) {
			return LF_ARCHIVE_FORMAT::ZIP;
		}
		//cab
		if (read >= 4 &&
			header[0] == 'M' && header[1] == 'S' && header[2] == 'C' && header[3] == 'F'){
			return LF_ARCHIVE_FORMAT::READONLY;// CAB;
		}
		//lzh/lzs
		if (read >= 7 &&
			header[2] == '-' && header[3] == 'l' && (header[4] == 'h' || header[4] == 'z') && header[6] == '-') {
			return LF_ARCHIVE_FORMAT::READONLY;// lzh;
		}

		//zstd: https://github.com/facebook/zstd/blob/dev/doc/zstd_compression_format.md
		if (read > 4 &&
			header[0] == 0x28 && header[1] == 0xB5 && header[2] == 0x2F && header[3] == 0xFD) {
			return LF_ARCHIVE_FORMAT::ZSTD;
		}

		//xz: xz-file-format.txt in XZ Utils[https://tukaani.org/xz/]
		if (read > 6 &&
			header[0] == 0xFD && header[1] == '7' && header[2] == 'z' &&
			header[3] == 'X' && header[4] == 'Z' && header[5] == 0x00) {
			return LF_ARCHIVE_FORMAT::XZ;
		}
		//lz4: https://github.com/lz4/lz4/blob/dev/doc/lz4_Frame_format.md
		{
			if (read >= 4 + 3 &&
				header[3] == 0x18 && header[2] == 0x4D && header[1] == 0x22 && header[0] == 0x04) {
				return LF_ARCHIVE_FORMAT::LZ4;
			}
		}
		//lzma: lzma-file-format.txt in XZ Utils[https://tukaani.org/xz/]
		{
			if (read > 13) {
				uint8_t prop = header[0];
				if (prop <= (4 * 5 + 4) * 9 + 8) {
					uint32_t dictsize = (header[1] | header[2] << 8) << 8 | (header[3] | header[4] << 8);
					if (dictsize != 0 && __popcnt(dictsize) <= 2) {
						return LF_ARCHIVE_FORMAT::LZMA;
					}
				}
			}
		}
		//7z
		{
			if (read > 6 &&
				header[0] == '7' && header[1] == 'z' && header[2] == 0xbc &&
				header[3] == 0xaf && header[4] == 0x27 && header[5] == 0x1c) {
				return LF_ARCHIVE_FORMAT::_7Z;
			}
		}
		//iso9660: https://en.wikipedia.org/wiki/List_of_file_signatures
		{
			size_t offsets[] = { 0x8001,0x8801,0x9001 };
			for (const auto& offset : offsets) {
				if (0 == _fseeki64(fp, offset, SEEK_SET)) {
					std::vector<unsigned char> h(bufSize);	//local header
					size_t local_read = fread(&h[0], 1, bufSize, fp);
					if (local_read > 5 &&
						h[0] == 'C' && h[1] == 'D' && h[2] == '0' && h[3] == '0' && h[4] == '1')
						return LF_ARCHIVE_FORMAT::READONLY;	//iso9600
				}
			}
		}
		//cpio: https://github.com/libyal/dtformats/blob/main/documentation/Copy%20in%20and%20out%20(CPIO)%20archive%20format.asciidoc
		{
			if (read > 2 && header[0] == 0x71 && header[1] == 0xc7) {
				return LF_ARCHIVE_FORMAT::READONLY;	//cpio
			}
			if (read > 2 && header[0] == 0xc7 && header[1] == 0x71) {
				return LF_ARCHIVE_FORMAT::READONLY;	//cpio
			}
			if (read > 6 && header[0] == '0' && header[1] == '7' && header[2] == '0' && header[3] == '7' && header[4] == '0' && header[5] == '7') {
				return LF_ARCHIVE_FORMAT::READONLY;	//cpio
			}
			if (read > 6 && header[0] == '0' && header[1] == '7' && header[2] == '0' && header[3] == '7' && header[4] == '0' && header[5] == '1') {
				return LF_ARCHIVE_FORMAT::READONLY;	//cpio
			}
			if (read > 6 && header[0] == '0' && header[1] == '7' && header[2] == '0' && header[3] == '7' && header[4] == '0' && header[5] == '2') {
				return LF_ARCHIVE_FORMAT::READONLY;	//cpio
			}
		}
		//z: https://ja.wikipedia.org/wiki/UNIX_Compress
		{
			if (read > 2 && header[0] == 0x1f && header[1] == 0x9d) {
				return LF_ARCHIVE_FORMAT::READONLY;
			}
		}
		//uuencode: https://en.wikipedia.org/wiki/Uuencoding
		{
			if (read > 10) {
				std::regex re("begin \\d\\d\\d ");
				std::cmatch results;
				if (std::regex_search((const char*)&header[0], (const char*)&header[0] + bufSize, results, re)) {
					return LF_ARCHIVE_FORMAT::READONLY; //uuencode
				}
			}
		}
		//tar: https://www.gnu.org/software/tar/manual/html_node/Standard.html
		{
			if (0 == fseek(fp, 257, SEEK_SET)) {
				std::vector<unsigned char> h(bufSize);	//local header
				size_t local_read = fread(&h[0], 1, bufSize, fp);
				if (local_read > 8 &&
					h[0] == 'u' && h[1] == 's' && h[2] == 't' && h[3] == 'a' && h[4] == 'r' &&
					(
						(h[5] == ' ' && h[6] == ' ' && h[7] == '\0') ||	//OLDGNU_MAGIC
						(h[5] == '\0')		//TMAGIC
						)
					) {
					return LF_ARCHIVE_FORMAT::TAR;
				}
			}
		}

		return LF_ARCHIVE_FORMAT::INVALID;
	}
};

struct LA_FILE_TO_WRITE
{
	DISALLOW_COPY_AND_ASSIGN(LA_FILE_TO_WRITE);
	struct archive* _arc;
	LF_LA_ENTRY _entry;

	LA_FILE_TO_WRITE() :_arc(nullptr) {}
	virtual ~LA_FILE_TO_WRITE() {
		close();
	}

	void open(const std::filesystem::path& arcname,
		LF_ARCHIVE_FORMAT fmt,
		const std::map<std::string, std::string>& archive_options,
		std::shared_ptr<ILFPassphrase> passphrase) {
		const auto& cap = la_get_compression_capability(fmt);

		int la_filter = cap.mapped_libarchive_format & ~ARCHIVE_FORMAT_BASE_MASK;
		int la_fmt = cap.mapped_libarchive_format & ARCHIVE_FORMAT_BASE_MASK;

		write_open_la(arcname, la_fmt, { la_filter }, archive_options, passphrase);
	}
	void write_open_la(const std::wstring& arcname,
		int la_fmt,
		const std::vector<int>& filters,
		const std::map<std::string, std::string>& archive_options,
		std::shared_ptr<ILFPassphrase> passphrase) {
		close();
		_arc = archive_write_new();

		for (auto f : filters) {
			int r = archive_write_add_filter(_arc, f);
			if (r < ARCHIVE_OK) {
				throw LA_EXCEPTION(r);
			}
		}

		int r = archive_write_set_format(_arc, la_fmt);
		if (r < ARCHIVE_OK) {
			throw LA_EXCEPTION(r);
		}

		r = archive_write_set_passphrase_callback(_arc, passphrase.get(), LF_LA_passphrase);
		if (r < ARCHIVE_OK) {
			throw LA_EXCEPTION(r);
		}
		for (auto& ite : archive_options) {
			r = archive_write_set_option(_arc, nullptr, ite.first.c_str(), ite.second.c_str());
			if (r < ARCHIVE_OK) {
				throw LA_EXCEPTION(r);
			}
		}
		r = archive_write_open_filename_w(_arc, arcname.c_str());
		if (r < ARCHIVE_OK) {
			throw LA_EXCEPTION(r);
		}
	}
	void close() {
		if (_arc) {
			archive_write_close(_arc);
			archive_write_free(_arc);
		}
		_arc = nullptr;
	}

	template<typename T>
	void add_entry(LF_LA_ENTRY& entry, T& dataProvider) {
		int r = archive_write_header(_arc, entry.la_entry());
		if (r < ARCHIVE_OK) {
			throw LA_EXCEPTION(r);
		}

		while (true) {
			LF_BUFFER_INFO ibi = dataProvider();
			if (ibi.size) {
				archive_write_data(_arc, ibi.buffer, (size_t)ibi.size);
			} else {
				break;
			}
		}
	}
	void add_directory(LF_LA_ENTRY& entry) {
		int r = archive_write_header(_arc, entry.la_entry());
		if (r < ARCHIVE_OK) {
			throw LA_EXCEPTION(r);
		}
	}
	operator struct archive* () { return _arc; }


	//get most similar option
	static std::tuple<int/*la_format*/, std::vector<int>/*filters*/, bool /*is_encrypted*/>
		mimic_archive_property(LA_FILE_TO_READ& src_archive) {
		//scan for file content; to know archive information
		bool is_src_encrypted = false;
		for (LF_LA_ENTRY* entry = src_archive.begin(); entry; entry = src_archive.next()) {
			if (entry->_lf_stat.is_encrypted) {
				is_src_encrypted = true;
				break;
			}
		}

		int la_format = archive_format(src_archive);

		std::vector<int> filters;
		auto filter_count = archive_filter_count(src_archive);
		for (int i = 0; i < filter_count; i++) {
			auto code = archive_filter_code(src_archive, i);
			filters.push_back(code);
		}

		src_archive.rewind();

		return { la_format, filters, is_src_encrypted };
	}
};
