#pragma once
#include "archive.h"
#include "Utilities/FileOperation.h"

//GZA/BZA interface; bzip/gzip based compression, with archiving feature
// this code provides extract only
// original archiver: http://www.csdinc.co.jp/archiver/lib/bga32.html
// this code is based on xacrett: http://www.kmonos.net/lib/xacrett.ja.html
class CLFArchiveBGA :public ILFArchiveFile
{
	DISALLOW_COPY_AND_ASSIGN(CLFArchiveBGA);
protected:
	struct BgaHeader;
	std::shared_ptr<BgaHeader> _header;
	int64_t _current_entry_offset;
	LF_ENTRY_STAT _entry_stat;

	CAutoFile _fp;
	bool readHeader();
	bool isEntryCompressed()const;
	std::wstring get_method_name()const;

	struct Decoder {
		virtual ~Decoder() {}
		virtual void decode(std::function<void(const void*, int64_t/*data size*/)> data_receiver) = 0;
	};
	struct DecoderGZ; struct DecoderBZ2; struct DecoderRaw;
	std::shared_ptr<Decoder> _decoder;
public:
	CLFArchiveBGA();
	virtual ~CLFArchiveBGA();

	void read_open(const std::filesystem::path& file, ILFPassphrase& )override;
	void write_open(const std::filesystem::path& file, LF_ARCHIVE_FORMAT format, LF_WRITE_OPTIONS options, const LF_COMPRESS_ARGS& args, ILFPassphrase& passphrase)override {
		throw LF_EXCEPTION(L"Read only format");
	}
	void close()override;

	bool is_modify_supported()const override { return false; }
	//make a copy, and returns in "write_open" state
	std::unique_ptr<ILFArchiveFile> make_copy_archive(
		const std::filesystem::path& dest_path,
		const LF_COMPRESS_ARGS& args,
		std::function<bool(const LF_ENTRY_STAT&)> false_if_skip) {
		throw ARCHIVE_EXCEPTION(ENOSYS);
	}

	//archive property
	std::wstring get_format_name()override { return L"BZA/GZA"; }
	std::vector<LF_COMPRESS_CAPABILITY> get_compression_capability()const override {
		//read only
		return {};
	}

	//entry seek; returns null if it reached EOF
	LF_ENTRY_STAT* read_entry_begin()override;
	LF_ENTRY_STAT* read_entry_next()override;
	void read_entry_end()override;

	bool is_bypass_io_supported()const override { return false; }

	//read entry
	void read_file_entry_block(std::function<void(const void*, size_t, const offset_info*)> data_receiver)override;
	void read_file_entry_bypass(std::function<void(const void*, size_t, const offset_info*)> data_receiver)override {
		throw ARCHIVE_EXCEPTION(ENOSYS);
	}

	//write entry
	void add_file_entry(const LF_ENTRY_STAT&, std::function<LF_BUFFER_INFO()> dataProvider)override {
		throw ARCHIVE_EXCEPTION(ENOSYS);
	}
	void add_file_entry_bypass(const LF_ENTRY_STAT&, std::function<LF_BUFFER_INFO()> dataProvider)override {
		throw ARCHIVE_EXCEPTION(ENOSYS);
	}
	void add_directory_entry(const LF_ENTRY_STAT&)override {
		throw ARCHIVE_EXCEPTION(ENOSYS);
	}

	static bool is_known_format(const std::filesystem::path& arcname);
};
