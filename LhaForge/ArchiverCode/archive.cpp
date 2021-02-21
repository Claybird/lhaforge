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
	m_ptr = guessSuitableArchiver(file);
	m_ptr->read_open(file, passphrase);
}

void CLFArchive::write_open(const std::filesystem::path& file, LF_ARCHIVE_FORMAT format, const std::map<std::string, std::string> &flags, ILFPassphrase& passphrase)
{
	m_ptr = guessSuitableArchiver(format);
	m_ptr->write_open(file, format, flags, passphrase);
}

std::vector<LF_COMPRESS_CAPABILITY> CLFArchive::get_compression_capability()const
{
	std::vector<LF_COMPRESS_CAPABILITY> caps;
	auto capsLA = CLFArchiveLA().get_compression_capability();
	caps.insert(caps.end(), capsLA.begin(), capsLA.end());
	return caps;
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

