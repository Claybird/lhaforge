#pragma once
#include "archive.h"

class CLFArchiveZIP :public ILFArchiveFile
{
	DISALLOW_COPY_AND_ASSIGN(CLFArchiveZIP);
protected:
	struct INTERNAL;
	INTERNAL* _internal;
	LF_ENTRY_STAT _entry;
	LF_ENTRY_STAT* read_entry_attrib();
	LF_ENTRY_STAT* read_entry_internal(std::function<int32_t(void*)>);
public:
	CLFArchiveZIP();
	virtual ~CLFArchiveZIP();
	void read_open(const std::filesystem::path& file, ILFPassphrase&)override;
	void write_open(const std::filesystem::path& file, LF_ARCHIVE_FORMAT format, LF_WRITE_OPTIONS options, const LF_COMPRESS_ARGS& args, ILFPassphrase& passphrase)override;
	void close()override;

	bool is_modify_supported()const override;
	//make a copy, and returns in "write_open" state
	std::unique_ptr<ILFArchiveFile> make_copy_archive(
		const std::filesystem::path& dest_path,
		const LF_COMPRESS_ARGS& args,
		std::function<bool(const LF_ENTRY_STAT&)> false_if_skip);

	//archive property
	std::wstring get_format_name()override { return L"ZIP"; }
	std::vector<LF_COMPRESS_CAPABILITY> get_compression_capability()const override;

	//entry seek; returns null if it reached EOF
	LF_ENTRY_STAT* read_entry_begin()override;
	LF_ENTRY_STAT* read_entry_next()override;
	void read_entry_end()override;

	bool is_bypass_io_supported()const override { return true; }

	//read entry
	void read_file_entry_block(std::function<void(const void*, size_t, const offset_info*)> data_receiver)override;
	void read_file_entry_bypass(std::function<void(const void*, size_t, const offset_info*)> data_receiver)override;

	//write entry
	void add_file_entry(const LF_ENTRY_STAT&, std::function<LF_BUFFER_INFO()> dataProvider)override;
	void add_file_entry_bypass(const LF_ENTRY_STAT&, std::function<LF_BUFFER_INFO()> dataProvider)override;
	void add_directory_entry(const LF_ENTRY_STAT&)override;
	static bool is_known_format(const std::filesystem::path& arcname);
};
