#include "stdafx.h"
#include "archive.h"
#include "archive_libarchive.h"

std::unique_ptr<ILFArchiveFile> guessSuitableArchiver(const std::filesystem::path& path)
{
	if (CLFArchiveLA::is_known_format(path))return std::make_unique<CLFArchiveLA>();
	RAISE_EXCEPTION(L"Unknown format");
}

std::unique_ptr<ILFArchiveFile> guessSuitableArchiver(LF_ARCHIVE_FORMAT format)
{
	switch (format) {
	case LF_FMT_ZIP:
	case LF_FMT_7Z:
	case LF_FMT_GZ:
	case LF_FMT_BZ2:
	case LF_FMT_LZMA:
	case LF_FMT_XZ:
	case LF_FMT_ZSTD:
	case LF_FMT_TAR:
	case LF_FMT_TAR_GZ:
	case LF_FMT_TAR_BZ2:
	case LF_FMT_TAR_LZMA:
	case LF_FMT_TAR_XZ:
	case LF_FMT_TAR_ZSTD:
	case LF_FMT_UUE:
		return std::make_unique<CLFArchiveLA>();
	default:
		RAISE_EXCEPTION(L"Unknown format");
	}
}

void CLFArchive::read_open(const std::filesystem::path& file, ILFPassphrase& passphrase)
{
	close();
	m_ptr = guessSuitableArchiver(file);
	m_ptr->read_open(file, passphrase);
}

void CLFArchive::write_open(
	const std::filesystem::path& file,
	LF_ARCHIVE_FORMAT format,
	LF_WRITE_OPTIONS options,
	const LF_COMPRESS_ARGS &args,
	ILFPassphrase& passphrase)
{
	close();
	m_ptr = guessSuitableArchiver(format);
	m_ptr->write_open(file, format, options, args, passphrase);
}

std::vector<LF_COMPRESS_CAPABILITY> CLFArchive::get_compression_capability()const
{
	std::vector<LF_COMPRESS_CAPABILITY> caps;
	auto capsLA = CLFArchiveLA().get_compression_capability();
	caps.insert(caps.end(), capsLA.begin(), capsLA.end());
	return caps;
}

LF_COMPRESS_CAPABILITY CLFArchive::get_compression_capability(LF_ARCHIVE_FORMAT format)
{
	auto arc = guessSuitableArchiver(format);
	std::vector<LF_COMPRESS_CAPABILITY> caps = arc->get_compression_capability();
	for (const auto cap : caps) {
		if (cap.format == format) {
			return cap;
		}
	}
	RAISE_EXCEPTION(L"Unknown format");
}

//-1 if no information is given
int64_t CLFArchive::get_num_entries()
{
	if (m_numEntries == -1) {
		//no cache available
		try {
			m_numEntries = 0;
			for (auto entry = read_entry_begin(); entry; entry = read_entry_next()) {
				m_numEntries++;
			}
		} catch (const LF_EXCEPTION&) {
			m_numEntries = -1;
		}
	}
	return m_numEntries;
}

bool CLFArchive::is_known_format(const std::filesystem::path& path)
{
	try {
		guessSuitableArchiver(path);
		return true;
	} catch(...) {
		return false;
	}
	return false;
}

