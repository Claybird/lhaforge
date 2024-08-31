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
#include "Utilities/Utility.h"
#include "Utilities/FileOperation.h"
#include "ArchiverCode/archive.h"

#include "ConfigCode/ConfigFile.h"
#include "ConfigCode/ConfigGeneral.h"
#include "ConfigCode/ConfigCompress.h"
#include "ConfigCode/ConfigCompressFormat.h"


enum class COMPRESS_IGNORE_TOP_DIR :int {
	None,
	FirstTop,
	Recursive,
	ENUM_COUNT_AND_LASTITEM
};

struct LF_COMPRESS_ARGS {
	CConfigGeneral general;
	CConfigCompress compress;
	struct _formats {
		CConfigCompressFormatZIP zip;
		CConfigCompressFormat7Z sevenzip;
		CConfigCompressFormatTAR tar;
		CConfigCompressFormatGZ gz;
		CConfigCompressFormatBZ2 bz2;
		CConfigCompressFormatLZMA lzma;
		CConfigCompressFormatXZ xz;
		CConfigCompressFormatZSTD zstd;
		CConfigCompressFormatLZ4 lz4;

		void load(const CConfigFile& mngr) {
			zip.load(mngr);
			sevenzip.load(mngr);
			tar.load(mngr);
			gz.load(mngr);
			bz2.load(mngr);
			lzma.load(mngr);
			xz.load(mngr);
			zstd.load(mngr);
			lz4.load(mngr);
		}
	}formats;
	void load(const CConfigFile& mngr) {
		general.load(mngr);
		compress.load(mngr);
		formats.load(mngr);
	}
};

struct RAW_FILE_READER {
	CAutoFile fp;
	std::vector<unsigned char> buffer;
	RAW_FILE_READER() {
		buffer.resize(1024 * 1024 * 32);	//32MB cache
	}
	virtual ~RAW_FILE_READER() {}
	LF_BUFFER_INFO operator()() {
		LF_BUFFER_INFO buf = {};
		if (!fp || feof(fp)) {
			return buf;
		} else {
			buf.buffer = &buffer[0];
			buf.offset = nullptr;
			buf.size = fread(&buffer[0], 1, buffer.size(), fp);
			return buf;
		}
	}
	void open(const std::filesystem::path& path);
	void close() {
		fp.close();
	}
};


struct CMDLINEINFO;

bool GUI_compress_multiple_files(
	const std::vector<std::filesystem::path> &givenFiles,
	LF_ARCHIVE_FORMAT format,
	LF_WRITE_OPTIONS options,
	ILFProgressHandler &progressHandler,
	const CConfigFile& config,
	const CMDLINEINFO* lpCmdLineInfo);

//command line parameter table
struct COMPRESS_COMMANDLINE_PARAMETER{
	const std::wstring name;
	LF_ARCHIVE_FORMAT Type;
	int Options;
	WORD FormatName;
};
extern const std::vector<COMPRESS_COMMANDLINE_PARAMETER> g_CompressionCmdParams;
const COMPRESS_COMMANDLINE_PARAMETER& get_archive_format_args(LF_ARCHIVE_FORMAT fmt, int opts);
struct COMPRESS_SOURCES {
	virtual ~COMPRESS_SOURCES() {}
	struct PATH_PAIR {
		std::filesystem::path originalFullPath;
		std::filesystem::path entryPath;
	};
	std::filesystem::path basePath;
	std::vector<PATH_PAIR> pathPair;
};
