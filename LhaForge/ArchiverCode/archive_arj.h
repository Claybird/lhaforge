#pragma once
#include "archive.h"
#include "Utilities/FileOperation.h"

class CLzhDecoder2;
//Arj decoder
// this code provides extract only
// this code is based on xacrett: http://www.kmonos.net/lib/xacrett.ja.html
class CLFArchiveARJ :public ILFArchiveFile
{
	DISALLOW_COPY_AND_ASSIGN(CLFArchiveARJ);
	enum {
		ARJ_DICSIZE = 26624
	};
protected:
	enum {
		BINARY_TYPE = 0,
		TEXT_TYPE = 1,
	};
	struct CRC {
		enum {
			CRC_MASK = 0xffffffff,
		};
		DWORD crc;
		DWORD crctable[256];
		DWORD h_Origsize;
		CRC() :crc(CRC_MASK) {
			for (DWORD r, j, i = 0; i != 256; i++) {
				r = i;
				for (j = 8; j != 0; j--) {
					if (r & 1)	r = (r >> 1) ^ 0xEDB88320L;//CRCPOLY
					else		r >>= 1;
				}
				crctable[i] = r;
			}
		}
		virtual ~CRC() {}
		void reset() { crc = CRC_MASK; }
		void update(const BYTE* buf, int len) {
			while (len--)crc = crctable[((BYTE)(crc) ^ (*buf++)) & 0xff] ^ (crc >> 8);
		}
		DWORD get_crc()const { return crc ^ CRC_MASK; }
		size_t fread_crc(void* p, size_t n, FILE* f) {
			n = fread(p, 1, n, f);
			h_Origsize += n;
			update((BYTE*)p, n);
			return n;
		}
		void update_crc(const BYTE* p, int n) {
			update(p, n);
		}
	};
	struct Header {
		enum {
			ARJ_FNAME_MAX = 512,
			FIRST_HDR_SIZE = 30,
			COMMENT_MAX = 2048,
			HEADERSIZE_MAX = (FIRST_HDR_SIZE + 10 + ARJ_FNAME_MAX + COMMENT_MAX),
			HEADER_ID = 0xEA60,
			HEADER_ID_HI = 0xEA,
			HEADER_ID_LO = 0x60,
			ARJ_X_VERSION = 3,
		};
		WORD h_Size;	//headersize
		BYTE h_1stSize;	//first_hdr_size
		BYTE h_ArjVer;	//arj_nbr
		BYTE h_ArjXVer;	//arj_x_nbr
		BYTE h_OS;		//host_os
		BYTE h_Flags;	//arj_flags
		BYTE h_Method;	//method
		BYTE h_FileType;//file_type
		//BYTE h_PassInfo;
		DWORD h_TimeStamp;
		DWORD h_Compsize;
		DWORD h_Origsize;
		DWORD h_FileCRC;
		WORD  h_EntryPos;
		WORD  h_Attrib;
		WORD  h_HostData;
		//BYTE[n]; additional information; depends on h_Size
		char h_Filename[ARJ_FNAME_MAX];	// filename; null terminated
		//char[n] commend; null terminated
		DWORD h_CRC;	//header_crc

		static size_t find_header(const unsigned char* hdr, unsigned long siz);
		static std::pair<bool, Header> read_header(FILE* fp);
		bool validate_flags()const {
			if (h_ArjXVer > ARJ_X_VERSION)return false;
			if ((h_Flags & 0x01) != 0)return false;
			if (h_Method < 0 || h_Method>4 || (h_Method == 4 && h_ArjVer == 1))return false;
			if (h_FileType != BINARY_TYPE && h_FileType != TEXT_TYPE)return false;
			return true;
		}
	};
	std::shared_ptr<Header> _header;
	CRC _crc;

	CAutoFile _fp;
	LF_ENTRY_STAT _entry_stat;
	uint64_t _nextEntryPos;

	//---decoder
	struct decode_f {
		Header* _header;
		FILE* _fp;
		DWORD count;
		WORD bitbuf;
		WORD subbitbuf;
		int bitcount;
		short getlen;
		short getbuf;

		void initDecoder(FILE* fp, Header* hdr);
		void decode(std::function<void(const void*, int64_t/*data size*/)> data_receiver);

		void init_getbits();
		void fillbuf(int n);
		short decode_ptr();
		short decode_len();
	};
	decode_f _decode_f;
	std::shared_ptr<CLzhDecoder2> _lzh;
public:
	CLFArchiveARJ();
	virtual ~CLFArchiveARJ();

	void read_open(const std::filesystem::path& file, std::shared_ptr<ILFPassphrase>)override;
	void write_open(const std::filesystem::path& file, LF_ARCHIVE_FORMAT format, LF_WRITE_OPTIONS options, const LF_COMPRESS_ARGS& args, std::shared_ptr<ILFPassphrase> passphrase)override {
		throw LF_EXCEPTION(L"Read only format");
	}
	void close()override;

	bool is_modify_supported()const override { return false; }
	//make a copy, and returns in "write_open" state
	std::unique_ptr<ILFArchiveFile> make_copy_archive(
		const std::filesystem::path& dest_path,
		const LF_COMPRESS_ARGS& args,
		std::function<bool(const LF_ENTRY_STAT&)> false_to_skip) {
		throw ARCHIVE_EXCEPTION(ENOSYS);
	}

	//archive property
	LF_ARCHIVE_FORMAT get_format()override { return LF_ARCHIVE_FORMAT::READONLY; }
	std::wstring get_format_name()override { return L"ARJ"; }
	std::vector<LF_COMPRESS_CAPABILITY> get_compression_capability()const override {
		//read only
		return {};
	}

	//entry seek; returns null if it reached EOF
	LF_ENTRY_STAT* read_entry_begin()override;
	LF_ENTRY_STAT* read_entry_next()override;
	void read_entry_end()override;

	//read entry
	void read_file_entry_block(std::function<void(const void*, size_t/*data size*/, const offset_info*)> data_receiver)override;

	//write entry
	void add_file_entry(const LF_ENTRY_STAT&, std::function<LF_BUFFER_INFO()> dataProvider)override {
		throw ARCHIVE_EXCEPTION(ENOSYS);
	}
	void add_directory_entry(const LF_ENTRY_STAT&)override {
		throw ARCHIVE_EXCEPTION(ENOSYS);
	}

	static bool is_known_format(const std::filesystem::path& arcname);
};
