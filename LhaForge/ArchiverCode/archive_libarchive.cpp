#include "stdafx.h"
#include "archive_libarchive.h"
#include "archive_libarchive_impl.h"

#include <errno.h>

#include "ConfigCode/ConfigFile.h"
#include "Utilities/StringUtil.h"


const char* LF_LA_passphrase(struct archive * arc, void *_client_data)
{
	//when callback returns nullptr, it should be handled as "user cancel"
	ILFPassphrase *pc = (ILFPassphrase*)_client_data;
	if (pc) {
		auto out = (*pc)();
		if (out) {
			return out;
		} else {
			CANCEL_EXCEPTION();
		}
	} else {
		CANCEL_EXCEPTION();
	}
}

const static std::vector<LA_COMPRESSION_CAPABILITY> g_la_capabilities = {
	{LF_ARCHIVE_FORMAT::ZIP, L".zip", true, {
		LF_WOPT_STANDARD,
		LF_WOPT_DATA_ENCRYPTION
		}, ARCHIVE_FORMAT_ZIP, },
	{LF_ARCHIVE_FORMAT::_7Z, L".7z", true, {
		LF_WOPT_STANDARD,
		//LF_WOPT_DATA_ENCRYPTION,
		//LF_WOPT_HEADER_ENCRYPTION,
		//LF_WOPT_DATA_ENCRYPTION | LF_WOPT_HEADER_ENCRYPTION,
		//LF_WOPT_SFX | LF_WOPT_DATA_ENCRYPTION
		}, ARCHIVE_FORMAT_7ZIP, },
	{LF_ARCHIVE_FORMAT::GZ, L"{ext}.gz", false, {LF_WOPT_STANDARD}, ARCHIVE_FORMAT_RAW | ARCHIVE_FILTER_GZIP, },
	{LF_ARCHIVE_FORMAT::BZ2, L"{ext}.bz2", false, {LF_WOPT_STANDARD}, ARCHIVE_FORMAT_RAW | ARCHIVE_FILTER_BZIP2, },
	{LF_ARCHIVE_FORMAT::LZMA, L"{ext}.lzma", false, {LF_WOPT_STANDARD}, ARCHIVE_FORMAT_RAW | ARCHIVE_FILTER_LZMA, },
	{LF_ARCHIVE_FORMAT::XZ, L"{ext}.xz", false, {LF_WOPT_STANDARD}, ARCHIVE_FORMAT_RAW | ARCHIVE_FILTER_XZ, },
	{LF_ARCHIVE_FORMAT::ZSTD, L"{ext}.zst", false, {LF_WOPT_STANDARD}, ARCHIVE_FORMAT_RAW | ARCHIVE_FILTER_ZSTD, },
	{LF_ARCHIVE_FORMAT::LZ4, L"{ext}.lz4", false, {LF_WOPT_STANDARD}, ARCHIVE_FORMAT_RAW | ARCHIVE_FILTER_LZ4, },
	{LF_ARCHIVE_FORMAT::TAR, L".tar", true, {LF_WOPT_STANDARD}, ARCHIVE_FORMAT_TAR},
	{LF_ARCHIVE_FORMAT::TAR_GZ, L".tar.gz", true, {LF_WOPT_STANDARD}, ARCHIVE_FORMAT_TAR | ARCHIVE_FILTER_GZIP, },
	{LF_ARCHIVE_FORMAT::TAR_BZ2, L".tar.bz2", true, {LF_WOPT_STANDARD}, ARCHIVE_FORMAT_TAR | ARCHIVE_FILTER_BZIP2, },
	{LF_ARCHIVE_FORMAT::TAR_LZMA, L".tar.lzma", true, {LF_WOPT_STANDARD}, ARCHIVE_FORMAT_TAR | ARCHIVE_FILTER_LZMA, },
	{LF_ARCHIVE_FORMAT::TAR_XZ, L".tar.xz", true, {LF_WOPT_STANDARD}, ARCHIVE_FORMAT_TAR | ARCHIVE_FILTER_XZ, },
	{LF_ARCHIVE_FORMAT::TAR_ZSTD, L".tar.zst", true, {LF_WOPT_STANDARD}, ARCHIVE_FORMAT_TAR | ARCHIVE_FILTER_ZSTD, },
	{LF_ARCHIVE_FORMAT::TAR_LZ4, L".tar.lz4", true, {LF_WOPT_STANDARD}, ARCHIVE_FORMAT_TAR | ARCHIVE_FILTER_LZ4, },
};


const LA_COMPRESSION_CAPABILITY& la_get_compression_capability(LF_ARCHIVE_FORMAT fmt)
{
	for (const auto &cap : g_la_capabilities) {
		if (fmt == cap.format) {
			return cap;
		}
	}
	throw ARCHIVE_EXCEPTION(EINVAL);
}



