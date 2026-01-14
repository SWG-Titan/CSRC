// ======================================================================
// NunaToc.h - titanlst/TOC File Support for Nuna
// Copyright (c) Titan Project
// ======================================================================

#ifndef NUNA_TOC_H
#define NUNA_TOC_H

#include "Nuna.h"

namespace Nuna
{

// titanlst structures use Nuna.h types
using TitanlstHeader = TocFileHeader;
using TitanlstEntry = TocFileEntry;

// titanlst API
Result createTitanlst(const std::string& outputFile, const std::vector<std::string>& treFiles, const PackOptions& options);
Result validateTitanlst(const std::string& inputFile);
Result listTitanlst(const std::string& inputFile, const ListOptions& options);
Result unpackTitanlst(const std::string& inputFile, const std::string& outputDir, const UnpackOptions& options);

} // namespace Nuna

#endif // NUNA_TOC_H