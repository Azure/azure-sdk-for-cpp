// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <algorithm>
#include <functional>
#include <memory>
#include <vector>

namespace Azure { namespace Storage { namespace _internal {

  enum class StructuredMessageFlags
  {
    None = 0x0,
    Crc64 = 0x1,
  };

  enum class StructuredMessageCurrentRegion
  {
    StreamHeader,
    SegmentHeader,
    SegmentContent,
    SegmentFooter,
    StreamFooter,
    Completed
  };

  /**
   * @brief Helper class for reading and writing structured message format headers and footers.
   */
  class StructuredMessageHelper final {
  public:
    /** @brief Length of CRC64 checksum in bytes. */
    static constexpr size_t Crc64Length = 8;

    /** @brief Current structured message format version. */
    static constexpr uint8_t StructuredMessageVersion = 1;

    /** @brief Total length of the stream header in bytes. */
    static constexpr size_t StreamHeaderLength = 13;
    /** @brief Offset of the version field within the stream header. */
    static constexpr int64_t StreamHeaderVersionOffset = 0;
    /** @brief Offset of the message length field within the stream header. */
    static constexpr int64_t StreamHeaderMessageLengthOffset = 1;
    /** @brief Offset of the flags field within the stream header. */
    static constexpr int64_t StreamHeaderFlagsOffset = 9;
    /** @brief Offset of the segment count field within the stream header. */
    static constexpr int64_t StreamHeaderSegmentCountOffset = 11;

    /** @brief Total length of the segment header in bytes. */
    static constexpr size_t SegmentHeaderLength = 10;
    /** @brief Offset of the segment number field within the segment header. */
    static constexpr int64_t SegmentHeaderNumOffset = 0;
    /** @brief Offset of the segment content length field within the segment header. */
    static constexpr int64_t SegmentHeaderContentLengthOffset = 2;

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
     * @brief Writes a CRC64 checksum to the buffer in little-endian format.
     *
     * @param buffer Pointer to a buffer of at least Crc64Length bytes where the CRC64 will be
     * written.
     * @param bufferSize Size of the buffer in bytes.
     * @param crc64 The CRC64 checksum bytes to write. If empty, no data is written.
     */
    static void WriteCrc64(uint8_t* buffer, size_t bufferSize, std::vector<uint8_t> const& crc64);

    /**
     * @brief Reads a CRC64 checksum from the buffer.
     *
     * @param buffer Pointer to a buffer of at least Crc64Length bytes containing the CRC64.
     * @param bufferSize Size of the buffer in bytes.
     * @return The CRC64 checksum as a vector of bytes.
     */
    static std::vector<uint8_t> ReadCrc64(uint8_t const* buffer, size_t bufferSize);

    /**
     * @brief Reads the stream header from the buffer.
     *
     * @param buffer Pointer to a buffer of at least StreamHeaderLength bytes containing the
     * header.
     * @param bufferSize Size of the buffer in bytes.
     * @param messageLength Output parameter for the total message length.
     * @param flags Output parameter for the message flags.
     * @param segmentCount Output parameter for the total segment count.
     */
    static void ReadStreamHeader(
        uint8_t const* buffer,
        size_t bufferSize,
        uint64_t& messageLength,
        StructuredMessageFlags& flags,
        uint16_t& segmentCount);

    /**
     * @brief Reads the segment header from the buffer.
     *
     * @param buffer Pointer to a buffer of at least SegmentHeaderLength bytes containing the
     * header.
     * @param bufferSize Size of the buffer in bytes.
     * @param segmentNumber Output parameter for the segment number.
     * @param segmentLength Output parameter for the segment content length.
     */
    static void ReadSegmentHeader(
        uint8_t const* buffer,
        size_t bufferSize,
        uint16_t& segmentNumber,
        uint64_t& segmentLength);
  };

}}} // namespace Azure::Storage::_internal