#include "compress.h"

std::map<std::string, std::string> getLAOptionsFromConfig(
	int la_format,
	const std::vector<int> &la_filters,
	bool encrypt,
	const LF_COMPRESS_ARGS &args)
{
	std::map<std::string, std::string> params;
	//formats
	switch (la_format & ARCHIVE_FORMAT_BASE_MASK) {
	case ARCHIVE_FORMAT_ZIP:
		//ZIP should be handled by minizip-ng
	{
		merge_map(params, args.formats.zip.params);
		if (encrypt) {
			if (params["encryption"] == "zipcrypto") {
				params["encryption"] = "ZipCrypt";
			}
		} else {
			params.erase("encryption");
		}
	}
		break;
	case ARCHIVE_FORMAT_7ZIP:
		merge_map(params, args.formats.sevenzip.params);
		break;
	case ARCHIVE_FORMAT_TAR:
		merge_map(params, args.formats.tar.params);
		break;
	case ARCHIVE_FORMAT_RAW:
		//nothing to do
		break;
	}

	//filters
	for (auto la_filter : la_filters) {
		switch (la_filter & ~ARCHIVE_FORMAT_BASE_MASK) {
		case ARCHIVE_FILTER_GZIP:
			merge_map(params, args.formats.gz.params);
			break;
		case ARCHIVE_FILTER_BZIP2:
			merge_map(params, args.formats.bz2.params);
			break;
		case ARCHIVE_FILTER_LZMA:
			merge_map(params, args.formats.lzma.params);
			break;
		case ARCHIVE_FILTER_XZ:
			merge_map(params, args.formats.xz.params);
			break;
		case ARCHIVE_FILTER_ZSTD:
			merge_map(params, args.formats.zstd.params);
			break;
		case ARCHIVE_FILTER_LZ4:
			merge_map(params, args.formats.lz4.params);
			break;
		}
	}
	return params;
}

std::map<std::string, std::string> getLAOptionsFromConfig(
	const LF_COMPRESS_ARGS &args,
	LF_ARCHIVE_FORMAT format,
	LF_WRITE_OPTIONS options)
{
	const auto& cap = la_get_compression_capability(format);

	if (!isIn(cap.allowed_combinations, options)) {
		throw ARCHIVE_EXCEPTION(EINVAL);
	}

	int la_format = cap.mapped_libarchive_format & ARCHIVE_FORMAT_BASE_MASK;
	std::vector<int> la_filters = { cap.mapped_libarchive_format & ~ARCHIVE_FORMAT_BASE_MASK };

	bool encrypt = (options & LF_WOPT_DATA_ENCRYPTION) != 0;
	return getLAOptionsFromConfig(la_format, la_filters, encrypt, args);
}

//------

CLFArchiveLA::CLFArchiveLA() {}
CLFArchiveLA::~CLFArchiveLA() {}

void CLFArchiveLA::read_open(const std::filesystem::path& file, std::shared_ptr<ILFPassphrase> passhprase)
{
	close();
	_arc_read = std::make_unique<LA_FILE_TO_READ>();
	_arc_read->open(file, passhprase);
	_path = file;
}

void CLFArchiveLA::write_open(
	const std::filesystem::path& file,
	LF_ARCHIVE_FORMAT format,
	LF_WRITE_OPTIONS options,
	const LF_COMPRESS_ARGS& args,
	std::shared_ptr<ILFPassphrase> passphrase)
{
	_arc_write = std::make_unique<LA_FILE_TO_WRITE>();

	auto flags = getLAOptionsFromConfig(args, format, options);
	_arc_write->open(file, format, flags, passphrase);
	_path = file;
}

void CLFArchiveLA::close()
{
	if (_arc_read) {
		_arc_read->close();
		_arc_read.reset();
	}
	if (_arc_write) {
		_arc_write->close();
		_arc_write.reset();
	}
	_path.clear();
}

bool CLFArchiveLA::is_modify_supported()const
{
	if (!_arc_read)return false;
	try {
		auto[la_format, filters, is_encrypted] = LA_FILE_TO_WRITE::mimic_archive_property(*_arc_read);
		if ((la_format & ARCHIVE_FORMAT_BASE_MASK) == ARCHIVE_FORMAT_TAR)return true;
		if ((la_format & ARCHIVE_FORMAT_BASE_MASK) == ARCHIVE_FORMAT_RAW)return false;
		for (const auto &c : g_la_capabilities) {
			if ((c.mapped_libarchive_format & ARCHIVE_FORMAT_BASE_MASK) == la_format) {
				if (!c.contains_multiple_files) {
					return false;
				}
				return true;
			}
		}
		return false;
	} catch(...){
		return false;
	}
}


