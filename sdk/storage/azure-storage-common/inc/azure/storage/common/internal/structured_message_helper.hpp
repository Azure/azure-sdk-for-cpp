// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "azure/storage/common/dll_import_export.hpp"

#include <algorithm>
#include <functional>
#include <memory>

namespace Azure { namespace Storage { namespace _internal {

  enum StructuredMessageFlags : uint16_t
  {
    None = 0x0,
    Crc64 = 0x1,
  };

  enum StructuredMessageCurrentRegion
  {
    StreamHeader,
    StreamFooter,
    SegmentHeader,
    SegmentFooter,
    SegmentContent,
  };

  class StructuredMessageHelper final {
  public:
    AZ_STORAGE_COMMON_DLLEXPORT static const int64_t Crc64Length;

    AZ_STORAGE_COMMON_DLLEXPORT static const uint8_t StructuredMessageVersion;

    AZ_STORAGE_COMMON_DLLEXPORT static const int64_t StreamHeaderLength;
    AZ_STORAGE_COMMON_DLLEXPORT static const int64_t StreamHeaderVersionOffset;
    AZ_STORAGE_COMMON_DLLEXPORT static const int64_t StreamHeaderMessageLengthOffset;
    AZ_STORAGE_COMMON_DLLEXPORT static const int64_t StreamHeaderFlagsOffset;
    AZ_STORAGE_COMMON_DLLEXPORT static const int64_t StreamHeaderSegmentCountOffset;

    AZ_STORAGE_COMMON_DLLEXPORT static const int64_t SegmentHeaderLength;
    AZ_STORAGE_COMMON_DLLEXPORT static const int64_t SegmentHeaderNumOffset;
    AZ_STORAGE_COMMON_DLLEXPORT static const int64_t SegmentHeaderContentLengthOffset;

    static void WriteStreamHeader(
        uint8_t* buffer,
        const uint64_t& messageLength,
        const uint16_t& flags,
        const uint16_t& segmentCount);

    static void WriteSegmentHeader(
        uint8_t* buffer,
        const uint16_t& segmentCount,
        const uint64_t& segmentLength);

    static void WriteSegmentFooter(uint8_t* buffer, const uint8_t* crc64);

    static void WriteStreamFooter(uint8_t* buffer, const uint8_t* crc64);

    static void ReadStreamHeader(
        const uint8_t* buffer,
        uint64_t& messageLength,
        StructuredMessageFlags& flags,
        uint16_t& segmentCount);

    static void ReadSegmentHeader(const uint8_t* buffer, uint16_t& segmentCount, uint64_t& segmentLength);

    static void ReadSegmentFooter(const uint8_t* buffer, uint8_t* crc64);

    static void ReadStreamFooter(const uint8_t* buffer, uint8_t* crc64);
  };

}}} // namespace Azure::Storage::_internal