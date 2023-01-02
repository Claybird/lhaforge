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

//archive file support by libarchive
class CLFArchiveLA :public ILFArchiveFile
{
	DISALLOW_COPY_AND_ASSIGN(CLFArchiveLA);
protected:
	std::unique_ptr<struct LA_FILE_TO_READ> _arc_read;
	std::unique_ptr<struct LA_FILE_TO_WRITE> _arc_write;
public:
	CLFArchiveLA();
	virtual ~CLFArchiveLA();
	void read_open(const std::filesystem::path& file, std::shared_ptr<ILFPassphrase> passphrase)override;
	void write_open(const std::filesystem::path& file, LF_ARCHIVE_FORMAT format, LF_WRITE_OPTIONS options, const LF_COMPRESS_ARGS& args, std::shared_ptr<ILFPassphrase> passphrase)override;
	void close()override;

	bool is_modify_supported()const override;
	//make a copy, and returns in "write_open" state
	std::unique_ptr<ILFArchiveFile> make_copy_archive(
		const std::filesystem::path& dest_path,
		const LF_COMPRESS_ARGS& args,
		std::function<bool(const LF_ENTRY_STAT&)> false_to_skip);

	//archive property
	std::wstring get_format_name()override;
	std::vector<LF_COMPRESS_CAPABILITY> get_compression_capability()const override;

	//entry seek; returns null if it reached EOF
	LF_ENTRY_STAT* read_entry_begin()override;
	LF_ENTRY_STAT* read_entry_next()override;
	void read_entry_end()override;

	bool is_bypass_io_supported()const override{ return false; }

	//read entry
	void read_file_entry_block(std::function<void(const void*, size_t/*data size*/, const offset_info*)> data_receiver)override;
	void read_file_entry_bypass(std::function<void(const void*, size_t/*data size*/, const offset_info*)> data_receiver)override {
		throw ARCHIVE_EXCEPTION(ENOSYS);
	}

	//write entry
	void add_file_entry(const LF_ENTRY_STAT&, std::function<LF_BUFFER_INFO()> dataProvider)override;
	void add_file_entry_bypass(const LF_ENTRY_STAT&, std::function<LF_BUFFER_INFO()> dataProvider)override {
		throw ARCHIVE_EXCEPTION(ENOSYS);
	}
	void add_directory_entry(const LF_ENTRY_STAT&)override;

	static bool is_known_format(const std::filesystem::path &arcname);
};