LF_ARCHIVE_FORMAT CLFArchiveLA::get_format()
{
	if (_arc_read) {
		//scan for file content; to know archive information
		for (auto entry = read_entry_begin(); entry; entry = read_entry_next()) {
			continue;
		}
		int format = archive_format(*_arc_read);
		switch (format & ARCHIVE_FORMAT_BASE_MASK) {
		case ARCHIVE_FORMAT_ZIP:
			return LF_ARCHIVE_FORMAT::ZIP;
		case ARCHIVE_FORMAT_7ZIP:
			return LF_ARCHIVE_FORMAT::_7Z;
		case ARCHIVE_FORMAT_RAW:
			return LA_FILE_TO_READ::check_format(_arc_read.get()->_rewind.arcpath);
		case ARCHIVE_FORMAT_TAR:
			switch (LA_FILE_TO_READ::check_format(_arc_read.get()->_rewind.arcpath)) {
			case LF_ARCHIVE_FORMAT::GZ:
				return LF_ARCHIVE_FORMAT::TAR_GZ;
			case LF_ARCHIVE_FORMAT::BZ2:
				return LF_ARCHIVE_FORMAT::TAR_BZ2;
			case LF_ARCHIVE_FORMAT::LZMA:
				return LF_ARCHIVE_FORMAT::TAR_LZMA;
			case LF_ARCHIVE_FORMAT::XZ:
				return LF_ARCHIVE_FORMAT::TAR_XZ;
			case LF_ARCHIVE_FORMAT::ZSTD:
				return LF_ARCHIVE_FORMAT::TAR_ZSTD;
			case LF_ARCHIVE_FORMAT::LZ4:
				return LF_ARCHIVE_FORMAT::TAR_LZ4;
			case LF_ARCHIVE_FORMAT::INVALID:
				return LF_ARCHIVE_FORMAT::INVALID;
			default:
				return LF_ARCHIVE_FORMAT::TAR;
			}
			break;
		}
	}else if (_arc_write) {
		int format = archive_format(*_arc_write);
		switch (format & ARCHIVE_FORMAT_BASE_MASK) {
		case ARCHIVE_FORMAT_ZIP:
			return LF_ARCHIVE_FORMAT::ZIP;
		case ARCHIVE_FORMAT_7ZIP:
			return LF_ARCHIVE_FORMAT::_7Z;
		case ARCHIVE_FORMAT_RAW:
			switch (format & ~ARCHIVE_FORMAT_BASE_MASK) {
			case ARCHIVE_FILTER_NONE:
			case ARCHIVE_FILTER_GZIP:
				return LF_ARCHIVE_FORMAT::GZ;
			case ARCHIVE_FILTER_BZIP2:
				return LF_ARCHIVE_FORMAT::BZ2;
			case ARCHIVE_FILTER_LZMA:
				return LF_ARCHIVE_FORMAT::LZMA;
			case ARCHIVE_FILTER_XZ:
				return LF_ARCHIVE_FORMAT::XZ;
			case ARCHIVE_FILTER_ZSTD:
				return LF_ARCHIVE_FORMAT::ZSTD;
			case ARCHIVE_FILTER_LZ4:
				return LF_ARCHIVE_FORMAT::LZ4;
			}
			break;
		case ARCHIVE_FORMAT_TAR:
			switch (format & ~ARCHIVE_FORMAT_BASE_MASK) {
			case ARCHIVE_FILTER_GZIP:
				return LF_ARCHIVE_FORMAT::TAR_GZ;
			case ARCHIVE_FILTER_BZIP2:
				return LF_ARCHIVE_FORMAT::TAR_BZ2;
			case ARCHIVE_FILTER_LZMA:
				return LF_ARCHIVE_FORMAT::TAR_LZMA;
			case ARCHIVE_FILTER_XZ:
				return LF_ARCHIVE_FORMAT::TAR_XZ;
			case ARCHIVE_FILTER_ZSTD:
				return LF_ARCHIVE_FORMAT::TAR_ZSTD;
			case ARCHIVE_FILTER_LZ4:
				return LF_ARCHIVE_FORMAT::TAR_LZ4;
			case ARCHIVE_FILTER_NONE:
			default:
				return LF_ARCHIVE_FORMAT::TAR;
			}
			break;
		}
	}
	//fallback
	return LF_ARCHIVE_FORMAT::READONLY;
}


//archive property
std::wstring CLFArchiveLA::get_format_name()
{
	if (_arc_read) {
		//scan for file content; to know archive information
		for (auto entry = read_entry_begin(); entry; entry = read_entry_next()) {
			continue;
		}
		auto p = archive_format_name(*_arc_read);
		if (p)return UtilUTF8toUNICODE(p);
	}
	if (_arc_write) {
		auto p = archive_format_name(*_arc_write);
		if (p)return UtilUTF8toUNICODE(p);
	}
	return L"---";
}


