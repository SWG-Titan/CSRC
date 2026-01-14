// ======================================================================
// NunaCompression.h - Compression utilities for Nuna
// Copyright (c) Titan Project
// ======================================================================

#ifndef NUNA_COMPRESSION_H
#define NUNA_COMPRESSION_H

#include <vector>
#include <cstdint>
#include <zlib.h>

namespace Nuna
{
namespace Compression
{

// Compress data using zlib
inline bool compress(const uint8_t* data, size_t dataSize, 
                     std::vector<uint8_t>& compressed, int level = Z_DEFAULT_COMPRESSION)
{
    uLongf compSize = compressBound(static_cast<uLong>(dataSize));
    compressed.resize(compSize);
    
    int result = compress2(compressed.data(), &compSize,
                           data, static_cast<uLong>(dataSize), level);
    
    if (result != Z_OK)
        return false;
    
    compressed.resize(compSize);
    return true;
}

// Decompress data using zlib
inline bool decompress(const uint8_t* data, size_t dataSize,
                       std::vector<uint8_t>& decompressed, size_t expectedSize)
{
    decompressed.resize(expectedSize);
    uLongf destLen = static_cast<uLongf>(expectedSize);
    
    int result = uncompress(decompressed.data(), &destLen,
                            data, static_cast<uLong>(dataSize));
    
    if (result != Z_OK)
        return false;
    
    decompressed.resize(destLen);
    return true;
}

// Check if compression is worthwhile (at least 5% savings)
inline bool shouldCompress(size_t originalSize, size_t compressedSize)
{
    if (originalSize == 0)
        return false;
    return compressedSize < (originalSize * 95 / 100);
}

} // namespace Compression
} // namespace Nuna

#endif // NUNA_COMPRESSION_H
