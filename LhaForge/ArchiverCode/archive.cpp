#include "stdafx.h"
#include "archive.h"

bool LF_isKnownArchive(const std::filesystem::path& fname)
{
	return CLFArchiveLA::isKnownFormat(fname);
}