std::vector<LF_COMPRESS_CAPABILITY> CLFArchiveLA::get_compression_capability()const
{
	std::vector<LF_COMPRESS_CAPABILITY> caps(g_la_capabilities.begin(), g_la_capabilities.end());
	return caps;
}

LF_ENTRY_STAT* CLFArchiveLA::read_entry_begin()
{
	if (_arc_read) {
		auto p = _arc_read->begin();
		if (p) {
			return &p->_lf_stat;
		} else {
			return nullptr;
		}
	} else {
		throw ARCHIVE_EXCEPTION(EFAULT);
	}
}

LF_ENTRY_STAT* CLFArchiveLA::read_entry_next()
{
	if (_arc_read) {
		auto p = _arc_read->next();
		if (p) {
			return &p->_lf_stat;
		} else {
			return nullptr;
		}
	} else {
		throw ARCHIVE_EXCEPTION(EFAULT);
	}
}

void CLFArchiveLA::read_entry_end()
{
	if (_arc_read) {
		_arc_read->rewind();
	} else {
		throw ARCHIVE_EXCEPTION(EFAULT);
	}
}



//read file entry
void CLFArchiveLA::read_file_entry_block(std::function<void(const void*, size_t/*data size*/, const offset_info*/*offset*/)> data_receiver)
{
	if (_arc_read) {
		_arc_read->read_block(data_receiver);
	} else {
		throw ARCHIVE_EXCEPTION(EFAULT);
	}
}

//write entry
void CLFArchiveLA::add_file_entry(
	const LF_ENTRY_STAT& lf_stat,
	std::function<LF_BUFFER_INFO()> dataProvider)
{
	if (_arc_write) {
		LF_LA_ENTRY la_entry;
		la_entry.set_archive(*_arc_write);
		la_entry.set_stat(lf_stat);
		_arc_write->add_entry(la_entry, dataProvider);
	} else {
		throw ARCHIVE_EXCEPTION(EFAULT);
	}
}

void CLFArchiveLA::add_directory_entry(const LF_ENTRY_STAT& lf_stat)
{
	if (_arc_write) {
		LF_LA_ENTRY la_entry;
		la_entry.set_archive(*_arc_write);
		la_entry.set_stat(lf_stat);
		_arc_write->add_directory(la_entry);
	} else {
		throw ARCHIVE_EXCEPTION(EFAULT);
	}
}



//make a copy, and returns in "write_open" state
std::unique_ptr<ILFArchiveFile> CLFArchiveLA::make_copy_archive(
	const std::filesystem::path& dest_path,
	const LF_COMPRESS_ARGS& args,
	std::function<bool(const LF_ENTRY_STAT&)> false_to_skip)
{
	if (_arc_read) {
		ASSERT(_arc_read->_rewind.passphrase);
		auto[la_format, filters, is_encrypted] = LA_FILE_TO_WRITE::mimic_archive_property(*_arc_read);
		std::unique_ptr<CLFArchiveLA> dest_archive = std::make_unique<CLFArchiveLA>();
		dest_archive->_arc_write = std::make_unique<LA_FILE_TO_WRITE>();

		//- open an output archive in most similar option
		auto options = getLAOptionsFromConfig(la_format, filters, is_encrypted, args);
		dest_archive->_arc_write->write_open_la(dest_path, la_format, filters, options, _arc_read->_rewind.passphrase);

		//- then, copy entries if filter returns true
		//this would need overhead of extract on read and compress on write
		//there seems no way to get raw data
		_arc_read->rewind();
		for (LF_LA_ENTRY* entry = _arc_read->begin(); entry; entry = _arc_read->next()) {
			if (false_to_skip(entry->_lf_stat)) {
				if (entry->_lf_stat.is_directory()) {
					dest_archive->_arc_write->add_directory(*entry);
				} else {
					dest_archive->_arc_write->add_entry(*entry, [&]() {
						while (UtilDoMessageLoop())continue;	//TODO
						//TODO progress handler
						LF_BUFFER_INFO bi;
						_arc_read->read_block([&](const void* buf, int64_t size, const offset_info* offset) {
							bi.buffer = buf;
							bi.size = (size_t)size;
							bi.offset = offset;
						});
						return bi;
					});
				}
			}
		}

		//- copy finished. now the caller can add extra files
		return dest_archive;
	} else {
		throw ARCHIVE_EXCEPTION(EFAULT);
	}
}


#include "CommonUtil.h"

bool CLFArchiveLA::is_known_format(const std::filesystem::path &arcname)
{
	return LA_FILE_TO_READ::check_format(arcname) != LF_ARCHIVE_FORMAT::INVALID;
}

