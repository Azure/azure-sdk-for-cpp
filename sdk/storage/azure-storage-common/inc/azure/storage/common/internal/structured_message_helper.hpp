// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "azure/storage/common/dll_import_export.hpp"

#include <algorithm>
#include <functional>
#include <memory>

namespace Azure { namespace Storage { namespace _internal {

  enum class StructuredMessageFlags : uint16_t
  {
    None = 0x0,
    Crc64 = 0x1,
  };

  enum class StructuredMessageCurrentRegion
  {
    StreamHeader,
    StreamFooter,
    SegmentHeader,
    SegmentFooter,
    SegmentContent,
  };

  /**
   * @brief Helper class for reading and writing structured message format headers and footers.
   */
  class StructuredMessageHelper final {
  public:
    /** @brief Length of CRC64 checksum in bytes. */
    AZ_STORAGE_COMMON_DLLEXPORT static const size_t Crc64Length;

    /** @brief Current structured message format version. */
    AZ_STORAGE_COMMON_DLLEXPORT static const uint8_t StructuredMessageVersion;

    /** @brief Total length of the stream header in bytes. */
    AZ_STORAGE_COMMON_DLLEXPORT static const size_t StreamHeaderLength;
    /** @brief Offset of the version field within the stream header. */
    AZ_STORAGE_COMMON_DLLEXPORT static const int64_t StreamHeaderVersionOffset;
    /** @brief Offset of the message length field within the stream header. */
    AZ_STORAGE_COMMON_DLLEXPORT static const int64_t StreamHeaderMessageLengthOffset;
    /** @brief Offset of the flags field within the stream header. */
    AZ_STORAGE_COMMON_DLLEXPORT static const int64_t StreamHeaderFlagsOffset;
    /** @brief Offset of the segment count field within the stream header. */
    AZ_STORAGE_COMMON_DLLEXPORT static const int64_t StreamHeaderSegmentCountOffset;

    /** @brief Total length of the segment header in bytes. */
    AZ_STORAGE_COMMON_DLLEXPORT static const size_t SegmentHeaderLength;
    /** @brief Offset of the segment number field within the segment header. */
    AZ_STORAGE_COMMON_DLLEXPORT static const int64_t SegmentHeaderNumOffset;
    /** @brief Offset of the segment content length field within the segment header. */
    AZ_STORAGE_COMMON_DLLEXPORT static const int64_t SegmentHeaderContentLengthOffset;

    /**
     * @brief Writes the stream header to the buffer.
     *
     * @param buffer Pointer to a buffer of at least StreamHeaderLength bytes where the header will
     * be written.
     * @param messageLength Total length of the message content in bytes.
     * @param flags Flags indicating message options such as CRC64 checksum.
     * @param segmentCount Total number of segments in the message.
     */
    static void WriteStreamHeader(
        uint8_t* buffer,
        uint64_t messageLength,
        uint16_t flags,
        uint16_t segmentCount);

    /**
     * @brief Writes the segment header to the buffer.
     *
     * @param buffer Pointer to a buffer of at least SegmentHeaderLength bytes where the header
     * will be written.
     * @param segmentNum The 1-based segment number.
     * @param segmentLength Length of the segment content in bytes.
     */
    static void WriteSegmentHeader(uint8_t* buffer, uint16_t segmentNum, uint64_t segmentLength);

    /**
     * @brief Writes a CRC64 checksum to the buffer.
     *
     * @param buffer Pointer to a buffer of at least Crc64Length bytes where the CRC64 will be
     * written.
     * @param crc64 Pointer to the CRC64 checksum bytes to write. If nullptr, no data is written.
     */
    static void WriteCrc64(uint8_t* buffer, uint8_t const* crc64);

    /**
     * @brief Reads the stream header from the buffer.
     *
     * @param buffer Pointer to a buffer of at least StreamHeaderLength bytes containing the
     * header.
     * @param messageLength Output parameter for the total message length.
     * @param flags Output parameter for the message flags.
     * @param segmentCount Output parameter for the total segment count.
     */
    static void ReadStreamHeader(
        uint8_t const* buffer,
        uint64_t& messageLength,
        StructuredMessageFlags& flags,
        uint16_t& segmentCount);

    /**
     * @brief Reads the segment header from the buffer.
     *
     * @param buffer Pointer to a buffer of at least SegmentHeaderLength bytes containing the
     * header.
     * @param segmentNumber Output parameter for the segment number.
     * @param segmentLength Output parameter for the segment content length.
     */
    static void ReadSegmentHeader(
        uint8_t const* buffer,
        uint16_t& segmentNumber,
        uint64_t& segmentLength);
  };

}}} // namespace Azure::Storage::_internal