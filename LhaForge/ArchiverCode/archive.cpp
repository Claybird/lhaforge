#include "stdafx.h"
#include "archive.h"
#include "archive_libarchive.h"
#include "archive_bga.h"
#include "archive_arj.h"
#include "archive_zip.h"
#include "archive_rar.h"
#include "resource.h"

std::unique_ptr<ILFArchiveFile> guessSuitableArchiver(const std::filesystem::path& path)
{
	if (CLFArchiveZIP::is_known_format(path))return std::make_unique<CLFArchiveZIP>();
	if (CLFArchiveBGA::is_known_format(path))return std::make_unique<CLFArchiveBGA>();
	if (CLFArchiveARJ::is_known_format(path))return std::make_unique<CLFArchiveARJ>();
	if (CLFArchiveRAR::is_known_format(path))return std::make_unique<CLFArchiveRAR>();

	//check for libarchive is weak, in the current implementation
	if (CLFArchiveLA::is_known_format(path))return std::make_unique<CLFArchiveLA>();
	RAISE_EXCEPTION(UtilLoadString(IDS_ERROR_UNKNOWN_FORMAT));
}

std::unique_ptr<ILFArchiveFile> guessSuitableArchiver(LF_ARCHIVE_FORMAT format)
{
	switch (format) {
	case LF_ARCHIVE_FORMAT::ZIP:
		return std::make_unique<CLFArchiveZIP>();
	case LF_ARCHIVE_FORMAT::_7Z:
	case LF_ARCHIVE_FORMAT::GZ:
	case LF_ARCHIVE_FORMAT::BZ2:
	case LF_ARCHIVE_FORMAT::LZMA:
	case LF_ARCHIVE_FORMAT::XZ:
	case LF_ARCHIVE_FORMAT::ZSTD:
	case LF_ARCHIVE_FORMAT::LZ4:
	case LF_ARCHIVE_FORMAT::TAR:
	case LF_ARCHIVE_FORMAT::TAR_GZ:
	case LF_ARCHIVE_FORMAT::TAR_BZ2:
	case LF_ARCHIVE_FORMAT::TAR_LZMA:
	case LF_ARCHIVE_FORMAT::TAR_XZ:
	case LF_ARCHIVE_FORMAT::TAR_ZSTD:
	case LF_ARCHIVE_FORMAT::TAR_LZ4:
		return std::make_unique<CLFArchiveLA>();
	default:
		RAISE_EXCEPTION(UtilLoadString(IDS_ERROR_UNKNOWN_FORMAT));
	}
}


void CLFArchive::read_open(const std::filesystem::path& file, std::shared_ptr<ILFPassphrase> passphrase)
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
	std::shared_ptr<ILFPassphrase> passphrase)
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

	auto capsZIP = CLFArchiveZIP().get_compression_capability();
	caps.insert(caps.end(), capsZIP.begin(), capsZIP.end());

	auto capsBGA = CLFArchiveBGA().get_compression_capability();
	caps.insert(caps.end(), capsBGA.begin(), capsBGA.end());

	auto capsARJ = CLFArchiveARJ().get_compression_capability();
	caps.insert(caps.end(), capsARJ.begin(), capsARJ.end());

	auto capsRAR = CLFArchiveRAR().get_compression_capability();
	caps.insert(caps.end(), capsRAR.begin(), capsRAR.end());

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
	RAISE_EXCEPTION(UtilLoadString(IDS_ERROR_UNKNOWN_FORMAT));
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

